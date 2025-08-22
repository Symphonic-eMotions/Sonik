//
//  ProgressionPlayback.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 22/08/2025.
//


import Foundation

final class ProgressionPlayback {
    private var scheduled: [DispatchWorkItem] = []
    private(set) var isPlaying = false

    // Publieke API
    func play(prog: Progression,
              via rnbo: RNBOAudioUnitHostModel,
              baseOctave: Int = 4,
              velocity: UInt8 = 96,
              channel: UInt8 = 0,
              repeatCount: Int = 8) {
        stop(via: rnbo) // safety
        
        let (beatsPerBar, _) = parseTimeSignature(prog.meta.timeSignature) ?? (4, 4)
        let secPerBeat = 60.0 / max(1.0, prog.meta.tempo)
        let secPerBar  = Double(beatsPerBar) * secPerBeat
        
        let baseEvents = prog.timeline.sorted { $0.bar < $1.bar }
        
        // ⬇️ Bepaal effectieve patroonlengte (in maten)
        let maxEndFromEvents = baseEvents.map { $0.bar + $0.lengthBars - 1 }.max() ?? 0
        let patternBars = max(maxEndFromEvents, prog.meta.bars)
        let safePatternBars = max(1, patternBars)
        
        let startAnchor = DispatchTime.now()
        isPlaying = true
        
        // Plan 8 (repeatCount) herhalingen achter elkaar
        for rep in 0..<repeatCount {
            let barOffset = rep * safePatternBars
            
            for ev in baseEvents {
                let startBar = max(1, ev.bar + barOffset)
                let startSec = Double(startBar - 1) * secPerBar
                let durSec   = Double(max(1, ev.lengthBars)) * secPerBar
                let gate     = max(0.05, durSec * 0.92)
                
                let midiNotes = resolveToMIDINotes(spec: ev.chord,
                                                   keyString: prog.meta.key,
                                                   baseOctave: baseOctave)
                
                // NOTE ON
                let onTask = DispatchWorkItem { [weak rnbo] in
                    guard let rnbo = rnbo, self.isPlaying else { return }
                    let voiced = self.voice(midiNotes, lowerRoot: false)
                    for n in voiced { rnbo.sendNoteOn(n, velocity: velocity, channel: channel) }
                }
                scheduled.append(onTask)
                DispatchQueue.main.asyncAfter(deadline: startAnchor + startSec, execute: onTask)
                
                // NOTE OFF
                let offTask = DispatchWorkItem { [weak rnbo] in
                    guard let rnbo = rnbo else { return }
                    let voiced = self.voice(midiNotes, lowerRoot: false)
                    for n in voiced { rnbo.sendNoteOff(n, releaseVelocity: 0, channel: channel) }
                }
                scheduled.append(offTask)
                DispatchQueue.main.asyncAfter(deadline: startAnchor + startSec + gate, execute: offTask)
            }
        }
    }

    func stop(via rnbo: RNBOAudioUnitHostModel) {
        isPlaying = false
        scheduled.forEach { $0.cancel() }
        scheduled.removeAll()
        rnbo.sendAllNotesOff()
    }

    // MARK: - Helpers

    // "4/4" → (4,4)
    private func parseTimeSignature(_ ts: String) -> (Int, Int)? {
        let parts = ts.split(separator: "/")
        guard parts.count == 2,
              let num = Int(parts[0].trimmingCharacters(in: .whitespaces)),
              let den = Int(parts[1].trimmingCharacters(in: .whitespaces)),
              num > 0, den > 0
        else { return nil }
        return (num, den)
    }

    // Eenvoudige voicing: root evt. -12 voor body (alleen als 4+ noten niet gewenst)
    private func voice(_ notes: [UInt8], lowerRoot: Bool) -> [UInt8] {
        guard let root = notes.min() else { return notes }
        guard lowerRoot else { return Array(notes.sorted()) }
        let lowerRootNote: UInt8 = root >= 12 ? root &- 12 : root
        var out = Array(notes.sorted())
        if !out.contains(lowerRootNote) { out.insert(lowerRootNote, at: 0) }
        return out
    }

    // ChordSpec → MIDI noten (triad/7 etc) in gegeven key + baseOctave
    private func resolveToMIDINotes(spec: ChordSpec,
                                    keyString: String,
                                    baseOctave: Int) -> [UInt8] {
        let rootPC: Int = {
            if let deg = spec.degree {
                return degreeToPitchClass(deg, keyString: keyString)
            } else if let r = spec.root {
                return nameToPitchClass(r) ?? 0 // fallback C
            } else { return 0 }
        }()
        
        let intervals = qualityIntervals(spec.quality)
        let baseMidiRoot = 12 * (baseOctave + 1)  // 4 -> 60
        return intervals.map { i in UInt8(clamping: baseMidiRoot + rootPC + i) }}

    // MAJOR / MINOR schaal op basis van keyString
    private func degreeToPitchClass(_ degree: String, keyString: String) -> Int {
        let (rootName, mode) = ProgressionTheory.parseKey(keyString).map { ($0.root, $0.mode) } ?? ("C", .major)
        let tonic = nameToPitchClass(rootName) ?? 0
        // Diatonische graden (0..6) in semitones vanaf tonic
        let majorSteps = [0, 2, 4, 5, 7, 9, 11]
        let minorSteps = [0, 2, 3, 5, 7, 8, 10]

        let normalized = degree.replacingOccurrences(of: "°", with: "") // treat dim symbol only for quality
        let map: [String:Int] = ["I":0,"iii":2,"IV":3,"V":4,"vi":5,"vii":6,
                                 "i":0,"ii":1,"III":2,"iv":3,"v":4,"VI":5,"VII":6]

        let idx = map[normalized] ?? 0
        let steps = (mode == .major) ? majorSteps : minorSteps
        return (tonic + steps[idx]) % 12
    }

    private func nameToPitchClass(_ name: String) -> Int? {
        let table: [String:Int] = [
            "C":0, "B#":0,
            "C#":1, "Db":1,
            "D":2,
            "D#":3, "Eb":3,
            "E":4, "Fb":4,
            "F":5, "E#":5,
            "F#":6, "Gb":6,
            "G":7,
            "G#":8, "Ab":8,
            "A":9,
            "A#":10, "Bb":10,
            "B":11, "Cb":11
        ]
        return table[name.trimmingCharacters(in: .whitespacesAndNewlines).uppercased()]
    }

    private func qualityIntervals(_ q: ChordQuality) -> [Int] {
        switch q {
        case .maj:   return [0,4,7]
        case .m:     return [0,3,7]
        case .dim:   return [0,3,6]
        case .aug:   return [0,4,8]
        case .seven: return [0,4,7,10]
        case .maj7:  return [0,4,7,11]
        case .m7:    return [0,3,7,10]
        case .m7b5:  return [0,3,6,10]
        case .dim7:  return [0,3,6,9]
        case .sus2:  return [0,2,7]
        case .sus4:  return [0,5,7]
        }
    }
}

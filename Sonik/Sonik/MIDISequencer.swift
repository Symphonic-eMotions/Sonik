//
//  MIDISequencer.swift
//  SwiftRNBO_Example_multiplatfrom_SwiftUI
//
//  Created by Frans-Jan Wind on 17/07/2025.
//

import AudioKit

extension Comparable {
    func clamped(to limits: ClosedRange<Self>) -> Self {
        min(max(self, limits.lowerBound), limits.upperBound)
    }
}

// MARK: - Progression → Sequencer (block chords, no ARP yet)
extension MIDISequencer {

    /// Zet de volledige progression als blokakkoorden in de sequencer (één track).
    /// - Parameters:
    ///   - prog: Progressie uit je editor/JSON
    ///   - baseOctave: 4 => C4-gebied (MIDI 60 voor C)
    ///   - velocity: 0...127
    ///   - gate: fractie van de chordduur (0.92 = iets korter voor loslaten)
    ///   - channel: MIDI-kanaal
    func loadProgressionAsBlockChords(
        _ prog: Progression,
        baseOctave: Int = 4,
        velocity: MIDIVelocity = 100,
        gate: Double = 0.92,
        channel: MIDIChannel = 0
    ) {
        clearAllTracks()

        guard let track = sequencer.newTrack() else {
            print("❌ Kan geen nieuwe track maken.")
            return
        }
        track.setMIDIOutput(callbackInstrument.midiIn)

        // Sequencer tempo uit progression
        sequencer.setTempo(prog.meta.tempo)

        let (beatsPerBar, _) = parseTimeSignature(prog.meta.timeSignature) ?? (4, 4)
        let sorted = prog.timeline.sorted { $0.bar < $1.bar }

        for ev in sorted {
            let startBeats = Double(ev.bar - 1) * Double(beatsPerBar)
            let durBeats   = Double(max(1, ev.lengthBars)) * Double(beatsPerBar)
            let chordBeats = max(0.05, durBeats * gate)

            // Resolve chord → MIDI-noten
            let chordNotes = resolveChordToMIDINotes(
                spec: ev.chord,
                keyString: prog.meta.key,
                baseOctave: baseOctave
            )

            // Voeg alle noten tegelijk toe (blokakkoord)
            for n in chordNotes.sorted() {
                track.add(
                    noteNumber: n,
                    velocity: velocity,
                    position: Duration(beats: startBeats),
                    duration: Duration(beats: chordBeats),
                    channel: channel
                )
            }
        }

        // Sequencer lengte precies op content / hele maat afronden
        noteEvents = track.getMIDINoteData()
        sourceTrackEvents = noteEvents

        quantizeLoopToContent() // gebruikt jouw loopBeatsPerBar/autoPlay
        print("🎼 Progression geladen in sequencer: \(noteEvents.count) events")
    }

    // MARK: - Helpers

    private func parseTimeSignature(_ ts: String) -> (Int, Int)? {
        let parts = ts.split(separator: "/")
        guard parts.count == 2,
              let num = Int(parts[0].trimmingCharacters(in: .whitespaces)),
              let den = Int(parts[1].trimmingCharacters(in: .whitespaces)),
              num > 0, den > 0 else { return nil }
        return (num, den)
    }

    private func resolveChordToMIDINotes(
        spec: ChordSpec,
        keyString: String,
        baseOctave: Int
    ) -> [MIDINoteNumber] {
        let rootPC: Int = {
            if let deg = spec.degree {
                return degreeToPitchClass(deg, keyString: keyString)
            } else if let r = spec.root {
                return nameToPitchClass(r) ?? 0
            } else { return 0 }
        }()

        let intervals = qualityIntervals(spec.quality)
        // baseOctave: 4 → C4 = 60
        let baseMidiRoot = 12 * (baseOctave + 1)
        return intervals.map { i in MIDINoteNumber(clamping: baseMidiRoot + rootPC + i) }
    }

    private func degreeToPitchClass(_ degree: String, keyString: String) -> Int {
        let (rootName, mode) = ProgressionTheory.parseKey(keyString).map { ($0.root, $0.mode) } ?? ("C", .major)
        let tonic = nameToPitchClass(rootName) ?? 0
        let majorSteps = [0, 2, 4, 5, 7, 9, 11]
        let minorSteps = [0, 2, 3, 5, 7, 8, 10]

        let normalized = degree.replacingOccurrences(of: "°", with: "")
        let map: [String:Int] = ["I":0,"ii":1,"iii":2,"IV":3,"V":4,"vi":5,"vii":6,
                                 "i":0,"III":2,"iv":3,"v":4,"VI":5,"VII":6]

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

// MARK: - Progression → Sequencer (ARP, hot-swap)
extension MIDISequencer {

    /// Genereer ARP‑events voor de volledige progression.
    /// - pattern: indexen in chordNotes (bv. [0,1,2,1])
    /// - gridResolutionBeats: stapgrootte in beats (bv. 0.25 = zestienden)
    /// - noteLengthFactor: fractie vd grid voor gate (0.9 = iets korter dan stap)
    func loadProgressionAsArpeggio(
        _ prog: Progression,
        baseOctave: Int = 4,
        velocity: MIDIVelocity = 100,
        channel: MIDIChannel = 0,
        pattern: [Int] = [0, 1, 2, 1],
        octaveRange: Int = 1,
        gridResolutionBeats: Double = 0.25,
        noteLengthFactor: Double = 0.92,
        swapAtLoopBoundary: Bool = true
    ) {
        // Bouw de events
        let events = buildArpeggioEvents(
            prog,
            baseOctave: baseOctave,
            velocity: velocity,
            channel: channel,
            pattern: pattern,
            octaveRange: octaveRange,
            grid: gridResolutionBeats,
            gateFactor: noteLengthFactor
        )

        // Hot‑swap in de sequencer (zonder playback te stoppen)
        hotSwapActiveTrack(with: events, swapAtLoopBoundary: swapAtLoopBoundary)
    }

    // MARK: Builder

    private func buildArpeggioEvents(
        _ prog: Progression,
        baseOctave: Int,
        velocity: MIDIVelocity,
        channel: MIDIChannel,
        pattern: [Int],
        octaveRange: Int,
        grid: Double,
        gateFactor: Double
    ) -> [MIDINoteData] {

        sequencer.setTempo(prog.meta.tempo)

        let (beatsPerBar, _) = parseTimeSignature(prog.meta.timeSignature) ?? (4, 4)
        let sorted = prog.timeline.sorted { $0.bar < $1.bar }

        var out: [MIDINoteData] = []

        for ev in sorted {
            let startBeats = Double(ev.bar - 1) * Double(beatsPerBar)
            let endBeats   = startBeats + Double(max(1, ev.lengthBars)) * Double(beatsPerBar)

            // Chord → basisnotenset
            let chordNotes = resolveChordToMIDINotes(
                spec: ev.chord,
                keyString: prog.meta.key,
                baseOctave: baseOctave
            )
            guard !chordNotes.isEmpty, !pattern.isEmpty else { continue }

            // Loop door tijdvak [start, end) in grid‑stappen
            var cursor = startBeats
            var stepIx = 0

            while cursor < endBeats {
                // voor elk patroon‑element + octaaflaag
                for oct in 0..<octaveRange {
                    let idx = pattern[stepIx % pattern.count]
                    let base = chordNotes[idx % chordNotes.count]
                    let note = MIDINoteNumber(Int(base) + (oct * 12))

                    let qStart = (cursor / grid).rounded(.toNearestOrEven) * grid
                    if qStart >= endBeats { break }

                    // gate binnen deze grid‑cel
                    let stepEnd = min(qStart + grid, endBeats)
                    let dur = max(0.01, (stepEnd - qStart) * gateFactor)

                    out.append(
                        MIDINoteData(
                            noteNumber: note,
                            velocity: velocity,
                            channel: channel,
                            duration: Duration(beats: dur),
                            position: Duration(beats: qStart)
                        )
                    )
                }
                stepIx += 1
                cursor += grid
            }
        }

        return out.sorted { a, b in
            let ta = a.position.beats, tb = b.position.beats
            return (ta, a.noteNumber) < (tb, b.noteNumber)
        }
    }

    // MARK: Hot-swap

    /// Vervang de actieve track door een nieuwe met de aangeleverde events, optioneel precies op de volgende loopgrens.
    private func hotSwapActiveTrack(
        with events: [MIDINoteData],
        swapAtLoopBoundary: Bool
    ) {
        // 1) Maak een nieuwe track met events
        guard let newTrack = sequencer.newTrack() else {
            print("❌ Kon geen nieuwe track aanmaken (ARP)")
            return
        }
        newTrack.setMIDIOutput(callbackInstrument.midiIn)
        newTrack.clear()
        for e in events { newTrack.add(midiNoteData: e) }

        // 2) Lengte op content kwantiseren & loop activeren
        //    (we hergebruiken je eigen helper flow)
        //    NB: we zetten tijdelijk 'newTrack' als enige zodat helpers op 'first track' werken.
        //    -> Alternatief: we doen de length/loop hieronder zelf:

        let lastEnd = events.map { $0.position.beats + $0.duration.beats }.max() ?? 0.0
        let beatsPerBar = loopBeatsPerBar
        let fullBars = floor(lastEnd / beatsPerBar)
        let remainder = lastEnd - fullBars * beatsPerBar
        let quantizedBeats = (remainder <= loopSmallOverlapThresholdBeats)
            ? max(fullBars * beatsPerBar, beatsPerBar)
            : (fullBars + 1) * beatsPerBar

        sequencer.setLength(Duration(beats: quantizedBeats))
        sequencer.setLoopInfo(Duration(beats: quantizedBeats), loopCount: 0)
        sequencer.enableLooping()

        // 3) Bepaal wanneer we swappen
        let performSwap = { [weak self] in
            guard let self = self else { return }
            // mute & verwijder oude tracks
            for idx in self.sequencer.tracks.indices.dropLast().reversed() {
                self.sequencer.tracks[idx].clear()
                self.sequencer.deleteTrack(trackIndex: idx)
            }
            // 'newTrack' is nu de enige over, update state
            self.noteEvents = newTrack.getMIDINoteData()
            self.sourceTrackEvents = self.noteEvents
            print("♻️ Hot‑swap complete (\(self.noteEvents.count) events)")
        }

        if swapAtLoopBoundary {
            let posBeats = sequencer.currentPosition.beats   // altijd beschikbaar
            let loopLen = quantizedBeats
            let offsetInLoop = posBeats.truncatingRemainder(dividingBy: loopLen)
            let beatsUntilSwap = loopLen - offsetInLoop
            let secondsPerBeat = 60.0 / max(1.0, sequencer.tempo)
            let delay = beatsUntilSwap * secondsPerBeat

            DispatchQueue.main.asyncAfter(deadline: .now() + delay) {
                performSwap()
            }
            print("⏳ Hot-swap gepland over \(String(format: "%.2f", beatsUntilSwap)) beats")
        } else {
            // direct, kan een mini-dubbeling geven binnen lopende maat
            performSwap()
        }
    }
}


/// Een sequencer die MIDI-events laadt of genereert,
/// en deze via DispatchQueue op tijd stuurt naar de RNBOAudioUnitHostModel.
class MIDISequencer: ObservableObject {
    private let sequencer = AppleSequencer()
    private let callbackInstrument: MIDICallbackInstrument
    private(set) var sourceTrackEvents: [MIDINoteData] = []
    @Published private(set) var noteEvents: [MIDINoteData] = []
    @Published var loopBeatsPerBar: Double = 4.0
    @Published var loopSmallOverlapThresholdBeats: Double = 0.25
    @Published var loopAutoPlay: Bool = false
    private var sequenceLength: TimeInterval = 0
    private var isPlaying = false
    private weak var rnbo: RNBOAudioUnitHostModel?

    init(rnbo: RNBOAudioUnitHostModel) {
        self.rnbo = rnbo

        callbackInstrument = MIDICallbackInstrument { [weak rnbo] status, noteNumber, velocity in
            guard let rnbo = rnbo else { return }

            let command = status & 0xF0
            let channel = status & 0x0F

            switch command {
            case 0x90 where velocity > 0:
                rnbo.audioUnit.sendNoteOnMessage(withPitch: noteNumber,
                                                          velocity: velocity,
                                                          channel: UInt8(channel))
            case 0x80, 0x90:
                rnbo.audioUnit.sendNoteOffMessage(withPitch: noteNumber,
                                                           releaseVelocity: 0,
                                                           channel: UInt8(channel))
            default:
                break
            }
        }
        sequencer.enableLooping()
    }

    /// Ruim alle tracks en geplande events op.
    func clearAllTracks() {
        // Verwijder alle bestaande tracks
        for index in sequencer.tracks.indices.reversed() {
            sequencer.tracks[index].clear()  // lege track
            sequencer.deleteTrack(trackIndex: index)
        }
        noteEvents.removeAll()
        sequenceLength = 0
        stop()
    }

    /// Laad een `.mid` uit de bundle (zonder extensie meegeven).
    func loadMIDIFile(named fileName: String) {
        clearAllTracks()
        guard let url = Bundle.main.url(forResource: fileName, withExtension: "mid") else {
            print("❌ MIDI \(fileName).mid niet gevonden in bundle.")
            return
        }
        
        sequencer.loadMIDIFile(fromURL: url)
        // We pakken de eerste track
        if let track = sequencer.tracks.first {
            noteEvents = track.getMIDINoteData()
            sourceTrackEvents = noteEvents
            sequenceLength = sequencer.length.seconds
            track.setMIDIOutput(callbackInstrument.midiIn)
            sequencer.setLength(sequencer.length)
            // Na inladen en output-koppeling:
            quantizeLoopToContent()
            print("✅ MIDI geladen: \(noteEvents.count) events, lengte \(sequenceLength)s")
        }
    }
    
    /// Herschrijf de actieve track met een octaafversie van de originele brondata
    func applyOctaveShiftToSource() {
        guard let rnbo = rnbo else { return }

        // Verwijder bestaande doeltrack
        for index in sequencer.tracks.indices.reversed() {
            sequencer.tracks[index].clear()
            sequencer.deleteTrack(trackIndex: index)
        }

        guard let track = sequencer.newTrack() else {
            print("❌ Kon geen doeltrack aanmaken")
            return
        }
        track.setMIDIOutput(callbackInstrument.midiIn)

        // 0 = origineel, +1 = +12, -1 = -12
        let transposeSemitones = rnbo.currentOctave * 12

        // Transponeer noten en voeg toe
        for event in sourceTrackEvents {
            
            let transposed = Int(event.noteNumber) + transposeSemitones
            let clamped = transposed.clamped(to: 0...127)

            let shiftedNote = MIDINoteData(
                noteNumber: MIDINoteNumber(clamped),
                velocity: event.velocity,
                channel: event.channel,
                duration: event.duration,
                position: event.position
            )
            track.add(midiNoteData: shiftedNote)
        }

        noteEvents = track.getMIDINoteData()
//        sequenceLength = sequencer.length.seconds
        quantizeLoopToContent()

        print("⤴️ Octaaftranspositie toegepast op \(noteEvents.count) events")
    }


    /// Start de playback-loop
    func play() {
        guard !isPlaying else { return }
        isPlaying = true
        sequencer.rewind()
        sequencer.play()
        print("▶︎ Sequencer gestart")
    }


    /// Stop playback en breek geplande DispatchWorkItems af
    func stop() {
        guard isPlaying else { return }
        isPlaying = false
        sequencer.stop()
        noteEvents.map(\.noteNumber).forEach {
            rnbo?.audioUnit.sendNoteOffMessage(withPitch: $0, releaseVelocity: 0, channel: 0)
        }
        print("■ Sequencer gestopt")
    }
        
    //–– Helpers ––
    private func reset() {
        sequencer.tracks.forEach { $0.clear() }
        sequencer.tracks.indices.reversed().forEach { sequencer.deleteTrack(trackIndex: $0) }
        noteEvents = []
        sequenceLength = 0
        stop()
    }

    private func extractEvents() {
        if let t = sequencer.tracks.first {
            noteEvents = t.getMIDINoteData()
            sequenceLength = sequencer.length.seconds
        }
    }
    private func clipActiveTrackToLoop(_ loopBeats: Double) {
        guard let track = sequencer.tracks.first else { return }

        let events = track.getMIDINoteData()
        let clipped: [MIDINoteData] = events.compactMap { e in
            let start = e.position.beats
            // Noot start buiten de loop -> drop
            guard start < loopBeats else { return nil }

            let end = start + e.duration.beats
            let newDur = max(0, min(end, loopBeats) - start)
            guard newDur > 0 else { return nil }

            var n = e
            n.duration = Duration(beats: newDur)
            return n
        }

        track.clear()
        for n in clipped {
            track.add(midiNoteData: n)
        }

        // Zorg dat de sequencerlengte exact op de loop staat
        sequencer.setLength(Duration(beats: loopBeats))

        // State bijwerken
        noteEvents = track.getMIDINoteData()
        sequenceLength = sequencer.length.seconds
    }
    
    private func lastEventEndInBeats(from events: [MIDINoteData]) -> Double {
        events.map { $0.position.beats + $0.duration.beats }.max() ?? 0
    }
    
    func quantizeLoopToContent() {
        let beatsPerBar = loopBeatsPerBar
        let smallOverlapThresholdBeats = loopSmallOverlapThresholdBeats
        let autoPlay = loopAutoPlay

        // 1) Laatste eindtijd in beats
        let lastEnd = lastEventEndInBeats(from: noteEvents)

        // 2) Hele maten met “kleine overlap -> naar beneden”
        let fullBars = floor(lastEnd / beatsPerBar)
        let remainder = lastEnd - (fullBars * beatsPerBar)

        let quantizedBeats: Double = {
            if remainder <= smallOverlapThresholdBeats {
                return max(fullBars * beatsPerBar, beatsPerBar) // min 1 maat
            } else {
                return (fullBars + 1) * beatsPerBar
            }
        }()

        // 3) Lengte + clip + loop
        sequencer.setLength(Duration(beats: quantizedBeats))
        clipActiveTrackToLoop(quantizedBeats)

        sequencer.setLoopInfo(Duration(beats: quantizedBeats), loopCount: 0)
        sequencer.enableLooping()

        if autoPlay { play() }

        print("🔁 Loop op \(quantizedBeats) beats (\(quantizedBeats / beatsPerBar) maten)")
    }
}

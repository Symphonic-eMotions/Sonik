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

/// Een sequencer die MIDI-events laadt of genereert,
/// en deze via DispatchQueue op tijd stuurt naar de RNBOAudioUnitHostModel.
class MIDISequencer: ObservableObject {
    private let sequencer = AppleSequencer()
    private let callbackInstrument: MIDICallbackInstrument
    private(set) var sourceTrackEvents: [MIDINoteData] = []
    @Published private(set) var noteEvents: [MIDINoteData] = []
    @Published var loopBeatsPerBar: Double = 4.0
    @Published var loopSmallOverlapThresholdBeats: Double = 0.25
    @Published var loopAutoPlay: Bool = true
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


    // Nieuwe methode voor Arpeggio-generatie
    func generateArpeggioSequence(
        chordNotes: [MIDINoteNumber] = [60, 64, 67],
        pattern: [Int] = [0, 1, 2, 1],
        octaveRange: Int = 1,
        repeats: Int = 4,
        noteDuration: Duration = Duration(beats: 0.25),
        velocity: MIDIVelocity = 100
    ) {
        clearAllTracks()
        
        guard let track = sequencer.newTrack() else {
            print("❌ Kan geen nieuwe track maken.")
            return
        }
        track.setMIDIOutput(callbackInstrument.midiIn)
        
        // 16e noot grid definitie
        let gridResolution = 0.25
        var currentBeat = 0.0
        
        for _ in 0..<repeats {
            for octave in 0..<octaveRange {
                for index in pattern {
                    let midiNote = chordNotes[index % chordNotes.count] + MIDINoteNumber(octave * 12)
                    // Zorg dat elke noot exact op het 16e-grid geplaatst wordt
                    let quantizedBeat = (currentBeat / gridResolution).rounded() * gridResolution
                    track.add(noteNumber: midiNote,
                              velocity: velocity,
                              position: Duration(beats: quantizedBeat),
                              duration: noteDuration)
                    currentBeat += gridResolution
                }
            }
        }
        
        // Sequence-length precies op maat zetten
        sequencer.setLength(Duration(beats: currentBeat))
        
        noteEvents = track.getMIDINoteData()
        sequenceLength = currentBeat * (60.0 / sequencer.tempo)
        
        sourceTrackEvents = noteEvents
        
        quantizeLoopToContent()
        
        print("🎶 Strakke arpeggio sequence: \(noteEvents.count) events")
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

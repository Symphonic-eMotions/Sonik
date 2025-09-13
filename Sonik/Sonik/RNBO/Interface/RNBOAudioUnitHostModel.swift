//
//  RNBOController.swift
//  SwiftUI_RNBO_FromScratch_Test
//
//  Created by Eldar Sadykov on 13.02.2023.
//

import Foundation
import AudioKit
import RNBOKit

typealias RNBOContext = RNBOAudioUnitHostModel

extension RNBOAudioUnitHostModel {
    /// Biedt de AVAudioUnit-node voor MIDI-routing
    func getAVAudioUnitNode() -> AVAudioUnit? {
        return audioEngine.getAVAudioUnitNode()
    }
}

extension RNBOAudioUnitHostModel {
    
    private var configFileName: String { "parameterConfig" }
    
    static func loadParameterConfiguration(
        from parameters: [RNBOParameter],
        configFileName: String = "parameterConfig"
    ) -> [ParameterConfig] {
        guard let configURL = Bundle.main.url(forResource: configFileName, withExtension: "json") else {
            printExampleParameterConfig(parameters: parameters)
            return parameters.map {
                ParameterConfig(id: $0.id, displayName: $0.info.name, visible: true)
            }
        }

        do {
            let data = try Data(contentsOf: configURL)
            let loadedConfigs = try JSONDecoder().decode([ParameterConfig].self, from: data)
            return loadedConfigs
        } catch {
            print("Error loading parameter config: \(error)")
            printExampleParameterConfig(parameters: parameters)
            return parameters.map {
                ParameterConfig(id: $0.id, displayName: $0.info.name, visible: true)
            }
        }
    }

    static func printExampleParameterConfig(parameters: [RNBOParameter]) {
        let exampleConfig = parameters.map {
            ParameterConfig(id: $0.id, displayName: $0.info.name, visible: true)
        }
        do {
            let encoder = JSONEncoder()
            encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
            let jsonData = try encoder.encode(exampleConfig)
            let jsonString = String(data: jsonData, encoding: .utf8)!
            print("""
            === Example parameterConfig.json ===
            \(jsonString)
            ====================================
            """)
        } catch {
            print("Failed to encode example config: \(error)")
        }
    }
}

extension RNBOAudioUnitHostModel {

    /// Simuleert handmatige taps via dezelfde codepad als je on-screen keyboard.
    /// - Parameters:
    ///   - silently: zet volume tijdelijk uit met fade, en herstelt het aan het einde
    ///   - fade: duur van fade in/uit bij silently
    func warmUpBySimulatedKeyboardTaps(
        note: UInt8,
        velocity: UInt8,
        taps: Int,
        interval: TimeInterval,
        gate: TimeInterval,
        silently: Bool,
        fade: TimeInterval,
        tailPadding: TimeInterval
    ) {
        struct State { static var running = false }
        if State.running {
            print("[warmUpSimTap] ⏳ Al bezig—overslaan")
            return
        }
        State.running = true

        let vel = UInt8(max(1, min(127, Int(velocity))))
        var count = 0

        // Stil warm-uppen? Eerst zacht naar 0
        if silently { rampSynthVolume(to: 0.0, over: fade) }

        print("[warmUpSimTap] ▶️ start: note=\(note) taps=\(taps) interval=\(interval)s gate=\(gate)s silently=\(silently)")

        func finish() {
            let recoveryDelay = silently ? tailPadding : 0.0
            if silently {
                DispatchQueue.main.asyncAfter(deadline: .now() + recoveryDelay) {
                    self.rampSynthVolume(to: 1.0, over: fade)
                }
            }
            State.running = false
            print("[warmUpSimTap] ✅ klaar (\(taps)/\(taps))")
        }
        
        func tapOnce() {
            guard count < taps else { finish(); return }
            count += 1
            let idx = count
            
            self.sendNoteOn(note, velocity: vel)
            print("[warmUpSimTap] 🔔 tap \(idx)/\(taps): noteOn \(note)")
            
            DispatchQueue.main.asyncAfter(deadline: .now() + gate) {
                self.sendNoteOff(note)
                print("[warmUpSimTap] ⏹️ tap \(idx)/\(taps): noteOff \(note)")
                
                let wait = max(0, interval - gate)
                DispatchQueue.main.asyncAfter(deadline: .now() + wait) {
                    tapOnce()
                }
            }
        }
        
        DispatchQueue.main.async { tapOnce() }
    }
}

extension RNBOAudioUnitHostModel {
    @discardableResult
    func setParameterValueNormalized(byId id: String, to value: Double) -> Bool {
        guard let idx = paramIndexById[id] else {
            print("⚠️ setParameterValueNormalized: paramId '\(id)' niet gevonden")
            return false
        }
        let v = value.clamped(to: 0...1)
        audioUnit.setParameterValueNormalized(idx, valueNormalized: Float(v))
        // Houd lokale mirror bij (voor UI sync)
        parameters[idx].valueNormalized = v
        return true
    }

    func getParameterValueNormalized(byId id: String) -> Double? {
        guard let idx = paramIndexById[id] else { return nil }
        return parameters[idx].valueNormalized
    }
}

extension RNBOAudioUnitHostModel {
    func indexForParameter(id: String) -> Int? {
        if let i = paramIndexById[id] { return i }
        rebuildParamIndexMap()
        return paramIndexById[id]
    }
}

final class RNBOAudioUnitHostModel: ObservableObject {
    private let audioEngine = RNBOAudioEngine()
    private var _audioUnit: RNBOAudioUnit!
    public var audioUnit: RNBOAudioUnit { _audioUnit }
    private let eventHandler = RNBOEventHandler()
    private var paramIndexById: [String: Int] = [:]

    @Published var parameters: [RNBOParameter]
    @Published var parameterConfigs: [ParameterConfig]
    @Published var showInterface: UserInterface
    @Published var currentOctave: Int = 0
    @Published private(set) var activeMIDINotes: Set<UInt8> = []
    
    // MARK: Harmonicity state
    @Published var harmonicityScaleName: String = HarmonicityScale.names.first ?? "Equal (12-TET)"
    @Published var harmonicityIndex: Int = 0

    // Laad de schalen één keer
    lazy var harmonicityScales: [String: [Double]] = HarmonicityScale.parse()

    // Hulpfunctie: actuele collectie + waarde
    var currentHarmonicityValues: [Double] {
        harmonicityScales[harmonicityScaleName] ?? []
    }

    var currentHarmonicityValue: Double? {
        let vals = currentHarmonicityValues
        guard !vals.isEmpty else { return nil }
        let i = max(0, min(harmonicityIndex, vals.count - 1))
        return vals[i]
    }
    
    let description: RNBODescription?

    // MARK: - Progression (structuur blijft leidend)
    @Published var progression: Progression = Progression(
        meta: .init(key: "C major", timeSignature: "4/4", tempo: 120, bars: 8),
        timeline: [
            ChordEvent(bar: 1, lengthBars: 2, chord: .init(degree: "I", quality: .maj7)),
            ChordEvent(bar: 3, lengthBars: 2, chord: .init(degree: "vi", quality: .m7))
        ]
    )

    @Published var useRomanGlobal: Bool = true
    @Published var isWarmedUp: Bool = false

    private var progressionLoadedOnce = false

    // MARK: - Sequencer bridged playback (nieuw)
    lazy var midiSequencer: MIDISequencer = { MIDISequencer(rnbo: self) }()
    
    private func parameterIndex(for id: String) -> Int? {
        if let idx = paramIndexById[id] { return idx }
        // fallback: herbouw mapping 1x en probeer nogmaals
        rebuildParamIndexMap()
        return paramIndexById[id]
    }

    // Zet ruwe waarde voor param-id
    func setParameterValue(id: String, value: Double) {
        guard let idx = parameterIndex(for: id) else {
            print("⚠️ setParameterValue: onbekende id '\(id)'")
            return
        }
        setParameterValue(to: value, at: idx)
    }

    // Zet ruwe waarde + refresh (zoals je Hot-variant op index)
    func setParameterValueHot(id: String, value: Double) {
        guard let idx = parameterIndex(for: id) else {
            print("⚠️ setParameterValueHot: onbekende id '\(id)'")
            return
        }
        setParameterValueHot(to: value, at: idx)
    }

    // Zet genormaliseerd (0...1) op id
    func setParameterValueNormalized(id: String, valueNormalized: Double) {
        guard let idx = parameterIndex(for: id) else {
            print("⚠️ setParameterValueNormalized: onbekende id '\(id)'")
            return
        }
        setParameterValueNormalized(to: valueNormalized, at: idx)
    }

    func setParameterValueNormalizedHot(id: String, valueNormalized: Double) {
        guard let idx = parameterIndex(for: id) else {
            print("⚠️ setParameterValueNormalizedHot: onbekende id '\(id)'")
            return
        }
        setParameterValueNormalizedHot(to: valueNormalized, at: idx)
    }

    // Als de schaal wijzigt, clamp index en push waarde
    func changeHarmonicityScale(to name: String) {
        harmonicityScaleName = name
        let maxIdx = max(0, (currentHarmonicityValues.count - 1))
        harmonicityIndex = min(harmonicityIndex, maxIdx)
        applyHarmonicity()
    }
    
    func applyHarmonicity(index: Int? = nil) {
        if let idx = index { harmonicityIndex = idx }
        if let v = currentHarmonicityValue {
            // v is al de DOELWAARDE (bijv. 1.33484) → NIET afronden naar Int
            setParameterValueHot(id: "harmonicity", value: v)
        } else {
            print("⚠️ applyHarmonicity: geen waarde in huidige collectie")
        }
    }
    
    /// Zet de huidige progression in de sequencer als blokakkoorden (nog geen ARP).
    func loadProgressionIntoSequencerAsChords(
        baseOctave: Int = 4,
        velocity: MIDIVelocity = 100,
        gate: Double = 0.92,
        channel: MIDIChannel = 0
    ) {
        midiSequencer.loadProgressionAsBlockChords(
            progression,
            baseOctave: baseOctave,
            velocity: velocity,
            gate: gate,
            channel: channel
        )
    }

    func playSequencer() {
        midiSequencer.play()
    }

    func stopSequencer() {
        midiSequencer.stop()
        sendAllNotesOff()
    }

    // MARK: - Lifecycle / init

    init() {
        do {
            let url = Bundle.main.url(forResource: "description", withExtension: "json")!
            let data = try Data(contentsOf: url)
            description = try JSONDecoder().decode(RNBODescription.self, from: data)
        } catch {
            print("Error decoding JSON from URL: \(error)")
            description = nil
        }

        _audioUnit = audioEngine.getAudioUnit()
        let localParameters = description?.getParametersArray() ?? []
        self.parameters = localParameters
        self.parameterConfigs = Self.loadParameterConfiguration(from: localParameters)
        self.showInterface = UserInterface.xy
        rebuildParamIndexMap()
    }

    func ensureProgressionLoadedFromBundle() {
        guard !progressionLoadedOnce else { return }
        do {
            let p = try ProgressionIO.loadFromBundle(file: "chordSchema")
            self.progression = p
            self.progressionLoadedOnce = true
            print("✅ Loaded default progression from chordSchema.json")
        } catch {
            print("⚠️ Using fallback progression. Load error:", error)
        }
    }

    private func rebuildParamIndexMap() {
        self.paramIndexById = Dictionary(uniqueKeysWithValues:
            self.parameters.enumerated().map { ($1.id, $0) }
        )
    }

    // MARK: - UI helpers
    func visibleParameterConfigs() -> [ParameterConfig] {
        parameterConfigs.filter { $0.visible }
    }

    func parameterDisplayName(for id: String) -> String {
        parameterConfigs.first(where: { $0.id == id })?.displayName ?? id
    }

    // MARK: - Engine helpers (ongewijzigd)
    func playAudioFile() { audioEngine.playAudioFile() }
    func pauseAudioFile() { audioEngine.pauseAudioFile() }

    func toggleMic(_ on: Bool) {
        audioEngine.setMicrophoneAmplitude(on ? 1.0 : 0.0)
    }

    func refreshParameterValue(at parameterIndex: Int) {
        parameters[parameterIndex].value = Double(audioUnit.getParameterValue(parameterIndex))
    }

    func setParameterValue(to value: Double, at parameterIndex: Int) {
        audioUnit.setParameterValue(parameterIndex, value: Float(value))
    }

    func setParameterValueHot(to value: Double, at parameterIndex: Int) {
        setParameterValue(to: value, at: parameterIndex)
        refreshParameterValue(at: parameterIndex)
    }

    func setParameterValueNormalized(to valueNormalized: Double, at parameterIndex: Int) {
        audioUnit.setParameterValueNormalized(parameterIndex, valueNormalized: Float(valueNormalized))
    }

    func setParameterValueNormalizedHot(to valueNormalized: Double, at parameterIndex: Int) {
        setParameterValueNormalized(to: valueNormalized, at: parameterIndex)
        refreshParameterValue(at: parameterIndex)
    }

    // MARK: - RNBO MIDI (ongewijzigd + logging)
    func sendMessage(_ message: [Double]) {
        audioUnit.sendMessage("foo", list: message)
    }

    func sendNoteOn(_ pitch: UInt8, velocity: UInt8 = 127, channel: UInt8 = 0) {
        let transposedPitch = UInt8(Int(pitch) + currentOctave * 12)
        print("🎹 NOTE ON \(transposedPitch) vel \(velocity) ch \(channel)")
        audioUnit.sendNoteOnMessage(withPitch: transposedPitch, velocity: velocity, channel: channel)
        activeMIDINotes.insert(transposedPitch)
    }

    func sendNoteOff(_ pitch: UInt8, releaseVelocity: UInt8 = 0, channel: UInt8 = 0) {
        let transposedPitch = UInt8(Int(pitch) + currentOctave * 12)
        print("🎹 NOTE OFF \(transposedPitch) ch \(channel)")
        audioUnit.sendNoteOffMessage(withPitch: transposedPitch, releaseVelocity: releaseVelocity, channel: channel)
        activeMIDINotes.remove(transposedPitch)
    }

    func sendAllNotesOff(channel: UInt8 = 0) {
        for pitch in activeMIDINotes {
            audioUnit.sendNoteOffMessage(withPitch: pitch, releaseVelocity: 0, channel: channel)
        }
        activeMIDINotes.removeAll()
    }

    func sendAftertouch(_ pitch: UInt8, pressure: UInt8, channel: UInt8 = 0) {
        audioUnit.sendAftertouchMessage(withPitch: pitch, pressure: pressure, channel: channel)
    }

    func sendContinuousController(_ number: UInt8, value: UInt8, channel: UInt8 = 0) {
        audioUnit.sendContinuousController(withNumber: number, value: value, channel: channel)
    }

    func sendPatchChange(_ program: UInt8, channel: UInt8 = 0) {
        audioUnit.sendPatchChangeMessage(withProgram: program, channel: channel)
    }

    func sendChannelPressure(_ pressure: UInt8, channel: UInt8 = 0) {
        audioUnit.sendChannelPressureMessage(withPressure: pressure, channel: channel)
    }

    func sendPitchBend(_ value: UInt16, channel: UInt8 = 0) {
        audioUnit.sendPitchBendMessage(withValue: value, channel: channel)
    }

    func connectEventHandler() {
        audioUnit.setEventHandler(eventHandler)
        eventHandler.rnbo = self
    }

    func temporarilyMuteSynth(for seconds: TimeInterval, fade: TimeInterval = 0.02) {
        audioEngine.temporarilyMuteRNBO(for: seconds, fade: fade)
    }

    func setSynthMuted(_ muted: Bool) {
        audioEngine.setRNBOMute(muted)
    }

    func rampSynthVolume(to target: Float, over duration: TimeInterval = 0.08) {
        audioEngine.rampRNBOOutputVolume(to: target, over: duration)
    }
    
    func loadProgressionIntoSequencerAsArpeggio(
        baseOctave: Int = 4,
        velocity: MIDIVelocity = 100,
        channel: MIDIChannel = 0,
        pattern: [Int] = [0,1,2,1],
        octaveRange: Int = 1,
        gridResolutionBeats: Double = 0.25,
        noteLengthFactor: Double = 0.92,
        swapAtLoopBoundary: Bool = true
    ) {
        midiSequencer.loadProgressionAsArpeggio(
            progression,
            baseOctave: baseOctave,
            velocity: velocity,
            channel: channel,
            pattern: pattern,
            octaveRange: octaveRange,
            gridResolutionBeats: gridResolutionBeats,
            noteLengthFactor: noteLengthFactor,
            swapAtLoopBoundary: swapAtLoopBoundary
        )
    }
}

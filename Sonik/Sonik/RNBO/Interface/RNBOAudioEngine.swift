//
//  AudioEngine.swift
//  RNBO_AVAudioEngine
//
//  Created by Alex Nadzharov on 13.02.2023.
//

import AVFoundation
import AudioToolbox
import RNBOKit

extension RNBOAudioEngine {
    /// AVAudioUnit-node van de RNBO Audio Unit
    func getAVAudioUnitNode() -> AVAudioUnit? {
        return avAudioUnit
    }
}

final class RNBOAudioEngine {
    // Core
    private let engine = AVAudioEngine()
    private var avAudioUnit: AVAudioUnit?

    // Nodes
    private let playerNode = AVAudioPlayerNode()
    private let distortionEffect: AVAudioUnitDistortion
    private let inputMixer = AVAudioMixerNode()
    private let microphoneVolumeMixer = AVAudioMixerNode()
    private let rnboOutputMixer = AVAudioMixerNode() // <-- eigen output-mixer voor RNBO

    // Optional file player
    private let audioFile: AVAudioFile?

    public var audioUnit: RNBOAudioUnit {
        avAudioUnit!.auAudioUnit as! RNBOAudioUnit
    }

    // MARK: - Init

    init() {
        // AudioSession
        do {
            let audioSession = AVAudioSession.sharedInstance()
            let targetBufferDuration = 0.010
            try audioSession.setPreferredIOBufferDuration(targetBufferDuration)
            try audioSession.setPreferredSampleRate(48000)
            try audioSession.setPreferredIOBufferDuration(0.010)
            try audioSession.setActive(true)
            try audioSession.setCategory(.playAndRecord, options: [.defaultToSpeaker, .allowBluetoothA2DP, .mixWithOthers, .allowAirPlay])

            print("Actual sample rate: \(audioSession.sampleRate)")
            print("Actual buffer duration: \(audioSession.ioBufferDuration)")
            print("Actual input channels: \(audioSession.inputNumberOfChannels)")
            print("Actual output channels: \(audioSession.outputNumberOfChannels)")
        } catch {
            print("Audio session error: \(error.localizedDescription)")
        }

        distortionEffect = AVAudioUnitDistortion()
        distortionEffect.loadFactoryPreset(.multiEcho1)

        // Optional demo file
        if let url = Bundle.main.url(forResource: "Synth", withExtension: "aif") {
            do { audioFile = try AVAudioFile(forReading: url) } catch {
                print("Error initializing audio file: \(error)")
                audioFile = nil
            }
        } else {
            audioFile = nil
        }

        // RNBO AU instantiatie
        let desc = AudioComponentDescription(
            componentType: kAudioUnitType_Effect,
            componentSubType: 0x71717171,
            componentManufacturer: 0x70707070,
            componentFlags: 0,
            componentFlagsMask: 0
        )
        AUAudioUnit.registerSubclass(RNBOAudioUnit.self, as: desc, name: "RNBOAudioUnit", version: 1)

        AVAudioUnit.instantiate(with: desc, options: .loadOutOfProcess) { avAudioUnit, error in
            guard let avAudioUnit = avAudioUnit, error == nil else {
                print("Error instantiating AVAudioUnit: \(error!.localizedDescription)")
                return
            }
            self.avAudioUnit = avAudioUnit
            DispatchQueue.main.async { self.setupAudioChain() }
        }
    }

    // MARK: - Graph

    private func setupAudioChain() {
        guard let avAudioUnit = avAudioUnit else {
            print("avAudioUnit not ready!")
            return
        }

        // Attach
        engine.attach(inputMixer)
        engine.attach(microphoneVolumeMixer)
        engine.attach(playerNode)
        engine.attach(avAudioUnit)
        engine.attach(rnboOutputMixer)

        microphoneVolumeMixer.outputVolume = 0.0

        // Input routing
        let input = engine.inputNode
        let inputFormat = input.outputFormat(forBus: 0)
        let auInFormat = avAudioUnit.inputFormat(forBus: 0)

        if inputFormat.sampleRate != auInFormat.sampleRate {
            print("Input node sample rate: \(inputFormat.sampleRate)")
            print("AudioUnit sample rate: \(auInFormat.sampleRate)")
            engine.connect(input, to: microphoneVolumeMixer, format: inputFormat)
            engine.connect(microphoneVolumeMixer, to: inputMixer, format: inputFormat)
            engine.connect(inputMixer, to: avAudioUnit, format: auInFormat)
        } else {
            engine.connect(input, to: microphoneVolumeMixer, format: inputFormat)
            engine.connect(microphoneVolumeMixer, to: inputMixer, format: inputFormat)
            engine.connect(inputMixer, to: avAudioUnit, format: inputFormat)
        }

        // RNBO output -> eigen mixer -> mainMixer
        let auOutFormat = avAudioUnit.outputFormat(forBus: 0)
        engine.connect(avAudioUnit, to: rnboOutputMixer, format: auOutFormat)
        engine.connect(rnboOutputMixer, to: engine.mainMixerNode, format: auOutFormat)
        rnboOutputMixer.outputVolume = 1.0

        // Optional player
        if let audioFile = audioFile {
            engine.connect(playerNode, to: inputMixer, format: audioFile.processingFormat)
        }

        // main -> output
        let outputFormat = engine.outputNode.inputFormat(forBus: 0)
        engine.connect(engine.mainMixerNode, to: engine.outputNode, format: outputFormat)

        engine.prepare()
        do {
            try engine.start()
            print("Audio Engine started successfully.")
        } catch {
            print("Error starting audio engine: \(error.localizedDescription)")
        }
    }

    // MARK: - Input

    func setMicrophoneAmplitude(_ amp: Float) {
        microphoneVolumeMixer.outputVolume = amp
    }

    // MARK: - Convenience

    func getAudioUnit() -> RNBOAudioUnit {
        return avAudioUnit!.auAudioUnit as! RNBOAudioUnit
    }

    func playAudioFile() {
        playerNode.stop()
        guard let audioFile = audioFile else { return }
        playerNode.rate = 1
        playerNode.scheduleFile(audioFile, at: nil) { print("Audio playback finished") }
        playerNode.play()
    }

    func pauseAudioFile() {
        playerNode.pause()
    }
}

// MARK: - RNBO output volume helpers

extension RNBOAudioEngine {
    func setRNBOMute(_ muted: Bool) {
        DispatchQueue.main.async {
            self.rnboOutputMixer.outputVolume = muted ? 0.0 : 1.0
        }
    }

    /// Zachte volumeverandering om klikjes te vermijden
    func rampRNBOOutputVolume(to target: Float, over duration: TimeInterval = 0.08) {
        let steps = max(2, Int(duration * 60)) // ~60 Hz
        let start = rnboOutputMixer.outputVolume
        let delta = (target - start) / Float(steps)
        var i = 0
        let timer = DispatchSource.makeTimerSource(queue: .main)
        timer.schedule(deadline: .now(), repeating: duration / Double(steps))
        timer.setEventHandler { [weak self] in
            guard let self = self else { timer.cancel(); return }
            i += 1
            if i >= steps {
                self.rnboOutputMixer.outputVolume = target
                timer.cancel()
            } else {
                self.rnboOutputMixer.outputVolume = start + Float(i) * delta
            }
        }
        timer.resume()
    }

    /// Handig als je vooraf weet hoe lang de warm-up duurt
    func temporarilyMuteRNBO(for seconds: TimeInterval, fade: TimeInterval = 0.02) {
        rampRNBOOutputVolume(to: 0.0, over: fade)
        DispatchQueue.main.asyncAfter(deadline: .now() + seconds) {
            self.rampRNBOOutputVolume(to: 1.0, over: fade)
        }
    }
}

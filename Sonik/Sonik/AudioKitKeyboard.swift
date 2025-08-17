//
//  AudioKitKeyboard.swift
//  SwiftRNBO_Example_multiplatfrom_SwiftUI
//
//  Created by Eldar Sadykov on 17.11.2023.
//

#if !os(tvOS)
    import Keyboard
    import SwiftUI

    struct AudioKitKeyboard: View {
        @EnvironmentObject var rnbo: RNBOAudioUnitHostModel
        @State private var latch = false
        var body: some View {
            VStack {
                HStack {
                    Toggle(isOn: $rnbo.showParameterSelects){
                        Text("Edit")
                    }
                    Spacer()
                    Toggle(isOn: $latch) {
                        Text("Latch keys")
                    }
                }
                Keyboard(latching: latch) { pitch, point in
                    let pitch = UInt8(pitch.midiNoteNumber)
                    let velocity = latch ? 60 : UInt8(point.y * 127)
                    rnbo.sendNoteOn(pitch, velocity: velocity)
                } noteOff: { pitch in
                    let pitch = UInt8(pitch.midiNoteNumber)
                    rnbo.sendNoteOff(pitch)
                }
                .frame(height: 100)
                .frame(minWidth: 600)
            }
            .onAppear {
                
                // Warming up buffers:
                let note: UInt8 = 60
                let velocity: UInt8 = 110
                let taps = 12
                let interval: TimeInterval = 0.10
                let gate: TimeInterval = 0.10
                let fade: TimeInterval = 0.03
                let tailPadding: TimeInterval = 1.0

                rnbo.warmUpBySimulatedKeyboardTaps(
                    note: note,
                    velocity: velocity,
                    taps: taps,
                    interval: interval,
                    gate: gate,
                    silently: true,
                    fade: fade,
                    tailPadding: tailPadding
                )
            }
        }
    }

    #Preview {
        AudioKitKeyboard()
    }
#endif

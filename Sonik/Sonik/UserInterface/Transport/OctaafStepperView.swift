//
//  OctaafStepperView.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 06/08/2025.
//


import SwiftUI

struct OctaafStepperView: View {
    @EnvironmentObject var rnbo: RNBOAudioUnitHostModel
    @EnvironmentObject var sequencer: MIDISequencer
    
    let minOctave = -5
    let maxOctave = 5

    var body: some View {
        VStack(spacing: 6) {
            HStack(spacing: 15) {
                Button {
                    if rnbo.currentOctave > minOctave {
                        sequencer.stop()
                        rnbo.sendAllNotesOff()
                        rnbo.currentOctave -= 1
                        sequencer.applyOctaveShiftToSource()
                    }
                } label: {
                    Image(systemName: "minus.circle")
                }
                .buttonStyle(.borderedProminent)
                .disabled(rnbo.currentOctave <= minOctave)

                Button {
                    if rnbo.currentOctave < maxOctave {
                        sequencer.stop()
                        rnbo.sendAllNotesOff()
                        rnbo.currentOctave += 1
                        sequencer.applyOctaveShiftToSource()
                    }
                } label: {
                    Image(systemName: "plus.circle")
                }
                .buttonStyle(.borderedProminent)
                .disabled(rnbo.currentOctave >= maxOctave)
            }

            Text("Octaaf: \(rnbo.currentOctave)")
                .font(.footnote)
                .foregroundColor(.secondary)
        }
    }
}

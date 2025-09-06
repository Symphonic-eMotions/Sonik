//
//  SequencerTempoControl.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 06/09/2025.
//
import SwiftUI

struct SequencerTempoControl: View {
    @EnvironmentObject var rnbo: RNBOAudioUnitHostModel
    var displayName: String = "Tempo"

    private var binding: Binding<Double> {
        Binding(
            get: { rnbo.midiSequencer.tempoBPM },
            set: { rnbo.midiSequencer.setTempo($0) }
        )
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack {
                Text(displayName).font(.headline)
                Spacer()
                Text("\(Int(rnbo.midiSequencer.tempoBPM)) BPM")
                    .monospacedDigit()
                    .foregroundStyle(.secondary)
            }
            Slider(value: binding, in: 20...250, step: 1)
        }
        .padding(.vertical, 4)
    }
}

//
//  Sliders.swift
//  SwiftUI_RNBO_FromScratch_Test
//
//  Created by Эльдар Садыков on 13.02.2023.
//

import SwiftUI

struct Sliders: View {
    @EnvironmentObject var rnbo: RNBOAudioUnitHostModel

    var body: some View {
        ScrollView {
            VStack(spacing: 12) {
                ForEach(rnbo.visibleParameterConfigs(), id: \.id) { config in
                    switch config.id {
                    case "sequencer/tempo":
                        SequencerTempoControl(displayName: config.displayName)

                    case "harmonicity":
                        HarmonicityControl()

                    default:
                        if let idx = rnbo.indexForParameter(id: config.id) {
                            SliderView(parameter: $rnbo.parameters[idx],
                                       displayName: config.displayName)
                        } else {
                            // Optioneel: waarschuwing of fallback UI
                            EmptyView()
                        }
                    }
                }
            }
            .padding()
            .background()
            .padding(.bottom)
        }
        .frame(minHeight: 100)
        .padding(.bottom)
    }
}

struct SliderValueLabel: View {
    let value: Double
    var body: some View {
        Text(String(format: "%.2f", value))
            .frame(minWidth: 50)
    }
}

struct SliderNameLabel: View {
    let name: String
    var body: some View {
        Text(name)
            .frame(minWidth: 100)
    }
}

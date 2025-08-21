//
//  ParameterView.swift
//  SwiftRNBO_Example_multiplatfrom_SwiftUI
//
//  Created by Eldar Sadykov on 18.11.2023.
//

import SwiftUI

struct SliderView: View {
    @EnvironmentObject var rnbo: RNBOAudioUnitHostModel
    @Binding var parameter: RNBOParameter
    var displayName: String

    var body: some View {
        HStack {
            // 1) Parameter-naam
            Text(displayName)
                .frame(width: 80, alignment: .leading)
                .font(.caption)

            // 2) Native UISlider wrap met range 0…1
            NativeSlider(
                value: Binding<Float>(
                    get: { Float(parameter.valueNormalized) },
                    set: {
                        parameter.valueNormalized = Double($0)
                        rnbo.setParameterValueNormalized(to: Double($0),
                                                         at: parameter.info.index)
                    }
                ),
                range: 0...1,    // altijd genormaliseerd 0–1
                onEditingChanged: { _ in }
            )
            .frame(height: 44)   // verhoogt de touchable track-height
            .padding(.vertical, 4)

            // 3) Huidige absolute waarde
            Text(String(format: "%.2f", parameter.value))
                .frame(width: 50)
                .font(.caption)
        }
        .padding(.horizontal)
    }
}

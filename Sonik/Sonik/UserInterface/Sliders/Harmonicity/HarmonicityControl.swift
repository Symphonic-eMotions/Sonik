//
//  HarmonicityControl.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 06/09/2025.
//
import SwiftUI

struct HarmonicityControl: View {
    @EnvironmentObject var rnbo: RNBOAudioUnitHostModel

    private var values: [Double] { rnbo.currentHarmonicityValues }
    private var count: Int { max(1, values.count) }

    // Slider werkt met Double; we mappen naar Int
    private var sliderBinding: Binding<Double> {
        Binding<Double>(
            get: { Double(rnbo.harmonicityIndex) },
            set: { newVal in
                let idx = Int(newVal.rounded())
                rnbo.applyHarmonicity(index: max(0, min(idx, count - 1)))
            }
        )
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            // Label + actuele waarde
            HStack {
                Text("Harmonics")
                    .font(.headline)
                Spacer()
                if let v = rnbo.currentHarmonicityValue {
                    Text(String(format: "× %.6g", v))
                        .monospacedDigit()
                        .foregroundStyle(.secondary)
                }
            }

            // Picker voor schaal
            Picker("Schaal", selection: $rnbo.harmonicityScaleName) {
                ForEach(HarmonicityScale.names, id: \.self) { name in
                    Text(name).tag(name)
                }
            }
            .pickerStyle(.menu)
            .onChange(of: rnbo.harmonicityScaleName) { newName in
                rnbo.changeHarmonicityScale(to: newName)
            }

            // Slider over de indexen van de collectie
            VStack(alignment: .leading) {
                Slider(value: sliderBinding, in: 0...Double(count - 1), step: 1)
                HStack {
                    Text("Index \(rnbo.harmonicityIndex + 1) / \(count)")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                    Spacer()
                }
            }
        }
        .onAppear {
            // Zorg dat param meteen klopt bij binnenkomen
            rnbo.applyHarmonicity()
        }
    }
}

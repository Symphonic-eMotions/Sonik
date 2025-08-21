//
//  ChordPicker.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 22/08/2025.
//


import SwiftUI

struct ChordPicker: View {
    @Binding var spec: ChordSpec
    var currentKey: String
    let useRoman: Bool

    private let romanDegrees = ["I","ii","iii","IV","V","vi","vii°",
                                "bVII","bIII","V/ii","V/V"] // extra’s blijven gewoon beschikbaar
    private let roots = ["C","C#","Db","D","D#","Eb","E","F","F#","Gb","G","G#","Ab","A","A#","Bb","B"]

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            if useRoman {
                Picker("Trap", selection: Binding(
                    get: { spec.degree ?? "I" },
                    set: {
                        spec.degree = $0
                        spec.root = nil
                        // 🔽 diatonische default—zet automatisch de kwaliteit
                        spec.quality = ProgressionTheory.suggestedQuality(forRoman: $0, keyString: currentKey)
                    }
                )) {
                    ForEach(romanDegrees, id: \.self) { Text($0) }
                }
                .pickerStyle(.menu)
            } else {
                Picker("Root", selection: Binding(
                    get: { spec.root ?? "C" },
                    set: {
                        spec.root = $0
                        spec.degree = nil
                        // bij absolute root geen automatische kwaliteitswijziging
                    }
                )) {
                    ForEach(roots, id: \.self) { Text($0) }
                }
                .pickerStyle(.menu)
            }

            Picker("Kwaliteit", selection: Binding(
                get: { spec.quality },
                set: { spec.quality = $0 }
            )) {
                ForEach(ChordQuality.allCases, id: \.self) {
                    Text($0.rawValue)
                }
            }
            .pickerStyle(.menu)
        }
    }
}

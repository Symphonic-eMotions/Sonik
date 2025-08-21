//
//  DualXYPadsView.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 14/08/2025.
//
import SwiftUI

struct DualXYPadsView: View {
    @EnvironmentObject var rnbo: RNBOAudioUnitHostModel

    @State private var presets: [XYPreset] = []
    @State private var selectedPresetIndex: Int = 0

    // per pad-id de gekozen param ids (overrides); default = uit preset
    @State private var selectedXForPad: [String: String] = [:]
    @State private var selectedYForPad: [String: String] = [:]

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {

            // Preset selectie
//            if presets.indices.contains(selectedPresetIndex) {
//                Picker("Preset", selection: $selectedPresetIndex) {
//                    ForEach(presets.indices, id: \.self) { i in
//                        Text(presets[i].name).tag(i)
//                    }
//                }
//                .pickerStyle(.segmented)
//                .onChange(of: selectedPresetIndex) { _ in
//                    applyPresetDefaultsToSelection()
//                    loadSavedOverrides() // laad overrides voor nieuwe preset
//                }
//            }

            // Pads + pickers
            if presets.indices.contains(selectedPresetIndex) {
                let preset = presets[selectedPresetIndex]
                let options = rnbo.visibleParameterConfigs()

                let pads = preset.pads
                ForEach(Array(stride(from: 0, to: pads.count, by: 2)), id: \.self) { i in
                    HStack(alignment: .top, spacing: 12) {
                        padCell(pads[i], preset: preset, options: options)

                        if i + 1 < pads.count {
                            padCell(pads[i + 1], preset: preset, options: options)
                        } else {
                            Spacer(minLength: 0) // vul 2e kolom op bij een oneven aantal
                        }
                    }
                    .padding(.vertical, 6)
                }
            } else {
                Text("Geen presets beschikbaar").foregroundStyle(.secondary)
            }
        }
        .onAppear {
            presets = XYPresetStore.loadFromBundle(orPrintExampleUsing: rnbo)
            applyPresetDefaultsToSelection()
            loadSavedOverrides()
        }
        .padding(.vertical, 6)
    }
    
    @ViewBuilder
    private func padCell(_ pad: XYPadConfig, preset: XYPreset, options: [ParameterConfig]) -> some View {
        VStack(alignment: .leading, spacing: 8) {
            if rnbo.showParameterSelects {
                VStack {
                    HStack {
                        Text("Y")
                        Picker("", selection: Binding<String>(
                            get: { selectedYForPad[pad.id] ?? pad.yParamId },
                            set: { newId in
                                selectedYForPad[pad.id] = newId
                                saveOverrideId(newId, presetId: preset.id, padId: pad.id, axis: "y")
                            }
                        )) {
                            ForEach(options, id: \.id) { opt in
                                Text(opt.displayName).tag(opt.id)
                            }
                        }
                        .pickerStyle(.menu)
                    }

                    HStack {
                        Text("X")
                        Picker("", selection: Binding<String>(
                            get: { selectedXForPad[pad.id] ?? pad.xParamId },
                            set: { newId in
                                selectedXForPad[pad.id] = newId
                                saveOverrideId(newId, presetId: preset.id, padId: pad.id, axis: "x")
                            }
                        )) {
                            ForEach(options, id: \.id) { opt in
                                Text(opt.displayName).tag(opt.id)
                            }
                        }
                        .pickerStyle(.menu)
                    }
                }
            }

            XYPad(config: adjustedConfig(for: pad))
                .environmentObject(rnbo)
        }
        .frame(maxWidth: .infinity) // zorgt dat 2 kolommen netjes de breedte delen
    }


    // MARK: - View helpers (pure, geen ViewBuilder)

    private func adjustedConfig(for pad: XYPadConfig) -> XYPadConfig {
        let options = rnbo.visibleParameterConfigs()
        let desiredX = selectedXForPad[pad.id] ?? pad.xParamId
        let desiredY = selectedYForPad[pad.id] ?? pad.yParamId
        let nx = normalizeId(desiredX, options: options)
        let ny = normalizeId(desiredY, options: options)

        var cfg = pad
        cfg.xParamId = nx
        cfg.yParamId = ny
        return cfg
    }

    // MARK: - Persist (eenvoudig, dynamische keys via UserDefaults)

    private func key(_ presetId: String, _ padId: String, _ axis: String) -> String {
        "xyPad.\(presetId).\(padId).\(axis)" // axis: "x"|"y"
    }

    private func loadSavedOverrides() {
        guard presets.indices.contains(selectedPresetIndex) else { return }
        let preset = presets[selectedPresetIndex]
        let options = rnbo.visibleParameterConfigs()

        for pad in preset.pads {
            if let sx = UserDefaults.standard.string(forKey: key(preset.id, pad.id, "x")) {
                let nx = normalizeId(sx, options: options)
                selectedXForPad[pad.id] = nx
                if nx != sx { saveOverrideId(nx, presetId: preset.id, padId: pad.id, axis: "x") }
            }
            if let sy = UserDefaults.standard.string(forKey: key(preset.id, pad.id, "y")) {
                let ny = normalizeId(sy, options: options)
                selectedYForPad[pad.id] = ny
                if ny != sy { saveOverrideId(ny, presetId: preset.id, padId: pad.id, axis: "y") }
            }
        }
    }

    private func saveOverrideId(_ id: String, presetId: String, padId: String, axis: String) {
        UserDefaults.standard.set(id, forKey: key(presetId, padId, axis))
    }

    private func applyPresetDefaultsToSelection() {
        guard presets.indices.contains(selectedPresetIndex) else { return }
        let preset = presets[selectedPresetIndex]
        let options = rnbo.visibleParameterConfigs()

        for pad in preset.pads {
            // X
            let wantedX = selectedXForPad[pad.id] ?? pad.xParamId
            let nx = normalizeId(wantedX, options: options)
            selectedXForPad[pad.id] = nx

            // Y
            let wantedY = selectedYForPad[pad.id] ?? pad.yParamId
            let ny = normalizeId(wantedY, options: options)
            selectedYForPad[pad.id] = ny
        }
    }
    
    // Kies een id die én in je zichtbare parameterlijst zit én door RNBO bestaat.
    private func normalizeId(_ desired: String, options: [ParameterConfig]) -> String {
        // 1) Als preset-id in zichtbare opties zit → altijd gebruiken
        if options.contains(where: { $0.id == desired }) {
            return desired
        }
        // 2) Zo niet: kies de eerste zichtbare optie
        return options.first?.id ?? desired
    }
}

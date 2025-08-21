//
//  XYPresetStore.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 14/08/2025.
//


enum XYPresetStore {
    static let fileName = "xyPresets" // xyPresets.json in Bundle

    static func loadFromBundle(orPrintExampleUsing rnbo: RNBOAudioUnitHostModel) -> [XYPreset] {
        if let url = Bundle.main.url(forResource: fileName, withExtension: "json") {
            do {
                let data = try Data(contentsOf: url)
                return try JSONDecoder().decode([XYPreset].self, from: data)
            } catch {
                print("⚠️ xyPresets.json parse error: \(error)")
            }
        }
        // Geen bestand → print voorbeeld gebaseerd op jouw parameters
        printExamplePresets(rnbo: rnbo)
        return defaultExamplePresets(rnbo: rnbo)
    }

    private static func defaultExamplePresets(rnbo: RNBOAudioUnitHostModel) -> [XYPreset] {
        // Kies 2 eerste parameters als voorbeeld (veiligheid)
        let p = rnbo.parameters.map(\.id)
        let x = p.first ?? "param_x"
        let y = p.dropFirst().first ?? "param_y"

        return [
            XYPreset(
                id: "basic",
                name: "Basic",
                pads: [
                    XYPadConfig(id: "pad1", xParamId: x, yParamId: y),
                    XYPadConfig(id: "pad2", xParamId: x, yParamId: y, xInvert: true)
                ]
            )
        ]
    }

    private static func printExamplePresets(rnbo: RNBOAudioUnitHostModel) {
        let example = defaultExamplePresets(rnbo: rnbo)
        do {
            let data = try JSONEncoder().encode(example)
            if let pretty = String(data: data, encoding: .utf8) {
                print("""
                === Example xyPresets.json ===
                \(pretty)
                =================================
                """)
            }
        } catch {
            print("⚠️ Kon voorbeeld xyPresets niet encoden: \(error)")
        }
    }
}

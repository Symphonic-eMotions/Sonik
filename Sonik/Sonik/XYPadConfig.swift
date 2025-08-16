//
//  XYPadConfig.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 14/08/2025.
//


struct XYPadConfig: Codable, Identifiable {
    var id: String
    var title: String

    // Koppeling naar RNBO-parameters (paramId uit RNBO description)
    var xParamId: String
    var yParamId: String

    // Normalized mapping binnen 0…1 (optioneel beperken)
    var xMin: Double = 0.0
    var xMax: Double = 1.0
    var yMin: Double = 0.0
    var yMax: Double = 1.0

    // Invert (bijv. Y omhoog = waarde omlaag)
    var xInvert: Bool = false
    var yInvert: Bool = false

    // Startpositie (normalized 0…1); als nil -> wordt gelezen uit huidige parameterwaarden
    var initialX: Double? = nil
    var initialY: Double? = nil
}

struct XYPreset: Codable, Identifiable {
    var id: String
    var name: String
    var pads: [XYPadConfig]
}

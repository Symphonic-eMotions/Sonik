//
//  Progression.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 21/08/2025.
//


import Foundation

struct Progression: Codable {
    var meta: Meta
    var timeline: [ChordEvent]
    struct Meta: Codable {
        var key: String          // "C major" of "A minor"
        var timeSignature: String // "4/4"
        var tempo: Double
        var bars: Int
    }
}

struct ChordEvent: Codable, Identifiable {
    var id: UUID = UUID()
    var bar: Int          // 1-based
    var lengthBars: Int   // duur in maten
    var chord: ChordSpec
}

struct ChordSpec: Codable {
    // EITHER degree (relatief) OF root (absoluut). Beide mag ook; degree heeft voorrang.
    var degree: String?    // "I", "ii", "bVII", "V/ii", etc.
    var root: String?      // "C", "C#", "Db", ...
    var quality: ChordQuality
    var extensions: [Int]?     // optioneel: [9, 11, 13]
    var alterations: [String]? // optioneel: ["b9", "#11"]
}

enum ChordQuality: String, Codable, CaseIterable {
    case maj, m, dim, aug
    case seven = "7", maj7, m7, m7b5, dim7
    case sus2, sus4
}

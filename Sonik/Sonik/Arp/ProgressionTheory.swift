//
//  ProgressionTheory.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 22/08/2025.
//


import Foundation

enum KeyMode: String {
    case major, minor
}

struct ProgressionTheory {

    /// Parse "C major" / "A minor" (niet hoofdlettergevoelig)
    static func parseKey(_ keyString: String) -> (root: String, mode: KeyMode)? {
        // Losjes parsen: "C major", "c Major", "A minor", "Am", "Cmaj", "C min", etc.
        let s = keyString.trimmingCharacters(in: .whitespacesAndNewlines).lowercased()

        // snelle paden voor notatie als "Am" / "Cmaj" / "Cmin"
        if s.count <= 4 {
            // bv "am" -> root "a", minor
            if s.hasSuffix("m") { return (String(s.dropLast()).uppercased(), .minor) }
            if s.hasSuffix("min") { return (String(s.dropLast(3)).uppercased(), .minor) }
            if s.hasSuffix("maj") { return (String(s.dropLast(3)).uppercased(), .major) }
        }

        // "c major" / "a minor"
        if s.contains("major") {
            let root = s.replacingOccurrences(of: "major", with: "").trimmingCharacters(in: .whitespaces)
            return (root.uppercased(), .major)
        } else if s.contains("minor") {
            let root = s.replacingOccurrences(of: "minor", with: "").trimmingCharacters(in: .whitespaces)
            return (root.uppercased(), .minor)
        }

        // fallback: alleen een letter? neem major
        if s.range(of: #"^[a-g][b#]?$"#, options: .regularExpression) != nil {
            return (s.uppercased(), .major)
        }
        return nil
    }

    /// Standaard akkoordkwaliteit per graad in MAJOR
    /// I maj, ii m, iii m, IV maj, V maj(7), vi m, vii° dim (of m7b5)
    static func defaultQualityMajor(forRoman degree: String) -> ChordQuality {
        switch degree {
        case "I": return .maj
        case "ii": return .m
        case "iii": return .m
        case "IV": return .maj
        case "V": return .maj      // laat evt. .seven als je automatisch V7 wil
        case "vi": return .m
        case "vii°": return .dim
        default: return .maj // neutrale fallback
        }
    }

    /// Standaard akkoordkwaliteit per graad in MINOR (natuurlijk mineur, aanpasbaar)
    /// i m, ii° dim, III maj, iv m, v m (of V maj), VI maj, VII maj
    static func defaultQualityMinor(forRoman degree: String) -> ChordQuality {
        switch degree {
        case "i": return .m
        case "ii°": return .dim
        case "III": return .maj
        case "iv": return .m
        case "v": return .m        // kies .maj als je harmonisch mineur (V) wil
        case "VI": return .maj
        case "VII": return .maj
        default: return .m
        }
    }

    /// Kies default quality o.b.v. key (major/minor) en graad.
    static func suggestedQuality(forRoman degree: String, keyString: String) -> ChordQuality {
        let mode = parseKey(keyString)?.mode ?? .major
        switch mode {
        case .major: return defaultQualityMajor(forRoman: degree)
        case .minor: return defaultQualityMinor(forRoman: degree)
        }
    }
}

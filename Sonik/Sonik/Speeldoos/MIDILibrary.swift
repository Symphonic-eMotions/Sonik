//
//  MIDILibrary.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 05/09/2025.
//

struct MIDIItem: Identifiable, Hashable {
    let id = UUID()
    let fileBase: String   // zonder extensie
    let display: String    // korte naam voor in de UI
}

enum MIDILibrary {
    static let subdir = "midiFiles"

    static func scanBundle() -> [MIDIItem] {
        // 1) Probeer subdir
        var urls = Bundle.main.urls(forResourcesWithExtension: "mid", subdirectory: subdir) ?? []
        
        // 2) Fallback: zoek in bundleroot als subdir leeg is (bv. als map geen Folder Reference is)
        if urls.isEmpty {
            urls = Bundle.main.urls(forResourcesWithExtension: "mid", subdirectory: nil) ?? []
        }
        
        let items = urls.sorted { $0.lastPathComponent < $1.lastPathComponent }.map { url -> MIDIItem in
            let base = url.deletingPathExtension().lastPathComponent
            return MIDIItem(fileBase: base, display: prettyName(from: base))
        }
        
        if items.isEmpty {
            print("⚠️ Geen MIDI's gevonden. Controleer Target Membership en of de map een 'Folder Reference' (blauwe map) is.")
        } else {
            print("✅ Gevonden MIDI's: \(items.map{$0.fileBase})")
        }
        
        return items
    }

    /// Maak van bv. "beethoven-ode-to-joy" -> "Ode to Joy"
    /// "traditional-greensleeves2" -> "Greensleeves (2)"
    private static func prettyName(from base: String) -> String {
        let parts = base.split(separator: "-").map(String.init)
        guard parts.count >= 2 else {
            return base.replacingOccurrences(of: "-", with: " ").capitalized
        }

        // Heuristiek: eerste deel is componist/bron, rest is titel
        var title = parts.dropFirst().joined(separator: " ")
        // "something2" -> "something (2)"
        if let last = title.split(separator: " ").last,
           let n = Int(last), n > 1 {
            title = title.dropLast(last.count).trimmingCharacters(in: .whitespaces)
            title += " (\(n))"
        }
        // Mooie kapitalisatie + streepjes weg
        title = title.replacingOccurrences(of: "-", with: " ").trimmingCharacters(in: .whitespaces)
        // Houd bekende korte woordjes in lowercased waar passend
        let keepLower = Set(["of","the","a","to","and","for","in","on"])
        let words = title.split(separator: " ")
        let nice = words.enumerated().map { idx, w -> String in
            let s = String(w)
            return (idx > 0 && keepLower.contains(s.lowercased())) ? s.lowercased() : s.capitalized
        }.joined(separator: " ")
        return nice
    }
}

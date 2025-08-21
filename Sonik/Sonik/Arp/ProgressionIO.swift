//
//  ProgressionIO.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 22/08/2025.
//


import Foundation

enum ProgressionIOError: Error {
    case resourceNotFound(String)
}

struct ProgressionIO {

    /// Laad een Progression uit de app-bundle.
    /// - Parameters:
    ///   - file: bestandsnaam zonder extensie (bv. "chordSchema")
    ///   - subdirectory: optionele map in de bundle (bv. "Arp")
    static func loadFromBundle(file: String,
                               subdirectory: String? = nil) throws -> Progression {
        guard let url = Bundle.main.url(forResource: file,
                                        withExtension: "json",
                                        subdirectory: subdirectory) else {
            throw ProgressionIOError.resourceNotFound("\(subdirectory.map { "\($0)/" } ?? "")\(file).json")
        }
        let data = try Data(contentsOf: url)
        return try JSONDecoder().decode(Progression.self, from: data)
    }

    /// Schrijf Progression naar Documents/progression.json en print mooie JSON in de console.
    @discardableResult
    static func saveToDocumentsAndPrint(_ p: Progression,
                                        filename: String = "progression.json") throws -> URL {
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
        let data = try encoder.encode(p)

        let url = FileManager.default
            .urls(for: .documentDirectory, in: .userDomainMask)[0]
            .appendingPathComponent(filename)

        try data.write(to: url, options: .atomic)

        if let jsonString = String(data: data, encoding: .utf8) {
            print("✅ Saved to \(url.path)")
            print("📄 Progression JSON:\n\(jsonString)")
        }
        return url
    }
}

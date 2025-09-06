//
//  HarmonicityScale.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 06/09/2025.
//


enum HarmonicityScale {
    static let names: [String] = [
        "Equal (12-TET)",
        "Just (5-limit)",
        "Pythagorean",
        "Meantone (1/4 comma)",
        "Harmonic Series",
        "Hungarian Gypsy",
        "Equal limit",
        "Just limit",
        "Pythagorean limit",
        "Meantone limit",
        "Harmonic limit",
        "Hungarian limit"
    ]

    /// De ruwe blokken 1..12; exact zoals jij ze aanleverde.
    /// Tip: laat dit desnoods in een .txt / .json resource staan en laad het.
    static let rawCollections: String = """
1, 1. 1.059463 1.122462 1.189207 1.259921 1.33484 1.414214 1.498307 1.587401 1.681793 1.781797 1.887749 2. 2.118926 2.244924 2.378414 2.519842 2.66968 2.828427 2.996615 3.174802 3.363586 3.563595 3.775498 4. 4.237853 4.489849 4.756828 5.039684 5.339373 5.656854 5.993165 6.349604 6.727171 7.127189 7.550505 8. 8.470851 8.9757 9.414213;
2, 1. 1.066667 1.125 1.2 1.25 1.333333 1.40625 1.5 1.6 1.666667 1.8 1.875 2. 2.133334 2.25 2.4 2.5 2.666666 2.8125 3. 3.2 3.333334 3.6 3.75 4. 4.266668 4.5 4.8 5. 5.333332 5.625 6. 6.4 6.666668 7.2 7.5 8. 8.533336 9. 9.6 10.;
3, 1. 1.053497 1.125 1.185185 1.265625 1.333333 1.423828 1.5 1.580246 1.6875 1.777778 1.898438 2. 2.106994 2.25 2.37037 2.53125 2.666666 2.847656 3. 3.160492 3.375 3.555556 3.796876 4. 4.213988 4.5 4.74074 5.0625 5.333332 5.695312 6. 6.320984 6.75 7.111112 7.593752 8. 8.427976 9. 9.48148 10.125;
4, 1. 1.07177 1.11803 1.19628 1.25 1.33748 1.39754 1.49535 1.58073 1.67185 1.78886 1.86919 2. 2.14354 2.23606 2.39256 2.5 2.67496 2.79508 2.9907 3.16146 3.37772 3.73838 3.86482 4. 4.28708 4.47212 4.78512 5. 5.34992 5.59016 5.9814 6.32292 6.75544 7.47676 7.72964 8. 8.57416 8.94424 9.57024 10.;
5, 1. 1.08333 1.25 1.33333 1.41667 1.5 1.58333 1.66667 1.75 1.83333 1.91667 2. 2.08333 2.16667 2.25 2.33333 2.41667 2.5 2.58333 2.66667 2.75 2.83333 2.91667 3. 3.08333 3.16667 3.25 3.33333 3.41667 3.5 3.58333 3.66667 3.75 3.83333 3.91667 4. 4.08333 4.16667 4.25 4.33333 4.41667 4.5 4.58333 4.66667 4.75 4.83333 4.91667 5. 5.08333 5.16667 5.25 5.33333 5.41667 5.5 5.58333 5.66667 5.75 5.83333 5.91667 6. 6.08333 6.16667 6.25 6.33333 6.41667 6.5 6.58333 6.66667 6.75 6.83333 6.91667 7. 7.08333 7.16667 7.25 7.33333 7.41667 7.5 7.58333 7.66667 7.75 7.83333 7.91667 8. 8.08333 8.16667 8.25 8.33333 8.41667 8.5 8.58333 8.66667 8.75 8.83333 8.91667 9. 9.08333 9.16667 9.25 9.33333 9.41667 9.5 9.58333 9.66667 9.75 9.83333 9.91667 10.;
6, 1. 1.05946 1.18921 1.33484 1.49831 1.5874 1.7818 1.88775 2. 2.24492 2.37841 2.51984 2.66968 2.83848 3.07473 3.2457 3.5636 3.7755 3.99998 4.48984 4.75682 5.03968 5.33936 5.67696 6.14946 6.4914 7.1272 7.5509 7.99996 8.97968 9.51364 10.;
7, 1 1.259921 1.498307 1.887749 2.519842 5.039684 10.079368;
8, 1 1.25 1.5 2 3 4.5 9;
9, 1 1.265625 1.423828 1.777778 2.847656 4 8.427976;
10, 1 1.11803 1.25 1.39754 2 3.16146 8.94424;
11, 1 1.25 1.5 2 3 5 10;
12, 1 1.33484 1.591 2.004 3.254 4.48 8.97;
"""

    static func parse() -> [String: [Double]] {
        var result: [String: [Double]] = [:]

        // Split op regels eindigend met ';'
        let blocks = rawCollections
            .replacingOccurrences(of: "\r", with: "\n")
            .components(separatedBy: ";")
            .map { $0.trimmingCharacters(in: .whitespacesAndNewlines) }
            .filter { !$0.isEmpty }

        for block in blocks {
            // scheid "N, values..."
            guard let commaIdx = block.firstIndex(of: ",") else { continue }
            let indexStr = block[..<commaIdx].trimmingCharacters(in: .whitespaces)
            let valuesStr = block[block.index(after: commaIdx)...].trimmingCharacters(in: .whitespaces)

            guard let idx = Int(indexStr), idx >= 1, idx <= names.count else { continue }
            let name = names[idx - 1]

            // normaliseer: vervang meerdere spaties door één, verwijder komma’s
            let cleaned = valuesStr
                .replacingOccurrences(of: ",", with: " ")
                .replacingOccurrences(of: "  ", with: " ")
                .replacingOccurrences(of: "  ", with: " ") // dubbel nog eens
                .trimmingCharacters(in: .whitespacesAndNewlines)

            // tokens → Doubles (sta '1.' toe → '1')
            let tokens = cleaned.split(whereSeparator: { $0.isWhitespace })
            var values = [Double]()
            values.reserveCapacity(tokens.count)

            for t in tokens {
                var s = String(t)
                // Verwijder trailing puntjes zoals "2."
                while s.last == "." { s.removeLast() }
                if let v = Double(s) {
                    values.append(v)
                }
            }
            result[name] = values
        }
        return result
    }
}

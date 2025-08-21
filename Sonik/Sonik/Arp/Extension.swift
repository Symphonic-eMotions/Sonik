//
//  Extension.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 22/08/2025.
//


extension Array where Element == ChordEvent {
    func normalized(maxBars: Int) -> [ChordEvent] {
        let sorted = self.sorted { $0.bar < $1.bar }
        var out: [ChordEvent] = []
        var lastEnd = 0
        for var ev in sorted {
            ev.bar = Swift.max(1, Swift.min(ev.bar, maxBars))
            ev.lengthBars = Swift.max(1, ev.lengthBars)
            if ev.bar <= lastEnd { ev.bar = lastEnd + 1 }
            lastEnd = Swift.min(maxBars, ev.bar + ev.lengthBars - 1)
            out.append(ev)
            if lastEnd >= maxBars { break }
        }
        return out
    }
}

//
//  AudioKitMIDIManager.swift
//  SwiftRNBO_Example_multiplatfrom_SwiftUI
//
//  Created by Frans-Jan Wind on 17/07/2025.
//


import AudioKit
import SwiftUI

class AudioKitMIDIManager: ObservableObject {
    static let shared = AudioKitMIDIManager()
    
    let midi = MIDI()
    
    private init() {
        midi.openOutput()
        midi.openInput()
    }
}

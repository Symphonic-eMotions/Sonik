//
//  SwiftRNBO_Example_multiplatfrom_SwiftUIApp.swift
//  SwiftRNBO_Example_multiplatfrom_SwiftUI
//
//  Created by Эльдар Садыков on 19.02.2023.
//

import SwiftUI

@main
struct Sonik: App {
    @ObservedObject var rnbo = RNBOAudioUnitHostModel()
    @StateObject  var sequencer: MIDISequencer

    init() {
        let hostModel = RNBOAudioUnitHostModel()
        _rnbo = ObservedObject(wrappedValue: hostModel)
        _sequencer = StateObject(wrappedValue: MIDISequencer(rnbo: hostModel))
    }

    var body: some Scene {
        WindowGroup {
            ContentView()
                .onAppear { rnbo.connectEventHandler() }
                .environmentObject(rnbo)
                .environmentObject(sequencer)
        }
    }
}

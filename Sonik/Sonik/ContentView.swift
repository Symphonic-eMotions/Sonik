//
//  ContentView.swift
//  SwiftRNBO_Example_multiplatfrom_SwiftUI
//
//  Created by Эльдар Садыков on 19.02.2023.
//

import SwiftUI

struct ContentView: View {
    @EnvironmentObject var rnbo: RNBOAudioUnitHostModel
    @EnvironmentObject var sequencer: MIDISequencer

    var body: some View {
        VStack(spacing: 10) {
            if rnbo.showDescription {
                DescriptionView()
            } else {
                DualXYPadsView()
//                Sliders()
                AudioKitKeyboard()
                
                HStack {
                    
                    OctaafStepperView()
                        .frame(width: 150, alignment: .leading)
                    
                    VStack {
                        
                        HStack(spacing: 15) {
                            Button {
                                rnbo.currentOctave = 0
                                sequencer.loadMIDIFile(named: "midiMelody2")
                            } label: {
                                Label("", systemImage: "folder.fill")
                            }
                            
                            Button {
                                rnbo.sendAllNotesOff()
                                sequencer.generateArpeggioSequence(
                                    chordNotes: [60, 64, 67],
                                    pattern: [0, 1, 2, 1],
                                    octaveRange: 2,
                                    repeats: 4
                                )
                            } label: {
                                Label("", systemImage: "music.note.list")
                            }
                            
                            Button {
                                sequencer.clearAllTracks()
                            } label: {
                                Label("", systemImage: "trash.fill")
                            }
                        }
                        .buttonStyle(.borderedProminent)
                        
                        HStack(spacing: 15) {
                            Button {
                                sequencer.play()
                            } label: {
                                Label("Play", systemImage: "play.circle.fill")
                            }
                            
                            Button {
                                sequencer.stop()
                            } label: {
                                Label("Stop", systemImage: "stop.circle.fill")
                            }
                        }
                        .buttonStyle(.bordered)
                    }
                }
            }
        }
        .padding()
    }
}

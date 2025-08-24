//
//  ContentView.swift
//  SwiftRNBO_Example_multiplatfrom_SwiftUI
//
//  Created by Эльдар Садыков on 19.02.2023.
//

import SwiftUI

struct ContentView: View {
    @EnvironmentObject var rnbo: RNBOAudioUnitHostModel

    var body: some View {
        VStack(spacing: 10) {
            if rnbo.showInterface == .description {
                DescriptionView()
            } else if rnbo.showInterface == .arp {
                ProgressionEditorView()
            } else {
                if [.xy, .xyEdit].contains(rnbo.showInterface) {
                    DualXYPadsView()
                } else if rnbo.showInterface == .slider {
                    Sliders()
                }
                AudioKitKeyboard()
                
                HStack {
                    
                    OctaafStepperView()
                        .frame(width: 150, alignment: .leading)
                    
                    VStack {
                        
                        HStack(spacing: 15) {
                            Button {
                                rnbo.currentOctave = 0
                                rnbo.midiSequencer.loadMIDIFile(named: "midiMelody2")
                            } label: {
                                Label("", systemImage: "folder.fill")
                            }
                            
                            Button {
                                rnbo.sendAllNotesOff()
                                rnbo.showInterface = .arp
                            } label: {
                                Label("", systemImage: "music.note.list")
                            }
                            
                            Button {
                                rnbo.midiSequencer.clearAllTracks()
                            } label: {
                                Label("", systemImage: "trash.fill")
                            }
                        }
                        .buttonStyle(.borderedProminent)
                        
                        HStack(spacing: 15) {
                            Button {
                                rnbo.midiSequencer.play()
                            } label: {
                                Label("Play", systemImage: "play.circle.fill")
                            }
                            
                            Button {
                                rnbo.midiSequencer.stop()
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

//
//  ContentView.swift
//  SwiftRNBO_Example_multiplatfrom_SwiftUI
//
//  Created by Эльдар Садыков on 19.02.2023.
//

import SwiftUI

struct ContentView: View {
    @EnvironmentObject var rnbo: RNBOAudioUnitHostModel
    @State private var midiItems: [MIDIItem] = []
    @State private var selectedMIDI: MIDIItem?

    var body: some View {
        VStack(spacing: 10) {
            if rnbo.showInterface == .description {
                DescriptionView()
            } else if rnbo.showInterface == .arp {
                ArpAppView()
            } else {
                
                HStack(spacing: 12) {
                    Picker("Muziekdoosje", selection: $selectedMIDI) {
                        ForEach(midiItems) { item in
                            Text(item.display).tag(Optional(item))
                        }
                    }
                    .pickerStyle(.menu)

                    if let sel = selectedMIDI {
                        Button {
                            rnbo.midiSequencer.loadMIDIFile(named: sel.fileBase, subdirectory: MIDILibrary.subdir)
                        } label: {
                            Label("Herlaad", systemImage: "arrow.clockwise")
                        }
                        .buttonStyle(.bordered)
                    }
                }
                .onChange(of: selectedMIDI) { newValue in
                    guard let item = newValue else { return }
                    rnbo.midiSequencer.loadMIDIFile(named: item.fileBase, subdirectory: MIDILibrary.subdir)
                }
                
                
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
//                            Button {
//                                rnbo.currentOctave = 0
//                                rnbo.midiSequencer.loadMIDIFile(named: "midiMelody2")
//                            } label: {
//                                Label("", systemImage: "folder.fill")
//                            }
                            
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
        .onAppear {
            let items = MIDILibrary.scanBundle()
            midiItems = items
            if selectedMIDI == nil, let first = items.first {
                selectedMIDI = first
                rnbo.midiSequencer.loadMIDIFile(named: first.fileBase, subdirectory: MIDILibrary.subdir)
            }
        }

    }
}

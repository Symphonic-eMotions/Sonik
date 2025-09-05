//
//  ArpEditorView.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 28/08/2025.
//
import SwiftUI

struct ArpEditorView: View {
    @EnvironmentObject var rnbo: RNBOAudioUnitHostModel
    
    @State private var patternText: String = "0,1,2,1"
    @State private var grid: Double = 0.25
    @State private var octaveRange: Int = 1
    @State private var velocity: Double = 100
    @State private var noteLengthFactor: Double = 0.9
    @State private var swapAtLoop: Bool = true
    
    var body: some View {
        Form {
            Section("Patroon") {
                TextField("Index-patroon (bijv. 0,1,2,1)", text: $patternText)
            }
            
            Section("Tijd & lengte") {
                Stepper("Grid: \(grid, specifier: "%.2f") beats", value: $grid, in: 0.125...1, step: 0.125)
                Slider(value: $noteLengthFactor, in: 0.5...1.0) {
                    Text("Gate")
                }
                Text("Gate factor: \(noteLengthFactor, specifier: "%.2f")")
            }
            
            Section("Noten") {
                Stepper("Octaafbereik: \(octaveRange)", value: $octaveRange, in: 1...4)
                Slider(value: $velocity, in: 30...127, step: 1) {
                    Text("Velocity")
                }
                Text("Velocity: \(Int(velocity))")
            }
            
            Section("Swap") {
                Toggle("Hot-swap op loopgrens", isOn: $swapAtLoop)
            }
            
            Section {
                Button("Genereer ARP") {
                    let pattern = patternText
                        .split(separator: ",")
                        .compactMap { Int($0.trimmingCharacters(in: .whitespaces)) }
                    
                    rnbo.loadProgressionIntoSequencerAsArpeggio(
                        baseOctave: 4,
                        velocity: UInt8(velocity),
                        channel: 0,
                        pattern: pattern,
                        octaveRange: octaveRange,
                        gridResolutionBeats: grid,
                        noteLengthFactor: noteLengthFactor,
                        swapAtLoopBoundary: swapAtLoop
                    )
                }
                
                HStack {
                    Button("Play") { rnbo.playSequencer() }
                    Button("Stop") { rnbo.stopSequencer() }
                }
            }
        }
        .navigationTitle("Arpeggiator")
    }
}

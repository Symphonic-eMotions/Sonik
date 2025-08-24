//
//  ProgressionEditorView.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 22/08/2025.
//

import SwiftUI

struct ProgressionEditorView: View {
    
    @EnvironmentObject var rnbo: RNBOAudioUnitHostModel

    var body: some View {
        NavigationView {
            Form {
                Section("Meta") {
                    TextField("Key", text: $rnbo.progression.meta.key)
                    TextField("T.S.", text: $rnbo.progression.meta.timeSignature)
                    HStack {
                        Text("Tempo"); Spacer()
                        Text("\(Int(rnbo.progression.meta.tempo)) BPM")
                    }
                    Slider(value: $rnbo.progression.meta.tempo, in: 40...220, step: 1)
                    Stepper("Aantal maten: \(rnbo.progression.meta.bars)", value: $rnbo.progression.meta.bars, in: 1...256)
                    Toggle("Gebruik trapnotatie (I, ii, V…)", isOn: $rnbo.useRomanGlobal)
                }
                .onChange(of: rnbo.progression.meta.key) { newKey in
                    for i in rnbo.progression.timeline.indices {
                        if let deg = rnbo.progression.timeline[i].chord.degree {
                            rnbo.progression.timeline[i].chord.quality =
                                ProgressionTheory.suggestedQuality(forRoman: deg, keyString: newKey)
                        }
                    }
                }

                Section("Timeline") {
                    ForEach($rnbo.progression.timeline) { $ev in
                        VStack(alignment: .leading, spacing: 8) {
                            VStack(alignment: .leading, spacing: 4) {
                                Text("Start maat").font(.caption).foregroundColor(.secondary)
                                Stepper(value: $ev.bar, in: 1...rnbo.progression.meta.bars) {
                                    Text("\(ev.bar)")
                                }
                            }
                            VStack(alignment: .leading, spacing: 4) {
                                Text("Duur").font(.caption).foregroundColor(.secondary)
                                Stepper(value: $ev.lengthBars, in: 1...rnbo.progression.meta.bars) {
                                    Text("\(ev.lengthBars)")
                                }
                            }
                            ChordPicker(
                                spec: $ev.chord,
                                currentKey: rnbo.progression.meta.key,
                                useRoman: rnbo.useRomanGlobal
                            )
                        }
                        .padding(.vertical, 4)
                    }
                    .onMove { indices, newOffset in
                        rnbo.progression.timeline.move(fromOffsets: indices, toOffset: newOffset)
                    }

                    Button {
                        let lastEnd = rnbo.progression.timeline.map { $0.bar + $0.lengthBars }.max() ?? 1
                        let lastLen = rnbo.progression.timeline.max(by: { $0.bar < $1.bar })?.lengthBars ?? 1

                        let newSpec: ChordSpec = rnbo.useRomanGlobal
                            ? .init(degree: "I",
                                    quality: ProgressionTheory.suggestedQuality(forRoman: "I", keyString: rnbo.progression.meta.key))
                            : .init(degree: nil, root: "C", quality: .maj)

                        rnbo.progression.timeline.append(ChordEvent(
                            bar: min(lastEnd, rnbo.progression.meta.bars),
                            lengthBars: lastLen,  // neem laatste lengte over
                            chord: newSpec
                        ))
                    } label: {
                        Label("Akkoord toevoegen", systemImage: "plus.circle")
                    }
                }

                Section {
                    Button {
                        rnbo.loadProgressionIntoSequencerAsChords(baseOctave: 4, velocity: 100)
                    } label: {
                        Label("Load → Sequencer", systemImage: "tray.and.arrow.down.fill")
                    }
                    
                    Button {
                        rnbo.playSequencer()
                    } label: {
                        Label("Play (sequencer)", systemImage: "play.fill")
                    }
                    
                    Button {
                        rnbo.stopSequencer()
                    } label: {
                        Label("Stop", systemImage: "stop.fill")
                    }
                    
                    Button("Normaliseer (opruimen overlaps)") {
                        rnbo.progression.timeline = rnbo.progression.timeline.normalized(maxBars: rnbo.progression.meta.bars)
                    }
                    Button("Exporteer JSON") {
                        do { _ = try ProgressionIO.saveToDocumentsAndPrint(rnbo.progression) }
                        catch { print("❌ Export failed:", error) }
                    }
                    Button("Sluiten") {
                        rnbo.showInterface = .xy
                    }
                }
            }
            .navigationTitle("Progressie‑editor")
            .toolbar { EditButton() }
            .onAppear {
                rnbo.ensureProgressionLoadedFromBundle()
            }
        }
    }
}

//
//  ArpAppView.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 28/08/2025.
//
import SwiftUI

struct ArpAppView: View {
    @EnvironmentObject var rnbo: RNBOAudioUnitHostModel
    @State private var selectedTab = 0

    var body: some View {
        TabView(selection: $selectedTab) {
            
            ProgressionEditorView()
                .tabItem {
                    Label("Akkoorden", systemImage: "pianokeys")
                }
                .tag(0)
            
            ArpEditorView()
                .tabItem {
                    Label("Arpeggiator", systemImage: "sparkles")
                }
                .tag(1)
        }
    }
}

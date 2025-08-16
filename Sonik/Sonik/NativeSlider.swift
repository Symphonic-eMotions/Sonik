//
//  NativeSlider.swift
//  SwiftRNBO_Example_multiplatfrom_SwiftUI
//
//  Created by Frans-Jan Wind on 22/07/2025.
//


import SwiftUI

struct NativeSlider: UIViewRepresentable {
    @Binding var value: Float
    let range: ClosedRange<Float>
    var onEditingChanged: (Float) -> Void = { _ in }

    func makeUIView(context: Context) -> UISlider {
        let slider = UISlider()
        slider.minimumValue = range.lowerBound
        slider.maximumValue = range.upperBound
        slider.thumbTintColor = .systemBlue
        slider.minimumTrackTintColor = .systemBlue
        slider.maximumTrackTintColor = .secondarySystemFill

        slider.addTarget(context.coordinator,
                         action: #selector(Coordinator.valueChanged(_:)),
                         for: .valueChanged)
        return slider
    }

    func updateUIView(_ uiView: UISlider, context: Context) {
        // Zorg dat slider altijd synchroon blijft met je binding
        uiView.value = value
    }

    func makeCoordinator() -> Coordinator { Coordinator(self) }

    class Coordinator {
        let parent: NativeSlider
        init(_ parent: NativeSlider) { self.parent = parent }

        @objc
        func valueChanged(_ sender: UISlider) {
            parent.value = sender.value
            parent.onEditingChanged(sender.value)
        }
    }
}

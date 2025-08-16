//
//  XYPad.swift
//  Sonik
//
//  Created by Frans-Jan Wind on 14/08/2025.
//
import SwiftUI

struct XYPad: View {
    @EnvironmentObject var rnbo: RNBOAudioUnitHostModel
    let config: XYPadConfig

    @State private var nx: Double = 0.5 // normalized 0…1
    @State private var ny: Double = 0.5 // normalized 0…1
    @State private var didInitializeFromParams = false

    var body: some View {
        VStack(spacing: 6) {
            GeometryReader { geo in
                ZStack {
                    // Achtergrond
                    RoundedRectangle(cornerRadius: 10)
                        .fill(.ultraThinMaterial)

                    // Kruisdraden
                    Path { p in
                        let w = geo.size.width, h = geo.size.height
                        p.move(to: CGPoint(x: w * nx, y: 0))
                        p.addLine(to: CGPoint(x: w * nx, y: h))
                        p.move(to: CGPoint(x: 0, y: h * (1 - ny)))
                        p.addLine(to: CGPoint(x: w, y: h * (1 - ny)))
                    }
                    .stroke(.secondary.opacity(0.5), lineWidth: 1)

                    // Handle
                    Circle()
                        .frame(width: 18, height: 18)
                        .position(x: geo.size.width * nx,
                                  y: geo.size.height * (1 - ny))
                        .shadow(radius: 2)
                }
                .contentShape(RoundedRectangle(cornerRadius: 10))
                .gesture(
                    DragGesture(minimumDistance: 0)
                        .onChanged { value in
                            let w = max(1, geo.size.width)
                            let h = max(1, geo.size.height)
                            nx = (value.location.x / w).clamped(to: 0...1)
                            ny = (1 - (value.location.y / h)).clamped(to: 0...1)
                            sendToRNBO()
                        }
                )
                .onAppear {
                    // init state uit config of actuele RNBO-parameterwaarden
                    if !didInitializeFromParams {
                        nx = config.initialX ?? rnbo.getParameterValueNormalized(byId: config.xParamId) ?? 0.5
                        ny = config.initialY ?? rnbo.getParameterValueNormalized(byId: config.yParamId) ?? 0.5
                        nx = nx.clamped(to: 0...1); ny = ny.clamped(to: 0...1)
                        sendToRNBO()
                        didInitializeFromParams = true
                    }
                }
            }
            .aspectRatio(1, contentMode: .fit)
        }
        .padding(6)
    }

    private func sendToRNBO() {
        // Breng NX/NY naar gewenste subrange + invert (alles normalized)
        let xNorm = map(nx, min: config.xMin, max: config.xMax, invert: config.xInvert)
        let yNorm = map(ny, min: config.yMin, max: config.yMax, invert: config.yInvert)
        _ = rnbo.setParameterValueNormalized(byId: config.xParamId, to: xNorm)
        _ = rnbo.setParameterValueNormalized(byId: config.yParamId, to: yNorm)
    }

    private func map(_ v: Double, min: Double, max: Double, invert: Bool) -> Double {
        let t = invert ? (1 - v) : v
        return (min + (max - min) * t).clamped(to: 0...1)
    }
}

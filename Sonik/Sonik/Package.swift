// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "RNBOKit",
    platforms: [
        .iOS(.v15), .macOS(.v12)
    ],
    products: [
        .library(name: "RNBOKit", targets: ["RNBOKit"]),
    ],
    dependencies: [
        .package(url: "https://github.com/AudioKit/AudioKit.git", from: "5.6.5"),
    ],
    targets: [
        .target(
            name: "RNBOKit",
            dependencies: [
                .product(name: "AudioKit", package: "AudioKit"),
            ],
            path: "RNBOKit",
            sources: ["Sources"],
            resources: [
                .copy("Resources")
            ],
            publicHeadersPath: ".",

            cSettings: [
                .headerSearchPath("Sources"),
                .headerSearchPath("Sources/Public"),
                .headerSearchPath("Sources/CPP"),
                .headerSearchPath("Sources/CPP/Export"),
                .headerSearchPath("Sources/CPP/rnbo/src"),
                .headerSearchPath("Sources/CPP/rnbo/common"),
            ],
            cxxSettings: [
                .headerSearchPath("Sources"),
                .headerSearchPath("Sources/Public"),
                .headerSearchPath("Sources/CPP"),
                .headerSearchPath("Sources/CPP/Export"),
                .headerSearchPath("Sources/CPP/rnbo/src"),
                .headerSearchPath("Sources/CPP/rnbo/common"),
            ],
            linkerSettings: [
                .linkedFramework("Accelerate"),
                .linkedFramework("AudioToolbox"),
                .linkedFramework("AVFoundation"),
                .linkedFramework("CoreAudio"),
                .linkedFramework("CoreMIDI"),
            ]
        ),
        .testTarget(
            name: "RNBOKitTests",
            dependencies: ["RNBOKit"],
            path: "Tests/RNBOKitTests"
        )
    ]
)

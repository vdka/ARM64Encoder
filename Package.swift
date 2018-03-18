// swift-tools-version:4.0

import PackageDescription

let package = Package(
    name: "ARM64Encoder",
    products: [
        .library(name: "ARM64Encoder", targets: ["ARM64Encoder"]),
    ],
    targets: [
        .target(name: "ARM64Encoder", dependencies: []),
    ]
)

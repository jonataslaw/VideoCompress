// swift-tools-version: 5.9

import PackageDescription

let package = Package(
    name: "video_compress_advanced",
    platforms: [
        .iOS("13.0")
    ],
    products: [
        .library(name: "video-compress-advanced", targets: ["video_compress_advanced"])
    ],
    dependencies: [
        .package(name: "FlutterFramework", path: "../FlutterFramework")
    ],
    targets: [
        .target(
            name: "video_compress_advanced",
            dependencies: [
                .product(name: "FlutterFramework", package: "FlutterFramework")
            ],
            resources: [
                // Uncomment if PrivacyInfo.xcprivacy exists:
                // .process("PrivacyInfo.xcprivacy"),
            ]
        )
    ]
)

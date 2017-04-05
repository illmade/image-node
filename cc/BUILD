# Description:
#   TensorFlow C++ example
licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

cc_binary(
    name = "libmulticlassifier.so",
    linkshared = 1,
    srcs = ["GraphLoader.cpp", "GraphLoader.hpp",
            "Multibox.cpp", "Multibox.hpp",
            "FaceAlign.cpp", "FaceAlign.hpp",
            "Classifier.cpp", "Classifier.hpp",
            "MultiClassify.cpp", "MultiClassify.hpp",
            "ImageGraph.cpp", "ImageGraph.hpp",
            "VerifyApp.cpp",
            "timer.cpp", "timer.hpp", "log.h"],
    deps = [
        "//tensorflow/cc:cc_ops",
        "//tensorflow/cc:client_session",
        "//tensorflow/core:tensorflow",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "all_files",
    srcs = glob(
        ["**/*"],
        exclude = [
        "**/METADATA",
        "**/OWNERS",
        "bin/**",
        "gen/**",
    ],
    ),
    visibility = ["//tensorflow:__subpackages__"],
)

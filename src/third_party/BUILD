package(default_visibility = ["//visibility:public"])

cc_library(
    name = "folly",
    includes = ["include"],
    linkstatic=1,
    srcs = [
        "lib/libfolly.a",
    ],
    linkopts = [
        "-lboost_context"
        ,"-lboost_program_options"
        ,"-lboost_thread"
        ,"-lboost_filesystem"
        ,"-lboost_system"
        ,"-lboost_regex"
        ,"-lpthread"
        ,"-llzma"
        ,"-lz"
        ,"-lsnappy"
        ,"-llz4"
        ,"-ljemalloc"
        ,"-levent"
        ,"-ldouble-conversion"
        ,"-lssl"
        ,"-lgflags"
        ,"-lglog"
        ,"-lm"
    ],
    visibility = ["//visibility:public"]
)

cc_library(
    name = "wangle",
    includes = ["include"],
    linkstatic=1,
    srcs = [
        "lib/libwangle.a",
    ],
    deps = ["//src/third_party:folly"],
    visibility = ["//visibility:public"]
)

cc_library(
    name = "jsoncpp",
    includes = ["include"],
    linkstatic=1,
    srcs = [
        "lib/libjsoncpp.a",
    ],
    visibility = ["//visibility:public"]
)

cc_library(
    name = "proxygen",
    includes = ["include"],
    linkstatic=1,
    srcs = [
        "lib/libproxygenhttpserver.a",
        "lib/libproxygenlib.a"
    ],
    deps = ["//src/third_party:wangle"],
    linkopts = [
        "-lcrypto"
        ,"-lcap"
        ,"-lboost_context"
        ,"-lboost_program_options"
        ,"-lboost_filesystem"
        ,"-lboost_regex"
        ,"-lpthread"
        ,"-llzma"
        ,"-lz"
        ,"-lsnappy"
        ,"-llz4"
        ,"-ljemalloc"
        ,"-levent"
        ,"-lssl"
        ,"-lgflags"
        ,"-lglog"
        ,"-ldouble-conversion"
        ,"-lboost_system"
        ,"-lboost_thread"
        ,"-ldl"
    ],
    visibility = ["//visibility:public"]
)


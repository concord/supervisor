cc_binary(
    name = "smf",
    srcs = ["main.cc", "SupervisorDeleteHandler.cc", "SupervisorGetHandler.cc", "SupervisorPostHandler.cc", "SupervisorHandlerFactory.cc", "BadRequestHandler.cc" ],
    deps = ["//src/third_party:proxygen", "//src/third_party:jsoncpp"],
    linkopts = [ "-lprocps" ]
)

package(default_visibility = ["//visibility:public"])


cc_library(
    name = "mam_client",
    srcs = ["client.c"],
    hdrs = ["client.h"],
    deps = [
        "@entangled//cclient/api",
        "@entangled//common/trinary:trit_tryte",
        "@entangled//common/trinary:tryte_ascii",
        "@entangled//mam/api",
    ],
    visibility = ["//visibility:public"],
)
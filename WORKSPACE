workspace(name = "iota_cclient")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")


git_repository(
    name = "entangled",
    commit = "develop",
    remote = "https://github.com/iotaledger/entangled.git",
)

git_repository(
    name = "rules_iota",
    commit = "be6081052283769b9b934d66b824f31695014c82",
    remote = "https://github.com/iotaledger/rules_iota.git",
)

git_repository(
    name = "iota_toolchains",
    commit = "0f93f76fdff9f091dd3a99aa3f19b5d3f4f7f7fc",
    remote = "https://github.com/iotaledger/toolchains.git",
)

git_repository(
    name = "bazel_toolchains",
    commit = "b6875a7bb09b4fa1db8ea347852c0dc9ccae74ab",
    remote = "https://github.com/th0br0/bazel-toolchains.git",
)

load("@rules_iota//:defs.bzl", "iota_deps")

iota_deps()

load("@iota_toolchains//:toolchains.bzl", "setup_initial_deps")

setup_initial_deps()

load("@iota_toolchains//:defs.bzl", "setup_toolchains_repositories")

setup_toolchains_repositories()
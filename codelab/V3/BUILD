# Copyright 2021 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

exports_files(
    srcs = glob([
        "exercise*.h",
        "exercise*_test.cc",
    ]),
    visibility = ["//codelab/solutions:__pkg__"],
)

# Exclude tests from tap and glob runs since they start failing for the codelab.
# The solutions directory has test targets that are included to catch breaking changes.
EXERCISE_TEST_TAGS = [
    "manual",
    "notap",
    "norapid",
]

cc_library(
    name = "exercise1",
    srcs = ["exercise1.cc"],
    hdrs = ["exercise1.h"],
    tags = [
        "manual",
        "nobuilder",
    ],
    deps = [
        "//eval/public:activation",
        "//eval/public:builtin_func_registrar",
        "//eval/public:cel_expr_builder_factory",
        "//eval/public:cel_expression",
        "//eval/public:cel_options",
        "//eval/public:cel_value",
        "//internal:status_macros",
        "//parser",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_cel_spec//proto/cel/expr:syntax_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "exercise1_test",
    srcs = ["exercise1_test.cc"],
    tags = EXERCISE_TEST_TAGS,
    deps = [
        ":exercise1",
        "//internal:testing",
        "@com_google_absl//absl/status",
    ],
)

cc_library(
    name = "exercise2",
    srcs = ["exercise2.cc"],
    hdrs = ["exercise2.h"],
    deps = [
        ":cel_compiler",
        "//compiler",
        "//compiler:compiler_factory",
        "//compiler:standard_library",
        "//eval/public:activation",
        "//eval/public:builtin_func_registrar",
        "//eval/public:cel_expr_builder_factory",
        "//eval/public:cel_expression",
        "//eval/public:cel_options",
        "//eval/public:cel_value",
        "//internal:status_macros",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_cel_spec//proto/cel/expr:checked_cc_proto",
        "@com_google_cel_spec//proto/cel/expr:syntax_cc_proto",
        "@com_google_googleapis//google/rpc/context:attribute_context_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "exercise2_test",
    srcs = ["exercise2_test.cc"],
    tags = EXERCISE_TEST_TAGS,
    deps = [
        ":exercise2",
        "//internal:testing",
        "@com_google_absl//absl/status",
        "@com_google_googleapis//google/rpc/context:attribute_context_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "exercise3_test",
    srcs = ["exercise3_test.cc"],
    tags = EXERCISE_TEST_TAGS,
    deps = [
        ":exercise2",
        "//internal:testing",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_googleapis//google/rpc/context:attribute_context_cc_proto",
    ],
)

cc_library(
    name = "cel_compiler",
    hdrs = ["cel_compiler.h"],
    deps = [
        "//checker:validation_result",
        "//common:ast_proto",
        "//compiler",
        "//internal:status_macros",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_cel_spec//proto/cel/expr:checked_cc_proto",
    ],
)

cc_test(
    name = "cel_compiler_test",
    srcs = ["cel_compiler_test.cc"],
    deps = [
        ":cel_compiler",
        "//common:decl",
        "//common:type",
        "//compiler",
        "//compiler:compiler_factory",
        "//compiler:standard_library",
        "//eval/public:activation",
        "//eval/public:activation_bind_helper",
        "//eval/public:builtin_func_registrar",
        "//eval/public:cel_expr_builder_factory",
        "//eval/public:cel_function_adapter",
        "//eval/public:cel_value",
        "//eval/public/testing:matchers",
        "//internal:testing",
        "@com_google_absl//absl/log:absl_check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:status_matchers",
        "@com_google_googleapis//google/rpc/context:attribute_context_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "exercise4",
    srcs = ["exercise4.cc"],
    hdrs = ["exercise4.h"],
    deps = [
        ":cel_compiler",
        "//compiler",
        "//compiler:compiler_factory",
        "//compiler:standard_library",
        "//eval/public:activation",
        "//eval/public:activation_bind_helper",
        "//eval/public:builtin_func_registrar",
        "//eval/public:cel_expr_builder_factory",
        "//eval/public:cel_expression",
        "//eval/public:cel_options",
        "//eval/public:cel_value",
        "//internal:status_macros",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_cel_spec//proto/cel/expr:checked_cc_proto",
        "@com_google_googleapis//google/rpc/context:attribute_context_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "exercise4_test",
    srcs = ["exercise4_test.cc"],
    tags = EXERCISE_TEST_TAGS,
    deps = [
        ":exercise4",
        "//internal:testing",
        "@com_google_googleapis//google/rpc/context:attribute_context_cc_proto",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//:struct_cc_proto",
    ],
)


cc_binary(
    name = "manual_ast",
    srcs = ["manual_ast.cc"],
    deps = [
        "@com_google_cel_spec//proto/cel/expr:syntax_cc_proto", # 依赖包含 Expr 定义的库
    ],
)

# 1. 为 CelRule 创建一个库 (新添加)
cc_library(
    name = "cel_rule",
    hdrs = ["CelRule.h"],
    # C++17 是为下一个练习 (std::any) 准备，但现在不是必需的
    copts = ["-std=c++17"],
)

# 2. 更新 rule_manager 库
cc_library(
    name = "rule_manager",
    srcs = ["RuleManager.cc"],
    hdrs = ["RuleManager.h"],
    copts = ["-std=c++17"],
    deps = [
        # 添加对 cel_rule 库的依赖
        ":cel_rule",
        "@com_google_absl//absl/strings",
    ],
)

# 3. 更新 rule_manager_example 二进制文件
cc_binary(
    name = "rule_manager_example",
    srcs = ["main.cc"],
    copts = ["-std=c++17"],
    deps = [
        # main.cc 现在也需要 :rule_manager
        ":rule_manager",
    ],
)

# --- 为热替换（Hot-Swap）任务添加的新规则 ---

# 1. 为 RuleSet.h 创建库
cc_library(
    name = "hot_swap_rule_set",
    hdrs = ["RuleSet.h"],
    copts = ["-std=c++17"],
)

# 2. 为 RuleSetManager.h 创建库
cc_library(
    name = "hot_swap_rule_manager",
    hdrs = ["RuleSetManager.h"],
    copts = ["-std=c++17"],
    deps = [
        # 依赖于 RuleSet 库
        ":hot_swap_rule_set",
    ],
)

# 3. 为 hot_swap_main.cc 创建可执行文件
cc_binary(
    name = "hot_swap_example",
    srcs = ["hot_swap_main.cc"], # <-- 使用了你的新文件名
    copts = ["-std=c++17"],
    deps = [
        # 依赖于 Manager 库
        ":hot_swap_rule_manager",
    ],
)

# --- 为 Protobuf 任务添加的新规则 ---

# 1. 告诉 Bazel 这是一个 Protobuf 库
proto_library(
    name = "cel_rule_def_proto",
    srcs = ["cel_rule_def.proto"],
)

# 2. 从上面的 proto_library 生成 C++ 代码
#    这将创建 cel_rule_def.pb.h 和 cel_rule_def.pb.cc
cc_proto_library(
    name = "cel_rule_def_cc_proto",
    deps = [":cel_rule_def_proto"],
)

# 3. 编译你的 proto_test_example.cc
cc_binary(
    name = "proto_test_example",
    srcs = ["proto_test.cc"],
    deps = [
        # 依赖于上面生成的 C++ proto 库
        ":cel_rule_def_cc_proto",

        # 依赖于 protobuf 的核心 C++ 运行时库
        "@com_google_protobuf//:protobuf",
    ],
)

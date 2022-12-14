package(
    default_visibility = ["//visibility:public"],
    features = ["header_modules"],
)

licenses(["notice"])

cc_library(
    name = "reader",
    hdrs = ["reader.h"],
    deps = [
        ":reader_and_writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "writer",
    hdrs = ["writer.h"],
    deps = [
        ":reader_and_writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:cord_utils",
        "//riegeli/base:object",
        "//riegeli/base:reset",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/meta:type_traits",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "backward_writer",
    hdrs = ["backward_writer.h"],
    deps = [
        ":reader_and_writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:cord_utils",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/meta:type_traits",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "reader_and_writer",
    srcs = [
        "backward_writer.cc",
        "reader.cc",
        "writer.cc",
    ],
    hdrs = [
        "backward_writer.h",
        "reader.h",
        "writer.h",
    ],
    visibility = ["//visibility:private"],
    deps = [
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffer",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:cord_utils",
        "//riegeli/base:object",
        "//riegeli/base:reset",
        "//riegeli/base:status",
        "//riegeli/base:string_utils",
        "//riegeli/base:type_traits",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/meta:type_traits",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "read_all",
    srcs = ["read_all.cc"],
    hdrs = ["read_all.h"],
    deps = [
        ":reader",
        "//riegeli/base:arithmetic",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "copy_all",
    srcs = ["copy_all.cc"],
    hdrs = ["copy_all.h"],
    deps = [
        ":backward_writer",
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "write",
    hdrs = ["write.h"],
    deps = [
        ":backward_writer",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:dependency",
        "//riegeli/base:type_traits",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/meta:type_traits",
        "@com_google_absl//absl/status",
    ],
)

cc_library(
    name = "pullable_reader",
    srcs = ["pullable_reader.cc"],
    hdrs = ["pullable_reader.h"],
    deps = [
        ":backward_writer",
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffer",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "pushable_writer",
    srcs = ["pushable_writer.cc"],
    hdrs = ["pushable_writer.h"],
    deps = [
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:cord_utils",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "pushable_backward_writer",
    srcs = ["pushable_backward_writer.cc"],
    hdrs = ["pushable_backward_writer.h"],
    deps = [
        ":backward_writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:cord_utils",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "buffer_options",
    srcs = ["buffer_options.cc"],
    hdrs = ["buffer_options.h"],
    deps = [
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "buffered_reader",
    srcs = ["buffered_reader.cc"],
    hdrs = ["buffered_reader.h"],
    deps = [
        ":backward_writer",
        ":buffer_options",
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "buffered_writer",
    srcs = ["buffered_writer.cc"],
    hdrs = ["buffered_writer.h"],
    deps = [
        ":buffer_options",
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffer",
        "//riegeli/base:buffering",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "//riegeli/base:zeros",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "wrapped_reader",
    srcs = ["wrapped_reader.cc"],
    hdrs = ["wrapped_reader.h"],
    deps = [
        ":backward_writer",
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "wrapped_writer",
    srcs = ["wrapped_writer.cc"],
    hdrs = ["wrapped_writer.h"],
    deps = [
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "wrapped_backward_writer",
    srcs = ["wrapped_backward_writer.cc"],
    hdrs = ["wrapped_backward_writer.h"],
    deps = [
        ":backward_writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "limiting_reader",
    srcs = ["limiting_reader.cc"],
    hdrs = ["limiting_reader.h"],
    deps = [
        ":backward_writer",
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "limiting_writer",
    srcs = ["limiting_writer.cc"],
    hdrs = ["limiting_writer.h"],
    deps = [
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "limiting_backward_writer",
    srcs = ["limiting_backward_writer.cc"],
    hdrs = ["limiting_backward_writer.h"],
    deps = [
        ":backward_writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "prefix_limiting_reader",
    srcs = ["prefix_limiting_reader.cc"],
    hdrs = ["prefix_limiting_reader.h"],
    deps = [
        ":backward_writer",
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:status",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "prefix_limiting_writer",
    srcs = ["prefix_limiting_writer.cc"],
    hdrs = ["prefix_limiting_writer.h"],
    deps = [
        ":prefix_limiting_reader",
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:status",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "null_writer",
    srcs = ["null_writer.cc"],
    hdrs = ["null_writer.h"],
    deps = [
        ":buffer_options",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffer",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "null_backward_writer",
    srcs = ["null_backward_writer.cc"],
    hdrs = ["null_backward_writer.h"],
    deps = [
        ":backward_writer",
        ":buffer_options",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffer",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "joining_reader",
    srcs = ["joining_reader.cc"],
    hdrs = ["joining_reader.h"],
    deps = [
        ":pullable_reader",
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:status",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
    ],
)

cc_library(
    name = "splitting_writer",
    srcs = ["splitting_writer.cc"],
    hdrs = ["splitting_writer.h"],
    deps = [
        ":chain_reader",
        ":cord_reader",
        ":pushable_writer",
        ":string_reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:status",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "reader_factory",
    srcs = ["reader_factory.cc"],
    hdrs = ["reader_factory.h"],
    deps = [
        ":buffer_options",
        ":pullable_reader",
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:stable_dependency",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "array_writer",
    srcs = ["array_writer.cc"],
    hdrs = ["array_writer.h"],
    deps = [
        ":pushable_writer",
        ":reader",
        ":string_reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "array_backward_writer",
    srcs = ["array_backward_writer.cc"],
    hdrs = ["array_backward_writer.h"],
    deps = [
        ":pushable_backward_writer",
        "//riegeli/base:assert",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "string_reader",
    srcs = ["string_reader.cc"],
    hdrs = ["string_reader.h"],
    deps = [
        ":reader",
        "//riegeli/base:assert",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "string_writer",
    srcs = ["string_writer.cc"],
    hdrs = ["string_writer.h"],
    deps = [
        ":reader",
        ":string_reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:cord_utils",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "resizable_writer",
    srcs = ["resizable_writer.cc"],
    hdrs = ["resizable_writer.h"],
    deps = [
        ":reader",
        ":string_reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:cord_utils",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "chain_reader",
    srcs = ["chain_reader.cc"],
    hdrs = ["chain_reader.h"],
    deps = [
        ":backward_writer",
        ":pullable_reader",
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "chain_writer",
    srcs = ["chain_writer.cc"],
    hdrs = ["chain_writer.h"],
    deps = [
        ":chain_reader",
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "chain_backward_writer",
    srcs = ["chain_backward_writer.cc"],
    hdrs = ["chain_backward_writer.h"],
    deps = [
        ":backward_writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "cord_reader",
    srcs = ["cord_reader.cc"],
    hdrs = ["cord_reader.h"],
    deps = [
        ":backward_writer",
        ":pullable_reader",
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "cord_writer",
    srcs = ["cord_writer.cc"],
    hdrs = ["cord_writer.h"],
    deps = [
        ":cord_reader",
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffer",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:cord_utils",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "//riegeli/base:zeros",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "cord_backward_writer",
    srcs = ["cord_backward_writer.cc"],
    hdrs = ["cord_backward_writer.h"],
    deps = [
        ":backward_writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:buffer",
        "//riegeli/base:buffering",
        "//riegeli/base:chain",
        "//riegeli/base:cord_utils",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "//riegeli/base:zeros",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "file_mode_string",
    srcs = ["file_mode_string.cc"],
    hdrs = ["file_mode_string.h"],
    visibility = ["//visibility:private"],
    deps = [
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "fd_reader",
    srcs = [
        "fd_dependency.h",
        "fd_reader.cc",
    ],
    hdrs = ["fd_reader.h"],
    # fd_reader.cc has #define before #include to influence what the included
    # files provide.
    features = ["-use_header_modules"],
    deps = [
        ":buffer_options",
        ":buffered_reader",
        ":fd_close",
        ":fd_internal",
        ":reader",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:dependency",
        "//riegeli/base:no_destructor",
        "//riegeli/base:object",
        "//riegeli/base:status",
        "//riegeli/base:types",
    ] + select({
        "@platforms//os:windows": [
            "//riegeli/base:unicode",
        ],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "fd_mmap_reader",
    srcs = [
        "fd_dependency.h",
        "fd_mmap_reader.cc",
    ],
    hdrs = ["fd_mmap_reader.h"],
    # fd_mmap_reader.cc has #define before #include to influence what the
    # included files provide.
    features = ["-use_header_modules"],
    deps = [
        ":chain_reader",
        ":fd_close",
        ":fd_internal",
        ":reader",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:chain",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:status",
        "//riegeli/base:types",
    ] + select({
        "@platforms//os:windows": [
            "//riegeli/base:unicode",
        ],
        "//conditions:default": [
            "@com_google_absl//absl/status:statusor",
            "//riegeli/base:no_destructor",
        ],
    }),
)

cc_library(
    name = "fd_writer",
    srcs = [
        "fd_dependency.h",
        "fd_writer.cc",
    ],
    hdrs = ["fd_writer.h"],
    # fd_writer.cc has #define before #include to influence what the included
    # files provide.
    features = ["-use_header_modules"],
    deps = [
        ":buffer_options",
        ":buffered_writer",
        ":fd_close",
        ":fd_internal",
        ":fd_reader",
        ":reader",
        ":writer",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:dependency",
        "//riegeli/base:no_destructor",
        "//riegeli/base:object",
        "//riegeli/base:status",
        "//riegeli/base:types",
    ] + select({
        "@platforms//os:windows": [
            "//riegeli/base:unicode",
        ],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "fd_close",
    srcs = ["fd_close.cc"],
    hdrs = ["fd_close.h"],
    visibility = ["//visibility:private"],
    deps = [
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "fd_internal",
    srcs = ["fd_internal.cc"],
    hdrs = ["fd_internal.h"],
    visibility = ["//visibility:private"],
    deps = [
        "//riegeli/base:constexpr",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "std_io",
    srcs = ["std_io.cc"],
    hdrs = ["std_io.h"],
    deps = [
        ":fd_reader",
        ":fd_writer",
        "//riegeli/base:assert",
        "//riegeli/base:chain",
        "//riegeli/base:no_destructor",
        "//riegeli/base:object",
        "@com_google_absl//absl/base:core_headers",
    ],
)

cc_library(
    name = "istream_reader",
    srcs = [
        "istream_reader.cc",
        "stream_internal.h",
    ],
    hdrs = ["istream_reader.h"],
    deps = [
        ":buffer_options",
        ":buffered_reader",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:dependency",
        "//riegeli/base:no_destructor",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/meta:type_traits",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "ostream_writer",
    srcs = [
        "ostream_writer.cc",
        "stream_internal.h",
    ],
    hdrs = ["ostream_writer.h"],
    deps = [
        ":buffer_options",
        ":buffered_writer",
        ":istream_reader",
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:dependency",
        "//riegeli/base:no_destructor",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/meta:type_traits",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "reader_istream",
    srcs = ["reader_istream.cc"],
    hdrs = ["reader_istream.h"],
    deps = [
        ":reader",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "writer_ostream",
    srcs = ["writer_ostream.cc"],
    hdrs = ["writer_ostream.h"],
    deps = [
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "cfile_reader",
    srcs = [
        "cfile_dependency.h",
        "cfile_internal.h",
        "cfile_reader.cc",
    ],
    hdrs = ["cfile_reader.h"],
    features = select({
        "@platforms//os:windows": [],
        # cfile_reader.cc has #define before #include to influence what the
        # included files provide.
        "//conditions:default": ["-use_header_modules"],
    }),
    deps = [
        ":buffer_options",
        ":buffered_reader",
        ":file_mode_string",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/base:dynamic_annotations",
        "@com_google_absl//absl/meta:type_traits",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:constexpr",
        "//riegeli/base:dependency",
        "//riegeli/base:no_destructor",
        "//riegeli/base:object",
        "//riegeli/base:status",
        "//riegeli/base:types",
    ] + select({
        "@platforms//os:windows": [
            "//riegeli/base:unicode",
        ],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "cfile_writer",
    srcs = [
        "cfile_dependency.h",
        "cfile_internal.h",
        "cfile_writer.cc",
    ],
    hdrs = ["cfile_writer.h"],
    features = select({
        "@platforms//os:windows": [],
        # cfile_writer.cc has #define before #include to influence what the
        # included files provide.
        "//conditions:default": ["-use_header_modules"],
    }),
    deps = [
        ":buffer_options",
        ":buffered_writer",
        ":cfile_reader",
        ":file_mode_string",
        ":reader",
        ":writer",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/meta:type_traits",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:constexpr",
        "//riegeli/base:dependency",
        "//riegeli/base:no_destructor",
        "//riegeli/base:object",
        "//riegeli/base:status",
        "//riegeli/base:types",
    ] + select({
        "@platforms//os:windows": [
            "//riegeli/base:unicode",
        ],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "reader_cfile",
    srcs = [
        "cfile_dependency.h",
        "reader_cfile.cc",
    ],
    hdrs = ["reader_cfile.h"],
    # reader_cfile.cc has #define before #include to influence what the included
    # files provide.
    features = ["-use_header_modules"],
    deps = [
        ":reader",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:dependency",
        "//riegeli/base:status",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "writer_cfile",
    srcs = [
        "cfile_dependency.h",
        "writer_cfile.cc",
    ],
    hdrs = ["writer_cfile.h"],
    # writer_cfile.cc has #define before #include to influence what the included
    # files provide.
    features = ["-use_header_modules"],
    deps = [
        ":reader",
        ":writer",
        "//riegeli/base:arithmetic",
        "//riegeli/base:assert",
        "//riegeli/base:dependency",
        "//riegeli/base:status",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/base:dynamic_annotations",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "absl_stringify_writer",
    hdrs = ["absl_stringify_writer.h"],
    deps = [
        ":buffered_writer",
        ":prefix_limiting_writer",
        ":writer",
        "//riegeli/base:assert",
        "//riegeli/base:dependency",
        "//riegeli/base:object",
        "//riegeli/base:reset",
        "//riegeli/base:types",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
    ],
)

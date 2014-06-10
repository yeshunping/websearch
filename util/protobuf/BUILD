proto_library(
    name = 'unittest_proto',
    srcs = 'unittest.proto',
)

cc_library(
    name = 'proto_json_format',
    srcs = 'proto_json_format.cpp',
    deps = ['//base:base',
            '//thirdparty/jsoncpp:jsoncpp',
            '//thirdparty/protobuf:protobuf',
            '//thirdparty/zlib:z',
           ],
)

cc_test(
    name = 'proto_json_format_unittest',
    srcs = 'proto_json_format_unittest.cpp',
    deps = [':proto_json_format',
            ':unittest_proto',
            '//file:file',
           ],
)


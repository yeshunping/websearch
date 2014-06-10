proto_library(name = 'rpc_meta_proto',
    srcs = ['sofa/pbrpc/rpc_metadata.proto',
            ],
)

proto_library(name = 'builtin_service_proto',
    srcs = ['sofa/pbrpc/pb_builtin_service.proto',
            ],
)

proto_library(name = 'rpc_option_proto',
    srcs = ['sofa/pbrpc/rpc_option.proto',
            ],
)

cc_library(name = 'sofa',
    srcs = [
        'sofa/pbrpc/block_wrappers.cc',
        'sofa/pbrpc/boost_system_error_code.cc',
        'sofa/pbrpc/buffer.cc',
        'sofa/pbrpc/builtin_service_impl.cc',
        'sofa/pbrpc/builtin_service.pb.cc',
        'sofa/pbrpc/common.cc',
        'sofa/pbrpc/common_internal.cc',
        'sofa/pbrpc/compressed_stream.cc',
        'sofa/pbrpc/gzip_stream.cc',
        'sofa/pbrpc/lz4.cc',
        'sofa/pbrpc/mock_test_helper.cc',
        'sofa/pbrpc/rpc_channel.cc',
        'sofa/pbrpc/rpc_channel_impl.cc',
        'sofa/pbrpc/rpc_client.cc',
        'sofa/pbrpc/rpc_client_impl.cc',
        'sofa/pbrpc/rpc_controller.cc',
        'sofa/pbrpc/rpc_endpoint.cc',
        'sofa/pbrpc/rpc_error_code.cc',
        'sofa/pbrpc/rpc_meta.pb.cc',
        'sofa/pbrpc/rpc_option.pb.cc',
        'sofa/pbrpc/rpc_server.cc',
        'sofa/pbrpc/rpc_server_impl.cc',
        'sofa/pbrpc/thread_group.cc',
        'sofa/pbrpc/timeout_manager.cc',
        'sofa/pbrpc/timeout_manager_impl.cc'
    ],
    incs = ['.', '../../protobuf/include/'],
#    deps = [':rpc_meta_proto', ':rpc_option_proto'],
)


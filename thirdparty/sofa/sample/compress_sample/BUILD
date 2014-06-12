proto_library(name = 'echo_service_proto', srcs = 'echo_service.proto')

cc_binary(name = 'server',
           srcs = 'server.cc',
           deps = [
                   ':echo_service_proto',
                   '//thirdparty/sofa/src:sofa'
                   ],
           )

cc_binary(name = 'client',
           srcs = 'client.cc',
           deps = [
                   ':echo_service_proto',
                   '//thirdparty/sofa/src:sofa'
                   ],
           )

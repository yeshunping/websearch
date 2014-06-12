proto_library(name = 'echo_service_proto', srcs = 'echo_service.proto')

cc_binary(name = 'server',
           srcs = 'server.cc',
           incs = ['../../src', ],
           deps = [
                   ':echo_service_proto',
                   '//thirdparty/sofa/src:sofa'
                   ],
           )

cc_binary(name = 'client_async',
           srcs = 'client_async.cc',
           incs = ['../../src', ],
           deps = [
                   ':echo_service_proto',
                   '//thirdparty/sofa/src:sofa'
                   ],
           )

cc_binary(name = 'client_sync',
           srcs = 'client_sync.cc',
           incs = ['../../src', ],
           deps = [
                   ':echo_service_proto',
                   '//thirdparty/sofa/src:sofa'
                   ],
           )

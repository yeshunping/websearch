proto_library(name = 'echo_service_proto', srcs = 'echo_service.proto')

cc_test(name = 'mock_test_sample',
           srcs = 'mock_test_sample.cc',
           deps = [
                   ':echo_service_proto',
                   '//thirdparty/sofa/src:sofa'
                   ],
           )

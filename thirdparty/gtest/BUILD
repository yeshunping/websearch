cc_library(name = 'gtest',
           srcs = [
                  'src/gtest-all.cc',
                  'src/gtest.cc',
                  'src/gtest-filepath.cc',
                  'src/gtest-port.cc',
                  'src/gtest-printers.cc',
                  'src/gtest-test-part.cc'
                  ],
           deps = [
                   '//base:base',
                  ],
           )

cc_library(name = 'gtest_main',
           srcs = 'src/gtest_main.cc',
           deps = [
                   ':gtest',
           ],
           )

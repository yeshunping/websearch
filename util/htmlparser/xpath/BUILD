cc_library(name = 'xpath_extractor',
           srcs = [
                   'xpath_extractor.cc',
                  ],
           deps = ['//base:base',
                   '//util/htmlparser/htmlparser:htmlparser',
                  ],
           )

cc_test(name = 'xpath_extractor_test',
        srcs = ['xpath_extractor_test.cc',
               ],
        deps = ['//base:base',
                ':xpath_extractor',
               ]
        )

cc_binary(name = 'xpath_tool',
          srcs = ['xpath_tool.cc',
                 ],
          deps = ['//base:base',
                  '//file:file',
                  ':xpath_extractor',
                 ]
          )

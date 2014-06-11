cc_library(name = "url_encode",
           srcs = ["url_encode.cc",
                  ],
          )

#cc_library(name = "effective_tld_names",
#           srcs = ["effective_tld_names.cc",
#                  ],
#          )
cc_library(name = "registry_controlled_domain",
           srcs = ["registry_controlled_domain.cc",
                  ],
           deps = ['//base:base',
                   '//thirdparty/googleurl:gurl',
                   ]
          )

cc_test(name = "registry_controlled_domain_unittest",
           srcs = ["registry_controlled_domain_unittest.cc",
                  ],
           deps = ['//base:base',
                   '//thirdparty/googleurl:gurl',
                   ':registry_controlled_domain',
                   ]
          )

cc_library(name = "url_parser",
           srcs = ['url_parser.cc',
                  ],
           deps = ['//thirdparty/googleurl:gurl',
                  '//base:base',
                  '//file:file',
                  '//file:simple_line_reader',
                  ':registry_controlled_domain',
                 ]
         )

cc_test(name = "url_parser_unittest",
        srcs = ['url_parser_unittest.cc',
               ],
        deps = [':url_parser',
               ]
       )

cc_binary(name = "get_domain_benchmark",
        srcs = ['get_domain_benchmark.cc',
               ],
        deps = [':url_parser',
               ]
       )

# Copyright 2012. All Rights Reserved.
# Author: yeshunping@gmail.com (Shunping Ye)
#
# Description:
#   TODO(shunpingye): describe this package.

cc_library(name = 'file',
           srcs = ['file.cc', ],
           deps = ['//base:base',
                   ]
           )

cc_test(name = 'file_test',
        srcs = ['file_test.cc', ],
        deps = ['//base:base',
                ':file',
                ]
           )

cc_library(name = 'simple_line_reader',
           srcs = ['simple_line_reader.cc', ],
           deps = ['//base:base']
           )

cc_test(name = 'simple_line_reader_test',
        srcs = ['simple_line_reader_test.cc', ],
        deps = ['//base:base',
                ':simple_line_reader',
                ':file',
                ]
           )


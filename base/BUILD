# Copyright 2010. All Rights Reserved.
# Author: yeshunping@gmail.com (Shunping Ye)
#
# Description:
#   TODO(shunpingye): describe this package.

cc_test(name = 'at_exit_unittest',
             srcs = ['at_exit_unittest.cc',
                       ],
             deps = [':base',
                        ]
             )

cc_test(name = 'pickle_unittest',
             srcs = ['pickle_unittest.cc',
                       ],
             deps = [':base',
                        ]
             )

cc_test(name = 'base64_unittest',
             srcs = ['base64_unittest.cc',
                       ],
             deps = [':base',
                        ]
             )

cc_test(name = 'bits_unittest',
             srcs = ['bits_unittest.cc',
                       ],
             deps = [':base',
                        ]
             )

#cc_test(name = 'string_util_unittest',
#             srcs = ['string_util_unittest.cc',
#                       ],
#             deps = [':base',
#                        ]
#             )
#
cc_test(name = 'string_split_unittest',
             srcs = ['string_split_unittest.cc',
                       ],
             deps = [':base',
                        ]
             )

cc_test(name = 'singleton_unittest',
             srcs = ['singleton_unittest.cc',
                       ],
             deps = [':base',
                        ]
             )

cc_test(name = 'scoped_ptr_unittest',
             srcs = ['scoped_ptr_unittest.cc',
                       ],
             deps = [':base',
                        ]
             )

cc_test(name = 'sys_string_conversions_unittest',
             srcs = ['sys_string_conversions_unittest.cc',
                       ],
             deps = [':base',
                        ]
             )

cc_test(name = 'time_unittest',
             srcs = ['time_unittest.cc',
                       ],
             deps = [':base',
                        ]
             )

# use static log library instead of logging.cc in base/thrid_party
cc_library(name = 'base',
                srcs = ['at_exit.cc',
                          'atomicops_internals_x86_gcc.cc',
                          'base_switches.cc',
                          'base64.cc',
                          'debug_util.cc',
                          'debug_util_posix.cc',
                          'env_var.cc',
                          'pickle.cc',
                          'file_path.cc',
                          'file_util.cc',
                          'file_util_posix.cc',
                          'lazy_instance.cc',
                          'threading/platform_thread_posix.cc',
                          'threading/thread_local_posix.cc',
                          'threading/thread_restrictions.cc',
                          'platform_file.cc',
                          'platform_file_posix.cc',
                          'lock.cc',
                          'lock_impl_posix.cc',
                          'logging.cc',
                          'md5.cc',
                          'platform_thread_posix.cc',
                          'utf.cc',
                          'safe_strerror_posix.cc',
                          'string16.cc',
                          'string_piece.cc',
                          'string_split.cc',
                          'string_util.cc',
                          'sys_string_conversions_linux.cc',
                          'time.cc',
                          'time_posix.cc',
                          'thread.cc',
                          'thread_pool.cc',
                          'thirdparty/icu/icu_utf.cc',
                          'thirdparty/nspr/prtime.cc',
                          'thirdparty/symbolize/symbolize.cc',
                          'thirdparty/symbolize/demangle.cc',
                          'thirdparty/dynamic_annotations/dynamic_annotations.c',
                          'utf_string_conversion_utils.cc',
                          'utf_string_conversions.cc',
                          'thirdparty/modp_b64/modp_b64.cc',
                          'kmp.cc',
                          'message_converter.cc',
                          'hash.cc',
                          ],
                deps = [':binary_version',
                        '//thirdparty/stringencoders:stringencoders',
                        '//thirdparty/gflags:gflags',
                        ':dmg_fp',
                        '#pthread',
                        '#rt',
                        ]
                )

cc_library(
    name = 'binary_version',
    srcs = ['binary_version.cc',
            'setup_binary_version.cc',
            ],
    deps = ['//thirdparty/gflags:gflags',],
)

cc_library(name = 'dmg_fp',
           srcs = [
                  'thirdparty/dmg_fp/dtoa.cc',
                   'thirdparty/dmg_fp/g_fmt.cc',
                  ],
           warning='no',
)

# cc_test(name = 'rand_util_unittest',
#              srcs = ['rand_util_unittest.cc',
#                        ],
#              deps = [':base',
#                         ]
#              )

cc_test(name = 'hash_unittest',
        srcs = ['hash_unittest.cc',
               ],
        deps = [':base',
               ]
        )

cc_library(
    name = 'benchmark',
    srcs = 'benchmark.cpp',
    deps = [':base',
            '//thirdparty/perftools:tcmalloc_and_profiler',
           ]
)

cc_library(
    name = 'benchmark_main',
    srcs = 'benchmark_main.cpp',
    deps = [':benchmark',
           ]
)

cc_benchmark(name = 'hash_benchmark',
             srcs = 'hash_benchmark.cc',
             deps = [':base'],
             )

# cc_library(name = 'pcre',
#            prebuilt=True,
# )

cc_library(name = 'pcrecpp',
           srcs = [
                   'pcrecpp.cc',
                   'pcre_scanner.cc',
                   'pcre_stringpiece.cc',
                   ],
           incs = ['.'],
           defs = ['HAVE_CONFIG_H',],
)

cc_library(name = 'pcreposix',
           srcs = ['pcreposix.c',
                   ],
           incs = ['.'],
           defs = ['HAVE_CONFIG_H',],
)

cc_library(name = 'pcre',
    srcs = [
            'pcre_byte_order.c',
            'pcre_compile.c',
            'pcre_config.c',
            'pcre_dfa_exec.c',
            'pcre_exec.c',
            'pcre_fullinfo.c',
            'pcre_get.c',
            'pcre_globals.c',
            'pcre_jit_compile.c',
            'pcre_maketables.c',
            'pcre_newline.c',
            'pcre_ord2utf8.c',
            'pcre_refcount.c',
            'pcre_string_utils.c',
            'pcre_study.c',
            'pcre_tables.c',
            'pcre_ucd.c',
            'pcre_valid_utf8.c',
            'pcre_version.c',
            'pcre_xclass.c',
            'pcre_chartables.c',
            ],
    warning='no',
    incs = ['.'],
    defs = ['HAVE_CONFIG_H',],
)


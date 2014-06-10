cc_library(name = 'opencc',
    srcs =  [
      "src/config_reader.c",
      "src/converter.c",
      "src/dict_group.c",
      "src/dict_chain.c",
      "src/encoding.c",
      "src/utils.c",
      "src/opencc.c",
      "src/dict.c",
      "src/dictionary/datrie.c",
      "src/dictionary/text.c"
    ],
    warning = 'no',
)

cc_binary(name = 'opencc_main',
          srcs = 'src/tools/opencc.c',
          deps = ':opencc',
          )

cc_binary(name = 'opencc_dict',
          srcs = 'src/tools/opencc_dict.c',
          deps = ':opencc',
          )
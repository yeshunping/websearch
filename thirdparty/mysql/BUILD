cc_library(name = 'mysqlclient_r',
           prebuilt=True,
           deps = [
                   '//thirdparty/zlib:z',
                   '//thirdparty/openssl:ssl',
                   '//thirdparty/openssl:crypto',
                   ],
)

cc_library(name = 'mysql',
           deps = [':mysqlclient_r']
)

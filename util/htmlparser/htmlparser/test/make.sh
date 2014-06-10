g++ -g -o htmlparser -I ../include/ -I ../../utils/include/ -I ../../depends/log/include/ -I ../../depends/publish_1.0.1/include/ -I ../../depends/log/include/log4cpp/ htmlparser.cpp -L ../lib -L ../../depends/log/lib64/ -L ../../utils/lib/ -lhtmlparser -lutils -llog -llog4cpp -lrt -lpthread

g++ -g -o xpath xpath.cpp -I ../../utils/include/ -I ../include/ -L ../../utils/lib/ -L ../lib/ -lhtmlparser_debug -lutils_debug

output="runonpagedb"
source="runonpagedb.cpp"

CXX="g++ -g "

INCLUDE=" -I../../include -I../../depends/pcre-8.31/include/ -I../../depends/xml2 -I../../bbsparser/include -I../../linkparser/include -I../../htmlparser/include/ -I../../vhtmlparser/include/ -I../../ahtmlparser/include/ -I../../markparser/include/ -I../../extractor/include/ -I../../cssparser/include/ -I../../utils/include/ -I../../depends/sign/include/ -I../../depends/dict/include/ -I../../depends/log/include/ -I../../depends/log/include/log4cpp/ -I../../depends/publish_1.0.1/include/ -I../../depends/CCharset/include/ -I../../depends/cssserver/include/ -I../../depends/mysqlc/include/ -I../../depends/libiconv/include "

CFLAGS="-L./ -L../../lib -L../../depends/pcre-8.31/lib -L../../depends/xml2/lib -L../../bbsparser/lib -L../../linkparser/lib -L../../extractor/lib -L../../markparser/lib -L../../ahtmlparser/lib -L../../vhtmlparser/lib -L../../cssparser/lib -L../../htmlparser/lib -L../../depends/sign/lib -L../../depends/CCharset/lib64/ -L../../depends/cssserver/edblib -L../../utils/lib -L../../depends/log/lib64 -L../../depends/mysqlc/lib  -L../../depends/libiconv/lib"

LIBS=" -llinkparser -lbbsparser -lextractorparser -lmarkparser -lahtmlparser -lvhtmlparser -lcssparser -lhtmlparser -lutils -ledbbatch -ledb -lccharset -lsign -llog -llog4cpp -lxml2 -lpthread -lssl -lrt -Wl,-Bstatic -lmysqlclient -liconv -lpcre -lboost_thread -Wl,-Bdynamic"
LIBS_D=" -llinkparser -lbbsparser -lextractorparser -lmarkparser -lahtmlparser -lvhtmlparser -lcssparser -lhtmlparser -lutils -ledbbatch -ledb -lccharset -lsign -llog -llog4cpp -lxml2 -lpthread -lssl -lrt -Wl,-Bstatic -lmysqlclient -liconv -lpcre -lboost_thread -Wl,-Bdynamic"
LIBS_NOCSS=" -lextractorparser -lmarkparser -lahtmlparser -lvhtmlparser_nocss -lcssparser -lhtmlparser -lutils -ledbbatch -ledb -lsegWords -lccharset -lsign -ltries -llog -llog4cpp -lpthread -lssl -lrt "
LIBS_NOCSS_D=" -lextractorparser_debug -lmarkparser_debug -lahtmlparser_debug -lvhtmlparser_nocss_debug -lcssparser_debug -lhtmlparser_debug -lutils_debug -ledbbatch -ledb -lsegWords -lccharset -lsign -ltries -llog -llog4cpp -lpthread -lssl -lrt "

$CXX -g $INCLUDE -o $output $source $CFLAGS $LIBS -DNEED_OUT_CSS
#$CXX -g $INCLUDE -o $output"_d" $source $CFLAGS $LIBS_D -DNEED_OUT_CSS

#$CXX $INCLUDE -o $output"_d" $source $CFLAGS $LIBS_D -DNEED_OUT_CSS
#$CXX $INCLUDE -o $output"_nocss" $source $CFLAGS $LIBS_NOCSS
#$CXX $INCLUDE -o $output"_nocss_d" $source $CFLAGS $LIBS_NOCSS_D

INCLUDE = \
		-I/usr/local/include \
		-I/usr/local/gperftools-2.0/include/ \
		-I./ \
		-I../../depends/CCharset/include \
		-I../../depends/chardet/include \
		-I../../depends/include	\
		-I../../depends/log/include	\
		-I../../depends/log/include/log4cpp \
		-I../../depends/libiconv/include/ \
		-I../../depends/mysqlc/include/ \
		-I../../depends/jansson-2.4/include/ \
		-I../../depends/pcre-8.31/include/ \
		-I../../depends/publish_1.0.1/include/ \
		-I../../depends/sign/include/ \
		-I../../depends/cssserver/include/ \
		-I../../depends/stthrift/include/ \
		-I../../depends/thrift-0.7.0/include/thrift/ \
		-I../../depends/classify/include/ \
		-I../../test \
		-I../../utils/include \
		-I../../htmlparser/include \
		-I../../cssparser/include \
		-I../../vhtmlparser/include \
		-I../../ahtmlparser/include \
		-I../../markparser/include \
		-I../../extractor/include \
		-I../../extractor/thrift \
		-I../../bbsparser/include \
		-I../../linkparser/include

LIB_DIR  = \
		-L/usr/local/lib \
		-L/usr/local/gperftools-2.0/lib/ \
		-L./ \
		-L../lib \
		-L../../depends/CCharset/lib64 \
		-L../../depends/chardet/lib64 \
		-L../../depends/cssserver/edblib \
		-L../../depends/log/lib64 \
		-L../../depends/libiconv/lib/ \
		-L../../depends/mysqlc/lib/ \
		-L../../depends/jansson-2.4/lib/ \
		-L../../depends/pcre-8.31/lib \
		-L../../depends/sign/lib \
		-L../../depends/stthrift/lib \
		-L../../depends/thrift-0.7.0/lib/ \
		-L../../depends/classify/lib/ \
		-L../../htmlparser/lib \
		-L../../cssparser/lib \
		-L../../vhtmlparser/lib \
		-L../../ahtmlparser/lib \
		-L../../markparser/lib \
		-L../../extractor/lib \
		-L../../bbsparser/lib \
		-L../../linkparser/lib \
		-L../../utils/lib 
		

LIB = \
		-lpageclassify \
		-llinkparser \
		-lbbsparser \
		-lextractorparser \
		-lmarkparser \
		-lahtmlparser \
		-lvhtmlparser \
		-lhtmlparser \
		-lcssparser \
		-lutils \
		-ledbbatch -ledb \
		-ljansson \
		-lpcre \
		-lsign \
		-lstthrift \
		-llog -llog4cpp	\
		-lssl -lpthread -lrt \
		-Wl,-Bstatic -lboost_date_time -lxml2 -lthrift -lmysqlclient -liconv -lccharset -lchardet -Wl,-Bdynamic
		
LIB_PROF = \
		-lpageclassify \
		-llinkparser \
		-lbbsparser \
		-lextractorparser \
		-lmarkparser \
		-lahtmlparser \
		-lvhtmlparser \
		-lhtmlparser \
		-lcssparser \
		-lutils \
		-ledbbatch -ledb \
		-ljansson \
		-lpcre \
		-lsign \
		-lstthrift \
		-llog -llog4cpp	\
		-lssl -lpthread -lrt -lprofiler \
		-Wl,-Bstatic -lboost_date_time -lxml2 -lthrift -lmysqlclient -liconv -lccharset -lchardet -Wl,-Bdynamic

LIB_DEBUG = \
		-lpageclassify \
		-llinkparser \
		-lbbsparser \
		-lextractorparser_debug \
		-lmarkparser \
		-lahtmlparser \
		-lvhtmlparser_debug \
		-lhtmlparser_debug \
		-lcssparser \
		-lutils_time \
		-ledbbatch -ledb \
		-ljansson \
		-lpcre \
		-lsign \
		-lstthrift \
		-llog -llog4cpp	\
		-lssl -lpthread -lrt \
		-Wl,-Bstatic -lboost_date_time -lxml2 -lthrift -lmysqlclient -liconv -lccharset -lchardet -Wl,-Bdynamic

LIB_SAVECSS = \
		-lpageclassify \
		-llinkparser \
		-lbbsparser \
		-lextractorparser_debug \
		-lmarkparser \
		-lahtmlparser \
		-lvhtmlparser_savecss \
		-lhtmlparser_debug \
		-lcssparser \
		-lutils_time \
		-ledbbatch -ledb \
		-ljansson \
		-lpcre \
		-lsign \
		-lstthrift \
		-llog -llog4cpp	\
		-lssl -lpthread -lrt \
		-Wl,-Bstatic -lboost_date_time -lxml2 -lthrift -lmysqlclient -liconv -lccharset -lchardet -Wl,-Bdynamic
		
LIB_TIME = \
		-lpageclassify \
		-llinkparser \
		-lbbsparser \
		-lextractorparser_time \
		-lmarkparser \
		-lahtmlparser \
		-lvhtmlparser \
		-lhtmlparser \
		-lcssparser \
		-lutils_time \
		-ledbbatch -ledb \
		-ljansson \
		-lpcre \
		-lsign \
		-lstthrift \
		-llog -llog4cpp	\
		-lssl -lpthread -lrt \
		-Wl,-Bstatic -lboost_date_time -lxml2 -lthrift -lmysqlclient -liconv -lccharset -lchardet -Wl,-Bdynamic
		
LIB_DUMP = \
		-lpageclassify \
		-llinkparser \
		-lbbsparser \
		-lextractorparser_dump \
		-lmarkparser_dump \
		-lahtmlparser_dump \
		-lvhtmlparser_dump \
		-lhtmlparser_dump \
		-lcssparser_dump \
		-lutils_dump \
		-ledbbatch -ledb \
		-ljansson \
		-lpcre \
		-lsign \
		-lstthrift \
		-llog -llog4cpp	\
		-lssl -lpthread -lrt \
		-Wl,-Bstatic -lboost_date_time -lxml2 -lthrift -lmysqlclient -liconv -lccharset -lchardet -Wl,-Bdynamic

COMMON_DEFINES = -DLINUX -D_REENTERANT -DNEED_OUT_CSS
CFLAGS= -fPIC  -Wall $(COMMON_DEFINES) $(INCLUDE)

CXX=g++ -g -pipe $(CFLAGS)
CC = g++ -Wall -fPIC -DLINUX -D_REENTERANT -D_FILE_OFFSET_BITS=64 -DDEBUG_TIME -g -o
		
%.o : %.cpp
	g++ -O2 -o $@ $< -c $(INCLUDE)
%_d.o : %.cpp
	g++ -g -o $@ $< -c $(INCLUDE)
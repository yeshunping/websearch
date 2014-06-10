WORKROOT = ../../depends/
	
LIBPATH=$(WORKROOT)publish_1.0.1

INCLUDE = \
		-I./ \
		-I../include \
		-I../../htmlparser/include \
		-I../../cssparser/include \
		-I../../utils/include \
		-I$(LIBPATH)/include	\
		-I$(LIBPATH)/log/include	\
		-I$(LIBPATH)/log/include/log4cpp	\
		-I../../depends/CCharset/include \
		-I../../depends/mysqlc/include/ \
		-I../../depends/libiconv/include/
		
ifeq ($(m),32)
LIB  = \
		-L./ \
		-L../lib \
		-L../../htmlparser/lib \
		-L../../cssparser/lib \
		-L../../common/lib \
		-L../../utils/lib \
		-L../../depends/log/lib32 \
		-L../../depends/CCharset/lib32 \
		-L../../depends/chardet/lib32 \
		-L../../depends/cssserver/lib32 \
		-lvhtmlparser \
		-lhtmlparser \
		-lcssparser \
		-lutils \
		-lccharset -lchardet\
		-llog -llog4cpp	\
		-lssl -lpthread \
		-ledbbatch \
		-ledb -lrt
else
LIB  = \
		-L./ \
		-L../lib \
		-L../../htmlparser/lib \
		-L../../cssparser/lib \
		-L../../common/lib \
		-L../../utils/lib \
		-L../../depends/log/lib64 \
		-L../../depends/CCharset/lib64 \
		-L../../depends/chardet/lib64 \
		-L../../depends/cssserver/edblib \
		-L../../depends/mysqlc/lib \
		-L../../depends/libiconv/lib \
		-lvhtmlparser_debug \
		-lhtmlparser_debug \
		-lcssparser_debug \
		-lutils_debug \
		-liconv\
		-lmysqlclient \
		-lccharset -lchardet\
		-llog -llog4cpp	\
		-lssl -lpthread \
		-ledbbatch \
		-ledb -lrt
endif
	
ifeq ($(css),yes)
COMMON_DEFINES = -D_INDEX_IO_DEBUG -DLINUX -D_REENTERANT -DNEED_OUT_CSS -DDEBUG_INFO
else
COMMON_DEFINES = -D_INDEX_IO_DEBUG -DLINUX -D_REENTERANT -DDEBUG_INFO
endif	

CFLAGS= -fPIC  -Wall $(COMMON_DEFINES) $(INCLUDE)

ifeq ($(o),yes)
CXX=g++ -O2 -pipe $(CFLAGS)
CC = g++ -Wall -fPIC -DLINUX -D_REENTERANT -D_FILE_OFFSET_BITS=64 -O2 -o
else
CXX=g++ -g -pipe $(CFLAGS)
CC = g++ -Wall -fPIC -DLINUX -D_REENTERANT -D_FILE_OFFSET_BITS=64 -g -o
endif

EASOU_TEST_SRC:=$(wildcard *.cpp)

EASOU_TEST_OBJ:=$(EASOU_TEST_SRC:%.cpp=%.o)

TESTS=vhtml_parser_test

all: $(TESTS) parser clear

clear:
	/bin/rm -rf *.o
	
clean:
	/bin/rm -f *.o
	/bin/rm -f $(TESTS)
	rm -f parser
	
rebuild : clean all

vhtml_parser_test: $(EASOU_TEST_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB)

parser: parser.o
	$(CC) $@ $^ $(INCLUDE) $(LIB)
	

WORKROOT = ../../depends/
	
LIBPATH=$(WORKROOT)publish_1.0.1

INCLUDE = \
		-I./ \
		-I../include \
		-I../../depends/CCharset/include \
		-I../../depends/chardet/include \
		-I../../htmlparser/include \
		-I../../cssparser/include \
		-I../../vhtmlparser/include \
		-I../../ahtmlparser/include \
		-I../../utils/include \
		-I../../extractor/include \
		-I$(LIBPATH)/include	\
		-I$(LIBPATH)/log/include	\
		-I$(LIBPATH)/log/include/log4cpp
		
ifeq ($(m),32)
LIB  = \
		-L./ \
		-L../lib \
		-L../../htmlparser/lib \
		-L../../cssparser/lib \
		-L../../vhtmlparser/lib \
		-L../../ahtmlparser/lib \
		-L../../extractor/lib \
		-L../../utils/lib \
		-L../../test/depends/log/lib32 \
		-L../../depends/CCharset/lib32 \
		-L../../depends/chardet/lib32 \
		-lextractorparser \
		-lmarkparser \
		-lahtmlparser \
		-lvhtmlparser \
		-lhtmlparser \
		-lcssparser \
		-lutils \
		-llog -llog4cpp	\
		-lccharset -lchardet \
		-lssl -lpthread
else
LIB  = \
		-L./ \
		-L../lib \
		-L../../depends/CCharset/lib64 \
		-L../../depends/chardet/lib64 \
		-L../../depends/cssserver/edblib \
		-L../../depends/log/lib64 \
		-L../../htmlparser/lib \
		-L../../cssparser/lib \
		-L../../vhtmlparser/lib \
		-L../../ahtmlparser/lib \
		-L../../extractor/lib \
		-L../../utils/lib \
		-lextractorparser \
		-lmarkparser \
		-lahtmlparser \
		-lvhtmlparser \
		-lhtmlparser \
		-lcssparser \
		-lutils \
		-lccharset -lchardet \
		-ledbbatch -ledb \
		-llog -llog4cpp	\
		-lssl -lpthread -lrt 
endif

COMMON_DEFINES = -D_INDEX_IO_DEBUG -DLINUX -D_REENTERANT -DNEED_OUT_CSS
CFLAGS= -fPIC  -Wall $(COMMON_DEFINES) $(INCLUDE)

ifeq ($(o),2)
CXX=g++ -O2 -pipe $(CFLAGS)
CC = g++ -Wall -fPIC -DLINUX -D_REENTERANT -D_FILE_OFFSET_BITS=64 -O2 -o
else ifeq ($(o),3)
CXX=g++ -O3 -pipe $(CFLAGS)
CC = g++ -Wall -fPIC -DLINUX -D_REENTERANT -D_FILE_OFFSET_BITS=64 -O3 -o
else
CXX=g++ -g -pipe $(CFLAGS)
CC = g++ -Wall -fPIC -DLINUX -D_REENTERANT -D_FILE_OFFSET_BITS=64 -g -o
endif

EASOU_TEST_SRC:=$(wildcard *.cpp)

EASOU_TEST_OBJ:=$(EASOU_TEST_SRC:%.cpp=%.o)

TESTS=mark_parser_test

all: $(TESTS) clear

clear:
	/bin/rm -rf *.o
	
clean:
	/bin/rm -f *.o
	/bin/rm -f $(TESTS)
	
rebuild : clean all

mark_parser_test: $(EASOU_TEST_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB) 

INCLUDE= \
	-I./ \
	-I../include	\
	-I../../utils/include \
	-I../../depends/CCharset/include \
	-I../../depends/chardet/include \
	-I../../depends/publish_1.0.1/include \
	-I../../depends/log/include	\
	-I../../depends/log/include/log4cpp

LIB= \
	-L./ \
	-L../lib \
	-L../../utils/lib \
	-L../../depends/log/lib64 \
	-L../../depends/CCharset/lib64 \
	-L../../depends/chardet/lib64 \
	-lccharset -lchardet \
	-llog -llog4cpp	\
	-lssl -lpthread	\
	-lhtmlparser	\
	-lutils
	
COMMON_DEFINES = -DLINUX -D_REENTERANT
CFLAGS= -fPIC  -Wall $(COMMON_DEFINES) $(INCLUDE)

CXX=g++ -g -pipe $(CFLAGS)
CC = g++ -Wall -fPIC -DLINUX -D_REENTERANT -D_FILE_OFFSET_BITS=64 -g -o

EASOU_TEST_SRC:=easou_html_test.cpp DownLoad.cpp

EASOU_TEST_OBJ:=$(EASOU_TEST_SRC:%.cpp=%.o)

TESTS=html_parser_test

all	: $(TESTS) clear

clear:
	/bin/rm -rf *.o
	
clean:
	/bin/rm -f *.o
	/bin/rm -f $(TESTS)
	
rebuild : clean all

html_parser_test: $(EASOU_TEST_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB) 


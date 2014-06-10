WORKROOT = ../../../
	
LIBPATH=$(WORKROOT)publish_1.0.1

INCLUDE = \
		-I./ \
		-I../include \
		-I../../htmlparser/include \
		-I../../utils/include \
		-I$(LIBPATH)/include	\
		-I$(LIBPATH)/log/include	\
		-I$(LIBPATH)/log/include/log4cpp
		
ifeq ($(m),32)
LIB  = \
		-L./ \
		-L../lib \
		-L../../htmlparser/lib \
		-L../../utils/lib \
		-L../../depends/log/lib32 \
		-lcssparser \
		-lhtmlparser \
		-lutils \
		-llog -llog4cpp	\
		-lssl -lpthread
else
LIB  = \
		-L./ \
		-L../lib \
		-L../../htmlparser/lib \
		-L../../utils/lib \
		-L$(LIBPATH)/log/lib \
		-L$(LIBPATH)/log/lib/log4cpp \
		-lcssparser \
		-lhtmlparser \
		-lutils \
		-llog -llog4cpp	\
		-lssl -lpthread
endif

COMMON_DEFINES = -D_INDEX_IO_DEBUG -DLINUX -D_REENTERANT
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

TESTS = css_parser_test

all	: $(TESTS) clear

clear:
	/bin/rm -rf *.o

clean:
	/bin/rm -rf *.o
	/bin/rm -rf $(TESTS)

rebuild : clean all

css_parser_test : $(EASOU_TEST_OBJ) 
	$(CC) $@ $^ $(INCLUDE) $(LIB) 
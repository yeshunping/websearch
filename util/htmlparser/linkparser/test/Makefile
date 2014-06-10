INCLUDE = \
		-I./ \
		-I../include \
		-I../../bbsparser/include/ \
		-I../../depends/pcre-8.31/include/ \
		-I../../depends/CCharset/include/ \
		-I../../depends/chardet/include/ \
		-I../../depends/cssserver/include/ \
		-I../../htmlparser/include \
		-I../../cssparser/include \
		-I../../vhtmlparser/include \
		-I../../ahtmlparser/include \
		-I../../markparser/include \
		-I../../utils/include \
		-I../../extractor/include \
		-I../../depends/include	\
		-I../../depends/log/include	\
		-I../../depends/log/include/log4cpp \
		-I../../depends/publish_1.0.1/include/ \
		-I../../depends/mysqlc/include/ \
		-I../../depends/libiconv/include/
		
ifeq ($(m),32)
LIB  = \
		-L./ \
		-L../output/lib \
		-L../../bbsparser/lib/ \
		-L../../depends/pcre-8.31/lib/ \
		-L../../depends/cssserver/lib32/ \
		-L../../htmlparser/lib \
		-L../../cssparser/lib \
		-L../../vhtmlparser/lib \
		-L../../ahtmlparser/lib \
		-L../../markparser/lib \
		-L../../extractor/lib \
		-L../../utils/lib \
		-L../../test/depends/log/lib32 \
		-L../../depends/CCharset/lib32 \
		-L../../depends/chardet/lib32 \
		-llinkparser \
		-lbbsparser \
		-lextractorparser \
		-lmarkparser \
		-lahtmlparser \
		-lvhtmlparser \
		-lhtmlparser \
		-lcssparser \
		-lutils \
		-ledbbatch \
		-ledb \
		-llog -llog4cpp	\
		-lccharset -lchardet \
		-lpcre \
		-lssl -lpthread
else
LIB  = \
		-L./ \
		-L../lib \
		-L../../bbsparser/lib/ \
		-L../../depends/pcre-8.31/lib/ \
		-L../../depends/CCharset/lib64/ \
		-L../../depends/chardet/lib64/ \
		-L../../depends/cssserver/edblib/ \
		-L../../depends/log/lib64/ \
		-L../../depends/libiconv/lib/ \
		-L../../depends/mysqlc/lib/ \
		-L../../htmlparser/lib \
		-L../../cssparser/lib \
		-L../../vhtmlparser/lib \
		-L../../ahtmlparser/lib \
		-L../../markparser/lib \
		-L../../extractor/lib \
		-L../../utils/lib \
		-llinkparser \
		-lbbsparser \
		-lextractorparser \
		-lmarkparser \
		-lahtmlparser\
		-lvhtmlparser \
		-lhtmlparser \
		-lcssparser \
		-lutils \
		-ledbbatch \
		-ledb \
		-llog -llog4cpp	\
		-lpcre \
		-lssl -lpthread \
		-lrt \
		-Wl,-Bstatic -lmysqlclient -liconv -lccharset -lchardet -Wl,-Bdynamic
endif

ifeq ($(css),yes)
COMMON_DEFINES = -D_INDEX_IO_DEBUG -DLINUX -D_REENTERANT -DNEED_OUT_CSS -DDEBUG_INFO
else
COMMON_DEFINES = -D_INDEX_IO_DEBUG -DLINUX -D_REENTERANT
endif		  

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

EASOU_TEST_OBJ:=easou_linkparser_test.o DownLoad.o
EASOU_TEST1_OBJ:=extract_img_infos.o DownLoad.o ../../test/area_visual_tool.o
EASOU_TEST2_OBJ:=img_stable.o
EASOU_TEST3_OBJ:=extract_news_img.o DownLoad.o ../../test/area_visual_tool.o

TESTS=linkparser_test
TESTS1=extract_img_infos
TESTS2=img_stable
TESTS3=extract_news_img

all: $(TESTS) $(TESTS1) $(TESTS2) $(TESTS3) clear

clear:
	/bin/rm -rf *.o
	
clean:
	/bin/rm -f *.o
	/bin/rm -f $(TESTS) $(TESTS1) $(TESTS2) $(TESTS3)
	
rebuild : clean all

linkparser_test: $(EASOU_TEST_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB) 

$(TESTS1): $(EASOU_TEST1_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB)
$(TESTS2): $(EASOU_TEST2_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB)
$(TESTS3): $(EASOU_TEST3_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB)

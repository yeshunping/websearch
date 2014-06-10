WORKROOT = ../../../

ifeq ($(MAC),64)
	LIB2=$(WORKROOT)/lib2-64/
else
	LIB2=$(WORKROOT)/lib2/
endif

VAREA_MARK_MODULE = ../varea-mark

ifeq ($(VAREA_MARK_MODULE), $(wildcard $(VAREA_MARK_MODULE)))
	VAREA_MARK_EXIST = 1
else
	VAREA_MARK_EXIST = 0
endif

ifeq ($(VAREA_MARK_EXIST),1)
INCLUDE_VAREA_MARK = -I../varea-mark
LOCATE_VAREA_MARK = -L../varea-mark
LINK_VAREA_MARK = -lvareamark
DEFINE_VAREA_MARK = -D_VAREA_MARK_EXIST_
endif

INCLUDE = -I./ \
		  -I../utils/include \
		  -I../htmlparser/include \
		  -I../cssparser/include \
		  -I../vhtmlparser/include/ \
		  -I../ahtmlparser/include/ \
		  -I../markparser/include/ \
		  -I../extractor/include/ \
		  -I../pagetype/include/ \
		  -I../bbsparser/include \
		  -I../depends/pcre-8.31/include/ \
		  -I../depends/log/include/ \
		  -I../depends/publish_1.0.1/include/ 
LDFLAGS  = -L./ \
		   -L../bbsparser \
		   -L../extractor/lib/  \
		   -L../vhtmlparser/lib/  \
		   -L../markparser/lib/ \
		   -L../ahtmlparser/lib/ \
		   -L../vhtmlparser/lib/ \
		   -L../cssparser/lib/ \
		   -L../htmlparser/lib/  \
		   -L../utils/lib/ \
		   -L../depends/log/lib64/ \
		   -lbbsparser \
		   -lextractorparser \
		   -lmarkparser \
		   -lahtmlparser \
		   -lvhtmlparser \
		   -lcssparser \
		   -lhtmlparser \
		   -lutils \
		   -lpcre \
		   -llog \
		   -llog4cpp \
		   -lpthread 

ifeq "$(O)" "debug"
	COMMON_DEFINES = -DLINUX -D_REENTERANT -D_FILE_OFFSET_BITS=64 -fPIC -D_DUMP_ME_ $(DEFINE_VAREA_MARK)
else
	COMMON_DEFINES = -DLINUX -D_REENTERANT -D_FILE_OFFSET_BITS=64  -fPIC -Werror $(DEFINE_VAREA_MARK)
endif

ifeq ($(o),2)
	DEFINES=$(COMMON_DEFINES) -DNDEBUG
	CPPFLAGS= -O2 -Wall $(DEFINES) $(INCLUDE)
	CFLAGS= -O2 -Wall $(DEFINES) $(INCLUDE)  
	CXXFLAGS= -O2 -Wall $(DEFINES) $(INCLUDE)
else ifeq ($(o),3)
	DEFINES=$(COMMON_DEFINES) -DNDEBUG
	CPPFLAGS= -O3 -Wall $(DEFINES) $(INCLUDE)
	CFLAGS= -O3 -Wall $(DEFINES) $(INCLUDE)  
	CXXFLAGS= -O3 -Wall $(DEFINES) $(INCLUDE)
else
	DEFINES=$(COMMON_DEFINES)
	CPPFLAGS= -g -Wall $(DEFINES) $(INCLUDE)
	CFLAGS= -g -Wall $(DEFINES) $(INCLUDE)  
	CXXFLAGS= -g -Wall $(DEFINES) $(INCLUDE)
endif

ifeq "$(MAKECMDGOALS)" "notime"
	CPPFLAGS:=$(CPPFLAGS) -D__TIME__="\"no_time\"" -D__DATE__="\"no_date\"" -g0 
	CFLAGS:=$(CFLAGS) -D__TIME__="\"no_time\"" -D__DATE__="\"no_date\"" -g0 
endif

CC  = g++ $(CPPFLAGS) 
CPP = gcc
CXX = gcc 
AR  = ar
#=========================================================================
#LIBS=liblinkparser.a liblinkparser.so
LIBS=liblinkparser.a
RELEASE_LIBS=
TEST_LIBS=
EXECUTABLE = 
TEST_EXEC =

notime : all 

all	: $(LIBS) $(EXECUTABLE) $(TEST_EXEC) output

clean :
	/bin/rm -f *.o *~ *.so
	/bin/rm -f $(EXECUTABLE) $(TEST_EXEC) $(LIBS)

rebuild : clean all

release : rebuild
	cp liblinkparser.a ../../pub/linkparser

output:
	/bin/rm -f *.o *~
	cp -f ./liblinkparser.a ./lib/
	cp -f ./easou_link.h ./include/
	cp -f ./easou_extract_blogtime.h ./include/
	cp -f ./easou_link_mark.h ./include/
	cp -f ./easou_link_timematch.h ./include/
	cp -f ./easou_link_common.h ./include/
	rm lib*.a

liblinkparser.a : easou_link.o easou_extract_blogtime.o easou_linktype_quotation.o easou_linktype_cont_interlude.o \
	easou_linktype_bbsre.o easou_link_timematch.o easou_linktype_blogre.o easou_linktype_blogmain.o \
	easou_link_common.o easou_linktype_nav.o easou_linktype_image.o easou_linktype_friend.o easou_linktype_iframe.o easou_linktype_friendex.o \
	easou_linktype_selfhelp.o easou_linktype_invalid_and_control.o easou_linktype_css.o \
	easou_linktype_img_src.o easou_linktype_embed_src.o easou_linktype_copyright.o easou_linktype_nofollow.o	easou_linktype_mypos.o easou_link_area.o \
	easou_linktype_hidden.o easou_linktype_text.o
	ar rcv $@ $^

easou_link.o: easou_link.cpp easou_link.h
	$(CC) -o easou_link.o -c easou_link.cpp $(INCLUDE)

vhplink.so : vhplinkso.cpp
	$(CC) -fPIC -g -O2 -shared -Wno-deprecated -o vhplink.so vhplinkso.cpp vhp_link.o html_extract_blogtime.o vhp_quotation_type.o \
		vhp_cont_interlude_type.o \
		vhp_bbsre_type.o timematch.o vhp_blogre_type.o vhp_blogmain_type.o \
		vhp_link_common.o vhp_nav_type.o vhp_image_type.o vhp_friend_type.o vhp_friendex_type.o \
		vhp_selfhelp_type.o vhp_invalid_and_control_type.o vhp_css_type.o \
		vhp_img_src_type.o vhp_embed_src_type.o vhp_copyright.o vhp_nofollow.o  vhp_mypos.o vhplk_area.o \
		vhp_hidden_type.o vhp_text_link.o vhp_page_border.o $(LDFLAGS) -lc 

INCLUDE = -I./ \
		-I./thrift \
		-I../utils/include\
		-I../htmlparser/include \
		-I../cssparser/include \
		-I../vhtmlparser/include \
		-I../ahtmlparser/include \
		-I../markparser/include \
		-I../depends/publish_1.0.1/include \
		-I../depends/jansson-2.4/include \
		-I../depends/mysqlc/include \
		-I../depends/chardet/include \
		-I../depends/CCharset/include \
		-I../depends/log/include	\
		-I../depends/log/include/log4cpp \
		-I../depends/thrift-0.7.0/include/thrift/ \

COMMON_DEFINES = 	-DLINUX -D_REENTERANT
COMMON_DEFINES_DUMP = 	-DLINUX -D_REENTERANT -DDEBUG_INFO
COMMON_DEFINES_TIME = 	-DLINUX -D_REENTERANT -DDEBUG_TIME

CFLAGS= 	-Wall -fPIC $(COMMON_DEFINES) 	$(INCLUDE)
CFLAGS_DUMP= 	-Wall -fPIC $(COMMON_DEFINES_DUMP) $(INCLUDE)
CFLAGS_TIME= 	-Wall -fPIC $(COMMON_DEFINES_TIME) $(INCLUDE)

EASOU_EXTRA_SRC:=$(wildcard *.cpp)
EASOU_THRIFT_SRC:=thrift/StructData_constants.cpp thrift/StructData_types.cpp

EASOU_EXTRA_OBJ:=$(EASOU_EXTRA_SRC:%.cpp=%.o)
EASOU_EXTRA_OBJ_D:=$(EASOU_EXTRA_SRC:%.cpp=%_d.o)
EASOU_EXTRA_OBJ_DUMP:=$(EASOU_EXTRA_SRC:%.cpp=%_dump.o)
EASOU_EXTRA_OBJ_TIME:=$(EASOU_EXTRA_SRC:%.cpp=%_time.o)
EASOU_THRIFT_OBJ:=$(EASOU_THRIFT_SRC:%.cpp=%.o)

LIBS=libextractorparser.a
LIBS_D=libextractorparser_debug.a
LIBS_DUMP=libextractorparser_dump.a
LIBS_TIME=libextractorparser_time.a

RELEASE_HEADER = easou_extractor_stdata.h easou_extractor_stdata_video.h easou_extractor_stdata_bk.h easou_extractor_stdata_db.h easou_extractor_template.h easou_extractor_com.h easou_extractor_content.h easou_extractor_mypos.h easou_extractor_title.h 

%.o : %.cpp
	g++ -O2 -o $@ $< -c $(CFLAGS)
%_d.o : %.cpp
	g++ -g -o $@ $< -c $(CFLAGS)
%_dump.o : %.cpp
	g++ -g -o $@ $< -c $(CFLAGS_DUMP)
%_time.o : %.cpp
	g++ -O2 -o $@ $< -c $(CFLAGS_TIME)
	
all	: thrift_task $(LIBS) $(LIBS_D) $(LIBS_DUMP) $(LIBS_TIME) output 
debug	: $(LIBS_D) output
dump	: $(LIBS_DUMP) output
time	: $(LIBS_TIME) output
release : $(LIBS) output

$(LIBS) : 	$(EASOU_EXTRA_OBJ) $(EASOU_THRIFT_OBJ)
	ar rcv $@ $^
$(LIBS_D) : 	$(EASOU_EXTRA_OBJ_D) $(EASOU_THRIFT_OBJ)
	ar rcv $@ $^
$(LIBS_DUMP) : 	$(EASOU_EXTRA_OBJ_DUMP) $(EASOU_THRIFT_OBJ)
	ar rcv $@ $^
$(LIBS_TIME) : 	$(EASOU_EXTRA_OBJ_TIME) $(EASOU_THRIFT_OBJ)
	ar rcv $@ $^
thrift_task : $(EASOU_THRIFT_OBJ)

output : 
	cp -f $(RELEASE_HEADER) include/
	cp -f *.a lib/
	/bin/rm -rf *.o *.a *.so
	
clean:
	/bin/rm -f *.o *.a include/*.h lib/*.a
	/bin/rm -f thrift/*.o
	
rebuild : clean all

	

INCLUDE= \
	-I./ \
	-I../utils/include \
	-I../depends/publish_1.0.1/include \
	-I../depends/log/include	\
	-I../depends/log/include/log4cpp 
	
COMMON_DEFINES = 	-DLINUX -D_REENTERANT
COMMON_DEFINES_DUMP = 	-DLINUX -D_REENTERANT -DDEBUG_INFO
COMMON_DEFINES_TIME = 	-DLINUX -D_REENTERANT -DDEBUG_TIME

CFLAGS=  -Wall -fPIC $(COMMON_DEFINES) $(INCLUDE)
CFLAGS_DUMP=  -Wall -fPIC $(COMMON_DEFINES_DUMP) $(INCLUDE)
CFLAGS_TIME=  -Wall -fPIC $(COMMON_DEFINES_TIME) $(INCLUDE)

EASOU_HTML_SRC:=$(wildcard *.cpp)

EASOU_HTML_OBJ:=$(EASOU_HTML_SRC:%.cpp=%.o)
EASOU_HTML_OBJ_D:=$(EASOU_HTML_SRC:%.cpp=%_d.o)
EASOU_HTML_OBJ_DUMP:=$(EASOU_HTML_SRC:%.cpp=%_dump.o)
EASOU_HTML_OBJ_TIME:=$(EASOU_HTML_SRC:%.cpp=%_time.o)

LIBS=libhtmlparser.a
LIBS_D=libhtmlparser_debug.a
LIBS_DUMP=libhtmlparser_dump.a
LIBS_TIME=libhtmlparser_time.a

%.o : %.cpp
	g++ -O2 -o $@ $< -c $(CFLAGS)
%_d.o : %.cpp
	g++ -g -o $@ $< -c $(CFLAGS)
%_dump.o : %.cpp
	g++ -O2 -o $@ $< -c $(CFLAGS_DUMP)
%_time.o : %.cpp
	g++ -O2 -o $@ $< -c $(CFLAGS_TIME)
	
all: $(LIBS) $(LIBS_D) $(LIBS_DUMP) $(LIBS_TIME) output
debug	: $(LIBS_D) output
dump	: $(LIBS_DUMP) output
time	: $(LIBS_TIME) output
release	: $(LIBS) output

$(LIBS): $(EASOU_HTML_OBJ)
	ar rcv $@ $^
$(LIBS_D): $(EASOU_HTML_OBJ_D)
	ar rcv $@ $^
$(LIBS_DUMP): $(EASOU_HTML_OBJ_DUMP)
	ar rcv $@ $^
$(LIBS_TIME): $(EASOU_HTML_OBJ_TIME)
	ar rcv $@ $^
	
output : 
	cp -f *.h include/
	cp -f *.a lib/
	/bin/rm -rf *.o *.a *.so
	
clean:
	rm -f *.o *.a *.so include/*.h lib/*.a
	
rebuild : clean all


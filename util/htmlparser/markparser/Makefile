INCLUDE = -I./ \
		-I../utils/include\
		-I../htmlparser/include \
		-I../cssparser/include \
		-I../vhtmlparser/include \
		-I../ahtmlparser/include \
		-I../depends/publish_1.0.1/include \
		-I../depends/log/include	\
		-I../depends/log/include/log4cpp
		  
COMMON_DEFINES =	-DLINUX -D_REENTERANT
COMMON_DEFINES_DUMP =	-DLINUX -D_REENTERANT -DDEBUG_INFO
COMMON_DEFINES_TIME =	-DLINUX -D_REENTERANT -DDEBUG_TIME

CFLAGS=		-Wall -fPIC $(COMMON_DEFINES)	$(INCLUDE)
CFLAGS_DUMP=	-Wall -fPIC $(COMMON_DEFINES_DUMP) $(INCLUDE)
CFLAGS_TIME=	-Wall -fPIC $(COMMON_DEFINES_TIME) $(INCLUDE)

EASOU_MARK_SRC:=$(wildcard *.cpp)
EASOU_FUNCTION_SRC:=$(wildcard mark_function/*.cpp)

EASOU_MARK_OBJ:=$(EASOU_MARK_SRC:%.cpp=%.o)
EASOU_FUNCTION_OBJ:=$(EASOU_FUNCTION_SRC:%.cpp=%.o)

EASOU_MARK_OBJ_D:=$(EASOU_MARK_SRC:%.cpp=%_d.o)
EASOU_FUNCTION_OBJ_D:=$(EASOU_FUNCTION_SRC:%.cpp=%_d.o)

EASOU_MARK_OBJ_TIME:=$(EASOU_MARK_SRC:%.cpp=%_time.o)
EASOU_FUNCTION_OBJ_TIME:=$(EASOU_FUNCTION_SRC:%.cpp=%_time.o)

EASOU_MARK_OBJ_DUMP:=$(EASOU_MARK_SRC:%.cpp=%_dump.o)
EASOU_FUNCTION_OBJ_DUMP:=$(EASOU_FUNCTION_SRC:%.cpp=%_dump.o)

LIBS=libmarkparser.a
LIBS_D=libmarkparser_debug.a
LIBS_TIME=libmarkparser_time.a
LIBS_DUMP=libmarkparser_dump.a

RELEASE_HEADER = easou_mark_parser.h easou_mark_baseinfo.h easou_mark_markinfo.h easou_mark_switch.h \
				easou_mark_srctype.h easou_mark_func.h easou_mark_sem.h easou_mark_conf.h easou_mark_inner.h

%.o : %.cpp
	g++ -O2 -o $@ $< -c $(CFLAGS)
%_d.o : %.cpp
	g++ -g -o $@ $< -c $(CFLAGS)
%_time.o : %.cpp
	g++ -O2 -o $@ $< -c $(CFLAGS_TIME)
%_dump.o : %.cpp
	g++ -g -o $@ $< -c $(CFLAGS_DUMP)

all	: $(LIBS) $(LIBS_D) $(LIBS_TIME) dump output 
time	: $(LIBS_TIME) output
debug	: $(LIBS_D) output
dump	: $(LIBS_DUMP) output
release : $(LIBS) output

$(LIBS) : 	$(EASOU_MARK_OBJ) $(EASOU_FUNCTION_OBJ)
	ar rcv $@ $^

$(LIBS_D) : 	$(EASOU_MARK_OBJ_D) $(EASOU_FUNCTION_OBJ_D)
	ar rcv $@ $^

$(LIBS_TIME) : 	$(EASOU_MARK_OBJ_TIME) $(EASOU_FUNCTION_OBJ_TIME)
	ar rcv $@ $^

$(LIBS_DUMP) : 	$(EASOU_MARK_OBJ_DUMP) $(EASOU_FUNCTION_OBJ_DUMP)
	ar rcv $@ $^

output : 
	cp -f $(RELEASE_HEADER) include/
	cp -f *.a lib/
	/bin/rm -rf *.o *.a *.so
	
clean:
	/bin/rm -rf *.o
	/bin/rm -rf mark_function/*.o
	/bin/rm -f *.a
	/bin/rm -rf include/*.h
	/bin/rm -rf lib/*.a
	
rebuild : clean all


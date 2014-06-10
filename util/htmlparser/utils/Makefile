INCLUDE=-I./ \
-I../depends/publish_1.0.1/include \
-I../depends/log/include	\
-I../depends/log/include/log4cpp

COMMON_DEFINES = 	-DLINUX -D_REENTERANT
COMMON_DEFINES_TIME = 	-DLINUX -D_REENTERANT -DDEBUG_TIME
COMMON_DEFINES_DUMP = 	-DLINUX -D_REENTERANT -DDEBUG_INFO

CFLAGS= 	-Wall -fPIC $(COMMON_DEFINES) $(INCLUDE)
CFLAGS_TIME= 	-Wall -fPIC $(COMMON_DEFINES_TIME) $(INCLUDE)
CFLAGS_DUMP= 	-Wall -fPIC $(COMMON_DEFINES_DUMP) $(INCLUDE)

%.o : %.cpp
	g++ -O2 -o $@ $< -c $(CFLAGS)
%_d.o : %.cpp
	g++ -g -o $@ $< -c $(CFLAGS)
%_time.o : %.cpp
	g++ -O2 -o $@ $< -c $(CFLAGS_TIME)
%_dump.o : %.cpp
	g++ -O2 -o $@ $< -c $(CFLAGS_DUMP)
	
EASOU_UTIL_SRC:=$(wildcard *.cpp)

EASOU_UTIL_OBJ:=$(EASOU_UTIL_SRC:%.cpp=%.o)
EASOU_UTIL_OBJ_D:=$(EASOU_UTIL_SRC:%.cpp=%_d.o)
EASOU_UTIL_OBJ_TIME:=$(EASOU_UTIL_SRC:%.cpp=%_time.o)
EASOU_UTIL_OBJ_DUMP:=$(EASOU_UTIL_SRC:%.cpp=%_dump.o)

LIBS=libutils.a
LIBS_D=libutils_debug.a
LIBS_TIME=libutils_time.a
LIBS_DUMP=libutils_dump.a

all: $(LIBS) $(LIBS_D) $(LIBS_TIME) $(LIBS_DUMP) output
debug: $(LIBS_D) output
dump: $(LIBS_DUMP) output
time: $(LIBS_TIME) output

$(LIBS): $(EASOU_UTIL_OBJ)
	ar rcv $@ $^
$(LIBS_D): $(EASOU_UTIL_OBJ_D)
	ar rcv $@ $^
$(LIBS_TIME): $(EASOU_UTIL_OBJ_TIME)
	ar rcv $@ $^
$(LIBS_DUMP): $(EASOU_UTIL_OBJ_DUMP)
	ar rcv $@ $^

output : 
	cp -f *.h include/
	cp -f *.a lib/
	/bin/rm -rf *.o *.a 
	
clean:
	/bin/rm -f *.o *.a include/*.h lib/*.a
	
rebuild : clean all


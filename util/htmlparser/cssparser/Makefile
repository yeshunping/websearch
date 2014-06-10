INCLUDE = \
		-I./ \
		-I../htmlparser/include \
		-I../utils/include \
		-I../depends/publish_1.0.1/include	\
		-I../depends/log/include	\
		-I../depneds/log/include/log4cpp
		
COMMON_DEFINES = 	-DLINUX -D_REENTERANT
COMMON_DEFINES_DUMP = 	-DLINUX -D_REENTERANT -DDEBUG_INFO
COMMON_DEFINES_TIME = 	-DLINUX -D_REENTERANT -DDEBUG_TIME

CFLAGS=  	-Wall -fPIC $(COMMON_DEFINES) $(INCLUDE)
CFLAGS_DUMP=  	-Wall -fPIC $(COMMON_DEFINES_DUMP) $(INCLUDE)
CFLAGS_TIME=  	-Wall -fPIC $(COMMON_DEFINES_TIME) $(INCLUDE)

EASOU_CSS_SRC:=$(wildcard *.cpp)

EASOU_CSS_OBJ:=$(EASOU_CSS_SRC:%.cpp=%.o)
EASOU_CSS_OBJ_D:=$(EASOU_CSS_SRC:%.cpp=%_d.o)
EASOU_CSS_OBJ_TIME:=$(EASOU_CSS_SRC:%.cpp=%_time.o)
EASOU_CSS_OBJ_DUMP:=$(EASOU_CSS_SRC:%.cpp=%_dump.o)

LIBS=libcssparser.a 
LIBS_D=libcssparser_debug.a 
LIBS_TIME=libcssparser_time.a 
LIBS_DUMP=libcssparser_dump.a 

notime : all

%.o : %.cpp
	g++ -O2 -o $@ $< -c $(CFLAGS)
%_d.o : %.cpp
	g++ -g -o $@ $< -c $(CFLAGS)
%_time.o : %.cpp
	g++ -O2 -o $@ $< -c $(CFLAGS_TIME)
%_dump.o : %.cpp
	g++ -O2 -o $@ $< -c $(CFLAGS_DUMP)
	
all	: $(LIBS) $(LIBS_D) $(LIBS_TIME) $(LIBS_DUMP) output
time	: $(LIBS_TIME) output
dump	: $(LIBS_DUMP) output
debug	: $(LIBS_D) output
release : $(LIBS) output

$(LIBS) : $(EASOU_CSS_OBJ) 
	ar rcv $@ $^
$(LIBS_D) : $(EASOU_CSS_OBJ_D) 
	ar rcv $@ $^
$(LIBS_TIME) : $(EASOU_CSS_OBJ_TIME) 
	ar rcv $@ $^
$(LIBS_DUMP) : $(EASOU_CSS_OBJ_DUMP) 
	ar rcv $@ $^
	
output :
	cp *.a lib/
	cp *.h include/
	/bin/rm -rf *.o *.a *.so

clean:
	/bin/rm -rf *.o *.a include/*.h lib/*.a

rebuild : clean all

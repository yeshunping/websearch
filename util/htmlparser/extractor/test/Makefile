include ../../linkparser/test/basic.mk

EASOU_TEST_SRC:=tmplt_single_test.cpp ../../test/tool_global.cpp
EASOU_TMPLT_STABLE_SRC:=tmplt_stable_test.cpp ../../test/tool_global.cpp
STDATA_DESERIAL_SRC:=stdata_deserial_test.cpp ../../test/tool_global.cpp
STDATA_SINGLE_SRC:=stdata_single_test.cpp ../../test/tool_global.cpp
STDATA_STABLE_SRC:=stdata_stable_test.cpp ../../test/tool_global.cpp
TEST_RT_SRC:=extract.cpp ../../test/tool_global.cpp ../../test/area_visual_tool.cpp ../../vhtmlparser/test/vtree_print_utils.cpp
CONTENT_PIC_SRC:=content_pic.cpp ../../test/tool_global.cpp

STDATA_DESERIAL_OBJ:=$(STDATA_DESERIAL_SRC:%.cpp=%_d.o)
STDATA_SINGLE_OBJ:=$(STDATA_SINGLE_SRC:%.cpp=%_d.o)
STDATA_STABLE_OBJ:=$(STDATA_STABLE_SRC:%.cpp=%_d.o)
EASOU_TEST_OBJ:=$(EASOU_TEST_SRC:%.cpp=%.o)
EASOU_TEST_OBJ01:=$(EASOU_TEST_SRC:%.cpp=%_dump.o)
EASOU_TMPLT_STABLE_OBJ:=$(EASOU_TMPLT_STABLE_SRC:%.cpp=%.o)
EASOU_TEST12_OBJ:=$(EASOU_TEST1_SRC:%.cpp=%_prof.o)
EASOU_TEST2_OBJ:=$(EASOU_TEST1_SRC:%.cpp=%_trace.o)
EASOU_TEST3_OBJ:=$(EASOU_TEST1_SRC:%.cpp=%_tracesave.o)
TEST_RT_OBJ:=$(TEST_RT_SRC:%.cpp=%_d.o)
RT_SINGLE_DUMP_OBJ:=$(TEST_RT_SRC:%.cpp=%_d_rt.o)
MC_SINGLE_DUMP_OBJ:=$(TEST_RT_SRC:%.cpp=%_d_maincont.o)
NAV_SINGLE_DUMP_OBJ:=$(TEST_RT_SRC:%.cpp=%_d_nav.o)
MYPOS_SINGLE_DUMP_OBJ:=$(TEST_RT_SRC:%.cpp=%_d_mypos.o)
SUBTITLE_SINGLE_DUMP_OBJ:=$(TEST_RT_SRC:%.cpp=%_d_subtitle.o)
TURNPAGE_SINGLE_DUMP_OBJ:=$(TEST_RT_SRC:%.cpp=%_d_turnpage.o)
AREAPIC_SINGLE_DUMP_OBJ:=$(TEST_RT_SRC:%.cpp=%_d_areapic.o)
CONTENT_PIC_OBJ:=$(CONTENT_PIC_SRC:%.cpp=%_d.o)

%_d_rt.o : %.cpp
	g++ -g -o $@ $< -c $(INCLUDE) -DREALTITLE
%_d_maincont.o : %.cpp
	g++ -g -o $@ $< -c $(INCLUDE) -DMAIN_CONTENT
%_d_nav.o : %.cpp
	g++ -g -o $@ $< -c $(INCLUDE) -DNAVIGATION
%_d_mypos.o : %.cpp
	g++ -g -o $@ $< -c $(INCLUDE) -DMYPOS
%_d_subtitle.o : %.cpp
	g++ -g -o $@ $< -c $(INCLUDE) -DSUBTITLE
%_d_turnpage.o : %.cpp
	g++ -g -o $@ $< -c $(INCLUDE) -DTURNPAGE
%_d_areapic.o : %.cpp
	g++ -g -o $@ $< -c $(INCLUDE) -DAREA_PIC
%_prof.o : %.cpp
	g++ -g -DDEBUG_TIME -DNEWS_TEST -D_DEBUG_PROFILER_ -o $@ $< -c $(INCLUDE)
%_dump.o : %.cpp
	g++ -g -DDEBUG_INFO -DNEWS_TEST -o $@ $< -c $(INCLUDE)
%_trace.o : %.cpp
	g++ -g -DDEBUG_TIME -DNEWS_TEST -DTRACE_URL -o $@ $< -c $(INCLUDE)
%_tracesave.o : %.cpp
	g++ -g -DDEBUG_TIME -DNEWS_TEST -DTRACE_URL -DTRACE_SAVE -o $@ $< -c $(INCLUDE)

TESTS=tmplt_single_test
TESTS01=tmplt_single_test_time
TESTS02=tmplt_single_test_dump
TMPLT_STABLE=tmplt_stable_test
TEST11=tmplt_stable_time
TEST12=tmplt_stable_prof
TEST2=tmplt_stable_test_trace
TEST3=tmplt_stable_test_tracesave
TEST_RT1=realtitle_single_savecss
RT_SINGLE_DUMP=realtitle
MC_SINGLE_DUMP=maincont
NAV_SINGLE_DUMP=navigation
MYPOS_SINGLE_DUMP=mypos
SUBTITLE_SINGLE_DUMP=subtitle
TURNPAGE_SINGLE_DUMP=turnpage
AREAPIC_SINGLE_DUMP=areapic
CONTENT_PIC=content_pic
STDATA_DESERIAL=stdata_deserial
STDATA_SINGLE=stdata
STDATA_STABLE=stdata_stable

all: $(STDATA_SINGLE) $(STDATA_DESERIAL) $(STDATA_STABLE) \
	$(TMPLT_STABLE) $(TEST_RT) $(RT_SINGLE_DUMP) $(MC_SINGLE_DUMP) \
	$(NAV_SINGLE_DUMP) $(MYPOS_SINGLE_DUMP) $(SUBTITLE_SINGLE_DUMP) $(TURNPAGE_SINGLE_DUMP) $(AREAPIC_SINGLE_DUMP) clear

clear:
	/bin/rm -rf *.o
	/bin/rm -f ../../test/*.o
	
clean:
	/bin/rm -f *.o
	/bin/rm -f ../../test/*.o
	/bin/rm -f ../../vhtmlparser/test/*.o
	/bin/rm -f $(TESTS) $(TESTS02) $(TMPLT_STABLE) $(TEST_RT) $(TEST_RT1) $(RT_SINGLE_DUMP)
	/bin/rm -f $(MC_SINGLE_DUMP) $(NAV_SINGLE_DUMP) $(SUBTITLE_SINGLE_DUMP) \
	$(TURNPAGE_SINGLE_DUMP) $(AREAPIC_SINGLE_DUMP) $(MYPOS_SINGLE_DUMP)
	/bin/rm -f $(STDATA_DESERIAL) $(STDATA_STABLE) $(STDATA_SINGLE) $(CONTENT_PIC)
	
rebuild : clean all

$(TESTS): $(EASOU_TEST_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DEBUG)

$(TEST_RT): $(TEST_RT_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DEBUG)
$(TEST_RT1): $(TEST_RT_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_SAVECSS)

$(RT_SINGLE_DUMP): $(RT_SINGLE_DUMP_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DUMP)

$(MC_SINGLE_DUMP): $(MC_SINGLE_DUMP_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DUMP)
	
$(NAV_SINGLE_DUMP): $(NAV_SINGLE_DUMP_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DUMP)
		
$(MYPOS_SINGLE_DUMP): $(MYPOS_SINGLE_DUMP_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DUMP)
	
$(SUBTITLE_SINGLE_DUMP): $(SUBTITLE_SINGLE_DUMP_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DUMP)
	
$(TURNPAGE_SINGLE_DUMP): $(TURNPAGE_SINGLE_DUMP_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DUMP)

$(AREAPIC_SINGLE_DUMP): $(AREAPIC_SINGLE_DUMP_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DUMP)
	
$(CONTENT_PIC): $(CONTENT_PIC_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DEBUG)

$(TESTS01): $(EASOU_TEST_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_TIME)
	
$(TESTS02): $(EASOU_TEST_OBJ01)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DUMP)

$(TMPLT_STABLE): $(EASOU_TMPLT_STABLE_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB)

$(TEST11): $(EASOU_TEST1_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_TIME)
	
$(TEST12): $(EASOU_TEST12_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_PROF)
	
$(TEST2): $(EASOU_TEST2_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DEBUG)
	
$(TEST3): $(EASOU_TEST3_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DEBUG)
	
$(STDATA_DESERIAL): $(STDATA_DESERIAL_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DEBUG)
$(STDATA_SINGLE): $(STDATA_SINGLE_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DEBUG)
$(STDATA_STABLE): $(STDATA_STABLE_OBJ)
	$(CC) $@ $^ $(INCLUDE) $(LIB_DIR) $(LIB_DEBUG)

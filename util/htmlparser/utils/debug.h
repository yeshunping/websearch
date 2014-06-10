/**
 * @Description	: 调试信息打印工具
 * @Author	: sue
 * @Date	: 2012-12-28
 */

#ifndef EASOU_DEBUG_H_
#define EASOU_DEBUG_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <string>
#include <map>

#define PARSER_DEBUG			"parser.debug"
#define CSS_DEBUG			"css.debug"
#define MARK_POS			"position.debug"
#define MARK_REALTITLE			"realtitle.debug"
#define MARK_CENTRAL			"central.debug"
#define MARK_MYPOS			"mypos.debug"
#define MARK_TEXT			"text.debug"
#define MARK_TIME			"time.debug"
#define MARK_SRCTYPE			"srctype.debug"
#define MARK_SUBTITLE			"subtitle.debug"
#define MARK_FRIEND			"friend.debug"
#define MARK_COPYRIGHT	"copyright.debug"
#define MARK_RELATELINK	"relatelink.debug"
#define MARK_TURNPAGE	"turnpage.debug"
#define MARK_AC		"articleContent.debug"
#define DIVIDING_AREA	"partition.debug"
#define MARK_POS	"position.debug"
#define CALC_VTREE	"calcvtree.debug"
#define TMPLT_EXTRACT	"tmplt_extract.debug"

#ifdef DEBUG_TIME

#include <sys/time.h>
#define TIMEDELTA_US(time1,time0) ((time1.tv_sec-time0.tv_sec)*1000*1000+(time1.tv_usec-time0.tv_usec))

extern std::map<std::string, uint64_t> g_debug_time_map;

#define timeinit() timeval endtv1,endtv2;uint64_t utimes;

#define timestart() {gettimeofday(&endtv1, NULL);}

#define timeend(name, desp) {\
	gettimeofday(&endtv2, NULL);\
	utimes = TIMEDELTA_US(endtv2, endtv1);\
	std::string tm_fname=name;\
	tm_fname+=":";\
	tm_fname+=desp;\
	std::map<std::string, uint64_t>::iterator _it_ = g_debug_time_map.find(tm_fname);\
	if (_it_ != g_debug_time_map.end()) {\
		_it_->second += utimes;\
	}else{\
		g_debug_time_map.insert(std::pair<std::string, uint64_t>(tm_fname, utimes));\
	}}

#define printtime(count) {\
	printf("all:%d\n", count);\
	for(std::map<std::string, uint64_t>::iterator _it_ = g_debug_time_map.begin();_it_!=g_debug_time_map.end();_it_++) {\
		const char* name = _it_->first.c_str();\
		uint64_t utimes = _it_->second;\
		printf("%30s avg:%-10.2fus\n", name, (float)utimes/(count));\
	}}

#else

#define timeinit()
#define timestart()
#define timeend(name, desp)
#define printtime(count)

#endif

#ifdef DEBUG_COUNT

struct debug_avg_item_t
{
	uint64_t allnum;
	uint64_t total;
};

extern std::map<std::string, uint64_t> g_debug_counter_map;
extern std::map<std::string, uint64_t> g_debug_max_map;
extern std::map<std::string, debug_avg_item_t> g_debug_avg_map;


#define counter_add(name, num) {\
				std::map<std::string, uint64_t>::iterator it = g_debug_counter_map.find(std::string(name));\
				if (it == g_debug_counter_map.end()) {\
					g_debug_counter_map.insert(std::pair<std::string, uint64_t>(std::string(name), (uint64_t)num));\
					it = g_debug_counter_map.find(name);\
				}\
				it->second = it->second + num;\
}

#define counter_add_condition(name, num, condition) {\
				if (condition) {\
					counter_add(name, num);\
				}\
}

#define print_counter(all) {\
				printf("all:%d\n", all);\
				for(std::map<std::string, uint64_t>::iterator _it_ = g_debug_counter_map.begin();_it_!=g_debug_counter_map.end();_it_++) {\
					const char* name = _it_->first.c_str();\
					uint64_t count = _it_->second;\
					printf("%30s avg:%-10.2f\n", name, (float)count/all);\
				}}

#define record_max(name, num) {\
				std::map<std::string, uint64_t>::iterator it = g_debug_max_map.find(std::string(name));\
				if (it == g_debug_max_map.end()) {\
					g_debug_max_map.insert(std::pair<std::string, uint64_t>(std::string(name), (uint64_t)num));\
					it = g_debug_max_map.find(name);\
				}\
				it->second = it->second < num ? num : it->second;\
}

#define record_avg(name, num) {\
				std::map<std::string, debug_avg_item_t>::iterator it = g_debug_avg_map.find(name);\
				if (it == g_debug_avg_map.end()) {\
					debug_avg_item_t avg_item;\
					avg_item.allnum = 0;\
					avg_item.total = 0;\
					g_debug_avg_map.insert(std::pair<std::string, debug_avg_item_t>(name, avg_item));\
					it = g_debug_avg_map.find(name);\
				}\
				it->second.total+=num;\
				it->second.allnum+=1;\
}

#define record_avg_condition(name, num, condition) {\
				if (condition) {\
					record_avg(name, num);\
				}\
}

#define print_max() {\
				for(std::map<std::string, uint64_t>::iterator _it_ = g_debug_max_map.begin();_it_!=g_debug_max_map.end();_it_++) {\
					const char* name = _it_->first.c_str();\
					uint64_t count = _it_->second;\
					printf("%30s max:%lu\n", name, count);\
				}}

#define print_avg() {\
				for(std::map<std::string, debug_avg_item_t>::iterator _it_ = g_debug_avg_map.begin();_it_!=g_debug_avg_map.end();_it_++) {\
					const char* name = _it_->first.c_str();\
					uint64_t allnum = _it_->second.allnum;\
					uint64_t count = _it_->second.total;\
					printf("%30s avg:%.2f\n", name, (float)count/allnum);\
				}}

#define init_debug_int(name)	int name=0;

#define debug_int_add(name, val) name+=val;

#else

#define counter_add(name, num)
#define counter_add_condition(name, num, condition)
#define print_counter(all)
#define record_max(name, num)
#define record_avg(name, num)
#define record_avg_condition(name, num, condition)
#define print_max()
#define print_avg()
#define init_debug_int(name)
#define debug_int_add(name, val)

#endif

#ifdef DEBUG_INFO

typedef struct _debug_item_t
{
	char *buf;
	char *pos;
	int left;
}debug_item_t;

extern std::map<std::string, debug_item_t*> g_debug_item_map;

#define DEBUG_BUF_SIZE (1<<23) //8M
//init for debug buffer
#define debuginfo_on(name) {\
	std::map<std::string, debug_item_t*>::iterator _it_ = g_debug_item_map.find(name);\
	if(_it_ == g_debug_item_map.end()){\
		char *p = (char*)malloc(DEBUG_BUF_SIZE);\
		assert(p != NULL);\
		debug_item_t* debug_item = new debug_item_t;\
		assert(debug_item!=NULL);\
		debug_item->buf = p;\
		debug_item->pos = p;\
		debug_item->left = DEBUG_BUF_SIZE - 1;\
		g_debug_item_map.insert(std::pair<std::string, debug_item_t*>(name, debug_item));\
	} else {\
		_it_->second->pos = _it_->second->buf;\
		_it_->second->left = DEBUG_BUF_SIZE - 1;\
		_it_->second->buf[0] = 0;\
	}}

//free for debug buffer
#define debuginfo_off(name) {\
	std::map<std::string, debug_item_t*>::iterator _it_ = g_debug_item_map.find(name);\
	if(_it_ != g_debug_item_map.end()){\
		debug_item_t* item = _it_->second;\
		g_debug_item_map.erase(_it_);\
		if(item->buf)\
			free(item->buf);\
		delete item;\
	}}

//add raw info
#define debugmsg(name,msg, ...) {\
	std::map<std::string, debug_item_t*>::iterator _it_ = g_debug_item_map.find(name);\
	if(_it_ != g_debug_item_map.end()) {\
		int ret = snprintf(_it_->second->pos,_it_->second->left, msg, ##__VA_ARGS__);\
		_it_->second->pos += ret;\
		_it_->second->left -= ret;\
		*(_it_->second->pos) = 0;\
	}}

//add debug info
#define debuginfo(name,info, ...) {\
	std::map<std::string, debug_item_t*>::iterator _it_ = g_debug_item_map.find(name);\
	if (_it_ != g_debug_item_map.end()) {\
		int ret = snprintf(_it_->second->pos, _it_->second->left, info, ##__VA_ARGS__);\
		int ret2 = snprintf(_it_->second->pos + ret, _it_->second->left - ret, " at %s(%d)-%s\n", __FILE__, __LINE__, __FUNCTION__);\
		_it_->second->pos+=(ret + ret2);\
		_it_->second->left -= (ret + ret2);\
		*(_it_->second->pos) = 0;\
	}}

#define debuginfo_cond(cond, name, info, ...) {\
	if(cond){\
		debuginfo(name,info, ##__VA_ARGS__);\
	}}

#define debugarray(name,array,count,info) {\
	std::map<std::string, debug_item_t*>::iterator _it_ = g_debug_item_map.find(name);\
	if (_it_ != g_debug_item_map.end()) {\
		int rett=snprintf(_it_->second->pos,_it_->second->left,"%s ",info);\
		_it_->second->pos+=rett;\
		_it_->second->left-=rett;\
		for(int i=0;i<count;i++){\
			int ret = snprintf(_it_->second->pos,_it_->second->left,"[%.2f]",array[i]);\
			_it_->second->pos+=ret;\
			_it_->second->left-=ret;\
		}\
		int ret2 = snprintf(_it_->second->pos, _it_->second->left, " at %s(%d)-%s\n", __FILE__, __LINE__, __FUNCTION__);\
		_it_->second->pos+=ret2;\
		_it_->second->left-=ret2;\
		*(_it_->second->pos) = 0;\
	}}

//save debug info to file
#define dumpdebug(name,fileName) {\
	std::map<std::string, debug_item_t*>::iterator _it_ = g_debug_item_map.find(name);\
	if (_it_ != g_debug_item_map.end()) {\
		FILE *fp = fopen(fileName, "w");\
		assert(fp!=NULL);\
		fprintf(fp, "%s", _it_->second->buf);\
		fclose(fp);\
		fp = NULL;\
	}}

#define dassert(exp) assert(exp)

#define dprintf(info, ...) printf(info, ##__VA_ARGS__)

#else

//do nothing in release version
#define debuginfo_on(name)
#define debuginfo_off(name)
#define debuginfo(name,info, ...)
#define debuginfo_cond(cond, name, info, ...)
#define debugarray(name,array,count,info)
#define dumpdebug(name,fileName)
#define debugmsg(name,msg, ...)
#define dassert(exp)
#define dprintf(info, ...)

#endif //PTDEBUG_INFO
#endif //PTDEBUG_H_

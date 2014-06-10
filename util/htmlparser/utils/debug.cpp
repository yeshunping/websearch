#include "easou_debug.h"

#ifdef DEBUG_INFO
std::map<std::string, debug_item_t*> g_debug_item_map;
#endif

#ifdef DEBUG_TIME
std::map<std::string, uint64_t> g_debug_time_map;
#endif

#ifdef DEBUG_COUNT
std::map<std::string, uint64_t> g_debug_counter_map;
std::map<std::string, uint64_t> g_debug_max_map;
std::map<std::string, debug_avg_item_t> g_debug_avg_map;
#endif

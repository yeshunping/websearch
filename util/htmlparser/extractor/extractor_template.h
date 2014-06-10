/*
 * easou_extractor_template.h
 *
 * Description:	提供模板抽取功能
 *
 *  Created on: 2013-6-13
 *      Author: sue
 */

#ifndef EASOU_EXTRACTOR_TEMPLATE_H_
#define EASOU_EXTRACTOR_TEMPLATE_H_

#include "simplehashmap.h"
#include "easou_html_tree.h"

#include <jansson.h>
#include <regex.h>
#include <time.h>

#define TMPLT_SLEN		20
#define TMPLT_MLEN		50
#define TMPLT_LLEN		1024

#define TMPLT_STATUS_VALID		1
#define TMPLT_STATUS_INVALID	-1

struct tmplt_nexturl_info_t
{
	char action[TMPLT_SLEN];
	char expression[TMPLT_LLEN];
	int expression_len;
	json_t* json;
	char url_regex[TMPLT_LLEN];
	regex_t* regex;
	bool valid;
};

typedef struct _tmplt_string_list_t
{
	char* str;
	int str_len;
	_tmplt_string_list_t* next;
} tmplt_string_list_t;

struct tmplt_regex_t
{
	char* regstr;
	int regstr_len;
	regex_t* reg;
	int reg_num;
};

typedef struct _tmplt_regex_list_t
{
	char* regstr;
	int regstr_len;
	regex_t* reg;
	_tmplt_regex_list_t* next;
} tmplt_regex_list_t;

typedef struct _tmplt_tagtype_list_t
{
	html_tag_type_t tag_type;
	_tmplt_tagtype_list_t* next;
} tmplt_tagtype_list_t;

typedef struct _tmplt_attr_list_t
{
	html_attr_type_t attr_type;
	char* name;
	int name_len;
	char* value;
	int value_len;
	_tmplt_attr_list_t* next;
} tmplt_attr_list_t;

union tmplt_path_item_t
{
	tmplt_tagtype_list_t* tag;
	tmplt_attr_list_t* attr;
	tmplt_tagtype_list_t* alter_tag;
	tmplt_attr_list_t* alter_attr;
	tmplt_regex_list_t* reg;
	tmplt_regex_t* source_reg;
};

enum tmplt_json_action_enum
{
	TMPLT_JACTION_UNKNOWN = 0, //
	TMPLT_JACTION_SOURCE_REG, //
	TMPLT_JACTION_REG, //
	TMPLT_JACTION_TAG, //
	TMPLT_JACTION_ATTR, //
	TMPLT_JACTION_ALTER_TAG, //
	TMPLT_JACTION_ALTER_ATTR, //
	TMPLT_JACTION_EX_TAG, //
	TMPLT_JACTION_EX_ATTR, //
	TMPLT_JACTION_EX_REG, //
	TMPLT_JACTION_END_TAG, //
	TMPLT_JACTION_END_ATTR, //
	TMPLT_JACTION_END_REG, //
	TMPLT_JACTION_LAST_FLAG //
};

/**
 * @brief 抽取类型
 */
enum tmplt_extract_type_enum
{
	TMPLT_EXTRACT_REALTITLE = 0, //
	TMPLT_EXTRACT_MYPOS = 1, //
	TMPLT_EXTRACT_ARTICLE = 2, //
	TMPLT_EXTRACT_PUBTIME = 3, //
	TMPLT_EXTRACT_NEXTURL = 4, //
	TMPLT_EXTRACT_UNKNOWN,
};

const static char* tmplt_extract_type_desp[] =
{ "realtitle", "mypos", "article", "pubtime", "next_url" };

struct tmplt_extract_define_t
{
	tmplt_extract_type_enum type;
	const char* name;
};

tmplt_json_action_enum get_json_action_type(const char* action);

tmplt_extract_type_enum get_tmplt_extract_type(const char* name);

struct tmplt_json_action_t
{
	tmplt_json_action_enum type;
	const char* name;
};

#define TMPLT_JACTION_KEY					"action"
#define TMPLT_JACTION_TAG_VALUE		"name"
#define TMPLT_JACTION_ATTR_KEY		"key"
#define TMPLT_JACTION_ATTR_VALUE	"value"
#define TMPLT_JACTION_REG_VALUE		"value"

extern const tmplt_json_action_t g_tmplt_json_action_def[];
extern const int g_tmplt_jaction_num;

extern const tmplt_extract_define_t g_tmplt_extract_def[];
extern const int g_tmplt_extract_num;

/**
 * @brief 路径选择类型
 */
enum tmplt_path_item_type_enum
{
	TMPLT_PATH_ITEM_TAG = 1, //标签选择
	TMPLT_PATH_ITEM_ATTR = 2, //属性选择
	TMPLT_PATH_ITEM_ALTER_TAG = 3, //更改标签选择
	TMPLT_PATH_ITEM_ALTER_ATTR = 4, //更改属性选择
	TMPLT_PATH_ITEM_REG = 5, //正则选择
	TMPLT_PATH_ITEM_SOURCE_REG = 6, //源代码正则选择
};

/**
 * @brief 选择路径定义
 */
typedef struct _tmplt_select_path_t
{
	tmplt_path_item_t item;
	tmplt_path_item_type_enum item_type;
	_tmplt_select_path_t* next;
} tmplt_select_path_t;

struct tmplt_regular_t
{
	char* regstr;
	int regstr_len;
	regex_t* reg;
};

struct tmplt_select_t
{
	tmplt_select_path_t* path;

	tmplt_attr_list_t* append_attr;
	tmplt_attr_list_t* ex_attr;
	tmplt_attr_list_t* end_attr;

	tmplt_tagtype_list_t* append_tag;
	tmplt_tagtype_list_t* ex_tag;
	tmplt_tagtype_list_t* end_tag;

	tmplt_regex_list_t* ex_reg;
	tmplt_regex_list_t* end_reg;

	char* source_regstr;
	int source_regstr_len;
	regex_t* source_reg;
};

struct tmplt_select_info_t
{
	tmplt_select_t* select;
	tmplt_tagtype_list_t* hold_tags;
	bool has_img;
	char* date_pattern;
	int date_pattern_len;
	char* date_regstr;
	int date_regstr_len;
	regex_t* date_reg;
};

/**
 * @brief 模板
 */
struct template_info_t
{
	int template_id;
	char* source; //来源
	int source_len;
	char* classify; //分类
	int classify_len;
	tmplt_string_list_t* webname_list; //入口
	tm update_time;
	bool valid; //是否有效

	tmplt_select_info_t* extract_tmplts;

	regex_t* pubtime_regex;
	char* pubtime_regstr;
};

typedef struct _template_list_t
{
	int template_id;
	template_info_t* template_info;
	_template_list_t* next;
} template_list_t;

struct tmplt_url_entry_t
{
	char* url_regex;
	int url_regex_len;
	regex_t* regex;
	template_list_t* templates;
	bool valid;
};

typedef struct _tmplt_url_entry_list_t
{
	tmplt_url_entry_t* url_entry;
	_tmplt_url_entry_list_t* next;
} tmplt_url_entry_list_t;

typedef struct _tmplt_mysql_config_t
{
	char host[20];
	int port;
	char dbname[20];
	char user[20];
	char password[20];
	int connect_timeout;
	_tmplt_mysql_config_t* next;
	_tmplt_mysql_config_t* prev;
} tmplt_mysql_config_t;

struct templates_t
{
	hashmap_t* hm_url_entry;
	hashmap_t* hm_template;
	hashmap_t* hm_site_regex;
	tmplt_mysql_config_t* mysql_info;
	char* buf;
	unsigned int buf_size;
	unsigned int avail;
};

struct tmplt_extract_result_t
{
	char* str;
	int str_len;
	html_node_list_t* selected;
	html_node_list_t* selected_imgs;
	tmplt_regex_list_t* ex_reg;
};

/**
 * @brief 模版抽取的输出，如果某项未抽取到，则为null
 * @author sue
 * @date 2013-06-17
 */
struct tmplt_result_t
{
	tmplt_extract_result_t* extract_result;
	tm pubtime_tm;
	char* classify;
	int classify_len;
	char* source;
	int source_len;

	//fields below only for inner use
	char* buf;
	unsigned int size;
	unsigned int avail;
};

tmplt_result_t* create_tmplt_result(unsigned int size = 1 << 21);

void del_tmplt_result(tmplt_result_t* data);

templates_t* create_templates(const char* path);

/**
 * @brief 加载模板，从mysql数据库中读取模板信息
 * @param dicts [in/out], 模板对象，调用 create_templates 创建
 * @return 0，表示成功。其它表示失败
 */
int load_templates(templates_t* dicts);

void del_templates(templates_t* dict);

int try_template_extract(templates_t* dict, const char* page, int page_len, const char* url, int url_len, html_tree_t* tree, tmplt_result_t* output, int res = 0);

int tmplt_apply_exreg_rule(char* buf, int len, tmplt_regex_list_t* ex_reg);

void printf_templates_info(templates_t* dicts);

#endif /* EASOU_EXTRACTOR_TEMPLATE_H_ */

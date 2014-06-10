/*
 * easou_extractor_template.cpp
 *
 *  Created on: 2013-6-13
 *      Author: sue
 */

#include "easou_extractor_template.h"
#include "easou_html_extractor.h"
#include "easou_html_attr.h"
#include "easou_string.h"
#include "easou_debug.h"

#include "chardet.h"
#include "CCharset.h"

#include <mysql.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>

#define MYSQL_CONF_FILE "mysql.conf"

#define TMPLT_REGEX_RES_NUM 8

static json_t* tmplt_json_compile(const char* str, int len);
static char* tmplt_alloc_buf(int size, templates_t* dict);
static tmplt_select_t* create_tmplt_select_info(const char* json_str, int json_str_len, templates_t* dict);

const tmplt_json_action_t g_tmplt_json_action_def[] =
{
{ TMPLT_JACTION_REG, "reg" },
{ TMPLT_JACTION_TAG, "tag" },
{ TMPLT_JACTION_ATTR, "attr" },
{ TMPLT_JACTION_ALTER_TAG, "alter-tag" },
{ TMPLT_JACTION_ALTER_ATTR, "alter-attr" },
{ TMPLT_JACTION_EX_TAG, "ex-tag" },
{ TMPLT_JACTION_EX_ATTR, "ex-attr" },
{ TMPLT_JACTION_EX_REG, "ex-reg" },
{ TMPLT_JACTION_END_TAG, "end-tag" },
{ TMPLT_JACTION_END_ATTR, "end-attr" },
{ TMPLT_JACTION_END_REG, "end-reg" }, };

const int g_tmplt_jaction_num = sizeof(g_tmplt_json_action_def) / sizeof(g_tmplt_json_action_def[0]);

const tmplt_extract_define_t g_tmplt_extract_def[] =
{
{ TMPLT_EXTRACT_REALTITLE, "contentTitle" },
{ TMPLT_EXTRACT_MYPOS, "navigation" },
{ TMPLT_EXTRACT_ARTICLE, "content" },
{ TMPLT_EXTRACT_PUBTIME, "publishTime" },
{ TMPLT_EXTRACT_NEXTURL, "nextUrl" }, };

const int g_tmplt_extract_num = sizeof(g_tmplt_extract_def) / sizeof(g_tmplt_extract_def[0]);

static void free_tmplt_regex_list(tmplt_regex_list_t* reglist)
{
	if (reglist == NULL)
		return;
	tmplt_regex_list_t* list = reglist;
	while (list)
	{
		if (list->reg)
		{
			regfree(list->reg);
			delete list->reg;
			list->reg = NULL;
		}
		list = list->next;
	}
}

static void free_tmplt_select_info(tmplt_select_t* select_info)
{
	if (select_info == NULL)
		return;
	tmplt_select_path_t* path = select_info->path;
	while (path)
	{
		if (path->item_type == TMPLT_PATH_ITEM_REG)
		{
			if (path->item.reg->reg)
			{
				regfree(path->item.reg->reg);
				delete path->item.reg->reg;
				path->item.reg->reg = NULL;
			}
		}
		else if (path->item_type == TMPLT_PATH_ITEM_SOURCE_REG)
		{
			if (path->item.source_reg->reg)
			{
				for (int i = 0; i < TMPLT_REGEX_RES_NUM; i++)
				{
					regfree(&path->item.source_reg->reg[i]);
				}
				delete[] path->item.source_reg->reg;
				path->item.source_reg->reg = NULL;
			}
		}
		path = path->next;
	}
	free_tmplt_regex_list(select_info->ex_reg);
	free_tmplt_regex_list(select_info->end_reg);
}

static void tmplt_free_tmplt(template_info_t* tmplt)
{
	if (tmplt->pubtime_regex)
	{
		regfree(tmplt->pubtime_regex);
		delete tmplt->pubtime_regex;
		tmplt->pubtime_regex = NULL;
	}
	for (int i = 0; i < g_tmplt_extract_num; i++)
	{
		free_tmplt_select_info(tmplt->extract_tmplts[i].select);
		if (tmplt->extract_tmplts[i].date_reg)
		{
			regfree(tmplt->extract_tmplts[i].date_reg);
			delete tmplt->extract_tmplts[i].date_reg;
			tmplt->extract_tmplts[i].date_reg = NULL;
		}
	}
}

static void set_tm_info_by_date_pattern(const char* date_pattern, int date_pattern_len, const char* date_str, int date_str_len, tm* tm_time)
{
	memset(tm_time, 0, sizeof(tm));
	if (date_pattern == NULL || date_str == NULL || tm_time == NULL || date_pattern_len <= 0 || date_str_len <= 0)
		return;

	static const char digits[] =
	{ 'y', 'M', 'd', 'H', 'h', 'm', 's', 0 };

	const char* p = date_pattern;
	const char* end = date_pattern + date_pattern_len;

	int last_idx = -1;
	int digits_idx = -1;
	int c_digit = 0;

	const char* pd = date_str;
	const char* pdend = date_str + date_str_len;

#define IS_OK_S(s,send) (s<send&&*s)
#define SKIP_TO_DIGIT(s, send) while(IS_OK_S(s,send)&&!isdigit(*s)){s++;}
#define SKIP_TO_NO_DIGIT(s, send) while(IS_OK_S(s,send)&&isdigit(*s)){s++;}
#define SET_TM(pd, pdend, tm) \
	SKIP_TO_DIGIT(pd, pdend); \
	if (!IS_OK_S(pd, pdend)) \
		return; \
	const char* tmp = pd; \
	SKIP_TO_NO_DIGIT(pd, pdend); \
	int dlen = pd - tmp; \
	if (dlen == c_digit) \
	{ \
		if (last_idx == 0) \
			tm->tm_year = atoi(tmp); \
		else if (last_idx == 1) \
			tm->tm_mon = atoi(tmp); \
		else if (last_idx == 2) \
			tm->tm_yday = atoi(tmp); \
		else if (last_idx == 3 || last_idx == 4) \
			tm->tm_hour = atoi(tmp); \
		else if (last_idx == 5) \
			tm->tm_min = atoi(tmp); \
		else if (last_idx == 6) \
			tm->tm_sec = atoi(tmp); \
	} \
	if (!IS_OK_S(pd, pdend)) \
		return;

	while (p < end)
	{
		bool is_digit = false;
		digits_idx = -1;
		for (int i = 0;; i++)
		{
			if (digits[i] == 0)
				break;
			if (p[0] == digits[i])
			{
				is_digit = true;
				digits_idx = i;
				break;
			}
		}
		if (is_digit)
		{
			if (last_idx == -1)
			{
				last_idx = digits_idx;
				c_digit++;
			}
			else if (last_idx != digits_idx)
			{
				SET_TM(pd, pdend, tm_time);

				last_idx = digits_idx;
				c_digit = 1;
			}
			else
				c_digit++;
		}
		else
		{
			if (c_digit > 0)
			{
				SET_TM(pd, pdend, tm_time);
			}
			last_idx = -1;
			c_digit = 0;
		}
		p++;
	}
	if (c_digit > 0)
	{
		last_idx = digits_idx;
		SET_TM(pd, pdend, tm_time);
	}
}

static tmplt_tagtype_list_t* construct_hold_tags(const char* str, int len, templates_t* dict)
{
	if (len <= 0 || str == NULL)
		return NULL;

	tmplt_tagtype_list_t* nlist = NULL;

	const char* p = str;
	const char* end = str + len;

	int nlen;
	const char* start = p;
	while (*p && p < end)
	{
		if (p[0] == '|')
		{
			nlen = p - start;
			if (nlen > 0)
			{
				html_tag_type_t tag_type = get_tag_type(start, nlen);
				if (tag_type != TAG_UNKNOWN)
				{
					tmplt_tagtype_list_t* list = (tmplt_tagtype_list_t*) tmplt_alloc_buf(sizeof(tmplt_tagtype_list_t), dict);
					list->tag_type = tag_type;
					list->next = nlist;
					nlist = list;
				}
			}
			start = p + 1;
		}
		p++;
	}
	nlen = end - start;
	if (nlen > 0)
	{
		html_tag_type_t tag_type = get_tag_type(start, nlen);
		if (tag_type != TAG_UNKNOWN)
		{
			tmplt_tagtype_list_t* list = (tmplt_tagtype_list_t*) tmplt_alloc_buf(sizeof(tmplt_tagtype_list_t), dict);
			list->tag_type = tag_type;
			list->next = nlist;
			nlist = list;
		}
	}
	return nlist;
}

static int tmplt_convert_to_gb18030(const char *src, int src_len, char *dest, int dest_size)
{
	char charset[64];
	CCharset m_cCharset;
	int ret;
	if ((ret = m_cCharset.GetCharset(src, src_len, charset)) < 0)
	{
		dest[0] = 0;
		return 0;
	}
	if (strncasecmp(charset, "gb18030", 7) == 0)
	{
		ret = src_len < dest_size ? src_len : (dest_size - 1);
		memcpy(dest, src, ret);
		*(dest + ret) = 0;
		return ret;
	}

	if ((ret = m_cCharset.TransCToGb18030(src, src_len, dest, dest_size, charset)) > 0)
	{
		*(dest + ret) = 0;
		return ret;
	}
	dest[0] = 0;
	return 0;
}

static char* alloc_and_copy_converted_gb18030_string(const char* src, unsigned int len, templates_t* dict, int& nlen)
{
	assert(dict && (dict->avail + len + 1) < dict->buf_size);
	char* psave = dict->buf + dict->avail;
	if (src != NULL)
	{
		nlen = tmplt_convert_to_gb18030(src, len, psave, len + 1);
		len = nlen;
	}
	dict->avail += len;
	dict->buf[dict->avail++] = 0;
	return psave;
}

static char* alloc_buf(char*& buf, unsigned int buf_size, unsigned int& avail, int item_size)
{
	if (avail + item_size >= buf_size)
		return NULL;
	char* p = buf + avail;
	avail += item_size;
	memset(p, 0, item_size);
	return p;
}

static int repair_regex_str(const char* src, int len, char* buf, int size, const char* to_be_replaced[], const char* replace_str[])
{
	const char* p = src;
	const char* end = src + len;

	char *pd = buf;
	char *pdend = buf + size;

	while (p < end && pd < pdend)
	{
		int index = -1;
		for (int i = 0;; i++)
		{
			if (to_be_replaced[i] == 0)
				break;
			if (p + strlen(to_be_replaced[i]) < end && strncmp(p, to_be_replaced[i], strlen(to_be_replaced[i])) == 0 && pd + strlen(replace_str[i]) < pdend)
			{
				index = i;
				break;
			}
		}
		if (index != -1)
		{
			memcpy(pd, replace_str[index], strlen(replace_str[index]));
			pd += strlen(replace_str[index]);
			p += strlen(to_be_replaced[index]);
		}
		else
			*pd++ = *p++;
	}
	pd[0] = 0;
	return pd - buf;
}

static void tmplt_free_mysql_config(templates_t* dict)
{
	if (dict == NULL)
		return;
	tmplt_mysql_config_t* cfg = dict->mysql_info;
	while (cfg)
	{
		tmplt_mysql_config_t* tmp = cfg;
		cfg = cfg->next;
		delete tmp;
	}
	dict->mysql_info = NULL;
}

static int tmplt_read_mysql_config(tmplt_mysql_config_t **head, const char* path)
{
	char file_path[2048];
	int ret = snprintf(file_path, 2048, "%s/%s", path, MYSQL_CONF_FILE);
	file_path[ret] = 0;

	FILE *fp = fopen(file_path, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "can't open file %s\n", file_path);
		return -1;
	}

#define SQLINFO_HOST			1
#define SQLINFO_PORT			2
#define SQLINFO_DBNAME		4
#define SQLINFO_USER			8
#define SQLINFO_PWD				16
#define SQLINFO_TIMEOUT	32
#define SQLINFO_ALL				63

	int count = 0;
	tmplt_mysql_config_t mysql_info;
	int status = 0;
	char line[2048];
	while (fgets(line, 2048, fp))
	{
		remove_tail_rnt(line);
		if (line[0] == '[')
		{
			status = 0;
			continue;
		}
		else if (line[0] == '#')
			continue;

		if (strncmp(line, "host=", 5) == 0)
		{
			int len = strlen(&line[5]);
			memcpy(mysql_info.host, &line[5], len);
			mysql_info.host[len] = 0;
			status |= SQLINFO_HOST;
		}
		else if (strncmp(line, "port=", 5) == 0)
		{
			mysql_info.port = atoi(&line[5]);
			if (mysql_info.port > 0)
				status |= SQLINFO_PORT;
		}
		else if (strncmp(line, "dbname=", 7) == 0)
		{
			int len = strlen(&line[7]);
			memcpy(mysql_info.dbname, &line[7], len);
			mysql_info.dbname[len] = 0;
			status |= SQLINFO_DBNAME;
		}
		else if (strncmp(line, "username=", 9) == 0)
		{
			int len = strlen(&line[9]);
			memcpy(mysql_info.user, &line[9], len);
			mysql_info.user[len] = 0;
			status |= SQLINFO_USER;
		}
		else if (strncmp(line, "password=", 9) == 0)
		{
			int len = strlen(&line[9]);
			memcpy(mysql_info.password, &line[9], len);
			mysql_info.password[len] = 0;
			status |= SQLINFO_PWD;
		}
		else if (strncmp(line, "connect_timeout=", 16) == 0)
		{
			mysql_info.connect_timeout = atoi(&line[16]);
			if (mysql_info.connect_timeout > 0)
				status |= SQLINFO_TIMEOUT;
		}

		if (status == SQLINFO_ALL)
		{
			tmplt_mysql_config_t *info = new tmplt_mysql_config_t;
			assert(info);
			memcpy(info, &mysql_info, sizeof(tmplt_mysql_config_t));
			info->next = *head;
			info->prev = NULL;
			if (*head)
			{
				(*head)->prev = info;
			}
			*head = info;

			status = 0;
			count++;
		}
	}

#undef SQLINFO_HOST
#undef SQLINFO_PORT
#undef SQLINFO_DBNAME
#undef SQLINFO_USER
#undef SQLINFO_PWD
#undef SQLINFO_TIMEOUT
#undef SQLINFO_ALL

	fclose(fp);
	return count;
}

tmplt_result_t* create_tmplt_result(unsigned int size)
{
	tmplt_result_t* result = new tmplt_result_t;
	assert(result);
	memset(result, 0, sizeof(tmplt_result_t));

	result->size = size;
	result->buf = (char*) malloc(result->size);
	assert(result->buf);
	result->extract_result = new tmplt_extract_result_t[g_tmplt_extract_num];
	assert(result->extract_result);
	return result;
}

void del_tmplt_result(tmplt_result_t* result)
{
	if (result == NULL)
		return;
	if (result->extract_result)
		delete[] result->extract_result;
	if (result->buf)
		free(result->buf);
	delete result;
}

templates_t* create_templates(const char* path)
{
	templates_t* dicts = new templates_t;
	assert(dicts);

	dicts->hm_url_entry = hashmap_create(9973);
	assert(dicts->hm_url_entry);

	dicts->hm_template = hashmap_create(7039);
	assert(dicts->hm_template);

	dicts->hm_site_regex = hashmap_create();
	assert(dicts->hm_site_regex);

	dicts->mysql_info = NULL;
	int count = tmplt_read_mysql_config(&dicts->mysql_info, path);
	if (count <= 0)
	{
		delete dicts;
		fprintf(stderr, "[extractor_template] read_mysql_config fail, ret:%d\n", count);
		return NULL;
	}

	dicts->buf = (char*) malloc(1 << 24);
	assert(dicts->buf != NULL);
	dicts->buf_size = 1 << 24;
	dicts->avail = 0;

	return dicts;
}

void printf_templates_info(templates_t* dicts)
{
}

static void insert_tmplt_path_item(tmplt_select_t* ext, tmplt_select_path_t* item)
{
	assert(ext!= NULL && item != NULL);
	if (ext->path == NULL)
	{
		ext->path = item;
		return;
	}
	tmplt_select_path_t* tail = ext->path;
	while (tail->next)
		tail = tail->next;
	tail->next = item;
}

static void path_add_source_reg(const char* src, int len, templates_t* dict, tmplt_select_t* select)
{
	if (select == NULL)
		return;

	tmplt_regex_t* reglist = (tmplt_regex_t*) tmplt_alloc_buf(sizeof(tmplt_regex_t), dict);

	reglist->regstr = alloc_and_copy_converted_gb18030_string(src, len, dict, reglist->regstr_len);
	char *buf = &dict->buf[dict->avail];

	static const char* to_be_replaced[] =
	{ "[\\d]", "[\\s\\S]", 0 };
	static const char* replace_str[] =
	{ "[0-9]", ".", 0 };
	int nlen = repair_regex_str(reglist->regstr, reglist->regstr_len, buf, dict->buf_size - dict->avail, to_be_replaced, replace_str);

	dict->avail += (nlen + 1);
	reglist->regstr = buf;
	reglist->regstr_len = nlen;

	reglist->reg = new regex_t[TMPLT_REGEX_RES_NUM];
	bool flag = true;
	for (int i = 0; i < TMPLT_REGEX_RES_NUM; i++)
	{
		if (0 != regcomp(&reglist->reg[i], reglist->regstr, REG_EXTENDED))
		{
			regfree(&reglist->reg[i]);
			flag = false;
			break;
		}
	}
	if (!flag)
	{
		delete[] reglist->reg;
		reglist->reg = NULL;
		reglist->reg_num = 0;
	}
	else
		reglist->reg_num = TMPLT_REGEX_RES_NUM;

	tmplt_select_path_t* path = (tmplt_select_path_t*) tmplt_alloc_buf(sizeof(tmplt_select_path_t), dict);
	path->item_type = TMPLT_PATH_ITEM_SOURCE_REG;
	path->item.source_reg = reglist;

	if (select->path == NULL)
		select->path = path;
	else
		insert_tmplt_path_item(select, path);
}

static int load_next_url_info(MYSQL* conn, templates_t* dict)
{
	const char* sql = "select b.id, a.action, a.expression, a.urlregex "
					"from news_template_contentpage a, news_template_seed b "
					"where a.fid=b.contentpage_fid and b.`status`=1";

	static unsigned int sql_len = strlen(sql);

	if (mysql_real_query(conn, sql, sql_len))
	{
		fprintf(stderr, "mysql_real_query fail, %s, %s\n", sql, mysql_error(conn));
		return -1;
	}

	MYSQL_RES* res = mysql_store_result(conn);
	if (res == NULL)
	{
		fprintf(stderr, "mysql_store_result fail, %s, %s\n", sql, mysql_error(conn));
		return -1;
	}

	MYSQL_ROW row;
	unsigned int field_num = mysql_field_count(conn);
	while ((row = mysql_fetch_row(res)) != NULL)
	{
		bool flag = true;
		for (unsigned int i = 0; i < field_num; i++)
			if (row[i] == NULL)
			{
				flag = false;
				break;
			}
		if (!flag)
			continue;

		unsigned long int* lengths = mysql_fetch_lengths(res);

		int template_id = atoi(row[0]);

		int ret = snprintf(dict->buf + dict->avail, 10, "%d", template_id);
		assert(ret < 10 && dict->avail + ret < dict->buf_size);

		int bucketno = key_to_hash_bucket_index(dict->buf + dict->avail, ret, dict->hm_template);
		template_info_t* tmplt = (template_info_t*) hashmap_get(dict->hm_template, bucketno, dict->buf + dict->avail, ret);
		if (tmplt == NULL)
		{
			//fprintf(stderr, "skip next url for template:%d for template info not created\n", template_id);
			continue;
		}

		if (strcmp("select", row[1]) == 0)
		{
			tmplt->extract_tmplts[TMPLT_EXTRACT_NEXTURL].select = create_tmplt_select_info(row[2], lengths[2], dict);
			path_add_source_reg(row[3], lengths[3], dict, tmplt->extract_tmplts[TMPLT_EXTRACT_NEXTURL].select);
		}
		else if (strcmp("regular", row[1]) == 0)
		{
			tmplt->extract_tmplts[TMPLT_EXTRACT_NEXTURL].select = (tmplt_select_t*) tmplt_alloc_buf(sizeof(tmplt_select_t), dict);
			path_add_source_reg(row[2], lengths[2], dict, tmplt->extract_tmplts[TMPLT_EXTRACT_NEXTURL].select);
			path_add_source_reg(row[3], lengths[3], dict, tmplt->extract_tmplts[TMPLT_EXTRACT_NEXTURL].select);
		}
		else
		{
			//fprintf(stderr, "unknown action for next_url, %s\n", row[1]);
			continue;
		}

		tmplt->extract_tmplts[TMPLT_EXTRACT_NEXTURL].has_img = false;
		tmplt->extract_tmplts[TMPLT_EXTRACT_NEXTURL].hold_tags = construct_hold_tags("a", 1, dict);
	}

	mysql_free_result(res);
	return 0;
}

static char* tmplt_alloc_buf(int size, templates_t* dict)
{
	assert(dict && dict->avail + size < dict->buf_size);
	char* p = dict->buf + dict->avail;
	memset(p, 0, size);
	dict->avail += size;
	return p;
}

static char* alloc_and_copy_string(const char* src, unsigned int len, templates_t* dict)
{
	assert(dict && (dict->avail + len) < dict->buf_size);
	char* psave = dict->buf + dict->avail;
	if (src != NULL)
		memcpy(psave, src, len);
	dict->avail += len;
	dict->buf[dict->avail++] = 0;
	return psave;
}

static tmplt_string_list_t* create_webname_list(templates_t* dict, const char* src, int len)
{
	if (src == NULL || len <= 0)
		return NULL;
	assert(dict != NULL);

	tmplt_string_list_t* list_head = NULL;

	const char* s = src;
	const char* p = src;
	const char* end = src + len;

	int nlen = 0;
	while (*p && p < end)
	{
		if (p[0] == ',')
		{
			nlen = p - s;
			if (nlen > 0)
			{
				tmplt_string_list_t* list_node = (tmplt_string_list_t*) tmplt_alloc_buf(sizeof(tmplt_string_list_t), dict);
				list_node->str = alloc_and_copy_string(s, nlen, dict);
				list_node->str_len = nlen;
				list_node->next = list_head;
				list_head = list_node;
			}
			s = p + 1;

		}
		p++;
	}
	nlen = p - s;
	if (nlen > 0)
	{
		tmplt_string_list_t* list_node = (tmplt_string_list_t*) tmplt_alloc_buf(sizeof(tmplt_string_list_t), dict);
		list_node->str = alloc_and_copy_string(s, nlen, dict);
		list_node->str_len = nlen;
		list_node->next = list_head;
		list_head = list_node;
	}
	return list_head;
}

static void inline set_time_field(tm* t, const char* timestr, const char* formatstr)
{
	if (timestr == NULL || formatstr == NULL)
		return;
	sscanf(timestr, formatstr, &t->tm_year, &t->tm_mon, &t->tm_yday, &t->tm_hour, &t->tm_min, &t->tm_sec);
}

static int load_regex_info(MYSQL* conn, templates_t* dict)
{
	const char* sql = "select a.regex, b.id "
					"from news_template_content a, news_template_seed b "
					"where b.status=1 and a.regex is not null and a.regex!=\"\" and a.fid=b.content_fid";

	static int sql_len = strlen(sql);

	if (0 != mysql_real_query(conn, sql, sql_len))
	{
		fprintf(stderr, "[extractor_template] mysql_real_query fail, %s, %s\n", sql, mysql_error(conn));
		return -1;
	}

	MYSQL_RES* res = mysql_store_result(conn);
	if (res == NULL)
	{
		fprintf(stderr, "[extractor_template] mysql_store_result fail, %s\n", sql);
		return -1;
	}

	int record_num = 0;
	int url_entry_num = 0;
	int reg_compile_fail = 0;
	int ret;
	char error[1024];

	int num_fields = mysql_field_count(conn);
	MYSQL_ROW row;
	while ((row = mysql_fetch_row(res)) != NULL)
	{
		bool flag = true;
		for (int i = 0; i < num_fields; i++)
			if (row[i] == NULL)
			{
				flag = false;
				break;
			}
		if (!flag)
			continue;

		long unsigned int* lengths = mysql_fetch_lengths(res);

		int template_id = atoi(row[1]);

		record_num++;
		int bucketno = key_to_hash_bucket_index(row[0], lengths[0], dict->hm_url_entry);
		tmplt_url_entry_t* url_entry = (tmplt_url_entry_t*) hashmap_get(dict->hm_url_entry, bucketno, row[0], lengths[0]);
		if (url_entry == NULL)
		{
			url_entry_num++;
			url_entry = (tmplt_url_entry_t*) tmplt_alloc_buf(sizeof(tmplt_url_entry_t), dict);
			url_entry->url_regex = alloc_and_copy_string(row[0], lengths[0], dict);
			url_entry->url_regex_len = lengths[0];
			url_entry->valid = true;
			url_entry->regex = new regex_t;
			assert(url_entry->regex);
			if (0 != (ret = regcomp(url_entry->regex, url_entry->url_regex, REG_EXTENDED)))
			{
				reg_compile_fail++;
				regerror(ret, url_entry->regex, error, 1024);
				//fprintf(stderr, "[url_entry] regex compile fail for %s template_id:%d, reason:%s\n", url_entry->url_regex, template_id, error);
				regfree(url_entry->regex);
				delete url_entry->regex;
				url_entry->regex = NULL;
				url_entry->valid = false;
			}

			url_entry->templates = (template_list_t*) tmplt_alloc_buf(sizeof(template_list_t), dict);
			url_entry->templates->template_id = template_id;

			hashmap_put(dict->hm_url_entry, bucketno, url_entry->url_regex, url_entry->url_regex_len, url_entry);
		}
		else
		{
			bool exist = false;
			template_list_t* tmplt_list = url_entry->templates;
			while (tmplt_list)
			{
				if (tmplt_list->template_id == template_id)
				{
					exist = true;
					break;
				}
				tmplt_list = tmplt_list->next;
			}
			if (!exist)
			{
				tmplt_list = (template_list_t*) tmplt_alloc_buf(sizeof(template_list_t), dict);
				tmplt_list->template_id = template_id;

				tmplt_list->next = url_entry->templates;
				url_entry->templates = tmplt_list;
			}
		}
	}

	//printf("[url_entry] entry_num:%d compile_fail:%d record_num:%d\n", url_entry_num, reg_compile_fail, record_num);
	mysql_free_result(res);
	return 0;
}

tmplt_json_action_enum get_json_action_type(const char* action)
{
	if (action == NULL)
		return TMPLT_JACTION_UNKNOWN;
	for (int i = 0; i < g_tmplt_jaction_num; i++)
	{
		if (strcmp(g_tmplt_json_action_def[i].name, action) == 0)
			return g_tmplt_json_action_def[i].type;
	}
	return TMPLT_JACTION_UNKNOWN;
}

tmplt_extract_type_enum get_tmplt_extract_type(const char* name)
{
	if (name == NULL)
		return TMPLT_EXTRACT_UNKNOWN;
	for (int i = 0; i < g_tmplt_extract_num; i++)
	{
		if (strcmp(g_tmplt_extract_def[i].name, name) == 0)
			return g_tmplt_extract_def[i].type;
	}
	return TMPLT_EXTRACT_UNKNOWN;
}

static void insert_tmplt_attr_item(tmplt_attr_list_t* list, tmplt_attr_list_t* item)
{
	assert(list!= NULL && item != NULL);
	tmplt_attr_list_t* tail = list;
	while (tail->next)
		tail = tail->next;
	tail->next = item;
}

static void insert_tmplt_tag_item(tmplt_tagtype_list_t* list, tmplt_tagtype_list_t* item)
{
	assert(list!= NULL && item != NULL);
	tmplt_tagtype_list_t* tail = list;
	while (tail->next)
		tail = tail->next;
	tail->next = item;
}

static void insert_tmplt_reg_item(tmplt_regex_list_t* list, tmplt_regex_list_t* item)
{
	assert(list!= NULL && item != NULL);
	tmplt_regex_list_t* tail = list;
	while (tail->next)
		tail = tail->next;
	tail->next = item;
}

static tmplt_select_t* create_tmplt_select_info(const char* json_str, int json_str_len, templates_t* dict)
{
	json_t* jarray = tmplt_json_compile(json_str, json_str_len);
	if (jarray == NULL)
	{
		//fprintf(stderr, "compile json fail, %s\n", json_str);
		return NULL;
	}

	tmplt_select_t* select_info = (tmplt_select_t*) tmplt_alloc_buf(sizeof(tmplt_select_t), dict);

	int size = json_array_size(jarray);
	for (int i = 0; i < size; i++)
	{
		json_t* jobj = json_array_get(jarray, i);
		json_t* jaction = json_object_get(jobj, TMPLT_JACTION_KEY);
		if (jaction == NULL)
			continue;
		const char* action_str = json_string_value(jaction);
		if (action_str == NULL)
			continue;
		tmplt_json_action_enum action_type = get_json_action_type(action_str);
		if (action_type == TMPLT_JACTION_UNKNOWN)
			continue;

		if (action_type == TMPLT_JACTION_TAG || action_type == TMPLT_JACTION_ALTER_TAG || action_type == TMPLT_JACTION_EX_TAG || action_type == TMPLT_JACTION_END_TAG)
		{
			json_t* jv = json_object_get(jobj, TMPLT_JACTION_TAG_VALUE);
			const char* tagname = json_string_value(jv);
			html_tag_type_t tag_type = get_tag_type(tagname, strlen(tagname));
			if (tag_type != TAG_UNKNOWN)
			{
				tmplt_tagtype_list_t* tag = (tmplt_tagtype_list_t*) tmplt_alloc_buf(sizeof(tmplt_tagtype_list_t), dict);
				tag->tag_type = tag_type;

				if (action_type == TMPLT_JACTION_TAG || action_type == TMPLT_JACTION_ALTER_TAG)
				{
					tmplt_select_path_t* path = (tmplt_select_path_t*) tmplt_alloc_buf(sizeof(tmplt_select_path_t), dict);
					path->item_type = TMPLT_PATH_ITEM_TAG;
					path->item.tag = tag;
					if (action_type == TMPLT_JACTION_ALTER_TAG)
						select_info->path = path;
					else
						insert_tmplt_path_item(select_info, path);
				}
				else if (action_type == TMPLT_JACTION_EX_TAG)
				{
					if (select_info->ex_tag == NULL)
						select_info->ex_tag = tag;
					else
						insert_tmplt_tag_item(select_info->ex_tag, tag);
				}
				else if (action_type == TMPLT_JACTION_END_TAG)
				{
					if (select_info->end_tag == NULL)
						select_info->end_tag = tag;
					else
						insert_tmplt_tag_item(select_info->end_tag, tag);
				}
			}
		}
		else if (action_type == TMPLT_JACTION_ATTR || action_type == TMPLT_JACTION_ALTER_ATTR || action_type == TMPLT_JACTION_EX_ATTR || action_type == TMPLT_JACTION_END_ATTR)
		{
			json_t* jvk = json_object_get(jobj, TMPLT_JACTION_ATTR_KEY);
			json_t* jvv = json_object_get(jobj, TMPLT_JACTION_ATTR_VALUE);
			const char* attr_key = json_string_value(jvk);
			const char* attr_value = json_string_value(jvv);
			html_attr_type_t attr_type = get_attr_type(attr_key, strlen(attr_key));
			if (attr_key && attr_value)
			{
				tmplt_attr_list_t* attr = (tmplt_attr_list_t*) tmplt_alloc_buf(sizeof(tmplt_attr_list_t), dict);
				attr->attr_type = attr_type;
				attr->value_len = strlen(attr_value);
				attr->value = alloc_and_copy_string(attr_value, attr->value_len, dict);
				attr->name_len = strlen(attr_key);
				attr->name = alloc_and_copy_string(attr_key, attr->name_len, dict);

				if (action_type == TMPLT_JACTION_ATTR || action_type == TMPLT_JACTION_ALTER_ATTR)
				{
					tmplt_select_path_t* path = (tmplt_select_path_t*) tmplt_alloc_buf(sizeof(tmplt_select_path_t), dict);
					path->item_type = TMPLT_PATH_ITEM_ATTR;
					path->item.attr = attr;
					if (action_type == TMPLT_JACTION_ALTER_ATTR)
						select_info->path = path;
					else if (action_type == TMPLT_JACTION_ATTR)
						insert_tmplt_path_item(select_info, path);
				}
				else if (action_type == TMPLT_JACTION_EX_ATTR)
				{
					if (select_info->ex_attr == NULL)
						select_info->ex_attr = attr;
					else
						insert_tmplt_attr_item(select_info->ex_attr, attr);
				}
				else if (action_type == TMPLT_JACTION_END_ATTR)
				{
					if (select_info->end_attr == NULL)
						select_info->end_attr = attr;
					else
						insert_tmplt_attr_item(select_info->end_attr, attr);
				}
			}
		}
		else if (action_type == TMPLT_JACTION_REG || action_type == TMPLT_JACTION_EX_REG || action_type == TMPLT_JACTION_END_REG)
		{
			json_t* jreg = json_object_get(jobj, TMPLT_JACTION_REG_VALUE);
			const char* reg_value = json_string_value(jreg);
			int reg_value_len = strlen(reg_value);

			tmplt_regex_list_t* reglist = (tmplt_regex_list_t*) tmplt_alloc_buf(sizeof(tmplt_regex_list_t), dict);
			reglist->regstr = alloc_and_copy_converted_gb18030_string(reg_value, reg_value_len, dict, reglist->regstr_len);
			regex_t* reg = new regex_t;
			assert(reg);
			if (0 != regcomp(reg, reglist->regstr, REG_EXTENDED))
			{
				//fprintf(stderr, "regex compile fail, %s\n", reg_value);
				regfree(reg);
				delete reg;
				reglist->reg = NULL;
			}
			else
				reglist->reg = reg;

			if (action_type == TMPLT_JACTION_REG)
			{
				tmplt_select_path_t* path = (tmplt_select_path_t*) tmplt_alloc_buf(sizeof(tmplt_select_path_t), dict);
				path->item_type = TMPLT_PATH_ITEM_REG;
				path->item.reg = reglist;
				if (select_info->path == NULL)
					select_info->path = path;
				else
					insert_tmplt_path_item(select_info, path);
				break;
			}
			else if (action_type == TMPLT_JACTION_EX_REG)
			{
				if (select_info->ex_reg == NULL)
					select_info->ex_reg = reglist;
				else
					insert_tmplt_reg_item(select_info->ex_reg, reglist);
			}
			else if (action_type == TMPLT_JACTION_END_REG)
			{
				if (select_info->end_reg == NULL)
					select_info->end_reg = reglist;
				else
					insert_tmplt_reg_item(select_info->end_reg, reglist);
			}
		}
	}

	json_delete(jarray);
	return select_info;
}

static char* convert_date_pattern_to_regstr(const char* src, int len, templates_t* dict, int& nlen)
{
	assert(dict->avail + len * 10 < dict->buf_size);

	static const char digits[] =
	{ 'y', 'M', 'd', 'H', 'h', 'm', 's', 0 };

	const char* p = src;
	const char* end = src + len;
	char* pd = &dict->buf[dict->avail];
	char* pdend = pd + len * 10;

	int c_digit = 0;
	while (p < end && pd < pdend)
	{
		bool is_digit = false;
		for (int i = 0;; i++)
		{
			if (digits[i] == 0)
				break;
			if (p[0] == digits[i])
			{
				is_digit = true;
				break;
			}
		}
		if (is_digit)
			c_digit++;
		else
		{
			if (c_digit > 0)
			{
				int ret = sprintf(pd, "[0-9]{1,%d}", c_digit);
				pd += ret;
				assert(pd < pdend - 1);
			}
			c_digit = 0;
			*pd++ = *p;
		}
		p++;
	}
	if (c_digit > 0)
	{
		int ret = sprintf(pd, "[0-9]{1,%d}", c_digit);
		pd += ret;
		assert(pd < pdend - 1);
	}
	pd[0] = 0;
	nlen = pd - &dict->buf[dict->avail];
	char *str = &dict->buf[dict->avail];
	dict->avail += nlen + 1;
	return str;
}

static int load_basic_info(MYSQL* conn, templates_t* dict)
{
	const char* sql = "select b.id, a.hasImage, a.holdTags, a.datePattern, a.action, a.expression, a.property,"
					" a.`name`, a.fid, b.source, b.classify, b.update_time, b.webname "
					"from news_template_basicinfo a, news_template_seed b "
					"where a.fid=b.basicinfo_fid and b.`status`=1";

	static unsigned int sql_len = strlen(sql);

	if (mysql_real_query(conn, sql, sql_len))
	{
		fprintf(stderr, "mysql_real_query fail, %s, %s\n", sql, mysql_error(conn));
		return -1;
	}

	MYSQL_RES* res = mysql_store_result(conn);
	if (res == NULL)
	{
		fprintf(stderr, "mysql_store_result fail, %s, %s\n", sql, mysql_error(conn));
		return -1;
	}

	int regfail_num = 0;
	int tmplt_num = 0;
	int record_num = 0;

	MYSQL_ROW row;
	while ((row = mysql_fetch_row(res)) != NULL)
	{
		record_num++;

		unsigned long int* lengths = mysql_fetch_lengths(res);

		int template_id = atoi(row[0]);

		int ret = snprintf(dict->buf + dict->avail, 10, "%d", template_id);
		assert(ret < 10 && dict->avail + ret < dict->buf_size);

		int bucketno = key_to_hash_bucket_index(dict->buf + dict->avail, ret, dict->hm_template);

		template_info_t* tmplt = (template_info_t*) hashmap_get(dict->hm_template, bucketno, dict->buf + dict->avail, ret);
		if (tmplt == NULL)
		{
			tmplt_num++;

			char* key = dict->buf + dict->avail;
			dict->avail += ret;
			dict->buf[dict->avail++] = 0;

			tmplt = (template_info_t*) tmplt_alloc_buf(sizeof(template_info_t), dict);
			tmplt->extract_tmplts = (tmplt_select_info_t*) tmplt_alloc_buf(sizeof(tmplt_select_info_t) * g_tmplt_extract_num, dict);
			tmplt->valid = true;
			tmplt->template_id = template_id;
			tmplt->source = alloc_and_copy_converted_gb18030_string(row[9], lengths[9], dict, tmplt->source_len);
			tmplt->classify = alloc_and_copy_converted_gb18030_string(row[10], lengths[10], dict, tmplt->classify_len);
			tmplt->webname_list = create_webname_list(dict, row[12], lengths[12]);
			set_time_field(&(tmplt->update_time), row[11], "%d-%d-%d %d:%d:%d");

			hashmap_put(dict->hm_template, bucketno, key, ret, tmplt);
		}

		tmplt_extract_type_enum extract_type = get_tmplt_extract_type(row[6]);
		if (extract_type == TMPLT_EXTRACT_UNKNOWN)
			continue;

		if (strcmp("select", row[4]) == 0)
		{
			tmplt->extract_tmplts[extract_type].select = create_tmplt_select_info(row[5], lengths[5], dict);

			if (tmplt->extract_tmplts[extract_type].select == NULL && row[5][0] == '<')
			{
				tmplt->extract_tmplts[extract_type].select = (tmplt_select_t*) tmplt_alloc_buf(sizeof(tmplt_select_t), dict);
				path_add_source_reg(row[5], lengths[5], dict, tmplt->extract_tmplts[extract_type].select);
			}
		}
		else if (strcmp("regular", row[4]) == 0)
		{
			tmplt->extract_tmplts[extract_type].select = (tmplt_select_t*) tmplt_alloc_buf(sizeof(tmplt_select_t), dict);
			path_add_source_reg(row[5], lengths[5], dict, tmplt->extract_tmplts[extract_type].select);
		}
		else
		{
			//fprintf(stderr, "unknown action %s\n", row[4]);
			continue;
		}

		if (extract_type == TMPLT_EXTRACT_PUBTIME)
		{
			tmplt->extract_tmplts[extract_type].date_pattern = alloc_and_copy_converted_gb18030_string(row[3], lengths[3], dict, tmplt->extract_tmplts[extract_type].date_pattern_len);
			tmplt->extract_tmplts[extract_type].date_regstr = convert_date_pattern_to_regstr(tmplt->extract_tmplts[extract_type].date_pattern, tmplt->extract_tmplts[extract_type].date_pattern_len, dict, tmplt->extract_tmplts[extract_type].date_regstr_len);
			if (tmplt->extract_tmplts[extract_type].date_regstr_len > 0)
			{
				tmplt->extract_tmplts[extract_type].date_reg = new regex_t;
				assert(tmplt->extract_tmplts[extract_type].date_reg);
				int error = 0;
				if (0 != (error = regcomp(tmplt->extract_tmplts[extract_type].date_reg, tmplt->extract_tmplts[extract_type].date_regstr, REG_EXTENDED)))
				{
					char error_buf[1024];
					regerror(error, tmplt->extract_tmplts[extract_type].date_reg, error_buf, 1024);
					//fprintf(stderr, "regex compile fail, date_regstr:%s, reason:%s\n", tmplt->extract_tmplts[extract_type].date_regstr, error_buf);
					regfree(tmplt->extract_tmplts[extract_type].date_reg);
					delete tmplt->extract_tmplts[extract_type].date_reg;
					tmplt->extract_tmplts[extract_type].date_reg = NULL;
				}
			}
		}
		tmplt->extract_tmplts[extract_type].has_img = atoi(row[1]) == 0 ? false : true;
		tmplt->extract_tmplts[extract_type].hold_tags = construct_hold_tags(row[2], lengths[2], dict);
	}

	//printf("[basic_info] template_num:%d record_num:%d regfail_num:%d\n", tmplt_num, record_num, regfail_num);
	mysql_free_result(res);
	return 0;
}

static MYSQL* get_mysql_connection(tmplt_mysql_config_t* mysql_info)
{
	MYSQL* conn = mysql_init(0);
	if (conn == NULL)
	{
		fprintf(stderr, "mysql_init fail\n");
		return NULL;
	}

	mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8");

	bool flag = false;

	while (mysql_info)
	{
		mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &mysql_info->connect_timeout);
		if (!mysql_real_connect(conn, mysql_info->host, mysql_info->user, mysql_info->password, mysql_info->dbname, mysql_info->port, 0, 0))
		{
			fprintf(stderr, "[extractor_template] mysql_real_connect fail, host:%s\n", mysql_info->host);
		}
		else
		{
			flag = true;
			break;
		}
		mysql_info = mysql_info->next;
	}
	if (!flag)
	{
		fprintf(stderr, "[extractor_template] mysql_real_connect all fail\n");
		return NULL;
	}
	return conn;
}

static bool is_template_valid(template_info_t* tmplt)
{
	if (tmplt == NULL || tmplt->extract_tmplts == NULL)
		return false;
	if (tmplt->extract_tmplts[TMPLT_EXTRACT_REALTITLE].select == NULL || tmplt->extract_tmplts[TMPLT_EXTRACT_ARTICLE].select == NULL)
		return false;
	return true;
}

static int compare_tmplt_select_path(tmplt_select_path_t* s1, tmplt_select_path_t* s2)
{
	if (s1 == NULL && s2 == NULL)
		return 0;
	else if (s1 == NULL)
		return 2;
	else if (s2 == NULL)
		return 1;

	tmplt_select_path_t* t1 = s1;
	tmplt_select_path_t* t2 = s2;
	for (int path_depth = 0;; path_depth++)
	{
		if (t1->item_type != t2->item_type)
			return -1;
		if (t1->item_type == TMPLT_PATH_ITEM_ATTR || t1->item_type == TMPLT_PATH_ITEM_ALTER_ATTR)
		{
			if (strcmp(t1->item.attr->name, t2->item.attr->name) != 0)
				return -1;
			if (strcmp(t1->item.attr->value, t2->item.attr->value) != 0)
				return -1;
		}
		else if (t1->item_type == TMPLT_PATH_ITEM_TAG || t1->item_type == TMPLT_PATH_ITEM_ALTER_TAG)
		{
			if (t1->item.tag->tag_type != t2->item.tag->tag_type)
				return -1;
		}
		else if (t1->item_type == TMPLT_PATH_ITEM_REG || t1->item_type == TMPLT_PATH_ITEM_SOURCE_REG)
		{
			if (strcmp(t1->item.reg->regstr, t2->item.reg->regstr) != 0)
				return -1;
		}
		t1 = t1->next;
		t2 = t2->next;
		if (t1 == NULL && t2 == NULL)
			return 0;
		else if (t1 == NULL)
			return 2;
		else if (t2 == NULL)
			return 1;
	}
	return 0;
}

//0, equal; 1, s1 more than s2; 2, s2 more than s1; -1, not equal
static int compare_tmplt_select(tmplt_select_t* s1, tmplt_select_t* s2)
{
	if (s1 == NULL && s2 == NULL)
		return 0;
	else if (s1 == NULL)
		return 2;
	else if (s2 == NULL)
		return 1;

	return compare_tmplt_select_path(s1->path, s2->path);
}

static bool is_equal_select_info(tmplt_select_info_t* s1, tmplt_select_info_t* s2)
{
	if (s1 == NULL && s2 == NULL)
		return true;
	else if (s1 == NULL)
		return false;
	else if (s2 == NULL)
		return false;
	if (0 == compare_tmplt_select(s1->select, s2->select))
		return true;
	return false;
}

static bool is_equal_template(template_info_t* tmplt1, template_info_t* tmplt2)
{
	if (tmplt1 == NULL && tmplt2 == NULL)
		return true;
	else if (tmplt1 == NULL)
		return false;
	else if (tmplt2 == NULL)
		return false;
	for (int i = 0; i < g_tmplt_extract_num; i++)
	{
		if (!is_equal_select_info(&tmplt1->extract_tmplts[i], &tmplt2->extract_tmplts[i]))
			return false;
	}
	return true;
}

static void fill_info_for_url_entry(templates_t* dicts)
{
	char key[10];
	int ret;

	tmplt_url_entry_t* url_entry;
	hashmap_iter_begin(dicts->hm_url_entry);
	while ((url_entry = (tmplt_url_entry_t*) hashmap_iter_next(dicts->hm_url_entry)) != NULL)
	{
		template_list_t* list = url_entry->templates;
		template_list_t* prev = NULL;
		while (list)
		{
			ret = snprintf(key, 10, "%d", list->template_id);
			key[ret] = 0;

			template_info_t* tmplt = (template_info_t*) hashmap_get(dicts->hm_template, key, ret);
			if (tmplt == NULL)
			{
				//fprintf(stderr, "url_entry:%s template_id:%d fail, template hasn't configured\n", url_entry->url_regex, list->template_id);
				if (prev != NULL)
				{
					prev->next = list->next;
					list = list->next;
					continue;
				}
				else if (list->next != NULL)
				{
					url_entry->templates = list->next;
					list = list->next;
					continue;
				}
				else
				{
					url_entry->templates = NULL;
					break;
				}
			}
			else
				list->template_info = tmplt;

			assert(list->template_info != NULL);
			prev = list;
			list = list->next;
		}
	}
}

static void remove_invalid_template(tmplt_url_entry_t* url_entry)
{
	assert(url_entry != NULL);
	template_list_t* list = url_entry->templates;
	template_list_t* prev = NULL;
	while (list)
	{
		if (list->template_info->valid == false)
		{
			tmplt_free_tmplt(list->template_info);
			if (prev != NULL)
			{
				prev->next = list->next;
				list = list->next;
				continue;
			}
			else if (list->next != NULL)
			{
				url_entry->templates = list->next;
				list = list->next;
				continue;
			}
			else
			{
				url_entry->templates = NULL;
				break;
			}
		}
		prev = list;
		list = list->next;
	}
}

static int tmplt_cmp_time(tm* t1, tm* t2)
{
	assert(t1 != NULL && t2 != NULL);
	if (t1->tm_year != t2->tm_year)
		return t1->tm_year - t2->tm_year;
	if (t1->tm_mon != t2->tm_mon)
		return t1->tm_mon - t2->tm_mon;
	if (t1->tm_yday != t2->tm_yday)
		return t1->tm_yday - t2->tm_yday;
	if (t1->tm_hour != t2->tm_hour)
		return t1->tm_hour - t2->tm_hour;
	if (t1->tm_min != t2->tm_min)
		return t1->tm_min - t2->tm_min;
	if (t1->tm_sec != t2->tm_sec)
		return t1->tm_sec - t2->tm_sec;
	return 0;
}

static bool re_order_by_uptime(tmplt_url_entry_t* url_entry)
{
	assert(url_entry != NULL && url_entry->templates != NULL);

	template_list_t* prev_updtime_max = NULL;
	template_list_t* uptime_max = url_entry->templates;
	tm* max_tm = &url_entry->templates->template_info->update_time;

	template_list_t* prev = url_entry->templates;
	template_list_t* list = url_entry->templates->next;
	while (list)
	{
		int ret = tmplt_cmp_time(max_tm, &list->template_info->update_time);
		if (ret < 0)
		{
			uptime_max = list;
			max_tm = &list->template_info->update_time;
			prev_updtime_max = prev;
		}
		prev = list;
		list = list->next;
	}

	if (prev_updtime_max != NULL)
	{
		prev_updtime_max->next = uptime_max->next;
		uptime_max->next = url_entry->templates;
		url_entry->templates = uptime_max;
	}

	if (max_tm->tm_year != 0 || max_tm->tm_mon != 0 || max_tm->tm_yday != 0)
		return true;
	else
		return false;
}

static void check_invalid_url_entry(templates_t* dicts)
{
	int invalid = 0;
	int valid_num = 0;

	template_info_t* tmplt;
	hashmap_iter_begin(dicts->hm_template);
	while ((tmplt = (template_info_t*) hashmap_iter_next(dicts->hm_template)) != NULL)
	{
		tmplt->valid = is_template_valid(tmplt);
		if (!tmplt->valid)
			invalid++;
		else
			valid_num++;
	}
	//fprintf(stdout, "invalid template:%d, valid template:%d\n", invalid, valid_num);

	invalid = 0;
	valid_num = 0;

	tmplt_url_entry_t* url_entry;
	hashmap_iter_begin(dicts->hm_url_entry);
	while ((url_entry = (tmplt_url_entry_t*) hashmap_iter_next(dicts->hm_url_entry)) != NULL)
	{
		remove_invalid_template(url_entry);
		if (url_entry->templates == NULL)
		{
			invalid++;
			url_entry->valid = false;
			//fprintf(stderr, "url_regex %s is invalid for it contains none templates\n", url_entry->url_regex);
			continue;
		}

		if (re_order_by_uptime(url_entry))
		{
			valid_num++;
			continue;
		}

		bool valid = true;
		template_list_t* pre = url_entry->templates;
		template_list_t* list = url_entry->templates->next;
		while (list != NULL)
		{
			if (!is_equal_template(url_entry->templates->template_info, list->template_info))
			{
				valid = false;
				//fprintf(stderr, "url_regex %s is invalid for it contains two diff templates\n", url_entry->url_regex);
				break;
			}
			list = list->next;
			pre = list;
		}
		url_entry->valid = valid;

		if (url_entry->valid)
			valid_num++;
		else
			invalid++;
	}
	//printf("%d invalid url_entry, %d valid url_entry\n", invalid, valid_num);
}

static int get_site(const char* src, int len, char* buf, int size)
{
	if (len < 9)
		return 0;
	const char* p = src;
	const char* end = src + len;
	char* dest = buf;
	char* dend = buf + size;

	if (strncmp("http://", p, 7) == 0)
		p += 7;
	while (p && p < end && dest < dend)
	{
		if (p[0] == '/')
		{
			p++;
			break;
		}
		*dest++ = *p++;
	}
	return dest - buf;
}

static int get_regex_site(char* src, char* buf, int size)
{
	int len = strlen(src);
	if (len < 9)
		return 0;
	char* p = src;
	char* end = src + len;
	char* dest = buf;
	char* dend = buf + size;

	if (p[0] == '^')
		p++;
	if (strncmp("http://", p, 7) == 0)
		p += 7;
	while (p && p < end && dest < dend)
	{
		if (p[0] == '\\')
		{
			p++;
			continue;
		}
		if (p[0] == '/')
		{
			p++;
			break;
		}
		if (p[0] == '[' || p[0] == ']' || p[0] == '+')
			return 0;
		*dest++ = *p++;
	}
	return dest - buf;
}

static const char* to_be_repaired_strs[] =
{ "\\", "	", 0 };
static const char* repair_tmp_strs[] =
{ "\\\\", "", 0 };

static int repair_json_str_2(char *src, int len, char* buf, int size)
{
	char* pd = buf;
	char* p = src;
	char* end = src + len;
	while (p < end)
	{
		int index = -1;
		for (int i = 0; to_be_repaired_strs[i]; i++)
		{
			if (strncmp(p, to_be_repaired_strs[i], strlen(to_be_repaired_strs[i])) == 0)
			{
				index = i;
				break;
			}
		}
		if (index != -1)
		{
			memcpy(pd, repair_tmp_strs[index], strlen(repair_tmp_strs[index]));
			pd += strlen(repair_tmp_strs[index]);
			p += strlen(to_be_repaired_strs[index]);
		}
		else
			*pd++ = *p++;
	}
	pd[0] = 0;
	return pd - buf;
}

static int repair_json_str_1(const char *src, int len, char* buf, int size)
{
	const char* p = src;
	const char* end = src + len;
	char* pd = buf;
	char* pdend = buf + size;

	bool scape = false;
	bool in_0 = false;
	bool in_1 = false;
	bool start = false;
	bool added = false;
	while (p < end && pd < pdend)
	{
		if (p[0] == ' ')
		{
			*pd++ = *p++;
			continue;
		}
		if (p[0] == '\\')
			scape = true;
		else
			scape = false;
		if (!scape)
		{
			if (p[0] == '{')
				in_0 = true;
			else if (p[0] == '}')
				in_0 = false;
			if (in_0)
			{
				if (in_1 == false && p[0] == '"')
				{
					in_1 = true;
					start = false;
				}
				else if (in_1 == true && p[0] == '"')
					in_1 = false;
				else if (in_1 == false && p[0] == ',')
				{
					start = true;
				}
				else if (start)
				{
					*pd++ = '"';
					start = false;
					added = true;
				}
				else if (p[0] == ':' && added)
				{
					*pd++ = '"';
					added = false;
				}
			}
		}
		*pd++ = *p++;
	}
	int nlen = pd - buf;
	buf[nlen] = 0;
	return nlen;
}

static int repair_json_str(const char *src, int len, char* buf, int size)
{
	char buf1[TMPLT_LLEN];
	int len1 = repair_json_str_1(src, len, buf1, TMPLT_LLEN);
	int len2 = repair_json_str_2(buf1, len1, buf, size);
	return len2;
}

static json_t* tmplt_json_compile(const char* str, int len)
{
	json_error_t error;
	json_t* json = json_loadb(str, len, JSON_DECODE_ANY, &error);
	if (json == NULL)
	{
//		fprintf(stderr, "json first parse error, reason:%s, str:%s\n", error.text, str);
		char buf[TMPLT_LLEN];
		int nlen = repair_json_str(str, len, buf, TMPLT_LLEN);
		json = json_loadb(buf, nlen, JSON_DECODE_ANY, &error);
		if (json == NULL)
		{
			//fprintf(stderr, "json parse error after repair, reason:%s, str:%s\n", error.text, buf);
			return false;
		}
	}

	if (!json_is_array(json))
	{
		//fprintf(stderr, "it is not json array, str:%s\n", str);
		json_delete(json);
		return NULL;
	}
	return json;
}

#define FREE_JSON(a) if (a && a->json) { json_delete(a->json); a->json=NULL;}

static void tmplt_free_regex(templates_t* dicts)
{
	tmplt_url_entry_t* url_entry;
	hashmap_iter_begin(dicts->hm_url_entry);
	while ((url_entry = (tmplt_url_entry_t*) hashmap_iter_next(dicts->hm_url_entry)) != NULL)
	{
		if (url_entry->regex)
		{
			regfree(url_entry->regex);
			delete url_entry->regex;
			url_entry->regex = NULL;
		}
	}

	template_info_t* tmplt;
	hashmap_iter_begin(dicts->hm_template);
	while ((tmplt = (template_info_t*) hashmap_iter_next(dicts->hm_template)) != NULL)
	{
		tmplt_free_tmplt(tmplt);
	}
}

static void group_url_entry_by_site(templates_t* dicts)
{
	tmplt_url_entry_t* url_entry;
	hashmap_iter_begin(dicts->hm_url_entry);
	while ((url_entry = (tmplt_url_entry_t*) hashmap_iter_next(dicts->hm_url_entry)) != NULL)
	{
		if (dicts->avail + TMPLT_SLEN >= dicts->buf_size)
		{
			fprintf(stderr, "buf not enough\n");
			assert(false);
		}
		unsigned int ret = get_regex_site(url_entry->url_regex, dicts->buf + dicts->avail, TMPLT_SLEN);
		if (ret == 0)
		{
			memcpy(dicts->buf + dicts->avail, "default", 7);
			ret = 7;
		}
		dicts->buf[dicts->avail + ret] = 0;
		char *key = dicts->buf + dicts->avail;
		int bucketno = key_to_hash_bucket_index(key, ret, dicts->hm_site_regex);
		tmplt_url_entry_list_t* list = (tmplt_url_entry_list_t*) hashmap_get(dicts->hm_site_regex, bucketno, key, ret);
		if (list == NULL)
		{
			dicts->avail += ret;
			dicts->buf[dicts->avail++] = 0;

			list = (tmplt_url_entry_list_t*) tmplt_alloc_buf(sizeof(tmplt_url_entry_list_t), dicts);
			list->url_entry = url_entry;

			hashmap_put(dicts->hm_site_regex, bucketno, key, ret, list);
		}
		else
		{
			tmplt_url_entry_list_t* tmp_list = (tmplt_url_entry_list_t*) (dicts->buf + dicts->avail);
			dicts->avail += sizeof(tmplt_url_entry_list_t);

			tmp_list->url_entry = url_entry;
			tmp_list->next = list->next;
			list->next = tmp_list;
		}
	}
}

int load_templates(templates_t* dicts)
{
	int error = -1;

	MYSQL* conn = get_mysql_connection(dicts->mysql_info);
	if (conn == NULL)
		return -1;

	if ((error = load_regex_info(conn, dicts)))
		goto FINISH;
	if ((error = load_basic_info(conn, dicts)))
		goto FINISH;
	if ((error = load_next_url_info(conn, dicts)))
		goto FINISH;

	fill_info_for_url_entry(dicts);
	check_invalid_url_entry(dicts);
	group_url_entry_by_site(dicts);

	error = 0;

	FINISH: mysql_close(conn);
	return error;
}

void del_templates(templates_t* dict)
{
	if (dict == NULL)
		return;
	tmplt_free_mysql_config(dict);
	tmplt_free_regex(dict);
	if (dict->hm_url_entry)
		hashmap_destroy(dict->hm_url_entry);
	if (dict->hm_template)
		hashmap_destroy(dict->hm_template);
	if (dict->hm_site_regex)
		hashmap_destroy(dict->hm_site_regex);
	if (dict->buf)
		free(dict->buf);
	delete dict;
}

int tmplt_findmax(const char * str1, int lenstr1, const char * str2, int lenstr2, tmplt_result_t* output)
{
	if (sizeof(int*) * (lenstr1 + 1) + sizeof(int) * (lenstr2 + 1) * (lenstr1 + 1) >= output->size)
	{
		return -1;
	}

	int avail = 0;
	int** len = (int**) (output->buf);
	avail += sizeof(int*) * (lenstr1 + 1);

	for (int i = 0; i <= lenstr1; i++)
	{
		len[i] = (int*) (output->buf + avail);
		avail += sizeof(int) * (lenstr2 + 1);
	}
	for (int i = 0; i <= lenstr1; i++)
	{
		for (int j = lenstr2; j >= 0; j--)
		{
			len[i][j] = 0;
		}
	}
	int max = 0;
	for (int i = 1; i <= lenstr1; i++)
	{
		for (int j = lenstr2; j > 0; j--)
		{
			if (str1[i - 1] == str2[j - 1])
			{
				len[i][j] = (len[i - 1][j - 1] + 1);
				max = max > len[i][j] ? max : len[i][j];
			}
		}
	}
	return max;
}

tmplt_url_entry_t* find_url_entry(const char* url, int url_len, templates_t* dict, tmplt_result_t* output)
{
	char site[TMPLT_MLEN];
	int len = get_site(url, url_len, site, TMPLT_MLEN);
	if (len <= 0)
		return NULL;
	site[len] = 0;

	tmplt_url_entry_list_t* list = (tmplt_url_entry_list_t*) hashmap_get(dict->hm_site_regex, site, len);
	if (list == NULL)
		list = (tmplt_url_entry_list_t*) hashmap_get(dict->hm_site_regex, "default", 7);
	if (list == NULL)
		return NULL;

	int match_score = 0;
	tmplt_url_entry_list_t* url_entry = NULL;

	bool find = false;
	while (list)
	{
		if (list->url_entry->valid && regexec(list->url_entry->regex, url, 0, NULL, 0) == 0)
		{
			template_list_t* dir_list = list->url_entry->templates;
			while (dir_list)
			{
				int lcs_len = tmplt_findmax(url + 7, url_len - 7, dir_list->template_info->webname_list->str, strlen(dir_list->template_info->webname_list->str), output);
				if (lcs_len == -1)
					return NULL;
				if (lcs_len > match_score)
				{
					match_score = lcs_len;
					url_entry = list;
				}
				dir_list = dir_list->next;
			}
			find = true;
		}
		list = list->next;
	}
	if (find)
		return url_entry->url_entry;
	return NULL;
}

struct tmplt_extract_visit_t
{
	int template_id;
	const char* url;
	const char* page;
	char* buf;
	unsigned int buf_size;
	unsigned int avail;

	tmplt_select_path_t* select_path;
	tmplt_tagtype_list_t* hold_tags;

	tmplt_tagtype_list_t* end_tag;
	tmplt_attr_list_t* end_attr;
	tmplt_regex_list_t* end_reg;

	tmplt_tagtype_list_t* ex_tag;
	tmplt_attr_list_t* ex_attr;
	tmplt_regex_list_t* ex_reg;

	html_node_list_t* selected_nodes;
	html_node_list_t* selected_head;
	html_node_list_t* img_list;
	html_node_list_t* img_head;
	bool selected;
	bool has_img;
	int ex_st;
	html_node_t* goto_node;
	int res;
};

bool tmplt_is_hit(tmplt_tagtype_list_t* end_tag, html_tag_t* tag, tmplt_attr_list_t* end_attr, tmplt_regex_list_t* end_reg)
{
	bool end_flag = false;
	while (end_tag)
	{
		if (end_tag->tag_type == tag->tag_type)
		{
			end_flag = true;
			break;
		}
		end_tag = end_tag->next;
	}

	if (!end_flag)
	{
		while (end_attr)
		{
			const char* attr_value = NULL;
			if (end_attr->attr_type == ATTR_UNKNOWN)
				attr_value = get_attribute_value(tag, end_attr->name);
			else
				attr_value = get_attribute_value(tag, end_attr->attr_type);
			if (attr_value && strcmp(attr_value, end_attr->value) == 0)
			{
				end_flag = true;
				break;
			}
			end_attr = end_attr->next;
		}
	}
//
//	if (!end_flag && tag->text)
//	{
//		while (end_reg)
//		{
//			regmatch_t matches[10];
//			if (0 == regexec(end_reg->reg, tag->text, 10, matches, 0))
//			{
//				end_flag = true;
//				for (int i = 0; i < 10; i++)
//				{
//					char ch = tag->text[matches[i].rm_eo];
//					tag->text[matches[i].rm_eo] = 0;
//					printf("%s\n", tag->text[matches[i].rm_so]);
//					tag->text[matches[i].rm_eo] = ch;
//				}
//				break;
//			}
//			end_reg = end_reg->next;
//		}
//	}
	return end_flag;
}

struct tmplt_visit_offset_t
{
	int node_offset;
	html_node_t* finded;
};

static int start_visit_offset(html_tag_t* tag, void* result, int flag)
{
	if (tag->tag_type == TAG_ROOT || tag->tag_type == TAG_HTML || tag->tag_type == TAG_BODY)
		return VISIT_NORMAL;
	if (tag->tag_type == TAG_DOCTYPE || tag->tag_type == TAG_COMMENT || tag->tag_type == TAG_HEAD || tag->tag_type == TAG_SCRIPT || tag->tag_type == TAG_STYLE || tag->tag_type == TAG_LINK)
		return VISIT_SKIP_CHILD;
	if (tag->page_offset <= 0 || tag->nodelength <= 0)
		return VISIT_NORMAL;

	int tag_offset = tag->page_offset;
	int tag_endset = tag->page_offset + tag->nodelength;

	tmplt_visit_offset_t* visit = (tmplt_visit_offset_t*) result;
	int node_offset = visit->node_offset;

	if (node_offset == tag_offset)
	{
		visit->finded = tag->html_node;
		return VISIT_FINISH;
	}
	else if (node_offset > tag_offset && node_offset <= tag_endset)
	{
		return VISIT_NORMAL;
	}
	else if (node_offset > tag_endset)
		return VISIT_SKIP_CHILD;
	else if (node_offset < tag_offset)
	{
		visit->finded = tag->html_node;
		return VISIT_FINISH;
	}
	return VISIT_NORMAL;
}

static html_node_t* find_node_by_source_offset(const char* page, html_tree_t* tree, int offset)
{
	tmplt_visit_offset_t visit;
	memset(&visit, 0, sizeof(visit));
	visit.node_offset = offset;

	html_tree_visit(tree, start_visit_offset, NULL, &visit, 0);

	return visit.finded;
}

static int add_selected_nodes_before(html_node_t* end_node, tmplt_extract_visit_t* visit)
{
	if (end_node == NULL || visit == NULL)
		return 0;
	html_node_t* upnode = end_node;
	while (!(upnode->user_ptr != NULL && upnode->user_ptr != (void*) (&visit->ex_st) && ((tmplt_select_path_t*) upnode->user_ptr)->next == NULL))
	{
		html_node_t* prev_node = upnode->prev;
		while (prev_node)
		{
			html_node_list_t* list = (html_node_list_t*) alloc_buf(visit->buf, visit->buf_size, visit->avail, sizeof(html_node_list_t));
			if (list == NULL)
				return -1;
			list->html_node = prev_node;
			if (visit->selected_nodes == NULL)
			{
				visit->selected_nodes = list;
				visit->selected_head = list;
			}
			else
			{
				list->next = visit->selected_nodes;
				visit->selected_nodes->prev = list;
				visit->selected_nodes = list;
				visit->selected_head = list;
			}
			prev_node = prev_node->prev;
		}
		upnode = upnode->parent;
	}
	return 0;
}

static int start_visit_for_tmplt(html_tag_t* tag, void* result, int flag)
{
	if (tag->tag_type == TAG_ROOT)
		return VISIT_NORMAL;

	tmplt_extract_visit_t* visit = (tmplt_extract_visit_t*) result;
	html_node_t* node = tag->html_node;
	tmplt_select_path_t* rt_path = visit->select_path;

	if (visit->goto_node != NULL && node != visit->goto_node)
	{
		if (flag == 0)
			node->user_ptr = NULL;
		return VISIT_NORMAL;
	}
	else if (visit->goto_node != NULL && rt_path == NULL)
	{
		visit->selected = true;
		visit->goto_node = NULL;
	}
	else
	{
		if (flag == 0)
			node->user_ptr = NULL;
	}

	if (rt_path)
	{
		if (rt_path->item_type == TMPLT_PATH_ITEM_TAG || rt_path->item_type == TMPLT_PATH_ITEM_ALTER_TAG)
		{
			if (tag->tag_type == rt_path->item.tag->tag_type)
			{
				node->user_ptr = rt_path;
				visit->select_path = rt_path->next;
				if (rt_path->next == NULL)
				{
					visit->selected = true;
				}
			}
		}
		else if (rt_path->item_type == TMPLT_PATH_ITEM_ATTR || rt_path->item_type == TMPLT_PATH_ITEM_ALTER_ATTR)
		{
			const char* value = NULL;
			if (rt_path->item.alter_attr->attr_type != ATTR_UNKNOWN)
				value = get_attribute_value(tag, rt_path->item.attr->attr_type);
			else
				value = get_attribute_value(tag, rt_path->item.attr->name);
			if (value != NULL && strcmp(value, rt_path->item.attr->value) == 0)
			{
				node->user_ptr = rt_path;
				visit->select_path = rt_path->next;
				if (rt_path->next == NULL)
				{
					visit->selected = true;
				}
				else if (rt_path->next->item_type == TMPLT_PATH_ITEM_SOURCE_REG)
				{
					if (tag->page_offset > 0 && tag->nodelength > 0)
					{
						if (rt_path->next->item.source_reg->reg == NULL)
						{
							debuginfo(TMPLT_EXTRACT, "source_reg regex is NULL, fail select path");
							return VISIT_FINISH;
						}
						const char* source_start = visit->page + tag->page_offset;
						char* source_end = (char*) (visit->page + tag->page_offset + tag->nodelength);
						char ch = source_end[0];
						source_end[0] = 0;

						bool flag = false;
						regmatch_t matches[1];
						if (0 == regexec(&(rt_path->next->item.source_reg->reg[visit->res]), source_start, 1, matches, 0) && matches[0].rm_so != -1)
						{
							html_node_t* target = find_node_by_source_offset(visit->page, node->owner, matches[0].rm_so + tag->page_offset);
							if (target)
							{
								flag = true;
								visit->goto_node = target;
								target->user_ptr = rt_path->next;
								visit->select_path = rt_path->next->next;
							}
						}
						source_end[0] = ch;
						if (!flag)
							return VISIT_FINISH;
					}
				}

				if (flag == 0)
				{ //匹配上后以当前节点开始下一次匹配检查
					if (visit->select_path)
					{
						tmplt_select_path_t* bak_path = NULL;
						do
						{
							bak_path = visit->select_path;
							int this_visit = start_visit_for_tmplt(tag, visit, 1);
							if (this_visit == VISIT_FINISH)
								return VISIT_FINISH;
							if (this_visit == VISIT_ERROR)
								return VISIT_ERROR;
						} while (bak_path != visit->select_path && visit->select_path);
					}
				}
			}
		}
		else if (rt_path->item_type == TMPLT_PATH_ITEM_REG)
		{
			if (rt_path->item.reg->reg != NULL && tag->text != NULL)
			{
				regmatch_t matches[1];
				if ((0 == regexec(rt_path->item.reg->reg, tag->text, 1, matches, 0)) && matches[0].rm_so > 0)
				{
					html_node_list_t* list = (html_node_list_t*) alloc_buf(visit->buf, visit->buf_size, visit->avail, sizeof(html_node_list_t));
					if (list == NULL)
						return VISIT_ERROR;
					list->html_node = node;
					visit->selected_nodes = list;
					visit->selected_head = list;
					return VISIT_FINISH;
				}
			}
		}
		else if (rt_path->item_type == TMPLT_PATH_ITEM_SOURCE_REG)
		{
			if (tag->page_offset > 0 && tag->nodelength > 0)
			{
				const char* source_start = visit->page + tag->page_offset;
				char* source_end = (char*) (visit->page + tag->page_offset + tag->nodelength);
				char ch = source_end[0];
				source_end[0] = 0;

				bool flag = false;
				regmatch_t matches[1];
				if ((0 == regexec(&(rt_path->item.source_reg->reg[visit->res]), source_start, 1, matches, 0)) && matches[0].rm_so != -1)
				{
					html_node_t* target = find_node_by_source_offset(visit->page, node->owner, matches[0].rm_so + tag->page_offset);
					if (target)
					{
						flag = true;
						visit->goto_node = target;
						target->user_ptr = rt_path;
						visit->select_path = rt_path->next;
					}
				}
				source_end[0] = ch;
				if (!flag)
					return VISIT_SKIP_CHILD;
			}
		}
	}
	else if (visit->selected)
	{
		bool end_flag = tmplt_is_hit(visit->end_tag, tag, visit->end_attr, visit->end_reg);
		if (end_flag)
		{
			if (0 != add_selected_nodes_before(node, visit))
				return VISIT_ERROR;
			return VISIT_FINISH;
		}
		bool ex_flag = tmplt_is_hit(visit->ex_tag, tag, visit->ex_attr, visit->ex_reg);
		if (ex_flag)
		{
			html_node_t* tmpnode = node;
			while (tmpnode->user_ptr == NULL)
			{
				tmpnode->user_ptr = (void*) (&visit->ex_st);
				tmpnode = tmpnode->parent;
			}
			return VISIT_SKIP_CHILD;
		}
	}
	return VISIT_NORMAL;
}

bool is_hit_hold_tags(tmplt_tagtype_list_t* list, html_tag_t* tag)
{
	if (list == NULL)
		return true;
	if (tag->tag_type == TAG_PURETEXT)
		return true;
	tmplt_tagtype_list_t* nlist = list;
	while (nlist)
	{
		if (nlist->tag_type == tag->tag_type)
			return true;
		nlist = nlist->next;
	}
	return false;
}

static bool simple_img_filter(html_node_t* node)
{
	const char* height = get_attribute_value(&node->html_tag, ATTR_HEIGHT);
	const char* width = get_attribute_value(&node->html_tag, ATTR_WIDTH);
	if (height && atoi(height) <= 15)
	{
		return false;
	}
	if (width && atoi(width) <= 15)
	{
		return false;
	}
	return true;
}

static int start_visit_for_add_img_nodes(html_tag_t* tag, void* result, int flag)
{
	tmplt_extract_visit_t* visit = (tmplt_extract_visit_t*) result;
	html_node_t* node = tag->html_node;
	if (node->user_ptr == NULL && tag->tag_type == TAG_IMG && simple_img_filter(node))
	{
		html_node_list_t* list = (html_node_list_t*) alloc_buf(visit->buf, visit->buf_size, visit->avail, sizeof(html_node_list_t));
		if (list == NULL)
			return VISIT_ERROR;
		list->html_node = node;
		if (visit->img_list == NULL)
		{
			visit->img_list = list;
			visit->img_head = list;
		}
		else
		{
			visit->img_list->next = list;
			list->prev = visit->img_list;
			visit->img_list = list;
		}
	}
	return VISIT_NORMAL;
}

static int start_visit_for_add_nodes(html_tag_t* tag, void* result, int flag)
{
	tmplt_extract_visit_t* visit = (tmplt_extract_visit_t*) result;
	html_node_t* node = tag->html_node;
	if (node->user_ptr == NULL && is_hit_hold_tags(visit->hold_tags, tag))
	{
		html_node_list_t* list = (html_node_list_t*) alloc_buf(visit->buf, visit->buf_size, visit->avail, sizeof(html_node_list_t));
		if (list == NULL)
			return VISIT_ERROR;
		list->html_node = node;
		if (visit->selected_nodes == NULL)
		{
			visit->selected_nodes = list;
			visit->selected_head = list;
		}
		else
		{
			visit->selected_nodes->next = list;
			list->prev = visit->selected_nodes;
			visit->selected_nodes = list;
		}
		return VISIT_SKIP_CHILD;
	}
	else if (node->user_ptr != NULL && node->user_ptr == (void*) &visit->ex_st)
	{
		bool child_st = false;
		html_node_t* child = tag->html_node->child;
		while (child)
		{
			if (child->user_ptr != NULL && child->user_ptr == (void*) &visit->ex_st)
			{
				child_st = true;
				break;
			}
			child = child->next;
		}
		if (child_st)
			return VISIT_NORMAL;
		else
			return VISIT_SKIP_CHILD;
	}
	return VISIT_NORMAL;
}

static int finish_visit_for_tmplt(html_tag_t* tag, void* result)
{
	if (tag->tag_type == TAG_ROOT)
		return VISIT_NORMAL;

	tmplt_extract_visit_t* visit = (tmplt_extract_visit_t*) result;
	html_node_t* node = tag->html_node;

	if (node->user_ptr != NULL && node->user_ptr != (void*) (&visit->ex_st) && ((tmplt_select_path_t*) node->user_ptr)->next == NULL)
	{
		visit->select_path = (tmplt_select_path_t*) node->user_ptr;
		visit->selected = false;

		bool flag = false;
		html_node_t *child = node->child;
		while (child)
		{
			if (child->user_ptr != NULL && child->user_ptr == (void*) (&visit->ex_st))
			{
				flag = true;
				break;
			}
			child = child->next;
		}
		if (!flag)
			node->user_ptr = NULL;
		if (VISIT_ERROR == html_node_visit(node, start_visit_for_add_nodes, NULL, visit, 0))
			return VISIT_ERROR;
		if (visit->has_img)
		{
			if (VISIT_ERROR == html_node_visit(node, start_visit_for_add_img_nodes, NULL, visit, 0))
				return VISIT_ERROR;
		}
		return VISIT_FINISH;
	}
	return VISIT_NORMAL;
}

static void set_visit_t(tmplt_extract_visit_t* visit, tmplt_select_info_t* ext)
{
	visit->selected_head = NULL;
	visit->selected_nodes = NULL;
	visit->img_list = NULL;
	visit->img_head = NULL;
	visit->selected = false;
	visit->end_attr = ext->select->end_attr;
	visit->end_tag = ext->select->end_tag;
	visit->end_reg = ext->select->end_reg;
	visit->ex_attr = ext->select->ex_attr;
	visit->ex_tag = ext->select->ex_tag;
	visit->ex_reg = ext->select->ex_reg;
	visit->select_path = ext->select->path;
	visit->hold_tags = ext->hold_tags;
	visit->has_img = ext->has_img;
}

static int tmplt_keep_range(char* buf, int len, int start, int end)
{
	int rlen = end - start;
	for (int i = 0; i < rlen; i++)
	{
		buf[i] = buf[i + start];
	}
	buf[rlen] = 0;
	return rlen;
}

static int tmplt_remove_range(char* buf, int len, int start, int end)
{
	int rlen = end - start;
	int nlen = len - rlen;
	for (int i = start; i <= nlen - start; i++)
	{
		buf[i] = buf[i + rlen];
	}
	buf[nlen] = 0;
	return nlen;
}

int tmplt_apply_exreg_rule(char* buf, int len, tmplt_regex_list_t* ex_reg)
{
	if (ex_reg == NULL || len <= 0)
		return len;

#define TMPLT_REG_RESULT_NUM 100
	regmatch_t matches[TMPLT_REG_RESULT_NUM];
	int nlen = len;
	tmplt_regex_list_t* reg = ex_reg;
	while (reg)
	{
		int counter = 0;
		while (reg->reg && (0 == regexec(reg->reg, buf, TMPLT_REG_RESULT_NUM, matches, 0)))
		{
			if (counter++ >= 20)
				break;
			int removed_len = 0;
			for (int i = 0; i < TMPLT_REG_RESULT_NUM; i++)
			{
				if (matches[i].rm_so == -1)
					break;
				debuginfo(TMPLT_EXTRACT, "ex-reg(%s) matched [%d], start:%d, end:%d", reg->regstr, i, matches[i].rm_so, matches[i].rm_eo);
				nlen = tmplt_remove_range(buf, nlen, matches[i].rm_so - removed_len, matches[i].rm_eo - removed_len);
				removed_len += (matches[i].rm_eo - matches[i].rm_so);
				debuginfo(TMPLT_EXTRACT, "after remove matched part, len:%d,strlen:%d,%s\n", nlen, strlen(buf), buf);
			}
		}
		reg = reg->next;
	}
#undef TMPLT_REG_RESULT_NUM
	return nlen;
}

static int extract_use_tmplt(tmplt_result_t* output, template_info_t* tmplt, html_tree_t* tree, const char* page, const char* url, int res)
{
	tmplt_extract_visit_t visit;
	memset(&visit, 0, sizeof(tmplt_extract_visit_t));
	visit.buf = output->buf;
	visit.buf_size = output->size;
	visit.avail = output->avail;
	visit.page = page;
	visit.url = url;
	visit.template_id = tmplt->template_id;
	if (res < 0)
		visit.res = 0;
	else
		visit.res = res % TMPLT_REGEX_RES_NUM;

	for (int i = 0; i < g_tmplt_extract_num; i++)
	{
		if (tmplt->extract_tmplts[i].select)
		{
			debuginfo(TMPLT_EXTRACT, "begin extract %s", tmplt_extract_type_desp[i]);

			set_visit_t(&visit, &tmplt->extract_tmplts[i]);

			if (VISIT_ERROR == html_tree_visit(tree, start_visit_for_tmplt, finish_visit_for_tmplt, &visit, 0))
				return -1;

			if (visit.has_img)
				output->extract_result[i].selected_imgs = visit.img_head;
			else
				output->extract_result[i].selected_imgs = NULL;

			output->extract_result[i].selected = visit.selected_head;
			output->extract_result[i].str = visit.buf + visit.avail;
			output->extract_result[i].str_len = 0;

			bool nline = false;
			html_node_list_t* list = visit.selected_head;
			while (list && visit.buf_size > (visit.avail + 1))
			{
				if (nline)
					visit.buf[visit.avail++] = '\n';
				int len = html_node_extract_content(list->html_node, visit.buf + visit.avail, visit.buf_size - visit.avail);
				visit.avail += len;
				if (list->html_node->html_tag.tag_type == TAG_P || list->html_node->html_tag.tag_type == TAG_BR)
					nline = true;
				else
					nline = false;
				list = list->next;
			}
			output->extract_result[i].str_len = &visit.buf[visit.avail] - output->extract_result[i].str;
			visit.buf[visit.avail++] = 0;

			debuginfo(TMPLT_EXTRACT, "%s before ex-reg,len:%d,strlen:%d,%s\n", tmplt_extract_type_desp[i], output->extract_result[i].str_len, strlen(output->extract_result[i].str), output->extract_result[i].str);
			output->extract_result[i].str_len = tmplt_apply_exreg_rule(output->extract_result[i].str, output->extract_result[i].str_len, visit.ex_reg);
			output->extract_result[i].ex_reg = visit.ex_reg;

			if (tmplt->extract_tmplts[i].date_reg && output->extract_result[i].str_len > 0)
			{
				regmatch_t matches[1];
				if (0 == regexec(tmplt->extract_tmplts[i].date_reg, output->extract_result[i].str, 1, matches, 0))
				{
					output->extract_result[i].str_len = tmplt_keep_range(output->extract_result[i].str, output->extract_result[i].str_len, matches[0].rm_so, matches[0].rm_eo);
				}
				else
				{
					const char* to_be_repaired[] =
					{ "&nbsp;", " ", 0 };
					const char* repaired[] =
					{ "", "", 0 };
					char* buf = &visit.buf[visit.avail];
					int nlen = repair_regex_str(output->extract_result[i].str, output->extract_result[i].str_len, buf, visit.buf_size - visit.avail, to_be_repaired, repaired);

					if (0 == regexec(tmplt->extract_tmplts[i].date_reg, buf, 1, matches, 0))
					{
						debuginfo(TMPLT_EXTRACT, "%s rm_so:%d rm_eo:%d", buf, matches[0].rm_so, matches[0].rm_eo);
						output->extract_result[i].str_len = tmplt_keep_range(buf, nlen, matches[0].rm_so, matches[0].rm_eo);
						debuginfo(TMPLT_EXTRACT, "%s\n", buf);

						output->extract_result[i].str = buf;
						visit.avail += nlen + 1;
					}
					else
					{
						debuginfo(TMPLT_EXTRACT, "date fail, regstr:%s, str:%s", tmplt->extract_tmplts[i].date_regstr, buf);
						output->extract_result[i].str[0] = 0;
						output->extract_result[i].str_len = 0;
					}
				}
			}
		}
		else
		{
			output->extract_result[i].selected = NULL;
			output->extract_result[i].selected_imgs = NULL;
			output->extract_result[i].str = NULL;
			output->extract_result[i].str_len = 0;
			output->extract_result[i].ex_reg = NULL;
		}
	}
	output->avail = visit.avail;

	return 0;
}

int try_template_extract(templates_t* dict, const char* page, int page_len, const char* url, int url_len, html_tree_t* tree, tmplt_result_t* output, int res)
{
	assert(tree != NULL && output != NULL && output->buf != NULL);
	if ((unsigned int) page_len * 2 >= output->size)
		return -1;

	debuginfo_on(TMPLT_EXTRACT);

	output->avail = 0;
	timeinit();timestart();
	tmplt_url_entry_t* url_entry = find_url_entry(url, url_len, dict, output);
	timeend("template", "entry");

	if (url_entry == NULL)
	{
		dumpdebug(TMPLT_EXTRACT, TMPLT_EXTRACT);
		return -1;
	}

	timestart();
	template_info_t* tmplt = url_entry->templates->template_info;

	debuginfo(TMPLT_EXTRACT, "select template:%d\n", tmplt->template_id);

	if (0 != extract_use_tmplt(output, tmplt, tree, page, url, res))
		return -1;

	set_tm_info_by_date_pattern(tmplt->extract_tmplts[TMPLT_EXTRACT_PUBTIME].date_pattern, tmplt->extract_tmplts[TMPLT_EXTRACT_PUBTIME].date_pattern_len, output->extract_result[TMPLT_EXTRACT_PUBTIME].str, output->extract_result[TMPLT_EXTRACT_PUBTIME].str_len, &output->pubtime_tm);

	if (output->avail + tmplt->classify_len + 1 < output->size)
	{
		output->classify = output->buf + output->avail;
		memcpy(output->classify, tmplt->classify, tmplt->classify_len);
		output->avail += tmplt->classify_len;
		output->classify[output->avail++] = 0;
		output->classify_len = tmplt->classify_len;
	}
	else
	{
		output->classify = NULL;
		output->classify_len = 0;
		fprintf(stderr, "output's buffer not enough to save classify field\n");
	}

	if (output->avail + tmplt->source_len + 1 < output->size)
	{
		output->source = output->buf + output->avail;
		memcpy(output->source, tmplt->source, tmplt->source_len);
		output->avail += tmplt->source_len;
		output->buf[output->avail++] = 0;
		output->source_len = tmplt->source_len;
	}
	else
	{
		output->source = NULL;
		output->source_len = 0;
		fprintf(stderr, "output's buffer not enough to save source field\n");
	}
	timeend("template", "extract");

	dumpdebug(TMPLT_EXTRACT, TMPLT_EXTRACT);
	return 0;
}


/*
 * easou_extractor_stdata_video.cpp
 *
 *  Created on: 2013-09-06
 *
 *  Author: round
 */

#include <string>
#include "easou_extractor_stdata_video.h"

#include "easou_html_tree.h"
#include "easou_vhtml_tree.h"
#include "easou_vhtml_basic.h"
#include "easou_html_extractor.h"
#include "easou_html_attr.h"
#include "html_text_utils.h"
#include "./thrift/StructData_types.h"

#include "easou_mark_markinfo.h"
#include "easou_mark_func.h"

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>

#include <stack>


using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using boost::shared_ptr;

struct extract_img_link
{
	StructureData *stdata;
	int flag_img;
	int flag_link;
	const char *url;
	char *title;
	char *first;//第一集链接
	int len_first;
	char *end;//最后一集链接
	int flag;
	int pagetype;
};


//去掉字符串首尾的空格
static void clean_string(char *str)
{
	if(str == NULL || str[0] == '\0')
		return;
	char *start = str - 1;
	char *end = str;
	char *p = str;
	while(*p)
	{
		switch(*p)
		{
			case ' ':
			case '\r':
			case '\n':
			case '\t':
			case '_':
			case '-':
				{
					if(start + 1==p)
					{
						start = p;
					}
				}
				break;
			default:
				break;
		}
		++p;
	}
	//现在来到了字符串的尾部 反向向前
	--p;
	++start;
	if(*start == 0)
	{
		//已经到字符串的末尾了
		*str = 0 ;
		return;
	}
	end = p + 1;
	while(p > start)
	{
		switch(*p)
		{
			case ' ':
			case '\r':
			case '\n':
			case '\t':
			case '_':
			case '-':
				{
					if(end - 1 == p)
						end = p;
				}
				break;
			default:
				break;
		}
		--p;
	}
	memmove(str,start,end-start);
	*(str + (end - start)) = 0;
}

//去掉字符串内的空白字符
static void remove_str_blank(const char *src, char *dst)
{
	char c;

	while (c = *src++)
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			*dst++ = c;
	*dst = 0;
}

//视频类提取
enum extract_video_type_enum
{
	ENUM_DIRECTOR = 0, //导演
	ENUM_ACTOR = 1, //主演
	ENUM_HOST = 2, //主持人
	ENUM_GUEST = 3, //嘉宾
	ENUM_TYPE = 4, //类型
	ENUM_REGION = 5, //地区
	ENUM_SCORE = 6, //评分
	ENUM_TIME = 7,//更新时间
	ENUM_INTRODUCE = 8,//简介
	ENUM_LAST = 9,
};

const static char *g_seps[] =
{ "：", ":", "]", "∶", 0};//分隔符


//查找文本中的分隔符的个数
static int find_num_seps(char *text)
{
	int ret = 0;
	if(strlen(text) > 2047)
		return 0;
	char content[2048] = {0};
	int len = sprintf(content, "%s", text);
	content[len] = '\0';
	for(int i = 0; g_seps[i]; i++)
	{
		while(content[0] != '\0')
		{
			char *sub_str = strstr(content, g_seps[i]);
			if(sub_str != NULL)
			{
				content[0] = '\0';
				ret++;
				len = sprintf(content, "%s", sub_str + strlen(g_seps[i]));
				content[len] = '\0';
			}
			else
				break;
		}
	}

	return ret;
}



const static char *g_video_director[] =
{ "导演", 0 }; 
const static char *g_video_actor[] =
{ "主演", "演员", "主要演员", "主角", 0 }; 
const static char *g_video_host[] =
{ "主持人", "主持", 0 };
const static char *g_video_guest[] =
{ "嘉宾", 0 };
const static char *g_video_type[] =
{ "类型", "类别", 0 }; 
const static char *g_video_region[] =
{ "地区", "产地", "制片国家/地区", 0 };
const static char *g_video_score[] =
{ "评分", 0 };
const static char *g_video_time[] = {"更新到", "更新至", 0};
//{ "年代", "年份", 0 };
const static char *g_video_introduce[] =
{"剧情介绍", "剧情梗概", "内容简介", "简介", 0 };


struct extract_video_desp_t
{
	extract_video_type_enum type;
	const char** words;
	int flag; //用来去除重复 防止一个关键字出现多次
};

extract_video_desp_t g_video_desps[ENUM_LAST] =
{
	{ ENUM_DIRECTOR, g_video_director, 0 },
	{ ENUM_ACTOR, g_video_actor, 0 },
	{ ENUM_HOST, g_video_host, 0 },
	{ ENUM_GUEST, g_video_guest, 0 },
	{ ENUM_TYPE, g_video_type, 0 },
	{ ENUM_REGION, g_video_region, 0 }, 
	{ ENUM_SCORE, g_video_score, 0 },
	{ ENUM_TIME, g_video_time, 0 }, 
	{ ENUM_INTRODUCE, g_video_introduce, 0 }, 
};

static void remove_space(char *text) //若text中有分隔符，则将分隔符前的空格作为结束符
{
	if(text == NULL || text[0] == '\0')
		return;
	int len = strlen(text);
	char *seps = strstr(text, "：");
	if(seps != NULL)
		len = len - strlen(seps);	
	for(int i = len-1; i >= 0; i--)
	{
		if(i != 0 && (text[i] == ' ' || text[i] == '\t' || text[i] == '\n'))
		{
			text[i] = '\0';
			break;
		}
	}
}


//检查文字中包含关键字以及分割符 均不包含返回－1 包含关键字但不包含分割符-关键字不结尾返回2-关键字结尾返回3   均包含且以分割符结尾 返回 1  不以分割符结尾 返回0
//（特别地 对于简介、介绍可无分割符）(特殊情况—key：后是空格)
static int is_contain(char *text, extract_video_desp_t *desp, char *des, int num)
{
	if (text == NULL || text[0] == '\0')
		return -1;
	for (int i = 0; desp->words[i] && desp->flag == 0; i++)
	{
		char *key = strstr(text, desp->words[i]);
		if(key == NULL || (num <= 1 && desp->type != ENUM_INTRODUCE && strlen(key) < strlen(text)))//分隔符只有一个或者没有时，保证关键字开头
			continue;
		char *seps;
		for (int j = 0; g_seps[j]; j++) //要保证word之后便是分割符 （eg更新版本：的出现-两个关键字）
		{
			seps = strstr(key, g_seps[j]);
			if(seps == NULL || strlen(seps) + strlen(desp->words[i]) != strlen(key))
				continue;
			else if (strncmp((key + strlen(desp->words[i])), g_seps[j], strlen(g_seps[j])) == 0) //比较去除空格后的长度
			{
				if (strlen(seps) == strlen(g_seps[j]))
				{
					return 1;
				}
				else
				{
					char *src = seps + strlen(g_seps[j]);
					if(strlen(src) > 2047)
						return -1;
					int len = sprintf(des, "%s", src);
					des[len] = '\0';
					if(len > 0 && num > 1)
						remove_space(des);
					return 0;
				}
			}
			else
				break;
		}
		if (seps == NULL)//针对 更新至2013-09-15
		{
			char *src = key + strlen(desp->words[i]);
			if(strlen(src) > 2047)
				return -1;
			int len = sprintf(des, "%s", src);
			des[len] = '\0';
			if(len > 5)
				return 2;
			else if(len == 0)
				return 3;
		}
	}
	return -1;
}

//找下一个文本节点 但不能包含分隔符-除非是时间、简介字段 classify表示0-app或者1-music
static char *find_next_text(html_vnode_t *vnode, int i)
{
	static char content[2048] = {0};
	int len = 0;
	char text[2048] = {0};
	int len_text = 0;
	html_vnode_t *v_node;
	if (vnode->nextNode != NULL)
	{
		html_vnode_t *vnode_next = vnode->nextNode;

		while(vnode_next->hpNode->html_tag.tag_type == TAG_BR)
		{
			vnode_next = vnode_next->nextNode;
			if(vnode_next == NULL)
				return NULL;
		}

		if(vnode_next->hpNode->html_tag.tag_type != TAG_A && vnode_next->hpNode->html_tag.tag_type != TAG_PURETEXT)
			v_node = vnode_next->firstChild;
		else
			v_node = vnode_next;
		while(v_node != NULL)
		{
			len = html_node_extract_content(v_node->hpNode, content, 2047);
			content[len] = '\0';
			if(len + len_text > 2047)
				break;
			if(i == ENUM_INTRODUCE || (strcmp(content, "/") != 0 && len_text < 200 && strstr(content, "更多") == NULL))
				len_text += sprintf(text + len_text, "%s ", content);
			v_node = v_node->nextNode;
		}
		clean_string(text);
		return (len_text > 0 ? text : NULL);
	}
	else if (vnode->upperNode != NULL && vnode->upperNode->nextNode != NULL)
	{
		html_vnode_t *vnode_upper = vnode->upperNode->nextNode;
		if (vnode_upper->hpNode->html_tag.tag_type == TAG_PURETEXT)
		{
			char *text = vnode_upper->hpNode->html_tag.text;
			if(i == ENUM_TIME || i == ENUM_INTRODUCE || (strstr(text, "：") == NULL && strstr(text, ":") == NULL))
				return (strlen(text) > 0 ? text : NULL);
		}
		else
		{
			if(vnode_upper->hpNode->html_tag.tag_type != TAG_A)
				v_node = vnode_upper->firstChild;
			else
				v_node = vnode_upper;
			while(v_node != NULL)
			{
				len = html_node_extract_content(v_node->hpNode, content, 2047);
				content[len] = '\0';
				if(len + len_text > 2047)
					break;
				if(strcmp(content, "/") != 0 && len > 1 && len_text < 200 && strstr(content, "更多") == NULL)
					len_text += sprintf(text + len_text, "%s ", content);
				v_node = v_node->nextNode;
			}
			clean_string(text);
			if(i == ENUM_TIME || i == ENUM_INTRODUCE || (strstr(content, "：") == NULL && strstr(content, ":") == NULL))
				return (len_text > 0 ? text : NULL);
		}
	}
	//专门针对http://tv.sohu.com/s2013/aqyyycx/
	else if(i == ENUM_INTRODUCE && vnode->upperNode != NULL && vnode->upperNode->upperNode != NULL && vnode->upperNode->upperNode->nextNode != NULL)
	{
		html_vnode_t *vnode_upper = vnode->upperNode->upperNode->nextNode;
		len = html_node_extract_content(vnode_upper->hpNode, content, 2047);
		content[len] = '\0';
		return content;
	}

	return NULL;
}

static int start_for_key_value(html_vnode_t *vnode, void *data)
{
	StructureData *stdata = (StructureData *)data;
	if (vnode->hpNode->html_tag.tag_type == TAG_TEXTAREA) //针对http://www.iqiyi.com/dianshiju/20100611/n19573.html-类型:
		return VISIT_SKIP_CHILD;
	else if (vnode->hpNode->html_tag.tag_type == TAG_PURETEXT)
	{
		char *text = vnode->hpNode->html_tag.text;
		if(strlen(text) > 2047)
			return VISIT_SKIP_CHILD;
		char text_copy[2048] = {0};
		char text_blank[2048] = {0};
		int len_text = sprintf(text_copy, "%s", text);
		text_copy[len_text] = '\0';

		int num = find_num_seps(text_copy);
		if(num == 1 || num == 0)
		{
			clean_string(text_copy);
			copy_html_text(text_copy, 0, 2048, text_copy);
			remove_str_blank(text_copy, text_blank);
			len_text = strlen(text_blank);
		}
		else
		{
			len_text = sprintf(text_blank, "%s", text);
			text_blank[len_text] = '\0';
		}

		for (int i = ENUM_DIRECTOR; i < ENUM_LAST; i++)
		{
			char des[2048] = {0};
			int ret = is_contain(text_blank, &(g_video_desps[i]), des, num);
			//printf("text_blank=%s, des=%s, num=%d, ret=%d\n", text_blank, des, num, ret);
			if(ret == 0 || (ret == 2 && i == ENUM_TIME))
			{
				char text_end[2048] = {0};
				int len = copy_html_text(text_end, 0, 2048, des);
				if(i == ENUM_INTRODUCE && len < 20)
					 return VISIT_SKIP_CHILD;
				StructureKV temp_kv;
				temp_kv.key = i;
				temp_kv.value = string(text_end, strlen(text_end));
				int k = atoi(text_end);
				if(len > 0 && (i != ENUM_SCORE || k != 0)) //评分是数字
				{
					stdata->all.push_back(temp_kv);
					g_video_desps[i].flag = 1;
				}
			}
			else if(ret == 1 || ret == 3)//http://tv.sohu.com/s2013/aqyyycx/ http://movie.douban.com/subject/6518754/
			{
				int flag_seps = 0;
				if(ret == 3 && i != ENUM_INTRODUCE && vnode->upperNode != NULL && vnode->upperNode->nextNode != NULL)
				{
					char *next = vnode->upperNode->nextNode->hpNode->html_tag.text;
					for(int i = 0; g_seps[i]; i++)
					{
						if(next != NULL && strstr(next, g_seps[i]) != NULL)
						{
							flag_seps = 1;
							vnode = vnode->upperNode->nextNode;
							break;
						}
					}
				}
				if(ret == 1 || i == ENUM_INTRODUCE || flag_seps == 1)
				{
					char *text_next = find_next_text(vnode, i);
					if(text_next == NULL || (text_next != NULL && i == ENUM_INTRODUCE && strlen(text_next) < 20))//针对http://tv.sohu.com/s2013/dzzehdsn/  剧情简介提取为相关
						return VISIT_SKIP_CHILD;
					if (text_next != NULL && strstr(text_next, "精彩看点") == NULL && strstr(text_next, "请选择") == NULL && strstr(text_next, "稍后补充") == NULL) //专门针对http://v.qq.com/cover/t/twt6ahdnsy0uqdd.html的简介  http://tv.sohu.com/s2013/dzzehdsn/的地区： http://v.youku.com/v_show/id_XNjAyMTc3Njc2.html的简介
					{
						if(i >= ENUM_TIME || strstr(text_next, "：") == NULL) //针对http://v.pptv.com/show/vibiaPDXXbS4llwSiaOWg.html地区：为空
						{
							char text_end[2048] = {0};
							int len = copy_html_text(text_end, 0, 2048, text_next);
							StructureKV temp_kv;
							temp_kv.key = i;
							temp_kv.value = string(text_end, len);
							int k = atoi(text_end);
							if(len > 0 && (i != ENUM_SCORE || k != 0))
							{
								stdata->all.push_back(temp_kv);
								g_video_desps[i].flag = 1;
							}
						}
					}
				}
			}
		}
		//对简介字段特殊处理--主要是优酷站点
		if((strcmp(text_blank, "查看全部") == 0 || strcmp(text_blank, "详情") == 0) && vnode->upperNode != NULL && vnode->upperNode->hpNode->html_tag.tag_type == TAG_A && vnode->upperNode->prevNode != NULL && g_video_desps[8].flag == 0)
		{
			//printf("text_blank=%s\n", text_blank);
			html_vnode_t *pre_vnode = vnode->upperNode->prevNode;
			if(pre_vnode->hpNode->html_tag.tag_type == TAG_PURETEXT)
			{
				char *text = pre_vnode->hpNode->html_tag.text;
				if(strlen(text) <= 0)
					return VISIT_SKIP_CHILD;
				StructureKV temp_kv;
				temp_kv.key = 8;
				temp_kv.value = string(text, strlen(text));
				stdata->all.push_back(temp_kv);
				g_video_desps[8].flag = 1;

			}
			else
			{
				char content[2048] = {0};	
				int len = html_node_extract_content(pre_vnode->hpNode, content, 2047);
				content[len] = '\0';
				if(len <= 0 || len > 2047)
					return VISIT_SKIP_CHILD;
				StructureKV temp_kv;
				temp_kv.key = 8;
				temp_kv.value = string(content, len);
				stdata->all.push_back(temp_kv);
				g_video_desps[8].flag = 1;
			}
			return VISIT_SKIP_CHILD;

		}
		return VISIT_SKIP_CHILD;
	}
	return VISIT_NORMAL;
}

static int start_visit_tvmao(html_tag_t* html_tag, void* result, int flag)
{
	extract_img_link *img_link = (extract_img_link *)result;
	//缩略图
	if (html_tag->tag_type == TAG_IMG && img_link->flag_img == 0)
	{
		char *attr = get_attribute_value(html_tag, "itemprop");
		char *attr_src = get_attribute_value(html_tag, ATTR_SRC);
		if(attr_src != NULL && attr != NULL && strcmp(attr, "image") == 0)
		{
			StructureKV temp_kv;
			temp_kv.key = 9;
			temp_kv.value = string(attr_src, strlen(attr_src));
			img_link->stdata->all.push_back(temp_kv);
			img_link->flag_img = 1;
			return VISIT_SKIP_CHILD;
		}
	}	
	//评分
	else if(g_video_desps[6].flag == 0 && html_tag->tag_type == TAG_SPAN)
	{
		char *attr = get_attribute_value(html_tag, "itemprop");
		if(attr != NULL && strcmp(attr, "ratingValue") == 0)
		{
			char content[10] = {0};
			int len = html_node_extract_content(html_tag->html_node, content, 9);
			content[len] = '\0';
			StructureKV temp_kv;
			temp_kv.key = 6;
			temp_kv.value = string(content, len);
			img_link->stdata->all.push_back(temp_kv);
			g_video_desps[6].flag = 1;
			return VISIT_SKIP_CHILD;
		}
	}
	//分集链接
	else if(html_tag->tag_type == TAG_H2 && img_link->flag_link == 0)
	{
		char url_link[2048] = {0};
		int len_link = 0;
		char *attr = get_attribute_value(html_tag, ATTR_CLASS);
		if(attr != NULL && strcmp(attr, "mt10 mb10 clear bg_lblue") == 0 && html_tag->html_node->child != NULL)
		{
			for(html_node_t *node = html_tag->html_node->child; node != NULL; node = node->next)
			{
				if(node->html_tag.tag_type == TAG_A)
				{
					char *attr_itemprop = get_attribute_value(&node->html_tag, "itemprop");
					char *attr_href = get_attribute_value(&node->html_tag, ATTR_HREF);
					if(attr_itemprop != NULL && strcmp(attr_itemprop, "episodes") == 0 && attr_href != NULL)
					{
						if(len_link + strlen(attr_href) + 20  > 2047)
							break;
						len_link += sprintf(url_link+len_link, "http://www.tvmao.com%s ", attr_href);
					}
				}
			}
			if(len_link > 0)
			{
				StructureKV temp_kv;
				temp_kv.key = 10;
				temp_kv.value = string(url_link, len_link);
				img_link->stdata->all.push_back(temp_kv);
				img_link->flag_link = 1;
				return VISIT_SKIP_CHILD;
			}
		}
	}
	//简介
	else if(g_video_desps[8].flag == 0)
	{
		char *str_1 = strstr(img_link->url, "/episode");
		char *str_2 = strstr(img_link->url, "/episode/");
		char *stop;
		
		if(str_2 != NULL && html_tag->tag_type == TAG_P)
		{
			strtol(str_2+9, &stop, 10);
			if(stop[0] == '\0')
			{
				char content[300] = {0};
				int len = html_node_extract_content(html_tag->html_node, content, 299);
				content[len] = '\0';
				StructureKV temp_kv;
				temp_kv.key = 8;
				temp_kv.value = string(content, len);
				img_link->stdata->all.push_back(temp_kv);
				g_video_desps[8].flag = 1;
				return VISIT_SKIP_CHILD;
			}
		}
		else if(str_1 != NULL && strlen(str_1) == 8 && html_tag->tag_type == TAG_P)//专门针对以/episode结尾的网页
		{
			char content[300] = {0};
			int len = html_node_extract_content(html_tag->html_node, content, 299);
			content[len] = '\0';
			StructureKV temp_kv;
			temp_kv.key = 8;
			temp_kv.value = string(content, len);
			img_link->stdata->all.push_back(temp_kv);
			g_video_desps[8].flag = 1;
			return VISIT_SKIP_CHILD;
				

		}
		else if(html_tag->tag_name != NULL && (strcasecmp(html_tag->tag_name, "article") ==0 || html_tag->tag_type == TAG_DIV))
		{
			char *attr = get_attribute_value(html_tag, "itemprop");
			if(attr != NULL && strcmp(attr, "description") == 0)
			{
				char content[300] = {0};
				int len = html_node_extract_content(html_tag->html_node, content, 299);
				content[len] = '\0';
				StructureKV temp_kv;
				temp_kv.key = 8;
				temp_kv.value = string(content, len);
				img_link->stdata->all.push_back(temp_kv);
				g_video_desps[8].flag = 1;
				return VISIT_SKIP_CHILD;

			}
		}
	}
	return VISIT_NORMAL;
}

int is_first_end_link(char *text)
{
	char *end;
	int num = strtol(text, &end, 10);
	//if((strstr(text, "第") != NULL && strstr(text, "集") != NULL) || (num > 0 && num < 300 && end[0] == '\0'))
	if(strstr(text, "第") != NULL)
	{
		if(strstr(text, "集") != NULL)
			return 1;
		else 
			return 0;
	}
	else if(num > 0 && num < 300 && end[0] == '\0')
		return 1;
	else
		return 0;
}


static int start_visit_img(html_tag_t* html_tag, void* result, int flag)
{
	extract_img_link *img_link = (extract_img_link *)result;
	//填充title字段
	if(html_tag->tag_type == TAG_TITLE)
	{
		char content[2048] = {0};
		char content_[2048] = {0};
		int len = html_node_extract_content(html_tag->html_node, content, 2047);
		content[len] = '\0';
		copy_html_text(content_, 0, 2047, content);
		if(strlen(content_) > 2047)
			return VISIT_SKIP_CHILD;
		sprintf(img_link->title, "%s", content_);
		return VISIT_SKIP_CHILD;

	}
	//缩略图
	else if (html_tag->tag_type == TAG_IMG && img_link->flag_img == 0 && (img_link->pagetype == 13 || img_link->pagetype ==1314))
	{
		char *attr = get_attribute_value(html_tag, ATTR_ALT);
		char *attr_src = get_attribute_value(html_tag, ATTR_SRC);
		if(attr_src != NULL && attr != NULL && img_link->title[0] != '\0' && strstr(attr_src, "logo") == NULL && (strstr(img_link->title, attr) != NULL || strncmp(img_link->url, "http://movie.douban.com/subject/", 32) == 0) && strcmp(attr, "腾讯视频") != 0 && strcmp(attr, "搜狐视频") != 0)//针对tv.sohu.com 以及 http://v.qq.com/cover/8/84c8923xlk8hf6p.html
		{
			StructureKV temp_kv;
			temp_kv.key = 9;
			temp_kv.value = string(attr_src, strlen(attr_src));
			img_link->stdata->all.push_back(temp_kv);
			img_link->flag_img = 1;
			return VISIT_SKIP_CHILD;
		}
	}
	//第一集链接 最后一集链接
	else if(html_tag->tag_type == TAG_A && html_tag->html_node->child != NULL && html_tag->html_node->child->html_tag.tag_type == TAG_PURETEXT && img_link->flag == 0 && (img_link->pagetype == 13 || img_link->pagetype ==1314))
	{
		char *attr_href = get_attribute_value(html_tag, ATTR_HREF);
		char *text = html_tag->html_node->child->html_tag.text;
		int i = is_first_end_link(text);
		char href[2048] = {0};
		if(attr_href != NULL && i == 1) //排除年份-全是数字 以及年龄18岁以上
		{
			if (strncmp(attr_href, "http://", 7) == 0)
			{
				if(strlen(attr_href) > 2047)
					return VISIT_SKIP_CHILD;
				sprintf(href, "%s", attr_href);
			}
			else if(strncmp(attr_href, "/", 1) == 0 || strncmp(attr_href, "../", 3) == 0)
			{
				char url_new[2048] = {0};
				if(strlen(img_link->url) > 2047)
					return VISIT_SKIP_CHILD;
				sprintf(url_new, "%s", img_link->url);
				char *ptr = strchr(url_new + 7, '/');
				int pos = ptr - url_new;
				if(strlen(attr_href) + pos > 2047)
					return VISIT_SKIP_CHILD;
				strcpy(url_new + pos, strstr(attr_href, "/"));
				if(strlen(url_new) > 2047)
					return VISIT_SKIP_CHILD;
				sprintf(href, "%s ", url_new);
			}
			else
			{
				char url_new[2048] = {0};
				if(strlen(img_link->url) > 2047)
					return VISIT_SKIP_CHILD;
				sprintf(url_new, "%s", img_link->url);
				char *ptr = strrchr(url_new + 7, '/');
				int pos = ptr - url_new;
				if(strlen(attr_href) + pos + 1 > 2047)
					return VISIT_SKIP_CHILD;
				strcpy(url_new + pos + 1, href);
				if(strlen(url_new) > 2047)
					return VISIT_SKIP_CHILD;
				sprintf(href, "%s ", url_new);
			}
			if(strlen(href) > 2047)
				return VISIT_SKIP_CHILD;
			if(img_link->first[0] == '\0')
			{
				sprintf(img_link->first, "%s", href);
				img_link->len_first = strlen(href);
			}
			else if(strlen(href) == img_link->len_first && strcmp(img_link->first, href) != 0)
			{
				if(img_link->end[0] == '\0' || strcmp(img_link->end, href) != 0)
					sprintf(img_link->end, "%s", href);
				else
					img_link->flag = 1;
			}
			else
				img_link->flag = 1;
			return VISIT_SKIP_CHILD;
		}
	}
	//主要针对pptv的评分
	if(strncmp(img_link->url, "http://v.pptv.com/show/", 23) == 0 && html_tag->tag_type == TAG_SPAN && g_video_desps[6].flag == 0)
	{
		char *attr = get_attribute_value(html_tag, ATTR_CLASS);
		if(attr != NULL && strcmp(attr, "num") == 0)
		{
			char content[10] = {0};
			int len = html_node_extract_content(html_tag->html_node, content, 9);
			content[len] = '\0';
			float k = atof(content);
			if(k <= 0 || k >= 100 || len <= 0 || len > 5)
				return VISIT_SKIP_CHILD;
			char score[5];
			len = sprintf(score, "%.1f", k);
			StructureKV temp_kv;
			temp_kv.key = 6;
			temp_kv.value = string(score, len);
			img_link->stdata->all.push_back(temp_kv);
			g_video_desps[6].flag = 1;
			return VISIT_SKIP_CHILD;
		}
		else if(attr != NULL && strcmp(attr, "star") == 0 && html_tag->html_node->next != NULL && html_tag->html_node->next->html_tag.tag_type == TAG_EM)
		{
			char content[10] = {0};
			int len = html_node_extract_content(html_tag->html_node->next, content, 9);
			content[len] = '\0';
			if(len <= 0 || len > 5)
				return VISIT_SKIP_CHILD;
			StructureKV temp_kv;
			temp_kv.key = 6;
			temp_kv.value = string(content, len);
			img_link->stdata->all.push_back(temp_kv);
			g_video_desps[6].flag = 1;
			return VISIT_SKIP_CHILD;
		}
	}
	return VISIT_NORMAL;
}

//视频类
int html_tree_extract_video_stdata(const char *url, int url_len, unsigned int pagetype, html_tree_t *html_tree, html_vtree_t *html_vtree, area_tree_t *atree, StructureData *stdata)
{
	stdata->type = 0;
	stdata->version = 0;
	stdata->all.clear();

	stdata->version = 1;
	stdata->type = pagetype;

	//调用前进行初始化
	for(int i = ENUM_DIRECTOR; i < ENUM_LAST; i++)
		g_video_desps[i].flag = 0;

	//key_value结构提取
	html_vtree_visit(html_vtree, start_for_key_value, NULL, stdata);

	char title[2048] = {0};
	char first[2048] = {0};
	char end[2048] = {0};
	extract_img_link img_link;
	memset(&img_link, 0, sizeof(img_link));
	img_link.stdata = stdata;
	img_link.flag_img = 0;
	img_link.flag_link = 0;
	img_link.url = url;
	img_link.title = title;
	img_link.first = first;
	img_link.len_first = 0;
	img_link.end = end;
	img_link.flag = 0;
	img_link.pagetype = pagetype;

	//tvmao-配模板 缩略图 分集链接 介绍
	if(strncmp(url, "http://www.tvmao.com/", 21) == 0)
	{
		html_tree_visit(html_tree, start_visit_tvmao, NULL, &img_link, 0);
	}
	else
	{
		html_tree_visit(html_tree, start_visit_img, NULL, &img_link, 0);
	}
	
	if(img_link.first[0] != '\0' && img_link.end[0] != '\0')
	{
		StructureKV temp_kv;
		temp_kv.key = 11;
		temp_kv.value = string(img_link.first, strlen(img_link.first));
		img_link.stdata->all.push_back(temp_kv);
		StructureKV temp_kv1;
		temp_kv1.key = 12;
		temp_kv1.value = string(img_link.end, strlen(img_link.end));
		img_link.stdata->all.push_back(temp_kv1);
	}

	return 0;
}

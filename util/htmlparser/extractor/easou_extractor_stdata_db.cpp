/*
 * easou_extractor_stdata_db.cpp
 *
 *  Created on: 2013-10-08
 *
 *  Author: round
 */

#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include "easou_extractor_stdata_db.h"

#include "easou_html_tree.h"
#include "easou_vhtml_tree.h"
#include "easou_vhtml_basic.h"
#include "easou_html_extractor.h"
#include "easou_html_attr.h"
#include "html_text_utils.h"
#include "./thrift/StructData_types.h"

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>

using namespace std;

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using boost::shared_ptr;

#define MAX_NUM 300

namespace {
std::map<std::string, int> db_local_map_ks;
const char * db_local_str=" 1 导演 2 主演 3 编剧 4 类型 5 制片国家/地区 6 片长 7 集数 8 单集片长 9 作者 10 译者 11 出版社 12 出版日期 12 出版年 13 又名 14 语言 15 上映日期 16 页数 ";
}
const static char *g_seps[] ={ "：", ":", 0};//分隔符:

struct STDATA_KS
{
	map<string, int> map_ks;
	StructureData *stdata;
	int flag[MAX_NUM];
};


//去掉首尾空白字符
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


static char *find_next_text(html_vnode_t *vnode)
{
	static char content[4096] = {0};
	int len = 0;
	html_vnode_t *vnode_upper = vnode->upperNode;
	html_vnode_t *vnode_next;
	if(vnode_upper->nextNode == NULL)
		return NULL;
	vnode_next = vnode_upper->nextNode;
	if(vnode_next->hpNode->html_tag.tag_type == TAG_PURETEXT)
	{
		if(strncmp(vnode_next->hpNode->html_tag.text, g_seps[1], strlen(g_seps[1])) == 0)
			vnode_next = vnode_next->nextNode;
		else
			return vnode_next->hpNode->html_tag.text;
	}
	while(vnode_next != NULL)
	{
		char text[4096] = {0};
		int len_text = 0;
		len_text = html_node_extract_content(vnode_next->hpNode, text, 4095);
		text[len_text] = '\0';
		if(len + len_text > 4095 || vnode_next->hpNode->html_tag.tag_type == TAG_BR)
			break;
		vnode_next = vnode_next->nextNode;
		len += sprintf(content+len, "%s", text);
	}
	return content;
}

#include <stack>
static char *get_sub_img(html_vnode_t *vnode)
{
	static char content[8192] = {0};
	int len = 0;
	std::stack<html_vnode_t *> stack_node; //定义栈

	if (vnode == NULL || vnode->firstChild == NULL)
		return NULL;

	html_vnode_t *child = vnode->firstChild;

	while (child != NULL || !stack_node.empty())
	{
		while (child != NULL)
		{
			char *src = get_attribute_value(&child->hpNode->html_tag, ATTR_SRC);
			if (child->hpNode->html_tag.tag_type == TAG_IMG && src != NULL && len + strlen(src) < 8192)
			{
				len += sprintf(content+len, "%s ", src);	
				break;
			}
			else
			{
				stack_node.push(child);
				child = child->firstChild;
			}
		}
		if (!stack_node.empty())
		{
			child = stack_node.top();
			stack_node.pop();
			child = child->nextNode;
		}
	}
	return content;
}

static int start_for_ks_value(html_vnode_t *vnode, void *data)
{
	STDATA_KS *std_map = (STDATA_KS *)data;
	//针对配置文件中的key_value格式
	if (vnode->hpNode->html_tag.tag_type == TAG_PURETEXT)
	{
		char *text = vnode->hpNode->html_tag.text;
		if(text == NULL || (text != NULL && strlen(text) > 99))
			return VISIT_SKIP_CHILD;
		int len_text = strlen(text);
		char text_name[100] = {0};
		sprintf(text_name, "%s", text);

		//目前仅考虑一种分隔符或者没有分隔符（豆瓣电影、豆瓣读书）key与value是分开的情况
		char *str = strstr(text_name, g_seps[1]);
		if(str == NULL || (str != NULL && strlen(str) == strlen(g_seps[1])))
		{
			int len_str = 0;
			if(str != NULL)
				len_str = strlen(str);
			text_name[len_text - len_str] = '\0';

			string key = string(text_name);
			map<string, int>::iterator it = std_map->map_ks.find(key);
			if(it == std_map->map_ks.end())
				return VISIT_SKIP_CHILD;
			//printf("key=%s ", key.c_str());
			//printf("stdata_key=%d\n", std_map->map_ks[key]);

			char *value = find_next_text(vnode);
			//printf("value=%s\n", value);

			if(value != NULL && strlen(value) > 0)
			{
				clean_string(value);
				StructureKV temp_kv;
				temp_kv.key = std_map->map_ks[key];
				temp_kv.value = string(value, strlen(value));
				if(std_map->flag[temp_kv.key] == 1)
					return VISIT_SKIP_CHILD;
				std_map->stdata->all.push_back(temp_kv);
				std_map->flag[temp_kv.key] = 1;
			}

			return VISIT_SKIP_CHILD;
		}
		return VISIT_SKIP_CHILD;
	}
	//采用模板提取标题、图片url等字段信息
	else if(vnode->hpNode->html_tag.tag_type == TAG_SPAN)
	{
		//标题
		char *attr = get_attribute_value(&vnode->hpNode->html_tag, "property");
		if(attr != NULL && strcmp(attr, "v:itemreviewed") == 0)
		{
			char content[1024] = {0};
			int len = html_node_extract_content(vnode->hpNode, content, 1023);
			content[len] = '\0';
			clean_string(content);
			StructureKV temp_kv;
			temp_kv.key = 17;
			temp_kv.value = string(content, len);
			if(std_map->flag[17] == 1)
				return VISIT_SKIP_CHILD;
			std_map->stdata->all.push_back(temp_kv);
			std_map->flag[temp_kv.key] = 1;
			return VISIT_SKIP_CHILD;
		}
	}
	else if(vnode->hpNode->html_tag.tag_type == TAG_IMG)
	{
		//封面图片url
		char *attr = get_attribute_value(&vnode->hpNode->html_tag, "rel");
		char *attr_src = get_attribute_value(&vnode->hpNode->html_tag, ATTR_SRC);
		if(attr != NULL && attr_src != NULL && (strcmp(attr, "v:image") == 0 || strcmp(attr, "v:photo") == 0))
		{
			clean_string(attr_src);
			StructureKV temp_kv;
			temp_kv.key = 18;
			temp_kv.value = string(attr_src, strlen(attr_src));
			if(std_map->flag[18] == 1)
				return VISIT_SKIP_CHILD;
			std_map->stdata->all.push_back(temp_kv);
			std_map->flag[temp_kv.key] = 1;
			return VISIT_SKIP_CHILD;
		}
	}
	else if(vnode->hpNode->html_tag.tag_type == TAG_STRONG)
	{
		//评分
		char *attr = get_attribute_value(&vnode->hpNode->html_tag, "property");
		if(attr != NULL && strcmp(attr, "v:average") == 0)
		{
			char content[10] = {0};
			int len = html_node_extract_content(vnode->hpNode, content, 9);
			content[len] = '\0';
			clean_string(content);
			StructureKV temp_kv;
			temp_kv.key = 22;
			temp_kv.value = string(content, len);
			if(std_map->flag[22] == 1)
				return VISIT_SKIP_CHILD;
			std_map->stdata->all.push_back(temp_kv);
			std_map->flag[temp_kv.key] = 1;
			return VISIT_SKIP_CHILD;
		}
	}
	else if(vnode->hpNode->html_tag.tag_type == TAG_DIV)
	{
		char *attr = get_attribute_value(&vnode->hpNode->html_tag, ATTR_ID);
		char content[300] = {0};
		int len = 0;
		//剧情简介||内容简介
		if(attr != NULL && strcmp(attr, "link-report") == 0)
		{
			len = html_node_extract_content(vnode->hpNode, content, 299);
			content[len] = '\0';
			clean_string(content);
			StructureKV temp_kv;
			temp_kv.key = 20;
			temp_kv.value = string(content, len);
			if(std_map->flag[20] == 1)
				return VISIT_SKIP_CHILD;
			std_map->stdata->all.push_back(temp_kv);
			std_map->flag[temp_kv.key] = 1;
			return VISIT_SKIP_CHILD;
		}
		//目录
		else if(attr != NULL && strstr(attr, "dir_") != NULL && strstr(attr, "_short") != NULL)
		{
			len = html_node_extract_content(vnode->hpNode, content, 299);
			content[len] = '\0';
			clean_string(content);
			StructureKV temp_kv;
			temp_kv.key = 21;
			temp_kv.value = string(content, len);
			if(std_map->flag[21] == 1)
				return VISIT_SKIP_CHILD;
			std_map->stdata->all.push_back(temp_kv);
			std_map->flag[temp_kv.key] = 1;
			return VISIT_SKIP_CHILD;

		}
	}
	else if(vnode->hpNode->html_tag.tag_type == TAG_UL)
	{
		//剧照url
		char *attr = get_attribute_value(&vnode->hpNode->html_tag, ATTR_CLASS);
		if(attr != NULL && strstr(attr, "related-pic-bd") != NULL)
		{
			char *img_url = get_sub_img(vnode);
			if(img_url == NULL || (img_url != NULL && strlen(img_url) <= 0))
				return VISIT_SKIP_CHILD;
			clean_string(img_url);
			StructureKV temp_kv;
			temp_kv.key = 19;
			temp_kv.value = string(img_url, strlen(img_url));
			if(std_map->flag[19] == 1)
				return VISIT_SKIP_CHILD;
			std_map->stdata->all.push_back(temp_kv);
			std_map->flag[temp_kv.key] = 1;
			return VISIT_SKIP_CHILD;
		}
	}
	
	return VISIT_NORMAL;
}

//豆瓣KS类
int html_tree_extract_db_stdata(const char *url, int url_len, unsigned int pagetype, html_tree_t *html_tree, html_vtree_t *html_vtree, area_tree_t *atree, StructureData *stdata)
{
	stdata->type = 0;
	stdata->version = 0;
	stdata->all.clear();

	stdata->version = 1;
	//stdata->type = pagetype;
	stdata->type = 23;

	//hash_map<char *,int> map_ks;
        if ( db_local_map_ks.empty() ){
                // load
                std::stringstream tmp;
                int value;
                std::string key;
                tmp << db_local_str;
                while (tmp.good()){
                        tmp >> value >> key;
                        db_local_map_ks[key] = value;
                }
//              for(map<string, int>::iterator it = local_map_ks.begin(); it != local_map_ks.end() ; ++it) {
//                      cout << it->first << "--> " << it->second << endl;
//              }
        }
	
	//读取配置文件
	//FILE *fp = fopen("./Key_douban.conf", "r");
	//assert(fp);

	//while(2 == fscanf(fp,"%d,%s\n", &key, name))
	//{
	//	string value = string(name);
	//	map_ks[value] = key;
	//}


	STDATA_KS std_map;
	memset(&std_map, 0, sizeof(std_map));
	std_map.stdata = stdata;
	std_map.map_ks = db_local_map_ks;

	//for(map<string, int>::iterator itb = std_map.map_ks.begin(); itb != std_map.map_ks.end(); itb++)
	//	printf("name:%s key:%d\n", itb->first.c_str(), itb->second);

	//key_value结构提取
	html_vtree_visit(html_vtree, start_for_ks_value, NULL, &std_map);

//	fclose(fp);
	return 0;
}

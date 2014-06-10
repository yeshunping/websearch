/*
 * easou_extractor_stdata_bk.cpp
 *
 *  Created on: 2013-09-27
 *
 *  Author: round
 */

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include "easou_extractor_stdata_bk.h"
#include "easou_extractor_stdata.h"

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
std::map<std::string, int> local_map_ks;
const char * local_str="12 中文名 12 中文名称 13 外文名 13 外文名称 14 别名 15 国籍 16 民族 17 出生地 18 出生日期 19 星座 20 职业 21 身高 22 体重 23 生肖 24 血型 25 毕业院校 26 主要成就 27 代表作品 28 经纪公司 29 校训 30 创办时间 31 学校类型 32 主管部门 33 现任校长 34 主要院系 35 知名校友 36 硕士点 37 博士点 38 简称 39 国家重点学科 40 主要奖项 40 所获荣誉 41 学校地址 42 占地面积 43 发展目标 44 类别 45 世界排名 46 学生 47 教师 48 教授 49 院长 50 学校属性 51 类型 52 出品时间 53 出品公司 54 制片地区 54 地区 55 片长 55 动画片时长 56 上映时间 57 集数 58 导演 59 主演 60 编剧 61 制片人 62 其他译名 63 书名 64 又名 65 作者 65 原作者 66 出版时间 67 章回 68 电话区号 69 所属地区 70 面积 71 人口 72 邮政区码 73 行政区类别 74 下辖地区 75 地理位置 76 著名景点 77 气候条件 78 车牌代码 79 方言 80 火车站 81 机场 82 市花 83 市树 83 市树、市花 84 GDP 85 市长 86 市委书记 87 著名高校 87 主要高校 87 高等学府 88 建立时间 89 特产 90 现任书记 91 代码 92 市政区划代码 93 章数 94 页数 95 出版社 96 主要配音 97 发行商 98 原著 99 语言";
}
const static char *g_seps[] ={ "：", ":", 0};//分隔符

struct STDATA_KS
{
	map<string, int> map_ks;
	StructureData *stdata;
	int flag[MAX_NUM];
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
	--p;
	++start;
	if(*start == 0)
	{
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



char *find_next_text(html_vnode_t *vnode)
{
	static char content[4096] = {0};
	int len = 0;
	//跟兄弟结点为不为空 判断
	if(vnode->nextNode != NULL)
	{
		html_vnode_t *next = vnode->nextNode;
		while(next != NULL)
		{
			char text[1024] = {0};
			int len_text = html_node_extract_content(next->hpNode, text, 1023);
			if(len + len_text > 4095)
				return content;
			len += sprintf(content+len, "%s", text);
			next = next->nextNode;
		}
		return content;
	}
	else if(vnode->upperNode != NULL && vnode->upperNode->nextNode != NULL)
	{
		html_vnode_t *vnode_next = vnode->upperNode->nextNode;
		len = html_node_extract_content(vnode_next->hpNode, content, 4095);
		content[len] = '\0';
		return content;
	}
	return NULL;
}

static int start_for_ks_value(html_vnode_t *vnode, void *data)
{
	STDATA_KS *std_map = (STDATA_KS *)data;
	if (vnode->hpNode->html_tag.tag_type == TAG_PURETEXT)
	{
		char *text = vnode->hpNode->html_tag.text;
		if(text == NULL || (text != NULL && strlen(text) > 99))
			return VISIT_NORMAL;
		int len_text = strlen(text);
		char text_name[100] = {0};
		sprintf(text_name, "%s", text);

		//目前仅考虑一种分隔符,且只考虑以分隔符结尾的情况
		char *str = strstr(text_name, g_seps[0]);
		if(str == NULL || (str != NULL && strlen(str) != strlen(g_seps[0])))
			return VISIT_SKIP_CHILD;

		int len_str = strlen(str);
		text_name[len_text - len_str] = '\0';

		string key = string(text_name);
		map<string, int>::iterator it = std_map->map_ks.find(key);
		if(it == std_map->map_ks.end())
			return VISIT_SKIP_CHILD;
		//printf("key=%s ", key.c_str());
		//printf("stdata_key=%d\n", std_map->map_ks[key]);

		char *value = find_next_text(vnode);

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
	return VISIT_NORMAL;
}

//百科KS类
int html_tree_extract_bk_stdata(const char *url, int url_len, unsigned int pagetype, html_tree_t *html_tree, html_vtree_t *html_vtree, area_tree_t *atree, StructureData *stdata)
{
	stdata->type = 0;
	stdata->version = 0;
	stdata->all.clear();

	stdata->type = pagetype;

	int ret = html_tree_extract_stdata(url, url_len, pagetype, html_tree, stdata);
	if(ret != 0)
		return -1;

	if(strncmp(url, "http://baike.baidu.com/view/", 28) != 0 && strncmp(url, "http://baike.baidu.com/subview/", 31) != 0)
		return 0;
	
	//hash_map<char *,int> map_ks;
	//map<string, int> local_map_ks;
	if ( local_map_ks.empty() ){
		// load
		std::stringstream tmp;
		int value;
		std::string key;
		tmp << local_str;	
		while (tmp.good()){
			tmp >> value >> key;
			local_map_ks[key] = value;
		}  
//		for(map<string, int>::iterator it = local_map_ks.begin(); it != local_map_ks.end() ; ++it) {
//			cout << it->first << "--> " << it->second << endl;
//		}
        }

	
	//读取配置文件
//	FILE *fp = fopen("./Key_baike.conf", "r");
//	assert(fp);
//
//	while(2 == fscanf(fp,"%d,%s\n", &key, name))
//	{
//		string value = string(name);
//		map_ks[value] = key;
//	}
	

	STDATA_KS std_map;
	memset(&std_map, 0, sizeof(std_map));
	std_map.stdata = stdata;
	std_map.map_ks = local_map_ks;

	//for(map<string, int>::iterator itb = std_map.map_ks.begin(); itb != std_map.map_ks.end(); itb++)
	//	printf("name:%s key:%d\n", itb->first.c_str(), itb->second);

	//key_value结构提取
	html_vtree_visit(html_vtree, start_for_ks_value, NULL, &std_map);

	//fclose(fp);
	return 0;
}



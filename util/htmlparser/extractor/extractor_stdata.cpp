/*
 * easou_extractor_stdata.cpp
 *
 *  Created on: 2013-07-19
 *
 *  modify : 2013-08-15
 *  
 *  Author: round
 */

#include <string>
#include "easou_extractor_stdata.h"

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


struct visit_for_bk_t
{
	const char* url;
	int url_len;
	StructureData* stdata;
	int dir_num;

	char* buf;
	int avail;
	int size;
	int count;
	int count1;//限制图片链接数
	
	//疾病类
	char *title[10];
	char *desp[10];
	int number_title;
	int number_desp;
};

struct visit_for_jy_t
{
	StructureData *stdata;

	const char *url;
	int url_len;

	char *buf_img;
	char *buf_jy;
	char *buf_jq;
	int avail_img;
	int avail_jy;
	int avail_jq;
	int size;
	int count;
};

struct visit_for_dp_t
{
	StructureData *stdata;
	char *buf;
	int avail;
	int size;
};

//去掉字符串首尾的空格
void clean_string(char *str)
{
	if(str == NULL)
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
void remove_str_blank(const char *src, char *dst)
{
	char c;

	while (c = *src++)
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			*dst++ = c;
	*dst = 0;
}

static html_node_t *get_sub_a_node(html_node_t *node_t)
{
	std::stack<html_node_t *> stack_node; //定义栈

	if (node_t == NULL || node_t->child == NULL)
		return NULL;
	if (node_t->html_tag.tag_type == TAG_A)
		return node_t;

	html_node_t *node = node_t->child;

	while (node != NULL || !stack_node.empty())
	{
		while (node != NULL)
		{
			if (node->html_tag.tag_type == TAG_A)
			{
				return node;
			}
			else
			{
				stack_node.push(node);
				node = node->child;
			}
		}
		if (!stack_node.empty())
		{
			node = stack_node.top();
			stack_node.pop();
			node = node->next;
		}
	}
	return NULL;
}


int start_visit_bk(html_tag_t* html_tag, void* result, int flag)
{
	visit_for_bk_t* data = (visit_for_bk_t*) result;

	//标题
	if (html_tag->tag_type == TAG_H1)
	{
		char *attr = get_attribute_value(html_tag, ATTR_CLASS);
		if(attr != NULL && strcmp(attr, "title") == 0)
		{

			char content[1024] = {0};
			int len = html_node_extract_content(html_tag->html_node, content, 1023);
			StructureKV temp_kv;
			temp_kv.key = 0;
			temp_kv.value = string(content, len);
			data->stdata->all.push_back(temp_kv);

		}
	} 
	else if (html_tag->tag_type == TAG_DIV)
	{
		//介绍
		char *attr = get_attribute_value(html_tag, ATTR_CLASS);
		if(attr != NULL && strcmp(attr, "card-summary-content") == 0 && data->count == 1)
		{

			char content[10240] = {0};
			int len = html_node_extract_content(html_tag->html_node, content, 10239);
			if(len > 0)
			{
				StructureKV temp_kv;
				temp_kv.key = 2;
				temp_kv.value = string(content, len);
				data->stdata->all.push_back(temp_kv);
				data->count = 0;
			}
		}
		char *attr_id = get_attribute_value(html_tag, ATTR_ID);
		if(attr_id != NULL && strstr(attr_id, "card-show-tab") != NULL)
		{
			char content[10240] = {0};
			int len = html_node_extract_content(html_tag->html_node->child, content, 10239);
			content[len] ='\0';

			char *attr_a = get_attribute_value(&html_tag->html_node->child->next->html_tag, ATTR_HREF);

			if(len + data->url_len + strlen(attr_a) > 40959)
				return VISIT_ERROR;
			sprintf(data->desp[data->number_desp], "%s %s%s", content, data->url, attr_a);

			data->number_desp++;
		}
	}
	else if (html_tag->tag_type == TAG_DD)
	{
		//前5个目录及链接
		char *attr = get_attribute_value(html_tag, ATTR_CLASS);
		//baike改版 链接http://baike.baidu.com/view/ http://baike.baidu.com/link?url=
		//modify 提取目录链接 2013-09-27
		if(data->dir_num < 6 && attr != NULL && strcmp(attr, "catalog-item") == 0 && html_tag->html_node->child != NULL)
		{
			//找DD节点下的A标签
			html_node_t *node = get_sub_a_node(html_tag->html_node);
			if(node == NULL)
				return VISIT_SKIP_CHILD;
			char *attr_a = get_attribute_value(&node->html_tag, ATTR_HREF);
			if (attr_a)
			{
				int attr_len = strlen(attr_a);

				char content[1024] = {0};
				int len = html_node_extract_content(node, content, 1023);
				content[len] = '\0';

				if (data->avail + len + data->url_len + attr_len + 1 >= data->size || attr_len > 2)
					return VISIT_ERROR;

				char content_black[1024] = {0};
				remove_str_blank(content, content_black); ////拼接字符串时 应该去掉空格
				data->avail += sprintf(data->buf + data->avail, "%s %s%s ", content_black, data->url, attr_a);
				data->dir_num++;
			}
		}
		char *attr_tag = get_attribute_value(html_tag, "classtag");
		if(attr_tag != NULL && strstr(attr_tag, "card-show-tab") != NULL)
		{
			char content[20] = {0};
			int len = html_node_extract_content(html_tag->html_node, content, 19);
			content[len]='\0';

			strncpy(data->title[data->number_title], content, len);
			//printf("data->title=%s\n", content);	
			data->number_title++;
		}

	}
	if(html_tag->tag_type == TAG_IMG)
	{
		char *attr = get_attribute_value(html_tag, ATTR_CLASS);
		char *attr_src = get_attribute_value(html_tag, ATTR_SRC);
		if(attr != NULL && strstr(attr, "card-image") != NULL && attr_src != NULL && data->count1 == 1)
		{
			int len = strlen(attr_src);
			if(len > 0)
			{
				StructureKV temp_kv;
				temp_kv.key = 11;
				temp_kv.value = string(attr_src, len);
				data->stdata->all.push_back(temp_kv);		
				data->count1 = 0;
			}
		}
	}

	return 0;
}

int start_visit_jy(html_tag_t* html_tag, void* result, int flag)
{
	visit_for_jy_t *data = (visit_for_jy_t *)result;

	//标题
	if (html_tag->tag_type == TAG_SPAN)
	{
		char *attr = get_attribute_value(html_tag, ATTR_CLASS);
		if(attr != NULL && strcmp(attr, "exp-title") == 0)
		{

			char content[1024] = {0};
			int len = html_node_extract_content(html_tag->html_node, content, 1024);

			StructureKV temp_kv;
			temp_kv.key = 0;
			temp_kv.value = string(content, len);
			data->stdata->all.push_back(temp_kv);

		}
	}

	//首段文字
	else if (html_tag->tag_type == TAG_DIV)
	{
		char *attr = get_attribute_value(html_tag, ATTR_CLASS);
		if(attr != NULL && strcmp(attr, "content-listblock-text") == 0 && data->count == 1)
		{

			char content[40960] = {0};
			int len = html_node_extract_content(html_tag->html_node, content, 40960);
			if(len > 0)
			{
				StructureKV temp_kv;
				temp_kv.key = 2;
				temp_kv.value = string(content, len);
				data->stdata->all.push_back(temp_kv);
				data->count = 0;
			}

		}
	}

	//图片页面链接以及图片本身链接
	else if (html_tag->tag_type == TAG_LI)
	{
		char *attr = get_attribute_value(html_tag, ATTR_CLASS);
		if(attr != NULL && strcmp(attr, "thumb-item") == 0 && html_tag->html_node->child != NULL && html_tag->html_node->child->html_tag.tag_type == TAG_A)
		{
			//html_node_t *child = html_tag->html_node->child;
			char *attr_a = get_attribute_value(&html_tag->html_node->child->html_tag, ATTR_HREF);
			char *attr_img = get_attribute_value(&html_tag->html_node->child->child->html_tag, ATTR_SRC);
			int len1 = strlen(attr_a);
			int len2 = strlen(attr_img);

			if (data->avail_img + data->url_len + len1 + len2 + 1 >= data->size)
				return VISIT_ERROR;
			data->avail_img += sprintf(data->buf_img+data->avail_img, "%s%s ", data->url, attr_a);
			data->avail_img += sprintf(data->buf_img+data->avail_img, "%s ", attr_img);


		}

	}

	//相关经验文字及链接pos:relate_title
	else if (html_tag->tag_type == TAG_A)
	{
		char *attr_log = get_attribute_value(html_tag, "log");
		if(attr_log != NULL && strstr(attr_log, "pos:relate_title") != NULL)
		{
			char content[1024] = {0};
			int len = html_node_extract_content(html_tag->html_node, content, 1024);
			content[len] = '\0';

			char content_black[1024] = {0};
			remove_str_blank(content, content_black);
			len = strlen(content_black);
			char *attr_a = get_attribute_value(html_tag, ATTR_HREF);
			int len1 = strlen(attr_a);

			if (data->avail_jy + data->url_len + len + len1 + 1 >= data->size)
				return VISIT_ERROR;

			data->avail_jy += sprintf(data->buf_jy+data->avail_jy, "%s %s%s ", content_black, data->url, attr_a);

		}
		//相关方法/技巧大全文字及链接article-thumbnail-img
		char *attr = get_attribute_value(html_tag, ATTR_CLASS);
		if(attr != NULL && strcmp(attr, "article-thumbnail-img") == 0)
		{
			char *attr_tit = get_attribute_value(html_tag, ATTR_TITLE);
			char attr_tit_black[1024] = {0};
			remove_str_blank(attr_tit, attr_tit_black);
			char *attr_href = get_attribute_value(html_tag, ATTR_HREF);
			int len1 = strlen(attr_tit_black);
			int len2 = strlen(attr_href);
			if (data->avail_jq + data->url_len + len1 + len2 + 1 >= data->size)
				return VISIT_ERROR;

			data->avail_jq += sprintf(data->buf_jq+data->avail_jq, "%s %s%s ", attr_tit_black, data->url, attr_href);
		}



	}

	return 0;
}

int start_visit_dp(html_tag_t* html_tag, void* result, int flag)
{
	visit_for_dp_t *data = (visit_for_dp_t *)result;

	//名字
	if (html_tag->tag_type == TAG_H1)
	{
		char *attr = get_attribute_value(html_tag, ATTR_CLASS);
		if(attr != NULL && strcmp(attr, "shop-title") == 0)
		{

			char content[1024] = {0};
			int len = html_node_extract_content(html_tag->html_node, content, 1024);

			StructureKV temp_kv;
			temp_kv.key = 0;
			temp_kv.value = string(content, len);
			data->stdata->all.push_back(temp_kv);
			
		}
	}

	//地址
	else if (html_tag->tag_type == TAG_A)
	{
		char *attr = get_attribute_value(html_tag, ATTR_CLASS);
		if(attr != NULL && strcmp(attr, "link-dk") == 0 && html_tag->html_node->next != NULL)
		{

			char content1[2048] = {0};
			char content2[1024] = {0};
			int len1 = html_node_extract_content(html_tag->html_node, content1, 1024);
			int len2 = html_node_extract_content(html_tag->html_node->next, content2, 1024);
			content1[len1] = '\0';
			content2[len2] = '\0';

			memcpy(content1+len1, content2, len2);
			StructureKV temp_kv;
			temp_kv.key = 1;
			temp_kv.value = string(content1, len1+len2);
			data->stdata->all.push_back(temp_kv);


		}
	}

	//评价数 or 星标
	else if (html_tag->tag_type == TAG_EM || html_tag->tag_type == TAG_SPAN)
	{
		char *attr = get_attribute_value(html_tag, "itemprop");
		if(attr != NULL && strcmp(attr, "count") == 0)
		{

			char content[100] = {0};
			int len = html_node_extract_content(html_tag->html_node, content, 100);

			StructureKV temp_kv;
			temp_kv.key = 2;
			temp_kv.value = string(content, len);
			
			data->stdata->all.push_back(temp_kv);

		}
		//星标-几星级
		if(html_tag->tag_type == TAG_SPAN && html_tag->html_node->parent != NULL && html_tag->html_node->parent->html_tag.tag_type == TAG_DIV)
		{

			char *attr_div = get_attribute_value(&html_tag->html_node->parent->html_tag, ATTR_CLASS);
			char *attr_class = get_attribute_value(html_tag, ATTR_CLASS);
			char *attr_title = get_attribute_value(html_tag, ATTR_TITLE);
			if(attr_class != NULL && attr_title != NULL && attr_div != NULL && strcmp(attr_div, "comment-rst") == 0 && strstr(attr_class, "item-rank-rst irr-star") != NULL && (strstr(attr_title, "星商户") != NULL || strstr(attr_title, "暂无星级")))
			{
				int star_i = atoi(attr_class + 22);//22为item-rank-rst等的串长
				float star_f = 0.1 * star_i;
				char star_str[3] = {0};
				int len = sprintf(star_str, "%.1f", star_f);

				StructureKV temp_kv;
				temp_kv.key = 6;
				temp_kv.value = string(star_str, len);

				data->stdata->all.push_back(temp_kv);
			}
		}
	}
	//人均价格
	else if (html_tag->tag_type == TAG_STRONG)
	{
		char *attr = get_attribute_value(html_tag, ATTR_CLASS);
		//printf("attr=%s\n\n\n", attr);
		if(attr != NULL && strcmp(attr, "stress") == 0)
		{

			char content[100] = {0};
			int len = html_node_extract_content(html_tag->html_node, content, 100);

			char con[20] = {0};
			int len_con = sprintf(con, "%s", "&yen;189");
			len_con = copy_html_text(con, 0, 20, con);
			printf("con=%s,len_con=%d\n", con, len_con);

			StructureKV temp_kv;
			temp_kv.key = 3;
			temp_kv.value = string(content, len);
			data->stdata->all.push_back(temp_kv);

		}
	}

	//meta中内容
	else if (html_tag->tag_type == TAG_META)
	{
		char *attr = get_attribute_value(html_tag, ATTR_NAME);
		if(attr != NULL && strcmp(attr, "Description") == 0 && html_tag->html_node->next != NULL)
		{

			char *attr_content = get_attribute_value(html_tag, ATTR_CONTENT);
			int len =  strlen(attr_content);

			StructureKV temp_kv;
			temp_kv.key = 4;
			temp_kv.value = string(attr_content, len);
			data->stdata->all.push_back(temp_kv);

		}
	}

	//mypos | 人均
	else if (html_tag->tag_type == TAG_DIV)
	{
		char *attr = get_attribute_value(html_tag, ATTR_CLASS);
		if(attr != NULL && strcmp(attr, "breadcrumb") == 0 && html_tag->html_node->child != NULL)
		{

			html_node_t *node = html_tag->html_node->child;
			for(; node != NULL; node = node->next)
			{

				if(node->html_tag.tag_type == TAG_B && node->child != NULL)
				{
					char *attr_a = get_attribute_value(&node->child->html_tag, ATTR_HREF);
					int len_a = strlen(attr_a);

					char content[1024] = {0};
					int len = html_node_extract_content(node->child, content, 1024);//文字
					content[len] = '\0';
					char content_black[1024] = {0};
					remove_str_blank(content, content_black);
					len = strlen(content_black);

					if (data->avail + len_a + len + 1 >= data->size)
						return VISIT_ERROR;

					data->avail += sprintf(data->buf+data->avail, "%s %s ", content_black, attr_a);
				}
				else if(node->html_tag.tag_type == TAG_STRONG)
				{
					char content[1024] = {0};
					int len = html_node_extract_content(node->child, content, 1024);//文字

					if (data->avail + len + 1 >= data->size)
						return VISIT_ERROR;

					data->avail += sprintf(data->buf+data->avail, "%s", content);
				}
			}


		}
		else if(attr != NULL && strcmp(attr, "shop-name") == 0 && html_tag->html_node->next != NULL && html_tag->html_node->next->last_child != NULL)
		{
			html_node_t *node_last = html_tag->html_node->next->last_child;
			
			if(node_last->last_child != NULL && node_last->last_child->html_tag.tag_type == TAG_DD)
			{
				char content[50] = {0};
				int len = html_node_extract_content(node_last->last_child, content, 50);

				StructureKV temp_kv;
				temp_kv.key = 3;
				temp_kv.value = string(content, len);
				data->stdata->all.push_back(temp_kv);

			}


		}
	}


	return 0;
}

//结构化数据抽取
int html_tree_extract_stdata(const char *url, int url_len, unsigned int pagetype, html_tree_t *html_tree, StructureData *stdata)
{

	stdata->type = 0;
	stdata->version = 0;
	stdata->all.clear();

	stdata->version = 1;
	stdata->type = pagetype;

	char tmp_buf[40960] = {0};
	char tmp_buf1[40960] = {0};
	char tmp_buf2[40960] = {0};

	char tmp_buf_title[10][40960] = {{0}};
	char tmp_buf_desp[10][40960] = {{0}};

	//百度百科
	if(strncmp(url, "http://baike.baidu.com/view/", 28) == 0 || strncmp(url, "http://baike.baidu.com/subview/", 31) == 0)
	{
		
		//链接
		StructureKV kv_data;
		kv_data.key = 1;
		kv_data.value = string(url, url_len);
		stdata->all.push_back(kv_data);

		visit_for_bk_t data;
		memset(&data, 0, sizeof(visit_for_bk_t));
		data.url = url;
		data.url_len = url_len;
		data.stdata = stdata;
		data.buf = tmp_buf;
		data.avail = 0;
		data.size = 40960;
		data.dir_num = 1;
		data.count = 1;
		data.count1 = 1;

		for(int i = 0; i < 10; i++)
		{
			data.title[i]= tmp_buf_title[i];
			data.desp[i] = tmp_buf_desp[i];
		}
		data.number_title = 0;
		data.number_desp = 0;

		html_tree_visit(html_tree, start_visit_bk, NULL, &data, 0);
		//data.buf[data.avail] = 0;

		if(data.avail > 0)
		{
			StructureKV temp_kv;
			temp_kv.value = string(data.buf, data.avail);
			temp_kv.key = 3;
			stdata->all.push_back(temp_kv);
		}

		for(int i = 0; i< data.number_title; i++)
		{
			int len = strlen(data.desp[i]);
			if(strcmp(data.title[i], "病因") == 0)
			{
				StructureKV temp_kv;
				temp_kv.key = 4;
				temp_kv.value = string(data.desp[i], len);
				stdata->all.push_back(temp_kv);
			}
			else if(strcmp(data.title[i], "症状") == 0)
			{
				StructureKV temp_kv;
				temp_kv.key = 5;
				temp_kv.value = string(data.desp[i], len);
				stdata->all.push_back(temp_kv);
			}
			else if(strcmp(data.title[i], "预防") == 0)
			{
				StructureKV temp_kv;
				temp_kv.key = 6;
				temp_kv.value = string(data.desp[i], len);
				stdata->all.push_back(temp_kv);
			}
			else if(strcmp(data.title[i], "治疗") == 0)
			{
				StructureKV temp_kv;
				temp_kv.key = 7;
				temp_kv.value = string(data.desp[i], len);
				stdata->all.push_back(temp_kv);
			}
			else if(strcmp(data.title[i], "检查") == 0)
			{
				StructureKV temp_kv;
				temp_kv.key = 8;
				temp_kv.value = string(data.desp[i], len);
				stdata->all.push_back(temp_kv);
			}
			else if(strcmp(data.title[i], "诊断") == 0)
			{
				StructureKV temp_kv;
				temp_kv.key = 9;
				temp_kv.value = string(data.desp[i], len);
				stdata->all.push_back(temp_kv);
			}
			else if(strcmp(data.title[i], "并发症") == 0)
			{
				StructureKV temp_kv;
				temp_kv.key = 10;
				temp_kv.value = string(data.desp[i], len);
				stdata->all.push_back(temp_kv);
			}
		}
	}
	else if(strncmp(url, "http://jingyan.baidu.com/article/", 33) == 0)
	{ //百度经验

		//链接
		StructureKV kv_data;
		kv_data.key = 1;
		kv_data.value = string(url, url_len);
		stdata->all.push_back(kv_data);

		visit_for_jy_t data1;
		memset(&data1, 0, sizeof(visit_for_jy_t));
		data1.url = "http://jingyan.baidu.com";
		data1.url_len = 24;
		data1.stdata = stdata;
		data1.buf_img = tmp_buf;
		data1.buf_jy = tmp_buf1;
		data1.buf_jq = tmp_buf2;
		data1.avail_img = 0;
		data1.avail_jy = 0;
		data1.avail_jq = 0;
		data1.size = 40960;
		data1.count = 1;
		html_tree_visit(html_tree, start_visit_jy, NULL, &data1, 0);
		//data1.buf_img[data1.avail_img] = 0;
		//data1.buf_jy[data1.avail_jy] = 0;
		//data1.buf_jq[data1.avail_jq] = 0;

		//图片
		if(data1.avail_img > 0)
		{
			StructureKV temp_kv;
			temp_kv.key = 3;
			temp_kv.value = string(data1.buf_img, data1.avail_img);
			stdata->all.push_back(temp_kv);
		}
		//相关经验
		if(data1.avail_jy > 0)
		{
			StructureKV temp_kv;
			temp_kv.key = 4;
			temp_kv.value = string(data1.buf_jy, data1.avail_jy);
			stdata->all.push_back(temp_kv);
		}
		//技巧大全
		if(data1.avail_jq > 0)
		{
			StructureKV temp_kv;
			temp_kv.key = 5;
			temp_kv.value = string(data1.buf_jq, data1.avail_jq);
			stdata->all.push_back(temp_kv);
		}


	}

	//大众点评
	else if(strncmp(url, "http://www.dianping.com/shop/", 29) == 0 && strstr(url+29, "/") == NULL)
	{

		visit_for_dp_t data;
		memset(&data, 0, sizeof(visit_for_dp_t));
		data.stdata = stdata;
		data.buf = tmp_buf;
		data.avail = 0;
		data.size = 40960;
		html_tree_visit(html_tree, start_visit_dp, NULL, &data, 0);
		//data.buf[data.avail] = 0;
		
		//mypos
		if(data.avail > 0)
		{
			StructureKV temp_kv;
			temp_kv.key = 5;
			temp_kv.value = string(data.buf, data.avail);
			stdata->all.push_back(temp_kv);
		}

	}

	return 0;

}


//通用下载页面
enum extract_type_enum
{
	ENUM_TIME = 0, 
	ENUM_VERSION = 1, 
	ENUM_SIZE = 2, 
	ENUM_OS = 3, 
	ENUM_FREE = 4, 
	ENUM_INTRODUCE = 5, 
	ENUM_LAST = 6,
};

const static char *g_seps[] =
{ "：", ":", "]", "∶", 0 };//分隔


//查找文本中的分隔符的个数
int find_num_seps(char *text, int &end)
{
	char seps[10] = {0};
	int ret = 0;
	for(int i = 0; g_seps[i]; i++)
	{
		char *str = strstr(text, g_seps[i]);
		if(str != NULL)
		{
			if(strlen(str) == strlen(g_seps[i]))
				end = 1;//分隔符结尾
			sprintf(seps, "%s", g_seps[i]);
			break;
		}
	}
	char content[2048] = {0};
	int len = sprintf(content, "%s", text);
	content[len] = '\0';
	while(seps[0] != '\0' && content != NULL)
	{
		char *sub_str = strstr(content, seps);
		if(sub_str != NULL)
		{
			ret++;
			len = sprintf(content, "%s", sub_str + strlen(seps));
			content[len] = '\0';
		}
		else
			break;
	}
	return ret;
}



const static char *g_game_time[] =
{ "更新时间", "更新日期", "发布时间", "发布日期", "更新", "时间", "[时间]", 0 }; //更新时间： 更新日期： 更新： 发布时间：
const static char *g_game_version[] =
{ "版本信息", "版本", "[版 本", "[版本", 0 }; //版本： 版本信息：
const static char *g_game_size[] =
{ "文件大小", "资源大小", "游戏大小", "应用大小", "软件大小", "大小", "[软件大小]", "[文件大小", "[大小", 0 }; //文件大小： 资源大小： 大小： 游戏大小：
const static char *g_game_os[] =
{ "系统要求", "适用系统", "适用机型", "操作系统", "适用于", "支持", "系统", "当前适配机型", "适配机型", "运行环境", "[系统要求]", 0 }; //适用机型： 适用于：
const static char *g_game_free[] =
{ "资费", "收费类型", "是否收费", "应用价格", "授权方式", "授权", "软件性质", "价格", "软件授权", "[资费", "[资 费", 0 }; //资费： 收费类型： 是否收费： 类别：
const static char *g_game_introduce[] =
{ "简介", "介绍", 0 };


struct extract_desp_t
{
	extract_type_enum type;
	const char** words;
	int flag; //用来去除重复 防止一个关键字出现多次
};

extract_desp_t g_desps[ENUM_LAST] =
{
	{ ENUM_TIME, g_game_time, 0 },
	{ ENUM_VERSION, g_game_version, 0 },
	{ ENUM_SIZE, g_game_size, 0 },
	{ ENUM_OS, g_game_os, 0 },
	{ ENUM_FREE, g_game_free, 0 },
	{ ENUM_INTRODUCE, g_game_introduce, 0 }, 
};


//检查文字中包含关键字以及分割符 均不包含返回－1 包含关键字但不包含分割符返回2   均包含且以分割符结尾 返回 1  不以分割符结尾 返回0
//（特别地 对于简介、介绍可无分割符）(特殊情况—key：后是空格)
int is_contain(char *text, extract_desp_t *desp, char *des)
{
	if (text == NULL)
		return -1;
	for (int i = 0; desp->words[i] && desp->flag == 0; i++)
	{
		char *key = strstr(text, desp->words[i]);
		if(key == NULL)
			continue;
		int len_key = strlen(key);
		char *seps;
		for (int j = 0; g_seps[j]; j++) //要保证word之后便是分割符 （eg更新版本：的出现-两个关键字）
		{
			seps = strstr(key, g_seps[j]);
			if(seps == NULL || strlen(seps) + strlen(desp->words[i]) != strlen(key))
				continue;
			else if (strncmp((key + strlen(desp->words[i])), g_seps[j], strlen(g_seps[j])) == 0 && (desp->type == 5 || len_key == strlen(text))) //比较去除空格后的长度
			{
				if (strlen(seps) == strlen(g_seps[j]))
				{
					return 1;
				}
				else
				{
					char *src = seps + strlen(g_seps[j]);
					int len = sprintf(des, "%s", src);
					des[len] = '\0';
					return 0;
				}
			}
			else
				break;
		}
		if (seps == NULL)
			return 2;
	}
	return -1;
}

//找下一个文本节点 但不能包含分隔符-除非是时间、简介字段 classify表示0-app或者1-music
char *find_next_text(html_vnode_t *vnode, int flag, int i, int classify)
{
	char content[2048] = {0};
	if (vnode->nextNode != NULL)
	{
		html_vnode_t *vnode_next = vnode->nextNode;
		int len = html_node_extract_content(vnode_next->hpNode, content, 2048);
		content[len] = '\0';
		if(flag == 1)
			return (len > 0 ? content : NULL);
		else if(flag == 2)
			return (len > 10 ? content : NULL);
	}
	else if (vnode->upperNode != NULL && vnode->upperNode->nextNode != NULL)
	{
		html_vnode_t *vnode_upper = vnode->upperNode->nextNode;
		if (vnode_upper->hpNode->html_tag.tag_type == TAG_PURETEXT)
		{
			char *text = vnode_upper->hpNode->html_tag.text;
			if(classify == 0)
			{
				if(flag == 1 && (i == ENUM_TIME || i == ENUM_INTRODUCE || (strstr(text, "：") == NULL && strstr(text, ":") == NULL)))
					return (strlen(text) > 0 ? text : NULL);
				else if(flag == 2 && (i == ENUM_TIME || i == ENUM_INTRODUCE || (strstr(text, "：") == NULL && strstr(text, ":") == NULL)))
					return (strlen(text) > 10 ? text : NULL);
			}
			else if(classify == 1)
			{
				if(flag == 1 && (i == 1 || (strstr(text, "：") == NULL && strstr(text, ":") == NULL)))
					return (strlen(text) > 0 ? text : NULL);
				else if(flag == 2 && (i == 1 || (strstr(text, "：") == NULL && strstr(text, ":") == NULL)))
					return (strlen(text) > 10 ? text : NULL);
			}
		}
		else
		{
			int len = html_node_extract_content(vnode_upper->hpNode->child, content, 2048);
			content[len] = '\0';
			if(classify == 0)
			{
				if(flag == 1 && (i == ENUM_TIME || i == ENUM_INTRODUCE || (strstr(content, "：") == NULL && strstr(content, ":") == NULL)))
					return (len > 0 ? content : NULL);
				else if(flag == 2 && (i == ENUM_TIME || i == ENUM_INTRODUCE || (strstr(content, "：") == NULL && strstr(content, ":") == NULL)))
					return (len > 10 ? content : NULL);
			}
			else if(classify == 1)
			{
				if(flag == 1 && (i == ENUM_TIME || i == ENUM_INTRODUCE || (strstr(content, "：") == NULL && strstr(content, ":") == NULL)))
					return (len > 0 ? content : NULL);
				else if(flag == 2 && (i == 1 || (strstr(content, "：") == NULL && strstr(content, ":") == NULL)))
					return (len > 10 ? content : NULL);
			}
		}
	}
	return NULL;
}

struct extract_mypos
{
	const char *url;//页面url
	int url_len;
	char *content;//存放mypos文字+url
	int con_len;
	int size;
	char *seps;//存放mypos中的分隔符
};

struct extract_download
{
	StructureData *stdata;
	int flag;//主要保证下载地址只取一个
	const char *url;//下载地址是相对路径时，进行补充
	int url_len;
};


void remove_space(char *text, int ret)//ret为分隔符个数 大于1时 也用空格分割
{
	int len = strlen(text);
	if((ret == 0 || ret == 1) && (text[0] >= 'a' && text[0] <= 'z') || (text[0] >= 'A' && text[0] <= 'Z'))
	{
		for(int i = 0; i < len; i++)
		{
			if(i != 0 && (text[i] == '\n' || text[i] == '\t' || text[i] == '-' || text[i] == '_' || text[i] == '('))
			{
				text[i] = '\0';
				break;
			}
		}
	}
	else
	{	for(int i = 0; i < len; i++)
		{
			if(i != 0 && (text[i] == ' ' || text[i] == '\n' || text[i] == '\t' || text[i] == '-' || text[i] == '_' || text[i] == '('))
			{
				text[i] = '\0';
				break;
			}
		}
	}
}

//主要找文字中的key_value形式(http://down.tech.sina.com.cn/content/49535.html 其中 简介提取不到——原本提取到的是评论条)
int start_for_key_value(html_vnode_t *vnode, void *data)
{
	extract_download *down = (extract_download *)data;

	char *attr_title = get_attribute_value(&vnode->hpNode->html_tag, ATTR_TITLE);
	char *attr_href = get_attribute_value(&vnode->hpNode->html_tag, ATTR_HREF);//可能为下载地址，保证其以/或者http://或者＃开头(＃表示该网页内部跳转)

	//if (!vnode->isValid)
	//	return VISIT_SKIP_CHILD;

	if (vnode->hpNode->html_tag.tag_type == TAG_PURETEXT)
	{
		char *text = vnode->hpNode->html_tag.text;
		if(strlen(text) > 2048)
			return VISIT_SKIP_CHILD;
		char text_copy[2048] = {0};
		char text_blank[2048] = {0};
		int len_text = sprintf(text_copy, "%s", text);
		text_copy[len_text] = '\0';

		int end = 0;
		int num = find_num_seps(text_copy, end);
		if(num == 1)//???分隔符后面的字符不应去空格???
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

		for (int i = ENUM_TIME; i < ENUM_LAST; i++) //若分隔符结尾，则所取长度不能超过30
		{
			//printf("g_desps[%d].flag=%d\t", i, g_desps[i].flag);
			char des[2048] = {0};
			int ret = is_contain(text_blank, &(g_desps[i]), des); //若两个关键字放同一个文本块，则会丢失???。需增加参数-分隔符个数！！！
			//printf("text=%s,ret=%d\n", text_blank, ret);
			if (ret == 0) //分割符不结尾
			{
				char text_end[2048] = {0};
				//检查text是否包含关键字 且 包含分割符 分割符结尾 或 不结尾(去除转义字符)
				int len = copy_html_text(text_end, 0, 2048, des);
				//针对资费，调整—不能含有空白字符
				if(i == ENUM_FREE)
					remove_space(text_end, 0);	
				StructureKV temp_kv;
				temp_kv.key = i;
				temp_kv.value = string(text_end, strlen(text_end));
				if(len > 0)
				{
					down->stdata->all.push_back(temp_kv);	
					g_desps[i].flag = 1;
				}
			}
			else if (ret == 1 || (ret == 2 && i == ENUM_INTRODUCE)) //分割符结尾 先找兄度节点的文字 后找其父节点的兄弟节点的文字
			{
				char *text_next = find_next_text(vnode, ret, i, 0);

				if (text_next != NULL)
				{
					char text_end[2048] = {0};//简介取前100个字
					int len = copy_html_text(text_end, 0, 2048, text_next);
					//printf("222i=%d\ttext=%s\t%s\n", i, text, text_end);
					StructureKV temp_kv;
					temp_kv.key = i;
					temp_kv.value = string(text_end, len);
					if(len > 0)
					{
						down->stdata->all.push_back(temp_kv);	
						g_desps[i].flag = 1;
					}

				}
			}
		}
		return VISIT_SKIP_CHILD;
	}
	//加入纵坐标，防止提取到头部的广告下载地址
	else if(vnode->ypos > 90 && vnode->hpNode->html_tag.tag_type == TAG_A && attr_href != NULL)
	{
		char content[2048] = {0};
		int len = html_node_extract_content(vnode->hpNode, content, 2048);
		content[len] = '\0';

		if(((attr_title != NULL && strstr(attr_title, "下载") != NULL) || (content != NULL && strstr(content, "下载") != NULL && strstr(content, "下载首页") == NULL)) && (strstr(attr_href, "down") != NULL || strstr(attr_href, ".exe") != NULL || strstr(attr_href, ".jar") != NULL || strstr(attr_href, ".zip") != NULL || strstr(attr_href, ".rar") != NULL) && strstr(attr_href, "help") == NULL && down->flag == 0)
		{
			char url_end[2048] = {0};
			char url_full[2048] = {0};
			int len_url_end = copy_html_text(url_end, 0, 2048, attr_href);
			url_end[len_url_end] = '\0';
			int len_url_full = 0;

			//针对相对路径以及转移字符进行操作
			if (strncmp(url_end, "http://", 7) == 0)
			{
				len_url_full = sprintf(url_full, "%s", url_end);
			}
			else if(strncmp(url_end, "#", 1) == 0)
			{
				len_url_full = sprintf(url_full, "%s%s", down->url, url_end);

			}
			else if(strncmp(url_end, "/", 1) == 0 || strncmp(url_end, "../", 3) == 0)
			{
				strncpy(url_full, down->url, down->url_len);
				url_full[down->url_len] = '\0';
				char *ptr = strchr(url_full + 7, '/');
				int pos = ptr - url_full;
				len_url_full = sprintf(url_full+pos, "%s", strstr(url_end, "/"));
				len_url_full += pos;
				//strcpy(url_full + pos, url_end);
				
			}
			else
			{
				strncpy(url_full, down->url, down->url_len);
				url_full[down->url_len] = '\0';
				char *ptr = strrchr(url_full + 7, '/');
				int pos = ptr - url_full;
				len_url_full = sprintf(url_full+pos+1, "%s", url_end);
				len_url_full += pos+1;
			}

			StructureKV temp_kv;
			temp_kv.key = 8;
			temp_kv.value = string(url_full, len_url_full);
			if(len_url_full > 0)
			{
				down->stdata->all.push_back(temp_kv);
				down->flag = 1;
				if(strstr(content, "免费") != 0 && g_desps[4].flag == 0)//下载锚文中出现了免费字眼
				{
					StructureKV temp_kv;
					temp_kv.key = 4;
					temp_kv.value = string("免费", 4);
					down->stdata->all.push_back(temp_kv);
					g_desps[4].flag = 1;
				}
			}
		}
		return VISIT_SKIP_CHILD;
	}
	return VISIT_NORMAL;
}

//提取mypos(部分url未去掉mypos中的分隔符)
int start_for_mypos(html_vnode_t *vnode, void *data) //找文字节点  （包含当前位置 或者 mypos的分割符等均要去掉）
{
	extract_mypos *mypos = (extract_mypos *)data;
	if (!vnode->isValid)
		return VISIT_SKIP_CHILD;

	if (vnode->hpNode->html_tag.tag_type == TAG_PURETEXT && vnode->hpNode->html_tag.textlength < 20)
	{
		char content[4096] = {0};
		int len = html_node_extract_content(vnode->hpNode, content, 4096);
		content[len] = '\0';

		if (len < 4 || strstr(content, "位置：") != 0 || strstr(content, "位置:") != 0 || strstr(content, "当前位置") != 0)
		{
			if(len < 4)
			{
				strncpy(mypos->seps, content, len);//存分隔符
				mypos->seps[len]= '\0';
			}
			return VISIT_SKIP_CHILD;
		}

		//需要判断是否有空间写入
		//if (len + 1 >= mypos->size)
		//	return VISIT_ERROR;

		//去掉空白字符 以及 mypos的分隔符 对content转义

		clean_string(content);
		clean_string(mypos->seps);

		//判断文字中是否包含分隔符

		if(mypos->seps != NULL && strncmp(content, mypos->seps, strlen(mypos->seps)) == 0)
			mypos->con_len += sprintf(mypos->content + mypos->con_len, "%s ", content+strlen(mypos->seps));
		else if(strncmp(content, ">>", 2) == 0 || strncmp(content, "->", 2) == 0)
			mypos->con_len += sprintf(mypos->content + mypos->con_len, "%s ", content+2);
		else if(strncmp(content, ">", 1) == 0)
		{
			mypos->con_len += sprintf(mypos->content + mypos->con_len, "%s ", content+1);
		}
		else
			mypos->con_len += sprintf(mypos->content + mypos->con_len, "%s ", content);

		char *attr_href = get_attribute_value(&vnode->upperNode->hpNode->html_tag, ATTR_HREF);
		if (vnode->upperNode->hpNode->html_tag.tag_type == TAG_A && attr_href != NULL)
		{
			char href[2048] = {0};
			copy_html_text(href, 0, 2048, attr_href);
			if (strncmp(attr_href, "http://", 7) == 0)
			{
				mypos->con_len += sprintf(mypos->content + mypos->con_len, "%s ", href);
			}
			else if(strncmp(attr_href, "/", 1) == 0 || strncmp(attr_href, "../", 3) == 0)
			{
				char url_new[2048] = {0};
				strncpy(url_new, mypos->url, mypos->url_len);
				url_new[mypos->url_len] = '\0';
				char *ptr = strchr(url_new + 7, '/');
				int pos = ptr - url_new;
				strcpy(url_new + pos, strstr(href, "/"));
				mypos->con_len += sprintf(mypos->content + mypos->con_len, "%s ", url_new);
			}
			//相对路径，但无/
			else
			{
				char url_new[2048] = {0};
				strncpy(url_new, mypos->url, mypos->url_len);
				url_new[mypos->url_len] = '\0';
				char *ptr = strrchr(url_new + 7, '/');
				int pos = ptr - url_new;
				strcpy(url_new + pos + 1, href);
				mypos->con_len += sprintf(mypos->content + mypos->con_len, "%s ", url_new);
			}
		}
	}
	return VISIT_NORMAL;
}

//获取某个节点下的img标签的src属性
char *get_sub_img_vnode(html_vnode_t *vnode, const char *url, int url_len)
{
	std::stack<html_vnode_t *> stack_node; //定义栈

	if (vnode == NULL || vnode->firstChild == NULL)
		return NULL;

	html_vnode_t *node = vnode->firstChild;

	while (node != NULL || !stack_node.empty())
	{
		while (node != NULL)
		{
			//去除定义height和width的img标签，再去掉后缀名为.gif的src
			char *attr_src = get_attribute_value(&node->hpNode->html_tag, ATTR_SRC);
			char *attr_height = get_attribute_value(&node->hpNode->html_tag, ATTR_HEIGHT);
			char *attr_width = get_attribute_value(&node->hpNode->html_tag, ATTR_WIDTH);
			//if (node->hpNode->html_tag.tag_type == TAG_IMG && attr_src != NULL && strstr(attr_src, ".gif") == NULL && attr_height == NULL && attr_width == NULL)
			if (node->hpNode->html_tag.tag_type == TAG_IMG && attr_src != NULL && attr_height == NULL && attr_width == NULL)
			{
				//printf("src=%s\txpos=%d, ypos=%d, wx=%d, hx=%d, trust=%d\n", attr_src, node->xpos, node->ypos, node->wx, node->hx, node->trust);
				//node = node->nextNode;
				if (strncmp(attr_src, "http://", 7) == 0)
					return attr_src;
				else if(strncmp(attr_src, "/", 1) == 0)
				{
					static char url_new[2048] = {0};
					strncpy(url_new, url, url_len);
					url_new[url_len] = '\0';
					char *ptr = strchr(url_new + 7, '/');
					int pos = ptr - url_new;
					strcpy(url_new + pos, attr_src);
					return url_new;
				}
				else
					//return NULL;	
					break;
			}
			else
			{
				stack_node.push(node);
				node = node->firstChild;
			}

		}
		if (!stack_node.empty())
		{
			node = stack_node.top();
			stack_node.pop();
			node = node->nextNode;
		}
	}
	return NULL;
}


//通用下载页面
int html_tree_extract_download_stdata(const char *url, int url_len, unsigned int pagetype, html_tree_t *html_tree, html_vtree_t *html_vtree, area_tree_t *atree, StructureData *stdata)
{
	stdata->type = 0;
	stdata->version = 0;
	stdata->all.clear();

	stdata->version = 1;
	stdata->type = pagetype;

	extract_download down;
	memset(&down, 0, sizeof(extract_download));
	down.stdata = stdata;
	down.flag = 0;
	down.url = url;
	down.url_len = url_len;
	
	//提取mypos 文字+url

	extract_mypos mypos;
	//init
	char content[4096] = {0};
	char seps[4] = {0};
	memset(&mypos, 0, sizeof(extract_mypos));
	mypos.url = url;
	mypos.url_len = url_len;
	mypos.content = content;
	mypos.con_len = 0;
	mypos.size = 4096;
	mypos.seps = seps;

	
	//提取key_value格式 以及下载地址url
	//因为会修改g_desps的flag变量，所以要每次初始化
	for(int i = ENUM_TIME; i < ENUM_LAST; i++)
		g_desps[i].flag = 0;

	html_vtree_visit(html_vtree, start_for_key_value, NULL, &down);

	const area_list_t *alist = get_func_mark_result(atree, AREA_FUNC_MYPOS);
	if (alist && alist->head && alist->head->area->begin)
	{
		//mypos
		html_vnode_t *vnode_t = alist->head->area->begin;
		if(vnode_t->hpNode->html_tag.tag_type == TAG_DIV || vnode_t->hpNode->html_tag.tag_type == TAG_TABLE)
			html_vnode_visit_ex(vnode_t, start_for_mypos, NULL, &mypos);
		else
		{
			html_tag_type_t type = vnode_t->hpNode->html_tag.tag_type;
			for(html_vnode_t *v_node = vnode_t; v_node && (v_node->hpNode->html_tag.tag_type == type || v_node->hpNode->html_tag.tag_type == TAG_PURETEXT); v_node = v_node->nextNode)
				html_vnode_visit_ex(v_node, start_for_mypos, NULL, &mypos);
		}
		//LOGO 找有效节点
		while(vnode_t->nextNode != NULL && !vnode_t->nextNode->isValid)
			vnode_t = vnode_t->nextNode;
		if(vnode_t->nextNode != NULL)
		{
			char *attr_value = get_sub_img_vnode(vnode_t->nextNode, url, url_len);
			if(attr_value != NULL)	
			{
				StructureKV temp_kv;
				temp_kv.value = string(attr_value, strlen(attr_value));
				temp_kv.key = 7;
				stdata->all.push_back(temp_kv);
			}

		}

	}

	
	if(mypos.con_len > 0)
	{
		StructureKV temp_kv;
		temp_kv.value = string(mypos.content, mypos.con_len);
		temp_kv.key = 6;
		stdata->all.push_back(temp_kv);
		
	}

	return 0;
}

//MP3类
enum extract_music_type_enum
{
	ENUM_MUSIC_SING = 0, 
	ENUM_MUSIC_TIME = 1, 
	ENUM_MUSIC_ALBUM = 2, 
	ENUM_MUSIC_COMPANY = 3, 
	ENUM_MUSIC_LYRICS = 4, 
	ENUM_MUSIC_COMPOSITION = 5, 
	ENUM_MUSIC_SONG = 6, 
	ENUM_MUSIC_VERTION = 7, 
	ENUM_MUSIC_SIZE = 8, 
	ENUM_MUSIC_LAST = 9,
};


const static char *g_music_sing[] =
{ "演唱者", "艺人", "歌手名", "歌手", "演唱", "原唱", "歌手姓名", "歌手名称", 0 }; //歌手名
const static char *g_music_time[] =
{ "发行时间", "发布时间", "时间", 0 }; //发行时间
const static char *g_music_album[] =
{ "专辑类别", "所属专辑", "歌曲专辑", "专辑", 0 }; //所属专辑
const static char *g_music_company[] =
{ "唱片公司", "发行公司", 0 }; //唱片公司
const static char *g_music_lyrics[] = 
{ "作词", "词曲", "词/曲", "作词/作曲", "词", 0 }; //作词
const static char *g_music_composition[] = 
{ "作曲", "词曲", "词/曲", "作词/作曲", "曲", 0 }; //作曲 (曲关键字与歌曲关键字冲突)
const static char *g_music_song[] =
{ "歌曲名", "歌名", "歌曲", "铃声名称", "您正在试听", "歌曲试听", 0 }; //歌曲名"正在试听"
const static char *g_music_version[] =
{ "版本信息", "版本", 0 }; //版本： 版本信息：
const static char *g_music_size[] =
{ "文件大小", "资源大小", "大小", 0 }; //文件大小： 资源大小： 大小： 


struct extract_music_desp_t
{
	extract_music_type_enum type;
	const char** words;
	int flag; //用来去除重复 防止一个关键字出现多次
};


extract_music_desp_t g_music_desps[ENUM_MUSIC_LAST] =
{
	{ ENUM_MUSIC_SING, g_music_sing, 0 },
	{ ENUM_MUSIC_TIME, g_music_time, 0 },
	{ ENUM_MUSIC_ALBUM, g_music_album, 0 },
	{ ENUM_MUSIC_COMPANY, g_music_company, 0 },
	{ ENUM_MUSIC_LYRICS, g_music_lyrics, 0 },
	{ ENUM_MUSIC_COMPOSITION, g_music_composition, 0 }, 
	{ ENUM_MUSIC_SONG, g_music_song, 0 }, 
	{ ENUM_MUSIC_VERTION, g_music_version, 0 }, 
	{ ENUM_MUSIC_SIZE, g_music_size, 0 }, 
};


//检查文字中包含关键字以及分割符 均不包含返回－1 包含关键字但不包含分割符返回2   均包含且以分割符结尾 返回 1  不以分割符结尾 返回0 des保存分隔符后的内容
int is_music_contain(char *text, extract_music_desp_t *desp, char *des, int ret)
{
	if (text == NULL)
		return -1;
	for (int i = 0; desp->words[i] && desp->flag == 0; i++)
	{
		//若有分隔符，需要保证找到的文本大小与关键字大小一样（反例—推荐歌曲：）
		//if(strncmp(text, desp->words[i]))
		char *key = strstr(text, desp->words[i]);
		if (key == NULL)
			continue;
		char *seps = 0;
		for (int j = 0; g_seps[j]; j++) //要保证word之后便是分割符 （eg更新版本：的出现-两个关键字）
		{
			seps = strstr(key, g_seps[j]);
			if(seps == NULL)	
				continue;
			else if(ret > 1 && strncmp((key + strlen(desp->words[i])), g_seps[j], strlen(g_seps[j])) == 0)
			{
				//专门针对http://music.show160.com/381973这种页面 http://www.mtv123.com/mp3/16426/195251.shtml
				char src[2048] = {0};
				sprintf(src, "%s", seps+strlen(g_seps[j]));
				char *sub = strstr(src, "，");
				int len_sub = 0;
				if(sub != NULL)//，分割
					len_sub = strlen(sub);
				else
					remove_space(src, ret);//空格分割
				int len = snprintf(des, strlen(src)-len_sub+1, "%s", src);
				des[len] = '\0';
				return 0;
			}
			else if (ret == 1 && strncmp((key + strlen(desp->words[i])), g_seps[j], strlen(g_seps[j])) == 0 && strlen(key)== strlen(text))
			{
					if (strlen(seps) == strlen(g_seps[j]))
						return 1;
					else
					{
						char *src = seps + strlen(g_seps[j]);
						int len = sprintf(des, "%s", src);
						des[len] = '\0';
						return 0;
					}
			}
			else
				break;
		}
		if (seps == NULL)
			return 2;
	}
	return -1;
}

struct extract_music
{
	StructureData *stdata;
	char *title;//<title>
	char *singer;//演唱者与歌曲名，1.与title作对比 2.从title中查找
	char *song;
	int num_br;//<BR>的个数
	char *content;//存歌词
	int len;
	unsigned int pagetype;
	const char *url;
	int url_len;
	int flag;
};


//针对歌曲、歌手名，去掉多余的字符—MP3，歌词，（，.等
void remove_false_str(char *text)
{
	char *wrong[] = {"MP3", "歌词", "(", "（", ".", 0};
	for(int i = 0; wrong[i] && text[0] != '\0'; i++)
	{
		char *str = strstr(text, wrong[i]);
		if(str != NULL)
			text[str - text] = '\0';
	}
}

//MP3类的key_value
int start_for_music_key_value(html_vnode_t *vnode, void *data)
{
	extract_music *music = (extract_music *)data;

	char *attr_title = get_attribute_value(&vnode->hpNode->html_tag, ATTR_TITLE);
	char *attr_href = get_attribute_value(&vnode->hpNode->html_tag, ATTR_HREF);//可能为下载地址，保证其以/或者http://或者＃开头(＃表示该网页内部跳转)

	if (!vnode->isValid)
		return VISIT_SKIP_CHILD;

	if (vnode->hpNode->html_tag.tag_type == TAG_PURETEXT)
	{
		char *text = vnode->hpNode->html_tag.text;
		int len_text = strlen(text);
		if(len_text == 0 || len_text > 2048)
			return VISIT_SKIP_CHILD;

		char text_copy[2048] = {0};
		char text_blank[2048] = {0};
		len_text = sprintf(text_copy, "%s", text);
		text_copy[len_text] = '\0';

		int end = 0;
		int num = find_num_seps(text_copy, end); //特例-时间：2013-09-16 19:23
		if(num == 1)
		{
			clean_string(text_copy);
			copy_html_text(text_copy, 0, 2048, text_copy);
			if(strstr(text_copy, "时间") == NULL && end == 1)
			{
				remove_str_blank(text_copy, text_blank);
				len_text = strlen(text_blank);
			}
			else
			{
				len_text = sprintf(text_blank, "%s", text_copy);
				text_blank[len_text] = '\0';

			}
		}
		else
		{
			len_text = sprintf(text_blank, "%s", text_copy);
			text_blank[len_text] = '\0';
		}
		for (int i = ENUM_MUSIC_SING; i < ENUM_MUSIC_LAST && len_text > 3; i++)//除了歌词，其余key_value值都应在30个长度以内
		{
			char des[2048] = {0};
			int ret = is_music_contain(text_blank, &(g_music_desps[i]), des, num);//关键字均采用完全匹配规则
			//printf("des=%s, ret=%d, i=%d\n", des, ret, i);

			if (ret == 0) //分割符不结尾
			{
				char text_end[2048] = {0};
				//检查text是否包含关键字 且 包含分割符 分割符结尾 或 不结尾(去除转义字符)
				int len = copy_html_text(text_end, 0, 2048, des);
				clean_string(text_end);
				if(i != ENUM_MUSIC_TIME)
					remove_space(text_end, 0);
				if(i == 1 || i == 6)
					remove_false_str(des);

				len = strlen(text_end);
				//printf("111i=%d,text=%s,title=%s\n", i, text_end, music->title);//value为分隔符之后的内容
				if(music->title[0] != '\0' && len > 0)
				{
					/*char titles[1024];
					char titles_non[1024];
					sprintf(titles, "%s", music->title);
					remove_str_blank(titles, titles_non);*/

					if(music->pagetype == 17 || (i != 0 && i != 6) || (strstr(music->title, text_end) != NULL))
					{
						g_music_desps[i].flag = 1;
						StructureKV temp_kv;
						temp_kv.key = i;
						temp_kv.value = string(text_end, len);
						music->stdata->all.push_back(temp_kv);
						if(i == 0 && len < 300)
							sprintf(music->singer, "%s", text_end);
						else if(i == 6 && len < 300)
							sprintf(music->song, "%s", text_end);
						return VISIT_SKIP_CHILD;
					}
				}
				else if(music->title[0] == '\0' && len > 0)
				{
					g_music_desps[i].flag = 1;
					StructureKV temp_kv;
					temp_kv.key = i;
					temp_kv.value = string(text_end, len);
					music->stdata->all.push_back(temp_kv);
					return VISIT_SKIP_CHILD;
				}
			}
			//else if (ret == 1 || ret == 2) //分割符结尾 先找兄度节点的文字 后找其父节点的兄弟节点的文字
			else if (len_text < 50 && ret == 1) //分割符结尾 先找兄度节点的文字 后找其父节点的兄弟节点的文字
			{
				char *text_next = find_next_text(vnode, ret, i, 1);
				if (text_next != NULL)
				{
					char text_end[2048] = {0};
					copy_html_text(text_end, 0, 2048, text_next);
					//printf("222i=%d,text=%s-%s\n", i, text, text_end);
					if(i == 1 || i == 6)
						remove_false_str(text_end);
					int len = strlen(text_end);
					if(music->title[0] != '\0' && len > 0)
					{
						if(music->pagetype == 17 || music->pagetype == 22 || (i != 0 && i != 6) || (strstr(music->title, text_end) != NULL))
						{
							clean_string(text_end);
							len = strlen(text_end);
							g_music_desps[i].flag = 1;
							StructureKV temp_kv;
							temp_kv.key = i;
							temp_kv.value = string(text_end, len);
							music->stdata->all.push_back(temp_kv);
							if(i == 0 && len < 300)
								sprintf(music->singer, "%s", text_end);
							else if(i == 6 && len < 300)
								sprintf(music->song, "%s", text_end);
							return VISIT_SKIP_CHILD;
						}
					}
				}
			}
		}
		return VISIT_SKIP_CHILD;
	}
	else if(vnode->ypos > 90 && vnode->hpNode->html_tag.tag_type == TAG_A && attr_href != NULL)
	{
		char content[2048] = {0};
		int len = html_node_extract_content(vnode->hpNode, content, 2048);
		content[len] = '\0';

		if(((attr_title != NULL && strstr(attr_title, "下载") != NULL) || (content != NULL && strstr(content, "下载") != NULL && strstr(content, "下载首页") == NULL)) && (strstr(attr_href, "down") != NULL || strstr(attr_href, ".exe") != NULL || strstr(attr_href, ".jar") != NULL || strstr(attr_href, ".zip") != NULL || strstr(attr_href, ".rar") != NULL) && strstr(attr_href, "help") == NULL && music->flag == 0)
		{
			char url_end[2048] = {0};
			char url_full[2048] = {0};
			int len_url_end = copy_html_text(url_end, 0, 2048, attr_href);
			url_end[len_url_end] = '\0';
			int len_url_full = 0;

			//针对相对路径以及转移字符进行操作
			if (strncmp(url_end, "http://", 7) == 0)
			{
				len_url_full = sprintf(url_full, "%s", url_end);
			}
			else if(strncmp(url_end, "#", 1) == 0)
			{
				len_url_full = sprintf(url_full, "%s%s", music->url, url_end);

			}
			else if(strncmp(url_end, "/", 1) == 0 || strncmp(url_end, "../", 3) == 0)
			{
				strncpy(url_full, music->url, music->url_len);
				url_full[music->url_len] = '\0';
				char *ptr = strchr(url_full + 7, '/');
				int pos = ptr - url_full;
				len_url_full = sprintf(url_full+pos, "%s", strstr(url_end, "/"));
				len_url_full += pos;
				//strcpy(url_full + pos, url_end);
				
			}
			else
			{
				strncpy(url_full, music->url, music->url_len);
				url_full[music->url_len] = '\0';
				char *ptr = strrchr(url_full + 7, '/');
				int pos = ptr - url_full;
				len_url_full = sprintf(url_full+pos+1, "%s", url_end);
				len_url_full += pos+1;
			}

			StructureKV temp_kv;
			temp_kv.key = 9;
			temp_kv.value = string(url_full, len_url_full);
			if(len_url_full > 0)
			{
				music->stdata->all.push_back(temp_kv);
				music->flag = 1;
			}
		}
		return VISIT_SKIP_CHILD;
	}
	return VISIT_NORMAL;
}

void remove_time(char *text)
{
	char *str_so = strchr(text, '[');
	char *str_eo = strchr(text, ']');
	if(str_so != NULL && str_eo != NULL)
	{
		int pos_so = str_so - text;
		int pos_eo = str_eo - text;
		if(pos_eo - pos_so == 9)
		{
			if(pos_so == 0)
			{
				sprintf(text, "%s", text+pos_eo+1);
			}
			else
				text[pos_so] = '\0';
		}
	}

}
//<title>中的文字 以及TEXT-BR文本-歌词
int start_for_title_br(html_tag_t* html_tag, void* result, int flag)
{
	extract_music *music = (extract_music *)result;
	if(html_tag->tag_type == TAG_TITLE)
	{
		char content[1024] = {0};
		int len = html_node_extract_content(html_tag->html_node, content, 1024);
		content[len] = '\0';
		copy_html_text(content, 0, 1024, content);
		sprintf(music->title, "%s", content);
		return VISIT_SKIP_CHILD;
	}
	else if(html_tag->tag_type == TAG_PURETEXT && html_tag->html_node->next != NULL && html_tag->html_node->next->html_tag.tag_type == TAG_BR)
	{
		char text[2048] = {0};
		if(strlen(html_tag->text) > 2048)
			return VISIT_SKIP_CHILD;
		sprintf(text, "%s", html_tag->text);
		if(text != NULL && music->num_br < 10)//取前10个BR的文字，歌词满足连续10个均无：分隔符
		{
			remove_time(text);
			if(strlen(text) < 50 && strstr(text, "：") == NULL && strstr(text, "www.") == NULL && strstr(text, ":") == NULL && strstr(text, "-") == NULL && strstr(text, "=") == NULL && strstr(text, "∶") == NULL && strstr(text, "歌词") == NULL)
			{
				music->num_br++;
				if(music->num_br < 4)
				{
					//需去除content中的时间以及开头的换行符 eg[00:00:00]
					copy_html_text(text, 0, 2048, text);
					music->len += sprintf(music->content + music->len, "%s ", text);
					music->content[music->len] = '\0';
				}
				return VISIT_SKIP_CHILD;
			}
			else
			{
				music->num_br = 0;
				music->len = 0;
				music->content[0] = '\0';
			}
		}
	}
	return VISIT_NORMAL;
}



//MP3类页面的提取
int html_tree_extract_music_stdata(const char *url, int url_len, unsigned int pagetype, html_tree_t *html_tree, html_vtree_t *html_vtree, area_tree_t *atree, StructureData *stdata)
{

	stdata->type = 0;
	stdata->version = 0;
	stdata->all.clear();

	stdata->version = 1;
	stdata->type = pagetype;

	char title[1024] = {0};
	char singer[300] = {0};
	char song[300] = {0};
	char content[2048] = {0};
	extract_music music;
	memset(&music, 0, sizeof(extract_music));
	music.stdata = stdata;
	music.title = title;
	music.singer = singer;
	music.song = song;
	music.num_br = 0;
	music.content = content;
	music.len = 0;
	music.pagetype = pagetype;
	music.url = url;
	music.url_len = url_len;
	music.flag = 0;

	for(int i = ENUM_MUSIC_SING; i < ENUM_MUSIC_LAST; i++)
		g_music_desps[i].flag = 0;

	printf("parser-url=%s\n", url);		
	html_tree_visit(html_tree, start_for_title_br, NULL, &music, 0);

	html_vtree_visit(html_vtree, start_for_music_key_value, NULL, &music);


	char songs[300] = {0};
	char singers[300] = {0};
	
	//针对歌曲名i=6与歌手名i=0 存在演唱者 找歌曲
	if(music.title[0] != '\0' && g_music_desps[0].flag == 1 && g_music_desps[6].flag == 0)
	{
		char *str = strstr(music.title, music.singer);
		if(str != NULL && strlen(str) < strlen(music.title))//title的格式为歌曲+歌手
		{
			snprintf(songs, strlen(music.title) - strlen(str) +1, "%s", music.title);
			clean_string(songs);
			remove_space(songs, 0);
			remove_false_str(songs);
			sprintf(music.song, "%s", songs);

		}
		else if(str != NULL && strlen(str) == strlen(music.title)) //title的格式为歌手+歌曲
		{
			sprintf(songs, "%s", music.title + strlen(music.singer));
			clean_string(songs);
			char *space = strchr(songs, ' ');
			if(space != NULL)
			{
				int pos = space - songs;
				if(pos < 5)
					sprintf(songs, "%s", songs+pos+1);
			}
			remove_space(songs, 0);
			remove_false_str(songs);
			if(songs[0] != '\0')
				sprintf(music.song, "%s", songs);
		}
		if(strlen(music.song) > 0)
		{
			g_music_desps[6].flag = 1;
			StructureKV temp_kv;
			temp_kv.key = 6;
			temp_kv.value = string(music.song, strlen(music.song));
			music.stdata->all.push_back(temp_kv);
		}
	}
	//存在歌曲名 找演唱者
	else if(music.title[0] != '\0' && g_music_desps[0].flag == 0 && g_music_desps[6].flag == 1)
	{
		char *str = strstr(music.title, music.song);
		if(str != NULL && strlen(str) < strlen(music.title))//title的格式为歌手+歌曲
		{
			snprintf(singers, strlen(music.title) - strlen(str) +1, "%s", music.title);
			clean_string(singers);
			remove_space(singers, 0);
			remove_false_str(singers);
			sprintf(music.singer, "%s", singers);

		}
		else if(str != NULL && strlen(str) == strlen(music.title))
		{
			sprintf(singers, "%s", music.title + strlen(music.song));
			clean_string(singers);
			char *space = strchr(singers, ' ');
			if(space != NULL)
			{
				int pos = space - singers;
				if(pos < 5)
					sprintf(singers, "%s", singers + pos + 1);
			}
			remove_space(singers, 0);
			remove_false_str(singers);
			sprintf(music.singer, "%s", singers);

		}
		if(strlen(music.singer) > 0)
		{
			g_music_desps[0].flag = 1;
			StructureKV temp_kv;
			temp_kv.key = 0;
			temp_kv.value = string(music.singer, strlen(music.singer));
			music.stdata->all.push_back(temp_kv);
		}

	}

	//歌词
	if(pagetype != 18 && music.num_br > 8 && music.len > 0)
	{
		StructureKV temp_kv;
		temp_kv.key = 10;
		temp_kv.value = string(music.content, music.len);
		music.stdata->all.push_back(temp_kv);
	}

	return 0;
}

//序列化
int html_tree_extract_serial(StructureData *stdata, uint8_t *buf_ptr, uint32_t size)
{
	shared_ptr<TMemoryBuffer> mem_buf(new TMemoryBuffer);
	shared_ptr<TBinaryProtocol> bin_proto(new TBinaryProtocol(mem_buf));

	stdata->write(bin_proto.get());

	uint8_t* ptr;
	uint32_t len;
	mem_buf->getBuffer(&ptr, &len);

	if (len >= size)
		return -1;

	memcpy(buf_ptr, ptr, len);
	buf_ptr[len] = 0;

	return len;
}

//反序列化化
int html_tree_extract_deserial(StructureData *stdata, uint8_t *buf_ptr, uint32_t sz)
{

	if(sz <= 0)
		return -1;
	shared_ptr<TMemoryBuffer> membuffer(new TMemoryBuffer());
	shared_ptr<TProtocol> protocol(new TBinaryProtocol(membuffer));

	membuffer->resetBuffer(buf_ptr, sz);
	stdata->read(protocol.get());
	return 0;
}


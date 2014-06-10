/*
 * easou_mark_test.cpp
 *
 *  Created on: 2012-8-27
 *      Author: sue
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/time.h>
#include <cstring>
#include <cstdio>

#include "CCharset.h" //编码转换
#include "log.h" //写日志
#include "easou_url.h"
#include "easou_html_dom.h"
#include "easou_ahtml_tree.h"
#include "easou_html_extractor.h"
#include "easou_extractor_mypos.h"
#include "easou_extractor_title.h"
#include "easou_extractor_content.h"
#include "easou_extractor_com.h"
#include "easou_mark_baseinfo.h"

#define TIMEDELTA_MS(time1,time0) ((time1.tv_sec-time0.tv_sec)*1000+(time1.tv_usec-time0.tv_usec)/1000)

using namespace EA_COMMON;

char *url = NULL;
char *csspage_conversion;

/***** 解析树 *****/
area_tree_t *atree;
html_tree_t *html_tree;
html_vtree_t *vtree;
vtree_in_t *vtree_in;
/***** 解析树 *****/

#define MAX_PAGE_LEN 1<<20

static int code_conversionwithlength(char *page, int page_len, char *buffer, int buffer_len)
{
	// detect  page charset
	char charset[64] =
	{ 0 };
	CCharset m_cCharset;
	int ret;
	if ((ret = m_cCharset.GetCharset(page, page_len, charset)) < 0)
	{
		return 0;
	}

	if ((ret = m_cCharset.TransCToGb18030(page, page_len, buffer, buffer_len, charset)) > 0)
	{
		return ret;
	}
	return 0;
}

int parse(char* page, int page_len, char* url)
{
	//解析页面前需要把页面源代码转换成GB18030的，这里使用的是公共转码库
	char page_conversion[MAX_PAGE_SIZE];
	memset(page_conversion, 0, MAX_PAGE_SIZE);
	page_len = code_conversionwithlength(page, page_len, page_conversion, MAX_PAGE_SIZE);
	if (page_len <= 0)
	{
		Warn("code conversion fail, url:%s", url);
		return -1;
	}

	//重置，解析下一个页面时需要重置
	if (atree)
		mark_tree_reset(atree);
	if (vtree)
		html_vtree_reset(vtree);
	if (vtree_in)
		vtree_in_reset(vtree_in);
	if (html_tree)
		html_tree_reset_no_destroy((struct html_tree_impl_t*) html_tree);

	/**********  start 解析页面 ****************/
	if (0 == html_tree_parse(vtree->hpTree, page, page_len))
	{
		printf("html_tree_parse fail\n");
		return -1;
	}

	if (VTREE_ERROR == html_vtree_parse_with_tree(vtree, vtree_in, url))
	{
		printf("html_vtree_parse_with_tree fail\n");
		return -1;
	}

	if (1 != area_partition(atree, vtree, url))
	{
		printf("area_partition fail\n");
		return -1;
	}

	if (!mark_area_tree(atree, url))
	{
		printf("mark_area_tree fail\n");
		return -1;
	}
	/**************end 完成页面解析 *******************/

	// TODO 树解析好后，就可以开始基于树进行各种提取了
	const area_list_t *alist = get_func_mark_result(atree, AREA_FUNC_NAV);
	if (alist)
	{
		for (area_list_node_t* node = alist->head; node; node = node->next)
		{
			printf("nav ara no:%d\n", node->area->no);
			if (node == alist->tail)
				break;
		}
	}

	return 0;
}

int main(int argc, char** argv)
{
	//初始化日志
	if (!Init_Log("log.conf"))
	{
		printf("log.conf not exist\n");
		return 0;
	}

	if (argc != 3)
	{
		printf("Usage:%s path url\n", argv[0]);
		exit(-1);
	}
	char* pagePath = argv[1];
	char* url = argv[2];

	struct timeval endtv1, endtv2;
	FILE *fpPage = NULL;
	char pageBuf[128000];
	pageBuf[0] = 0;
	int pagelen;
	int tot_time1;
	csspage_conversion = (char*) malloc(1 << 21);

	/******* 连接css服务器 ******/
	if (!init_css_server("./config.ini", "./log"))
	{
		printf("init css server fail\n");
		exit(-1);
	}

	/************** start 创建解析相关树，在开始创建一次即可，单线程使用  *****************/
	html_tree = html_tree_create(MAX_PAGE_SIZE);
	if (!html_tree)
	{
		printf("html_tree_create fail\n");
		goto FAIL;
	}
	vtree_in = vtree_in_create();
	if (!vtree_in)
	{
		printf("vtree_in_create fail\n");
		goto FAIL;
	}
	vtree = html_vtree_create_with_tree(html_tree);
	if (!vtree)
	{
		printf("html_vtree_create_with_tree fail\n");
		goto FAIL;
	}
	atree = area_tree_create(NULL);
	if (!atree)
	{
		printf("area_tree_create fail\n");
		goto FAIL;
	}
	/************** end 创建解析相关树，在开始创建一次即可，单线程使用  *****************/

	// 测试单个页面
	fpPage = fopen(pagePath, "r");
	if (fpPage == NULL)
	{
		printf("can't read file\n");
		exit(-1);
	}

	pagelen = fread(pageBuf, 1, 128000, fpPage);
	fclose(fpPage);
	fpPage = NULL;

	gettimeofday(&endtv1, NULL); //开始时间

	parse(pageBuf, pagelen, url);

	gettimeofday(&endtv2, NULL); //结束时间
	tot_time1 = TIMEDELTA_MS(endtv2, endtv1); //单位：毫秒

	printf("time=%dms\n", tot_time1);

	FAIL: if (csspage_conversion)
	{
		free(csspage_conversion);
		csspage_conversion = NULL;
	}

	/**********  start 释放解析空间，最后调用即可 *************/
	if (atree)
	{
		mark_tree_destory(atree);
		atree = NULL;
	}
	if (vtree)
	{
		html_vtree_del(vtree);
		vtree = NULL;
	}
	if (vtree_in)
	{
		vtree_in_destroy(vtree_in);
		vtree_in = NULL;
	}
	if (html_tree)
	{
		html_tree_del(html_tree);
		html_tree = NULL;
	}
	/**********  end 释放解析空间，最后调用即可 *************/

	return 0;
}

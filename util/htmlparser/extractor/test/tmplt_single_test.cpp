/**
 * @brief 单页面抽取demo - 模版抽取
 * @author sue
 * @date 2013-06-12
 */

//解析相关头文件
#include "easou_extractor_template.h" //模板抽取需要加入的头文件
#include "easou_html_extractor.h"
#include "easou_html_attr.h"
#include "easou_debug.h"

//编码转换
#include "pagetranslate.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define PAGE_SIZE (1<<20) //1M
int main(int argc, char** argv)
{
	timeinit();
	if (argc < 3)
	{
		printf("Usage: %s path url\n", argv[0]);
		exit(-1);
	}

	char *path = argv[1];
	char *url = argv[2];
	int url_len = strlen(url);

	char *page = (char*) malloc(PAGE_SIZE);
	assert(page != NULL);
	char* pageGBBuf = (char*) malloc(PAGE_SIZE * 2);
	assert(pageGBBuf);

	FILE *fp = fopen(path, "r");
	assert(fp != NULL);
	int page_len = fread(page, 1, PAGE_SIZE, fp);
	page[page_len] = 0;

	timestart();
	/**********************1. 初始化，并加载模版，全局开始初始化一次，templates_t可多线程使用******************/
	templates_t* dicts = create_templates("../conf");
//	templates_t* dicts = create_templates("extractor/conf"); //多线程使用templates_t

	if (dicts == NULL)
	{
		printf("create template dict error\n");
		exit(-1);
	}

	int try_count = 0;
	int load_ret = 0;
	while ((load_ret = load_templates(dicts)) != 0) //连接mysql初始化模板
	{
		if (++try_count >= 5) //当mysql负载高时，初始化会失败，所以需要多此尝试
		{
			printf("already try load templates 5 times, program will exist\n");
			exit(-1);
		}
		printf("load templates from mysql fail, try later...\n");
		sleep(1);
	}
	printf("template load success\n");
	/**********************1. 初始化，并加载模版，全局开始初始化一次，templates_t可多线程使用******************/
	timeend("extractor_news", "init");

	//输入的page需要转码成GB18030编码
	PageTranslator pt;
	int pageGBLen = pt.translate(page, page_len, pageGBBuf, PAGE_SIZE * 2);
	if (pageGBLen <= 0)
	{
		printf("page code conversion to utf fail\n");
		exit(-1);
	}
	pageGBBuf[pageGBLen] = 0;

	//解析DOM树，模板抽取的输入需要解析好的DOM树
	html_tree_t* tree = html_tree_create(MAX_PAGE_SIZE);
	html_tree_parse(tree, pageGBBuf, pageGBLen);

	/**********************2. 创建结果对象，模板抽取的输出，只能单线程使用******************/
	tmplt_result_t* result = create_tmplt_result();
	/**********************2. 创建结果对象，模板抽取的输出，只能单线程使用******************/

	timestart();
	/**********************3. 调用抽取方法******************/
	int extract_ret = try_template_extract(dicts, pageGBBuf, pageGBLen, url, url_len, tree, result);
	/**********************3. 调用抽取方法******************/
	timeend("extractor_news", "one_page");

	/**********************4. 使用模板抽取的结果******************/
	if (extract_ret != 0)
	{
		//不存在对应的模版
		printf("there is no template for this url\n");
	}
	else if (result->extract_result[TMPLT_EXTRACT_REALTITLE].str_len == 0)
	{
		//存在对应的模板，但抽取到的title长度为0，说明模板对该网页不适用
		printf("template extract realtitle fail for this url\n");
	}
	else if (result->extract_result[TMPLT_EXTRACT_ARTICLE].str_len == 0)
	{
		//存在对应的模板，但抽取到的article长度为0，说明模板对该网页不适用
		printf("template extract article fail for this url\n");
	}
	else
	{
		//模板抽取成功
		printf("extract success\n");
		//打印各项抽取结果，i值的定义参考enum tmplt_extract_type_enum(抽取类型定义)
		for (int i = 0; i < g_tmplt_extract_num; i++)
		{
			printf("%s %s\n", tmplt_extract_type_desp[i], result->extract_result[i].str ? result->extract_result[i].str : "NULL");
		}
		//打印发布时间
		printf("year:%d mon:%d day:%d hour:%d min:%d sec:%d\n", result->pubtime_tm.tm_year, result->pubtime_tm.tm_mon, result->pubtime_tm.tm_yday, result->pubtime_tm.tm_hour, result->pubtime_tm.tm_min, result->pubtime_tm.tm_sec);
		//打印文章图片信息

		//这样取到的是绝对路径，未加http://
		html_node_list_t* list = result->extract_result[TMPLT_EXTRACT_ARTICLE].selected_imgs;
		link_t img_links[300];
		int img_num = 300;
		html_tree_extract_link(list, url, img_links, img_num);
		for (int i = 0; i < img_num; i++)
		{
			printf("img_url:http://%s\n", img_links[i].url);
		}

//这样取的有可能是相对路径
//		while (list)
//		{
//			const char* src = get_attribute_value(&list->html_node->html_tag, "src");
//			if (src)
//				printf("%s\n", src ? src : "NULL");
//			list = list->next;
//		}
	}
	/**********************4. 使用模板抽取的结果******************/

	/**********************5. 释放模板抽取的资源******************/
	del_tmplt_result(result); //每个线程释放
	del_templates(dicts); //全局释放
	/**********************5. 释放模板抽取的资源******************/
	printf("free templates dict success\n");

	free(page);
	free(pageGBBuf);
	html_tree_del(tree);
	printtime(1); // 打印时间信息
	return 0;
}

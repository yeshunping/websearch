/*
   ============================================================================
Name        : testxpath.c
Author      : zsw
Version     :
Copyright   : Your copyright notice
Description : Hello World in C, Ansi-style
============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <string>
#include <assert.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>

#include "easou_xpath.h"
#include "easou_html_tree.h"
#include "easou_html_extractor.h"
#include "easou_html_attr.h"

#define MAX_PAGE_LEN 1<<20

bool parseByXpath(char* page, int page_len, char* url, xpathnode** xpathlist, int xpathlength) 
{
	//创建DOM树
	html_tree_t *html_tree = html_tree_create(MAX_PAGE_SIZE);
	//解析DOM树
	if (0 == html_tree_parse(html_tree, page, page_len, true)) 
	{
		printf("html_tree_parse fail\n");
		exit(-1);
	}

	int resultlen=5000;
	XPATH_RESULT_NODE* result = (XPATH_RESULT_NODE*)malloc(sizeof(XPATH_RESULT_NODE) * resultlen);

	xpathselect(&html_tree->root, xpathlist, xpathlength, result, resultlen);

	char content[1000*1000]={0};

	for(int i = 0; i < resultlen; i++)
	{
		if(result[i].type == 0)
		{
			html_node_extract_content((html_node_t*)(result[i].node), content, 1000000);
			printf("content:%s\n", content);
		}
	}

	free(result);

	//销毁解析树
FAIL: if (html_tree) 
	      html_tree_del(html_tree);

      return 1;
}

int main(int argc, char** argv) 
{
	if (argc < 4)
	{
		printf("Usage: %s file url xpath\n", argv[0]);
		exit(-1);
	}
	char* filepath = argv[1];
	char* url = argv[2];
	char* xpath= argv[3];
	printf("xpath expressition is:%s;\n",xpath);

	xpathnode* xpathlist[10] = {0};
	int xpathlength = 10;
	parserxpath(xpath, xpathlist, &xpathlength);
	printf("xpath:\n");
	printxpath(xpathlist, xpathlength);
	printf("\n");

	FILE *fpage = fopen(filepath, "r");
	if (fpage == NULL) 
	{
		printf("fopen fail, %s\n", filepath);
		exit(-1);
	} 
	char pagebuf[MAX_PAGE_LEN] = { 0 };
	int len = fread(pagebuf, 1, MAX_PAGE_LEN - 1, fpage);
	pagebuf[len] = 0;
	fclose(fpage);
	fpage = NULL;

	parseByXpath(pagebuf, len, url, xpathlist, xpathlength);

	freexpath(xpathlist);
	return 1;
}

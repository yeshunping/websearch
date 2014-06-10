#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DownLoad.h"
#include "easou_html_dtd.h"
#include "debuginfo.h"
#include "easou_debug.h"
#include "pagetranslate.h"
#include "log.h"
#include "mysql.h"

#include "easou_html_tree.h"
#include "easou_vhtml_tree.h"
#include "easou_debug.h"

#ifdef _DEBUG_PROFILER_
#include <gperftools/profiler.h>
#endif

using namespace EA_COMMON;

#ifdef DEBUG_INFO
char *debugbuf;
int debugbuflen;
#endif

struct classify_res_t
{
	char* pageGBBuf;
	int size;
	html_tree_t* tree;
	vtree_in_t* vtree_in;
	html_vtree_t* vtree;
};

bool createDVATrees(html_tree_t*& tree, vtree_in_t*& vtree_in, html_vtree_t*& vtree)
{
	tree = html_tree_create(MAX_PAGE_SIZE);
	if (tree == NULL)
	{
		printf("html_tree_create fail\n");
		return false;
	}

	vtree_in = vtree_in_create();
	if (vtree_in == NULL)
	{
		printf("vtree_in_create fail\n");
		return false;
	}

	vtree = html_vtree_create_with_tree(tree);
	if (vtree == NULL)
	{
		printf("html_vtree_create_with_tree fail\n");
		return false;
	}

#ifdef DEBUG_INFO
	g_debugbuf = (char*) malloc (1<<28);
	assert(g_debugbuf);
	g_debugbuflen = 1<<28;

	debugbuf = g_debugbuf;
	debugbuflen = 1<<28;
#endif
	return true;
}

void resetDVATrees(html_tree_t*& tree, vtree_in_t*& vtree_in, html_vtree_t*& vtree)
{
	if (vtree)
		html_vtree_reset(vtree);
	if (vtree_in)
		vtree_in_reset(vtree_in);
	if (tree)
		html_tree_reset_no_destroy((struct html_tree_impl_t*) tree);
}

bool parseDVATrees(html_tree_t*& tree, vtree_in_t*& vtree_in, html_vtree_t*& vtree, const char* url, int urlLen, char* page, int pageLen)
{
	timeinit();
	timestart();
	int ret = html_tree_parse(tree, page, pageLen);
	if (ret != 1)
	{
		printf("html_tree_parse fail, url:%s", url);
		timeend("html_tree_parse", "");
		return false;
	}
	timeend("html_tree_parse", "");

	timestart();
	if (VTREE_ERROR == html_vtree_parse_with_tree(vtree, vtree_in, url))
	{
		printf("html_vtree_parse_with_tree fail, url:%s", url);
		timeend("html_vtree_parse_with_tree", "");
		return false;
	}
	timeend("html_vtree_parse_with_tree", "");
	vhtml_struct_prof(vtree);

	dprintf("style_txt_num:%d used_css_num:%d\n", vtree_in->css_env->page_css.style_txt_num, vtree_in->css_env->css_pool.used_css_num);

#ifdef DEBUG_INFO
	char *buf = (char*) malloc (1<<22);
	print_vtree(vtree, buf, 1<<22);
	FILE *fp = fopen("vtree.html", "w");
	assert(fp);
	fwrite(buf, 1, strlen(buf), fp);
	fclose(fp);
#endif
	return true;
}

void destroyDVATrees(html_tree_t*& tree, vtree_in_t*& vtree_in, html_vtree_t*& vtree)
{
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
	if (tree)
	{
		html_tree_del(tree);
		tree = NULL;
	}
}

bool parse_one_page(const char* url, int urlLen, char* page, int pageLen, char* pageGBBuf, int size, html_tree_t*& tree, vtree_in_t*& vtree_in, html_vtree_t*& vtree)
{
	timeinit();

	PageTranslator pt;
	int pageGBLen = pt.translate(page, pageLen, pageGBBuf, size);
	pageGBBuf[pageGBLen] = 0;

	timestart();
	resetDVATrees(tree, vtree_in, vtree);
	if (!parseDVATrees(tree, vtree_in, vtree, url, urlLen, pageGBBuf, pageGBLen))
	{
		printf("parse tree fail, url:%s\n", url);
		timeend("parse", "");
		return false;
	}
	timeend("parse", "");

	return true;
}

bool process_one_page(const char* url, int urlLen, char* page, int pageLen, classify_res_t* res)
{
	bool flag = parse_one_page(url, urlLen, page, pageLen, res->pageGBBuf, res->size, res->tree, res->vtree_in, res->vtree);
	return flag;
}

#define PAGE_SIZE 1<<20
classify_res_t* create_classify_res(const char* dir)
{
	classify_res_t* res = new classify_res_t;
	if (!res)
		goto _FAIL;
	res->size = PAGE_SIZE;
	res->pageGBBuf = (char*) malloc(PAGE_SIZE);
	if (!res->pageGBBuf)
		goto _FAIL;
	if (!createDVATrees(res->tree, res->vtree_in, res->vtree))
	{
		printf("create trees fail\n");
		goto _FAIL;
	}
	return res;
_FAIL: if (!res)
	       return NULL;
       destroyDVATrees(res->tree, res->vtree_in, res->vtree);
       if (res->pageGBBuf)
	       free(res->pageGBBuf);
       delete res;
       return NULL;
}

void destroy_classify_res(classify_res_t* res)
{
	if (!res)
		return;
	destroyDVATrees(res->tree, res->vtree_in, res->vtree);
	if (res->pageGBBuf)
		free(res->pageGBBuf);
	delete res;
}

enum
{
	TEST_FROM_MYSQL, TEST_FROM_LOCAL
};

int main(int argc, char** argv)
{
	const char *path = NULL;
	const char *url = NULL;
	int test_type = 0;

	if (argc == 2)
	{
		url = argv[1];
		test_type = TEST_FROM_MYSQL;
	}
	else if (argc == 3)
	{
		path = argv[1];
		url = argv[2];
		test_type = TEST_FROM_LOCAL;
	}
	else
	{
		printf("Usage: %s path url\n", argv[0]);
		printf("Usage: %s url\n", argv[0]);
		exit(-1);
	}

	if (!Init_Log("log.conf"))
	{
		printf("log.conf not exist\n");
		exit(-1);
	}

	if (!init_css_server("./config.ini", "./log", 1000, 10))
	{
		printf("init css server fail\n");
		exit(-1);
	}

	int url_len = strlen(url);

	classify_res_t* res = create_classify_res("../conf");
	assert(res);

	char *page_buf = (char*) malloc(PAGE_SIZE);
	assert(page_buf);
	int page_len = 0;
	FILE *fp = NULL;

	if (test_type == TEST_FROM_LOCAL)
	{
		fp = fopen(argv[1], "r");
		assert(fp);
		page_len = fread(page_buf, 1, PAGE_SIZE, fp);
		page_buf[page_len] = 0;
		fclose(fp);
	}
	else if (test_type == TEST_FROM_MYSQL)
	{
		MYSQL* conn = mysql_init(0);
		assert(conn);

		if (!mysql_real_connect(conn, "10.18.45.83", "root", "shepard", "parser", 0, 0, 0))
		{
			printf("mysql_real_connect fail\n");
			exit(-1);
		}

		char query[3072];
		int ret = snprintf(query, 3072, "select page from testset where url='%s' limit 1", url);
		query[ret] = 0;
		if (mysql_query(conn, query))
		{
			printf("mysql query error\n");
			exit(-1);
		}

		MYSQL_RES* result = mysql_store_result(conn);
		if (result == NULL)
		{
			printf("mysql store result error\n");
			exit(-1);
		}

		MYSQL_ROW row = mysql_fetch_row(result);
		if (row == NULL || row[0] == NULL)
		{
			printf("row[0] is NULL\n");
			exit(-1);
		}
		unsigned long* lengths = mysql_fetch_lengths(result);

		if (lengths[0] >= PAGE_SIZE)
		{
			printf("page length %lu beyond limit\n", lengths[0]);
			exit(-1);
		}
		memcpy(page_buf, row[0], lengths[0]);
		page_buf[(int)lengths[0]];
		page_len = lengths[0];
		mysql_free_result(result);
		mysql_close(conn);
		mysql_library_end();
	}

#ifdef _DEBUG_PROFILER_
	ProfilerStart("pt.prof");
#endif

	process_one_page(url, url_len, page_buf, page_len, res);

#ifdef _DEBUG_PROFILER_
	ProfilerStop();
#endif

	printtime(1);
	print_counter(1);
	print_max();

	char *htmlbuf = (char*) malloc(1 << 23);
	assert(htmlbuf);

	free(page_buf);
	free(htmlbuf);
	destroy_classify_res(res);

	return 0;
}

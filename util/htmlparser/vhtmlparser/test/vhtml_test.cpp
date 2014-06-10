#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "easou_vhtml_tree.h"
#include "easou_html_extractor.h"
#include "DownLoad.h" //网页下载
#include "CCharset.h" //编码转换
#include "log.h" //写日志
#include "debuginfo.h"
#define MAX_PAGE_LEN 1<<20
#define TIMEDELTA_MS(time1,time0) ((time1.tv_sec-time0.tv_sec)*1000+(time1.tv_usec-time0.tv_usec)/1000)

html_tree_t *tree;
vtree_in_t *vtree_in;
html_vtree_t *vtree;

bool useOuterCss = false;
char *csspage_conversion;
char *debugInfoType = NULL;
char *page_path = NULL;
char *url = NULL;
char *p_debugBuf = NULL;

void destroy_all()
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

int create_all()
{
	tree = html_tree_create(MAX_PAGE_SIZE);
	if (tree == NULL)
	{
		printf("html_tree_create fail\n");
		destroy_all();
		return -1;
	}

	vtree_in = vtree_in_create();
	if (vtree_in == NULL)
	{
		printf("vtree_in_create fail\n");
		destroy_all();
		return -1;
	}

	vtree = html_vtree_create_with_tree(tree);
	if (vtree == NULL)
	{
		printf("html_vtree_create_with_tree fail\n");
		destroy_all();
		return -1;
	}
	return 1;
}

void print_vtree(FILE *file, html_vnode_t *vnode, int level)
{
	html_vnode_t *child;
	if (!vnode->isValid)
	{
		return;
	}
	html_node_t *node = vnode->hpNode;
	for (int i = 0; i < level; i++)
	{
		fprintf(file, "	");
	}
	if (node->html_tag.tag_name)
	{
		fprintf(file, "<%s", node->html_tag.tag_name);
		for (html_attribute_t* attr = node->html_tag.attribute; attr; attr = attr->next)
		{
			fprintf(file, " %s=%s", attr->name, attr->value);
		}
		fprintf(file, " tagcode=%d xpos=%d ypox=%d width=%d height=%d depth=%d, valid:%d", node->html_tag.tag_code, vnode->xpos,
				vnode->ypos, vnode->wx, vnode->hx, vnode->depth, vnode->isValid);
		if (node->html_tag.is_self_closed)
		{
			fprintf(file, "/>");
		}
		else
		{
			fprintf(file, ">");
		}
	}
	if (node->html_tag.text)
	{
		fprintf(file, "%s", node->html_tag.text);
	}
	if (vnode->firstChild)
	{
		fprintf(file, "\n");
	}
	for (child = vnode->firstChild; child != NULL; child = child->nextNode)
	{
		print_vtree(file, child, level + 1);
	}
	for (int i = 0; i < level; i++)
	{
		fprintf(file, "	");
	}
	if (node->html_tag.tag_name && !node->html_tag.is_self_closed)
	{
		fprintf(file, "</%s>", node->html_tag.tag_name);
	}
	fprintf(file, "\n");
}

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

int download_url(char *buf, int size, char *url, char*& page)
{
	DownLoad cDown;
	int page_len = cDown.download(url, strlen(url), buf, size, page);
	if (page_len <= 0)
	{
		return -1;
	}
	return page_len;
}

void get_outer_css(html_vtree_t* vtree, char* url, vtree_in_t* vtree_in)
{
	*csspage_conversion = 0;
	char* csspageBegin;
	char csspage[MAX_PAGE_LEN] = "";
	int csspageLen = 0;
	int csspos = 0;
	int linknum = 250;
	link_t links[250]; //保存链接信息
	//download and add outer css
	int reallinknum = html_tree_extract_csslink(vtree->hpTree, url, links, linknum);
	printf("get css linknum=%d\n", reallinknum);
	for (int i = 0; i < reallinknum; i++)
	{
		if (strlen(links[i].url) == 0)
		{
			break;
		}
		//download page from internet
		printf("begin download css, cssurl:%s\n", links[i].url);
		char httpurl[2096] = "";
		if (strstr(links[i].url, "://") == NULL)
		{
			sprintf(httpurl, "http://%s", links[i].url);
		}
		else
		{
			sprintf(httpurl, "%s", links[i].url);
		}

		csspageLen = download_url(csspage, MAX_PAGE_LEN, httpurl, csspageBegin);
		if (csspageLen < 0)
		{
			printf("download css fail, %s\n", url);
			continue;
		}
		*(csspageBegin + csspageLen) = 0;
		printf("download css url success\n");

		// code conversion
		/*
		 csspageLen = code_conversionwithlength(csspageBegin, csspageLen, csspage_conversion + csspos,
		 1<<21 - csspos);
		 if (csspageLen < 1)
		 {
		 printf("code conversion for css fail\n");
		 continue;
		 }
		 */
		memcpy(csspage_conversion + csspos, csspageBegin, csspageLen);
		*(csspage_conversion + csspos + csspageLen) = '\n';
		csspos = csspos + csspageLen + 1;
	}
	if ((vtree_in != NULL && csspos > 0))
	{
		add_out_style_text(&(vtree_in->css_env->page_css), csspage_conversion,NULL);
	}
}

void init_vars()
{
	useOuterCss = false;
	page_path = NULL;
	p_debugBuf = (char*) malloc(1 << 26);
	csspage_conversion = (char*) malloc(1 << 21);
}

void print_help_info()
{
	printf("--css, use outer css\n");
	printf("-p pagePath, assign page saved in local machine to test\n");
	printf("-u url, assign page url\n");
	printf("-debug debugType, print debug info(default 0, print none)\n");
	printf("--help or -h, print help information\n");
}

void assign_input_parameter(int argc, char** argv)
{
	for (int i = 0; i < argc; i++)
	{
		if (*argv[i] != '-')
		{
			continue;
		}
		if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0)
		{
			print_help_info();
			exit(0);
		}
		if (strcmp("--css", argv[i]) == 0)
		{
			useOuterCss = true;
		}
		if (strcmp("-p", argv[i]) == 0 && i < argc - 1)
		{
			page_path = argv[i + 1];
		}
		if (strcmp("-u", argv[i]) == 0 && i < argc - 1)
		{
			url = argv[i + 1];
		}
		if (strcmp("-debug", argv[i]) == 0 && i < argc - 1)
		{
			debugInfoType = argv[i + 1];
		}
	}
}

void set_debug_status(const char *debugString)
{
	if (debugString == NULL || (strcmp("nodebug", debugString) == 0))
	{
		g_EASOU_DEBUG = 0;
	}
	else if (strcmp("realtitle", debugString) == 0)
	{
		g_EASOU_DEBUG = MARK_REALTITLE;
	}
	if (g_EASOU_DEBUG > 0)
	{
		myprintf("\r\n ------------The debug info begin------------------------\r\n");
	}
}

int reset_all()
{
	if (vtree)
	{
		html_vtree_reset(vtree);
	}
	if (vtree_in)
	{
		vtree_in_reset(vtree_in);
	}
	if (tree)
	{
		html_tree_reset_no_destroy((struct html_tree_impl_t*) tree);
	}
	return 0;
}

int parse_only(char *page, int page_len, char *url)
{
	reset_all();
	int ret = html_tree_parse(tree, page, page_len);
	if (ret != 1)
	{
		Warn("html_tree_parse fail, url:%s", url);
		return -1;
	}
	if (useOuterCss)
	{
		get_outer_css(vtree, url, vtree_in);
	}

	if (VTREE_ERROR == html_vtree_parse_with_tree(vtree, vtree_in, url))
	{
		Warn("html_vtree_parse_with_tree fail, url:%s", url);
		return -1;
	}
	return 1;
}

static int handle_single_page_buf(char *page, int pageLen, char *resultBuf, int resultBufSize, char *pageurl)
{
	// code conversion
	char page_conversion[MAX_PAGE_LEN] = "";
	int page_len = code_conversionwithlength(page, pageLen, page_conversion, MAX_PAGE_LEN);
	if (page_len <= 0)
	{
		Warn("code conversion fail, url:%s", pageurl);
		return -1;
	}

	set_debug_status(debugInfoType);
	if (g_EASOU_DEBUG > 0)
	{
		memset(p_debugBuf, 0, 1 << 26);
		g_debugbuf = p_debugBuf;
		g_debugbuflen = 1 << 26;
	}

	//parse page
	memset(resultBuf, 0, resultBufSize);
	char urldefault[] = "http://wap.easou.com/";
	if (pageurl == NULL)
	{
		printf("set url to default http://wap.easou.com/");
		pageurl = urldefault;
	}

	int ret = parse_only(page_conversion, page_len, pageurl);
	if (ret != 1)
	{
		printf("parse fail\n");
		return -1;
	}

	FILE *fp = fopen("result.html", "w");
	print_vtree(fp, vtree->root, 0);
	fclose(fp);
	fp = NULL;

	if (g_EASOU_DEBUG > 0)
	{
		Info(p_debugBuf);
	}
	g_EASOU_DEBUG = 0;
	g_debugbuf = NULL;
	g_debugbuflen = 0;
	return 0;
}

static int handle_single_page(char *pagePath, char *url, char *resultBuf, int size)
{
	// test single page
	char page[MAX_PAGE_LEN] = "";
	int pageLen = 0;
	char *pageBegin;
	if (pagePath != NULL)
	{
		//read test page from disk
		FILE *fpage = fopen(pagePath, "r");
		if (fpage == NULL)
		{
			Warn("fopen fail, %s", pagePath);
			return -1;
		}
		pageLen = fread(page, 1, MAX_PAGE_LEN, fpage);
		fclose(fpage);
		fpage = NULL;
		pageBegin = page;
	}
	else
	{
		//download page from internet
		printf("begin download page, url:%s\n", url);
		pageLen = download_url(page, MAX_PAGE_LEN, url, pageBegin);
		if (pageLen < 0)
		{
			printf("download fail, %s\n", url);
			return -2;
		}
		printf("download page success\n");
	}

	handle_single_page_buf(pageBegin, pageLen, resultBuf, size, url);
	return 0;
}

int main(int argc, char** argv)
{
	if (!Init_Log("log.conf"))
	{
		printf("can't find log.conf in current path\n");
		exit(-1);
	}
	init_vars();
	assign_input_parameter(argc, argv);
	create_all();
	struct timeval endtv1, endtv2;
	gettimeofday(&endtv1, NULL); //开始时间

	if (page_path != NULL || url != NULL)
	{ //test single page
		char *resultBuf = (char*) malloc(1 << 21);
		handle_single_page(page_path, url, resultBuf, 1 << 21);
		free(resultBuf);
	}

	gettimeofday(&endtv2, NULL); //结束时间
	int tot_time1 = TIMEDELTA_MS(endtv2, endtv1); //单位：毫秒
	printf("time=%dms\n", tot_time1);

	if (p_debugBuf)
	{
		free(p_debugBuf);
		p_debugBuf = NULL;
	}
	if (csspage_conversion)
	{
		free(csspage_conversion);
		csspage_conversion = NULL;
	}
	destroy_all();
	return 0;
}

/*
 * easou_linkparser_test.cpp
 *
 *  Created on: 2012-9-1
 *      Author: sue
 */
#include "easou_html_tree.h" //DOM树
#include "easou_vhtml_tree.h"//可视树
#include "easou_ahtml_tree.h"//分块树
#include "easou_mark_parser.h"//标记树
#include "easou_link.h" //链接分类
#include "easou_html_extractor.h"//DOM树抽取/外部css
#include "../../test/area_visual_tool.h"
#include "DownLoad.h" //网页下载
#include "CCharset.h" //编码转换
#include "log.h" //写日志
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//add
#include "easou_link_common.h"

#define TIMEDELTA_MS(time1,time0) ((time1.tv_sec-time0.tv_sec)*1000+(time1.tv_usec-time0.tv_usec)/1000)

char *url = NULL;
char *page_path = NULL;

html_tree_t *tree;
vtree_in_t *vtree_in;
html_vtree_t *vtree;
area_tree_t *atree;
lt_res_t *res;

void init_vars()
{
	url = NULL;
	page_path = NULL;

	tree = NULL;
	vtree_in = NULL;
	vtree = NULL;
	atree = NULL;
}

void print_help_info()
{
	printf("-u url, assign url\n");
	printf("-p path, assign page path\n");
	printf("-h or --help, show help info\n");
}

void assign_input_parameter(int argc, char **argv)
{
	for (int i = 0; i < argc; i++)
	{
		if (*argv[i] != '-')
		{
			continue;
		}
		if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0)
		{
			print_help_info();
		}
		if (strcmp("-u", argv[i]) == 0 && i < argc - 1)
		{
			url = argv[i + 1];
		}
		if (strcmp("-p", argv[i]) == 0 && i < argc - 1)
		{
			page_path = argv[i + 1];
		}
	}
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

void destroy_all()
{
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
	if (tree)
	{
		html_tree_del(tree);
		tree = NULL;
	}
	if (res)
	{
		linktype_res_del(res);
		res = NULL;
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

	atree = area_tree_create(NULL);
	if (atree == NULL)
	{
		printf("area_tree_create fail\n");
		destroy_all();
		return -1;
	}

	res = linktype_res_create(VLINK_ALL);
	if (res == NULL)
	{
		printf("easou_res_create fail\n");
		destroy_all();
		return -1;
	}
	return 1;
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

int reset_all()
{
	if (res)
	{
		linktype_res_reset(res);
	}
	if (atree)
	{
		mark_tree_reset(atree);
	}
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

int parse(char *page, int page_len, char *url)
{
	reset_all();

	int ret = html_tree_parse(tree, page, page_len);
	if (ret != 1)
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
	return 1;
}

static void get_linktype_desp(vlink_t *vlink, char *buf)
{
	int link_func = vlink->linkFunc;
	if (link_func & VLINK_UNDEFINED)
	{ //未定义链接	0
		sprintf(buf, "未定义链接,");
	}
	if (link_func & VLINK_NAV)
	{ //导航链接	1<<0
		sprintf(buf, "导航链接,");
	}
	if (link_func & VLINK_FRIEND)
	{ //友情链接	1<<1
		sprintf(buf, "友情链接,");
	}
	if (link_func & VLINK_INVALID)
	{ //不可见的链接	1<<2
		sprintf(buf, "不可见的链接,");
	}
	if (link_func & VLINK_CONTROLED)
	{ //通过css控制视觉的链接	1<<3
		sprintf(buf, "通过css控制视觉的链接,");
	}
	if (link_func & VLINK_IMAGE)
	{ //图片链接	1<<4
		sprintf(buf, "图片链接,");
	}
	if (link_func & VLINK_FRIEND_EX)
	{ //友情自助链接	1<<5
		sprintf(buf, "友情自助链接,");
	}
	if (link_func & VLINK_SELFHELP)
	{ //自组链接	1<<6
		sprintf(buf, "自组链接,");
	}
	if (link_func & VLINK_BLOG_MAIN)
	{ //博客正文	1<<7
		sprintf(buf, "博客正文,");
	}
	if (link_func & VLINK_CSS)
	{ //css链接	1<<8
		sprintf(buf, "css链接,");
	}
	if (link_func & VLINK_QUOTATION)
	{ //引文		1<<9
		sprintf(buf, "引文,");
	}
	if (link_func & VLINK_CONT_INTERLUDE)
	{ //内容穿插	1<<10
		sprintf(buf, "内容穿插,");
	}
	if (link_func & VLINK_BBSRE)
	{ //bbs回复	1<<11
		sprintf(buf, "bbs回复,");
	}
	if (link_func & VLINK_BLOGRE)
	{ //blog回复	1<<12
		sprintf(buf, "blog回复,");
	}
	if (link_func & VLINK_IMG_SRC)
	{ //图片资源链接	1<<13
		sprintf(buf, "图片资源链接,");
	}
	if (link_func & VLINK_EMBED_SRC)
	{ //嵌入资源链接	1<<14
		sprintf(buf, "嵌入资源链接,");
	}
	if (link_func & VLINK_FROM_CONT)
	{ //!!!!!!!!!!!ec中定义和实现的链接类型，防止冲突，加入	1<<15
		sprintf(buf, "VLINK_FROM_CONT,");
	}
	if (link_func & VLINK_BBSCONT)
	{ //	1<<16
		sprintf(buf, "VLINK_BBSCONT,");
	}
	if (link_func & VLINK_BBSSIG)
	{ //	1<<17
		sprintf(buf, "VLINK_BBSSIG,");
	}
	if (link_func & VLINK_COPYRIGHT)
	{ //	1<<18
		sprintf(buf, "版权链接,");
	}
	if (link_func & VLINK_NOFOLLOW)
	{ //nofollow	1<<19
		sprintf(buf, "nofollow链接,");
	}
	if (link_func & VLINK_MYPOS)
	{ //mypos	1<<20
		sprintf(buf, "mypos链接,");
	}
	if (link_func & VLINK_HIDDEN)
	{ //	1<<21
		sprintf(buf, "隐藏链接,");
	}
	if (link_func & VLINK_TEXT_LINK)
	{ //穿插的链接，伪装成文本形式	1<<22
		sprintf(buf, "穿插的链接，伪装成文本形式,");
	}
	if (link_func & VLINK_FRIEND_SPAM)
	{ //预留给超链系统	1<<24
		sprintf(buf, "VLINK_FRIEND_SPAM,");
	}
	if (link_func & VLINK_IN_LINK)
	{ //预留给超链系统	1<<25
		sprintf(buf, "VLINK_IN_LINK,");
	}
	if (link_func & VLINK_TEXT)
	{ // 文字链接
		sprintf(buf, " 文字链接,");
	}
	if (link_func & VLINK_NEWS_ANCHOR)
	{ //新闻页正文中介绍的网站链接(预留给EC)
		sprintf(buf, "新闻页正文中介绍的网站链接,");
	}
	//add
	if (link_func & VLINK_IFRAME)
	{ //IFRAME链接	1<<28
		sprintf(buf, "IFRAME链接,");
	}

	int len = strlen(buf);
	if (*(buf + len - 1) == ',')
	{
		*(buf + len - 1) = '\0';
	}
}

void print_imgs_info(img_info_t *imgs, int img_num)
{
	for (int i = 0; i < img_num; i++)
	{
		img_info_t *img = imgs + i;
		if (img->owner != NULL)
		{
			// printf("%d %d anchor:%s url:%s img_url:%s trust:%d img_hx:%d img_wx:%d\n", i, img->type, img->owner->text, 
			//	img->owner->url, img->img_url, img->trust, img->img_hx, img->img_wx);
		}
		else
		{
			printf("%d %d url:%s img_url:%s trust:%d img_hx:%d img_wx:%d\n", i, img->type, url, img->img_url, img->trust, img->img_hx, img->img_wx);
		}
	}
}

void print_linktype_info(vlink_t *vlinks, int link_num)
{
	for (int i = 0; i < link_num; i++)
	{
		vlink_t *vlink = vlinks + i;
		char desp[256] = "";
		get_linktype_desp(vlink, desp);
		printf("no:%d, linktype:%s, anchor:%s, url:%s\n", i, desp, vlink->text, vlink->url);
	}
}

int consume_linktype()
{
	vlink_t vlinks[300];
	int link_num = html_vtree_extract_vlink(vtree, atree, url, vlinks, 300);
	int pagetype = 0; //TODO pagetype
	html_vtree_mark_vlink(vtree, atree, url, vlinks, link_num, pagetype, VLINK_ALL, res);
	//print_linktype_info(vlinks, link_num);

	img_info_t imgs[300];
	int img_num = extract_img_infos(url, strlen(url), atree, vlinks, link_num, imgs, 300);
	print_imgs_info(imgs, img_num);
	return 1;
}

//-u url or -u url -p path(loaded)
int main(int argc, char **argv)
{
	if (!Init_Log("log.conf"))
	{
		printf("log.conf not exist\n");
		return 0;
	}

	init_vars();
	assign_input_parameter(argc, argv);

	if (url == NULL)
	{
		print_help_info();
		exit(-1);
	}

	if (init_css_server("./config.ini", "./log", 1000, 10))
		printf("init css server success\n");
	else
		printf("init css server fail\n");

	char *page;
	int page_len = 0;
	char page_buf[MAX_PAGE_SIZE] = "";
	char page_buf_converted[MAX_PAGE_SIZE] = "";
	int page_len_converted = 0;
	if (page_path == NULL)
	{
		page_len = download_url(page_buf, MAX_PAGE_SIZE, url, page);
		if(page_len == -1)
		{
			printf("download fail\n");
			exit(-1);
		}
	}
	else
	{
		FILE *fp = fopen(page_path, "r");
		if (fp == NULL)
		{
			printf("%s not exist\n", page_path);
			exit(-1);
		}
		page_len = fread(page_buf, 1, MAX_PAGE_SIZE, fp);
		fclose(fp);
		fp = NULL;
		page = page_buf;
	}
	page_len_converted = code_conversionwithlength(page, page_len, page_buf_converted, MAX_PAGE_SIZE);
	if (page_len_converted <= 0)
	{
		printf("code conversion fail\n");
		exit(-1);
	}

	int ret = create_all();
	if (ret != 1)
	{
		printf("create all fail\n");
		exit(-1);
	}

	struct timeval endtv1, endtv2;
	gettimeofday(&endtv1, NULL); //开始时间

	ret = parse(page_buf_converted, page_len_converted, url);
	
	char* html_buf = (char*)malloc(1<<22);
	assert(html_buf);
	int html_len = printFuncHtml(atree, html_buf, 1<<22, AREA_FUNC_ARTICLE_CONTENT);
	FILE *fp = fopen("ac.html", "w");
	fwrite(html_buf, 1, strlen(html_buf), fp);
	fclose(fp);

	html_len = printAreaTreeHtml(atree, html_buf, 1<<22, 0, 0);
	fp = fopen("atree.html", "w");
	fwrite(html_buf, 1, strlen(html_buf), fp);
	fclose(fp);

	free(html_buf);

	if (ret == 1)
	{
		printf("parse success\n");
		consume_linktype();
	}
	else
	{
		printf("parse fail\n");
	}

	gettimeofday(&endtv2, NULL); //结束时间
	int tot_time1 = TIMEDELTA_MS(endtv2, endtv1);//单位：毫秒
	printf("time=%dms\n", tot_time1);

	destroy_all();
	return 0;
}


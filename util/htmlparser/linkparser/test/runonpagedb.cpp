#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "pagetranslate.h"
#include "easou_string.h"
#include "easou_debug.h"
#include "easou_link.h"
#include "log.h"

#include "easou_link_common.h"

#include "easou_html_tree.h"
#include "easou_vhtml_tree.h"
#include "easou_ahtml_tree.h"
#include "easou_mark_parser.h"

#include "mysql.h"

//includes for edb 
#include "RandomRead.h"
#include "ScannerClient.h"
#include "Scanner.h"
#include "readlocal.h"
#include "ClientConfig.h"
#include "PageDBValues.h"
#include "GlobalFun.h"
#include "dump.h"
#include "Queue.h"
#include "Log.h"

using namespace std;
using namespace EA_COMMON;
using namespace nsPageDB;

#define Q_BUF_SIZE (1<<21)  //2M
#define PAGE_SIZE (1<<20) //1M
#define URL_SIZE 2048

//CLog g_Log;
CClientConfig gConfig;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct classify_res_t
{
	char* pageGBBuf;
	int size;
	html_tree_t* tree;
	vtree_in_t* vtree_in;
	html_vtree_t* vtree;
	area_tree_t* atree;
	lt_res_t *lt_res;
};

static void get_linktype_desp(vlink_t *vlink, char *buf)
{
        int link_func = vlink->linkFunc;
        if (link_func & VLINK_UNDEFINED)
        { //未定义链接  0
                sprintf(buf, "未定义链接,");
        }
        if (link_func & VLINK_NAV)
        { //导航链接    1<<0
                sprintf(buf, "导航链接,");
        }
        if (link_func & VLINK_FRIEND)
        { //友情链接    1<<1
                sprintf(buf, "友情链接,");
        }
        if (link_func & VLINK_INVALID)
        { //不可见的链接        1<<2
                sprintf(buf, "不可见的链接,");
        }
        if (link_func & VLINK_CONTROLED)
        { //通过css控制视觉的链接       1<<3
                sprintf(buf, "通过css控制视觉的链接,");
        }
        if (link_func & VLINK_IMAGE)
        { //图片链接    1<<4
                sprintf(buf, "图片链接,");
        }
        if (link_func & VLINK_FRIEND_EX)
        { //友情自助链接        1<<5
                sprintf(buf, "友情自助链接,");
        }
        if (link_func & VLINK_SELFHELP)
        { //自组链接    1<<6
                sprintf(buf, "自组链接,");
        }
        if (link_func & VLINK_BLOG_MAIN)
        { //博客正文    1<<7
                sprintf(buf, "博客正文,");
        }
	if (link_func & VLINK_CSS)
        { //css链接     1<<8
                sprintf(buf, "css链接,");
        }
        if (link_func & VLINK_QUOTATION)
        { //引文                1<<9
                sprintf(buf, "引文,");
        }
        if (link_func & VLINK_CONT_INTERLUDE)
        { //内容穿插    1<<10
                sprintf(buf, "内容穿插,");
        }
        if (link_func & VLINK_BBSRE)
        { //bbs回复     1<<11
                sprintf(buf, "bbs回复,");
        }
        if (link_func & VLINK_BLOGRE)
        { //blog回复    1<<12
                sprintf(buf, "blog回复,");
        }
        if (link_func & VLINK_IMG_SRC)
        { //图片资源链接        1<<13
                sprintf(buf, "图片资源链接,");
        }
        if (link_func & VLINK_EMBED_SRC)
        { //嵌入资源链接        1<<14
                sprintf(buf, "嵌入资源链接,");
        }
        if (link_func & VLINK_FROM_CONT)
        { //!!!!!!!!!!!ec中定义和实现的链接类型，防止冲突，加入 1<<15
                sprintf(buf, "VLINK_FROM_CONT,");
        }
        if (link_func & VLINK_BBSCONT)
        { //    1<<16
                sprintf(buf, "VLINK_BBSCONT,");
        }
        if (link_func & VLINK_BBSSIG)
        { //    1<<17
                sprintf(buf, "VLINK_BBSSIG,");
        }
        if (link_func & VLINK_COPYRIGHT)
        { //    1<<18
                sprintf(buf, "版权链接,");
        }
        if (link_func & VLINK_NOFOLLOW)
        { //nofollow    1<<19
                sprintf(buf, "nofollow链接,");
        }
        if (link_func & VLINK_MYPOS)
        { //mypos       1<<20
                sprintf(buf, "mypos链接,");
        }
	if (link_func & VLINK_HIDDEN)
        { //    1<<21
                sprintf(buf, "隐藏链接,");
        }
        if (link_func & VLINK_TEXT_LINK)
        { //穿插的链接，伪装成文本形式  1<<22
                sprintf(buf, "穿插的链接，伪装成文本形式,");
        }
        if (link_func & VLINK_FRIEND_SPAM)
        { //预留给超链系统      1<<24
                sprintf(buf, "VLINK_FRIEND_SPAM,");
        }
        if (link_func & VLINK_IN_LINK)
        { //预留给超链系统      1<<25
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
	{ //IFRAME链接  1<<28
		sprintf(buf, "IFRAME链接,");
	}

	int len = strlen(buf);
	if (*(buf + len - 1) == ',')
	{
		*(buf + len - 1) = '\0';
	}
}


static void print_linktype_info(vlink_t *vlinks, int link_num)
{
        for (int i = 0; i < link_num; i++)
        {
                vlink_t *vlink = vlinks + i;
                char desp[256] = "";
                get_linktype_desp(vlink, desp);
                printf("no:%d, linktype:%s, anchor:%s, url:%s\n", i, desp, vlink->text, vlink->url);
        }
}

static void print_imgs_info(img_info_t *imgs, int img_num, char *url)
{
	for (int i = 0; i < img_num; i++)
	{
		img_info_t *img = imgs + i;
		//printf("%d anchor:%s url:http://%s img_url:%s trust:%d img_hx:%d img_wx:%d\n", i, img->owner->text, img->owner->url, img->img_url, img->trust, img->img_hx, img->img_wx);
		//modify
		if (img->owner != NULL)
                        printf("%d %d anchor:%s url:%s img_url:%s trust:%d img_hx:%d img_wx:%d\n", i, img->type, img->owner->text, img->owner->url, img->img_url, img->trust, img->img_hx, img->img_wx);
                else
                        printf("%d %d url:%s img_url:%s trust:%d img_hx:%d img_wx:%d\n", i, img->type, url, img->img_url, img->trust, img->img_hx, img->img_wx);
	}
}

int consume_linktype(classify_res_t* res, char* url)
{
	vlink_t vlinks[300];
	int link_num = html_vtree_extract_vlink(res->vtree, res->atree, url, vlinks, 300);
	int pagetype = 0; //TODO pagetype
	html_vtree_mark_vlink(res->vtree, res->atree, url, vlinks, link_num, pagetype, VLINK_ALL, res->lt_res);
	print_linktype_info(vlinks, link_num);

	img_info_t imgs[300];
	int img_num = extract_img_infos(url, strlen(url), res->atree, vlinks, link_num, imgs, 300);
	print_imgs_info(imgs, img_num, url);
	return 1;
}

bool createDVATrees(html_tree_t*& tree, vtree_in_t*& vtree_in, html_vtree_t*& vtree, area_tree_t*& atree)
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

	atree = area_tree_create(NULL);
	if (atree == NULL)
	{
		printf("area_tree_create fail\n");
		return false;
	}
	return true;
}

void resetDVATrees(html_tree_t*& tree, vtree_in_t*& vtree_in, html_vtree_t*& vtree, area_tree_t*& atree)
{
	if (atree)
		mark_tree_reset(atree);
	if (vtree)
		html_vtree_reset(vtree);
	if (vtree_in)
		vtree_in_reset(vtree_in);
	if (tree)
		html_tree_reset_no_destroy((struct html_tree_impl_t*) tree);
}

bool parseDVATrees(html_tree_t*& tree, vtree_in_t*& vtree_in, html_vtree_t*& vtree, area_tree_t*& atree, char* url, int urlLen, char* page, int pageLen)
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

        timestart();
	if (1 != area_partition(atree, vtree, url))
	{
		printf("area_partition fail, url:%s\n", url);
		timeend("area_partition", "");
		return false;
	}
	timeend("area_partition", "");

        timestart();
	if (!mark_area_tree(atree, url))
	{
		printf("mark_area_tree fail, url:%s\n", url);
		timeend("mark_area_tree", "");
		return false;
	}
	timeend("mark_area_tree", "");

	return true;
}

void destroyDVATrees(html_tree_t*& tree, vtree_in_t*& vtree_in, html_vtree_t*& vtree, area_tree_t*& atree)
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
}

bool getPageType(char* url, int urlLen, char* page, int pageLen, char* pageGBBuf, int size, html_tree_t*& tree, vtree_in_t*& vtree_in, html_vtree_t*& vtree, area_tree_t*& atree, lt_res_t*& lt_res)
{
	PageTranslator pt;
	int pageGBLen = pt.translate(page, pageLen, pageGBBuf, PAGE_SIZE);
	pageGBBuf[pageGBLen] = 0;

        timeinit();
        timestart();
	resetDVATrees(tree, vtree_in, vtree, atree);
	linktype_res_reset(lt_res);
	if (!parseDVATrees(tree, vtree_in, vtree, atree, url, urlLen, pageGBBuf, pageGBLen))
	{
		printf("parse tree fail, url:%s\n", url);
	        timeend("parse", "");
		return false;
	}
        timeend("parse", "");
	return true;
}

struct bucket_info_t
{
	int begin;
	int step;
	int end;
};

struct page_info_t
{
	char* pageBuf;
	int pageLen;
	int pageSize;
	char* url;
	int urlLen;
	int urlSize;
	int label;
};

typedef struct _page_info_listnode_t
{
	page_info_t* page_info;
	_page_info_listnode_t* next;
} page_info_listnode_t;

struct page_info_list_t
{
	page_info_listnode_t* head;
	page_info_listnode_t* tail;
	int num;
};

struct pool_t
{
	page_info_list_t *supply_list;
	page_info_list_t *produce_list;
	page_info_list_t *used_list;
	int node_size;
};

struct thread_param_t
{
	bucket_info_t binfo;
	pool_t* pool;
	int allnum;
};

page_info_t* crate_page_info(int url_size, int page_size)
{
	page_info_t* page_info = new page_info_t;
	if (!page_info)
		return NULL;
	page_info->pageBuf = (char*) malloc(page_size);
	page_info->url = (char*) malloc(url_size);
	if (!page_info->pageBuf || !page_info->url)
		goto _FAIL;
	page_info->urlSize = url_size;
	page_info->pageSize = page_size;
	return page_info;
	_FAIL: if (page_info->pageBuf)
		free(page_info->pageBuf);
	if (page_info->url)
		free(page_info->url);
	delete page_info;
	return NULL;
}

page_info_listnode_t* create_pageinfo_listnode(int url_size, int page_size)
{
	page_info_listnode_t* node = new page_info_listnode_t;
	if (!node)
		return NULL;
	page_info_t* page_info = crate_page_info(url_size, page_size);
	if (!page_info)
		goto _FAIL;
	node->page_info = page_info;
	node->next = NULL;
	return node;
	_FAIL: delete node;
	return NULL;
}

void destroy_page_info(page_info_t* page_info)
{
	if (!page_info)
		return;
	if (page_info->pageBuf)
		free(page_info->pageBuf);
	if (page_info->url)
		free(page_info->url);
	delete page_info;
}

void destroy_pageinfo_listnode(page_info_listnode_t* node)
{
	if (!node)
		return;
	destroy_page_info(node->page_info);
	delete node;
}

void insert_tail(page_info_list_t* list, page_info_listnode_t* node)
{
	node->next = NULL;
	if (list->num == 0)
	{
		list->head = node;
		list->tail = node;
	}
	else
	{
		list->tail->next = node;
		list->tail = node;
	}
	list->num++;
	return;
}

void destroy_pageinfo_list(page_info_list_t* list)
{
	if (!list)
		return;
	page_info_listnode_t* node = list->head;
	while (node)
	{
		destroy_pageinfo_listnode(node);
		node = node->next;
	}
	delete list;
}

void destroy_pool(pool_t* pool)
{
	if (!pool)
		return;
	destroy_pageinfo_list(pool->supply_list);
	destroy_pageinfo_list(pool->produce_list);
	destroy_pageinfo_list(pool->used_list);
}

pool_t* create_pool(int node_size)
{
	pool_t* pool = new pool_t;
	if (!pool)
		return NULL;
	pool->supply_list = new page_info_list_t;
	pool->produce_list = new page_info_list_t;
	pool->used_list = new page_info_list_t;
	if (!pool->supply_list || !pool->produce_list || !pool->used_list)
		goto _FAIL;
	pool->supply_list->num = 0;
	pool->produce_list->num = 0;
	pool->used_list->num = 0;
	for (int i = 0; i < node_size; i++)
	{
		page_info_listnode_t* node = create_pageinfo_listnode(URL_SIZE, PAGE_SIZE);
		if (!node)
			goto _FAIL;
		insert_tail(pool->used_list, node);
	}
	pool->node_size = node_size;
	return pool;
	_FAIL: destroy_pool(pool);
	return NULL;
}

page_info_listnode_t* remove_head(page_info_list_t* list)
{
	if (list->num == 0)
		return NULL;
	page_info_listnode_t* node = list->head;
	list->head = node->next;
	list->num--;
	if (list->num == 0)
		list->tail = NULL;
	node->next = NULL;
	return node;
}

bool produce_to_pool(pool_t* pool, const char* url, int urlLen, const char* page, int pageLen)
{
	if (urlLen >= 2048 || pageLen >= PAGE_SIZE)
	{
		printf("url or page to long\n");
		return false;
	}
	page_info_listnode_t* node = remove_head(pool->used_list);
	if (!node)
		return false;
	memcpy(node->page_info->url, url, urlLen);
	node->page_info->url[urlLen] = 0;
	node->page_info->urlLen = urlLen;
	memcpy(node->page_info->pageBuf, page, pageLen);
	node->page_info->pageBuf[pageLen] = 0;
	node->page_info->pageLen = pageLen;

	insert_tail(pool->supply_list, node);

	return true;
}

int process_remote_bucket(int bucketno, pool_t* pool)
{
	int pid = getpid();
	vector<UINT8> familyno;
	familyno.push_back(0); //page
	familyno.push_back(4); //feature

	CScanner reador;
	int ret = reador.open(bucketno, familyno, &gConfig, NULL, 0);
	if (CPageDBValues::DB_OPR_SCAN_COMPLETE == ret)
	{
		printf("complete!\n");
		return 0;
	}
	if (ret)
	{
		printf("errorno:%d\n", ret);
		return -1;
	}
	CTblRowReador *rowreador;
	while (true)
	{
		ret = reador.next(rowreador);
		if (CPageDBValues::DB_OPR_SCAN_COMPLETE == ret)
			break;
		if (ret)
		{
			printf("errorno:%d\n", ret);
			return -1;
		}

		string line;
		const char* key;
		UINT16 keylen;
		rowreador->getKey(key, keylen);
		line = string(key, keylen);
		//if (strncmp(key, "css", 3) == 0)
		if (strncmp(key, "http://", 7) != 0)
			continue;

		char* v;
		int vlen;

		/*
		rowreador->get(4, 46, v, vlen);
		int idx_level = *(UINT8*) v;
		if (idx_level != 4 && idx_level != 5)
			//&& idx_level != 103 && idx_level != 104 && idx_level != 105)
			continue;
		*/

		//dead link skip
		rowreador->get(4, 2, v, vlen);
		int dead_link = *(UINT8*) v;
		if (dead_link == 1)
			continue;

		/*
		rowreador->get(4, 12, v, vlen);
		int http_type = *(UINT8*) v;
		if (http_type == 1 || http_type == 2)
		{
			//printf("%s is wap page\n", line.c_str());
			continue;
		}
		*/

		rowreador->get(0, 1, v, vlen);
		string page;
		page = string(v, vlen);

		pthread_mutex_lock(&mutex);
		while (pool->used_list->num <= pool->node_size / 10)
			pthread_cond_wait(&cond, &mutex);
		if (!produce_to_pool(pool, line.c_str(), line.length(), page.c_str(), page.length()))
			printf("produce to pool fail, %s\n", line.c_str());
		pthread_mutex_unlock(&mutex);
	}
	reador.close();
	return 1;
}

void* read_thread(void* p)
{
	thread_param_t* param = (thread_param_t*) p;
	for (int i = param->binfo.begin; i < param->binfo.end; i += param->binfo.step)
		process_remote_bucket(i, param->pool);
	return NULL;
}

classify_res_t* create_classify_res(const char* dir)
{
	classify_res_t* res = new classify_res_t;
	if (!res)
		goto _FAIL;
	res->size = PAGE_SIZE;
	res->pageGBBuf = (char*) malloc(PAGE_SIZE);
	if (!res->pageGBBuf)
		goto _FAIL;
	if (!createDVATrees(res->tree, res->vtree_in, res->vtree, res->atree))
	{
		printf("create trees fail\n");
		goto _FAIL;
	}
	if ((res->lt_res = linktype_res_create(VLINK_ALL)) == NULL)
	{
		printf("linktype_res_create fail\n");
		goto _FAIL;
	}
	return res;

	_FAIL: if (!res)
		return NULL;
	destroyDVATrees(res->tree, res->vtree_in, res->vtree, res->atree);
	if (res->pageGBBuf)
		free(res->pageGBBuf);
	delete res;
	return NULL;
}

bool get_pagetype(char* url, int urlLen, char* page, int pageLen, classify_res_t* res)
{
	bool flag = getPageType(url, urlLen, page, pageLen, res->pageGBBuf, res->size, res->tree, res->vtree_in, res->vtree, res->atree, res->lt_res);
	//add
	if(!flag)
		return false;

	consume_linktype(res, url);
	return flag;
}

void* consume_thread(void* p)
{
	int pid = getpid();
	thread_param_t* param = (thread_param_t*) p;

	classify_res_t* res = create_classify_res("../conf");
	if (res == NULL)
	{
		printf("[%d]create classify res fail\n", pid);
		return NULL;
	}

	int count = 0;
	while (true)
	{
		page_info_listnode_t* node = NULL;
		pthread_mutex_lock(&mutex);
		if (param->pool->supply_list->num <= param->pool->node_size / 10)
			pthread_cond_signal(&cond);
		node = remove_head(param->pool->supply_list);
		pthread_mutex_unlock(&mutex);
		if (!node)
		{
			if (count >= 10)
			{
				printf("[%d] waited 10 seconds, finish!\n", pid);
				break;
			}
			sleep(1);
			count++;
			continue;
		}

		count = 0;
		char* url = node->page_info->url;
		int urlLen = node->page_info->urlLen;
		char* page = node->page_info->pageBuf;
		int pageLen = node->page_info->pageLen;
		if (!get_pagetype(url, urlLen, page, pageLen, res))
			goto _CONTINUE;

		pthread_mutex_lock(&mutex);
		param->allnum++;
		if (param->allnum % 10000 == 0)
		{
			printf("processed %d urls\n", param->allnum);
			printtime(param->allnum);
		}
		pthread_mutex_unlock(&mutex);

		_CONTINUE: pthread_mutex_lock(&mutex);
		insert_tail(param->pool->used_list, node);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void destroy_classify_res(classify_res_t* res)
{
	if (!res)
		return;
	destroyDVATrees(res->tree, res->vtree_in, res->vtree, res->atree);
	if (res->pageGBBuf)
		free(res->pageGBBuf);
	if (res->lt_res)
	{
		linktype_res_del(res->lt_res);
		res->lt_res = NULL;
	}
	delete res;
}

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		printf("Usage:%s startBucket bucketNum threadNum\n", argv[0]);
		return -1;
	}
	int startBucket = atoi(argv[1]);
	int bucketNum = atoi(argv[2]);
	int threadCount = atoi(argv[3]);

	pool_t* pool = create_pool(300);
	assert(pool);

	thread_param_t tparam;
	memset(&tparam, 0, sizeof(thread_param_t));
	tparam.binfo.begin = startBucket;
	tparam.binfo.step = 1;
	tparam.binfo.end = startBucket + bucketNum;
	tparam.pool = pool;

	if (!Init_Log("log.conf"))
	{
		printf("log.conf not exist\n");
		exit(-1);
	}
	if (!init_css_server("./config.ini", "./log"))
	//if (!init_css_server("./config.ini", "./log", 1000, 10))
	{
		printf("init css server fail\n");
		exit(-1);
	}
	if (gConfig.init("./config.ini", "pagedb") != 0)
	{
		printf("config init error\n");
		exit(-1);
	}

	pthread_t r_id;
	int error = pthread_create(&r_id, NULL, read_thread, &tparam);
	if (error != 0)
	{
		printf("create read thread fail\n");
		exit(-1);
	}

	pthread_t* p_ids = new pthread_t[threadCount];
	for (int i = 0; i < threadCount; i++)
	{
		error = pthread_create(&p_ids[i], NULL, consume_thread, &tparam);
		if (error != 0)
		{
			printf("create consume thread fail\n");
			exit(-1);
		}
	}

	//waitting..
	pthread_join(r_id, NULL);
	for (int i = 0; i < threadCount; i++)
		pthread_join(p_ids[i], NULL);

	_FAIL: gConfig.destroy();
	if (p_ids)
		delete[] p_ids;
	return 0;
}



//char *video_url[] = { "http://v.qq.com/", "http://tv.sohu.com", "http://www.letv.com/", "http://www.tudou.com/", "http://v.youku.com/" };


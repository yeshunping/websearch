/**
 * stdata_single_test.cpp
 * 	结构化数据抽取的单网页demo
 *
 *  Created on: 2013-07-22
 *      Author: round
 */

//结构化抽取需要用到的头文件
#include "easou_html_tree.h" //DOM树
#include "easou_vhtml_tree.h"//可视树
#include "easou_ahtml_tree.h"//分块树
#include "easou_mark_parser.h"
#include "easou_extractor_stdata.h" //结构化数据抽取接口

//测试程序常用工具方法
#include "../../test/tool_global.h" 

//一些公共库
#include "CCharset.h" //编码转换
#include "log.h" //写日志

//thrift接口需要用到的头文件
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//分类接口
#include "PageClassify.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using boost::shared_ptr;
using namespace std;
using namespace nsPanPC;


html_tree_t *tree;
vtree_in_t *vtree_in;
html_vtree_t *vtree;
area_tree_t *atree;

void init_vars()
{
	tree = NULL;
	vtree_in = NULL;
	vtree = NULL;
	atree = NULL;
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
	return 1;
}

int reset_all()
{
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

void print_stdata_info(StructureData *stdata)
{
	//print debug info
	printf("StructureData info:\n\tversion:%d type=%d\n", stdata->version, stdata->type);
	for(vector<StructureKV>::iterator j=stdata->all.begin(); j!=stdata->all.end(); j++)
	{
		printf("\tkey=%d\tvalue=%s\tlength:%d\n", j->key, j->value.c_str(), j->value.length());
		assert(j->value.length() == strlen(j->value.c_str()));
	}
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Usage: %s url [path]\n", argv[0]);
		exit(-1);
	}
	char* url = argv[1];
	int url_len = strlen(url);
	const char* path = NULL;
	if (argc == 3)
		path = argv[2];

	char page_buf[MAX_PAGE_SIZE];
	char page_buf_converted[MAX_PAGE_SIZE];
	int page_len = 0;
	if (path)
	{
		FILE *fp = fopen(path, "r");
		assert(fp);
		page_len = fread(page_buf, 1, MAX_PAGE_SIZE, fp);
		fclose(fp);
		fp = NULL;
	}
	else
	{
		page_len = get_page_from_pagedb((char*)url, url_len, "../../test/config.ini", "pagedb", page_buf, MAX_PAGE_SIZE);
		FILE *fp = fopen("appp.html", "w");
		assert(fp);
		fwrite(page_buf, page_len, 1, fp);
		fclose(fp);
	}
	if (page_len <= 0)
	{
		fprintf(stderr, "read page fail\n");
		exit(-1);
	}

	int page_len_converted = code_conversionwithlength(page_buf, page_len, page_buf_converted, MAX_PAGE_SIZE);
	if (page_len_converted <= 0)
	{
		printf("code conversion fail\n");
		exit(-1);
	}

	//prepair. 结构化抽取需要依赖解析好的dom树
	

	init_vars();
	int ret = create_all();
	if (ret != 1)
	{
		printf("create all fail\n");
		exit(-1);
	}
	ret = parse(page_buf_converted, page_len_converted, url);
	if (ret != 1)
	{
		printf("parse fail\n");
		exit(-1);
	}

	StructureData* stdata = new StructureData;

	uint8_t *buf_ptr = (uint8_t*) malloc (8192);
	assert(buf_ptr);

	uint32_t sz = 0;

	StructureData* stdata2 = new StructureData;

	int des = 0;

	//调用分类接口，获取pagetype
	PtInput input;
	int pagetype = 0;
	PageClassify *classify = new PageClassify();
	assert(classify);
	if (!classify->Init("../../depends/classify/conf"))
	{
		printf("init pagetype fail\n");
		return -1;
	}

	//调用分类器 获得pagetype
	memset(&input, 0, sizeof(input));
	if(!classify->CacultePtInput(&input, url, url_len, page_len_converted, tree, vtree, atree))
	{
		printf("CacultePtInput fail in pagetype\n");
		goto _FAIL;
	}
	if (0 != classify->Process(&input))
	{
		printf("classify fail");
		goto _FAIL;
	}
	else
	{
		printf("识别结果：%lu\n", input.pagetype64);
		if (NPT_IS_BAIKE(input.pagetype64))
		{
			printf("是百科\n");
			pagetype = 1;
			//调用结构化抽取方法
			ret = html_tree_extract_stdata(url, url_len, pagetype, tree, stdata);
			if (ret != 0)
			{
				printf("extract structure data fail\n");
				goto _FAIL;
			}
			goto _CONTINUE;
		}
		else if(strncmp(url, "http://www.dianping.com/shop/", 29) == 0 && strstr(url+29, "/") == NULL)
		{	
			printf("是点评类\n");
			pagetype = 12;
			//调用结构化抽取方法
			ret = html_tree_extract_stdata(url, url_len, pagetype, tree, stdata);
			if (ret != 0)
			{
				printf("extract structure data fail\n");
				goto _FAIL;
			}
			goto _CONTINUE;
		}
		else if(strncmp(url, "http://jingyan.baidu.com/article/", 33) == 0)
		{
			printf("是百度经验类\n");
			pagetype = 2;
			//调用结构化抽取方法
			ret = html_tree_extract_stdata(url, url_len, pagetype, tree, stdata);
			if (ret != 0)
			{
				printf("extract structure data fail\n");
				goto _FAIL;
			}
			goto _CONTINUE;
		}
		else if(NPT_IS_DOWNLOAD(input.pagetype64) && !NPT_IS_HUB(input.pagetype64))
		{
			printf("是下载且非HUB页\n");
			pagetype = 3;
			//调用结构化抽取方法
			ret = html_tree_extract_download_stdata(url, url_len, pagetype, tree, vtree, atree, stdata);
			if (ret != 0)
			{
				printf("extract structure data fail\n");
				goto _FAIL;
			}
			goto _CONTINUE;
		}
		//else if(NPT_IS_MUSIC(input.pagetype64) && !NPT_IS_HUB(input.pagetype64))
		/*else if(NPT_IS_MUSIC(input.pagetype64))
		{
			printf("音乐页\n");
			if(NPT_IS_PLAY(input.pagetype64))
			{

				printf("是音乐播放页\n");
				pagetype = 16;
			}
			else if(NPT_IS_CONTENT(input.pagetype64))
			{
				printf("是歌词内容页\n");
				pagetype = 17;
			}
			else if(NPT_IS_DOWNLOAD(input.pagetype64))
			{
				printf("是音乐下载页\n");
				pagetype = 18;
			}
			else
				goto _FAIL;

			//调用结构化抽取方法
			ret = html_tree_extract_music_stdata(url, url_len, pagetype, tree, vtree, atree, stdata);
			if (ret != 0)
			{
				printf("extract structure data fail\n");
				goto _FAIL;
			}
			goto _CONTINUE;
		}*/
		else
			goto _FAIL;
	}


	//print debug info
_CONTINUE:
	printf("\nextract info:\n");
	print_stdata_info(stdata);


	//序列化接口
	sz = html_tree_extract_serial(stdata, buf_ptr, 8192);
	if(sz == -1)
	{
		printf("serial fail");
		goto _FAIL;
	}
	printf("\nserial length:%d\tserial buf:%s\n", sz, (char*)buf_ptr);

	//解序列化接口
	des = html_tree_extract_deserial(stdata2, buf_ptr, sz);
	if(des != 0)
	{
		printf("deserial fail");
		goto _FAIL;
	}
	printf("\ndeserial info:\n");
	print_stdata_info(stdata2);

_FAIL:
	//释放分类器
	if (classify)
	{
		classify->Release();
		delete classify;
		classify = NULL;
	}
	if(buf_ptr)
		free(buf_ptr);
	if(stdata2)
		delete stdata2;
	destroy_all();
	if(stdata)
		delete stdata;
	return 0;
}


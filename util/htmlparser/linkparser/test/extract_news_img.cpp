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
#include <string>
#include <vector>
#include <map>
#include <list>
#include "easou_link_common.h"

#define TIMEDELTA_MS(time1,time0) ((time1.tv_sec-time0.tv_sec)*1000+(time1.tv_usec-time0.tv_usec)/1000)

html_tree_t *tree;
vtree_in_t *vtree_in;
html_vtree_t *vtree;
area_tree_t *atree;
lt_res_t *res;

std::vector<std::string> g_url;
typedef std::map<std::string, std::list<std::string> > tdImgType;
tdImgType g_local_img;
tdImgType g_parse_img;

int split_line(const char* line, std::string& k, std::list<std::string>& v)
{
	std::string sline = line;
	bool bfirst = false;
	size_t tbegin = 0;
	size_t tpos = 0;
	while ((tpos = sline.find(",", tbegin)) != std::string::npos)
	{
		if (!bfirst)
		{
			k = sline.substr(tbegin, tpos-tbegin);
			bfirst = true;
		}
		else
		{
			std::string img = sline.substr(tbegin, tpos-tbegin);
			v.push_back(img);
		}

		tbegin = tpos+1;
		if (sline.at(tbegin) == ',')
		{
			break;
		}
	}


	return 0;
}

int diff()
{
	FILE* fp0 = fopen("same.xls", "a+");
	FILE* fp1 = fopen("local.xls", "a+");
	FILE* fp2 = fopen("parse.xls", "a+");
	FILE* fp3 = fopen("diff.xls", "a+");
	FILE* fp4 = fopen("failed.xls", "a+");

        if (NULL == fp0 || NULL == fp1 || NULL == fp2 || NULL == fp3)
        {
                return -1;
        }

	tdImgType::iterator it = g_local_img.begin();
	while (it != g_local_img.end())
	{
		std::list<std::string>& _list = it->second;
		tdImgType::iterator it0 = g_parse_img.find(it->first.c_str());
		if (it0 != g_parse_img.end())
		{
			std::list<std::string>& _list0 = it0->second;
			if (_list == _list0) 
			{
				fwrite(it->first.c_str(), 1, it->first.length(), fp0);
				fwrite("\r\n", 1, strlen("\r\n"), fp0);
			}
			else
			{
				if (_list.size() ==  _list0.size())
				{
					fwrite(it->first.c_str(), 1, it->first.length(), fp3);
					fwrite("\r\n", 1, strlen("\r\n"), fp3);
				}
				else if (_list.size() < _list0.size())
				{
					fwrite(it->first.c_str(), 1, it->first.length(), fp2);
                                        fwrite("\r\n", 1, strlen("\r\n"), fp2);
				}
				else
				{
					fwrite(it->first.c_str(), 1, it->first.length(), fp1);
                                        fwrite("\r\n", 1, strlen("\r\n"), fp1);
				}
			}
		}
		else
		{
			fwrite(it->first.c_str(), 1, it->first.length(), fp4);
                        fwrite("\r\n", 1, strlen("\r\n"), fp4);
		}

		++it;
	}
	
	fclose(fp0);
	fclose(fp1);
	fclose(fp2);
        fclose(fp3);
	fclose(fp4);
}

int initurl(const char* path)
{
	if (NULL == path || *path == '0') 
	{
		return -1;
	}

	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	fp = fopen(path, "r");
	if (fp == NULL)
	{
		return -1;
	}

	while ((read = getdelim(&line, &len, '\n', fp)) != -1) 
	{
		if (line[read-1] == '\n')
		{
			line[read-1] = '\0';
		}

		g_url.push_back(line);	
       	}
	
	if (line != NULL)
	{
		free(line);
	}

	fclose(fp);

	return 0;
}

int initlocalimg(const char* path)
{
	if (NULL == path || *path == '0')
        {
                return -1;
        }

        FILE * fp;
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        fp = fopen(path, "r");
        if (fp == NULL)
        {
                return -1;
        }

        while ((read = getdelim(&line, &len, '\n', fp)) != -1)
        {
                if (line[read-1] == '\n')
                {
                        line[read-1] = '\0';
                }
		
		std::string key;
		std::list<std::string> value;
		split_line(line, key, value);
		g_local_img.insert(std::make_pair(key, value));	
        }

        if (line != NULL)
        {
                free(line);
        }

        fclose(fp);

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

int init()
{
        tree = NULL;
        vtree_in = NULL;
        vtree = NULL;
        atree = NULL;

        if (create_all() != 1)
        {
                printf("create all fail\n");
                return -1;
        }

        if (!Init_Log("log.conf"))
        {
                printf("log.conf not exist\n");
                return -1;
        }

        if (initurl("news.lst") != 0)
        {
                printf("load news.lst failed!\n");
                return -1;
        }

	if (initlocalimg("local_img.csv") != 0)
	{
		printf("load local_img.xls failed!\n");
                return -1;
	}

        if (!init_css_server("./config.ini", "./log", 1000, 10))
        {
                printf("init css server fail\n");
                return -1;
        }

        return 0;
}

int uninit()
{
        destroy_all();
	g_url.clear();
}

int code_conversionwithlength(char *page, int page_len, char *buffer, int buffer_len)
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

void get_linktype_desp(vlink_t *vlink, char *buf)
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

void export_news_img(char* url, img_info_t *imgs, int img_num)
{
	FILE* fp = fopen("news_img.xls", "a+");
	if (fp == NULL)
	{
		return;
	}

	fwrite(url, 1, strlen(url), fp);
	std::vector<std::string> v_imgpool;
	for (int i = 0; i < img_num; ++i)
	{
		img_info_t *img = imgs + i;
		if (img->owner != NULL)
		{
			//TODO:
		}
		else
		{
			std::vector<std::string>::iterator it = std::find(v_imgpool.begin(), v_imgpool.end(), img->img_url);
			if (it != v_imgpool.end())
			{
				printf("image repeated!\n");
				continue;
			}
			else
			{
				printf("img_url:%s \n", img->img_url);
				v_imgpool.push_back(img->img_url);
				
				std::string tmp = "http://";
        			char buff[2048] = {0};
				if (0 ==  easou_combine_url(buff, url, img->img_url))
				{
					tmp += buff;
					g_parse_img[url].push_back(tmp);

					std::string news_img = "\t";
					news_img += tmp;
					news_img += "\r\n";

					fwrite(news_img.c_str(), 1, news_img.length(), fp);
				}
			}
		}
	}

	fclose(fp);
}

int consume_linktype(char* url)
{
        vlink_t vlinks[300];
        int link_num = html_vtree_extract_vlink(vtree, atree, url, vlinks, 300);
        int pagetype = 0; //TODO pagetype
        html_vtree_mark_vlink(vtree, atree, url, vlinks, link_num, pagetype, VLINK_ALL, res);

        img_info_t imgs[300];
        int img_num = extract_img_infos(url, strlen(url), atree, vlinks, link_num, imgs, 300);
        export_news_img(url, imgs, img_num);

        return 1;
}

int extract(char* url)
{
	char* page_path = NULL; // TODO:
	char *page;
	int page_len = 0;
	char page_buf[MAX_PAGE_SIZE] = "";
	char page_buf_converted[MAX_PAGE_SIZE] = "";
	int page_len_converted = 0;
	if (page_path == NULL)
	{
		page_len = download_url(page_buf, MAX_PAGE_SIZE, url, page);
		if (page_len == -1)
		{
			printf("download fail, url=%s\n", url);

			do 
			{
				FILE* fp = fopen("download_failed.xls", "a+");
				if (NULL == fp)
				{
					return -1;
				}
				
				fwrite(url, 1, strlen(url), fp);
				fwrite("\r\n", 1, strlen("\r\n"), fp);
				fclose(fp);
			} while (0);

			return -1;
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
		return -1;
	}

	int ret = parse(page_buf_converted, page_len_converted, url);
	if (ret == 1)
        {
                consume_linktype(url);
        }
        else
        {
                printf("parse fail\n");
        }
}

int main(int argc, char **argv)
{
	// init 
	if (init() != 0)
	{
		printf("init config failed!\n");
		return -1;
	}

	std::vector<std::string>::iterator it = g_url.begin();
	while (it != g_url.end())
	{
		extract((char*)it->c_str());
		++it;
	}

	diff();

	// uninit 	
	uninit();

	return 0;
}


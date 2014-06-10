#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <string>
#include <assert.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "DownLoad.h" //网页下载
#include "CCharset.h" //编码转换
#include "log.h" //easou日志公共库
#include "easou_html_tree.h" //DOM树解析、遍历
#include "easou_html_extractor.h" //从DOM树上抽取信息
using namespace EA_COMMON;
//easou公共库

#define MAX_PAGE_LEN 1<<20
#define MAX_BUF_SIZE 1<<21 //2M
#define URL_BUF_SIZE 1024
#define MAX_PATH_LEN 2048

long allnum;
long parse_fail_num;
char resultBuf[MAX_PAGE_LEN];
html_tree_t *html_tree = NULL;

//params
bool testxl = false;
char *dirpath = NULL;
char *pagepath = NULL;
char *url = NULL;
char *edbfile = NULL;
bool debug = false;
bool output = false;
int maxSearchFile = 0;
int start = 0;
int indexNum = 0;

html_tree_t *tree;

char *result_buf = NULL;
int result_buf_size = MAX_PAGE_SIZE;
char *page = NULL;
int page_size = MAX_PAGE_SIZE;
char *page_converted = NULL;
int page_converted_size = MAX_PAGE_SIZE;

#define TIMEDELTA_MS(time1,time0) ((time1.tv_sec-time0.tv_sec)*1000+(time1.tv_usec-time0.tv_usec)/1000)

int parse_only(char *page, int page_len, char *url);

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

static void init_vars()
{
	allnum = 0;
	parse_fail_num = 0;
	debug = false;
	output = false;
	maxSearchFile = INT_MAX;
	start = 0;
	indexNum = 0;
	html_tree = NULL;
}

static void print_vars_info()
{
	Info("allnum:%lu parse_fail_num:%lu", allnum, parse_fail_num);
}

static void print_help()
{
	printf("Usage:\n");
	printf("\t-dir [dirPath], assign test dir\n");
	printf("\t-p [pagePath], assign test page\n");
	printf("\t-u [url], assign url, use with -p param\n");
	printf("\t-ef [edbFilePath], assign edb format file to test\n");
	printf("\t--debug, save last page which going to be parsed as current.html\n");
	printf("\t-output, save every parsed page in dirctory test-pages, save every parse result in directory test-results\n");
	printf("\t-h, print help info\n");
}

/**
 * @return:
 * 1, not exist
 * 0, is a directory
 * -1, not a directory or unrecognized
 */
static int check_dir(const char * dirpath)
{
	struct stat buf;
	int result = stat(dirpath, &buf);
	if (result != 0)
	{
		if (errno == ENOENT)
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		if (buf.st_mode & S_IFDIR)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
}

static int code_conversion(char *page, int page_len, char *buffer, int buffer_len)
{
	// detect  page charset
	char charset[64] =
	{ 0 };
	CCharset m_cCharset;
	int ret;
	if ((ret = m_cCharset.GetCharset(page, page_len, charset)) < 0)
	{
		return -1;
	}

	if ((ret = m_cCharset.TransCToGb18030(page, page_len, buffer, buffer_len, charset)) < 0)
	{
		return -1;
	}
	return 0;
}

void PrintNode(html_node_t *html_node, int level, FILE *fp)
{
	html_attribute_t *attribute;
	html_node_t *child;

	if (level != 0)
	{
		fprintf(fp, " ");
	}
	for (int i = 0; i < level - 1; i++)
	{
		fprintf(fp, "  ");
	}

	if (html_node->html_tag.tag_type != TAG_PURETEXT && html_node->parent != NULL)
	{
		if (html_node->html_tag.tag_type == TAG_DOCTYPE)
		{
			fprintf(fp, "<!DOCTYPE %s tagCode=\"%d\"", html_node->html_tag.text, html_node->html_tag.tag_code);
		}
		else if (html_node->html_tag.tag_type == TAG_COMMENT)
		{
			fprintf(fp, "<!-- %s tagCode=\"%d\" -->", html_node->html_tag.text, html_node->html_tag.tag_code);
		}
		else
		{
			//打印标签名称
			if (html_node->html_tag.tag_name)
			{
				fprintf(fp, "<%s tagCode=\"%d\"", html_node->html_tag.tag_name, html_node->html_tag.tag_code);
			}
			//打印属性列表
			for (attribute = html_node->html_tag.attribute; attribute != NULL; attribute = attribute->next)
			{
				fprintf(fp, " %s", attribute->name);
				if (attribute->value != NULL)
				{
					fprintf(fp, "=\"%s\"", attribute->value);
				}
			}
			fprintf(fp, ">\n");
		}
	}

	if (html_node->html_tag.tag_type == TAG_SCRIPT || html_node->html_tag.tag_type == TAG_STYLE || html_node->html_tag.tag_type == TAG_PURETEXT)
	{
		//打印标签文本
		fprintf(fp, "%s\n", html_node->html_tag.text);
	}
	else
	{
		//遍历子节点
		for (child = html_node->child; child != NULL; child = child->next)
		{
			PrintNode(child, level + 1, fp);
		}
	}
	if (html_node->html_tag.tag_type != TAG_PURETEXT && html_node->parent != NULL && html_node->html_tag.tag_type != TAG_COMMENT && html_node->html_tag.tag_type != TAG_DOCTYPE && !html_node->html_tag.is_self_closed)
	{
		for (int i = 0; i < level - 1; i++)
		{
			fprintf(fp, "  ");
		}
		fprintf(fp, "</%s>\n", html_node->html_tag.tag_name);
	}
}

int parse(char* page, int page_len, char* url, char* resultBuf, int bufLen)
{
	char *pret = resultBuf;
	allnum++;
	int ret = 0;

	char tagTitle[MAX_TITLE_SIZE] =
	{ 0 };
	char abstract[MAX_CONTENT_SIZE] =
	{ 0 };
	link_t links[MAX_LINK_NUM];
	memset(links, 0, sizeof(links));

	//创建DOM树
	html_tree = html_tree_create(MAX_PAGE_SIZE);
	if (!html_tree)
	{
		Warn("html_tree_create fail");
		exit(-1);
	}

	//解析DOM树
	if (0 == html_tree_parse(html_tree, page, page_len))
	{
		parse_fail_num++;
		Warn("html_tree_parse fail");
		html_tree_del(html_tree);
		return -1;
	}

	//打印DOM树
	if (debug)
	{
		FILE *fp = fopen("result.html", "w");
		if (fp != NULL)
		{
			PrintNode(&html_tree->root, 0, fp);
			fclose(fp);
			fp = NULL;
		}
	}

	//提取tag-title
	ret = html_tree_extract_title(html_tree, tagTitle, MAX_TITLE_SIZE);
	if (ret < 0)
	{
		Warn("html_tree_extract_title fail");
	}
	else
	{
		pret += sprintf(pret, "tag-title:%s\n", tagTitle);
	}

	//提取abstract
	ret = html_tree_extract_abstract(html_tree, abstract, MAX_CONTENT_SIZE);
	if (ret < 0)
	{
		Warn("html_tree_extract_abstract fail");
	}
	else
	{
		pret += sprintf(pret, "abstract:%s\n", abstract);
	}

	//抽取链接信息
	int link_array_size = MAX_LINK_NUM;
	int link_num = html_tree_extract_link(html_tree, url, links, link_array_size);
	for (int i = 0; i < link_num; i++)
	{
		pret += sprintf(pret, "link%d, text:%s, url:%s\n", i, links[i].text, links[i].url);
	}

	//销毁解析树
	html_tree_del(html_tree);
	html_tree = NULL;
	return 0;
}

void List(const char *path, int level)
{
	struct dirent* ent = NULL;
	DIR *pDir;
	pDir = opendir(path);
	if (pDir == NULL)
	{
		//被当作目录，但是执行opendir后发现又不是目录，比如软链接就会发生这样的情况。
		return;
	}
	while (NULL != (ent = readdir(pDir)))
	{
		if (ent->d_type == DT_REG)
		{
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
			{
				continue;
			}
			//file
			char filepath[1024] =
			{ 0 };
			sprintf(filepath, "%s/%s", path, ent->d_name);
			FILE *fpPage = fopen(filepath, "r");
			if (fpPage == NULL)
			{
				Warn((char*) "fopen fail, %s", filepath);
			}
			else
			{
				if (indexNum++ <= start)
				{
					return;
				}
				if (allnum >= maxSearchFile)
				{
					return;
				}
				char pageBuf[128000] =
				{ 0 };
				int pagelen = fread(pageBuf, 1, 128000, fpPage);
				fclose(fpPage);
				fpPage = NULL;
				memset(resultBuf, 0, MAX_PAGE_LEN);
				parse(pageBuf, pagelen, (char*) "http://wap.easou.com/", resultBuf, MAX_PAGE_LEN);
				if (output)
				{
					char outputpath[MAX_PATH_LEN] =
					{ 0 };
					sprintf(outputpath, "test-result/%d.txt", indexNum);
					FILE *fpret = fopen(outputpath, "w");
					if (fpret != NULL)
					{
						fwrite(resultBuf, 1, strlen(resultBuf), fpret);
						fclose(fpret);
						fpret = NULL;
					}
					// save page
					char savepagepath[MAX_PATH_LEN] =
					{ 0 };
					sprintf(savepagepath, "test-pages/%d.html", indexNum);
					FILE *fpsave = fopen(savepagepath, "w");
					if (fpsave != NULL)
					{
						fwrite(pageBuf, 1, pagelen, fpsave);
						fclose(fpsave);
						fpsave = NULL;
					}
				}
			}
		}
		else
		{
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
			{
				continue;
			}
			if (allnum >= maxSearchFile)
			{
				return;
			}
			//directory
			std::string _path(path);
			std::string _dirName(ent->d_name);
			std::string fullDirPath = _path + "/" + _dirName;
			List(fullDirPath.c_str(), level + 1);
		}
	}
}

static int shift_buf(char *buf, int size, int shiftlen)
{
	if (shiftlen <= 0)
	{
		return shiftlen;
	}
	if (shiftlen >= size)
	{
		memset(buf, 0, size);
		return size;
	}
	char *p = buf, *pend = buf + size - shiftlen;
	while (p != pend)
	{
		*p = *(p + shiftlen);
		p++;
	}
	memset(buf + size - shiftlen, 0, shiftlen);
	return shiftlen;
}

static char* read_more_from_file(char *buf, int size, char *handledpos, FILE *fpdata)
{
	int handledlen = handledpos - buf;
	if (handledlen == 0)
	{
		// 处理文件中的 0
		char *pend = buf + size;
		char *pstart = buf;
		while (pstart != pend && *pstart == 0)
		{
			pstart++;
		}
		handledlen = pstart - buf;
		if (handledlen == 0)
		{
			return buf;
		}
	}
	int shiftedlen = shift_buf(buf, size, handledlen);
	int readlen = fread(buf + size - shiftedlen, 1, shiftedlen, fpdata);
	return buf + size - shiftedlen + readlen;
}

static int handle_edb_data_file(const char* datapath)
{
	Info("start test edb data file, %s\n", datapath);

	FILE *fpdata = fopen64(datapath, "r");
	if (fpdata == NULL)
	{
		Warn("fopen fail, %s\n", datapath);
		return -1;
	}

	char buf[MAX_BUF_SIZE] =
	{ 0 };
	int size = MAX_BUF_SIZE - 1;

	int readlen = fread(buf, 1, size, fpdata);

	char *preadpos = buf, *pend = buf + readlen;
	char *pkeypos = NULL, *pkeyposend = NULL;
	char *ppagepos = NULL, *ppageend = NULL;
	char key[URL_BUF_SIZE] =
	{ 0 }; //save url
	char page[MAX_PAGE_LEN] =
	{ 0 }; //save page
	char page_conversion[MAX_PAGE_LEN] =
	{ 0 }; //save gb18030 page

	while (true)
	{
		pkeypos = strstr(preadpos, "key:http://");
		if (pkeypos == NULL && pend != (buf + size))
		{
			break;
		}
		if (pkeypos == NULL)
		{
			int strlennum = strlen(preadpos);
			int handledlen = pend - preadpos;
			if (strlennum == handledlen)
			{
				char *newpend = read_more_from_file(buf, size, preadpos, fpdata);
				if (newpend == buf)
				{
					break;
				}
				pend = newpend;
				preadpos = buf;
				continue;
			}
			else
			{
				preadpos += strlennum;
				while (preadpos != pend && *preadpos == 0)
				{
					preadpos++;
				}
				continue;
			}
		}

		pkeyposend = strstr(pkeypos, "\n");
		if (pkeyposend == NULL && pend != (buf + size))
		{
			break;
		}
		if (pkeyposend == NULL)
		{
			int strlennum = strlen(preadpos);
			int handledlen = pend - preadpos;
			if (strlennum == handledlen)
			{
				char *newpend = read_more_from_file(buf, size, preadpos, fpdata);
				if (newpend == buf)
				{
					break;
				}
				pend = newpend;
				preadpos = buf;
				continue;
			}
			else
			{
				preadpos += strlennum;
				while (preadpos != pend && *preadpos == 0)
				{
					preadpos++;
				}
				continue;
			}
		}

		ppagepos = strstr(pkeypos, "page:");
		if (ppagepos == NULL && pend != (buf + size))
		{
			break;
		}
		if (ppagepos == NULL)
		{
			int strlennum = strlen(preadpos);
			int handledlen = pend - preadpos;
			if (strlennum == handledlen)
			{
				char *newpend = read_more_from_file(buf, size, preadpos, fpdata);
				if (newpend == buf)
				{
					break;
				}
				pend = newpend;
				preadpos = buf;
				continue;
			}
			else
			{
				preadpos += strlennum;
				while (preadpos != pend && *preadpos == 0)
				{
					preadpos++;
				}
				continue;
			}
		}

		ppageend = strstr(ppagepos, "key:");
		if (ppageend == NULL && pend != (buf + size))
		{
			break;
		}
		if (ppageend == NULL)
		{
			int strlennum = strlen(preadpos);
			int handledlen = pend - preadpos;
			if (strlennum == handledlen)
			{
				char *newpend = read_more_from_file(buf, size, preadpos, fpdata);
				if (newpend == buf)
				{
					break;
				}
				pend = newpend;
				preadpos = buf;
				continue;
			}
			else
			{
				preadpos += strlennum;
				while (preadpos != pend && *preadpos == 0)
				{
					preadpos++;
				}
				continue;
			}
		}

		memset(key, 0, URL_BUF_SIZE);
		pkeypos += 4; //skip key:
		int keylen = pkeyposend - pkeypos;
		if (keylen > URL_BUF_SIZE)
		{
			preadpos = ppageend;
			continue;
		}
		memcpy(key, pkeypos, keylen);

		memset(page, 0, MAX_PAGE_LEN);
		ppagepos += 5; //skip page:
		int pagelen = ppageend - ppagepos;
		if (pagelen > MAX_PAGE_LEN)
		{
			pagelen = MAX_PAGE_LEN;
		}
		memcpy(page, ppagepos, pagelen);

		// code conversion
		if (code_conversion(page, pagelen, page_conversion, MAX_PAGE_LEN))
		{
			// if code conversion fail
			preadpos = ppageend;
			continue;
		}

		if (indexNum++ <= start)
		{
			preadpos = ppageend;
			continue;
		}

		if (debug)
		{
			// save page
			FILE *fpsave = fopen("current.html", "w");
			if (fpsave == NULL)
			{
				Warn("save page fail\n");
				preadpos = ppageend;
				continue;
			}
			fwrite(key, 1, keylen, fpsave);
			fwrite("\n", 1, 1, fpsave);
			fwrite(page, 1, pagelen, fpsave);
			fclose(fpsave);
			fpsave = NULL;
		}

		memset(resultBuf, 0, MAX_PAGE_LEN);
		parse(page_conversion, strlen(page_conversion), key, resultBuf, MAX_PAGE_LEN);

		preadpos = ppageend;

		if (output)
		{
			char outputpath[MAX_PATH_LEN] =
			{ 0 };
			sprintf(outputpath, "test-result/%d.txt", indexNum);
			FILE *fpret = fopen(outputpath, "w");
			if (fpret != NULL)
			{
				fwrite(key, 1, strlen(key), fpret);
				fwrite("\n", 1, 1, fpret);
				fwrite(resultBuf, 1, strlen(resultBuf), fpret);
				fclose(fpret);
				fpret = NULL;
			}
			// save page
			char savepagepath[MAX_PATH_LEN] =
			{ 0 };
			sprintf(savepagepath, "test-pages/%d.html", indexNum);
			FILE *fpsave = fopen(savepagepath, "w");
			if (fpsave != NULL)
			{
				fwrite(key, 1, keylen, fpsave);
				fwrite("\n", 1, 1, fpsave);
				fwrite(page, 1, pagelen, fpsave);
				fclose(fpsave);
				fpsave = NULL;
			}
		}
		if (allnum >= maxSearchFile)
		{
			break;
		}
	}

	fclose(fpdata);
	fpdata = NULL;

	print_vars_info();

	return 0;
}

void assign_input_param(int argc, char **argv)
{
	for (int i = 0; i < argc; i++)
	{
		if (*argv[i] != '-')
		{
			continue;
		}
		if (strcmp("-dir", argv[i]) == 0 && i < argc - 1)
		{
			dirpath = argv[i + 1];
		}
		if (strcmp("-p", argv[i]) == 0 && i < argc - 1)
		{
			pagepath = argv[i + 1];
		}
		if (strcmp("-u", argv[i]) == 0 && i < argc - 1)
		{
			url = argv[i + 1];
		}
		if (strcmp("-ef", argv[i]) == 0 && i < argc - 1)
		{
			edbfile = argv[i + 1];
		}
		if (strcmp("--debug", argv[i]) == 0)
		{
			debug = true;
		}
		if (strcmp("-output", argv[i]) == 0)
		{
			output = true;
		}
		if (strcmp("-limit", argv[i]) == 0 && i < argc - 1)
		{
			maxSearchFile = atol(argv[i + 1]);
		}
		if (strcmp("-start", argv[i]) == 0 && i < argc - 1)
		{
			start = atol(argv[i + 1]);
		}
		if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0)
		{
			print_help();
			exit(0);
		}
	}
}

static int code_conversionwithlength(char *page, int page_len, char *buffer, int buffer_len)
{
	// detect  page charset
	char charset[64] = "";
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

static int handle_single_page_buf(char *page, int pageLen, char *resultBuf, int resultBufSize, char *url)
{
	// code conversion
	int page_len = code_conversionwithlength(page, pageLen, page_converted, page_converted_size);
	if (page_len <= 0)
	{
		Warn("code conversion fail, url:%s", url);
		return -1;
	}
	*result_buf = '\0';
	*(page_converted + page_len) = '\0';

	char urldefault[] = "http://wap.easou.com/";
	if (url == NULL)
	{
		printf("set url to default http://wap.easou.com/");
		url = urldefault;
	}

	int ret = parse_only(page_converted, page_len, url);
	if (ret != 1)
	{
		parse_fail_num++;
		return -1;
	}

	//打印DOM树
	if (debug)
	{
		FILE *fp = fopen("result.html", "w");
		if (fp != NULL)
		{
			PrintNode(&tree->root, 0, fp);
			fclose(fp);
			fp = NULL;
		}
	}

//	char *p = result_buf;
//	p = consume_linktype(result_buf, result_buf_size, url);
//	saveParseInfo(page_converted, page_len, result_buf);

//	save_test_result(page, pageLen, result_buf, strlen(result_buf));
	return 0;
}

void destroy_all()
{
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
	return 1;
}

int reset_all()
{
	if (tree)
	{
		html_tree_reset_no_destroy((struct html_tree_impl_t*) tree);
	}
	return 0;
}

static int handle_single_page(char *pagePath, char *url, char *resultBuf, int size)
{
	// test single page
	*page = '\0';
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
		pageLen = fread(page, 1, page_size, fpage);
		fclose(fpage);
		fpage = NULL;
		pageBegin = page;
	}
	else
	{
		//download page from internet
		printf("begin download page, url:%s\n", url);
		pageLen = download_url(page, page_size, url, pageBegin);
		if (pageLen < 0)
		{
			printf("download fail, %s\n", url);
			return -1;
		}
		printf("download page success\n");
		*(pageBegin + pageLen) = '\0';
	}

	handle_single_page_buf(pageBegin, pageLen, resultBuf, size, url);
	return 0;
}

int parse_only(char *page, int page_len, char *url)
{
	allnum++;
	reset_all();

	int ret = html_tree_parse(tree, page, page_len);
	if (ret != 1)
	{
		Warn("html_tree_parse fail, url:%s", url);
		return -1;
	}
	char sourcecode[1024 * 1024] =
	{ 0 };
	int len = printTreeSrc(tree, sourcecode, 1024 * 1024);
	FILE *psouce = fopen("sourcecode.txt", "w");
	if (psouce)
	{
		fwrite(sourcecode, 1, len, psouce);
		fclose(psouce);
		psouce = NULL;
	}
	return 1;
}

int main(int argc, char** argv)
{
	if (!Init_Log("log.conf"))
	{
		return 0;
	}

	init_vars();
	assign_input_param(argc, argv);

	struct timeval endtv1, endtv2;
	gettimeofday(&endtv1, NULL); //开始时间

	int ret = create_all();
	if (ret != 1)
	{
		printf("create all fail\n");
		exit(-1);
	}
	result_buf = (char*) malloc(result_buf_size + 1);
	page = (char*) malloc(page_size + 1);
	page_converted = (char*) malloc(page_converted_size + 1);
	assert(page&&page_converted);

	if (dirpath)
	{
		List(dirpath, 0);
		print_vars_info();
	}
	else if (pagepath != NULL || url != NULL)
	{
		handle_single_page(pagepath, url, result_buf, result_buf_size);
	}
	else if (edbfile)
	{
		if (output)
		{
			int dirret = check_dir("test-result");
			if (dirret == 1)
			{ // dir not exist
				mkdir("test-result", 0777);
			}
			else
			{
				printf("test-result already exist, please delete it before test\n");
				return 0;
			}
			dirret = check_dir("test-pages");
			if (dirret == 1)
			{ // dir not exist
				mkdir("test-pages", 0777);
			}
			else
			{
				printf("test-pages already exist, please delete it before test\n");
				return 0;
			}
		}
		handle_edb_data_file(edbfile);
	}
	else
	{
		printf("use -h to see help\n");
		return -1;
	}

	gettimeofday(&endtv2, NULL); //结束时间
	destroy_all();
	free(result_buf);
	free(page);
	free(page_converted);
	int tot_time1 = TIMEDELTA_MS(endtv2, endtv1); //单位：毫秒
	printf("time=%dms\n", tot_time1);
	return 0;
}

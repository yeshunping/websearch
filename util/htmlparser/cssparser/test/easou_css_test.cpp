#include "easou_html_tree.h"
#include "easou_css_pool.h"
#include "easou_css_parser.h"
#include "easou_css_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>

void print_url(easou_css_pool_t *css_pool, const char *url) {
	printf("%s\n", url);
}

/**
int doPack(char *path, char *fileName, int *total) {
	char page[MAX_PAGE_SIZE];
	memset(page, 0, sizeof(page));
	char url[MAX_URL_LEN];
	memset(url, 0, sizeof(url));
	CPacket packet(path, fileName);
//  CPacket packet("../testdata/test1/", "package.0000000198");
	int nErr;
	int pageCount = 0;
	while (1) {
		Page *ptr = packet.next(nErr);
		if (ptr == NULL) {
			break;
		}
		memcpy(page, ptr->page, ptr->pagelen);
		memcpy(url, ptr->url, ptr->urllen);
		printf("%s---%d---%d\n", fileName, pageCount++, (*total)++);

		easou_css_env_t * css_env =  css_env_create(256*1024, 128);
		css_parser_init();
		html_tree_t *html_tree = html_tree_create(MAX_PAGE_SIZE);
		html_tree_clean(html_tree);
		html_tree_parse(html_tree, page, strlen(page));
		get_parse_css_inpage(css_env, html_tree, url);
		for (int i = 0; i < css_env->css_pool.used_css_num; i++) {
//			print_css(css_env->css_pool.css_array[i], fout);
		}
		css_env_destroy(css_env);
		html_tree_clean(html_tree);
		html_tree_del(html_tree);
		memset(page, 0, sizeof(page));
		memset(url, 0, sizeof(url));
	}
	return 0;
}
*/

int parse(char* page, int page_len, char* url) {
	FILE* fp = fopen("result", "w");
	html_tree_t *html_tree = NULL;
	easou_css_env_t * css_env =  css_env_create(256*1024, 128);
//	css_parser_init();
	html_tree = html_tree_create(MAX_PAGE_SIZE);
	if (!html_tree){
		printf("解析失败！\n");
		goto FAIL;
	}
	if(0 == html_tree_parse(html_tree, page, page_len)){
		printf("解析失败！\n");
		goto FAIL;
	}
	else{
		printf("解析成功！	");
	}
	get_parse_css_inpage(css_env, html_tree, url);

FAIL:
	if (html_tree){
		html_tree_del(html_tree);
	}
	if(fp){
		fclose(fp);
	}
	if(css_env){
		css_env_destroy(css_env);
	}
	return 0;
}

int main(int argc, char** argv) {
	if(!Init_Log("log.conf")){
		return 0;
	}
	if (argc != 2) {
		printf("参数不对！\n");
		return 0;
	}
	char url[1024];
	char path[256];
	memset(url, 0, 1024);
	memset(path, 0, 256);
	memcpy(path, argv[1], strlen(argv[1]));
	sprintf(path+strlen(path), "/url.txt");
	FILE *index = fopen(path, "r");
	int count = 0;
	while(fgets(url, 1024, index) != 0){
		url[strlen(url)-2] = 0;
		memset(path, 0, 256);
		memcpy(path, argv[1], strlen(argv[1]));
		sprintf(path + strlen(path), "/%d", count++);
		sprintf(path + strlen(path), ".htm");
		FILE* fp = fopen(path, "r");
		if (0 == fp) {
			printf("打开文件%s失败\n", argv[1]);
			continue;
		}
		fseek(fp, 0, SEEK_END);
		int len = ftell(fp);
		assert(len <= MAX_PAGE_SIZE);
		fseek(fp, 0, SEEK_SET);
		char page[MAX_PAGE_SIZE];
		fread(page, 1, len, fp);
		page[len] = 0;
		parse(page, strlen(page), url);
		fclose(fp);
		memset(url, 0, 1024);
	}
	fclose(index);
	return 0;
}


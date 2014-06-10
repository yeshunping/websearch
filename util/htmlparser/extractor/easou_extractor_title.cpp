/*
 * easou_extractor_title.cpp
 *
 *  Created on: 2012-1-11
 *      Author: xunwu
 */

#include <string.h>
#include "easou_html_extractor.h"
#include "easou_extractor_com.h"
#include "easou_extractor_title.h"



/**
 * @brief 提取网页的tagtitle
 * 返回实际提取的字符串长度
**/
int html_tree_extract_tagtitle(html_tree_t *html_tree, char* tagtitle, int size)
{
	return html_tree_extract_title(html_tree, tagtitle, size);
}

/**
 * @brief 提取网页的realtitle，要求网页已经完成mark
**/
int html_atree_extract_realtitle(area_tree_t *area_tree, char *realtitle, int size)
{
	if(!realtitle||size<1){
		return 0;
	}
	memset(realtitle, 0, size);
	get_area_content(realtitle, size, area_tree, AREA_SEM_REALTITLE);
	int length=strlen(realtitle);
	return length;
}

/**
 * @brief 提取网页的subtitle，要求网页已经完成mark
**/
int html_atree_extract_subtitle(area_tree_t *area_tree, char *subtitle, int size)
{
	memset(subtitle, 0, size);
	get_area_content(subtitle, size, area_tree, AREA_FUNC_SUBTITLE);
	int length=strlen(subtitle);
	if(length == 0){
		return 0;
	}
	return length;
}

static const char* url_prefix = "http://tieba.baidu.com/f?kw=";
static const int url_prefix_len = strlen(url_prefix);

static const char* tagtitle_suffix = "吧_百度贴吧";
static const int tagtitle_suffix_len = strlen(tagtitle_suffix);

/**
 * @brief 判断是否为吧首页
 */
static bool is_home_tieba(const char* url, int url_len, const char* tagtitle, int tagtitle_len)
{

	if (url == NULL || tagtitle == NULL)
		return false;
	if (strstr(url, url_prefix) != url)
		return false;
	if (tagtitle_len <= tagtitle_suffix_len)
		return false;
	if (strcmp(tagtitle_suffix, tagtitle + tagtitle_len - tagtitle_suffix_len) != 0)	
		return false;

	return true;
}

int easou_extract_realtitle(realtitle_input_t* input, char* realtitle, int size)
{
	if (input == NULL || realtitle == NULL || size <= 0)
		return -1;
	bool tieba_home = is_home_tieba(input->url, input->url_len, input->tagtitle, input->tagtitle_len);
	if (tieba_home)
	{
		int left_len = input->tagtitle_len - tagtitle_suffix_len;
		int cpy_len = left_len < size ? left_len : (size - 1);
		memcpy(realtitle, input->tagtitle, cpy_len);
		realtitle[cpy_len] = 0;
		return cpy_len;
	}
	else
		return html_atree_extract_realtitle(input->atree, realtitle, size);
}

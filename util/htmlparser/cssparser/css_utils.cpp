/**
 * css_utils.cpp
 * Description: cssparser对外接口
 *  Created on: 2011-06-20
 * Last modify: 2012-10-26 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util/htmlparser/utils/log.h"
#include "util/htmlparser/htmlparser/html_pool.h"
#include "util/htmlparser/htmlparser/html_attr.h"
#include "util/htmlparser/htmlparser/html_tree.h"
#include "css_pool.h"
#include "css_parser.h"
#include "css_utils.h"
#include "util/htmlparser/utils/debug.h"
#include "util/htmlparser/utils/string_util.h"

using namespace EA_COMMON;

/**
 * @brief 销毁css环境

 * @date 2011/06/20
 **/
void css_env_destroy(css_env_t *cc)
{
	if (cc)
	{
		css_pool_destroy(&cc->css_pool);
		free(cc);
	}
}

/**
 * @brief CSS环境的创建.
 * @param [in] max_css_page_size   : int 最大css
 * @param [in] css_num   : int	最多解析css的数量.
 * @return  css_env_t*	css解析环境.

 * @date 2011/06/20
 **/
css_env_t *css_env_create(int max_css_page_size, int css_num)
{
	css_env_t *cc = (css_env_t *) calloc(1, sizeof(css_env_t));
	if (NULL == cc)
	{
		Fatal((char*) "%s:%d:alloc error!", __FILE__, __LINE__);
		goto ERR;
	}
	if (css_pool_init(&cc->css_pool, max_css_page_size, css_num) == -1)
	{
		goto ERR;
	}
	return cc;
	ERR: css_env_destroy(cc);
	return NULL;
}

/**
 * @brief 清空css信息
 */
static void cssinfo_keep_clean(cssinfo_keep_t *css_keep)
{
	css_keep->url = NULL;
	css_keep->is_skip_child = false;
}

bool is_apply_for_screen_media_list(const char *pvalue)
{
	if (pvalue == NULL || pvalue[0] == '\0')
	{
		return true;
	}
	const char *nextp = NULL;
	for (const char *p = pvalue; *p; p = nextp)
	{
		p = skip_space(p);
		const char *sep = strchr(p, ',');
		int l = 0;
		if (sep != NULL)
		{
			l = sep - p;
			nextp = p + l + 1;
		}
		else
		{
			l = strlen(p);
			nextp = p + l;
		}
		if (strncasecmp(p, "all", strlen("all")) == 0)
		{
			if (is_only_space_between(p + strlen("all"), p + l))
			{
				return true;
			}
		}
		else if (strncasecmp(p, "screen", strlen("screen")) == 0)
		{
			if (is_only_space_between(p + strlen("screen"), p + l))
			{
				return true;
			}
		}
	}
	return false;
}

/**
 * @brief 判断该节点是否会影响screen

 * @date 2011/06/20
 **/
bool is_apply_for_screen_media(html_tag_t *html_tag)
{
	assert(html_tag->tag_type == TAG_LINK || html_tag->tag_type == TAG_STYLE);
	const char *pvalue = get_attribute_value(html_tag, ATTR_MEDIA);
	return is_apply_for_screen_media_list(pvalue);
}

/**
 * @brief 判断标签是否是css link

 * @date 2011/06/20
 **/
bool is_css_link_tag(html_tag_t *html_tag)
{
	/**必须是link*/
	if (html_tag->tag_type != TAG_LINK)
	{
		return false;
	}
	/**ATTR_REL属性必须是stylesheet*/
	const char *pvalue = get_attribute_value(html_tag, ATTR_REL);
	if (pvalue != NULL && strcasecmp(pvalue, "stylesheet") == 0 && is_apply_for_screen_media(html_tag))
	{
		return true;
	}
	return false;
}

static void get_style_text(page_css_t *css_keep, html_tag_t *html_tag, const char *url)
{
	if (!is_apply_for_screen_media(html_tag))
	{
		return;
	}
	char *ptxt = html_tag->text;
	if (ptxt && ptxt[0] != '\0')
	{
		if ((unsigned) css_keep->style_txt_num < sizeof(css_keep->style_txt) / sizeof(char *))
		{
			css_keep->style_txt[css_keep->style_txt_num++] = ptxt;
		}
		else
		{
//			Warn("%s:too many style text:%s!", __FUNCTION__, url);
		}
		/**处理style的import进来的css，不考虑*/
		//	collect_import_css_url(css_keep, ptxt, url);
	}
}

static int start_visit_for_cssinfo(html_tag_t *html_tag, void *result, int flag)
{
	cssinfo_keep_t *css_keep = (cssinfo_keep_t *) result;
	if (html_tag->tag_type == TAG_STYLE)
	{
		get_style_text(css_keep->page_css, html_tag, css_keep->url);
	}
	return VISIT_NORMAL;
}

/**
 * @brief 获取页面中的css.
 * @see

 * @date 2011/06/20
 **/
static void get_cssinfo(cssinfo_keep_t *css_keep, const html_tree_t *html_tree, const char *url)
{
	css_keep->url = url;
	html_tree_visit((html_tree_t *) html_tree, &start_visit_for_cssinfo, NULL, css_keep, 0);
}

/**
 * @brief	从页面中获取css信息
 * @param [out] page_css   : page_css_t*	页面中的css信息
 * @param [in] html_tree   : const html_tree_t*	创建好的dom树
 * @param [in] url   : const char*	页面的url

 * @date 2011/06/20
 **/
void get_page_css_info(page_css_t *page_css, const html_tree_t *html_tree, const char *url)
{
	cssinfo_keep_t css_keep;
	css_keep.page_css = page_css;
	cssinfo_keep_clean(&css_keep);
	get_cssinfo(&css_keep, html_tree, url);
}

/**
 * @brief 解析页面中的css

 * @date 2011/06/20
 **/
void parse_internal_css(css_env_t *css_env, page_css_t *page_css, const char *page_url, bool test_import)
{
	css_pool_t *css_pool = &css_env->css_pool;
	css_pool_clean(css_pool);
	int order = css_pool->used_css_num;
	int success = 0;
	int fail = 0;
	int select_id = 0;
	for (int i = 0; i < page_css->style_txt_num; i++)
	{
		if (css_pool->used_css_num < css_pool->alloc_css_num)
		{
			int ret = css_parse(css_pool->css_array[css_pool->used_css_num], page_css->style_txt[i], css_env->page_css.css_url[css_pool->used_css_num], false, test_import);
			if (ret < 0)
			{
//				Warn("%s:parse css error:%s.", __FUNCTION__, page_url);
				fail++;
			}
			else
			{
				css_pool->order[css_pool->used_css_num] = order++;

				for (css_ruleset_t* ruleset = css_pool->css_array[css_pool->used_css_num]->all_ruleset_list; ruleset != NULL; ruleset = ruleset->next)
					ruleset->id = select_id++;

				css_create_hash_index(css_pool->css_array[css_pool->used_css_num], css_pool->hm);

				css_pool->used_css_num++;
				success++;
			}
		}
		else
		{
//			Warn("%s:css pool full:%s!", __FUNCTION__, page_url);
			break;
		}
	}
	css_pool_sort(css_pool);

	counter_add("css_selector_num", select_id); record_max("css_selector_max", (unsigned int)select_id);
}

/**
 * @brief	获取并解析页面中的css.
 * @param [out] css_env   : css_env_t*	css解析环境.
 * @param [in] html_tree   : html_tree_t*	解析过的html树.
 * @param [in] url   : const char*	页面URL.

 * @date 2011/06/20
 **/
int get_parse_css_inpage(css_env_t *css_env, const html_tree_t *html_tree, const char *url, bool test_import)
{
	get_page_css_info(&(css_env->page_css), html_tree, url);
	parse_internal_css(css_env, &(css_env->page_css), url, test_import);
#ifdef DEBUG_INFO
	csspool_print_selector(&css_env->css_pool, "css_selector.txt");
#endif
	return css_env->page_css.style_txt_num;
}

void csspool_print_selector(const css_pool_t *csspool, const char* filename)
{
	if (csspool->used_css_num <= 0)
		return;
	FILE *fp = fopen(filename, "w");
	if (fp == NULL)
		return;

	fprintf(fp, "csspool->used_css_num\t%d\n\n", csspool->used_css_num);
	for (int i = 0; i < csspool->used_css_num; i++)
		print_css(csspool->css_array[i], fp);
	fclose(fp);
}

/**
 * 复位css环境
 */
void css_env_reset(css_env_t *cc)
{
	if (cc)
	{
		css_pool_clean(&cc->css_pool);
		cc->page_css.style_txt_num = 0;
		memset(cc->page_css.style_txt, 0, sizeof(cc->page_css.style_txt));
	}
}

void add_out_style_text(page_css_t *css_keep, char * ptxt, char *css_url)
{
	if (css_keep == NULL)
	{
		return;
	}
	if (ptxt && ptxt[0] != '\0')
	{
		if ((unsigned) css_keep->style_txt_num < sizeof(css_keep->style_txt) / sizeof(char *))
		{
			css_keep->style_txt[css_keep->style_txt_num] = ptxt;
			memcpy(css_keep->css_url[css_keep->style_txt_num], css_url, strlen(css_url));
			css_keep->style_txt_num++;
		}
		else
		{
//			Warn("%s:too many style!", __FUNCTION__);
		}
		/**处理style的import进来的css，不考虑*/
//		collect_import_css_url(css_keep, ptxt, url);
	}
}


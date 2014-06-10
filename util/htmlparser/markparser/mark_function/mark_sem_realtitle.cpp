/**
 * mark_sem_realtitle.cpp
 * Description: 标记真实标题块
 *  Created on: 2011-11-23
 * Last modify: 2012-10-26 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#include "chinese.h"
#include "easou_url.h"
#include "easou_lang_relate.h"
#include "easou_mark_com.h"
#include "easou_mark_srctype.h"
#include "easou_mark_func.h"
#include "easou_mark_sem.h"
#include "easou_debug.h"

bool is_normal_link_color(const font_t *font);

static const int RT_CONT_ABSTRACT_LEN = 512; //正文摘要的长度

static const int RT_CONT_ABSTRACT_MIN_LEN = 128; //正文摘要的最短长度

static const int RT_MAX_LEN = 150; //
static const int RT_MIN_LEN = 4;

static const int MYPOS_MAX_LEN = 512;
static const int TITLE_MAX_LEN = 512;

typedef struct _rt_maincont_t
{
	html_area_t *root;
	char *cont;
	int size;
	html_area_t *main_cont_begin;
} rt_maincont_t;

/**
 * @brief	主要正文的密度是否足够高.如果该块的某一子块的文本站该块的80%，返回false
 **/
static bool is_dense(html_area_t *area)
{
	int upper_con = area->baseinfo->text_info.con_size - area->baseinfo->link_info.anchor_size;

	for (html_area_t *subarea = area->subArea; subarea; subarea = subarea->nextArea)
	{
		if (!subarea->isValid)
			continue;
		int my_con = subarea->baseinfo->text_info.con_size - subarea->baseinfo->link_info.anchor_size;
		if (my_con * 100 >= upper_con * 80)
			return false;
	}

	return true;
}

/**
 * 判断节点或该节点的后裔节点是否为TAG_MARQUEE、TAG_DL、TAG_LI节点；true：含有
 */
static bool vnode_has_unproper_tag(html_vnode_t *vnode)
{
	if (vnode->hpNode->html_tag.tag_type == TAG_MARQUEE || vnode->hpNode->html_tag.tag_type == TAG_DL //definition lists
	|| vnode->hpNode->html_tag.tag_type == TAG_LI)
		return true;

	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (child->isValid && child->subtree_textSize >= 4)
		{
			if (vnode_has_unproper_tag(child))
				return true;
		}
	}

	return false;
}

/**
 * @brief 提取正文块的起始块，只提取一个
 */
static int visit_for_rt_main_cont(html_area_t *area, void *data)
{
	/**
	 * 抽取主要正文的摘要，注重准确率.
	 **/
	rt_maincont_t *rt_mcont = (rt_maincont_t *) data;

	if (!area->isValid)
		return AREA_VISIT_SKIP;

	if (area->depth == 0)
	{
		return AREA_VISIT_NORMAL;
	}

	if (area->depth == 1 && area->pos_mark != RELA_MAIN)
	{
		return AREA_VISIT_SKIP;
	}

	html_area_t *root = rt_mcont->root;
	int per_con_size = area->baseinfo->text_info.con_size / (area->baseinfo->text_info.con_num + 1);

	if (area->baseinfo->text_info.con_size * 5 >= area->baseinfo->link_info.anchor_size * 100 && area->baseinfo->text_info.con_size >= RT_CONT_ABSTRACT_MIN_LEN && (area->area_info.ypos < root->area_info.height / 2 || area->baseinfo->text_info.con_size >= root->baseinfo->text_info.con_size / 2) && area->area_info.ypos < 1600 && is_dense(area) && area->area_info.height >= 50 && area->baseinfo->text_info.con_size >= root->baseinfo->text_info.con_size / 8 && per_con_size >= 24)
	{
		rt_mcont->main_cont_begin = area;
		extract_area_content(rt_mcont->cont, rt_mcont->size, area);
		return AREA_VISIT_FINISH;
	}

	return AREA_VISIT_NORMAL;
}

/**
 * @brief 获取主要正文摘要.
 **/
static html_area_t *rt_get_main_cont(char *cont, int size, area_tree_t *atree)
{
	cont[0] = '\0';
	rt_maincont_t rt_mcont;
	rt_mcont.root = atree->root;
	rt_mcont.cont = cont;
	rt_mcont.size = size;
	rt_mcont.main_cont_begin = NULL;
	areatree_visit(atree, visit_for_rt_main_cont, NULL, &rt_mcont);
	return rt_mcont.main_cont_begin;
}

static bool is_table_col(html_area_t *area)
{
	for (html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode)
	{
		if (vnode->isValid && (vnode->hpNode->html_tag.tag_type == TAG_TD || vnode->hpNode->html_tag.tag_type == TAG_TH))
		{
			return true;
		}
	}

	return false;
}

struct longest_text_visit_t
{
	char *buf;
	int buf_size;
	int text_len;
};

static int start_visit_for_longest_text(html_vnode_t *vnode, void *result)
{
	if (vnode->hpNode->html_tag.tag_type != TAG_PURETEXT)
	{
		return VISIT_NORMAL;
	}
	longest_text_visit_t *visit = (longest_text_visit_t*) result;
	if (vnode->isValid && vnode->textSize > visit->text_len)
	{
		copy_html_text(visit->buf, 0, visit->buf_size - 1, vnode->hpNode->html_tag.text);
		visit->text_len = vnode->textSize;
	}
	return VISIT_NORMAL;
}

/**
 * @brief 从分块中提取内容. 要求已完成分块标注.
 **/
static int extract_content_on_marked_area(char *cont, const int size, const html_area_t *area, bool isrealtile)
{
	bool hasH1Flag = false;
	html_vnode_t *target = NULL;
	if (isrealtile)
	{
		if (area->begin == area->end && area->valid_subArea_num == 0)
		{
			html_vnode_t *vnode = area->begin;
			while (vnode)
			{
				if (vnode->hpNode->html_tag.tag_type == TAG_H1)
				{
					hasH1Flag = true;
					target = vnode;
					break;
				}
				if (!vnode->firstChild || vnode->firstChild->nextNode != NULL)
				{
					break;
				}
				vnode = vnode->firstChild;
			}
		}
	}
	int avail = 0;
	cont[0] = '\0';
	int savedLength = 0;
	if (!isrealtile || !hasH1Flag)
	{
		vnode_list_t *vlist = area->baseinfo->text_info.cont_vnode_list_begin;
		for (; vlist; vlist = vlist->next)
		{
			html_vnode_t *vnode = vlist->vnode;
			if (vnode->isValid && vnode->textSize > 0)
			{
				if (isrealtile && is_func_area(area, AREA_FUNC_MYPOS))
				{
					avail = 0;
				}
				else if (isrealtile && hasH1Flag)
				{
					if (vnode->textSize > savedLength)
					{
						avail = 0;
						savedLength = vnode->textSize;
					}
					else
					{
						continue;
					}
				}
				avail = copy_html_text(cont, avail, size - 1, vnode->hpNode->html_tag.text);
				char *pcopypos = strrchr(cont, '>');
				if (pcopypos != NULL)
				{
					pcopypos++;
					char *pcopydest = cont;
					while (*pcopypos)
					{
						*pcopydest++ = *pcopypos++;
					}
					*pcopydest = 0;
				}
				if (avail >= size - 1)
				{
					break;
				}
			}
			if (vlist == area->baseinfo->text_info.cont_vnode_list_end)
				break;
		}
	}
	else
	{
		longest_text_visit_t longest_text_visit;
		longest_text_visit.buf = cont;
		longest_text_visit.buf_size = size;
		longest_text_visit.text_len = 0;
		html_vnode_visit(target, start_visit_for_longest_text, NULL, &longest_text_visit);
		avail = longest_text_visit.text_len;
	}
	return avail;
}

/**
 * @brief 是否表格的一行.
 **/
static bool is_table_row(html_area_t *area)
{
	if (area->area_info.height >= 60)
		return false;

	html_area_t *last = NULL;

	for (html_area_t *subarea = area->subArea; subarea; subarea = subarea->nextArea)
	{
		if (subarea->isValid && subarea->baseinfo->text_info.con_size > 0)
		{
			if (last != NULL)
			{
				if (subarea->area_info.ypos == last->area_info.ypos && is_table_col(subarea))
				{
					return true;
				}
				else
					return false;
			}
			last = subarea;
		}
	}

	return false;
}

/**
 * @brief 获取第一个小标题块.
 **/
static html_area_t *get_subtit_area(area_tree_t *atree)
{
	const area_list_t *subtit_area_list = get_func_mark_result(atree, AREA_FUNC_SUBTIT_4RT);
	if (subtit_area_list == NULL)
		return NULL;

	return subtit_area_list->head->area;
}

/**
 * @brief 获取页面上方的导航条.
 **/
static html_area_t *get_nav_area(area_tree_t *atree)
{
	const area_list_t *nav_area_list = get_func_mark_result(atree, AREA_FUNC_NAV);
	if (nav_area_list == NULL)
	{
		return NULL;
	}
	int page_height = atree->root->area_info.height;

	for (area_list_node_t *list_node = nav_area_list->head; list_node; list_node = list_node->next)
	{
		html_area_t *nav = list_node->area;
		if (nav->area_info.ypos < page_height / 3 && nav->area_info.width >= 500 && nav->area_info.width > nav->area_info.height * 2)
		{
			/* 位于上方的横条型导航条 */
			return nav;
		}
	}
	return NULL;
}

static const int MAX_CANDI_RTIT_NUM = 8;

typedef struct _rt_visitor_t
{
	area_tree_t *atree;
	html_area_t *cont_begin;
	html_area_t *mypos;
	html_area_t *nav;
	html_area_t *subtit;
	html_area_t *last_rtit;
	char *cont_abstract;
	char mypos_buf[MYPOS_MAX_LEN];
	int mypos_len;
	char tagtit_buf[TITLE_MAX_LEN];
	int tagtit_len;
	int tit_style_cnt;
	int curr_style_cnt;

	html_area_t *candi_rtit[MAX_CANDI_RTIT_NUM];
	int candi_weight[MAX_CANDI_RTIT_NUM];
	int candi_num;
	int max_candi_weight;
} rt_visitor_t;

/**
 * @brief 计算两个串的最大公共长度（字节）
 */
static int common_strlen(const char *stra, const char *strb)
{
	int cs = 0;
	char word[32];
	word[0] = '\0';

	for (const char *p = stra; *p;)
	{
		int l = GET_CHR_LEN(p);
		int wordlen = 0;
		if (l >= 2)
		{
			for (int i = 0; i < l; i++)
			{
				word[i] = p[i];
			}
			word[l] = '\0';
			wordlen = l;
		}
		else if (*p != ' ')
		{
			unsigned int i = 0;
			for (; p[i] && GET_CHR_LEN(p+i) == 1 && p[i] != ' ' && i < sizeof(word) - 1; i++)
			{
				word[i] = p[i];
			}
			word[i] = '\0';
			wordlen = i;
		}
		else
		{
			p++;
			continue;
		}

		if (!is_nonsense_word_use_in_realtitle(word))
		{
			if (strstr(p + wordlen, word) == NULL && strstr(strb, word) != NULL)
			{
				cs += wordlen;
			}
		}
		p += wordlen;
	}

	return cs;
}

static int weight_comp_with_tag_title(const char *candi_tit, int strl, const char *tag_tit, int strl_tt)
{
	int wi = 0;

	if (strstr(tag_tit, candi_tit) != NULL)
	{
		int lhalf = strl >= 32 ? 32 : strl;
		int rhalf = strl - lhalf;
		wi += lhalf * 2 - 4 + rhalf;
		return wi;
	}

	if (strl_tt >= 8)
	{
		int cs = common_strlen(candi_tit, tag_tit);
		if (strl <= 4)
			wi += strl > 0 ? (cs * 8 / strl) : 0;
		else
			wi += cs * 12 / strl;

		int lhalf = cs >= 32 ? 32 : cs;
		int rhalf = cs - lhalf;

		wi += lhalf / 2 + rhalf / 4;
	}

	return wi;
}

static int weight_comp_with_anchor(const char *candi_tit, int strl, const char *anchor)
{
	int wi = 0;
	int strl_tt = strlen(anchor);

	if (strstr(anchor, candi_tit) != NULL)
	{
		int lhalf = strl >= 32 ? 32 : strl;
		int rhalf = strl - lhalf;
		wi += lhalf - 4 + rhalf / 2;
		return wi;
	}

	if (strl_tt >= 8)
	{
		int cs = common_strlen(candi_tit, anchor);
		if (strl <= 4)
			wi += strl > 0 ? (cs * 6 / strl) : 0;
		else
			wi += cs * 9 / strl;

		int lhalf = cs >= 32 ? 32 : cs;
		int rhalf = cs - lhalf;

		wi += lhalf / 3 + rhalf / 6;
	}

	return wi;
}

static int weight_comp_with_mypos(const char *candi_tit, int strl, const char *mypos, int strl_mp)
{
	int wi = 0;

	const char *p = NULL;
	if ((p = strstr(mypos, candi_tit)) != NULL)
	{
		int pos_end = mypos + strl_mp - (p + strl);
		if (pos_end <= 6)
		{
			wi += strl * 2 - 6;
		}
		return wi;
	}

	if (strl_mp >= 8)
	{
		int cs = common_strlen(candi_tit, mypos);
		if (strl > 0)
			wi += cs * 12 / strl;

		int lhalf = cs >= 32 ? 32 : cs;
		int rhalf = cs - lhalf;

		wi += lhalf / 2 + rhalf / 4;
	}

	return wi;
}

static int get_url_pat_comp_weight(const char *url, const char *page_url, bool in_left, bool in_nav, bool is_header)
{
	char url_site[UL_MAX_SITE_LEN];
	char url_trunk[UL_MAX_SITE_LEN];
	char url_path[UL_MAX_PATH_LEN];
	char page_site[UL_MAX_SITE_LEN];
	char page_trunk[UL_MAX_SITE_LEN];
	char page_path[UL_MAX_PATH_LEN];
	url_site[0] = '\0';
	url_trunk[0] = '\0';
	url_path[0] = '\0';
	page_site[0] = '\0';
	page_trunk[0] = '\0';
	page_path[0] = '\0';

	easou_parse_url(url, url_site, NULL, url_path);
	easou_fetch_trunk(url_site, url_trunk, UL_MAX_SITE_LEN);
	easou_parse_url(page_url, page_site, NULL, page_path);
	easou_fetch_trunk(page_site, page_trunk, UL_MAX_SITE_LEN);
	int wi = 0;
	if (strcmp(url_site, page_site) != 0)
	{
		if (is_header && strcmp(url_trunk, page_trunk) == 0)
		{
			return 2;
		}
		wi -= 16;
	}

	if (strcasecmp(url_path, page_path) == 0)
	{
		wi += 22;
		if (in_left)
			wi += 10;
		return wi;
	}
	else if (in_left || in_nav)
	{
		wi -= 16;
		return wi;
	}

	const char *suffix_l = url_path;
	const char *suffix_r = page_path;
	const char *p_suffix_l = url_path;
	const char *p_suffix_r = page_path;

	while (*p_suffix_l && *p_suffix_r)
	{
		if (*p_suffix_l != *p_suffix_r)
			break;
		if (*p_suffix_l == '/')
		{
			suffix_l = p_suffix_l + 1;
			suffix_r = p_suffix_r + 1;
		}
		p_suffix_l++;
		p_suffix_r++;
	}

	int lcs = longest_common_substring(suffix_l, suffix_r);
	int lcs_value = 0;
	if (lcs > 0)
	{
		lcs_value = 32 * lcs / (strlen(suffix_l) + strlen(suffix_r));
	}
	if (lcs_value >= 6)
		wi = lcs_value - 8;
	else
	{
		wi = 5 * (lcs_value - 8) / 2;
	}

	int this_url_dep = get_url_depth(url);
	int pg_url_dep = get_url_depth(page_url);
	int delt_dep = abs(this_url_dep - pg_url_dep);
	if (delt_dep > 0 && lcs_value <= 10)
	{
		wi -= 11 * delt_dep;
	}

	return wi;
}

static bool is_a_link(html_area_t *area)
{
	if (area->baseinfo->link_info.num == 1 && area->baseinfo->link_info.anchor_size + 3 >= area->baseinfo->text_info.con_size && area->baseinfo->link_info.anchor_size * 10 >= area->baseinfo->text_info.con_size * 7)
	{
		return true;
	}
	return false;
}

static int url_pat_weight(html_area_t *area, mark_area_info_t *mark_info, bool in_left, bool in_nav)
{
	const char *page_url = mark_info->page_url;
	const char *base_href = mark_info->base_href;
	int wei = 0;

	if (!is_a_link(area))
	{
		if (area->baseinfo->link_info.num == 1)
		{
			//debuginfo(MARK_REALTITLE, "url_pat_weight return -5, cause area !is_a_link(area)&&link_info.num==1");
			return -5;
		}
		//debuginfo(MARK_REALTITLE, "url_pat_weight return %d, cause area !is_a_link(area)&&link_info.num==%d", -8 * area->baseinfo->link_info.num, area->baseinfo->link_info.num);
		return -8 * area->baseinfo->link_info.num;
	}

	char url[UL_MAX_URL_LEN];
	url[0] = '\0';

	for (vnode_list_t *list = area->baseinfo->link_info.url_vnode_list_begin; list; list = list->next)
	{
		const char *pclick = NULL;
		const char *href = NULL;
		html_vnode_t *vnode = list->vnode;
		if (vnode->wx < 60)
		{
			continue;
		}

		for (html_attribute_t *attr = vnode->hpNode->html_tag.attribute; attr; attr = attr->next)
		{
			if (attr->type == ATTR_ONCLICK)
			{
				pclick = attr->value;
				if (pclick != NULL)
				{
					wei -= 8;
					//debuginfo(MARK_REALTITLE, "url_pat_weight return %d, cause area has link with onclick property", wei);
					goto OUT;
				}
			}
			else if (attr->type == ATTR_HREF)
			{
				href = attr->value;
			}
		}

		if (href)
		{
			if (strchr(href, '#') != NULL)
			{
				wei -= 16;
				//debuginfo(MARK_REALTITLE, "url_pat_weight %d(-=16), cause area has link that its href contain '#'", wei);
			}
			else
			{
				bool is_header = false;
				html_area_t *child_area = area;
				while (child_area)
				{
					if (child_area->begin == child_area->end)
					{
						html_tag_type_t tag_type = child_area->begin->hpNode->html_tag.tag_type;
						if (tag_type == TAG_H1 || tag_type == TAG_H2)
						{
							is_header = true;
							break;
						}
						child_area = child_area->subArea;
					}
					else
					{
						break;
					}
				}
				easou_combine_url(url, base_href, href);
				wei += get_url_pat_comp_weight(url, page_url, in_left, in_nav, is_header);
				//debuginfo(MARK_REALTITLE, "url_pat_weight %d after get_url_pat_comp_weight(%s)", wei, url);
			}
		}

		if (list == area->baseinfo->link_info.url_vnode_list_end)
			break;
	}
	OUT: if (wei > 15)
	{
		wei = 15;
	}
	return wei;
}

static const int RT_MIN_WEIGHT = 15;

static int tit_font_weight(font_t *font[], int size, int left_con_size, int right_con_size, html_area_t *area)
{
	font_t *afont = font[0];
	font_t *bfont = font[size - 1];

	int font_w = 0;
	if (afont->header_size > 0 || bfont->header_size > 0)
	{
		int header_size = afont->header_size >= bfont->header_size ? afont->header_size : bfont->header_size;
		switch (header_size)
		{
		case 0:
			break;
		case 1:
			font_w += 15;
			break;
		case 2:
			font_w += 14;
			break;
		case 3:
			font_w += 13;
			break;
		case 4:
			font_w += 11;
			break;
		case 5:
			font_w += 10;
			break;
		case 6:
			font_w += 9;
			break;
		}
		//debuginfo(MARK_REALTITLE, "the score of the area font_w is %d  after afont->header_size", font_w);
	}

	if (afont->align == VHP_TEXT_ALIGN_CENTER && bfont->align == VHP_TEXT_ALIGN_CENTER && area->area_info.xpos + area->area_info.width >= 160)
	{
		if (left_con_size + right_con_size <= 8)
		{
			font_w += 11;
		}
		else if (left_con_size + right_con_size <= 20 && abs(left_con_size - right_con_size) <= 3)
		{
			font_w += 8;
		}
		//debuginfo(MARK_REALTITLE, "the score of the area font_w is %d after afont->align(left_con_size:%d, right_con_size:%d)", font_w, left_con_size, right_con_size);
	}

	if (afont->is_strong || bfont->is_strong)
	{
		font_w += 11;
		//debuginfo(MARK_REALTITLE, "the score of the area font_w is %d  after is_strong", font_w);
	}

	if (afont->is_bold || bfont->is_bold)
	{
		font_w += 10;
		//debuginfo(MARK_REALTITLE, "the score of the area font_w is %d  after is_bold", font_w);
	}

	if (afont->color != 0x0 && !is_gray_color(afont->color) && !is_normal_link_color(afont))
	{
		font_w += 6;
		//debuginfo(MARK_REALTITLE, "the score of the area font_w is %d  after color", font_w);
	}

	if (area->pos_plus == IN_PAGE_RIGHT || area->pos_plus == IN_PAGE_LEFT)
	{
		if (font_w > 5)
			font_w = 5;
	}
	else
	{
		if (font_w > 15)
			font_w = 15;
	}

	//debuginfo(MARK_REALTITLE, "the final score of the area font_w is %d", font_w);
	return font_w;
}

void add_candi_rtit(rt_visitor_t *visitor, html_area_t *area, int rt_weight)
{
	if (visitor->candi_num < MAX_CANDI_RTIT_NUM)
	{
		visitor->candi_rtit[visitor->candi_num] = area;
		visitor->candi_weight[visitor->candi_num] = rt_weight;
		visitor->candi_num++;
		if (rt_weight > visitor->max_candi_weight)
			visitor->max_candi_weight = rt_weight;
	}
}

static const int ATTR_HAS_TITLE_FEAT = 1;
static const int ATTR_HAS_CURR_FEAT = 2;

static int has_attr_feat(html_vnode_t *vnode)
{
	for (html_attribute_t *attr = vnode->hpNode->html_tag.attribute; attr; attr = attr->next)
	{
		if (attr->type == ATTR_CLASS || attr->type == ATTR_ID)
		{
			if (attr->value)
			{
				if (easou_strcasestr(attr->value, "title") != NULL)
				{
					return ATTR_HAS_TITLE_FEAT;
				}
				if (easou_strcasestr(attr->value, "curr") != NULL)
				{
					return ATTR_HAS_CURR_FEAT;
				}
			}
		}
	}

	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (child->isValid && child->subtree_textSize >= vnode->subtree_textSize - 4)
		{
			int ret = has_attr_feat(child);
			if (ret)
				return ret;
		}
	}

	return 0;
}

#define TITL_STYLE_CNT_VALID_LIMIT	1			  /**< 网页中class或id为titl的数量小于等于此值，这个特征才可以加权       */
#define CURR_STYLE_CNT_VALID_LIMIT	1		  /**< 网页中class或id为curr的数量小于等于此值，这个特征才可以加权       */

static int get_attr_feat_weight(html_area_t *area, rt_visitor_t *vst)
{
	int wi = 0;

	for (html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode)
	{
		int ret = 0;
		if (vnode->isValid && (ret = has_attr_feat(vnode)))
		{
			if (ret == ATTR_HAS_TITLE_FEAT && vst->tit_style_cnt <= TITL_STYLE_CNT_VALID_LIMIT)
				wi += 11;
			else if (ret == ATTR_HAS_CURR_FEAT && vst->curr_style_cnt <= CURR_STYLE_CNT_VALID_LIMIT)
				wi += 5;
			break;
		}

		if (vnode == area->end)
			break;
	}

	return wi;
}

static bool is_in_left(html_area_t *area)
{
	html_area_t *upper = area;
	while (upper)
	{
		if (upper->depth == 2)
		{
			if (upper->pos_mark == RELA_LEFT && upper->area_info.width < upper->area_info.height && upper->area_info.height >= 160 && upper->baseinfo->link_info.num >= 6 && get_right_area_in_page(area))
			{
				return true;
			}
		}
		else if (upper->depth == 1)
		{
			if ((upper->pos_mark == RELA_LEFT || (upper->area_info.xpos + upper->area_info.width <= 160 && upper->area_info.height >= upper->area_info.width * 2)) && get_right_area_in_page(area))
			{
				return true;
			}
			else
				return false;
		}
		upper = upper->parentArea;
	}

	return false;
}

static html_area_t *get_main_area_in(html_area_t *area)
{
	html_area_t *upper = area;
	while (upper)
	{
		if (upper->depth == 1)
		{
			if (upper->pos_mark == RELA_MAIN)
				return upper;
			else
				return NULL;
		}
		upper = upper->parentArea;
	}

	return NULL;
}

static int left_right_thing_weight(html_area_t *area)
{
	int wi = 0;

	int left_inter_num = 0;
	for (const html_area_t *left = get_left_area_inline(area); left; left = get_left_area_inline(left))
	{
		if (left->area_info.height <= area->area_info.height * 2)
		{
			left_inter_num += left->baseinfo->inter_info.input_num + left->baseinfo->inter_info.select_num + left->baseinfo->inter_info.option_num;
		}
		else
			break;
	}

	wi -= left_inter_num;

	int right_inter_num = 0;
	for (const html_area_t *right = get_right_area_inline(area); right; right = get_right_area_inline(right))
	{
		if (right->area_info.height <= area->area_info.height * 2)
		{
			right_inter_num += right->baseinfo->inter_info.input_num + right->baseinfo->inter_info.select_num + right->baseinfo->inter_info.option_num;
		}
		else
			break;
	}

	wi -= right_inter_num / 2;

	wi *= 3;

	return wi;
}

static int get_chr_num(const char *tit, const char c)
{
	const char *p = tit;
	int n = 0;

	while (*p)
	{
		p = strchr(p, c);
		if (p == NULL)
			break;
		n++;
		p++;
	}

	return n;
}

extern const char *realtit_plus_word[];

static int str_weight(const char *tit, int cur_wei, bool is_mypos)
{
	int wi = 0;

	for (int i = 0; realtit_plus_word[i]; i++)
	{
		const char *p = strstr(tit, realtit_plus_word[i]);
		if (p)
		{
			p += strlen(realtit_plus_word[i]);
			if (*p == ':' || *p == ' ')
				wi += 16;
		}
	}

	int colon_num = get_chr_num(tit, ':');

	if (colon_num >= 2)
	{
		wi -= colon_num * 9;
	}

	int comma_num = get_chr_num(tit, ',');

	if (comma_num < 3)
		wi -= 2 * comma_num;
	else
		wi -= 4 * comma_num;

	if (!is_mypos)
	{
		int bra_num = get_chr_num(tit, '>');
		wi -= 3 * bra_num;
	}

	int v_num = get_chr_num(tit, '|');

	wi -= 4 * v_num;

	if (strlen(tit) <= 8)
	{
		const char *p = strstr(tit, "相关");
		if (p)
		{
			if (p == tit || p[strlen("相关")] == '\0')
				wi -= 6;
		}
	}

	if (cur_wei >= RT_MIN_WEIGHT + 12)
	{
		const char *p = strstr(tit, "网");
		if (p != NULL && p[strlen("网")] == '\0')
			wi -= 6;
	}

	return wi;
}

static const char *end_char(const char *str)
{
	for (const char *p = str; *p;)
	{
		int l = GET_CHR_LEN(p);
		if (p[l] == '\0')
			return p;
		p += l;
	}

	return str;
}

/**
 * @brief 根据位置，与导航、mypos、subtitle、签名块的位置关系、与tagtitle的相似度、字体与背景字体差异计算得分
 */
static bool is_area_like_realtitle(html_area_t *area, area_tree_t *atree, rt_visitor_t *visitor, bool isHeader)
{
	const char *mypos = visitor->mypos_buf;
	const char *tag_tit = visitor->tagtit_buf;
	html_area_t *nav = visitor->nav;
	html_area_t *cont_begin = visitor->cont_begin;
	int height = area->area_info.height;
	int width = area->area_info.width;
	if (height <= 25 && width <= 25)
	{
		debuginfo(MARK_REALTITLE, "the area(id=%d) is not for its height<=25 and width<=25", area->no);
		return false;
	}

	int rt_weight = 0;

	bool in_left = is_in_left(area);
	bool in_nav = is_in_func_area(area, AREA_FUNC_NAV);
	bool is_mypos = is_func_area(area, AREA_FUNC_MYPOS);

	if ((!is_a_link(area) || area->baseinfo->text_info.con_size >= 12) && in_left)
	{
		/*在左边，非导航链接*/
		debuginfo(MARK_REALTITLE, "the area(id=%d) is not for it is at left and (not a link or content size > 12)", area->no);
		return false;
	}

	if (is_table_row(area))
	{
		debuginfo(MARK_REALTITLE, "the area(id=%d) is not for it is one row of table", area->no);
		return false;
	}

	char tit_buf[RT_MAX_LEN];
	tit_buf[0] = '\0';
	if (is_mypos)
	{
		copy_html_text(tit_buf, 0, RT_MAX_LEN - 1, area->baseinfo->text_info.cont_vnode_list_end->vnode->hpNode->html_tag.text);
		char *pcopypos = strrchr(tit_buf, '>');
		if (pcopypos != NULL)
		{
			pcopypos++;
			char *pcopydest = tit_buf;
			while (*pcopypos)
			{
				*pcopydest++ = *pcopypos++;
			}
			*pcopydest = 0;
		}
	}
	else
	{
		extract_content_on_marked_area(tit_buf, sizeof(tit_buf), area, true);
	}
	debuginfo(MARK_REALTITLE, "tit_buf:%s", tit_buf);

	trans2bj_lower(tit_buf, tit_buf);
	easou_trim_space(tit_buf);

	if (tit_buf[0] == '\0' || tit_buf[1] == '\0' || tit_buf[2] == '\0')
	{
		debuginfo(MARK_REALTITLE, "the content of the area(id=%d) is too small", area->no);
		return false;
	}

	int tit_len = strlen(tit_buf);

	if (nav)
	{
		if (area->area_info.ypos + area->area_info.height <= nav->area_info.ypos)
		{
			rt_weight -= 2;
			if (cont_begin && cont_begin->baseinfo->text_info.con_size >= 300)
			{
				rt_weight -= 24;
			}
		}
		else if (area->area_info.ypos + area->area_info.height <= nav->area_info.ypos + nav->area_info.height && (area->area_info.xpos < nav->area_info.xpos || area->area_info.xpos >= nav->area_info.xpos + nav->area_info.width))
		{
			rt_weight -= 20;
		}
	}
	debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after nav", area->no, rt_weight);

	if (visitor->tit_style_cnt <= TITL_STYLE_CNT_VALID_LIMIT || visitor->curr_style_cnt <= CURR_STYLE_CNT_VALID_LIMIT)
	{
		rt_weight += get_attr_feat_weight(area, visitor);
	}
	debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after title attr", area->no, rt_weight);

	/*
	 if(area->baseinfo->link_info.other_num >= 1&&!is_func_area(area,AREA_FUNC_MYPOS))
	 rt_weight -= 5;
	 debuginfo(MARK_REALTITLE,"the score of the area is %d link num",rt_weight,__FILE__,__LINE__,__FUNCTION__);
	 */

	if (!is_func_area(area, AREA_FUNC_MYPOS) && !is_func_area(area, AREA_FUNC_NAV) && !is_in_func_area(area, AREA_FUNC_NAV))
	{
		rt_weight += url_pat_weight(area, atree->mark_info, in_left, in_nav);
	}
	debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after url", area->no, rt_weight);

	rt_weight += weight_comp_with_tag_title(tit_buf, tit_len, tag_tit, visitor->tagtit_len);
	debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after tag_title", area->no, rt_weight);

	/*
	 if (visitor->cont_begin)
	 {
	 rt_weight += weight_comp_with_cont_abstract(area, tit_buf, tit_len, visitor->cont_begin,
	 visitor->cont_abstract);
	 }
	 debuginfo(MARK_REALTITLE,"the score of the area is %d after compare with abstract",rt_weight, __FILE__,__LINE__,__FUNCTION__);
	 */

	if (strlen(atree->mark_info->anchor) >= 12)
	{
		rt_weight += weight_comp_with_anchor(tit_buf, tit_len, atree->mark_info->anchor);
	}

	rt_weight += left_right_thing_weight(area);
	debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after left_right_thing ", area->no, rt_weight);

	if (!is_contain_func_area(area, AREA_FUNC_MYPOS))
	{
		if (visitor->mypos && area->area_info.ypos >= visitor->mypos->area_info.ypos && area->area_info.ypos < visitor->mypos->area_info.ypos + visitor->mypos->area_info.height && area->area_info.xpos >= visitor->mypos->area_info.xpos && area->area_info.xpos < visitor->mypos->area_info.xpos + visitor->mypos->area_info.width)
		{ //在mypos内部
		}
		else
		{
			rt_weight += weight_comp_with_mypos(tit_buf, tit_len, mypos, visitor->mypos_len);
			debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after %s compare with mypos(%s)", area->no, rt_weight, tit_buf, mypos);
		}
	}

	int left_con_size = get_left_leaf_cont_size(area);
	int right_con_size = get_right_leaf_con_size(area);

	rt_weight += 2 - left_con_size / 4;
	debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after left text", area->no, rt_weight);

	rt_weight += 2 - right_con_size / 6;
	debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after right text", area->no, rt_weight);

	const char *pend = end_char(tit_buf);
	if (*pend == ':')
	{
		rt_weight -= 3;
		if (right_con_size >= 4)
			rt_weight -= 14;
	}
	debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after last char ", area->no, rt_weight);

	font_t *font[2];
	unsigned int font_num = 0;
	bool is_outstand = check_area_font_outstand(area, &atree->hp_vtree->body->font, font, sizeof(font) / sizeof(font_t *), font_num);
	for (int i = 0; i < font_num ; i++)
	{
		//debuginfo(MARK_REALTITLE, "font{header_size:%d,is_bold:%d,is_strong:%d,align:%d,color:%d,size:%d,line-height:%d}", font[i]->header_size, font[i]->is_bold, font[i]->is_strong, font[i]->align, font[i]->color, font[i]->size, font[i]->line_height);
	}
	if (area == visitor->mypos)
	{
		is_outstand = false;
	}

	if (is_outstand)
	{
		if (area->depth <= 2)
			rt_weight += 7;
		else
			rt_weight += 6;
		if (font_num >= 1)
			rt_weight += tit_font_weight(font, font_num, left_con_size, right_con_size, area);
	}
	debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after font outstand", area->no, rt_weight);

	if (!isHeader)
	{
		if (tit_len >= 48)
		{
			rt_weight -= (tit_len - 42) / 6;
		}
		else if (tit_len <= 4)
		{
			if (strstr(tag_tit, tit_buf) == NULL)
				rt_weight -= 8;
		}
		debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after title_len", area->no, rt_weight);
	}

	rt_weight += str_weight(tit_buf, rt_weight, is_mypos);
	if (visitor->mypos && area->area_info.ypos == visitor->mypos->area_info.ypos && area->area_info.xpos >= visitor->mypos->area_info.xpos && area->area_info.xpos < visitor->mypos->area_info.xpos + visitor->mypos->area_info.width && rt_weight >= RT_MIN_WEIGHT + 16)
	{
		const char *p = strstr(mypos, tit_buf);
		if (p && !is_func_area(area, AREA_FUNC_MYPOS))
		{
			int pos_end = mypos + strlen(mypos) - (p + tit_len);
			rt_weight -= 4 + pos_end / 2;
		}
	}
	debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after mypos end char", area->no, rt_weight);

	html_area_t *main_in = get_main_area_in(area);
	if (main_in && area->area_info.xpos - main_in->area_info.xpos >= main_in->area_info.width / 2)
	{
		if (main_in->area_info.width > 0)
		{
			rt_weight -= ((area->area_info.xpos - main_in->area_info.xpos) * 5 / main_in->area_info.width);
			debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after compare xpos(%d) with main_in area(%d)", area->no, rt_weight, area->area_info.xpos, main_in->area_info.xpos);
		}
	}

	if (visitor->cont_begin != NULL && visitor->cont_begin->area_info.ypos < 500)
	{
		int keep_wei = rt_weight;

		int yspan = visitor->cont_begin->area_info.ypos - 45 - (area->area_info.ypos + area->area_info.height);
		int abs_yspan = abs(yspan);
		rt_weight -= abs_yspan / 64;

		if (visitor->cont_begin->baseinfo->text_info.con_size >= 200 && yspan > 0)
		{
			rt_weight -= yspan / 12;
		}
		debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after compare bottom pos(%d) with cont_begin area ypos(%d)", area->no, rt_weight, area->area_info.ypos + area->area_info.height, visitor->cont_begin->area_info.ypos);

		if (keep_wei >= RT_MIN_WEIGHT && rt_weight < RT_MIN_WEIGHT && visitor->last_rtit == NULL)
			rt_weight = RT_MIN_WEIGHT;
	}
	debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after content area text size", area->no, rt_weight);

	if (visitor->mypos != NULL)
	{
		int keep_wei = rt_weight;

		if ((area->area_info.ypos > visitor->mypos->area_info.ypos))
		{
			int yspan = area->area_info.ypos - 120 - visitor->mypos->area_info.ypos;
			int mypos_pos_val = -yspan / 16;
			//debuginfo(MARK_REALTITLE, "mypos_pos_val is %d after mypos", mypos_pos_val);
			if (mypos_pos_val < -16)
				mypos_pos_val = -16;
			rt_weight += mypos_pos_val;
		}

		if (keep_wei >= RT_MIN_WEIGHT && rt_weight < RT_MIN_WEIGHT && visitor->last_rtit == NULL)
			rt_weight = RT_MIN_WEIGHT;

		if ((area->area_info.ypos < visitor->mypos->area_info.ypos) && (area->area_tree->hp_vtree->hpTree->doctype > 1 && (area->area_tree->hp_vtree->hpTree->root.subnodetype & 14)))
		{
			rt_weight -= (visitor->mypos->area_info.ypos - area->area_info.ypos) / 40;
			if (visitor->cont_begin && visitor->cont_begin->baseinfo->text_info.con_size >= 300)
			{
				rt_weight -= 14;
			}
		}
	}
	debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after mypos", area->no, rt_weight);
	if (area->area_info.ypos >= atree->root->area_info.height / 3 && atree->root->area_info.height >= 500)
	{
		int keep_wei = rt_weight;
		rt_weight -= (area->area_info.ypos - atree->root->area_info.height / 3) / 28;
		if (keep_wei >= RT_MIN_WEIGHT && rt_weight < RT_MIN_WEIGHT && visitor->last_rtit == NULL)
			rt_weight = RT_MIN_WEIGHT;
	}
	debuginfo(MARK_REALTITLE, "the score of the area(id=%d) is %d after page height", area->no, rt_weight);
	if ((((area->begin->hpNode->owner->treeAttr & 1) == 0) && (area->baseinfo->text_info.text_area > area->baseinfo->link_info.link_area)) || (area->area_tree->hp_vtree->hpTree->doctype == 2 && (area->area_tree->hp_vtree->hpTree->root.subnodetype & 14) == 0))
	{
		if (visitor->candi_num == 0)
		{
			rt_weight = rt_weight + 13;
			debuginfo(MARK_REALTITLE, "the area(id=%d) is first area who is found and fit,weight+13, doctype:%d", area->no, area->area_tree->hp_vtree->hpTree->doctype);
		}
	}
	if (visitor->mypos != NULL && area == visitor->mypos)
	{
		if (rt_weight > 30)
		{
			rt_weight -= 5;
			if (rt_weight > 30)
				rt_weight = 30;
			debuginfo(MARK_REALTITLE, "the area(id=%d) score is %d for it is mypos", area->no, rt_weight);
		}
	}
	if (rt_weight >= RT_MIN_WEIGHT)
	{
		debuginfo(MARK_REALTITLE, "the area(id=%d) added for candidate, tree attr=%x\n", area->no, area->begin->hpNode->owner->treeAttr);
		add_candi_rtit(visitor, area, rt_weight);
		return true;
	}
	else
	{
		debuginfo(MARK_REALTITLE, "the area(id=%d) is not added for its score=%d(<%d)", area->no, rt_weight, RT_MIN_WEIGHT);
	}

	return false;
}

static bool is_near_page_sider(html_area_t *area)
{
	html_area_t *next = get_next_content_area(area);
	if (next != NULL)
	{
		if (is_func_area(next, AREA_FUNC_RELATE_LINK) || is_func_area(next, AREA_FUNC_FRIEND) || is_in_srctype_area(next, AREA_SRCTYPE_OUT)) //认为靠近外部块的地方不能为realtitle
			return true;
	}

	return false;
}

/**
 * @brief 是否大段链接列表。
 **/
static bool is_link_list(html_area_t *area)
{
	if (area->baseinfo->link_info.num < 8 || area->valid_subArea_num < 8 || area->baseinfo->link_info.anchor_size * 10 < area->baseinfo->text_info.con_size * 6)
	{
		return false;
	}

	int item_num = 0;

	for (html_area_t *subarea = area->subArea; subarea; subarea = subarea->nextArea)
	{
		if (!subarea->isValid)
			continue;
		if (subarea->area_info.width >= 10 && subarea->area_info.height >= 50)
			return false;
		if (subarea->baseinfo->link_info.num == 0)
		{
			return false;
		}
		if (subarea->area_info.height >= 5 && subarea->area_info.width >= 10)
		{
			item_num++;
		}
	}

	return (item_num >= 8);
}

/**
 * 标注所有可能的realtitle
 */
static int visit_for_realtitle(html_area_t *area, rt_visitor_t *visitor)
{
	area_tree_t *atree = visitor->atree;
	html_area_t *cont_begin = visitor->cont_begin;
	html_area_t *subtit = visitor->subtit;
	if (!area->isValid)
	{
		debuginfo(MARK_REALTITLE, "skip invalid area(id=%d)", area->no);
		return AREA_VISIT_SKIP;
	}
	int position = area->pos_plus;
	if ((position == IN_PAGE_LEFT && get_right_area_in_page(area)) || (position == IN_PAGE_RIGHT && get_left_area_in_page(area)) || (position == IN_PAGE_FOOTER))
	{
		debuginfo(MARK_REALTITLE, "skip area(id=%d) for it is at right or left or foot of the page", area->no);
		return AREA_VISIT_SKIP;
	}
	// shuangwei add 20120511
	if (!IS_LEAF_AREA(area->areaattr))
	{
		debuginfo(MARK_REALTITLE, "check into area(id=%d) for it is not leaf area", area->no);
		return AREA_VISIT_NORMAL;
	}

	if (is_in_func_area(area, AREA_FUNC_RELATE_LINK) || is_in_func_area(area, AREA_FUNC_FRIEND) || is_in_func_area(area, AREA_FUNC_SUBTIT_4RT) //todo
//			|| is_in_func_area(area,AREA_FUNC_CANDI_SUBTIT_4RT)
			|| is_in_srctype_area(area, AREA_SRCTYPE_OUT) || is_in_srctype_area(area, AREA_SRCTYPE_INTERACTION))
	{
		debuginfo(MARK_REALTITLE, "skip area(id=%d) for it is in relate link or friend link or src_out or interaction area", area->no);
		return AREA_VISIT_SKIP;
	}

	if (is_link_list(area))
	{
		debuginfo(MARK_REALTITLE, "skip area(id=%d) for it is link list", area->no);
		return AREA_VISIT_SKIP;
	}

	if (area->depth == 1)
	{
		if (area->pos_mark == RELA_RIGHT)
		{
			debuginfo(MARK_REALTITLE, "skip area(id=%d) for its depth is 1 and pos_mark=rela_right", area->no);
			return AREA_VISIT_SKIP;
		}
		if (area->pos_mark == RELA_FOOTER)
		{
			debuginfo(MARK_REALTITLE, "skip area(id=%d) for its depth is 1 and pos_mark=RELA_FOOTER", area->no);
			return AREA_VISIT_SKIP;
		}
	}

	{
		if (area->area_info.ypos * 3 > atree->root->area_info.height * 2 && atree->root->area_info.height >= 800)
		{
			debuginfo(MARK_REALTITLE, "skip area(id=%d) for its ypos > the height of page *2/3", area->no);
			return AREA_VISIT_SKIP;
		}
		if (area->area_info.ypos >= 1000 && area->area_info.ypos * 2 > atree->root->area_info.height * 1)
		{
			debuginfo(MARK_REALTITLE, "skip area(id=%d) for its ypos > 1000", area->no);
			return AREA_VISIT_SKIP;
		}
	}

	if (area->depth <= 2 && area->area_info.xpos >= 600 && area->area_info.width <= 400 && area->baseinfo->text_info.con_size * 70 <= area->baseinfo->link_info.anchor_size * 100 && area->baseinfo->link_info.num >= 8)
	{
		/**
		 * 靠右的边边框链接.
		 */
		debuginfo(MARK_REALTITLE, "skip area(id=%d) for its depth <=2 and xpos>=600 and width<=400 and text size < anchor size *100/70 and link num >8", area->no);
		return AREA_VISIT_SKIP;
	}

	if (is_contain_func_area(area, AREA_FUNC_RELATE_LINK) || is_contain_func_area(area, AREA_FUNC_NAV) || is_contain_func_area(area, AREA_FUNC_FRIEND)
	//|| is_contain_func_area(area,AREA_FUNC_MYPOS)
			|| is_contain_func_area(area, AREA_FUNC_ARTICLE_SIGN) //todo
//			|| is_contain_func_area(area,AREA_FUNC_CANDI_SUBTIT_4RT)
			|| is_contain_srctype_area(area, AREA_SRCTYPE_OUT) || is_contain_srctype_area(area, AREA_SRCTYPE_INTERACTION) || is_contain_srctype_area(area, AREA_SRCTYPE_PIC))
	{
		debuginfo(MARK_REALTITLE, "check into area(id=%d) for it is func link,nav friend,mypos,article_sign,src_out interaction or pic", area->no);
		return AREA_VISIT_NORMAL;
	}

	if (subtit != NULL)
	{
		if (area->area_info.ypos >= subtit->area_info.ypos)
		{
			if (area->area_info.xpos > subtit->area_info.xpos + subtit->area_info.width || area->area_info.xpos + area->area_info.width <= subtit->area_info.xpos)
			{
				;
			}
			else
			{
				if ((area->nodeTypeOfArea & 4) == 0)
				{
					debuginfo(MARK_REALTITLE, "the area is not realtitle for its ypos > subtitle.ypos");
					return AREA_VISIT_SKIP;
				}
			}
		}
	}

	bool isHeader = false;
	int rt_len_limit = RT_MAX_LEN;
	int area_height_limit = 200;
	html_area_t *upper_area = area;
	while (upper_area && upper_area->begin == upper_area->end)
	{
		html_tag_type_t tag_type = upper_area->begin->hpNode->html_tag.tag_type;
		if (tag_type == TAG_H1)
		{
			rt_len_limit = 250;
			area_height_limit = 250;
			isHeader = true;
			break;
		}
		upper_area = upper_area->parentArea;
	}
	if ((area->area_info.height >= area_height_limit || area->baseinfo->text_info.con_size > rt_len_limit || area->baseinfo->link_info.num >= 4
//			|| (area->baseinfo->link_info.num >= 2 && area->area_info.height >= 45)
			|| area->baseinfo->link_info.other_num >= 2) && !is_func_area(area, AREA_FUNC_MYPOS))
	{
		debuginfo(MARK_REALTITLE, "check into area(id=%d) for its height(%d)>=%d or link_num(%d)>=4,link_other(%d)>=2 or content size(%d)>%d", area->no, area->area_info.height, area_height_limit, area->baseinfo->link_info.num, area->baseinfo->link_info.other_num, area->baseinfo->text_info.con_size, rt_len_limit);
		return AREA_VISIT_NORMAL;
	}

	if (area->baseinfo->text_info.con_size < 4)
	{
		debuginfo(MARK_REALTITLE, "skip area(id=%d) for its content size(%d)<4", area->no, area->baseinfo->text_info.con_size);
		return AREA_VISIT_SKIP;
	}

	if (is_near_page_sider(area))
	{
		debuginfo(MARK_REALTITLE, "skip area(id=%d) for it is near the related link or friend area ", area->no);
		return AREA_VISIT_SKIP;
	}

	/*
	 if (has_unproper_tag(area))
	 {
	 debuginfo(MARK_REALTITLE,
	 "the area is not realtitle for it contains the marquee li or dl tag area ",
	 __FILE__, __LINE__, __FUNCTION__);
	 return AREA_VISIT_NORMAL;
	 }
	 */

	/*
	 if (!is_text_in_line(area))
	 {
	 debuginfo(MARK_REALTITLE,"the area is not realtitle for the nodes of the area is not in one line",__FILE__,__LINE__,__FUNCTION__);
	 return AREA_VISIT_NORMAL;
	 }
	 */

	if (is_area_like_realtitle(area, atree, visitor, isHeader))
	{
		visitor->last_rtit = area;

		if ((area->baseinfo->link_info.num == 0 || is_a_link(area)))
			return AREA_VISIT_SKIP;
	}

	return AREA_VISIT_NORMAL;
}

static int visit_fot_style_cnt(html_vnode_t *vnode, void *data)
{
	if (!vnode->isValid || vnode->subtree_textSize < RT_MIN_LEN)
		return VISIT_SKIP_CHILD;

	rt_visitor_t *vst = (rt_visitor_t *) data;

	if (vnode->subtree_textSize <= RT_MAX_LEN)
	{
		for (html_attribute_t *attr = vnode->hpNode->html_tag.attribute; attr; attr = attr->next)
		{
			if (attr->type == ATTR_CLASS || attr->type == ATTR_ID)
			{
				if (attr->value)
				{
					if (easou_strcasestr(attr->value, "title") != NULL)
					{
						vst->tit_style_cnt++;
					}
					else if (easou_strcasestr(attr->value, "curr") != NULL)
					{
						vst->curr_style_cnt++;
					}
				}
			}
		}
	}

	return VISIT_NORMAL;
}

static void filter_mypos(char *mypos_buf)
{
	for (int i = 0; mypos_filter_words[i]; i++)
	{
		char *p = strstr(mypos_buf, mypos_filter_words[i]);
		if (p)
			*p = '\0';
	}
}

/**
 * 判断网页是否是网站主页，true：主页
 */
bool IsHomePage(const char* url, const int urlLen)
{
	const char *begin = NULL;
	const char *slashpos = NULL;

	assert(url != NULL);
	begin = url;
	if (strncmp(begin, "http://", 7) == 0)
	{
		begin += 7;
	}
	// some  not invaild url is not suited
	if ((slashpos = strchr(begin, '/')) == NULL)
	{
		return true;
	}
	else
	{
		if ((*(slashpos + 1)) == 0)
		{
			return true;
		}
		else if (strncmp(slashpos + 1, "index", 5) == 0 || strncmp(slashpos + 1, "main", 4) == 0 || strncmp(slashpos + 1, "default", 7) == 0)
		{

			if (strchr(slashpos + 1, '?') == NULL && strchr(slashpos + 1, '/') == NULL)
				return true;
		}
	}
	return false;
}

bool mark_sem_realtitle(area_tree_t *atree)
{
	if (!atree || !(atree->mark_info))
	{
		return false;
	}
	char *page_url = atree->mark_info->page_url;
	int urllen = 0;
	if (page_url)
	{
		urllen = strlen(page_url);
		if (IsHomePage(page_url, urllen))
		{
			return false;
		}
	}

	debuginfo_on(MARK_REALTITLE);
	timeinit();
	timestart();

	char cont[RT_CONT_ABSTRACT_LEN];
	html_area_t *cont_begin = rt_get_main_cont(cont, sizeof(cont), atree); //得到第一个内容块
	trans2bj_lower(cont, cont);
	easou_trim_space(cont);

	html_area_t *mypos_area = get_first_mypos_area(atree);
	html_area_t *nav_area = get_nav_area(atree);
	html_area_t *subtit_area = get_subtit_area(atree);

	rt_visitor_t visitor;
	memset(&visitor, 0, sizeof(rt_visitor_t));
	visitor.atree = atree;
	visitor.cont_begin = cont_begin;
	visitor.mypos = mypos_area;
	visitor.nav = nav_area;
	visitor.subtit = subtit_area;
	visitor.cont_abstract = cont;
	visitor.candi_num = 0;

	html_vtree_visit((html_vtree_t *) atree->hp_vtree, visit_fot_style_cnt, NULL, &visitor);

	if (mypos_area && mypos_area->baseinfo)
	{
		extract_area_content(visitor.mypos_buf, sizeof(visitor.mypos_buf), mypos_area);
		trans2bj_lower(visitor.mypos_buf, visitor.mypos_buf);
		easou_trim_space(visitor.mypos_buf);
		filter_mypos(visitor.mypos_buf);
		visitor.mypos_len = strlen(visitor.mypos_buf);
	}
	trans2bj_lower(atree->mark_info->tag_title, visitor.tagtit_buf);
	easou_trim_space(visitor.tagtit_buf);
	visitor.tagtit_len = strlen(visitor.tagtit_buf);

	areatree_visit(atree, (FUNC_START_T) visit_for_realtitle, NULL, &visitor);
	for (int i = 0; i < visitor.candi_num; i++)
	{
		if (visitor.candi_weight[i] == visitor.max_candi_weight)
		{
			tag_area_sem(visitor.candi_rtit[i], AREA_SEM_REALTITLE);
		}
	}

	timeend("markparser", "realtitle");
	dumpdebug(MARK_REALTITLE, MARK_REALTITLE);
	return true;
}


/**
 * easou_func_subtitforrt.cpp
 * Description: 标记文章的副标题
 *  Created on: 2011-11-24
 * Last modify: 2012-10-26 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#include "easou_mark_baseinfo.h"
#include "easou_mark_com.h"
#include "easou_mark_srctype.h"
#include "easou_mark_func.h"
#include "debuginfo.h"

static const int MAX_SUBTIT_COLLECT_NUM = 16;
static const int SUBTIT_NUM_LIMIT = 16;

typedef struct _subtit_feat_t
{
	font_t *font;
	int xpos;
	int path_sign;
	int repeat_num;
	int last_ypos;
	int last_height;
	int min_hxspan;
	int max_hxspan;
	html_area_t *area[SUBTIT_NUM_LIMIT];
	char cont[SUBTIT_NUM_LIMIT][256];
} subtit_feat_t;

typedef struct _subtit_collector_t
{
	char tag_title[MAX_TITLE_LEN];
	subtit_feat_t subtit_feat[MAX_SUBTIT_COLLECT_NUM];
	int feat_num;
} subtit_collector_t;

static void subtit_collector_clean(subtit_collector_t *collect)
{
	collect->tag_title[0] = '\0';
	collect->feat_num = 0;
}

static bool is_unproper_tag(html_tag_type_t tag_type)
{
	if (tag_type == TAG_CODE)
		return true;
	return false;
}

/**
 * @brief 块是否含有code节点
 */
static bool is_in_unproper_tag(html_area_t *area)
{
	for (html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode)
	{
		if (vnode->isValid)
		{
			if (is_unproper_tag(vnode->hpNode->html_tag.tag_type))
				return true;
		}
		if (vnode == area->end)
			break;
	}

	int stat = 0;
	html_vnode_t *up = area->begin->upperNode;
	int level_wx = area->area_info.width;
	int level_hx = area->area_info.height;
	while (up)
	{
		if (is_unproper_tag(up->hpNode->html_tag.tag_type))
			return true;
		if (up->wx > level_wx || up->hx > level_hx)
		{
			stat++;
			if (stat == 2)
				break;
			level_wx = up->wx;
			level_hx = up->hx;
		}
		up = up->upperNode;
	}
	return false;
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

static bool is_to_skip_subtree(html_area_t *area)
{
	int use_con_size = area->baseinfo->text_info.con_size - area->baseinfo->text_info.no_use_con_size;
	if (use_con_size < 4)
	{
		marktreeprintfs(MARK_SUBTITFORRT, "the area(id=%d) is skip ,and is not subtitforrt for its use_con_size<4 at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}
	if (area->depth == 1 || (area->depth == 2 && area->area_info.height >= 200))
	{
		if (area->pos_mark == RELA_LEFT || area->pos_mark == RELA_RIGHT)
		{
			marktreeprintfs(MARK_SUBTITFORRT, "the area(id=%d) is to skip and not subtitforrt for it is at left or right,depth=%d,pos_mark=%d at %s(%d)-%s\r\n", area->no, area->depth, area->pos_mark, __FILE__, __LINE__, __FUNCTION__);
			return true;
		}
	}
	if (area->area_info.width <= 250 && area->area_info.height >= area->area_info.width)
	{
		//竖条
		marktreeprintfs(MARK_SUBTITFORRT, "the area(id=%d) is to skip and not subtitforrt ,for its height >= weight at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}
	if (is_in_func_area(area, AREA_FUNC_MYPOS) || is_in_func_area(area, AREA_FUNC_NAV) || is_in_func_area(area, AREA_FUNC_RELATE_LINK) || is_in_func_area(area, AREA_FUNC_FRIEND) || is_in_func_area(area, AREA_FUNC_ARTICLE_SIGN) || is_in_srctype_area(area, AREA_SRCTYPE_OUT)
			|| is_in_srctype_area(area, AREA_SRCTYPE_INTERACTION) || (is_in_srctype_area(area, AREA_SRCTYPE_PIC) && area->baseinfo->pic_info.pic_num >= 4))
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) is to skip,for is is in func or src ,func=%x,src=%x at %s(%d)-%s\r\n", area->no, area->upper_func_mark._mark_bits, area->upper_srctype_mark._mark_bits, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}
	if (is_in_unproper_tag(area))
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) is to skip for it contains code node at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}
	if (is_table_row(area))
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) is to skip for it is one row of a table at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}
	return false;
}

/**
 * @brief 该块是否含有DT节点且文字都在DT节点内；true：含有DT节点且文字都在DT节点内
 */
static bool is_dt_tag_area(html_area_t *area)
{
	for (html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode)
	{
		if (vnode->isValid)
		{
			// definition term
			if (vnode->hpNode->html_tag.tag_type == TAG_DT && vnode->subtree_textSize == area->baseinfo->text_info.con_size)
				return true;
			if (vnode->subtree_textSize > 0)
				return false;
		}
		if (vnode == area->end)
			break;
	}
	return false;
}

static bool is_to_skip_normal(html_area_t *area)
{
	if (!is_dt_tag_area(area))
	{
		if (area->baseinfo->text_info.con_size >= 100)
		{
			marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) is not in DT node,content size (%d)>=100,it is not subtitforrt at %s(%d)-%s\r\n", area->no, area->baseinfo->text_info.con_size, __FILE__, __LINE__, __FUNCTION__);
			return true;
		}
	}
	else
	{
		if (area->baseinfo->text_info.con_size >= 256)
		{
			marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) is in DT node,content size (%d)>=256 ,it is not subtitforrt at %s(%d)-%s\r\n", area->no, area->baseinfo->text_info.con_size, __FILE__, __LINE__, __FUNCTION__);
			return true;
		}
	}
	if (area->area_info.height >= 80)
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) height (%d)>=80 ,it is not subtitforrt at %s(%d)-%s\r\n", area->no, area->area_info.height, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}
	if (!is_text_in_line(area))
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the text of the area(id=%d) is not in one line,it is not subtitforrt at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}
	return false;
}

/**
 * @brief 计算块的符合，用于区分不同块
 */
static int get_path_sign(html_area_t *area)
{
	int path_sign = 0;
	html_vnode_t *vnode = area->begin;
	while (vnode)
	{
		html_tag_type_t tag_type = vnode->hpNode->html_tag.tag_type;
		if (tag_type != TAG_FORM)
			path_sign += tag_type;
		vnode = vnode->upperNode;
	}
	return path_sign;
}

/**
 * @brief 判断块与已存储的块是否相同；true：xpos ,font,path is same
 */
static bool is_same_feat(subtit_feat_t *a, html_area_t *area, font_t *font)
{
	if (a->xpos != area->area_info.xpos)
		return false;

	if (!is_same_font(a->font, font))
		return false;

	int path_sign = get_path_sign(area);

	if (a->path_sign != path_sign)
		return false;

	return true;
}

static bool is_same_pat(const html_area_t *a, const html_area_t *b)
{
	char aBuf[128];
	char bBuf[128];
	aBuf[0] = '\0';
	bBuf[0] = '\0';

	extract_area_content(aBuf, sizeof(aBuf), a);
	extract_area_content(bBuf, sizeof(bBuf), b);

	int lcs = longest_common_substring(aBuf, bBuf);

	int a_strl = strlen(aBuf);
	int b_strl = strlen(bBuf);

	int maxl = (a_strl > b_strl ? a_strl : b_strl);

	if (5 * lcs > 4 * maxl && maxl - lcs < 4) //diff部分不足够表意
		return true;

	if (maxl - lcs <= 1)
		return true;

	return false;
}

/**
 * @brief 记录相同位置及字体的块
 */
static void add_subtit_feat(subtit_collector_t *collect, html_area_t *area, font_t *font, const char *candi_subtit)
{
	for (int i = 0; i < collect->feat_num; i++)
	{
		subtit_feat_t *feat = &(collect->subtit_feat[i]);

		if (is_same_feat(feat, area, font))
		{
			int hspan = area->area_info.ypos - feat->last_ypos;
			if (hspan < feat->min_hxspan)
				feat->min_hxspan = hspan;
			if (hspan > feat->max_hxspan)
				feat->max_hxspan = hspan;
			if (feat->repeat_num < SUBTIT_NUM_LIMIT)
			{
				feat->area[feat->repeat_num] = area;
				snprintf(feat->cont[feat->repeat_num], sizeof(feat->cont[feat->repeat_num]), "%s", candi_subtit);
			}
			feat->repeat_num++;
			feat->last_ypos = area->area_info.ypos;
			feat->last_height = area->area_info.height;
			return;
		}
	}

	int n = sizeof(collect->subtit_feat) / sizeof(subtit_feat_t);

	if (collect->feat_num < n)
	{
		int path_sign = get_path_sign(area);
		subtit_feat_t *feat = &(collect->subtit_feat[collect->feat_num]);
		feat->area[0] = area;
		snprintf(feat->cont[0], sizeof(feat->cont[0]), "%s", candi_subtit);
		feat->font = font;
		feat->xpos = area->area_info.xpos;
		feat->path_sign = path_sign;
		feat->repeat_num = 1;
		feat->last_ypos = area->area_info.ypos;
		feat->last_height = area->area_info.height;
		feat->min_hxspan = 10000;
		feat->max_hxspan = 0;
		collect->feat_num++;
	}
}

static html_vnode_t *get_next_content_vnode(html_area_t *area)
{
	for (html_vnode_t *vnode = area->end->nextNode; vnode; vnode = vnode->nextNode)
	{
		if (vnode->isValid && vnode->subtree_textSize >= 4)
			return vnode;
	}
	return NULL;
}

static bool has_time_str(const char *tit)
{
	for (const char *p = tit; *p; p += GET_CHR_LEN(p))
	{
		if (is_time_str(p))
			return true;
	}
	return false;
}

static bool is_partof_tag_title(const char *tag_tit, const char *candi_tit)
{
	const char *ptit = candi_tit;
	if (strlen(candi_tit) >= 20)
		ptit += 6;

	/**
	 * 越过前缀,增加容错性
	 */
	if (strstr(tag_tit, ptit) != NULL)
		return true;

	const char *p = strstr(candi_tit, "标题");
	if (p != NULL)
	{
		p += strlen("标题");
		if (strncmp(p, ":", strlen(":")) == 0 || strncmp(p, "：", strlen("：")) == 0)
		{
			return true;
		}
	}

	return false;
}

static int visit_for_subtitle(html_area_t *area, subtit_collector_t *collect)
{
	if (!area->isValid)
	{
		marktreeprintfs(MARK_SUBTITFORRT, "skip the area(id=%d) for it is invalid at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_SKIP;
	}
	// shuangwei add 20120511
	if (!IS_LEAF_AREA(area->areaattr))
	{
		marktreeprintfs(MARK_SUBTITFORRT, "the area(id=%d) is not leaf,so it is not subtitforrt at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_NORMAL;
	}
	if (MARK_SUBTITFORRT == g_EASOU_DEBUG)
	{
		printlines("mark subtitforrt");
		printNode(area->begin->hpNode);
	}
	if (is_to_skip_subtree(area))
	{
		marktreeprintfs(MARK_SUBTITFORRT, "the area(id=%d) is to skip area at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_SKIP;
	}
	if (!is_single_inline(area))
	{
		if (area->area_info.width <= 250 && area->area_info.height <= 100)
		{
			/**一个小方块，左右有东西，里面应该没有子标题了吧~**/
			marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) is to skip for other area is in one line with it ,its width=%d ,height=%d at %s(%d)-%s\r\n", area->no, area->area_info.width, area->area_info.height, __FILE__, __LINE__, __FUNCTION__);
			return AREA_VISIT_SKIP;
		}
		else if (area->pos_mark == RELA_LEFT || area->pos_mark == RELA_RIGHT)
		{
			marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) is to skip for other area is in one line with it and is at left or right,its pos_mark=%d  at %s(%d)-%s\r\n", area->no, area->pos_mark, __FILE__, __LINE__, __FUNCTION__);
			return AREA_VISIT_SKIP;
		}
		else
		{
			marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) is not subtitforrt for other area is in one line with it  at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
			return AREA_VISIT_NORMAL;
		}
	}

	if (is_to_skip_normal(area))
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) is not subtitforrt  at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_NORMAL;
	}

	html_area_t *next = get_next_content_area(area);
	if (next)
	{
		if (is_func_area(next, AREA_FUNC_MYPOS) || is_func_area(next, AREA_FUNC_NAV) || is_func_area(next, AREA_FUNC_RELATE_LINK) || is_func_area(next, AREA_FUNC_FRIEND) || is_func_area(next, AREA_FUNC_ARTICLE_SIGN) || is_srctype_area(next, AREA_SRCTYPE_INTERACTION))
		{
			marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) is to skip for its next area is mypos,nav,relatelink,friend,article_sign or interaction  at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
			return AREA_VISIT_SKIP;
		}
	}

	font_t *font = NULL;
	unsigned int font_num = 0;
	bool is_exceed = get_area_font(area, &font, 1, 2, font_num);

	if (font_num == 0)
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) don't contain text with text size >2,so skip it  at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_SKIP;
	}

	if (font->color == font->bgcolor)
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) font color=bgcolor,so skip it  at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_SKIP;
	}

	char candi_subtit[256];
	extract_area_content(candi_subtit, sizeof(candi_subtit), area);

	str_trim_side_space(candi_subtit);
	trans2bj_lower(candi_subtit, candi_subtit);

	if (has_time_str(candi_subtit))
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) is to skip for it contain time at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_SKIP;
	}

	if (area->baseinfo->text_info.con_size >= 10 && is_partof_tag_title(collect->tag_title, candi_subtit))
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the content of the area(id=%d) is part of tag title,so skip it  at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_SKIP;
	}

	if (is_exceed)
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) is not subtitforrt ,for it font size >1  at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_NORMAL;
	}

	marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) maybe subtitforrt at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
	add_subtit_feat(collect, area, font, candi_subtit);

	return AREA_VISIT_SKIP;
}

static font_t *get_upper_font(const html_area_t *area)
{
	int area_text_size = area->baseinfo->text_info.con_size;

	html_vnode_t *vnode = area->begin->upperNode;
	while (vnode)
	{
		if (vnode->subtree_textSize > area_text_size)
			return &(vnode->font);
		vnode = vnode->upperNode;
	}
	return NULL;
}

static font_t *get_prev_font(const html_area_t *area)
{
	html_vnode_t *vnode = area->begin;
	for (html_vnode_t *prev = vnode->prevNode; prev; prev = prev->prevNode)
	{
		if (prev->isValid && prev->wx > 0 && prev->hx > 0)
			return &(prev->font);
	}
	return NULL;
}

static font_t *get_next_font(const html_area_t *area)
{
	html_vnode_t *vnode = area->end;
	for (html_vnode_t *next = vnode->nextNode; next; next = next->nextNode)
	{
		if (next->isValid && next->wx > 0 && next->hx > 0)
			return &(next->font);
	}
	return NULL;
}

static bool is_outstand_than(const font_t *font, const font_t *other_font)
{
	if (font->size >= other_font->size || font->header_size > other_font->header_size || font->is_bold > other_font->is_bold || font->is_strong > other_font->is_strong)
	{
	}
	else
		return false;

	if ((font->header_size > 0 && other_font->header_size == 0) || font->is_bold > other_font->is_bold || font->is_strong > other_font->is_strong || font->is_big > other_font->is_big || (font->size >= 20 && font->size > other_font->size) || font->is_italic > other_font->is_italic
			|| (font->line_height >= 30 && font->line_height * 2 >= other_font->line_height * 3) || font->bgcolor != other_font->bgcolor || (font->color != other_font->color && !is_gray_color(font->color)))
	{
		return true;
	}
	return false;
}

static bool is_outstand_font(subtit_feat_t *feat)
{
	html_vnode_t *next = get_next_content_vnode(feat->area[0]);
	if (next == NULL)
		return false;

	font_t *font = feat->font;

	const font_t *prev_font = get_prev_font(feat->area[0]);
	if (prev_font)
	{
		if (!is_outstand_than(font, prev_font))
			return false;
	}

	const font_t *next_font = get_next_font(feat->area[0]);
	if (next_font)
	{
		if (!is_outstand_than(font, next_font))
			return false;
	}
	else
		return false;

	const font_t *upper_font = get_upper_font(feat->area[0]);

	if (upper_font && is_outstand_than(font, upper_font))
	{
		return true;
	}

	return false;
}

static bool is_subtit_span_enough(int min_hxspan, int max_hxspan, int height)
{
	if (min_hxspan >= 3 * height)
		return true;

	if (min_hxspan - height >= 16 && max_hxspan >= 3 * height)
		return true;

	if (min_hxspan > height && (min_hxspan >= 50 || max_hxspan >= 80))
		return true;

	return false;
}

typedef struct _feat_count_t
{
	unsigned int area_depth;
	int path_sign;
	font_t *font;
	int candi_subtit_num;
	int feat_repeat_num;
	bool is_too_many_repeat;
} feat_count_t;

static int visit_for_feat_count(html_area_t *area, feat_count_t *fcnt)
{
	if (!area->isValid)
		return AREA_VISIT_SKIP;

	if (area->depth < fcnt->area_depth)
		return AREA_VISIT_NORMAL;

	int path_sign = get_path_sign(area);
	if (path_sign != fcnt->path_sign)
		return AREA_VISIT_SKIP;

	font_t *font = NULL;
	unsigned int font_num = 0;
	bool is_exceed = get_area_font(area, &font, 1, 2, font_num);
	if (!is_exceed && font_num == 1 && is_same_font(font, fcnt->font))
	{
		fcnt->feat_repeat_num++;
		if (fcnt->feat_repeat_num > fcnt->candi_subtit_num + 3)
		{
			fcnt->is_too_many_repeat = true;
			return AREA_VISIT_FINISH;
		}
	}

	return AREA_VISIT_SKIP;
}
/**
 * 其他块是否还含有相同位置块
 */
static bool too_many_repeat_feat(subtit_feat_t *feat, area_tree_t *atree)
{
	feat_count_t fcnt;
	fcnt.area_depth = feat->area[0]->depth;
	fcnt.path_sign = feat->path_sign;
	fcnt.font = feat->font;
	fcnt.candi_subtit_num = feat->repeat_num;
	fcnt.feat_repeat_num = 0;
	fcnt.is_too_many_repeat = false;

	areatree_visit(atree, (FUNC_START_T) visit_for_feat_count, NULL, &fcnt);

	return fcnt.is_too_many_repeat;
}

static bool is_same_font_except_color(font_t *a, font_t *b)
{
	if (a->align == b->align && a->header_size == b->header_size && a->is_bold == b->is_bold && a->is_strong == b->is_strong && a->is_big == b->is_big && a->is_small == b->is_small && a->is_italic == b->is_italic && a->is_underline == b->is_underline && a->size == b->size
			&& a->line_height == b->line_height && a->bgcolor == b->bgcolor)
	{
		return true;
	}

	return false;
}

static bool too_many_brother_color(html_area_t *area, font_t *font)
{
	html_area_t *parea = area->parentArea;

	int color_text_size = 0;

	for (html_area_t *brot = parea->subArea; brot; brot = brot->nextArea)
	{
		if (brot->isValid && brot->baseinfo->text_info.con_size >= 8)
		{
			font_t *brot_font = NULL;
			unsigned int font_num = 0;
			get_area_font(area, &brot_font, 1, 2, font_num);
			if (font_num > 0)
			{
				if (is_same_font_except_color(brot_font, font) && brot_font->color != 0x0 && !is_gray_color(brot_font->color))
				{
					color_text_size += brot->baseinfo->text_info.con_size;
				}
			}
		}
	}

	if (color_text_size * 9 >= parea->baseinfo->text_info.con_size * 10)
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area is not subtitforrt for color_text_size*9(%d)>=parea->baseinfo->text_info.con_size*10(%d)  at %s(%d)-%s\r\n", color_text_size*9, parea->baseinfo->text_info.con_size*10, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}

	return false;
}

static bool has_same_pat(subtit_feat_t *feat)
{
	for (int i = 1; i < feat->repeat_num && i < SUBTIT_NUM_LIMIT; i++)
	{
		for (int j = 0; j < i; j++)
		{
			if (is_same_pat(feat->area[i], feat->area[j]))
			{
				marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the feat is not subtitforrt for  same pattern  at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
				return true;
			}
		}
	}

	return false;
}

static bool subtit_to_filter(subtit_feat_t *feat, area_tree_t *atree)
{
	if (too_many_repeat_feat(feat, atree))
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area is not subtitforrt for  too_many_repeat  at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		return true;
	}

	int up_ypos = feat->area[0]->area_info.ypos;
	int down_ypos = feat->area[feat->repeat_num - 1]->area_info.ypos;

	if (up_ypos > 600 && 2 * up_ypos > atree->root->area_info.height)
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area is not subtitforrt for up_upos(%d)>0.5*the page height(%d)  at %s(%d)-%s\r\n", up_ypos, atree->root->area_info.height, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}

	if (down_ypos < 500 && 4 * down_ypos < atree->root->area_info.height)
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area is not subtitforrt for down_upos(%d)<0.25*the page height(%d)  at %s(%d)-%s\r\n", down_ypos, atree->root->area_info.height, __FILE__, __LINE__, __FUNCTION__);
		return true;
	}

	if (too_many_brother_color(feat->area[0], feat->font))
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area is not subtitforrt for too_many_brother_color()  at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		return true;
	}

	if (has_same_pat(feat))
	{
		marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area is not subtitforrt for has_same_pat()  at %s(%d)-%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		return true;
	}

	return false;
}

/**
 * @brief 标记小标题.
 * 		标记有两个产出：
 * 	AREA_FUNC_SUBTITLE : 小标题.
 * 	AREA_FUNC_CANDI_SUBTITLE : 候选小标题，主要用于辅助realtitle。
 **/
bool mark_func_subtit_forrt(area_tree_t *atree)
{
	mark_area_info_t * mark_info = atree->mark_info;
	subtit_collector_t collect;
	subtit_collector_clean(&collect);
	trans2bj_lower(mark_info->tag_title, collect.tag_title);

	areatree_visit(atree, (FUNC_START_T) visit_for_subtitle, NULL, &collect);

	for (int i = 0; i < collect.feat_num; i++)
	{
		subtit_feat_t *feat = &collect.subtit_feat[i];
		if (feat->repeat_num >= 2 && feat->repeat_num <= 16)
		{
			if (is_subtit_span_enough(feat->min_hxspan, feat->max_hxspan, feat->last_height) && is_outstand_font(feat) && !subtit_to_filter(feat, atree))
			{
				for (int i = 0; i < feat->repeat_num; i++)
				{
					tag_area_func(feat->area[i], AREA_FUNC_SUBTIT_4RT);
				}
			}
			else if (feat->repeat_num >= 3)
			{
				for (int i = 0; i < feat->repeat_num; i++)
				{
					marktreeprintfs(MARK_CANDI_SUBTIT_4RT, "subtitforrt:the area is  MARK_CANDI_SUBTIT_4RT for feat->repeat_num(%d) is >=3  at %s(%d)-%s\r\n", feat->repeat_num, __FILE__, __LINE__, __FUNCTION__);
					tag_area_func(feat->area[i], AREA_FUNC_CANDI_SUBTIT_4RT);
				}
			}
		}
		else
		{
			marktreeprintfs(MARK_SUBTITFORRT, "subtitforrt:the area(id=%d) is not subtitforrt for feat->repeat_num(%d) is not in 2 -- 16  at %s(%d)-%s\r\n", feat->area[0]->no, feat->repeat_num, __FILE__, __LINE__, __FUNCTION__);
		}
	}

	return true;
}

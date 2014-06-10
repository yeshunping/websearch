/*
 * mark_func_article_content.cpp
 * Description: 标记文章的正文块
 *  Created on: 2012-10-29
 *      Author: sue_zhang@staff.easou.com
 */

#include "easou_mark_func.h"
#include "easou_mark_markinfo.h"
#include "easou_mark_baseinfo.h"
#include "easou_mark_com.h"
#include "easou_mark_srctype.h"
#include "easou_mark_sem.h"
#include "easou_ahtml_dtd.h"
#include "easou_html_attr.h"
#include "easou_debug.h"

typedef struct _ac_pack_t
{
	mark_area_info_t *mark_info;
	const html_area_t *realtit;
	int realtitle_bottom; //真实标题底部的y坐标，如果未标注值为-1
	const html_area_t *articlesign;
	int articlesign_bottom; //文章签名底部的y坐标，如果未标注值为-1
	const html_area_t *turnpage; //翻页块
	const html_area_t *relateLink; //相关链接块
	int marked_num; //已标记为主体块的个数
	int marked_textSize; //已标记为主体块的文本数
	int marked_depth[10]; //已经标注为文章内容块的各层个数
} ac_pack_t;

typedef struct _ac_text_tag_info_t
{
	int tot_text_len;
	int max_text_len;
	int text_tag_num;
	int is_has_container_tag;
} ac_text_tag_info_t;

static void text_tag_info_clean(ac_text_tag_info_t *tt_info)
{
	memset(tt_info, 0, sizeof(ac_text_tag_info_t));
}

static bool is_h_ul_vnode(html_vnode_t *vnode)
{
	if (vnode->hpNode->html_tag.tag_type != TAG_DIV)
	{
		return false;
	}
	if (vnode->hx > 200 || vnode->wx > 800)
	{
		return false;
	}
	if (vnode->subtree_diff_font > 2)
	{
		return false;
	}
	if (!vnode->firstChild || !ahtml_small_header_map[vnode->firstChild->hpNode->html_tag.tag_type])
	{
		return false;
	}
	if (!vnode->firstChild->nextNode || vnode->firstChild->nextNode->hpNode->html_tag.tag_type != TAG_UL)
	{
		return false;
	}
	return true;
}

static int visit_for_text_tag_info(html_vnode_t *vnode, void *data)
{
	if (!vnode->isValid)
		return VISIT_SKIP_CHILD;
	ac_text_tag_info_t *tt_info = (ac_text_tag_info_t *) data;
	if (vnode->hpNode->html_tag.tag_type == TAG_PURETEXT)
	{
		tt_info->tot_text_len += vnode->textSize;
		tt_info->text_tag_num++;
		if (vnode->textSize > tt_info->max_text_len)
			tt_info->max_text_len = vnode->textSize;
	}
	else if (vnode->hpNode->html_tag.tag_type == TAG_A && get_attribute_value(&vnode->hpNode->html_tag, ATTR_HREF) != NULL)
	{
		return VISIT_SKIP_CHILD;
	}
	return VISIT_NORMAL;
}

static int visit_for_container_tag(html_vnode_t *vnode, void *data)
{
	if (vnode->isValid)
	{
		ac_text_tag_info_t *tt_info = (ac_text_tag_info_t *) data;
		html_tag_type_t tag_type = vnode->hpNode->html_tag.tag_type;
		if (tag_type == TAG_ROOT || tag_type == TAG_HTML || tag_type == TAG_BODY || tag_type == TAG_TABLE || tag_type == TAG_TBODY || tag_type == TAG_DIV || tag_type == TAG_COLGROUP || tag_type == TAG_CENTER || tag_type == TAG_FORM)
		{
			tt_info->is_has_container_tag = 1;
		}
	}
	return VISIT_NORMAL;
}

static int has_proper_tag_len(html_area_t *area, int page_width, int page_height, int page_text_len)
{
	ac_text_tag_info_t tt_info;
	text_tag_info_clean(&tt_info);
	/**
	 * 获取文本标签的信
	 */
	for (html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode)
	{
		if (vnode->isValid)
		{
			html_vnode_visit(vnode, visit_for_text_tag_info, visit_for_container_tag, &tt_info);
		}
		if (vnode == area->end)
			break;
	}
	if (tt_info.is_has_container_tag == 0 && (area->begin->hpNode->html_tag.tag_type == TAG_TABLE || (area->begin->hpNode->html_tag.tag_type == TAG_P && area->valid_subArea_num < 1)) && area->baseinfo->text_info.con_size > area->baseinfo->link_info.anchor_size * 2)
	{
		return 1;
	}
	/* *
	 * 根据文本标签的长度数量等信息判断是否主要内容
	 * 一般说来，正文的文本标签长度较大，数量相对较少
	 * 而如果文本比较支零破碎，可能是版权信息，或边框上的文字
	 */
	if (tt_info.max_text_len >= 100 || (tt_info.text_tag_num > 0 && tt_info.tot_text_len / tt_info.text_tag_num >= 70))
	{
		return 1;
	}
	int doctype = area->area_tree->hp_vtree->hpTree->doctype;
	if (area->area_info.ypos < page_height / 3)
	{
		if (tt_info.max_text_len >= 22 || ((tt_info.text_tag_num <= 3 && tt_info.max_text_len >= 4) && !(doctype >= doctype_xhtml_BP && doctype <= doctype_html5)))
			return 1;
	}
	if (tt_info.max_text_len >= page_text_len / 3)
	{
		//这个策略注重准确
		return 1;
	}
	return 0;
}

/**
 * @brief 判断块是否在页面两边
 */
static bool is_page_sider(html_area_t *area)
{
	if (area->pos_plus == IN_PAGE_LEFT)
	{
		/**
		 * 在页面左边，根据链接和交互标签特征判断是否边框.
		 */
		if (area->baseinfo->link_info.num + area->baseinfo->link_info.other_num >= 6)
		{
			debuginfo(MARK_AC, "the area is at the left side of page ,and link >=6");
			return true;
		}
		if (area->baseinfo->inter_info.input_num + area->baseinfo->inter_info.select_num >= 2)
		{
			debuginfo(MARK_AC, "the area is at the left side of page ,and input +  select >2=");
			return true;
		}
		if (area->area_info.height >= area->area_info.width * 2 || area->area_info.height >= 250)
		{
			debuginfo(MARK_AC, "the area is at the left side of page ,and height > width*2 or height>250 ");
			return true;
		}
	}
	if (area->pos_plus == IN_PAGE_RIGHT)
	{
//		if (area->area_info.height > area->area_info.width)
//		{
		debuginfo(MARK_AC, "the area is at the right side of page");
		return true;
//		}
	}
//	if (area->prevArea && !is_func_area(area->prevArea, AREA_FUNC_ARTICLE_CONTENT) && (area->abspos_mark == PAGE_LEFT || area->abspos_mark == PAGE_RIGHT))
//	{
//		debuginfo(MARK_AC, "the area(id=%d) is side area", area->no);
//		return true;
//	}
	debuginfo(MARK_AC, "the area is not the side ,height=%d ,width=%d ", area->area_info.height, area->area_info.width);
	return false;
}

/**
 * @brief 判断该节点是否为会移动的文字MARQUEE
 */
static int check_unproper_tag(html_vnode_t *vnode, void *data)
{
	if (!vnode->isValid || vnode->subtree_textSize <= 0)
		return VISIT_SKIP_CHILD;

	bool *has_unproper_tag = (bool *) data;

	if (vnode->hpNode->html_tag.tag_type == TAG_MARQUEE)
	{
		*has_unproper_tag = true;
		return VISIT_FINISH;
	}

	return VISIT_NORMAL;
}

/**
 * @brief 判断该块中是否含有会移动的文字；true：含有；不含有：false
 */
static bool has_unproper_tag(html_area_t *area)
{
	if (area->area_info.height >= 50)
	{
		return false;
	}
	bool has_unproper_tag = false;
	for (html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode)
	{
		if (vnode->isValid)
		{
			html_vnode_visit(vnode, check_unproper_tag, NULL, &has_unproper_tag);
			if (has_unproper_tag)
				break;
		}
		if (vnode == area->end)
			break;
	}
	return has_unproper_tag;
}

/**
 * @brief 判断是否核心正文块.
 **/
static int is_article_content_area(html_area_t * area, mark_area_info_t * mark_info)
{
	if (area->depth == 0)
		return false;

//	if (area->depth == 1)
//	{
//		/**
//		 * 子分块不含边框.
//		 */
//		for (html_area_t *subarea = area->subArea; subarea; subarea = subarea->nextArea)
//		{
//			if (subarea->isValid && is_page_sider(subarea))
//			{
//				debuginfo(MARK_AC, "the area is not for it is at side");
//				return false;
//			}
//		}
//	}

	if ((is_contain_func_area(area, AREA_FUNC_MYPOS) && !is_func_area(area, AREA_FUNC_MYPOS)) || is_contain_func_area(area, AREA_FUNC_NAV) || is_contain_func_area(area, AREA_FUNC_FRIEND) || is_contain_func_area(area, AREA_FUNC_RELATE_LINK) || is_contain_func_area(area, AREA_FUNC_COPYRIGHT) || is_contain_func_area(area, AREA_FUNC_ARTICLE_SIGN) || (is_contain_sem_area(area, AREA_SEM_REALTITLE) && !is_sem_area(area, AREA_SEM_REALTITLE)))
	{
		/**
		 * 控制粒度
		 */
		debuginfo(MARK_AC, "the area is not for it is func or src_out or realtitle");
		return false;
	}

	if (has_unproper_tag(area))
	{
		debuginfo(MARK_AC, "the area is not central for the marquee tag is in the area");
		return false;
	}
	if (area->baseinfo->link_info.link_area > area->baseinfo->text_info.text_area / 3 && area->valid_subArea_num > 0)
	{
		debuginfo(MARK_AC, "the area is not central for link_area> text_area /3 and it has sub area");
		return false;
	}

	return true;
}

/**
 * @brief 根据有意义的块面积.
 **/
//static int get_tot_use_cont_area(html_area_t *area)
//{
//	area_baseinfo_t *baseinfo = area->baseinfo;
//	int tot_area = baseinfo->inter_info.in_area + baseinfo->pic_info.pic_area + baseinfo->text_info.text_area - baseinfo->text_info.no_use_text_area;
//	return tot_area;
//}
/**
 * @brief 跳过如下区域.
 **/
static bool is_to_skip_mark_central(html_area_t *area, mark_area_info_t *mark_info, const html_area_t *realtit)
{
	area_baseinfo_t *baseinfo = area->baseinfo;
	if (area->depth == 1)
	{
		/**
		 * 顶部或顶部，若有较多链接或交互标签，认为是边框，过掉
		 */
		if (area->pos_mark == RELA_HEADER && !is_contain_sem_area(area, AREA_SEM_REALTITLE))
		{
			if (baseinfo->link_info.num + baseinfo->link_info.other_num >= 4 || baseinfo->link_info.other_num >= 2)
			{
				debuginfo(MARK_AC, "skip the area(id=%d) for it is at RELA_HEADER and don't contain realtitle and link_num=%d ,link_other_num=%d ", area->no, baseinfo->link_info.num, baseinfo->link_info.other_num);
				return true;
			}
			if (baseinfo->inter_info.input_num + baseinfo->inter_info.select_num >= 2)
			{
				debuginfo(MARK_AC, "skip the area(id=%d) for it is at RELA_HEADER and don't contain realtitle and input+select>2", area->no);
				return true;
			}
			if (baseinfo->text_info.con_size - baseinfo->text_info.no_use_con_size <= 0)
			{
				debuginfo(MARK_AC, "skip the area(id=%d) for it is at RELA_HEADER and don't contain realtitle and useful content size<0", area->no);
				return true;
			}
		}

		if (area->pos_mark == RELA_FOOTER && area->valid_subArea_num == 0 && area->no > (unsigned int) area->area_tree->base_info->max_text_leaf_area_no)
		{
			debuginfo(MARK_AC, "skip the area(id=%d) for it is at RELA_FOOTER", area->no);
			return true;
		}
	}

	/**
	 * 根据realtitle定位
	 */
	if (realtit != NULL && area->area_info.ypos + area->area_info.height < realtit->area_info.ypos && area->area_info.ypos < realtit->area_info.ypos - 100)
	{
		debuginfo(MARK_AC, "skip the area(id=%d) for it is above the real title area", area->no);
		return true;
	}

	if (!is_srctype_area(area, AREA_SRCTYPE_PIC) && is_page_sider(area))
	{ //图片块的位置标记不准，现对图片块豁免
		/**
		 * 过滤左右边框
		 */
		debuginfo(MARK_AC, "skip the area(id=%d) for it is the side area", area->no);
		return true;
	}

	if (is_in_func_area(area, AREA_FUNC_NAV) || is_in_func_area(area, AREA_FUNC_FRIEND) || is_in_func_area(area, AREA_FUNC_RELATE_LINK) || is_in_func_area(area, AREA_FUNC_ARTICLE_SIGN) || is_in_func_area(area, AREA_FUNC_COPYRIGHT) || is_in_srctype_area(area, AREA_SRCTYPE_INTERACTION))
	{
		debuginfo(MARK_AC, "skip the area(id=%d) for it is func or src area", area->no);
		return true;
	}

	/**
	 * 无意义的分块
	 */
//	if (get_tot_use_cont_area(area) <= 0)
//	{
//		debuginfo(MARK_AC, "skip the area(id=%d) for the useful total area size of it <=0", area->no);
//		return true;
//	}
	/**
	 * 无意义的分块
	 */
//	if (area->area_info.width <= 5 || area->area_info.height <= 5)
//	{
//		if (area->baseinfo->text_info.con_size - area->baseinfo->text_info.no_use_con_size <= 0)
//		{
//			debuginfo(MARK_AC, "skip the area(id=%d) for its useful text area size <=0", area->no);
//			return true;
//		}
//	}
	return false;
}

static bool is_area_has_same_root(const html_area_t *area, const html_area_t *another_area)
{
	bool is_contain = false;
	const html_area_t *upper = area->depth > another_area->depth ? area : another_area;
	while (upper)
	{
		if (upper->no == another_area->no)
		{
			is_contain = true;
			break;
		}
		upper = upper->parentArea;
	}
	return is_contain;
}

/**
 * @brief 分块主要由table组成
 * @date 2012-10-27
 * @author sue
 */
static int is_main_table(html_area_t *area)
{
	bool flag = false;
	html_vnode_t *vnode = NULL;
	if (area->begin == area->end)
	{
		vnode = area->begin;
	}
	while (vnode)
	{
		if (vnode->hpNode->html_tag.tag_type == TAG_TABLE)
		{
			flag = true;
			break;
		}
		if (vnode->firstChild && vnode->firstChild->nextNode == NULL)
		{
			vnode = vnode->firstChild;
			continue;
		}
		break;
	}
	return flag;
}

/**
 * @brief 判断分块的纯文本ypos是否在同一行上
 */
static bool is_text_in_one_line(html_area_t *area)
{
	int ypos = 0;
	bool is_in_one_line = true;
	vnode_list_t *list_begin = area->baseinfo->text_info.cont_vnode_list_begin;
	vnode_list_t *list_end = area->baseinfo->text_info.cont_vnode_list_end;
	if (list_begin == NULL && list_end == NULL)
	{
		return true;
	}
	ypos = list_begin->vnode->ypos;
	for (vnode_list_t *list = list_begin; list; list = list->next)
	{
		if (list->vnode->ypos != ypos)
		{
			is_in_one_line = false;
			break;
		}
		if (list == list_end)
		{
			break;
		}
	}
	return is_in_one_line;
}

/**
 * @brief 在块内判断是否有纯文本具有float:right属性
 */
static bool has_float_right_text(html_area_t *area)
{
	if (area->begin != area->end)
	{
		return false;
	}
	int depth = area->begin->depth;
	vnode_list_t *list_begin = area->baseinfo->text_info.cont_vnode_list_begin;
	vnode_list_t *list_end = area->baseinfo->text_info.cont_vnode_list_end;
	if (list_begin == NULL && list_end == NULL)
	{
		return false;
	}
	for (vnode_list_t *list = list_begin; list; list = list->next)
	{
		html_vnode_t *upper = list->vnode;
		while (upper && upper->depth >= depth)
		{
			char *float_value = get_css_attribute(upper, CSS_PROP_FLOAT);
			if (float_value != NULL && strstr(float_value, "right"))
			{
				return true;
			}
			upper = upper->upperNode;
		}
		if (list == list_end)
		{
			break;
		}
	}
	return false;
}

static int start_visit_for_mark_sem(html_area_t *area, void *data)
{
	if (area->isValid == false)
	{
		debuginfo(MARK_AC, "skip the area(id=%d) for it is not valid", area->no);
		return AREA_VISIT_SKIP;
	}
	ac_pack_t *sc_pack = (ac_pack_t *) data;
	float pure_text_rate = area->baseinfo->text_info.pure_text_rate;
	float max_pure_text_rate = area->area_tree->base_info->max_text_rate_leaf;
	int marked_textSize = sc_pack->marked_textSize;
	int anchor_size = area->baseinfo->link_info.anchor_size;
	int other_link_num = area->baseinfo->link_info.other_num;
	int link_num = area->baseinfo->link_info.num;
	int out_link_num = area->baseinfo->link_info.out_num;
	int cursor_num = area->baseinfo->inter_info.cursor_num;
	int script_num = area->baseinfo->inter_info.script_num;
	int time_num = area->baseinfo->text_info.time_num;
	int text_size = area->baseinfo->text_info.con_size;
	int cn_num = area->baseinfo->text_info.cn_num;
	int valid_text_size = text_size - area->baseinfo->text_info.no_use_con_size;
	int text_num = area->baseinfo->text_info.con_num;
	int valid_text_num = text_num - area->baseinfo->text_info.no_use_con_num;
	int pic_num = area->baseinfo->pic_info.pic_num;
	int size_fixed_pic_num = area->baseinfo->pic_info.size_fixed_num;
	int hx = area->area_info.height;
	int wx = area->area_info.width;
	int ypos = area->area_info.ypos;
	int xpos = area->area_info.xpos;
	int recommend_link_word = area->baseinfo->text_info.recommend_spec_word;
	int inter_word_num = area->baseinfo->inter_info.spec_word_num;

	bool hasOneChildVnode = false; //分块是否只有一个VNODE儿子
	html_tag_type_t oneChildTagType = TAG_UNKNOWN; //分块唯一VNODE儿子的节点类型
	if (area->begin == area->end)
	{
		hasOneChildVnode = true;
		oneChildTagType = area->begin->hpNode->html_tag.tag_type;
	}

	if (area->no == 0)
	{
		debuginfo(MARK_AC, "into the area(id=%d)", area->no);
		return AREA_VISIT_NORMAL;
	}
	if (sc_pack->realtit)
	{
		if (xpos == sc_pack->realtit->area_info.xpos)
		{ //如果和标题块的x坐标相同（这里主要是因为坐标有时候计算不准）
			if (ypos <= sc_pack->realtit->area_info.ypos)
			{ //如果y坐标在第一个标题块的上面，认为不会是文章内容块
				debuginfo(MARK_AC, "the area(id=%d) is not for it is above realtitle area", area->no);
				return AREA_VISIT_NORMAL;
			}
		}
		else if (ypos < sc_pack->realtitle_bottom)
		{
			debuginfo(MARK_AC, "the area(id=%d) is not for it is above realtitle area", area->no);
			return AREA_VISIT_NORMAL;
		}
	}
	if (sc_pack->relateLink)
	{
		const html_area_t *relateLinkArea = sc_pack->relateLink;
		if (area->valid_subArea_num == 0)
		{
			//刚好在相关链接块下面的叶子块，认为不是文章内容块
			if (relateLinkArea->area_info.xpos == xpos && ypos == (relateLinkArea->area_info.ypos + relateLinkArea->area_info.height))
			{
				debuginfo(MARK_AC, "the area(id=%d) is not for it is just bellow relate link area", area->no);
				return AREA_VISIT_NORMAL;
			}
		}
	}
	if (sc_pack->articlesign)
	{
		if (xpos == sc_pack->articlesign->area_info.xpos)
		{ //如果和文章签名块的x坐标相同（这里主要是因为坐标有时候计算不准）
			if (ypos <= sc_pack->articlesign->area_info.ypos)
			{ //如果y坐标在第一个文章签名块的上面，认为不会是文章内容块
				debuginfo(MARK_AC, "the area(id=%d) is not for it is above articlesign area", area->no);
				return AREA_VISIT_NORMAL;
			}
		}
		else if (ypos < sc_pack->articlesign_bottom)
		{//如果y坐标在第一个文章签名块的上面，认为不会是文章内容块
			debuginfo(MARK_AC, "the area(id=%d) is not for it is above articlesign area", area->no);
			return AREA_VISIT_NORMAL;
		}
	}
	if (area->valid_subArea_num == 0)
	{ // 是叶子块
		if (sc_pack->articlesign)
		{ //有标注文章签名块
			if (sc_pack->articlesign_bottom == area->area_info.ypos)
			{ //刚好在文章签名块的下面
				if (is_func_area(area, AREA_FUNC_SUBTITLE) || is_in_func_area(area, AREA_FUNC_SUBTITLE))
				{ //是子标题块
					debuginfo(MARK_AC, "the area(id=%d) is for it is subtitle area", area->no);
					tag_area_func(area, AREA_FUNC_ARTICLE_CONTENT);
					sc_pack->marked_num++;
					sc_pack->marked_textSize += valid_text_size;
					if (area->depth < 10)
					{
						sc_pack->marked_depth[area->depth]++;
					}
					return AREA_VISIT_SKIP;
				}
			}
		}
		if (sc_pack->articlesign_bottom > 0 && sc_pack->realtitle_bottom > 0)
		{ //文章签名块和标题块都存在
			if (hx > 0 && hx <= 30 && ypos <= sc_pack->realtitle_bottom && (ypos + hx) >= sc_pack->articlesign->area_info.ypos)
			{ //文章签名块和真实标题块之间的内容过滤掉 http://news.youth.cn/yl/201211/t20121114_2614466.htm
				debuginfo(MARK_AC, "skip the area(id=%d) for it between realtitle and articlesign", area->no);
				return AREA_VISIT_SKIP;
			}
		}
		//过滤掉靠左或靠右的链接块
		if (is_srctype_area(area, AREA_SRCTYPE_HUB) || is_in_srctype_area(area, AREA_SRCTYPE_HUB))
		{
			if (wx < 230 && hx > 500)
			{
				if (area->abspos_mark == PAGE_LEFT || area->abspos_mark == PAGE_RIGHT)
				{
					debuginfo(MARK_AC, "skip the area(id=%d) for it is side area", area->no);
					return AREA_VISIT_SKIP;
				}
			}
		}
		if (is_srctype_area(area, AREA_SRCTYPE_LINK) && is_srctype_area(area, AREA_SRCTYPE_HUB) && !is_contain_srctype_area(area, AREA_SRCTYPE_TEXT))
		{
			bool flag = false;
			if (area->begin == area->end)
			{
				char *class_value = get_attribute_value(&area->begin->hpNode->html_tag, ATTR_CLASS);
				if (class_value && strstr(class_value, "keywords"))
				{
					flag = true;
				}
			}
			if (hasOneChildVnode && (oneChildTagType == TAG_P || oneChildTagType == TAG_TABLE))
			{
				flag = true;
			}
			if (!flag)
			{
				if (pure_text_rate + 0.005 > max_pure_text_rate && marked_textSize > 0)
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d)", area->no);
					return AREA_VISIT_FINISH;
				}
				else
				{
					debuginfo(MARK_AC, "skip the area(id=%d)", area->no);
					return AREA_VISIT_SKIP;
				}
			}
		}
	}
	if (sc_pack->realtit && sc_pack->realtit->area_info.xpos > 0 && cn_num <= 6)
	{
		int xlimit = area->area_info.xpos + area->area_info.width;
		if (xlimit < sc_pack->realtit->area_info.xpos)
		{
			debuginfo(MARK_AC, "skip the area(id=%d)", area->no);
			return AREA_VISIT_SKIP;
		}
	}
	if (is_srctype_area(area, AREA_SRCTYPE_OUT))
	{ //TODO 外部块有可能是广告块，也有可能是视频等，先统一跳过了
		debuginfo(MARK_AC, "skip the area(id=%d) for it is out", area->no);
		return VISIT_SKIP_CHILD;
	}
	if (is_sem_area(area, AREA_SEM_REALTITLE))
	{
		debuginfo(MARK_AC, "skip the area(id=%d) for it is realtitle area", area->no);
		return AREA_VISIT_SKIP;
	}
	if (!is_srctype_area(area, AREA_SRCTYPE_PIC))
	{ //图片块的位置标记不准，先对图片块豁免该策略
		if (area->pos_plus == IN_PAGE_LEFT || area->pos_plus == IN_PAGE_RIGHT)
		{
			debuginfo(MARK_AC, "skip the area(id=%d) for it is side area", area->no);
			return AREA_VISIT_SKIP;
		}
	}
	//遇到可能是交互块，且接近最大纯文本比例，结束标记
	if (inter_word_num && other_link_num > 1 && pure_text_rate + 0.01 > max_pure_text_rate && sc_pack->marked_textSize)
	{
		debuginfo(MARK_AC, "skip all the left areas after area(id=%d)", area->no);
		return VISIT_FINISH;
	}
	if (inter_word_num && cn_num < 6 && other_link_num > 0)
	{
		debuginfo(MARK_AC, "the area(id=%d) is not for it maybe interaction area", area->no);
		return VISIT_SKIP_CHILD;
	}
	if (recommend_link_word && cn_num <= 5)
	{
		debuginfo(MARK_AC, "the area(id=%d) is not for it maybe title of recommend area", area->no);
		return VISIT_SKIP_CHILD;
	}
	//和最大纯文本比例差距不至于太大
	if (area->baseinfo->text_info.pure_text_rate + 0.02 > area->area_tree->base_info->max_text_rate_leaf && (!hasOneChildVnode || (oneChildTagType != TAG_P && oneChildTagType != TAG_TABLE)))
	{
		//遇到连续多个链接，且已经标记过有效主体内容了，结束标记
		if (sc_pack->marked_textSize > 0 && valid_text_size == anchor_size && area->end->hpNode->html_tag.tag_type != TAG_BR && link_num > 1)
		{
			if (area->prevArea)
			{
				area_baseinfo_t *pre_info = area->prevArea->baseinfo;
				if (pre_info->text_info.con_num - pre_info->text_info.no_use_con_num == 1 && pre_info->text_info.con_size < 20)
				{
					bool clear_pre = false;
					vnode_list_t *begin = pre_info->text_info.cont_vnode_list_begin;
					vnode_list_t *end = pre_info->text_info.cont_vnode_list_end;
					for (; begin; begin = begin->next)
					{
						if (begin->vnode->textSize > 0)
						{
							char *node_text = begin->vnode->hpNode->html_tag.text;
							char *node_text_end = node_text + strlen(node_text) - 1;
							if (*node_text_end == ':')
							{
								clear_pre = true;
								break;
							}
							else
							{
								node_text_end--;
								if (strncmp(node_text_end, "：", 2) == 0)
								{
									clear_pre = true;
									break;
								}
							}
						}
						if (begin == end)
						{
							break;
						}
					}
					if (clear_pre)
					{
						clear_func_area_tag(area->prevArea, AREA_FUNC_ARTICLE_CONTENT);
						debuginfo(MARK_AC, "clear area(id=%d)", area->no);
					}
				}
			}
			debuginfo(MARK_AC, "skip all the left areas after area(id=%d)", area->no);
			return VISIT_FINISH;
		}
	}
	if (is_func_area(area, AREA_FUNC_TURNPAGE) || is_func_area(area, AREA_FUNC_RELATE_LINK))
	{
		if (sc_pack->marked_textSize > 0 && pure_text_rate + 0.02 > max_pure_text_rate)
		{ //遇到翻页块或者相关链接块，且已经标记了主体块，则结束标记
			debuginfo(MARK_AC, "skip all the left areas for it is turn page area(id=%d)", area->no);
			return AREA_VISIT_FINISH;
		}
		else
		{ //遇到翻页块或者相关链接块，但还没有标记到有文字的主体块，则只跳过该分块
			debuginfo(MARK_AC, "skip the area(id=%d) for it is turn page area", area->no);
			return AREA_VISIT_SKIP;
		}
	}
	if (marked_textSize > 0 && pure_text_rate == max_pure_text_rate && valid_text_num == link_num && link_num > 1)
	{
		debuginfo(MARK_AC, "the area(id=%d) is not", area->no);
		return VISIT_SKIP_CHILD;
	}
	if (cn_num == 0 && pic_num == 0)
	{
		debuginfo(MARK_AC, "the area(id=%d) is not", area->no);
		return VISIT_SKIP_CHILD;
	}
	//翻页块和文章签名之间的内容，认为是主体块
	if (sc_pack->turnpage && area->depth == sc_pack->turnpage->depth && area->parentArea && sc_pack->turnpage->parentArea && area->parentArea->no == sc_pack->turnpage->parentArea->no && area->no < sc_pack->turnpage->no)
	{
		if (sc_pack->articlesign && area->depth == sc_pack->articlesign->depth && area->parentArea && sc_pack->articlesign->parentArea && area->parentArea->no == sc_pack->articlesign->parentArea->no && area->no > sc_pack->articlesign->no)
		{
			tag_area_func(area, AREA_FUNC_ARTICLE_CONTENT);
			sc_pack->marked_num++;
			sc_pack->marked_textSize += valid_text_size;
			if (area->depth < 10)
			{
				sc_pack->marked_depth[area->depth]++;
			}
			debuginfo(MARK_AC, "area(id=%d) is for it is between article sign and turn page", area->no);
			return VISIT_SKIP_CHILD;
		}
	}
	if (is_contain_func_area(area, AREA_FUNC_TURNPAGE))
	{
		debuginfo(MARK_AC, "check into area(id=%d) for it contain turn page area", area->no);
		return VISIT_NORMAL;
	}
	if (sc_pack->marked_num == 0 && valid_text_num == 0 && pic_num == 0)
	{
		debuginfo(MARK_AC, "skip the area(id=%d) for it hasn't useful information", area->no);
		return AREA_VISIT_SKIP;
	}
//	if (other_link_num >= 2 && area->baseinfo->text_info.con_size <= other_link_num * 10)
	if (other_link_num >= 2 && valid_text_size <= valid_text_num * 10 && area->valid_subArea_num < 5)
	{
		if (sc_pack->marked_textSize > 20 && sc_pack->marked_textSize > 0 && area->baseinfo->text_info.pure_text_rate + 0.1 > area->area_tree->base_info->max_text_rate_leaf)
		{
			debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it maybe interaction link area", area->no);
			return AREA_VISIT_FINISH;
		}
		else
		{
			debuginfo(MARK_AC, "area(id=%d) is not central for it maybe interaction link area", area->no);
			return AREA_VISIT_SKIP;
		}
	}
	if (area->begin == area->end)
	{
		if (is_h_ul_vnode(area->begin))
		{
			debuginfo(MARK_AC, "skip the area(id=%d) for it maybe link area", area->no);
			return AREA_VISIT_SKIP;
		}
	}
	if (area->valid_subArea_num > 0 && link_num > 0)
	{
		debuginfo(MARK_AC, "check into area(id=%d) for it contain turn page area", area->no);
		return VISIT_NORMAL;
	}
	if (sc_pack->realtit && sc_pack->articlesign && !is_area_has_same_root(area, sc_pack->articlesign))
	{
		int distance = sc_pack->articlesign->area_info.ypos - sc_pack->realtit->area_info.ypos;
		if (area->no >= sc_pack->realtit->no && area->no <= sc_pack->articlesign->no && distance < 100)
		{
			debuginfo(MARK_AC, "skip the area(id=%d) for it is between realtitle and article sign", area->no);
			return AREA_VISIT_SKIP;
		}
	}
	if (sc_pack->marked_num == 0 && anchor_size == text_size && pic_num == 0)
	{
		debuginfo(MARK_AC, "skip the area(id=%d) for it is link", area->no);
		return AREA_VISIT_SKIP;
	}
	if (sc_pack->marked_textSize > 0 && link_num == 0 && text_size <= 10 && text_num == 1)
	{
		char *text = area->baseinfo->text_info.cont_vnode_list_begin->vnode->hpNode->html_tag.text;
		if (strstr(text, "精彩推荐"))
		{
			debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches recommend area", area->no);
			return AREA_VISIT_FINISH;
		}
	}
	//遇到相关链接块，结束主体块标记
	if (is_func_area(area, AREA_FUNC_RELATE_LINK) && sc_pack->marked_num > 0 && area->no > 30 && area->baseinfo->text_info.con_size_before > 0)
	{
		debuginfo(MARK_AC, "skip all the areas after relate link area(id=%d)", area->no);
		return AREA_VISIT_FINISH;
	}
	if (area->depth >= 2 && area->valid_subArea_num > 10 && area->baseinfo->link_info.other_num >= 1)
	{
		debuginfo(MARK_AC, "skip the area(id=%d)", area->no);
		return VISIT_NORMAL;
	}

	unsigned int limit_area_no = 0;
	if (sc_pack->turnpage)
	{
		limit_area_no = sc_pack->turnpage->no;
	}
	if ((is_srctype_area(area, AREA_SRCTYPE_LINK) && area->no >= limit_area_no) || is_srctype_area(area, AREA_SRCTYPE_INTERACTION))
	{
		bool free = false;
		if (area->begin == area->end && area->begin->hpNode->html_tag.tag_type == TAG_TABLE && area->prevArea && area->nextArea)
		{
			free = true;
		}
		if (is_srctype_area(area, AREA_SRCTYPE_PIC) && area->prevArea && area->nextArea)
		{
			free = true;
		}
		if (sc_pack->marked_textSize <= 50)
		{
			free = true;
		}
		if (area->prevArea && (is_func_area(area->prevArea, AREA_FUNC_ARTICLE_CONTENT) || is_contain_func_area(area->prevArea, AREA_FUNC_ARTICLE_CONTENT)) && is_main_table(area))
		{
			free = true;
		}
		if (!free)
		{
			if (sc_pack->marked_textSize > 0 && ypos > 100 && (area->baseinfo->text_info.pure_text_rate + 0.1 > area->area_tree->base_info->max_text_rate_leaf))
			{
				debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it is link or interaction area and ypos is low", area->no);
				return VISIT_FINISH;
			}
			else
			{
				debuginfo(MARK_AC, "skip the area(id=%d) for it is hub or interaction area", area->no);
				return VISIT_SKIP_CHILD;
			}
		}
	}
	if (is_func_area(area, AREA_FUNC_MYPOS))
	{
		debuginfo(MARK_AC, "skip the area(id=%d) for it is mypos area", area->no);
		return AREA_VISIT_SKIP;
	}
	if (is_srctype_area(area, AREA_SRCTYPE_PIC))
	{ //过滤图片交互块
		if (size_fixed_pic_num == pic_num && ((hx < 100 && wx < 100) || script_num > 0))
		{
			debuginfo(MARK_AC, "skip the area(id=%d) for it maybe user info area", area->no);
			return AREA_VISIT_SKIP;
		}
	}
	if (cursor_num > 0 && area->baseinfo->text_info.con_num == cursor_num)
	{
		debuginfo(MARK_AC, "skip the area(id=%d) for it maybe interaction area", area->no);
		return AREA_VISIT_SKIP;
	}
	if (cursor_num >= 2 && area->baseinfo->text_info.con_size <= cursor_num * 10)
	{
		debuginfo(MARK_AC, "area(id=%d) is not central because it maybe interaction link area(cursor num:%d)", area->no, cursor_num);
		return AREA_VISIT_SKIP;
	}
	if (sc_pack->realtit)
	{
		//realtitle之前的分块不被标记成主体块
		int ypos = area->area_info.ypos;
		int realtitle_ypos = sc_pack->realtit->area_info.ypos;
		if (ypos < realtitle_ypos && area->no < sc_pack->realtit->no && (area->depth >= sc_pack->realtit->depth || area->valid_subArea_num == 0))
		{
			debuginfo(MARK_AC, "skip the area(id=%d) for it is above the realtitle area", area->no);
			return AREA_VISIT_SKIP;
		}
		if (area->no - sc_pack->realtit->no <= 5 && (cursor_num >= 2 || other_link_num >= 1) && time_num == 1 && (hx <= 40 || (hx <= 100 && area->baseinfo->text_info.con_size <= 60)))
		{
			debuginfo(MARK_AC, "skip the area(id=%d) for it maybe article sign area", area->no);
			return AREA_VISIT_SKIP;
		}
	}
	if (sc_pack->articlesign)
	{
		if (sc_pack->articlesign->parentArea && area->parentArea && sc_pack->articlesign->parentArea->no == area->parentArea->no)
		{
			if (area->baseinfo->link_info.other_num >= 1 && area->baseinfo->text_info.con_size <= 50)
			{
				debuginfo(MARK_AC, "skip the area(id=%d) for it maybe small interaction area", area->no);
				return AREA_VISIT_SKIP;
			}
		}
	}
	unsigned int limit1 = 0;
	if (sc_pack->articlesign)
	{
		limit1 = sc_pack->articlesign->no;
	}
	if (sc_pack->realtit && sc_pack->realtit->no > limit1)
	{
		limit1 = sc_pack->realtit->no;
	}
	if (area->prevArea == NULL && area->nextArea && is_srctype_area(area->nextArea, AREA_SRCTYPE_LINK) && area->no >= limit1 + 5)
	{
		debuginfo(MARK_AC, "skip the area(id=%d) for it maybe title of link area", area->no);
		return AREA_VISIT_SKIP;
	}
	if (pure_text_rate + 0.02 > max_pure_text_rate)
	{
		if (sc_pack->marked_textSize > 0 && area->area_info.ypos > 800 && area->prevArea && area->prevArea->baseinfo->text_info.cn_num > 1 && !is_func_area(area->prevArea, AREA_FUNC_ARTICLE_CONTENT) && !is_contain_func_area(area->prevArea, AREA_FUNC_ARTICLE_CONTENT))
		{
			debuginfo(MARK_AC, "skip the area(id=%d) for it is not important area", area->no);
			return AREA_VISIT_SKIP;
		}
	}
	if (area->subArea_num == 0 && area->begin == area->end)
	{
		bool flag = false;
		html_vnode_t *vnode = area->begin;
		while (vnode)
		{
			if (vnode->hpNode->html_tag.tag_type == TAG_UL)
			{
				flag = true;
				break;
			}
			if (vnode->firstChild && vnode->firstChild->nextNode == NULL)
			{
				vnode = vnode->firstChild;
			}
			else
			{
				break;
			}
		}
		if (flag && area->baseinfo->link_info.num > 0 && area->baseinfo->text_info.con_size > area->baseinfo->link_info.anchor_size)
		{
			debuginfo(MARK_AC, "skip the area(id=%d) for it maybe tab area", area->no);
			return AREA_VISIT_SKIP;
		}
	}

	mark_area_info_t *mark_info = sc_pack->mark_info;
	if (area->subArea != NULL)
	{
		html_area_t * childarea = NULL;
		for (childarea = area->subArea; childarea; childarea = childarea->nextArea)
		{
			if (childarea && childarea->isValid && ((childarea->baseinfo->text_info.text_area * 10) >= (area->baseinfo->text_info.text_area * 9)) && area->valid_subArea_num > 1)
			{
				debuginfo(MARK_AC, "into the area(id=%d)", area->no);
				return AREA_VISIT_NORMAL;
			}
		}
	}
	/**
	 * 过滤不是核心正文块的区域
	 */
	if (is_to_skip_mark_central(area, mark_info, sc_pack->realtit))
	{
		debuginfo(MARK_AC, "skip the area(id=%d) for is_to_skip_mark_central()", area->no);
		return AREA_VISIT_SKIP;
	}
	/**
	 * 对块的文本长度进行判断，平均长度太短，跳过
	 */
	html_area_t *rootarea = area->area_tree->root;
	int pg_width = rootarea->area_info.width;
	int pg_height = rootarea->area_info.height;
	int page_text_len = rootarea->baseinfo->text_info.con_size;
	int doctype = area->area_tree->hp_vtree->hpTree->doctype;
	/* comment(sue)
	if (area->valid_subArea_num == 0 && (doctype >= doctype_xhtml_BP && doctype <= doctype_html5) && has_proper_tag_len(area, pg_width, pg_height, page_text_len) == 0)
	{
		debuginfo(MARK_AC, "skip the area(id=%d)", area->no);
		return AREA_VISIT_SKIP;
	}
	*/
	if (sc_pack->marked_textSize > 10 && pic_num > 0 && valid_text_size <= pic_num * 5 && !is_srctype_area(area, AREA_SRCTYPE_PIC))
	{
		debuginfo(MARK_AC, "skip the area(id=%d)", area->no);
		return AREA_VISIT_SKIP;
	}
	if (sc_pack->marked_textSize > 0 && anchor_size < 20 && (anchor_size * 2 > valid_text_size || (valid_text_num == 1 && (valid_text_size <= 26 || anchor_size == valid_text_size))))
	{
		if (pure_text_rate + 0.02 > max_pure_text_rate)
		{
			char *text = NULL;
			for (vnode_list_t *list_node = area->baseinfo->text_info.cont_vnode_list_begin; list_node; list_node = list_node->next)
			{
				//TODO effective
				text = list_node->vnode->hpNode->html_tag.text;
				if (strstr(text, "相关热点"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches recommend area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "相关链接"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches recommend area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "返回顶部"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches return top area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "相关专题"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches recommend area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "相关阅读"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches recommend area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "相关新闻"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches recommend area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "更多"))
				{
					if (list_node->vnode->hpNode->html_tag.tag_type == TAG_A)
					{
						debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches recommend area", area->no);
						return VISIT_FINISH;
					}
				}
				else if (strstr(text, "共1页"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches turn page area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "编辑"))
				{
					tag_area_func(area, AREA_FUNC_ARTICLE_CONTENT);
					sc_pack->marked_num++;
					sc_pack->marked_textSize += valid_text_size;
					if (area->depth < 10)
					{
						sc_pack->marked_depth[area->depth]++;
					}
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches editor area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "分享到"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches share area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "发表评论"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches interaction area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "推荐阅读"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches recommend area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "上一篇"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches turn article area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "下一篇"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches turn article area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "更多精彩内容"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches recommend area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "我来补充两句"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches comment area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "更多相关文章"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches recommend area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "精彩推荐"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches recommend area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "关闭窗口") && sc_pack->marked_textSize > 20)
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches close window area", area->no);
					return VISIT_FINISH;
				}
				else if (strstr(text, "正在加载") && sc_pack->marked_textSize > 20)
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches close window area", area->no);
					return VISIT_FINISH;
				}
				if (list_node == area->baseinfo->text_info.cont_vnode_list_end)
				{
					break;
				}
			}
		}
	}
	if (valid_text_num <= 7 && sc_pack->marked_textSize > 0 && pure_text_rate + 0.02 > max_pure_text_rate)
	{
		vnode_list_t *list_begin = area->baseinfo->text_info.cont_vnode_list_begin;
		vnode_list_t *list_end = area->baseinfo->text_info.cont_vnode_list_end;
		if (list_begin != NULL && list_end != NULL)
		{
			for (vnode_list_t *list = list_begin; list; list = list->next)
			{
				if (list->vnode->textSize > 10)
				{
					continue;
				}
				if (strstr(list->vnode->hpNode->html_tag.text, "共1页"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches turn page area", area->no);
					return VISIT_FINISH;
				}
				if (list == list_end)
				{
					break;
				}
			}
		}
	}
	if (valid_text_num == 2 && link_num == 1 && sc_pack->marked_textSize > 0 && pure_text_rate + 0.02 > max_pure_text_rate)
	{
		vnode_list_t *list_begin = area->baseinfo->text_info.cont_vnode_list_begin;
		vnode_list_t *list_end = area->baseinfo->text_info.cont_vnode_list_end;
		if (list_begin != NULL && list_end != NULL)
		{
			for (vnode_list_t *list = list_begin; list; list = list->next)
			{
				if (list->vnode->textSize > 10)
				{
					continue;
				}
				if (strstr(list->vnode->hpNode->html_tag.text, "下一篇"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches next article area", area->no);
					return VISIT_FINISH;
				}
				if (strstr(list->vnode->hpNode->html_tag.text, "共1页"))
				{
					debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches turn page area", area->no);
					return VISIT_FINISH;
				}
				if (list == list_end)
				{
					break;
				}
			}
		}
	}
	int per_text_size = 0;
	int per_cn_num = 0;
	if (valid_text_num > 0)
	{
		per_text_size = valid_text_size / valid_text_num;
		per_cn_num = cn_num / valid_text_num;
	}
	if (per_text_size < 10 && valid_text_num < 3 && area->baseinfo->text_info.recommend_spec_word)
	{
		debuginfo(MARK_AC, "skip the area(id=%d)", area->no);
		return AREA_VISIT_SKIP;
	}
	if (area->begin == area->end)
	{
		if (per_text_size < 20 && valid_text_num == 2)
		{ //是否两个文字在同一行，且其中一个字漂浮在右边
			bool is_in_one_line = is_text_in_one_line(area);
			if (is_in_one_line)
			{
				bool has_float_right = has_float_right_text(area);
				if (has_float_right)
				{
					debuginfo(MARK_AC, "skip the area(id=%d)", area->no);
					return AREA_VISIT_SKIP;
				}
			}
		}
	}
	if (area->begin == area->end && area->begin->firstChild == NULL && area->begin->hpNode->html_tag.tag_type == TAG_DIV)
	{
		debuginfo(MARK_AC, "skip the area(id=%d) for it is empty div", area->no);
		return AREA_VISIT_SKIP;
	}
	//文字数比较多，且字体不止是一个，进入子块check
	if (area->valid_subArea_num > 1)
	{
		if (area->begin == area->end && valid_text_size > 1000 && area->begin->subtree_diff_font > 1)
		{
			debuginfo(MARK_AC, "into the area(id=%d)", area->no);
			return AREA_VISIT_NORMAL;
		}
	}
	if (pure_text_rate == max_pure_text_rate && sc_pack->marked_textSize > 0)
	{
		if (out_link_num > 0)
		{
			if (is_srctype_area(area, AREA_SRCTYPE_PIC))
			{
				debuginfo(MARK_AC, "skip the area(id=%d) for it maybe advertisement area", area->no);
				return AREA_VISIT_SKIP;
			}
		}
	}
	if (is_contain_srctype_area(area, AREA_SRCTYPE_INTERACTION))
	{
		debuginfo(MARK_AC, "check into the area(id=%d) for it contains interaction area", area->no);
		return AREA_VISIT_NORMAL;
	}
	if (sc_pack->marked_textSize == 0 && area->valid_subArea_num == 0 && area->begin == area->end)
	{
		html_vnode_t *vnode = area->begin;
		char *class_value = get_attribute_value(&vnode->hpNode->html_tag, ATTR_CLASS);
		if (class_value && strstr(class_value, "_ad"))
		{
			debuginfo(MARK_AC, "the area(id=%d) is not for it maybe ad area", area->no);
			return AREA_VISIT_SKIP;
		}
	}
	if (sc_pack->marked_textSize == 0 && is_srctype_area(area, AREA_SRCTYPE_HUB) && (area->baseinfo->inter_info.spec_word_num > 0 || (is_srctype_area(area, AREA_SRCTYPE_PIC) && link_num > 3)))
	{
		if ((sc_pack->realtit && area->area_info.ypos == sc_pack->realtitle_bottom) || (sc_pack->articlesign && area->area_info.ypos == sc_pack->articlesign_bottom))
		{
			debuginfo(MARK_AC, "the area(id=%d) is not", area->no);
			return AREA_VISIT_SKIP;
		}
	}
	if (pure_text_rate + 0.01 > max_pure_text_rate && valid_text_num > 3 && per_cn_num <= 5 && cursor_num > 0)
	{
		debuginfo(MARK_AC, "the area(id=%d) is not", area->no);
		return AREA_VISIT_SKIP;
	}
	//免责声明部分不被标记为文章内容块
	if (text_num == 1 && valid_text_size > 20)
	{
		if (strncmp("免责声明：", area->baseinfo->text_info.cont_vnode_list_begin->vnode->hpNode->html_tag.text, 10) == 0)
		{
			debuginfo(MARK_AC, "the area(id=%d) is not", area->no);
			return AREA_VISIT_SKIP;
		}
	}
	debuginfo(MARK_AC, "the area(id=%d) maybe central", area->no);
	if (is_article_content_area(area, mark_info))
	{
		debuginfo(MARK_AC, "the area(id=%d) is", area->no);
		tag_area_func(area, AREA_FUNC_ARTICLE_CONTENT);
		sc_pack->marked_num++;
		sc_pack->marked_textSize += valid_text_size;
		if (area->depth < 10)
		{
			sc_pack->marked_depth[area->depth]++;
		}
		return AREA_VISIT_SKIP;
	}
	debuginfo(MARK_AC, "into the area(id=%d)", area->no);
	return AREA_VISIT_NORMAL;
}

static void clear_subarea_central_tag(html_area_t *area)
{
	for (html_area_t *subarea = area->subArea; subarea; subarea = subarea->nextArea)
	{
		if (!subarea->isValid)
			continue;
		clear_func_area_tag(subarea, AREA_FUNC_ARTICLE_CONTENT);
	}
}

/**
 * @brief 对central子块进行合并，大部分儿子是则父亲是
 */
static int finish_visit_for_mark_central(html_area_t *area, void *data)
{
	if (!area->isValid || area->valid_subArea_num == 0)
	{
		return AREA_VISIT_NORMAL;
	}
	ac_pack_t *sc_pack = (ac_pack_t*) data;
	int sub_tot_area = 0;
	int sub_sem_area = 0;
	unsigned int sub_sem_num = 0;
	unsigned int use_subarea_num = 0;
	bool issamefather = true;
	html_vnode_t * parent = NULL;
	for (html_area_t *subarea = area->subArea; subarea; subarea = subarea->nextArea)
	{
		if (!subarea->isValid)
			continue;
		int this_area = subarea->area_info.width * subarea->area_info.height;
		if (NULL == parent)
		{
			parent = subarea->begin->upperNode;
		}
		else
		{
			if (parent != subarea->begin->upperNode && subarea->begin->upperNode)
			{
				issamefather = false;
			}
		}
		if (this_area > 0)
		{
			use_subarea_num++;
		}
		else
		{
			continue;
		}
		sub_tot_area += this_area;
		if (is_func_area(subarea, AREA_FUNC_ARTICLE_CONTENT))
		{
			sub_sem_area += this_area;
			sub_sem_num++;
		}
		if (this_area > 0)
		{
			use_subarea_num++;
		}
	}
	/**
	 * 对其子分块绝大部分被标为核心正文的块，
	 * 该分块本身也标记为核心正文
	 */
	if (sub_sem_area > 0 && sub_sem_area * 100 >= sub_tot_area * 95 && (sub_tot_area - sub_sem_area <= 30000) && (use_subarea_num * 1 < sub_sem_num * 2) && issamefather)
	{
		int text_size = area->baseinfo->text_info.con_size;
		int valid_text_size = text_size - area->baseinfo->text_info.no_use_con_size;

		clear_subarea_central_tag(area);
		debuginfo(MARK_AC, "mark the area(id=%d) is for most of the subarea are central", area->no);
		tag_area_func(area, AREA_FUNC_ARTICLE_CONTENT);
		sc_pack->marked_num++;
		sc_pack->marked_textSize += valid_text_size;
		if (area->depth < 10)
		{
			sc_pack->marked_depth[area->depth]++;
		}
	}
	if (sc_pack->marked_textSize > 0 && area->valid_subArea_num > 0 && !is_contain_func_area(area, AREA_FUNC_ARTICLE_CONTENT) && area->baseinfo->text_info.pure_text_rate + 0.1 > area->area_tree->base_info->max_text_rate_leaf)
	{
		debuginfo(MARK_AC, "skip all the left areas after area(id=%d) for it reaches a block of non central area", area->no);
		return VISIT_FINISH;
	}

	return AREA_VISIT_NORMAL;
}

static int visit_for_turnpage(html_area_t *area, html_area_t **p_turnpage)
{
	if (!area->isValid || !is_contain_func_area(area, AREA_FUNC_TURNPAGE))
	{
		return AREA_VISIT_SKIP;
	}
	if (is_func_area(area, AREA_FUNC_TURNPAGE))
	{
		*p_turnpage = area;
		return AREA_VISIT_FINISH;
	}
	return AREA_VISIT_NORMAL;
}

static int visit_for_articlesign(html_area_t *area, html_area_t **p_articlesing)
{
	if (!area->isValid || !is_contain_func_area(area, AREA_FUNC_ARTICLE_SIGN))
	{
		return AREA_VISIT_SKIP;
	}
	if (is_func_area(area, AREA_FUNC_ARTICLE_SIGN))
	{
		*p_articlesing = area;
		return AREA_VISIT_FINISH;
	}
	return AREA_VISIT_NORMAL;
}

static int visit_for_realtit(html_area_t *area, html_area_t **p_realtit)
{
	if (!area->isValid || !is_contain_sem_area(area, AREA_SEM_REALTITLE))
	{
		return AREA_VISIT_SKIP;
	}
	if (is_sem_area(area, AREA_SEM_REALTITLE))
	{
		*p_realtit = area;
		return AREA_VISIT_FINISH;
	}
	return AREA_VISIT_NORMAL;
}

/**
 * @brief 遍历area树，找到realtitle块返回
 */
static const html_area_t * get_realtit_area(area_tree_t *atree)
{
	html_area_t *realtit = NULL;
	/**
	 * 获取第一个realtitle块
	 */
	areatree_visit(atree, (FUNC_START_T) visit_for_realtit, NULL, &realtit);
	return realtit;
}

static const html_area_t * get_articlesign_area(area_tree_t *atree)
{
	html_area_t *area = NULL;
	areatree_visit(atree, (FUNC_START_T) visit_for_articlesign, NULL, &area);
	return area;
}

static const html_area_t * get_turnpage_area(area_tree_t *atree)
{
	html_area_t *area = NULL;
	areatree_visit(atree, (FUNC_START_T) visit_for_turnpage, NULL, &area);
	return area;
}

static const html_area_t * get_relate_link_area(area_tree_t *atree)
{
	const area_list_t *alist = get_func_mark_result(atree, AREA_FUNC_RELATE_LINK);
	if (alist == NULL || alist->num != 1)
		return NULL;
	return alist->head->area;
}

bool mark_func_article_content(area_tree_t *atree)
{
	debuginfo_on(MARK_AC);
	timeinit();
	timestart();

	ac_pack_t sc_pack;
	memset(&sc_pack, 0, sizeof(ac_pack_t));
	sc_pack.mark_info = atree->mark_info;
	sc_pack.realtit = get_realtit_area(atree);
	sc_pack.articlesign = get_articlesign_area(atree);
	sc_pack.turnpage = get_turnpage_area(atree);
	sc_pack.relateLink = get_relate_link_area(atree);
	if (sc_pack.realtit)
	{
		sc_pack.realtitle_bottom = sc_pack.realtit->area_info.ypos + sc_pack.realtit->area_info.height;
	}
	else
	{
		sc_pack.realtitle_bottom = -1;
	}
	if (sc_pack.articlesign)
	{
		sc_pack.articlesign_bottom = sc_pack.articlesign->area_info.ypos + sc_pack.articlesign->area_info.height;
	}
	else
	{
		sc_pack.articlesign_bottom = -1;
	}

	areatree_visit(atree, start_visit_for_mark_sem, finish_visit_for_mark_central, &sc_pack);

	timeend("markparser", "article_content");
	dumpdebug(MARK_AC, MARK_AC);
	return true;
}


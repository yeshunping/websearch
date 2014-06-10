/**
 * easou_ahtml_baseinfo.cpp
 * Description: 计算分块的基本信息
 *  Created on: 2011-11-18
 * Last modify: 2012-11-30 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#include <assert.h>
#include <string.h>
#include "chinese.h"
#include "easou_string.h"
#include "easou_url.h"
#include "easou_html_attr.h"
#include "easou_lang_relate.h"
#include "easou_mark_com.h"
#include "easou_mark_inner.h"
#include "easou_mark_baseinfo.h"
#include "debuginfo.h"

/**
 * @brief 初始化baseinfo管理器
 **/
int area_baseinfo_mgr_init(area_baseinfo_mgr_t * area_mgr, int m, int n)
{
	assert(m >=1 && n>=1 && area_mgr != NULL);
	memset(area_mgr, 0, sizeof(area_baseinfo_mgr_t));
	int ret = nodepool_init(&(area_mgr->np_area_out_info), sizeof(area_baseinfo_t), m);
	if (ret == 0)
	{
		goto FAILED;
	}
	ret = nodepool_init(&(area_mgr->np_vnode_list), sizeof(vnode_list_t), n);
	if (ret == 0)
	{
		goto FAILED;
	}
	return 1;
	FAILED: nodepool_destroy(&(area_mgr->np_area_out_info));
	nodepool_destroy(&(area_mgr->np_vnode_list));
	return 0;
}

/**
 * @brief 重置baseinfo管理器
 **/
void area_baseinfo_mgr_reset(area_baseinfo_mgr_t * area_mgr)
{
	nodepool_reset(&(area_mgr->np_area_out_info));
	nodepool_reset(&(area_mgr->np_vnode_list));
	memset(&(area_mgr->all_vnode_list), 0, sizeof(area_mgr->all_vnode_list));
}

/**
 * @brief 销毁baseinfo管理器
 **/
void area_baseinfo_mgr_des(area_baseinfo_mgr_t * area_mgr)
{
	nodepool_destroy(&(area_mgr->np_area_out_info));
	nodepool_destroy(&(area_mgr->np_vnode_list));
	memset(area_mgr, 0, sizeof(area_baseinfo_mgr_t));
	free(area_mgr);
	area_mgr = NULL;
}

/**
 * @brief 重置baseinfo
 **/
void area_baseinfo_reset(area_baseinfo_mgr_t * area_mgr)
{
	nodepool_reset(&(area_mgr->np_area_out_info));
	nodepool_reset(&(area_mgr->np_vnode_list));
	memset(&(area_mgr->all_vnode_list), 0, sizeof(area_mgr->all_vnode_list));
}

/**
 * @brief 获取baseinfo
 **/
area_baseinfo_t * get_area_baseinfo_node(area_baseinfo_mgr_t * area_mgr)
{
	return (area_baseinfo_t *) nodepool_get(&(area_mgr->np_area_out_info));
}

/**
 * @brief 获取baseinfo中的list
 **/
vnode_list_t * get_vnode_list_node(area_baseinfo_mgr_t * area_mgr)
{
	return (vnode_list_t *) nodepool_get(&(area_mgr->np_vnode_list));
}

#define DEFAULT_AREA_OUT_INFO_NUM 100
#define DEFAULT_USE_VNODE_NUM 200

#define COMPUTE_INFO_ERR 0
#define COMPUTE_INFO_OK 1

#define INSERT_EX 0
#define INSERT_IN 1
#define INSERT_PIC 2
#define INSERT_URL 3
#define INSERT_TEXT 5

/**
 * @brief 为area_base_info_mgr申请空间并初始化
 * @parma atree :easou_area_tree_t* 带分块的vtree
 * @return bool 成功/失败
 * @author xunwu
 * @date 2011/07/08
 **/
bool easou_add_out_info(area_tree_t * atree)
{
	assert(atree!=NULL && atree->root !=NULL);
	if (atree->area_binfo_mgr != NULL)
	{
		return true;
	}
	atree->area_binfo_mgr = (area_baseinfo_mgr_t*) malloc(sizeof(area_baseinfo_mgr_t));
	if (atree->area_binfo_mgr == NULL)
	{
		return false;
	}
	memset(atree->area_binfo_mgr, 0, sizeof(area_baseinfo_mgr_t));
	int ret = area_baseinfo_mgr_init(atree->area_binfo_mgr, DEFAULT_AREA_OUT_INFO_NUM, DEFAULT_USE_VNODE_NUM);
	if (ret == 1)
	{
		return true;
	}
	else
	{
		area_baseinfo_mgr_des(atree->area_binfo_mgr);
		return false;
	}
}

void compute_aoi_ex(area_baseinfo_t * pAOI, area_baseinfo_t * p_sub_AOI)
{
	assert(pAOI!=NULL && p_sub_AOI!=NULL);
	//更新extern面积
	pAOI->extern_info.extern_area += p_sub_AOI->extern_info.extern_area;
	//第一个作为开始
	if (pAOI->extern_info.extern_vnode_list_begin == NULL)
	{
		pAOI->extern_info.extern_vnode_list_begin = p_sub_AOI->extern_info.extern_vnode_list_begin;
	}
	//最后一块作为末尾
	if (p_sub_AOI->extern_info.extern_vnode_list_end != NULL)
	{
		pAOI->extern_info.extern_vnode_list_end = p_sub_AOI->extern_info.extern_vnode_list_end;
	}
}
void compute_aoi_in(area_baseinfo_t * pAOI, area_baseinfo_t * p_sub_AOI)
{
	assert(pAOI!=NULL && p_sub_AOI!=NULL);
	//更新input、input_radio、option、select等信息
	pAOI->inter_info.input_num += p_sub_AOI->inter_info.input_num;
	pAOI->inter_info.input_radio_num += p_sub_AOI->inter_info.input_radio_num;
	pAOI->inter_info.option_num += p_sub_AOI->inter_info.option_num;
	pAOI->inter_info.select_num += p_sub_AOI->inter_info.select_num;
	pAOI->inter_info.cursor_num += p_sub_AOI->inter_info.cursor_num;
	pAOI->inter_info.script_num += p_sub_AOI->inter_info.script_num;
	pAOI->inter_info.spec_word_num += p_sub_AOI->inter_info.spec_word_num;
	pAOI->inter_info.input_radio_txtlen += p_sub_AOI->inter_info.input_radio_txtlen;
	pAOI->inter_info.option_txtlen += p_sub_AOI->inter_info.option_txtlen;
	pAOI->inter_info.in_area += p_sub_AOI->inter_info.in_area;
	pAOI->inter_info.is_have_form = pAOI->inter_info.is_have_form || p_sub_AOI->inter_info.is_have_form;
	//第一块作为开始
	if (pAOI->inter_info.interaction_vnode_list_begin == NULL)
	{
		pAOI->inter_info.interaction_vnode_list_begin = p_sub_AOI->inter_info.interaction_vnode_list_begin;
	}
	//左后一块作为结尾
	if (p_sub_AOI->inter_info.interaction_vnode_list_end != NULL)
	{
		pAOI->inter_info.interaction_vnode_list_end = p_sub_AOI->inter_info.interaction_vnode_list_end;
	}
}
void compute_aoi_pic(area_baseinfo_t * pAOI, area_baseinfo_t * p_sub_AOI)
{
	assert(pAOI!=NULL && p_sub_AOI!=NULL);
	//更新pic_area、pic_num、link_pic_num、link_pic_area等信息
	pAOI->pic_info.pic_area += p_sub_AOI->pic_info.pic_area;
	pAOI->pic_info.pic_num += p_sub_AOI->pic_info.pic_num;
	pAOI->pic_info.size_fixed_num += p_sub_AOI->pic_info.size_fixed_num;
	pAOI->pic_info.link_pic_num += p_sub_AOI->pic_info.link_pic_num;
	pAOI->pic_info.link_pic_area += p_sub_AOI->pic_info.link_pic_area;
	if (pAOI->pic_info.pic_vnode_list_begin == NULL)
	{
		pAOI->pic_info.pic_vnode_list_begin = p_sub_AOI->pic_info.pic_vnode_list_begin;
	}
	if (p_sub_AOI->pic_info.pic_vnode_list_end != NULL)
	{
		pAOI->pic_info.pic_vnode_list_end = p_sub_AOI->pic_info.pic_vnode_list_end;
	}
}

void compute_aoi_link(area_baseinfo_t * pAOI, area_baseinfo_t * p_sub_AOI)
{
	pAOI->link_info.num += p_sub_AOI->link_info.num;
	pAOI->link_info.other_num += p_sub_AOI->link_info.other_num;
	pAOI->link_info.inner_num += p_sub_AOI->link_info.inner_num;
	pAOI->link_info.out_num += p_sub_AOI->link_info.out_num;
	pAOI->link_info.anchor_size += p_sub_AOI->link_info.anchor_size;
	pAOI->link_info.link_area += p_sub_AOI->link_info.link_area;
	if (pAOI->link_info.url_vnode_list_begin == NULL)
	{
		pAOI->link_info.url_vnode_list_begin = p_sub_AOI->link_info.url_vnode_list_begin;
	}
	if (p_sub_AOI->link_info.url_vnode_list_end != NULL)
	{
		pAOI->link_info.url_vnode_list_end = p_sub_AOI->link_info.url_vnode_list_end;
	}
}

void compute_aoi_text(area_baseinfo_t * pAOI, area_baseinfo_t * p_sub_AOI)
{
	pAOI->text_info.con_num += p_sub_AOI->text_info.con_num;
	pAOI->text_info.no_use_con_num += p_sub_AOI->text_info.no_use_con_num;
	pAOI->text_info.con_size += p_sub_AOI->text_info.con_size;
	pAOI->text_info.text_area += p_sub_AOI->text_info.text_area;
	pAOI->text_info.no_use_text_area += p_sub_AOI->text_info.no_use_text_area;
	pAOI->text_info.time_num += p_sub_AOI->text_info.time_num;
	pAOI->text_info.cn_num += p_sub_AOI->text_info.cn_num;
	if (pAOI->text_info.cont_vnode_list_begin == NULL)
	{
		pAOI->text_info.cont_vnode_list_begin = p_sub_AOI->text_info.cont_vnode_list_begin;
	}
	if (p_sub_AOI->text_info.cont_vnode_list_end != NULL)
	{
		pAOI->text_info.cont_vnode_list_end = p_sub_AOI->text_info.cont_vnode_list_end;
	}
}

void all_vnode_insert_inner(vnode_list_t ** begin, vnode_list_t **end, vnode_list_t *vnode_list)
{
	assert(begin !=NULL && end!=NULL && vnode_list!=NULL);
	if (*begin == NULL)
	{
		assert(*end==NULL);
		(*begin) = vnode_list;
		(*end) = vnode_list;
		vnode_list->next = NULL;
		vnode_list->pre = NULL;
	}
	else
	{
		assert(*end!=NULL);
		vnode_list->pre = *end;
		(*end)->next = vnode_list;
		vnode_list->next = NULL;
		(*end) = vnode_list;
	}
}

void all_vnode_insert(all_vnode_list_t * all_vnode_list, vnode_list_t * vnode_list, int insert_flag)
{
	assert(all_vnode_list!=NULL && vnode_list !=NULL);
	switch (insert_flag)
	{
	case INSERT_EX:
		all_vnode_insert_inner(&(all_vnode_list->ex_begin), &(all_vnode_list->ex_end), vnode_list);
		break;
	case INSERT_IN:
		all_vnode_insert_inner(&(all_vnode_list->in_begin), &(all_vnode_list->in_end), vnode_list);
		break;
	case INSERT_PIC:
		all_vnode_insert_inner(&(all_vnode_list->pic_begin), &(all_vnode_list->pic_end), vnode_list);
		break;
	case INSERT_URL:
		all_vnode_insert_inner(&(all_vnode_list->url_begin), &(all_vnode_list->url_end), vnode_list);
		break;
	case INSERT_TEXT:
		all_vnode_insert_inner(&(all_vnode_list->text_begin), &(all_vnode_list->text_end), vnode_list);
		break;
	default:
		assert(0);
		break;
	}
}

void add_vnode_info_ex_inner(html_vnode_t * vnode, AOI4ST_extern_t* p_ex)
{
	if (vnode->isValid && (vnode->hpNode->html_tag.tag_type == TAG_IFRAME || vnode->hpNode->html_tag.tag_type == TAG_OBJECT))
	{
		if (vnode->wx <= 0 || vnode->hx <= 0)
		{
//			Debug("wx=%d hx=%d",vnode->wx , vnode->hx ) ;
			return;
		}
		p_ex->extern_area += vnode->wx * vnode->hx;
	}
}

bool is_need_ex_save(html_vnode_t * vnode)
{
	html_tag_type_t tt = vnode->hpNode->html_tag.tag_type;
	if (tt == TAG_IFRAME || tt == TAG_OBJECT)
	{
		return true;
	}
	return false;
}

int add_vnode_info_ex(html_vnode_t * vnode, area_baseinfo_t * aoi, all_vnode_list_t * all_vnode_list, nodepool_t * np_vnode_list)
{
	assert(vnode!=NULL && aoi!=NULL && np_vnode_list !=NULL);
	if (!vnode->isValid)
	{
		return COMPUTE_INFO_OK;
	}

	add_vnode_info_ex_inner(vnode, &aoi->extern_info);
	if (is_need_ex_save(vnode))
	{
		vnode_list_t * tmp = (vnode_list_t *) nodepool_get(np_vnode_list);
		if (tmp == NULL)
		{
			Fatal((char*) "%s %s get node failed!", __FILE__, __FUNCTION__);
			return COMPUTE_INFO_ERR;
		}
		tmp->vnode = vnode;
		tmp->next = NULL;
		tmp->pre = NULL;
		all_vnode_insert(all_vnode_list, tmp, INSERT_EX);
		if (aoi->extern_info.extern_vnode_list_begin == NULL)
		{
			aoi->extern_info.extern_vnode_list_begin = tmp;
		}
		aoi->extern_info.extern_vnode_list_end = tmp;
		return COMPUTE_INFO_OK;
	}
	return COMPUTE_INFO_OK;
}

void add_vnode_info_in_inner(html_vnode_t * vnode, AOI4ST_interaction_t * p_in)
{
	const char * radio = "radio";
	if (vnode == NULL || p_in == NULL)
	{
//		Warn("%s:%s vnode or p_in is NULL!", __FILE__ , __FUNCTION__) ;
		return;
	}
	html_node_t * node = vnode->hpNode;
	assert(node!=NULL);
	if (node->html_tag.tag_type == TAG_PURETEXT && vnode->cn_num <= 6)
	{
		bool flag = false;
		for (int i = 0; for_interaction[i]; i++)
		{
			if (strstr(node->html_tag.text, for_interaction[i]) != NULL)
			{
				flag = true;
				break;
			}
		}
		if (flag)
		{
			p_in->spec_word_num++;
		}
	}
	if (node->html_tag.tag_type == TAG_SELECT)
	{
		p_in->select_num++;
		if (vnode->wx <= 0 || vnode->hx <= 0)
		{
//			Debug("wx=%d hx=%d",vnode->wx , vnode->hx ) ;
		}
		else
		{
			p_in->in_area += vnode->wx * vnode->hx;
		}
	}
	else if (node->html_tag.tag_type == TAG_INPUT)
	{
		p_in->input_num++;
		char * value = get_attribute_value(&(node->html_tag), radio);
		if (value != NULL)
		{
			p_in->input_radio_num++;
			p_in->input_radio_txtlen += vnode->textSize;
		}
		p_in->in_area += vnode->wx * vnode->hx;
	}
	else if (node->html_tag.tag_type == TAG_OPTION)
	{
		p_in->option_num++;
		p_in->option_txtlen += vnode->textSize;
	}
	else if (node->html_tag.tag_type == TAG_FORM)
	{
		p_in->is_have_form = true;
	}
	else if (node->html_tag.tag_type == TAG_TEXTAREA)
	{
		p_in->textarea_num++;
	}
	else
	{
		const char *cursor_value = get_css_attribute(vnode, CSS_PROP_CURSOR);
		if (cursor_value)
		{
			p_in->cursor_num++;
		}
	}
}

int add_vnode_info_in(html_vnode_t * vnode, area_baseinfo_t * aoi, all_vnode_list_t * all_vnode_list, nodepool_t * np_vnode_list)
{
	assert(vnode!=NULL && aoi!=NULL && np_vnode_list !=NULL);
	if (!vnode->isValid)
	{
		if (vnode->hpNode->html_tag.tag_type == TAG_SCRIPT)
		{
			aoi->inter_info.script_num++;
		}
		return COMPUTE_INFO_OK;
	}
	add_vnode_info_in_inner(vnode, &aoi->inter_info);
	if (interaction_tag_types[vnode->hpNode->html_tag.tag_type])
	{
		vnode_list_t * tmp = (vnode_list_t *) nodepool_get(np_vnode_list);
		if (tmp == NULL)
		{
			Fatal((char*) "%s %s get node failed!", __FILE__, __FUNCTION__);
			return COMPUTE_INFO_ERR;
		}
		tmp->vnode = vnode;
		tmp->next = NULL;
		tmp->pre = NULL;
		all_vnode_insert(all_vnode_list, tmp, INSERT_IN);
		if (aoi->inter_info.interaction_vnode_list_begin == NULL)
		{
			aoi->inter_info.interaction_vnode_list_begin = tmp;
		}
		aoi->inter_info.interaction_vnode_list_end = tmp;
		return COMPUTE_INFO_OK;
	}
	return COMPUTE_INFO_OK;
}

bool is_tagA_child(html_vnode_t * vnode)
{
	assert(vnode!=NULL);
	vnode = vnode->upperNode;
	int i;
	for (i = 0; i < 2; i++)
	{
		if (vnode == NULL)
		{
			break;
		}
		else if (vnode->hpNode->html_tag.tag_type == TAG_A)
		{
			return true;
		}
		vnode = vnode->upperNode;
	}
	return false;
}

void add_vnode_info_pic_inner(html_vnode_t * vnode, AOI4ST_pic_t * p_pic)
{
	bool is_a_child = false;
	if (vnode->hpNode->html_tag.tag_type == TAG_IMG)
	{
		if (vnode->wx < 90 * vnode->hx && vnode->hx < 8 * vnode->wx && vnode->wx * vnode->hx > 100)
		{
			p_pic->pic_num++;
			char *width_value = get_css_attribute(vnode, CSS_PROP_WIDTH);
			if (width_value == NULL)
			{
				width_value = get_attribute_value(&vnode->hpNode->html_tag, ATTR_WIDTH);
			}
			char *height_value = get_css_attribute(vnode, CSS_PROP_HEIGHT);
			if (height_value == NULL)
			{
				height_value = get_attribute_value(&vnode->hpNode->html_tag, ATTR_HEIGHT);
			}
			if (width_value && height_value)
			{
				p_pic->size_fixed_num++;
			}
		}
		is_a_child = is_tagA_child(vnode);
		if (is_a_child)
		{

			p_pic->link_pic_num++;
		}
		if (vnode->wx < 0 || vnode->hx < 0)
		{
//			Debug("wx=%d hx=%d" ,vnode->wx , vnode->hx ) ;
			return;
		}
		p_pic->pic_area += vnode->wx * vnode->hx;
		if (is_a_child)
		{
			p_pic->link_pic_area += vnode->wx * vnode->hx;
		}
	}
}

bool is_need_pic_save(html_vnode_t * vnode)
{
	if (vnode->hpNode->html_tag.tag_type == TAG_IMG)
	{
		return true;
	}
	return false;
}

int add_vnode_info_pic(html_vnode_t * vnode, area_baseinfo_t * aoi, all_vnode_list_t * all_vnode_list, nodepool_t * np_vnode_list)
{
	assert(vnode!=NULL && aoi!=NULL && np_vnode_list !=NULL);
	if (!vnode->isValid)
	{
		return COMPUTE_INFO_OK;
	}
	add_vnode_info_pic_inner(vnode, &aoi->pic_info);
	if (is_need_pic_save(vnode))
	{
		vnode_list_t * tmp = (vnode_list_t *) nodepool_get(np_vnode_list);
		if (tmp == NULL)
		{
			Fatal((char*) "%s %s get node failed!", __FILE__, __FUNCTION__);
			return COMPUTE_INFO_ERR;
		}
		tmp->vnode = vnode;
		tmp->next = NULL;
		tmp->pre = NULL;
		all_vnode_insert(all_vnode_list, tmp, INSERT_PIC);
		if (aoi->pic_info.pic_vnode_list_begin == NULL)
		{
			aoi->pic_info.pic_vnode_list_begin = tmp;
		}
		aoi->pic_info.pic_vnode_list_end = tmp;
		return COMPUTE_INFO_OK;
	}
	return COMPUTE_INFO_OK;
}

void add_vnode_info_link_inner(html_vnode_t * vnode, AOI4ST_link_t * p_link, area_tree_t *atree)
{
	assert(vnode->isValid);
	if (vnode->hpNode->html_tag.tag_type == TAG_A)
	{
		char * phref = get_attribute_value(&vnode->hpNode->html_tag, ATTR_HREF);
		if (phref == NULL)
		{
			p_link->other_num++;
		}
		else if (vnode->hx == 0 || vnode->wx == 0)
		{
			p_link->other_num++;
		}
		else
		{
			if (strncasecmp(phref, "javascript:", strlen("javascript:")) == 0 || strncasecmp(phref, "mailto:", strlen("mailto:")) == 0 || (phref[0] == '#' && phref[1] == '\0'))
			{
				p_link->other_num++;
			}
			else
			{
				p_link->num++;
			}
			int ltype = get_href_type(phref, atree->mark_info->page_url);
			if (ltype == IS_LINK_INNER)
			{
				p_link->inner_num++;
			}
			else if (ltype == IS_LINK_OUT)
			{
				p_link->out_num++;
			}
		}
		p_link->anchor_size += vnode->subtree_textSize;
		p_link->link_area += vnode->wx * vnode->hx;
	}
}

bool is_need_link_save(html_vnode_t * vnode)
{
	if (vnode->hpNode->html_tag.tag_type == TAG_A && vnode->wx > 0 && vnode->hx > 0)
	{
		return true;
	}
	return false;
}

int add_vnode_info_link(html_vnode_t * vnode, area_baseinfo_t * aoi, all_vnode_list_t * all_vnode_list, nodepool_t * np_vnode_list, area_tree_t *atree)
{
	assert(vnode!=NULL && aoi!=NULL && np_vnode_list !=NULL);
	if (vnode->isValid == false)
	{
		return COMPUTE_INFO_OK;
	}
	add_vnode_info_link_inner(vnode, &aoi->link_info, atree);
	if (is_need_link_save(vnode))
	{
		vnode_list_t * tmp_url = (vnode_list_t *) nodepool_get(np_vnode_list);
		if (tmp_url == NULL)
		{
			Fatal((char*) "%s %s get node failed!", __FILE__, __FUNCTION__);
			return COMPUTE_INFO_ERR;
		}
		tmp_url->vnode = vnode;
		tmp_url->next = NULL;
		tmp_url->pre = NULL;
		all_vnode_insert(all_vnode_list, tmp_url, INSERT_URL);
		if (aoi->link_info.url_vnode_list_begin == NULL)
		{
			aoi->link_info.url_vnode_list_begin = tmp_url;
		}
		aoi->link_info.url_vnode_list_end = tmp_url;
		return COMPUTE_INFO_OK;
	}
	return COMPUTE_INFO_OK;
}

static bool is_nouse_cont_tag(html_vnode_t * vnode)
{
	assert( vnode->hpNode->html_tag.tag_type == TAG_PURETEXT);
	char * p_text = vnode->hpNode->html_tag.text;
	while (*p_text)
	{
		if (*p_text == ' ' || *p_text == '\t' || *p_text == '\n' || *p_text == '\r' || *p_text == '|')
		{
			p_text++;
		}
		else if (IS_GB_SPACE(p_text))
		{
			p_text += 2;
		}
		else if (strncmp(p_text, "&nbsp;", strlen("&nbsp;")) == 0)
		{
			p_text += strlen("&nbsp;");
		}
		else
		{
			return false;
		}
	}
	return true;
}

void add_vnode_info_text_inner(html_vnode_t * vnode, AOI4ST_text_t * p_text)
{
	if (vnode->hpNode->html_tag.tag_type == TAG_PURETEXT)
	{
		p_text->con_num++;
		if (is_nouse_cont_tag(vnode))
		{
			p_text->no_use_con_num++;
			p_text->no_use_con_size += vnode->textSize;
			p_text->no_use_text_area += vnode->wx * vnode->hx;
		}
		if (is_time_vnode(vnode))
		{
			p_text->time_num++;
		}
		if (p_text->recommend_spec_word == false && vnode->textSize <= 10)
		{
			bool ret = is_has_special_word(for_rel_link, vnode->hpNode->html_tag.text);
			p_text->recommend_spec_word = ret;
		}
		p_text->con_size += vnode->textSize;
		p_text->cn_num += vnode->subtree_cn_num;
		if (vnode->wx < 0 || vnode->hx < 0)
		{
			return;
		}
		p_text->text_area += vnode->wx * vnode->hx;
	}
}

bool is_need_text_save(html_vnode_t * vnode)
{
	if (vnode->hpNode->html_tag.tag_type == TAG_PURETEXT)
	{
		return true;
	}
	return false;
}

int add_vnode_info_text(html_vnode_t * vnode, area_baseinfo_t * aoi, all_vnode_list_t * all_vnode_list, nodepool_t * np_vnode_list)
{
	assert(vnode!=NULL && aoi!=NULL && np_vnode_list != NULL);
	if (vnode->isValid == false)
	{
		return COMPUTE_INFO_OK;
	}
	add_vnode_info_text_inner(vnode, &aoi->text_info);
	if (is_need_text_save(vnode))
	{
		vnode_list_t * tmp = (vnode_list_t *) nodepool_get(np_vnode_list);
		if (tmp == NULL)
		{
			Fatal((char*) "%s %s get node failed!", __FILE__, __FUNCTION__);
			return COMPUTE_INFO_ERR;
		}
		tmp->vnode = vnode;
		tmp->next = NULL;
		tmp->pre = NULL;
		all_vnode_insert(all_vnode_list, tmp, INSERT_TEXT);
		if (aoi->text_info.cont_vnode_list_begin == NULL)
		{
			aoi->text_info.cont_vnode_list_begin = tmp;
		}
		aoi->text_info.cont_vnode_list_end = tmp;
		return COMPUTE_INFO_OK;
	}
	return COMPUTE_INFO_OK;

}

int add_vnode_info(html_vnode_t * vnode, area_baseinfo_t * aoi, all_vnode_list_t * all_vnode_list, nodepool_t * np_vnode_list, area_tree_t *atree)
{
	assert(vnode !=NULL && aoi!=NULL && all_vnode_list !=NULL);
	int ret = 0;
	if (vnode->hpNode->html_tag.tag_type != TAG_IFRAME)
	{
		html_vnode_t * vchild = NULL;
		vchild = vnode->firstChild;
		for (; vchild != NULL; vchild = vchild->nextNode)
		{
			if (COMPUTE_INFO_ERR == add_vnode_info(vchild, aoi, all_vnode_list, np_vnode_list, atree))
			{
				return COMPUTE_INFO_ERR;
			}
		}
	}

	ret = add_vnode_info_ex(vnode, aoi, all_vnode_list, np_vnode_list);
	if (ret == COMPUTE_INFO_ERR)
		goto FAILED;
	ret = add_vnode_info_in(vnode, aoi, all_vnode_list, np_vnode_list);
	if (ret == COMPUTE_INFO_ERR)
		goto FAILED;
	ret = add_vnode_info_pic(vnode, aoi, all_vnode_list, np_vnode_list);
	if (ret == COMPUTE_INFO_ERR)
		goto FAILED;
	ret = add_vnode_info_link(vnode, aoi, all_vnode_list, np_vnode_list, atree);
	if (ret == COMPUTE_INFO_ERR)
		goto FAILED;
	ret = add_vnode_info_text(vnode, aoi, all_vnode_list, np_vnode_list);
	if (ret == COMPUTE_INFO_ERR)
		goto FAILED;

	return COMPUTE_INFO_OK;
	FAILED: return COMPUTE_INFO_ERR;
}

int compute_leaf_area_info(html_area_t * area, area_baseinfo_mgr_t* area_mgr, area_tree_t *atree)
{
	assert(area!=NULL && area->subArea ==NULL);
	memset(area->baseinfo, 0, sizeof(area_baseinfo_t));
	html_vnode_t * vnode = area->begin;
	while (1)
	{
		if (vnode == NULL)
		{
			break;
		}
		if (COMPUTE_INFO_ERR == add_vnode_info(vnode, (area_baseinfo_t *) area->baseinfo, &area_mgr->all_vnode_list, &area_mgr->np_vnode_list, atree))
		{
			return COMPUTE_INFO_ERR;
		}
		if (vnode == area->end)
		{
			break;
		}
		vnode = vnode->nextNode;
	}
	return COMPUTE_INFO_OK;
}

/**
 * @brief 填充easou_html_area_t的baseinfo属性
 * @param [in/out] area   : easou_html_area_t*	待标注的分块.
 * @param [in/out] area_mgr   : area_baseinfo_mgr_t*	资源管理器.
 * @param [in/out] atree   : easou_area_tree_t*	area所属的vtree.
 * @return  bool	是否成功.
 **/
static int compute_area_info(html_area_t * area, area_baseinfo_mgr_t * area_mgr, area_tree_t *atree)
{
	assert(area!=NULL && area->baseinfo !=NULL && area_mgr !=NULL);
	//非叶子分块
	if (area->subArea != NULL)
	{
		//非叶子节点的信息可以通过子孙节点来累加
		memset(area->baseinfo, 0, sizeof(area_baseinfo_t));
		area_baseinfo_t * pAOI = (area_baseinfo_t *) area->baseinfo;
		for (html_area_t * sub_area = area->subArea; sub_area != NULL; sub_area = sub_area->nextArea)
		{
			if (sub_area->isValid == false)
			{
				continue;
			}
			area_baseinfo_t * p_sub_AOI = (area_baseinfo_t *) sub_area->baseinfo;
			//extern
			compute_aoi_ex(pAOI, p_sub_AOI);
			//inter
			compute_aoi_in(pAOI, p_sub_AOI);
			//pic
			compute_aoi_pic(pAOI, p_sub_AOI);
			//link
			compute_aoi_link(pAOI, p_sub_AOI);
			//text
			compute_aoi_text(pAOI, p_sub_AOI);
		}
	}
	else
	{
		//叶子分块
		assert(area->isValid == true);
		if (compute_leaf_area_info(area, area_mgr, atree) == COMPUTE_INFO_ERR)
		{
			return COMPUTE_INFO_ERR;
		}
	}
	return COMPUTE_INFO_OK;
}

static void clean_area_inner(html_area_t * area)
{
	area->abspos_mark = PAGE_UNDEFINED;
	area->srctype_mark._mark_bits = 0;
	area->func_mark._mark_bits = 0;
	area->sem_mark._mark_bits = 0;
	area->baseinfo = NULL;
}

static int finish_visit_for_fill_info(html_area_t *area, void * data)
{
	clean_area_inner(area);
	if (area->isValid == false)
	{
		return AREA_VISIT_NORMAL;
	}
	area_baseinfo_mgr_t * aoi_mgr = (area_baseinfo_mgr_t *) (((void**) data)[0]);
	area_baseinfo_t * aoi_info = get_area_baseinfo_node(aoi_mgr);
	area_tree_t * atree = (area_tree_t *) (((void **) data)[2]);
	area->baseinfo = aoi_info;
	//shuangwei add 20120604
	if (area->valid_subArea_num == 0)
	{
		SET_LEAF_AREA(area->areaattr);
	}
	compute_area_info(area, aoi_mgr, atree);
	return AREA_VISIT_NORMAL;
}

typedef struct _add_info_t
{
	int con_size_before;
	int anchor_size_before;
} add_info_t;

static void update_before(int &val_before, int &cumu_val_before, const int &my_val, const int &valid_sub_num)
{
	val_before = cumu_val_before;

	if (valid_sub_num == 0)
	{
		cumu_val_before += my_val;
	}
}

static void add_title_for_area(html_area_t *area)
{
	if (!area->isValid)
		return;
	html_area_t *titlearea = area->prevArea;
	html_area_t *parent = area->parentArea;
	bool isfind = false;
	int count = 0;

	marktreeprintfs(MARK_AREA_TITLE, "\n*******************************\narea no=%d, nodeTypeOfArea=%x,subareanum=%d at %s(%d)-%s\r\n", area->no, area->nodeTypeOfArea, area->subArea_num, __FILE__, __LINE__, __FUNCTION__);
//	if(area->baseinfo->text_info.con_size<100||area->subArea&&area->subArea->baseinfo->text_info.con_size<32||area->subArea&&area->subArea->subArea||area->subArea&&area->subArea->nextArea&&area->subArea->nextArea->subArea){
//		return;
//	}
	if (MARK_AREA_TITLE == g_EASOU_DEBUG)
	{
		if (area->prevArea)
		{
			myprintf("pre area no is:%d\n", area->prevArea->no);
		}

	}
	while (titlearea && count < 3 && area->baseinfo->text_info.cont_vnode_list_begin && (area->nodeTypeOfArea & 16 || (area->srctype_mark._mark_bits & 24 == 24) && (area->subArea_num < 1)))
	{
		marktreeprintfs(MARK_AREA_TITLE, "may be title no=%d, nodeTypeOfArea=%x  at %s(%d)-%s\r\n", titlearea->no, titlearea->nodeTypeOfArea, __FILE__, __LINE__, __FUNCTION__);
		vnode_list_t *tvnodelist = titlearea->baseinfo->text_info.cont_vnode_list_begin;
		vnode_list_t *avnodelist = area->baseinfo->text_info.cont_vnode_list_begin;
		if (MARK_AREA_TITLE == g_EASOU_DEBUG && tvnodelist && tvnodelist->vnode)
		{
			myprintf("title font:\n");
			print_font(&(tvnodelist->vnode->font));
		}
		if (MARK_AREA_TITLE == g_EASOU_DEBUG && avnodelist && avnodelist->vnode)
		{
			myprintf("area font:\n");
			print_font(&(avnodelist->vnode->font));
		}
		if (((titlearea->nodeTypeOfArea & 4) || tvnodelist && (tvnodelist->vnode->font.is_bold || tvnodelist->vnode->font.is_strong || tvnodelist->vnode->font.size > avnodelist->vnode->font.size || tvnodelist->vnode->font.color != avnodelist->vnode->font.color))
				&& area->baseinfo->link_info.num >= 2)
		{

			if (titlearea->parentArea && titlearea->parentArea == parent)
			{
				int avalen = titlearea->baseinfo->text_info.con_size;
				if (titlearea->baseinfo->link_info.num)
				{
					avalen = titlearea->baseinfo->link_info.anchor_size / (titlearea->baseinfo->link_info.num);
				}
				marktreeprintfs(MARK_AREA_TITLE, "may be title no=%d, con_size=%d,anchor_size=%d ,avalen=%d at %s(%d)-%s\r\n", titlearea->no, titlearea->baseinfo->text_info.con_size, titlearea->baseinfo->link_info.anchor_size, avalen, __FILE__, __LINE__, __FUNCTION__);
				if ((avalen < 20 && (titlearea->baseinfo->text_info.con_size < 64 && titlearea->baseinfo->link_info.anchor_size < 64)) && (titlearea->area_info.ypos < area->area_info.ypos) && (titlearea->area_info.xpos == area->area_info.xpos))
				{
					area->titleArea = titlearea;
					isfind = true;
					break;
				}
			}

		}
		titlearea = titlearea->prevArea;
		count++;
	}
	if (!isfind)
	{
		if (parent && ((area->prevArea) == NULL))
		{
			titlearea = parent->prevArea;
			parent = parent->parentArea;
			count = 0;
			marktreeprintfs(MARK_AREA_TITLE, "parent area no=%d, at %s(%d)-%s\r\n", parent?parent->no:-1, __FILE__, __LINE__, __FUNCTION__);
			while (titlearea && count < 3 && area->baseinfo->text_info.cont_vnode_list_begin && (area->nodeTypeOfArea & 16 || ((area->srctype_mark._mark_bits & 24) == 24) && (area->subArea_num < 1)))
			{
				marktreeprintfs(MARK_AREA_TITLE, "may be title no=%d, nodeTypeOfArea=%x  at %s(%d)-%s\r\n", titlearea->no, titlearea->nodeTypeOfArea, __FILE__, __LINE__, __FUNCTION__);
				vnode_list_t *tvnodelist = titlearea->baseinfo->text_info.cont_vnode_list_begin;
				vnode_list_t *avnodelist = area->baseinfo->text_info.cont_vnode_list_begin;
				if (MARK_AREA_TITLE == g_EASOU_DEBUG && tvnodelist && tvnodelist->vnode)
				{
					myprintf("title font:\n");
					print_font(&(tvnodelist->vnode->font));
				}
				if (MARK_AREA_TITLE == g_EASOU_DEBUG && avnodelist && avnodelist->vnode)
				{
					myprintf("area font:\n");
					print_font(&(avnodelist->vnode->font));
				}
				if (((titlearea->nodeTypeOfArea & 4) || tvnodelist && (tvnodelist->vnode->font.is_bold || tvnodelist->vnode->font.is_strong || tvnodelist->vnode->font.size > avnodelist->vnode->font.size || tvnodelist->vnode->font.color != avnodelist->vnode->font.color))
						&& area->baseinfo->link_info.num >= 2)
				{

					if (titlearea->parentArea && titlearea->parentArea == parent)
					{
						int avalen = titlearea->baseinfo->text_info.con_size;
						if (titlearea->baseinfo->link_info.num)
						{
							avalen = titlearea->baseinfo->link_info.anchor_size / (titlearea->baseinfo->link_info.num);
						}
						marktreeprintfs(MARK_AREA_TITLE, "may be title no=%d, con_size=%d,anchor_size=%d ,avalen=%d at %s(%d)-%s\r\n", titlearea->no, titlearea->baseinfo->text_info.con_size, titlearea->baseinfo->link_info.anchor_size, avalen, __FILE__, __LINE__, __FUNCTION__);
						if ((avalen < 20 && (titlearea->baseinfo->text_info.con_size < 64 && titlearea->baseinfo->link_info.anchor_size < 64)) && (titlearea->area_info.ypos < area->area_info.ypos) && (titlearea->area_info.xpos == area->area_info.xpos))
						{
							area->titleArea = titlearea;
							isfind = true;
							break;
						}
					}

				}
				titlearea = titlearea->prevArea;
				count++;
			}
		}
	}

}

static int visit_for_add_info(html_area_t *area, add_info_t *add_info)
{
	if (!area->isValid)
		return AREA_VISIT_SKIP;

	update_before(area->baseinfo->text_info.con_size_before, add_info->con_size_before, area->baseinfo->text_info.con_size, area->valid_subArea_num);

	update_before(area->baseinfo->link_info.anchor_size_before, add_info->anchor_size_before, area->baseinfo->link_info.anchor_size, area->valid_subArea_num);

	add_title_for_area(area);

	if (area->baseinfo->text_info.con_size_before == 0)
	{
		area->baseinfo->text_info.pure_text_rate = 0;
	}
	else
	{
		area->baseinfo->text_info.pure_text_rate = (float) (area->baseinfo->text_info.con_size_before - area->baseinfo->link_info.anchor_size_before) / (float) area->baseinfo->text_info.con_size_before;
	}
	if (area->baseinfo->text_info.pure_text_rate < 1)
	{
		if (area->baseinfo->text_info.pure_text_rate > area->area_tree->base_info->max_text_rate)
		{
			area->area_tree->base_info->max_text_rate = area->baseinfo->text_info.pure_text_rate;
			area->area_tree->base_info->max_text_area_no = area->no;
		}
		if (area->valid_subArea_num == 0)
		{
			if (area->baseinfo->text_info.pure_text_rate > area->area_tree->base_info->max_text_rate_leaf)
			{
				area->area_tree->base_info->max_text_rate_leaf = area->baseinfo->text_info.pure_text_rate;
				area->area_tree->base_info->max_text_leaf_area_no = area->no;
			}
		}
	}
	return AREA_VISIT_NORMAL;
}

/**
 * @brief 填充atree的area_binfo_mgr属性.并完善每个area中的baseinfo
 * @param [in/out] atree   : easou_area_tree_t*	待标注的atree.
 * @return  bool	是否成功.
 **/
bool fill_base_info(area_tree_t * atree)
{
	assert(atree !=NULL && atree->mark_info != NULL);
	//如果已经标注完成，直接返回
	if (IS_COMPUTED_INFO(atree->mark_status))
	{
		return true;
	}
	if (atree->area_binfo_mgr == NULL)
	{
		if (false == easou_add_out_info(atree))
		{
			return false;
		}
	}
	area_baseinfo_reset(atree->area_binfo_mgr);
	void * p_inner[3];
	p_inner[0] = atree->area_binfo_mgr;
	p_inner[2] = atree;
	if (false == areatree_visit(atree, NULL, finish_visit_for_fill_info, p_inner))
	{
		goto FAILED;
	}
	add_info_t ainfo;
	ainfo.con_size_before = 0;
	ainfo.anchor_size_before = 0;
	//更新文本信息中的con_size_before和 anchor_size_before两个属性，即当前块之前的锚文和纯文本的数量和长度
	areatree_visit(atree, (FUNC_START_T) visit_for_add_info, NULL, &ainfo);
	SET_COMPUTED_INFO(atree->mark_status);
	return true;
	FAILED: return false;
}

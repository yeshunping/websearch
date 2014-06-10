/**
 * easou_mark_articlesign.cpp
 * Description: 标记文章签名块（提供文章时间、来源、作者等信息的块）
 *  Created on: 2011-11-24
 * Last modify: 2012-10-26 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#include "chinese.h"
#include "easou_lang_relate.h"
#include "easou_mark_baseinfo.h"
#include "easou_mark_srctype.h"
#include "easou_mark_com.h"
#include "easou_mark_func.h"
#include "easou_mark_sem.h"
#include "debuginfo.h"
static const int TEXT_BUF_LEN = 256;

static const int ARTI_SIGN_TIME_FEAT_VAL = 1;
static const int ARTI_SIGN_ALIGN_CENTER_VAL = 1;
static const int ARTI_SIGN_THRESHOLD = 3;

const int ARTI_SIGN_FEAT_TERM_VAL[] = {2,1,1,1,2,2,2,1,1,1,1,1,1,1,0};

//反词
const char* anti_articlesign_term[] = {"举报", "发表时间", "所属专辑"};
int anti_articlesign_term_num = sizeof(anti_articlesign_term)/sizeof(anti_articlesign_term[0]);

typedef struct _marking_info_t{
	int plus;
	bool is_center;
}marking_info_t;

typedef struct _articlesign_pack_t
{
	mark_area_info_t *mark_info;
	const html_area_t *realtit;
} articlesign_pack_t;

/**
 * @brief 计算文本中含有的热词热度值
 */
static int get_feat_term_value(const char *text)
{
	int val = 0;

	for (int i = 0; ARTI_SIGN_FEAT_TERM[i]; i++)
	{
		if (strstr(text, ARTI_SIGN_FEAT_TERM[i]) != NULL)
		{
			marktreeprintfs(MARK_ARTICLESIGN, "article sign march word=%s at %s(%d)-%s\r\n", ARTI_SIGN_FEAT_TERM[i], __FILE__, __LINE__, __FUNCTION__);
			val += ARTI_SIGN_FEAT_TERM_VAL[i];
		}
	}
	//减分词
	for (int i = 0; i < anti_articlesign_term_num; i++)
	{
		if (strstr(text, anti_articlesign_term[i]) != NULL)
		{
			val--;
		}
	}
	return val;
}

/**
 * 计算字符串含有时间的度值
 */
static int get_time_str_value(const char *text)
{
	for(const char *p = text; *p ; p += GET_CHR_LEN(p)){
		if(isdigit(*p) && is_time_str(p)){
			marktreeprintfs(MARK_ARTICLESIGN,"%s contains time  at %s(%d)-%s\r\n",text,__FILE__,__LINE__,__FUNCTION__);
			return ARTI_SIGN_TIME_FEAT_VAL;}
	}

	return 0;
}

static int get_text_feat_value(const char *text)
{
	int val = 0;
	val += get_feat_term_value(text);
	return val;
}

static int visit_for_arti_sign(html_vnode_t *vnode, void *data)
{
	if (!vnode->isValid)
		return VISIT_SKIP_CHILD;

	marking_info_t *minfo = (marking_info_t *) data;
	if (vnode->hpNode->html_tag.tag_type == TAG_INPUT || vnode->hpNode->html_tag.tag_type == TAG_SELECT || vnode->hpNode->html_tag.tag_type == TAG_TEXTAREA || vnode->hpNode->html_tag.tag_type == TAG_OPTION)
	{
		minfo->plus = -1000;
		return VISIT_FINISH;
	}

	if (vnode->hpNode->html_tag.tag_type != TAG_PURETEXT)
		return VISIT_NORMAL;

	if (vnode->font.align != VHP_TEXT_ALIGN_CENTER)
		minfo->is_center = false;

	char text[TEXT_BUF_LEN];
	text[0] = '\0';

	copy_html_text(text, 0, sizeof(text) - 1, vnode->hpNode->html_tag.text);

	minfo->plus += get_text_feat_value(text);

	return VISIT_NORMAL;
}

static int visit_for_mark_article_sign_area(html_area_t * area, articlesign_pack_t* sc_pack)
{
	mark_area_info_t *mark_info = sc_pack->mark_info;
	if(MARK_ARTICLESIGN==g_EASOU_DEBUG){
		printlines("mark article sign");
		printNode(area->begin->hpNode);
	}
	if(!area->isValid){
		marktreeprintfs(MARK_ARTICLESIGN,"area(id=%d) is not valid ,it is not article sign and skip it at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP;
	}
	// shuangwei add 20120511
	if (!IS_LEAF_AREA(area->areaattr))
	{
		marktreeprintfs(MARK_ARTICLESIGN, "area(id=%d) is not leaf area, it is not article sign at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_NORMAL;
	}
	if(area->depth == 1){
		if(area->pos_mark == RELA_LEFT
			|| area->pos_mark == RELA_RIGHT
			|| area->pos_mark == RELA_FOOTER){
			marktreeprintfs(MARK_ARTICLESIGN,"the depth=1 and pos_mark(%d) is rela_left,rela_right,rela_footer ,area(id=%d) is not article sign and skip it at %s(%d)-%s\r\n", area->no, area->pos_mark,__FILE__,__LINE__,__FUNCTION__);
			return AREA_VISIT_SKIP;
		}
	}

	if(is_in_func_area(area,AREA_FUNC_MYPOS)
			|| is_in_func_area(area,AREA_FUNC_NAV)
			|| is_in_func_area(area,AREA_FUNC_FRIEND)
			|| is_in_func_area(area,AREA_FUNC_RELATE_LINK)){
		marktreeprintfs(MARK_ARTICLESIGN,"the area is in mypos or nav or friend or relate link ,its upper_func_mark=%x,area(id=%d) is not article sign and skip it at %s(%d)-%s\r\n", area->no, area->upper_func_mark._mark_bits,__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP;
	}

	if(area->baseinfo->text_info.con_size <= 8){
		marktreeprintfs(MARK_ARTICLESIGN,"the text size of the area is <=8 ,area(id=%d) is not article sign and skip it at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP;}

	/*
	if (!is_single_inline(area))
	{
		marktreeprintfs(MARK_ARTICLESIGN, "area(id=%d) is not single in line, it is not article sign at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_NORMAL;
	}
	*/

	area_tree_t *atree = mark_info->area_tree;
	html_area_t *root = atree->root;

	if (area->area_info.ypos > root->area_info.height * 3 / 5)
	{
		marktreeprintfs(MARK_ARTICLESIGN, "skip the area(id=%d) for its ypos is too low at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_SKIP;
	}

	if(area->area_info.ypos < 15){
		marktreeprintfs(MARK_ARTICLESIGN,"its ypos < 15 ,area(id=%d) is not article sign at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_NORMAL;}

	if (area->area_info.height > 100)
	{
		marktreeprintfs(MARK_ARTICLESIGN, "area(id=%d) is not article sign area for it is too high at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
		return AREA_VISIT_NORMAL;
	}

	const html_area_t *realtitle = sc_pack->realtit;
	if (!sc_pack->realtit || sc_pack->realtit->no + 1 != area->no)
	{ //对刚好在realtitle下面的块进行豁免
		int limit = 70;
		if(realtitle)
		{
			int lowpos_rt = realtitle->area_info.ypos + realtitle->area_info.height;
			int ydiff = area->area_info.ypos - lowpos_rt;
			if (area->baseinfo->text_info.time_num == 1 && ydiff >= 0 && ydiff < 60)
			{
				limit = 100;
			}
		}
		if (area->baseinfo->text_info.con_size >= limit)
		{
			marktreeprintfs(MARK_ARTICLESIGN, "area(id=%d) is not for its text size beyond limit at %s(%d)-%s\r\n", area->no, __FILE__, __LINE__, __FUNCTION__);
			return AREA_VISIT_NORMAL;
		}
	}
	//根据文字长度进行过滤，文字特别多的块不会是文章签名块
	int word_limit = 100;
	if(area->baseinfo->text_info.time_num == 1)
	{
		word_limit = 150;
	}
	if(area->baseinfo->text_info.con_size > word_limit)
	{
		marktreeprintfs(MARK_ARTICLESIGN,"area(id=%d) is not for it has too many words at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_NORMAL;
	}

	marking_info_t minfo;
	minfo.plus = 0;
	minfo.is_center = true;

	for (html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode)
	{
		html_vnode_visit(vnode, visit_for_arti_sign, NULL, &minfo);
		if (vnode == area->end)
			break;
	}
	if (minfo.is_center)
		minfo.plus += ARTI_SIGN_ALIGN_CENTER_VAL;
	if (area->baseinfo->text_info.time_num == 1)
	{
		minfo.plus++;
	}
	else if(area->baseinfo->text_info.time_num == 0)
	{
		minfo.plus -= 2;
	}
	if (sc_pack->realtit)
	{
		int distance = area->no - sc_pack->realtit->no;
		if (distance > 0 && distance <= 2)
		{
			minfo.plus += (2 - distance);
		}
		int realtitle_bottom = sc_pack->realtit->area_info.ypos + sc_pack->realtit->area_info.height;
		if(realtitle_bottom == area->area_info.ypos)
		{
			minfo.plus += 2;
		}
	}
	marktreeprintfs(MARK_ARTICLESIGN, "the score of the area(id=%d) is %d at %s(%d)-%s\r\n", area->no, minfo.plus, __FILE__, __LINE__, __FUNCTION__);
	if (minfo.plus >= ARTI_SIGN_THRESHOLD)
	{
		tag_area_func(area, AREA_FUNC_ARTICLE_SIGN);
		return AREA_VISIT_SKIP;
	}
	if (sc_pack->realtit && sc_pack->realtit->depth == area->depth && area->no == sc_pack->realtit->no + 1)
	{ //紧挨着realtitle分块的后面
		if (area->baseinfo->text_info.time_num == 1)
		{
			if (area->baseinfo->text_info.con_num - area->baseinfo->text_info.no_use_con_num - 1 == area->baseinfo->link_info.num)
			{
				tag_area_func(area, AREA_FUNC_ARTICLE_SIGN);
				return AREA_VISIT_SKIP;
			}
		}
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
 * 遍历area树，找到realtitle块返回
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

bool mark_func_article_sign(area_tree_t *atree)
{
	articlesign_pack_t sc_pack;
	sc_pack.mark_info = atree->mark_info;
	sc_pack.realtit = get_realtit_area(atree);
	areatree_visit(atree, (FUNC_START_T) visit_for_mark_article_sign_area, NULL, &sc_pack);
	return true;
}


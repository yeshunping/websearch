/*
 * mark_sem_hub_content.cpp
 *
 *  Created on: 2012-11-8
 *      Author: shuangwei
 */

#include "easou_mark_markinfo.h"
#include "easou_mark_baseinfo.h"
#include "easou_mark_com.h"
#include "easou_mark_srctype.h"
#include "easou_mark_func.h"
#include "easou_mark_sem.h"
#include "debuginfo.h"
#include"easou_html_attr.h"
typedef struct _sc_pack_t{
	mark_area_info_t *mark_info;
	const html_area_t *realtit;
}sc_pack_t;


typedef struct _text_tag_info_t{
	int fontTextLength[40];
	int maxlen;
	int minlen;
	int lines;
	int lastxpos;
}text_tag_info_t;

static void text_tag_info_clean_hub(text_tag_info_t *tt_info)
{
	memset(tt_info,0,sizeof(text_tag_info_t));
}

static int visit_for_text_font_info_hub(html_vnode_t *vnode, void *data)
{
	if(!vnode->isValid)
		return VISIT_SKIP_CHILD;

	text_tag_info_t *tt_info = (text_tag_info_t *)data;
	if(vnode->hpNode->html_tag.tag_type == TAG_PURETEXT&&vnode->font.size>=0&&vnode->font.size<40&&!vnode->font.is_bold){
		tt_info->fontTextLength[vnode->font.size] += vnode->textSize;
		if(vnode->textSize>tt_info->maxlen){
			tt_info->maxlen=vnode->textSize;
		}
		if(vnode->textSize<tt_info->minlen){
					tt_info->minlen=vnode->textSize;
		}
		if(vnode->ypos>tt_info->lastxpos+5){
			tt_info->lastxpos=vnode->ypos;
			tt_info->lines++;
		}

	}
	return VISIT_NORMAL;
}

static int has_proper_font_len_hub(html_area_t *area)
{
	text_tag_info_t tt_info;
	text_tag_info_clean_hub(&tt_info);

	/**
	 * 获取文本标签的信恄1�7
	 */
	for(html_vnode_t *vnode = area->begin; vnode; vnode=vnode->nextNode){
		if(vnode->isValid){
			html_vnode_visit(vnode,visit_for_text_font_info_hub,NULL,&tt_info);
		}
		if(vnode == area->end)
			break;
	}

	int minfontsize=-1;
	for(int j=0;j<40;j++){
		if(area->area_tree->root->begin->fontSizes[j]>0&&j>10){
			minfontsize=j;
			break;
		}
	}
    int areamaxfont=0;
    int allsize=1;
    for(int i=0;i<40;i++){
    	if(tt_info.fontTextLength[areamaxfont]<tt_info.fontTextLength[i]){
    		areamaxfont=i;
    	}
    	allsize+=tt_info.fontTextLength[i];
    }

    marktreeprintfs(MARK_HUB_CENTRAL,"the area(%d): areamaxfont=%d, minfontsize=%d,lines=%d  at %s(%d)-%s\r\n",area->no,areamaxfont,minfontsize,tt_info.lines,__FILE__,__LINE__,__FUNCTION__);
    if(tt_info.maxlen<10&&area->abspos_mark==PAGE_RIGHT){
    	return 0;
    }
    if(areamaxfont==minfontsize&&((tt_info.lines>1&&tt_info.fontTextLength[areamaxfont]*3>allsize&&(area->abspos_mark==PAGE_LEFT||area->abspos_mark==PAGE_RIGHT))||area->abspos_mark==PAGE_FOOTER||area->abspos_mark==PAGE_HEADER)){
    	return 0;
    }
    else{
    	return 1;
    }

}

/**
 * 判断块是否在页面两边
 */
static bool is_page_sider_hub(html_area_t *area)
{
    int page_width=area->area_tree->root->area_info.width;
	if(area->abspos_mark == PAGE_LEFT&&((area->area_info.xpos+area->area_info.width)*4<page_width)){
		return true;
	}

	if(area->abspos_mark == PAGE_RIGHT&&(area->area_info.xpos)*4>page_width*3){

			return true;
	}
	marktreeprintfs(MARK_HUB_CENTRAL,"the area is not the side ,height=%d ,width=%d  at %s(%d)-%s\r\n",area->area_info.height,area->area_info.width,__FILE__,__LINE__,__FUNCTION__);
	return false;
}

/**
 * 判断该节点是否为会移动的文字MARQUEE
 */
static int check_unproper_tag_hub(html_vnode_t *vnode, void *data)
{
	if(!vnode->isValid
			|| vnode->subtree_textSize <= 0)
		return VISIT_SKIP_CHILD;

	bool *has_unproper_tag = (bool *)data;

	if(vnode->hpNode->html_tag.tag_type == TAG_MARQUEE){
		*has_unproper_tag = true;
		return VISIT_FINISH;
	}

	return VISIT_NORMAL;
}

/**
 * 判断该块中是否含有会移动的文字；true：含有；不含有：false
 */
static bool has_unproper_tag_hub(html_area_t *area)
{
	if(area->area_info.height >= 50){
		return false;
	}

	bool has_unproper_tag = false;

	for(html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode){
		if(vnode->isValid){
			html_vnode_visit(vnode, check_unproper_tag_hub, NULL, &has_unproper_tag);
			if(has_unproper_tag)
				break;
		}
		if(vnode == area->end)
			break;
	}

	return has_unproper_tag;
}

/**
 * @brief 判断是否核心正文块.
**/
static int is_sem_central_area_hub(html_area_t * area ,mark_area_info_t * mark_info )
{
	if(area->depth == 0)
		return false;

	if(area->depth == 1){
		/**
		 * 子分块不含边框.
		 */
		for(html_area_t *subarea = area->subArea;subarea;subarea=subarea->nextArea){
			if(subarea->isValid
					&& is_page_sider_hub(subarea)){
				marktreeprintfs(MARK_HUB_CENTRAL,"the area is not central area for it is at side at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
				return false;
			}
		}
	}

	if((is_contain_func_area(area,AREA_FUNC_MYPOS) && !is_func_area(area, AREA_FUNC_MYPOS))
			|| is_contain_func_area(area,AREA_FUNC_NAV)
			|| is_contain_func_area(area,AREA_FUNC_FRIEND)
			|| is_contain_func_area(area,AREA_FUNC_RELATE_LINK)
			|| is_contain_func_area(area,AREA_FUNC_COPYRIGHT)
			|| is_contain_srctype_area(area,AREA_SRCTYPE_INTERACTION)
			|| is_contain_func_area(area,AREA_FUNC_ARTICLE_SIGN)
			|| is_contain_srctype_area(area,AREA_SRCTYPE_OUT)
			|| (is_contain_sem_area(area, AREA_SEM_REALTITLE) && !is_sem_area(area, AREA_SEM_REALTITLE))
			){

		/**
		 * 控制粒度
		 */
		marktreeprintfs(MARK_HUB_CENTRAL,"the area is not central area for it is func or src_out or realtitle at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	if(has_unproper_tag_hub(area)){
		marktreeprintfs(MARK_HUB_CENTRAL,"the area is not central for the marquee tag is in the area at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false;
	}
	if(has_proper_font_len_hub(area)==0){
		marktreeprintfs(MARK_HUB_CENTRAL,"the area is not central for font len at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false;
		}

	return true;
}

/**
 * @brief 根据有意义的块面积.
**/
static int get_tot_use_cont_area_hub(html_area_t *area)
{
	area_baseinfo_t *baseinfo = area->baseinfo;

	int tot_area = baseinfo->inter_info.in_area
			+ baseinfo->pic_info.pic_area
			+ baseinfo->text_info.text_area - baseinfo->text_info.no_use_text_area;

	return tot_area;
}

/**
 * @brief 跳过如下区域.
**/
static bool is_to_skip_mark_central_hub(html_area_t *area, mark_area_info_t *mark_info,
		const html_area_t *realtit)
{
	area_baseinfo_t *baseinfo = area->baseinfo;

	if(area->depth == 1){
		/**
		 * 顶部或顶部，若有较多链接或交互标签，认为是边框，过掉
		 */
		if(area->abspos_mark == PAGE_HEADER && !is_contain_sem_area(area, AREA_SEM_REALTITLE)){
			return true;
		}

		if(area->abspos_mark == PAGE_FOOTER){
			marktreeprintfs(MARK_HUB_CENTRAL,"skip the area for it is at  RELA_FOOTER at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
			return true;
		}
	}

	/**
	 * 根据realtitle定位
	 */
	if(realtit != NULL
			&& area->area_info.ypos + area->area_info.height < realtit->area_info.ypos
			&& area->area_info.ypos < realtit->area_info.ypos - 100){
		marktreeprintfs(MARK_HUB_CENTRAL,"skip the area for it is above the real title area at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return true;
	}

	if(is_page_sider_hub(area)){
		/**
		 * 过滤左右边框
		 */
		marktreeprintfs(MARK_HUB_CENTRAL,"skip the area for it is the side area at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return true;
	}

	if(is_in_func_area(area,AREA_FUNC_NAV)
			|| is_in_func_area(area,AREA_FUNC_FRIEND)
			|| is_in_func_area(area,AREA_FUNC_RELATE_LINK)
			|| is_in_func_area(area,AREA_FUNC_ARTICLE_SIGN)
			|| is_in_func_area(area,AREA_FUNC_COPYRIGHT)
			|| is_in_srctype_area(area,AREA_SRCTYPE_OUT)
			|| is_in_srctype_area(area,AREA_SRCTYPE_INTERACTION)
		){
		marktreeprintfs(MARK_HUB_CENTRAL,"skip the area for it is func or src area at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return true;
	}

	/**
	 * 无意义的分块
	 */
	if(get_tot_use_cont_area_hub(area) <= 0){
		marktreeprintfs(MARK_HUB_CENTRAL,"skip the area for the useful total area size of it <=0 at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return true;
	}

	/**
	 * 无意义的分块
	 */
	if(area->area_info.width <= 5 || area->area_info.height <= 5){
		if(area->baseinfo->text_info.con_size - area->baseinfo->text_info.no_use_con_size <= 0){
			marktreeprintfs(MARK_HUB_CENTRAL,"skip the area for its useful text area size <=0 at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
			return true;
		}
	}

	return false;
}

static int start_visit_for_mark_sem_hub(html_area_t *area, void *data)
{
	sc_pack_t *sc_pack = (sc_pack_t *)data;

	mark_area_info_t *mark_info = sc_pack->mark_info;
	if(MARK_HUB_CENTRAL==g_EASOU_DEBUG){
			printline();
			printSingleArea(area);
	}
	if(area->isValid == false){
		marktreeprintfs(MARK_HUB_CENTRAL,"skip the area for it is not valid at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP;
	}


	if(area->subArea!=NULL){
		html_area_t * childarea=NULL;
		for( childarea=area->subArea;childarea;childarea=childarea->nextArea){
			if(childarea&&childarea->isValid&&area->valid_subArea_num>1&&((childarea->baseinfo->text_info.text_area*10) >=(area->baseinfo->text_info.text_area*9))||childarea->nextArea&&childarea->abspos_mark!=childarea->nextArea->abspos_mark){
				return AREA_VISIT_NORMAL;
			}
		}
	}
//	if(area->subArea!=NULL){
//		return AREA_VISIT_NORMAL;
//	}
	/*
	 * 过滤不是核心正文块的区域
	 */
	if(is_to_skip_mark_central_hub(area, mark_info, sc_pack->realtit)){
		marktreeprintfs(MARK_HUB_CENTRAL,"skip the area for is_to_skip_mark_central() at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP;
	}
    /**
     * 对块的文本长度进行判断，平均长度太短，跳过
     */
	html_area_t *rootarea=area->area_tree->root;
	int pg_width = rootarea->area_info.width;
	int pg_height = rootarea->area_info.height;
	int page_text_len = rootarea->baseinfo->text_info.con_size;
	int doctype=area->area_tree->hp_vtree->hpTree->doctype;


	if(is_sem_central_area_hub(area,mark_info)){
		//todo
//		char buf[1024];
//		memset(&buf, 0, 1024);
//		extract_area_content(buf, sizeof(buf), area);
//		printf("central:	%s\n", buf);
		marktreeprintfs(MARK_HUB_CENTRAL,"the area is central at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		tag_area_sem(area, AREA_SEM_HUB_CENTRAL);
		return AREA_VISIT_SKIP;
	}

	return AREA_VISIT_NORMAL;
}


static int visit_for_realtit_hub(html_area_t *area, html_area_t **p_realtit)
{
	if(!area->isValid
			|| !is_contain_sem_area(area, AREA_SEM_REALTITLE)){
		return AREA_VISIT_SKIP;
	}

	if(is_sem_area(area,AREA_SEM_REALTITLE)){
		*p_realtit = area;
		return AREA_VISIT_FINISH;
	}

	return AREA_VISIT_NORMAL;
}
/**
 * 遍历area树，找到realtitle块返回
 */
static const html_area_t * get_realtit_area_hub(area_tree_t *atree)
{
	html_area_t *realtit = NULL;

	/**
	 * 获取第一个realtitle块
	 */
	areatree_visit(atree, (FUNC_START_T)visit_for_realtit_hub, NULL, &realtit);

	return realtit;
}

/**
 * @brief 标记核心正文块.
**/
bool mark_sem_hub_central(area_tree_t *atree)
{
	if(atree->root->valid_subArea_num == 0){
		tag_area_sem(atree->root,AREA_SEM_HUB_CENTRAL);
		return true ;
	}

	sc_pack_t sc_pack;
	sc_pack.mark_info = atree->mark_info ;
	sc_pack.realtit = get_realtit_area_hub(atree);

	areatree_visit(atree,start_visit_for_mark_sem_hub,
			NULL,
			&sc_pack);

	return true;
}

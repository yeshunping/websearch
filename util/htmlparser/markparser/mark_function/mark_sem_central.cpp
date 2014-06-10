/*
 * mark_sem_central.cpp
 *
 *  Created on: 2011-11-24
 *      Author: ddt
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
	int tot_text_len;
	int max_text_len;
	int text_tag_num;
	int is_has_container_tag;
}text_tag_info_t;

static void text_tag_info_clean(text_tag_info_t *tt_info)
{
	memset(tt_info,0,sizeof(text_tag_info_t));
}

static int visit_for_text_tag_info(html_vnode_t *vnode, void *data)
{
	if(!vnode->isValid)
		return VISIT_SKIP_CHILD;

	text_tag_info_t *tt_info = (text_tag_info_t *)data;
	if(vnode->hpNode->html_tag.tag_type == TAG_PURETEXT){
		tt_info->tot_text_len += vnode->textSize;
		tt_info->text_tag_num ++;
		if(vnode->textSize > tt_info->max_text_len)
			tt_info->max_text_len = vnode->textSize;
	}
	else if(vnode->hpNode->html_tag.tag_type == TAG_A
			&& get_attribute_value(&vnode->hpNode->html_tag,ATTR_HREF) != NULL){
		return VISIT_SKIP_CHILD;
	}

	return VISIT_NORMAL;
}

static int visit_for_container_tag(html_vnode_t *vnode, void *data)
{
	if(vnode->isValid){

		text_tag_info_t *tt_info = (text_tag_info_t *)data;
		html_tag_type_t tag_type=vnode->hpNode->html_tag.tag_type;
		if(tag_type == TAG_ROOT
			|| tag_type == TAG_HTML
			|| tag_type == TAG_BODY
			|| tag_type == TAG_TABLE
			|| tag_type == TAG_TBODY
			|| tag_type == TAG_DIV
			|| tag_type == TAG_COLGROUP
			|| tag_type == TAG_CENTER
			|| tag_type == TAG_FORM){
		tt_info->is_has_container_tag=1;
		}
	}
	return VISIT_NORMAL;
}

static int has_proper_tag_len(html_area_t *area, int page_width, int page_height, int page_text_len)
{
	text_tag_info_t tt_info;
	text_tag_info_clean(&tt_info);

	/**
	 * 获取文本标签的信恄1�7
	 */
	for(html_vnode_t *vnode = area->begin; vnode; vnode=vnode->nextNode){
		if(vnode->isValid){
			html_vnode_visit(vnode,visit_for_text_tag_info,visit_for_container_tag,&tt_info);
		}
		if(vnode == area->end)
			break;
	}

	if(tt_info.is_has_container_tag==0&&(area->begin->hpNode->html_tag.tag_type==TAG_TABLE||area->begin->hpNode->html_tag.tag_type==TAG_P&&area->valid_subArea_num<1)&&area->baseinfo->text_info.con_size>area->baseinfo->link_info.anchor_size*2){
		return 1;
	}

	/* 根据文本标签的长度�1�7�数量等信息判断是否主要内容〄1�7
	 * 丄1�7般说来，正文的文本标签长度较大，数量相对较少〄1�7
	 * 而如果文本比较支零破碎，可能是版权信息，或边框上的文字�1�7�1�7
	 * */
	if(tt_info.max_text_len >= 100 || (tt_info.text_tag_num > 0 && tt_info.tot_text_len/tt_info.text_tag_num >= 70)){
		return 1;
	}
	int doctype=area->area_tree->hp_vtree->hpTree->doctype;
	if(area->area_info.ypos < page_height/3){
		if(tt_info.max_text_len >= 22 || (tt_info.text_tag_num <= 3 && tt_info.max_text_len >= 4)&&!(doctype>=doctype_xhtml_BP&&doctype<=doctype_html5))
			return 1;
	}

	if(tt_info.max_text_len >= page_text_len/3){
		//这个策略注重准确
		return 1;
	}

	return 0;
}

/**
 * 判断块是否在页面两边
 */
static bool is_page_sider(html_area_t *area)
{

	if(area->pos_plus == IN_PAGE_LEFT){
		/**
		 * 在页面左边，根据链接和交互标签特征判断是否边框.
		 */
		if(area->baseinfo->link_info.num + area->baseinfo->link_info.other_num >= 6){
			marktreeprintfs(MARK_CENTRAL,"the area is at the left side of page ,and link >=6 at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
			return true;}
		if(area->baseinfo->inter_info.input_num + area->baseinfo->inter_info.select_num >= 2){
			marktreeprintfs(MARK_CENTRAL,"the area is at the left side of page ,and input +  select >2= at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
			return true;}
		if(area->area_info.height >= area->area_info.width * 2
				|| area->area_info.height >= 250){
			marktreeprintfs(MARK_CENTRAL,"the area is at the left side of page ,and height > width*2 or height>250  at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
			return true;}
	}

	if(area->pos_plus == IN_PAGE_RIGHT
			//|| (area->depth <= 2 && area->pos_mark == RELA_RIGHT)
			){
		if(area->area_info.height > area->area_info.width){
			marktreeprintfs(MARK_CENTRAL,"the area is at the right side of page ,and height > width at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
			return true;}
	}
	marktreeprintfs(MARK_CENTRAL,"the area is not the side ,height=%d ,width=%d  at %s(%d)-%s\r\n",area->area_info.height,area->area_info.width,__FILE__,__LINE__,__FUNCTION__);
	return false;
}

/**
 * 判断该节点是否为会移动的文字MARQUEE
 */
static int check_unproper_tag(html_vnode_t *vnode, void *data)
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
static bool has_unproper_tag(html_area_t *area)
{
	if(area->area_info.height >= 50){
		return false;
	}

	bool has_unproper_tag = false;

	for(html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode){
		if(vnode->isValid){
			html_vnode_visit(vnode, check_unproper_tag, NULL, &has_unproper_tag);
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
static int is_sem_central_area(html_area_t * area ,mark_area_info_t * mark_info )
{
	if(area->depth == 0)
		return false;

	if(area->depth == 1){
		/**
		 * 子分块不含边框.
		 */
		for(html_area_t *subarea = area->subArea;subarea;subarea=subarea->nextArea){
			if(subarea->isValid
					&& is_page_sider(subarea)){
				marktreeprintfs(MARK_CENTRAL,"the area is not central area for it is at side at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
				return false;
			}
		}
	}

	if((is_contain_func_area(area,AREA_FUNC_MYPOS) && !is_func_area(area, AREA_FUNC_MYPOS))
			|| is_contain_func_area(area,AREA_FUNC_NAV)
			|| is_contain_func_area(area,AREA_FUNC_FRIEND)
			|| is_contain_func_area(area,AREA_FUNC_RELATE_LINK)
			|| is_contain_func_area(area,AREA_FUNC_COPYRIGHT)
			|| is_contain_func_area(area,AREA_FUNC_ARTICLE_SIGN)
			|| is_contain_srctype_area(area,AREA_SRCTYPE_OUT)
			|| (is_contain_sem_area(area, AREA_SEM_REALTITLE) && !is_sem_area(area, AREA_SEM_REALTITLE))
			){

		/**
		 * 控制粒度
		 */
		marktreeprintfs(MARK_CENTRAL,"the area is not central area for it is func or src_out or realtitle at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	if(has_unproper_tag(area)){
		marktreeprintfs(MARK_CENTRAL,"the area is not central for the marquee tag is in the area at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false;
	}
    if(area->baseinfo->link_info.link_area>area->baseinfo->text_info.text_area/3&&area->valid_subArea_num>0){
    	marktreeprintfs(MARK_CENTRAL,"the area is not central for link_area> text_area /3 and it has sub area at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
    	return false;
    }

	return true;
}

/**
 * @brief 根据有意义的块面积.
**/
static int get_tot_use_cont_area(html_area_t *area)
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
static bool is_to_skip_mark_central(html_area_t *area, mark_area_info_t *mark_info,
		const html_area_t *realtit)
{
	area_baseinfo_t *baseinfo = area->baseinfo;

	if(area->depth == 1){
		/**
		 * 顶部或顶部，若有较多链接或交互标签，认为是边框，过掉
		 */
		if(area->pos_mark == RELA_HEADER && !is_contain_sem_area(area, AREA_SEM_REALTITLE)){
			if(baseinfo->link_info.num + baseinfo->link_info.other_num >= 4
					|| baseinfo->link_info.other_num >= 2){
				marktreeprintfs(MARK_CENTRAL,"skip the area for it is at RELA_HEADER and don't contain realtitle and link_num=%d ,link_other_num=%d  at %s(%d)-%s\r\n",baseinfo->link_info.num,baseinfo->link_info.other_num,__FILE__,__LINE__,__FUNCTION__);
				return true;}
			if(baseinfo->inter_info.input_num + baseinfo->inter_info.select_num >= 2){
				marktreeprintfs(MARK_CENTRAL,"skip the area for it is at RELA_HEADER and don't contain realtitle and input+select>2  at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
				return true;}
			if(baseinfo->text_info.con_size - baseinfo->text_info.no_use_con_size <= 0){
				marktreeprintfs(MARK_CENTRAL,"skip the area for it is at RELA_HEADER and don't contain realtitle and useful content size<0  at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
				return true;}
		}

		if(area->pos_mark == RELA_FOOTER){
			marktreeprintfs(MARK_CENTRAL,"skip the area for it is at  RELA_FOOTER at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
			return true;
		}
	}

	/**
	 * 根据realtitle定位
	 */
	if(realtit != NULL
			&& area->area_info.ypos + area->area_info.height < realtit->area_info.ypos
			&& area->area_info.ypos < realtit->area_info.ypos - 100){
		marktreeprintfs(MARK_CENTRAL,"skip the area for it is above the real title area at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return true;
	}

	if(is_page_sider(area)){
		/**
		 * 过滤左右边框
		 */
		marktreeprintfs(MARK_CENTRAL,"skip the area for it is the side area at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
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
		marktreeprintfs(MARK_CENTRAL,"skip the area for it is func or src area at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return true;
	}

	/**
	 * 无意义的分块
	 */
	if(get_tot_use_cont_area(area) <= 0){
		marktreeprintfs(MARK_CENTRAL,"skip the area for the useful total area size of it <=0 at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return true;
	}

	/**
	 * 无意义的分块
	 */
	if(area->area_info.width <= 5 || area->area_info.height <= 5){
		if(area->baseinfo->text_info.con_size - area->baseinfo->text_info.no_use_con_size <= 0){
			marktreeprintfs(MARK_CENTRAL,"skip the area for its useful text area size <=0 at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
			return true;
		}
	}

	return false;
}

static int start_visit_for_mark_sem(html_area_t *area, void *data)
{
	sc_pack_t *sc_pack = (sc_pack_t *)data;

	mark_area_info_t *mark_info = sc_pack->mark_info;
	if(MARK_CENTRAL==g_EASOU_DEBUG){
			printline();
			printSingleArea(area);
	}
	if(area->isValid == false){
		marktreeprintfs(MARK_CENTRAL,"skip the area for it is not valid at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP;
	}


	if(area->subArea!=NULL){
		html_area_t * childarea=NULL;
		for( childarea=area->subArea;childarea;childarea=childarea->nextArea){
			if(childarea&&childarea->isValid&&((childarea->baseinfo->text_info.text_area*10) >=(area->baseinfo->text_info.text_area*9))&&area->valid_subArea_num>1){
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
	if(is_to_skip_mark_central(area, mark_info, sc_pack->realtit)){
		marktreeprintfs(MARK_CENTRAL,"skip the area for is_to_skip_mark_central() at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
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
	if(area->valid_subArea_num==0&&(doctype>=doctype_xhtml_BP&&doctype<=doctype_html5)&&has_proper_tag_len(area,pg_width,pg_height,page_text_len)==0){
		return AREA_VISIT_SKIP;
	}
	if(is_sem_central_area(area,mark_info)){
		//todo
//		char buf[1024];
//		memset(&buf, 0, 1024);
//		extract_area_content(buf, sizeof(buf), area);
//		printf("central:	%s\n", buf);
		marktreeprintfs(MARK_CENTRAL,"the area is central at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		tag_area_sem(area, AREA_SEM_CENTRAL);
		return AREA_VISIT_SKIP;
	}

	return AREA_VISIT_NORMAL;
}

static void clear_subarea_central_tag(html_area_t *area)
{
	for(html_area_t *subarea = area->subArea; subarea; subarea = subarea->nextArea){
		if(!subarea->isValid)
			continue;
		clear_sem_area_tag(subarea, AREA_SEM_CENTRAL);
	}
}

/**
 * 对central子块进行合并，大部分儿子是则父亲是
 */
static int finish_visit_for_mark_central(html_area_t *area, void *data)
{
	if(!area->isValid || area->valid_subArea_num == 0){
		return AREA_VISIT_NORMAL;
	}

	int sub_tot_area = 0;
	int sub_sem_area = 0;
	unsigned int sub_sem_num = 0;
	unsigned int use_subarea_num = 0;
    bool issamefather=true;
    html_vnode_t * parent=NULL;
	for(html_area_t *subarea = area->subArea; subarea; subarea = subarea->nextArea){
		if(!subarea->isValid)
			continue;
		int this_area = subarea->area_info.width * subarea->area_info.height;
		if(NULL==parent){
			parent=subarea->begin->upperNode;
		}
		else{
			if(parent!=subarea->begin->upperNode&&subarea->begin->upperNode){
				issamefather=false;
			}
		}
		if(this_area > 0){
			use_subarea_num ++;
		}
		else{
			continue;
		}
		sub_tot_area += this_area;
		if(is_sem_area(subarea, AREA_SEM_CENTRAL)){
			sub_sem_area += this_area;
			sub_sem_num ++;
		}
		if(this_area > 0){
			use_subarea_num ++;
		}
	}

	/**
	 * 对其子分块绝大部分被标为核心正文的块，
	 * 该分块本身也标记为核心正文
	 */
	if(sub_sem_area > 0 && sub_sem_area * 100 >= sub_tot_area * 95 && (sub_tot_area - sub_sem_area <= 30000)&&(use_subarea_num*1<sub_sem_num*2)&&issamefather){
		clear_subarea_central_tag(area);

		//todo
//		char buf[32767];
//		memset(&buf, 0, 32767);
//		extract_area_content(buf, sizeof(buf), area);
//		printf("central:	%s\n", buf);
		marktreeprintfs(MARK_CENTRAL,"mark the area is central for  most of the subarea are central at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		tag_area_sem(area, AREA_SEM_CENTRAL);
	}

	return AREA_VISIT_NORMAL;
}

static int visit_for_realtit(html_area_t *area, html_area_t **p_realtit)
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
static const html_area_t * get_realtit_area(area_tree_t *atree)
{
	html_area_t *realtit = NULL;

	/**
	 * 获取第一个realtitle块
	 */
	areatree_visit(atree, (FUNC_START_T)visit_for_realtit, NULL, &realtit);

	return realtit;
}

/**
 * @brief 标记核心正文块.
**/
bool mark_sem_central(area_tree_t *atree)
{
	if(atree->root->valid_subArea_num == 0){
		tag_area_sem(atree->root,AREA_SEM_CENTRAL);
		return true ;
	}

	sc_pack_t sc_pack;
	sc_pack.mark_info = atree->mark_info ;
	sc_pack.realtit = get_realtit_area(atree);

	areatree_visit(atree,start_visit_for_mark_sem,
			finish_visit_for_mark_central,
			&sc_pack);

	return true;
}

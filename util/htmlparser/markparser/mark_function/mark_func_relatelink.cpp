/*
 * easou_func_relate.cpp
 *
 *  Created on: 2011-11-24
 *      Author: ddt
 */
#include "easou_url.h"
#include "easou_html_attr.h"
#include "easou_lang_relate.h"
#include "easou_mark_baseinfo.h"
#include "easou_mark_com.h"
#include "easou_mark_srctype.h"
#include "easou_mark_func.h"
#include "debuginfo.h"
#define FOR_REL_ANCHOR_HASH_NUM 1024 //计算相关链接用到的hash

//一些链接相关信息结构体
typedef struct _relate_link_info_t
{
	int inner_num ;//内链个数
	int out_num ;  //外链个数
	int ave_anchor_size ; //平均anchor长度
	int max_md_dup_num ;                      //maindomain最大重复次数,外链情形
	int anchor_digit_num ; 					  //anchor最后出现数字的anchor数量
}relate_link_info_t ;

//一些文本相关的信息结构体
typedef struct _relate_text_info_t
{
	bool is_has_rel ;        //是否出现相关等
	int  time_str_num ;      //出现时间的个数
}relate_text_info_t ;

/**
 * @brief 当前分块是否含有给定的词汇.
**/
bool is_area_has_given_words(const html_area_t *area, const char * given_words[])
{
	bool b_has_given_words = false;

    vnode_list_t * looper = area->baseinfo->text_info.cont_vnode_list_begin ;

	while( looper != NULL ){
		char * p_text = looper->vnode->hpNode->html_tag.text ;

		if(is_has_special_word(given_words, p_text)){
			b_has_given_words = true;
			break;
		}

		//循环退出条件
		if(looper == area->baseinfo->text_info.cont_vnode_list_end ){
			break ;
		}

		looper = looper->next ;
	}

	return b_has_given_words;
}

/**
 * @brief 获取与前一个链接重复urlpat的链接面积
**/
int get_repeat_url_pat_area(const html_area_t *area, const char *page_url)
{
	char last_urlpat[UL_MAX_URL_LEN];
	char last_url[UL_MAX_URL_LEN];
	last_urlpat[0] = '\0';
	last_url[0] = '\0';

	int sp_area = 0;

	AOI4ST_link_t *link_info = &area->baseinfo->link_info;

	vnode_list_t *vlist = link_info->url_vnode_list_begin;

	for(; vlist; vlist = vlist->next){
		html_vnode_t *vnode = vlist->vnode;

		const char *phref = get_attribute_value(&vnode->hpNode->html_tag, ATTR_HREF);

		if(phref != NULL && strchr(phref,'#') == NULL){
			char link[UL_MAX_URL_LEN];
			link[0] = '\0';
			easou_combine_url(link, page_url, phref);

			char pat[UL_MAX_URL_LEN];
			pat[0] = '\0';//todo
			ef_trans2pt(link, pat);

			if(strcmp(link, last_url) != 0
					&& strcmp(pat, last_urlpat) == 0){
				sp_area += vnode->wx * vnode->hx;
			}

			strcpy(last_urlpat, pat);
			strcpy(last_url,link);
		}
		if(vlist == link_info->url_vnode_list_end)
			break;
	}
	return sp_area;
}

static bool is_in_pagesider_rellink(const html_area_t *area)
{
	if(area->pos_plus == IN_PAGE_LEFT
			|| area->pos_plus == IN_PAGE_RIGHT){
		return true;
	}

	return false;
}

static bool is_to_skip_mark_rel(html_area_t *area, area_tree_t *atree)
{
	if(area->depth == 1
			&& area->pos_mark == RELA_HEADER){
		/**
		 * 页面头部
		 */
		marktreeprintfs(MARK_RELATELINK,"the area depth=1 and pos=rela_header ,it is not relate link at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return true;
	}
	/**
	 * 要么含有2条以上内链,要么含有3条以上同主域外链.
	 */
	if(area->baseinfo->link_info.inner_num < 2
			&& area->baseinfo->link_info.out_num < 3){
		marktreeprintfs(MARK_RELATELINK,"link_inner_num(%d)<2 and link_out_num(%d)<3 ,it is not relate link at %s(%d)-%s\r\n",area->baseinfo->link_info.inner_num,area->baseinfo->link_info.out_num,__FILE__,__LINE__,__FUNCTION__);
		return true;
	}

	if(is_in_func_area(area, AREA_FUNC_COPYRIGHT)
			|| is_in_func_area(area, AREA_FUNC_MYPOS)
			|| is_in_func_area(area, AREA_FUNC_NAV)){
		marktreeprintfs(MARK_RELATELINK,"the area is in copyright or mypos or NAV ,it is not relate link at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return true;
	}

	if(!is_in_pagesider_rellink(area)
			&& area->area_info.ypos + area->area_info.height < atree->root->area_info.height / 3
			&& area->area_info.ypos <= 360){
		marktreeprintfs(MARK_RELATELINK,"the ypos(%d) of the area<360,the height of area=%d,the height of page=%d   ,it is not relate link at %s(%d)-%s\r\n",area->area_info.ypos,area->area_info.height,atree->root->area_info.height,__FILE__,__LINE__,__FUNCTION__);
		return true;
	}
   if(area->baseinfo->text_info.text_area>area->baseinfo->link_info.link_area*2){
	   marktreeprintfs(MARK_RELATELINK,"the text_area>link_area,it is not relate link at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
	   return true;
   }
	return false;
}

/**
 * @brief 根据子分块判断当前备选块特征密度是否足够高.
 * 比如一个包含相关链接的网页，其根分块也有相关链接的特征，但密度不高，
 * 必存在密度足够高的子分块.
**/
static bool is_dense_rel_link(html_area_t *area, int tot_area)
{
	unsigned int link_area = area->baseinfo->link_info.link_area;

	bool is_dense = true;

	int prev_inner_num = 0;
	int prev_out_num = 0;

	for(html_area_t *subarea = area->subArea; subarea; subarea = subarea->nextArea){
		if(!subarea->isValid)
			continue;
		area_baseinfo_t *baseinfo = subarea->baseinfo;

		unsigned int s_link_area = baseinfo->link_info.link_area;

		/**
		 * 当前分块跟其前的分块链接的类型不一致。
		 */
		if(prev_inner_num == 0 && prev_out_num >= 2){
			if(baseinfo->link_info.out_num == 0
					&& baseinfo->link_info.inner_num >= 2){
				marktreeprintfs(MARK_RELATELINK," the link inner num (%d)>=2,it is not relate link at %s(%d)-%s\r\n",baseinfo->link_info.inner_num,__FILE__,__LINE__,__FUNCTION__);
				is_dense = false;
				break;
			}
		}

		if(prev_inner_num >= 2 && prev_out_num == 0){
			if(baseinfo->link_info.out_num >= 2
					&& baseinfo->link_info.inner_num == 0){
				marktreeprintfs(MARK_RELATELINK," the link out num (%d)>=2,it is not relate link at %s(%d)-%s\r\n",baseinfo->link_info.out_num,__FILE__,__LINE__,__FUNCTION__);
				is_dense = false;
				break;
			}
		}

		/**
		 * 若发现某分块链接面积远大于每个分块平均链接面积，则
		 * 可以再分
		 */
		if(s_link_area * area->valid_subArea_num > link_area * 2
				&& baseinfo->link_info.num >= 2){
			marktreeprintfs(MARK_RELATELINK," the link  num(%d) >=2 and its area size is big,it is not relate link at %s(%d)-%s\r\n",baseinfo->link_info.num,__FILE__,__LINE__,__FUNCTION__);
			is_dense = false;
			break;
		}

		if(baseinfo->link_info.num >= 8){
			marktreeprintfs(MARK_RELATELINK," the link  num(%d) >=8 ,it is not relate link at %s(%d)-%s\r\n",baseinfo->link_info.num,__FILE__,__LINE__,__FUNCTION__);
			is_dense = false;
			break;
		}

		prev_inner_num = baseinfo->link_info.inner_num;
		prev_out_num = baseinfo->link_info.out_num;
	}

	return is_dense;
}

static int get_tot_area(html_area_t *area)
{
	area_baseinfo_t *baseinfo = area->baseinfo;

	int tot_area = baseinfo->extern_info.extern_area + baseinfo->inter_info.in_area
			+ baseinfo->pic_info.pic_area
			+ baseinfo->text_info.text_area - baseinfo->text_info.no_use_text_area;

	return tot_area;
}

/**
 * @brief 是否相关链接粒度的分块.
**/
static bool is_rel_link_gran(html_area_t *area, area_tree_t *atree)
{
	area_baseinfo_t *baseinfo = area->baseinfo;

	if(area->depth == 0){
		marktreeprintfs(MARK_RELATELINK,"the area  depth=0 ,it is not relate link at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	if(is_contain_func_area(area, AREA_FUNC_MYPOS)
			|| is_contain_func_area(area, AREA_FUNC_COPYRIGHT)
			|| is_contain_func_area(area, AREA_FUNC_NAV)
			|| is_contain_srctype_area(area, AREA_SRCTYPE_OUT)){
		marktreeprintfs(MARK_RELATELINK,"the area  contain mypos or copyright or src_out ,it is not relate link at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	if(baseinfo->link_info.other_num > baseinfo->link_info.num){
		/**
		 * javascrpt:, mailto:等链接数量比正常链接多.
		 */
		marktreeprintfs(MARK_RELATELINK,"link_other_num(%d)>link_num(%d) ,it is not relate link at %s(%d)-%s\r\n",baseinfo->link_info.other_num,baseinfo->link_info.num,__FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	int tot_area = get_tot_area(area);

	if(baseinfo->link_info.link_area * 2 < tot_area){
		marktreeprintfs(MARK_RELATELINK,"the rate of link area <0.2 ,it is not relate link at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	if(!is_dense_rel_link(area, tot_area)){
		/**
		 * 是否密度足够高.
		 */
		marktreeprintfs(MARK_RELATELINK,"is_dense_rel_link()=false ,it is not relate link at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false;
	}
	return true;
}

/**
 * @brief	链接相互之间是否同一个pat.
**/
static bool is_same_urlpat_each_other_area(html_area_t *area, mark_area_info_t *mark_info, int tot_link_area)
{
	if(area->pos_plus != IN_PAGE_LEFT
			&& area->pos_plus != IN_PAGE_RIGHT){
		return false;
	}

	if(area->baseinfo->link_info.num > 0){
		int ave_anchor_size = area->baseinfo->link_info.anchor_size / area->baseinfo->link_info.num;

		if(ave_anchor_size <= 10){
			return false;
		}
	}

	int repeat_pat_area = get_repeat_url_pat_area(area, mark_info->base_href);

	if(repeat_pat_area * 10 >= tot_link_area * 6){
		return true;
	}

	return false;
}


/**
 * @brief 根据urlpat判断是否相关链接区域.
**/
static bool is_same_urlpat_rel_link_area(html_area_t *area, mark_area_info_t *mark_info)
{
	int link_num = area->baseinfo->link_info.num;

	int anchor_size = area->baseinfo->link_info.anchor_size;

	if(area->baseinfo->pic_info.link_pic_num <= link_num - 2
			&& anchor_size <= link_num * 2){
		/**
		 * 平均anchor长度太小，过滤翻页、日历等。
		 */
		marktreeprintfs(MARK_RELATELINK,"the ave length of anchor <2 ,it is not relate link at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	same_urlpat_t spat;
	get_same_pat_url_info(&spat, area, mark_info->page_url, mark_info->page_url_pat);
	int tot_link_area = area->baseinfo->link_info.link_area;

	if(spat.area * 10 >= tot_link_area * 6
			&& spat.num >= 3){
		return true;
	}

	if(spat.num == link_num){
		return true;
	}

	if(is_same_urlpat_each_other_area(area, mark_info, tot_link_area)){
		return true;
	}
	marktreeprintfs(MARK_RELATELINK,"url pattern match fail ,it is not relate link at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
	return false;
}

static const html_area_t *get_higher_useful_area(const html_area_t *area, int hi_limit)
{
	for(const html_area_t *prev = area->prevArea; prev; ){
		if(prev->isValid
				&& area->area_info.ypos - prev->area_info.ypos > hi_limit){
			break;
		}

		if(prev->isValid
				&& prev->area_info.width >= 10
				&& prev->area_info.height >= 10
				&& prev->area_info.ypos < area->area_info.ypos){
			return prev;
		}

		if(prev->prevArea != NULL){
			prev = prev->prevArea;
		}
		else{
			prev = prev->parentArea->prevArea;
		}
	}

	return NULL;
}

/**
 * @brief 是否含有“相关”，“推荐”等词汇.
**/
static bool has_rel_link_hint_word(const html_area_t *area)
{
	static const int HI_LIMIT = 90;

	if(area->baseinfo->link_info.num <= 3){
		marktreeprintfs(MARK_RELATELINK,"the link_num(%d)<=3 ,it is not relate link at %s(%d)-%s\r\n",area->baseinfo->link_info.num,__FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	const html_area_t *higher_area = get_higher_useful_area(area, HI_LIMIT);

	if(higher_area
			&& higher_area->area_info.height <= 50
			&& higher_area->baseinfo->text_info.con_size >= 4
			&& higher_area->baseinfo->text_info.con_size <= 32){
		if(is_area_has_given_words(higher_area, for_rel_link)){
			return true;
		}
	}

	for(html_area_t *subarea = area->subArea; subarea; subarea = subarea->nextArea){
		if(!subarea->isValid)
			continue;
		if(subarea->baseinfo->text_info.con_size >= 4
				&& subarea->baseinfo->link_info.num <= 2
				&& subarea->baseinfo->link_info.anchor_size <= 16){
			if(is_area_has_given_words(subarea, for_rel_link)){
				return true;
			}

			break;
		}
	}
	marktreeprintfs(MARK_RELATELINK,"the area  is not relate link for it don't contain the rel_link_word at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
	return false;
}

static int start_mark_relate_link(html_area_t *area, mark_area_info_t *mark_info)
{
	if(MARK_RELATELINK==g_EASOU_DEBUG){
		printlines("mark relate link");
		printNode(area->begin->hpNode);
	}
	if(!area->isValid){
		marktreeprintfs(MARK_RELATELINK,"the area  is not valid ,it is not relate link at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP;
	}
	// shuangwei add 20120511
//		if(!IS_LEAF_AREA(area->areaattr)){
//				return AREA_VISIT_NORMAL;
//		}
	if(!is_rel_link_gran(area, mark_info->area_tree)){
		return AREA_VISIT_NORMAL;
	}

	if(is_to_skip_mark_rel(area, mark_info->area_tree)){
		return AREA_VISIT_SKIP;
	}


	if(has_rel_link_hint_word(area) || is_same_urlpat_rel_link_area(area, mark_info)){
		//todo
//		char buf[1024];
//		memset(&buf, 0, 1024);
//		extract_area_content(buf, sizeof(buf), area);
//		printf("relalink:	%s\n", buf);

		tag_area_func(area, AREA_FUNC_RELATE_LINK) ;
		return AREA_VISIT_FINISH;//认为相关链接只有一块
	}

	return AREA_VISIT_SKIP ;
}

bool mark_func_relate_link(area_tree_t *atree)
{
	bool ret = areatree_visit(atree,
			(FUNC_START_T)start_mark_relate_link,
			NULL,atree->mark_info);

	return ret;
}


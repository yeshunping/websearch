/*
 * easou_mark_friend.cpp
 *
 *  Created on: 2011-11-24
 *      Author: ddt
 */

#include "easou_url.h"
#include "simplehashmap.h"
#include "easou_lang_relate.h"
#include "easou_html_attr.h"
#include "easou_vhtml_basic.h"
#include "easou_mark_switch.h"
#include "easou_mark_com.h"
#include "easou_mark_baseinfo.h"
#include "easou_mark_srctype.h"
#include "easou_mark_func.h"
#include "debuginfo.h"
//一些链接相关信息结构体
typedef struct _friend_link_info_t
{
	int inner_num ;//内链个数
	int out_num ;  //外链个数
	int top_num ;                             //首页url个数
	int spec_anchor_num ;                     //出现网站等友情链接容易出现的anchor的个数
	int max_md_dup_num ;                      //maindomain最大重复次数,外链情形
	bool is_icp_anchor ;                      //属于ICP num的数量
}friend_link_info_t ;

/**
 * true:含有icp的A节点
 */

static bool is_icp_link_node(html_vnode_t * vnode )
{
	if(vnode->hpNode->html_tag.tag_type != TAG_A )
		return false ;
	char anchor_buff[MAX_ANCHOR_LEN];
	anchor_buff[0] = '\0';
	int anchor_len = get_anchor_str(vnode , anchor_buff , MAX_ANCHOR_LEN) ;
	if(anchor_len <=0 )
		return false ;
	anchor_buff[anchor_len] = '\0';
	if(strstr(anchor_buff , "ICP备")!=NULL )
	{
		return true ;
	}
	return false ;
}

/**
 * 是否在末尾包含指定值；true：是
 */
static bool is_last_special_word(const char * src , const char * spec_list[])
{
	int src_len = strlen(src ) ;
	int i = 0 ;
	for(i =0 ; spec_list[i] ; i++ )
	{
		int search_len = strlen(spec_list[i]) ;
		if(src_len > search_len)
		{
			if(strcmp(src+(src_len - search_len) , spec_list[i]) ==0 )
			{
				return true ;
			}
		}
	}
	return false ;
}
/**
 * 获取该块下外联接、内链接数及包含关键词链接、同一网站数
 */
void get_friend_link_info4area(const html_area_t * area , const char * base_url , friend_link_info_t * d_link )
{
	assert(area!=NULL && area->baseinfo !=NULL ) ;
	area_baseinfo_t * paoi = (area_baseinfo_t *) area->baseinfo ;
	memset(d_link , 0 , sizeof(friend_link_info_t )) ;
    vnode_list_t * looper = paoi->link_info.url_vnode_list_begin ;
	char hash_4_dup_mp[256] = {0} ; //粗略的计算重复的最大maindomain数量。

	while( looper != NULL )
	{
		html_vnode_t * vnode = looper->vnode ;
		int ret = get_link_type(vnode , base_url ) ;
		if ( ret == IS_LINK_INNER )
		{
			d_link->inner_num++ ;
		}
		else if( ret == IS_LINK_OUT )
		{
			d_link->out_num++ ;
			char * phref = get_attribute_value(&vnode->hpNode->html_tag, ATTR_HREF ) ;
			char tmp[UL_MAX_URL_LEN];
			tmp[0] = '\0';
			snprintf(tmp,sizeof(tmp),"%s",phref) ;
			if(is_like_top_url(tmp))
			{
				d_link->top_num++ ;
			}
			//out http://
			char main_buff[UL_MAX_URL_LEN] ;
			main_buff[0] = '\0';
			fetch_maindomain_from_url(phref , main_buff , UL_MAX_URL_LEN ) ;
			if(*main_buff)
			{
				unsigned int hash = simple_hash(main_buff) ;
				hash_4_dup_mp[hash]++ ;
				if(hash_4_dup_mp[0]< hash_4_dup_mp[hash] )
				{
					hash_4_dup_mp[0] = hash_4_dup_mp[hash] ;
				}
			}
			if(is_icp_link_node(vnode))
			{
				d_link->is_icp_anchor = true ;
			}
		}

		//计算anchor信息
		html_vnode_t * pure_vnode = get_pure_text_child_a(vnode) ;
		if(pure_vnode!=NULL && pure_vnode->hpNode->html_tag.text != NULL
				&& is_last_special_word(pure_vnode->hpNode->html_tag.text, for_friend_link_inanchor))
		{
			d_link->spec_anchor_num++ ;
		}

		if(looper == paoi->link_info.url_vnode_list_end )
		{
			break ;
		}
		looper = looper->next ;
	}//end while

	d_link->max_md_dup_num = hash_4_dup_mp[0] ;
}
/**
 * 判断该块的祖先在指定深度内是否为link块
 */
static bool is_has_parent_link(const html_area_t * area , int level )
{
		if(area==NULL)
			return false ;
		html_area_t * finder = area->parentArea ;
		int i = 0 ;
		while(finder!=NULL)
		{
			if(finder->isValid == false )
				break ;
			if(is_srctype_area(finder,AREA_SRCTYPE_LINK))
				return true ;
			finder = finder->parentArea ;
			i++ ;
			if(i >= level )
			{
				break ;
			}
		}
		return false ;
}

static bool is_func_friend_area(const html_area_t * area ,mark_area_info_t * g_info )
{
	//必须是链接块或者是链接块的子块
	if(!is_srctype_area(area,AREA_SRCTYPE_LINK) && !is_has_parent_link(area , 2 ))
	{
//		Debug( "friend_%d_%d_f%d" , area->depth , area->order  , __LINE__) ;
		marktreeprintfs(MARK_FRIEND,"because the area is not link area,it is not friend at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
	friend_link_info_t d_link_info ;
	get_friend_link_info4area(area , g_info->page_url , &d_link_info) ;
	if(d_link_info.out_num <=0 )
	{
//		Debug( "friend_%d_%d_f%d" , area->depth , area->order  , __LINE__) ;
		marktreeprintfs(MARK_FRIEND,"because the link_out <=0,it is not friend at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
	if(d_link_info.out_num <=2 && d_link_info.is_icp_anchor == true && area->abspos_mark == PAGE_FOOTER )
	{
//		Debug( "friend_%d_%d_f%d" , area->depth , area->order  , __LINE__) ;
		marktreeprintfs(MARK_FRIEND,"because the area is icp and is at foot,it is not friend at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
	if(d_link_info.inner_num > 2*d_link_info.out_num && d_link_info.out_num >=3 )
	{
//		Debug( "friend_%d_%d_f%d" , area->depth , area->order  , __LINE__) ;
		marktreeprintfs(MARK_FRIEND,"because the link_inner(%d)>2*the link_out(%d),it is not friend at %s(%d)-%s\r\n",d_link_info.inner_num,d_link_info.out_num,__FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
	else if(d_link_info.inner_num >=8 )
	{
//		Debug( "friend_%d_%d_f%d" , area->depth , area->order  , __LINE__) ;
		marktreeprintfs(MARK_FRIEND,"because the link_inner(%d)>=8,it is not friend at %s(%d)-%s\r\n",d_link_info.inner_num,__FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
	//获取friend title
	bool ret = is_area_has_spec_word_by_index(area , for_friend_link_title);
	if(ret == false )
	{
		marktreeprintfs(MARK_FRIEND,"the area don't contain friend link title words at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		html_area_t * left = get_first_left_valid_area( area) ;
		if(left !=NULL && !is_func_area(left,AREA_FUNC_FRIEND))
		{
			ret = is_area_has_spec_word_by_index(left  , for_friend_link_title);
		}
		marktreeprintfs(MARK_FRIEND,"the left area of the area  contain friend link title words=%d at %s(%d)-%s\r\n",ret,__FILE__,__LINE__,__FUNCTION__);
	}
	//非链接块，不含有friend title 返回
	if(!is_srctype_area(area, AREA_SRCTYPE_LINK) && ret == false ){
		if(d_link_info.out_num == 0 || d_link_info.out_num < d_link_info.inner_num )
		{
//			Debug( "friend_%d_%d_f%d" , area->depth , area->order  , __LINE__) ;
			marktreeprintfs(MARK_FRIEND," the area  is not link area and its left not contains friend word and out link(%d)<inner link(%d),it is not friend area at %s(%d)-%s\r\n",d_link_info.out_num,d_link_info.inner_num,__FILE__,__LINE__,__FUNCTION__);
			return false ;
		}
	}
	if(d_link_info.inner_num == 0 && d_link_info.out_num == d_link_info.max_md_dup_num )
	{
//		Debug( "friend_%d_%d_f%d" , area->depth , area->order  , __LINE__) ;
		marktreeprintfs(MARK_FRIEND,"the out links are all the same  at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
	//策略1 根据"友情链接"  "网 ，站" 等判断
	if(ret == true)
	{
		if (d_link_info.spec_anchor_num >=2 && d_link_info.out_num >=1 && d_link_info.out_num >= d_link_info.inner_num )
		{
//			Debug( "friend_%d_%d_t%d" , area->depth , area->order  , __LINE__) ;
			return true ;
		}
		if(d_link_info.top_num *10 >= d_link_info.out_num*6 && d_link_info.out_num>=3 )
		{
//			Debug( "friend_%d_%d_t%d" , area->depth , area->order  , __LINE__) ;
			return true ;
		}
		else if(d_link_info.top_num == d_link_info.out_num && d_link_info.out_num >0 && d_link_info.out_num > d_link_info.inner_num)
		{
//			Debug( "friend_%d_%d_t%d" , area->depth , area->order  , __LINE__) ;
			return true ;
		}
		else if(d_link_info.out_num >=3 && d_link_info.out_num  > d_link_info.inner_num*3 )
		{
			if(d_link_info.top_num >=2 )
			{
//				Debug( "friend_%d_%d_t%d" , area->depth , area->order  , __LINE__) ;
				return true ;
			}
		}
	}
	if(d_link_info.out_num >=4 && d_link_info.out_num > d_link_info.inner_num
	 && d_link_info.top_num *10 >= d_link_info.out_num*7 && area->abspos_mark == PAGE_FOOTER )
	{
		if(d_link_info.max_md_dup_num <=2 )
		{
//			Debug( "friend_%d_%d_t%d" , area->depth , area->order  , __LINE__) ;
			return true ;
		}
	}
	//策略 内链数太多，外链数太少
	if( ret == false  && (d_link_info.inner_num > 2 || d_link_info.out_num < 2 ) )
	{
//		Debug( "friend_%d_%d_f%d" , area->depth , area->order  , __LINE__) ;
		marktreeprintfs(MARK_FRIEND," the area  is not link area and its left not contains friend word and out link(%d)<2 or inner link(%d)>2,it is not friend area at %s(%d)-%s\r\n",d_link_info.out_num,d_link_info.inner_num,__FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
	if( d_link_info.max_md_dup_num *2 > d_link_info.out_num && d_link_info.out_num > 4 )
	{
//		Debug( "friend_%d_%d_f%d" , area->depth , area->order  , __LINE__) ;
		marktreeprintfs(MARK_FRIEND,"  out link(%d)<max_md_dup_num*2(%d),it is not friend area at %s(%d)-%s\r\n",d_link_info.out_num,d_link_info.max_md_dup_num,__FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
	int score = 0 ;
	score = 100*(d_link_info.top_num + d_link_info.spec_anchor_num)/(2*d_link_info.out_num) ;
	marktreeprintfs(MARK_FRIEND," top_num=%d, out link=%d,spec_anchor_num=%d ,inner_num=%dat %s(%d)-%s\r\n",d_link_info.top_num,d_link_info.out_num,d_link_info.spec_anchor_num,d_link_info.spec_anchor_num,__FILE__,__LINE__,__FUNCTION__);
	if(d_link_info.inner_num>0)
	{
		score = score * d_link_info.out_num/(d_link_info.inner_num+d_link_info.out_num) ;
	}

	if(area->abspos_mark == PAGE_HEADER )
	{
		score = score/2 ;
		marktreeprintfs(MARK_FRIEND," the area  is at page header,score =score/2 at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
	}
	else if(area->abspos_mark != PAGE_FOOTER && area->abspos_mark != PAGE_LEFT)
	{
		score = score - 10 ;
		marktreeprintfs(MARK_FRIEND," the area  is not left or and right score =score -10 at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
	}

	//策略3 根据特殊词降权
    ret = is_area_has_spec_word_by_index( area , for_friend_link_no) ;
	if(ret)
	{
		score = score - 10 ;
		marktreeprintfs(MARK_FRIEND," the area  contain copyright word,score=score-10 at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
	}

//		Debug("friend_%d_%d[score=%d top=%d spec=%d max_md=%d spe=%d]" , area->depth , area->order,
//		score , d_link_info.top_num , d_link_info.spec_anchor_num , d_link_info.max_md_dup_num , ret );
	marktreeprintfs(MARK_FRIEND,"the score of the area =%d at %s(%d)-%s\r\n",score,__FILE__,__LINE__,__FUNCTION__);
	if(score > 50)
	{
//		Debug( "friend_%d_%d_t%d" , area->depth , area->order  , __LINE__) ;
		return true ;
	}

//	Debug( "friend_%d_%d_f%d" , area->depth , area->order  , __LINE__) ;
	return false ;
}

static int start_mark_friend(html_area_t *area, mark_area_info_t *mark_info)
{
	if(MARK_FRIEND==g_EASOU_DEBUG){
		printlines("mark friend");
		printNode(area->begin->hpNode);
	}
	if(!area->isValid){
		 marktreeprintfs(MARK_FRIEND,"the area is not valid,skip it at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP;
	}

	if(area->baseinfo->link_info.out_num <= 0){
		marktreeprintfs(MARK_FRIEND,"the out link number of the area is <0,skip it at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP;
	}

	if(is_func_friend_area(area, mark_info)){
		//todo
//		char buf[1024];
//		memset(&buf, 0, 1024);
//		extract_area_content(buf, sizeof(buf), area);
//		printf("friend:	%s\n", buf);
		marktreeprintfs(MARK_FRIEND,"the area is friend area!!!!!!!!! at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		tag_area_func(area, AREA_FUNC_FRIEND ) ;
		return AREA_VISIT_SKIP ;
	}

	return AREA_VISIT_NORMAL;
}

bool mark_func_friend(area_tree_t *atree)
{
	bool ret = areatree_visit(atree,
			(FUNC_START_T)start_mark_friend,
			NULL,atree->mark_info);

	return ret;
}


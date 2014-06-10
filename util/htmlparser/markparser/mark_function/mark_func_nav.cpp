/*
 * mark_func_nav.cpp
 *
 *  Created on: 2011-11-22
 *      Author: ddt
 */
#include "easou_string.h"
#include "easou_url.h"
#include "easou_lang_relate.h"
#include "easou_mark_inner.h"
#include "easou_mark_srctype.h"
#include "easou_mark_func.h"
#include "easou_mark_com.h"
#include "easou_html_attr.h"
#include "debuginfo.h"
/**
* @brief 导航条相关的链接信息.
*/
typedef struct _nav_link_info_t
{
	int inner_num ;//内链个数
	int out_num ;  //外链个数
	int ave_anchor_size ; //平均anchor长度
	int pic_link_num ;    //图片链接个数
	int longest_anchor_len;		  /**< 最大anchor长度       */
	int anchor_len_variable;
	int no_nav_num ; 						  //是否出现导航条不应该出现的anchor
	bool is_has_topanchor ;                   //是否出现"首页"anchor
	int samelenCount;
	int err_num;
	int not_fit_word;
}nav_link_info_t ;

/**
* @brief 导航条相关的文本信息.
*/
typedef struct _nav_text_info_t
{
	bool is_has_hot ;        //是否出现热点，排行，最新等，相关链接不应该出现的词汇
}nav_text_info_t ;

/**
 * @brief 计算候选导航条的文本信息.
**/
void get_nav_text_info4area(const html_area_t * area , nav_text_info_t * dt)
{
	area_baseinfo_t * paoi = (area_baseinfo_t *) area->baseinfo ;
	memset(dt , 0 , sizeof(nav_text_info_t)) ;
    vnode_list_t * looper = paoi->text_info.cont_vnode_list_begin ;
	while( looper != NULL )
	{
		assert(looper->vnode->hpNode->html_tag.tag_type==TAG_PURETEXT) ;
		char * p_text = looper->vnode->hpNode->html_tag.text ;
		bool b_anchor = is_anchor(looper->vnode) ;
		if(p_text == NULL || *p_text == 0 )
		{
			looper=looper->next ;
			continue ;
		}

		//排行榜
		if(b_anchor == false && dt->is_has_hot==false )
		{
			if(is_has_special_word(hot_rank , p_text ))
			{
				marktreeprintfs(MARK_NAV,"the area contain hot_rank word,node text=%s at %s(%d)-%s\r\n",p_text,__FILE__,__LINE__,__FUNCTION__);
				dt->is_has_hot = true ;
			}
		}
		//循环退出条件
		if(looper == paoi->text_info.cont_vnode_list_end )
		{
			break ;
		}
		looper = looper->next ;
	}
}

/**
 * @brief 计算关于候选导航条的链接信息.
**/
void get_nav_link_info4area(const html_area_t * area , const char * base_url , nav_link_info_t * d_link)
{
	assert(area!=NULL && area->baseinfo !=NULL ) ;
	area_baseinfo_t * paoi = (area_baseinfo_t *) area->baseinfo ;
	memset(d_link , 0 , sizeof(nav_link_info_t )) ;
    vnode_list_t * looper = paoi->link_info.url_vnode_list_begin ;
    vnode_list_t * txtlooper = paoi->text_info.cont_vnode_list_begin ;
    int txtcount=0;
    while(txtlooper!=NULL&&txtcount++<1){
    	html_vnode_t * pure_vnode=txtlooper->vnode;
    	if(pure_vnode !=NULL && is_has_special_word(typeword ,pure_vnode->hpNode->html_tag.text ))
    			{
    				d_link->not_fit_word++ ;
    			}
    	if(txtlooper==paoi->text_info.cont_vnode_list_end){
    		break;
    	}
    }
  if(d_link->not_fit_word==0&&area->prevArea&&area->prevArea->area_info.xpos==0&&area->prevArea->area_info.ypos==area->area_info.ypos){
	  txtlooper = area->prevArea->baseinfo->text_info.cont_vnode_list_begin ;
	     txtcount=0;
	      if(txtlooper!=NULL){
	      	html_vnode_t * pure_vnode=txtlooper->vnode;
	      	if(pure_vnode !=NULL && is_has_special_word(typeword ,pure_vnode->hpNode->html_tag.text ))
	      			{
	      				d_link->not_fit_word++ ;
	      			}

	      }
  }
	if(paoi->link_info.num != 0 )
	{
		d_link->ave_anchor_size = paoi->link_info.anchor_size / paoi->link_info.num ;

	}
    int mypos=0;
	while( looper != NULL )
	{
		html_vnode_t * vnode = looper->vnode ;
		int ret = get_link_type(vnode , base_url ) ;
		if ( ret == IS_LINK_INNER )
		{
			d_link->inner_num++ ;
		}
		if( ret == IS_LINK_OUT )
		{
			d_link->out_num++ ;
		}
		 if( ret == IS_LINK_ERR )
		{
			d_link->err_num++ ;
		}
		if(is_pic_link(vnode))
		{
			d_link->pic_link_num++ ;
		}
		if(vnode->prevNode&&vnode->prevNode->hpNode->html_tag.tag_type==TAG_PURETEXT){
			if(strstr(vnode->prevNode->hpNode->html_tag.text,"&gt;")){
				mypos++;
			}
		}
		//计算anchor信息
			html_vnode_t * pure_vnode = get_pure_text_child_a(vnode) ;
		//todo
		//int len = get_real_anchor_size(vnode) ;
		int len = 0 ;
		if(pure_vnode){
			len=strlen(pure_vnode->hpNode->html_tag.text);
		}
//		printf("len:	%d", len);

        if((len<=(d_link->ave_anchor_size+3)&&len>=(d_link->ave_anchor_size-3))&&(paoi->link_info.num != 0 )){
        	d_link->samelenCount++;
        }
		if(len > d_link->longest_anchor_len){
			d_link->longest_anchor_len = len;
		}

		if(len > 0){
			int delt_len = len - d_link->ave_anchor_size;
			d_link->anchor_len_variable += delt_len * delt_len;
		}




		if(pure_vnode !=NULL )
		{
			bool ishastop=false;
			ishastop=is_has_special_word(topanchor,pure_vnode->hpNode->html_tag.text );
					marktreeprintfs(MARK_NAV," ,anchor =%s,ishastop=%d at %s(%d)-%s\r\n",pure_vnode ? pure_vnode->hpNode->html_tag.text:"null",ishastop,__FILE__,__LINE__,__FUNCTION__);
			if(ret== IS_LINK_INNER &&ishastop){
				d_link->is_has_topanchor = true ;
			}
			else{
				marktreeprintfs(MARK_NAV," ,it contain top not inner at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
			}

		}

		if(pure_vnode !=NULL && is_has_special_word(no_nav_anchor ,pure_vnode->hpNode->html_tag.text ))
		{
			d_link->no_nav_num++ ;
			d_link->is_has_topanchor =false;
		}

		if(looper == paoi->link_info.url_vnode_list_end )
		{
			break ;
		}
		looper = looper->next ;
	}//end while
    if(mypos>1){
    	d_link->no_nav_num++ ;
    	d_link->is_has_topanchor =false;
    }
	if(paoi->link_info.num > 0){
		//d_link->anchor_len_variable /= paoi->link_info.num;
	}
}

/**
 * @brief 当前粒度是否密度足够高.
**/
static bool is_dense_nav(const html_area_t *area)
{
	int link_area = area->baseinfo->link_info.link_area;
	int link_num = area->baseinfo->link_info.num;

	for(html_area_t *s = area->subArea; s; s = s->nextArea){
		if(!s->isValid)
			continue;
		if(s->baseinfo->link_info.link_area*10 >= link_area*7){
			marktreeprintfs(MARK_NAV,"it is not dense  for sub link area(%d)*10>=link_area(%d)*7 at %s(%d)-%s\r\n",s->baseinfo->link_info.link_area,link_area,__FILE__,__LINE__,__FUNCTION__);
			return false;
		}

		if(s->baseinfo->link_info.num*10 >= link_num*7){
			marktreeprintfs(MARK_NAV,"it is not dense  for sub link num(%d)*10>=link_num(%d)*7 at %s(%d)-%s\r\n",s->baseinfo->link_info.num,link_num,__FILE__,__LINE__,__FUNCTION__);
			return false;
		}

		html_area_t *n = next_valid_area(s);
		if(n != NULL && n->baseinfo->link_info.num == 0){
			/*被分隔区域隔为多个链接块*/
			if(s->baseinfo->link_info.num >= 4&&s->baseinfo->link_info.num*2>link_num){
				marktreeprintfs(MARK_NAV,"it is not dense  for next area not link at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
				return false;}
		}
	}
	marktreeprintfs(MARK_NAV,"it is dense  for nav at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
	return true;
}

static bool is_in_page_header(const html_area_t *area, html_area_t *mypos)
{
	if(mypos != NULL){
		if(area->area_info.ypos >= mypos->area_info.ypos)
			{
			marktreeprintfs(MARK_NAV,"it is not header for mypos at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
			return false;
			}
		else
			{
			marktreeprintfs(MARK_NAV,"it is  header for mypos at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
			return true;
			}
	}

	if(area->abspos_mark == PAGE_HEADER){
		marktreeprintfs(MARK_NAV,"it is  header for abspos_mark at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return true;}
	marktreeprintfs(MARK_NAV,"it is  not header at last at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
	return false;
}

/**
 * @brief 是否合适的导航条粒度.
**/
static bool is_nav_gran(const html_area_t *area, area_tree_t *atree, html_area_t *mypos)
{
	/**导航栏的交互块很少*/
	if(area->baseinfo->inter_info.input_num + area->baseinfo->inter_info.select_num >= 3
			|| is_contain_srctype_area(area, AREA_SRCTYPE_INTERACTION)){
		marktreeprintfs(MARK_NAV," the area may be interaction area ,it is not nav at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return false;
	}
	/**非href链接过多，不是导航栏*/
	if(area->baseinfo->link_info.other_num*3 >= area->baseinfo->link_info.num){
		marktreeprintfs(MARK_NAV,"other_num(%d)*3 >= link_info.num(%d) ,it is not nav at %s(%d)-%s\r\n",area->baseinfo->link_info.other_num,area->baseinfo->link_info.num,__FILE__,__LINE__,__FUNCTION__);
		return false;
	}
	area_baseinfo_t *baseinfo = area->baseinfo;
	if(baseinfo->link_info.num < 2){
		marktreeprintfs(MARK_NAV," link_info.num(%d)<2 ,it is not nav at %s(%d)-%s\r\n",area->baseinfo->link_info.num,__FILE__,__LINE__,__FUNCTION__);
		return false;
	}
	int tot_area = baseinfo->extern_info.extern_area + baseinfo->inter_info.in_area
			+ baseinfo->pic_info.link_pic_area /*为避免导航栏中大图片的影响，只计链接图片面积*/
			+ baseinfo->text_info.text_area - baseinfo->text_info.no_use_text_area;

	if(baseinfo->link_info.link_area * 10 < tot_area * 7
			|| !is_dense_nav(area)){
		marktreeprintfs(MARK_NAV,"link area(%d) < total area(%d) *0.7 or sub area is dense ,it is not nav at %s(%d)-%s\r\n",baseinfo->link_info.link_area,tot_area,__FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	html_area_t *root = atree->root;

	if(area->area_info.width <= 300){
		if(area->area_info.height >= area->area_info.width){
			if(area->valid_subArea_num<1){
				return true;
			}
			else{
				if(area->subArea->baseinfo->link_info.num>2){
					return false;
				}

			}

		}
		if(area->area_info.width <= 100 || area->area_info.height >= 20&&area->valid_subArea_num<1)
			return true;
	}

	if(is_in_page_header(area, mypos) && area->area_info.height <= 200){
		if(area->area_info.width >= 3*area->area_info.height&&is_srctype_area(area,AREA_SRCTYPE_HUB))
			return true;
		if(area->area_info.width >= 600&&is_srctype_area(area,AREA_SRCTYPE_HUB))
			return true;
		if(area->area_info.width*2 >= root->area_info.width&&is_srctype_area(area,AREA_SRCTYPE_HUB))
			return true;
	}


	marktreeprintfs(MARK_NAV,"the judge reach the end at last ,it is not nav at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
	return false;
}

static bool is_in_pagesider(const html_area_t *area)
{
	for(const html_area_t *up = area; up; up = up->parentArea){
		if(up->depth == 1){
			if(up->pos_mark == RELA_LEFT||up->area_info.xpos<10){
				marktreeprintfs(MARK_NAV,"it is left ,depth=1 ,it is in_pagesider at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
				return true;
			//break;
			}
		}
		else if(up->depth == 2){
			if(up->pos_mark == RELA_LEFT && up->abspos_mark == PAGE_LEFT)
				{marktreeprintfs(MARK_NAV,"it is left,depth=2,it is in_pagesider at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
				return true;}
		}
	}

	return false;
}

static bool is_nav_shape_pics(const html_area_t *area)
{
	int tot_pic_wx = 0;
	int tot_pic_hx = 0;
	int tot_pic_wx_sqr = 0;
	int tot_pic_hx_sqr = 0;

	int same_xpos_num = 0;
	int same_ypos_num = 0;

	vnode_list_t *vlist = area->baseinfo->pic_info.pic_vnode_list_begin;

	int first_xpos = vlist->vnode->xpos;
	int first_ypos = vlist->vnode->ypos;

	for(; vlist; vlist = vlist->next){
		html_vnode_t *vnode  = vlist->vnode;
		tot_pic_wx += vnode->wx;
		tot_pic_hx += vnode->hx;
		tot_pic_wx_sqr += vnode->wx * vnode->wx;
		tot_pic_hx_sqr += vnode->hx * vnode->hx;

		if(vnode->xpos == first_xpos)
			same_xpos_num ++;
		if(vnode->ypos == first_ypos)
			same_ypos_num ++;

		if(vlist == area->baseinfo->pic_info.pic_vnode_list_end)
			break;
	}

	int pic_wx_var = 0;
	int pic_hx_var = 0;
	int pic_num = area->baseinfo->pic_info.pic_num;

	if(same_xpos_num != pic_num && same_ypos_num != pic_num){
		marktreeprintfs(MARK_NAV,"pic_num=%d,same_xpos_num=%d,same_ypos_num=%d ,it is not nav at %s(%d)-%s\r\n",pic_num,same_xpos_num,same_ypos_num,__FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	if(pic_num > 0){
		int ave_wx = tot_pic_wx / pic_num;
		int ave_hx = tot_pic_hx / pic_num;
		pic_wx_var = tot_pic_wx_sqr / pic_num - ave_wx * ave_wx;
		pic_hx_var = tot_pic_hx_sqr / pic_num - ave_hx * ave_hx;
	}

	if(same_xpos_num == pic_num){
		if(pic_wx_var == 0 && pic_hx_var <= 50)
			return true;
	}

	if(same_ypos_num == pic_num){
		if(pic_hx_var == 0 && pic_wx_var <= 100)
			return true;
	}
	marktreeprintfs(MARK_NAV,"the judge reach the end at last ,it is not nav at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
	return false;
}

typedef struct _nav_para_t{
	area_tree_t *atree;
	html_area_t *mypos;
	html_area_t * navarea;
	html_area_t * navarea_2;
	int pathdepth;
	int pathdepth_2;
	bool hastop;
	bool hasNav;
}nav_para_t;

bool hasNavAtrr(const html_area_t * area){
	if(area){
		if(area->begin&&area->begin==area->end){
			char *value=html_node_get_attribute_value(area->begin->hpNode,"id");
			if(value&&easou_strcasestr(value,"nav")){
				 marktreeprintfs(MARK_NAV,"node<%s> id =%s at %s(%d)-%s\r\n",area->begin->hpNode->html_tag.tag_name,value,__FILE__,__LINE__,__FUNCTION__);
				return true;
			}
			value=html_node_get_attribute_value(area->begin->hpNode,"class");
			if(value&&easou_strcasestr(value,"nav")){
				 marktreeprintfs(MARK_NAV,"node<%s> class =%s at %s(%d)-%s\r\n",area->begin->hpNode->html_tag.tag_name,value,__FILE__,__LINE__,__FUNCTION__);
							return true;
			}
		}
	}else{
		return false;
	}
	return false;
}
static bool is_func_nav(const html_area_t * area , nav_para_t * para)
{
	mark_area_info_t *g_info = para->atree->mark_info;
	if(area->nextArea&&area->nextArea->area_info.ypos==area->area_info.ypos&&area->nextArea->area_info.height>0&&area->nextArea->area_info.width>0&&((area->area_info.xpos+area->area_info.width)*3<area->area_tree->root->area_info.width)&&area->area_info.width>area->area_info.height*2&&area->area_tree->root->area_info.width<10000){
		marktreeprintfs(MARK_NAV,"the area is at left and is width>height ,it is not nav at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
				return false ;
	}
	if((area->area_info.xpos<5||(area->area_info.xpos+area->area_info.width-area->area_tree->root->area_info.width<5))&&area->area_info.ypos<5&&area->valid_subArea_num<1&&area->area_info.width*2<area->area_tree->root->area_info.width){
		marktreeprintfs(MARK_NAV,"the area is at left and top ,it is not nav at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
						return false ;
	}
	if((area->nextArea&&area->nextArea->area_info.height>0&&area->nextArea->area_info.width>0)&&area->area_info.width*3<area->area_tree->root->area_info.width&&area->area_info.width>area->area_info.height*2&&(area->area_info.xpos)*4>area->area_tree->root->area_info.width){
		marktreeprintfs(MARK_NAV,"area->area_info.width*3<area->area_tree->root->area_info.width and next area(%d) is valid ,it is not nav at %s(%d)-%s\r\n",area->nextArea->no,__FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
	if((!(area->nextArea)||area->nextArea&&area->nextArea->area_info.xpos<area->area_info.xpos)&&area->area_info.xpos*3>area->area_tree->root->area_info.width&&area->area_info.width<area->area_info.height){
			marktreeprintfs(MARK_NAV,"area->area_info.xpos*2>area->area_tree->root->area_info.width and next area is null ,it is not nav at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
			return false ;
		}
	nav_link_info_t d_link_info ;
	d_link_info.is_has_topanchor=false;
	if(area->baseinfo->link_info.anchor_size*3<area->baseinfo->text_info.con_size){
			marktreeprintfs(MARK_NAV,"the area is not nav , for anchor_size*3< con_size at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
			return false;
		}
	get_nav_link_info4area(area , g_info->page_url  ,&d_link_info) ;
   if(d_link_info.not_fit_word>0){
	   marktreeprintfs(MARK_NAV,"the area is not fit for contain ':' ,it is not nav at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
	   return false;
   }

	nav_text_info_t d_text_info ;
	get_nav_text_info4area(area , &d_text_info) ;
    if(area->baseinfo->link_info.num<5&&(area->baseinfo->inter_info.spec_word_num&&d_link_info.is_has_topanchor == false||area->baseinfo->inter_info.input_num>0||area->baseinfo->inter_info.is_have_form||area->baseinfo->link_info.anchor_size<4)){

    	 marktreeprintfs(MARK_NAV,"the area is not fit for area->baseinfo->link_info.num<5 at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
    	return false ;
    }

    if((area->area_info.xpos)*6>area->area_tree->root->area_info.width&&(area->area_info.xpos+area->area_info.width)*2<area->area_tree->root->area_info.width&&area->area_info.width>area->area_info.height*4){
    	 marktreeprintfs(MARK_NAV,"the area is middle area,it is not nav at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
    	return false ;
    }
	//策略6 排除包含有明显不是导航条词汇的链接
	if(d_link_info.no_nav_num >=1&&area->baseinfo->link_info.num<10|| d_link_info.no_nav_num >=2|| d_text_info.is_has_hot==true )
	{
//		Debug("nav_%d_%d_f%d" , area->depth , area->order,__LINE__ );
		marktreeprintfs(MARK_NAV,"no_nav_num(%d) >= 1 or is_has_hot(%d) >= true ,it is not nav at %s(%d)-%s\r\n",d_link_info.no_nav_num,d_text_info.is_has_hot,__FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
	//策略 3 出现首页，并且位于header中的一行
	if(d_link_info.is_has_topanchor == true && d_link_info.inner_num > d_link_info.out_num)
	{

		int flag = false ;
		if( (area->abspos_mark==PAGE_HEADER )&&is_a_like_one_row(area))
		{
			flag = true ;
		}
		else if((area->abspos_mark==PAGE_LEFT ||area->area_info.xpos<1)&&is_a_like_one_col(area))
		{
			flag = true ;
		}
		if( area->area_info.ypos*3<area->area_tree->root->area_info.height &&area->srctype_mark._mark_bits&24==24)
		{
					flag = true ;
		}
		if(flag == true )
		{
			para->hastop=true;
//			Debug( "nav_%d_%d_t%d" , area->depth , area->order,__LINE__ );
			marktreeprintfs(MARK_NAV,"the area is nav for has top at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
			return true ;
		}
	}
//	if(d_link_info.err_num>0&&area->area_info.ypos==0){
//			marktreeprintfs(MARK_NAV,"the area is not fit for err_link ,it is not nav at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
//			return false  ;
//		}


	//策略4 ，内链太少，外链太多
	if( d_link_info.inner_num < 4 || d_link_info.out_num*3 > d_link_info.inner_num )
	{
//		Debug( "nav_%d_%d_f%d" , area->depth , area->order,__LINE__ );
		marktreeprintfs(MARK_NAV,"the inner link of the area is <=4 or the out_link(%d) *3 > the inner_link(%d) ,it is not nav at %s(%d)-%s\r\n",d_link_info.out_num,d_link_info.inner_num,__FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
    int xishu=area->baseinfo->link_info.num/20;
	if(d_link_info.longest_anchor_len >= 20 || d_link_info.anchor_len_variable >= 125*(1+xishu*0.5)){
		marktreeprintfs(MARK_NAV,"longest_anchor_len(%d) >= 30 or anchor_len_variable(%d) >= 125 ,it is not nav at %s(%d)-%s\r\n",d_link_info.longest_anchor_len,d_link_info.anchor_len_variable,__FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	if(d_link_info.longest_anchor_len >= 16 && d_link_info.anchor_len_variable >= 20){
		marktreeprintfs(MARK_NAV,"longest_anchor_len(%d) >= 20 and anchor_len_variable(%d) >= 25 ,it is not nav at %s(%d)-%s\r\n",d_link_info.longest_anchor_len,d_link_info.anchor_len_variable,__FILE__,__LINE__,__FUNCTION__);
		return false;
	}
    if(d_link_info.samelenCount*4<area->baseinfo->link_info.num*2){
    	marktreeprintfs(MARK_NAV,"the number of same lenght node is less than 0.5,samelenCount=%d,link count=%d,is not nav at %s(%d)-%s\r\n",d_link_info.samelenCount,area->baseinfo->link_info.num,__FILE__,__LINE__,__FUNCTION__);
    			return false;
    }

	if(area->parentArea&&area->parentArea->valid_subArea_num > 20&&(area->begin->hpNode->html_tag.tag_type)==TAG_UL){
		marktreeprintfs(MARK_NAV,"brother count=%d ,is not nav at %s(%d)-%s\r\n",area->parentArea->valid_subArea_num,__FILE__,__LINE__,__FUNCTION__);
		return false ;
	}
	//策略8 大部分是内链，并且大部分的链接anchor比较短
	if(d_link_info.inner_num >= d_link_info.out_num * 3)
	{
		//anchor均值为2-4个字，此处默认输入是的编码是gbk big5 gb18030也不会有问题
		if(  d_link_info.ave_anchor_size <= 16 && d_link_info.ave_anchor_size  >= 3 )
		{
//			Debug("nav_%d_%d_t%d" , area->depth , area->order,__LINE__ );
			return true ;
		}
		if(area->baseinfo->pic_info.link_pic_num >= 2
				&& area->baseinfo->pic_info.link_pic_num >= area->baseinfo->link_info.num - 1
				&& is_nav_shape_pics(area)){
			return true;
		}
	}
	 if(hasNavAtrr(area)&&d_link_info.inner_num>2&&!hasNavAtrr(area->nextArea)&&!hasNavAtrr(area->prevArea)){
	   		 marktreeprintfs(MARK_NAV,"the area is nav for attr contain nav at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
	   		para->hastop=true;
	   		return true;
	   	}
//	Debug("nav_%d_%d_f%d" , area->depth , area->order,__LINE__ );
	marktreeprintfs(MARK_NAV,"the area is not fit for the condition of the nav at last ,it is not nav at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
	return false  ;
}

static int start_mark_nav(html_area_t *area, nav_para_t *para)
{
	if(MARK_NAV==g_EASOU_DEBUG){
		myprintf("-----mark nav,area no=%d\n",area->no);
		printNode(area->begin->hpNode);
	}
	if(!area->isValid){
		marktreeprintfs(MARK_NAV,"the area is not valid,skip it at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP;
	}


    if(area->area_info.ypos*5>=area->area_tree->root->area_info.height*4||para->mypos&&(area->area_info.ypos-para->mypos->area_info.ypos)>500){
    	return AREA_VISIT_SKIP;
    }
	if(is_func_area(area, AREA_FUNC_MYPOS)
			|| is_func_area(area, AREA_FUNC_RELATE_LINK)
			|| is_func_area(area, AREA_FUNC_COPYRIGHT)|| is_srctype_area(area, AREA_SRCTYPE_INTERACTION)
			){
		marktreeprintfs(MARK_NAV,"the area is mypos or relatelink or copyright,skip it at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP;
	}

	if(area->baseinfo->link_info.inner_num <= 3&&area->baseinfo->link_info.inner_num<area->baseinfo->link_info.num){
		marktreeprintfs(MARK_NAV,"the inner link of the area is <=3,skip it at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP;
	}

	if(is_in_func_area(area,AREA_FUNC_MYPOS)
			|| is_in_func_area(area, AREA_FUNC_RELATE_LINK)
			|| is_in_func_area(area, AREA_FUNC_COPYRIGHT)
			){
		marktreeprintfs(MARK_NAV,"skip it for  the  area is in mypos relate copyright  at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP;
	}

    if(area->area_info.xpos*3>area->area_tree->root->area_info.width*2){
    	marktreeprintfs(MARK_NAV,"the area is too right,skip it at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
    			return AREA_VISIT_NORMAL;
    }
	//导航条位于上部或者左部分
	if(!is_in_page_header(area, para->mypos) && !is_in_pagesider(area)){
		marktreeprintfs(MARK_NAV,"it is not in page header and not in page side ,it is not nav at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_NORMAL;
	}
    if(area->area_info.ypos*2>=area->area_tree->root->area_info.height||para->mypos&&area->area_info.ypos>para->mypos->area_info.ypos&&para->mypos->area_info.ypos*4>=area->area_tree->root->area_info.height){
    	return AREA_VISIT_SKIP;
    }
	//是否合适粒度
   // para->hasNav=hasNavAtrr(area);
	if(!is_nav_gran(area, para->atree, para->mypos)&&area->valid_subArea_num>0){
		marktreeprintfs(MARK_NAV,"it is   not nav gran at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_NORMAL;
	}
	para->hastop=false;
	if(is_func_nav(area, para)){
		//todo
//		char buf[1024];
//		memset(&buf, 0, 1024);
//		extract_area_content(buf, sizeof(buf), area);
//		printf("nav:	%s\n", buf);
		marktreeprintfs(MARK_NAV,"area no =%d is nav at %s(%d)-%s\r\n",area->no,__FILE__,__LINE__,__FUNCTION__);
        if(!(para->navarea)){
        	//tag_area_func(area, AREA_FUNC_NAV);
        	if(area->baseinfo->link_info.num>10||para->hastop&&area->baseinfo->link_info.num>4){
        		tag_area_func(area, AREA_FUNC_NAV);
        	}
        	para->navarea=area;
        }
        else{

        	if(area->baseinfo->link_info.num>para->navarea->baseinfo->link_info.num||area->prevArea==para->navarea){
        		if(area->area_info.ypos-para->navarea->area_info.ypos<400){
        			tag_area_func(area, AREA_FUNC_NAV);
        		}else{
        			tag_area_func(para->navarea, AREA_FUNC_NAV);
        		}
        		para->navarea=area;
        	}
        	else{
        		tag_area_func(para->navarea, AREA_FUNC_NAV);
        		para->navarea=area;
        	}
           if(para->navarea_2==NULL){
        	   para->navarea_2=area;
           }


        }

	}

	return AREA_VISIT_SKIP;
}

static int visit_for_mypos(html_area_t *area, html_area_t **mypos)
{
	if(!area->isValid)
		return AREA_VISIT_SKIP;

	if(!is_contain_func_area(area, AREA_FUNC_MYPOS))
		return AREA_VISIT_SKIP;

	if(is_func_area(area, AREA_FUNC_MYPOS)){
		*mypos = area;
		return AREA_VISIT_FINISH;
	}

	return AREA_VISIT_NORMAL;
}

static html_area_t * get_mypos_area(html_area_t *root)
{
	html_area_t *mypos = NULL;
	areatree_visit(root, (FUNC_START_T)visit_for_mypos, NULL,&mypos);
	return mypos;
}

bool mark_func_nav(area_tree_t *atree)
{
	/**
	 * 将利用MYPOS来定位
	 */
	html_area_t *mypos = get_mypos_area(atree->root);
	nav_para_t para;
	para.atree = atree;
	para.mypos = mypos;
	para.navarea=NULL;
	para.navarea_2=NULL;
	para.pathdepth=0;
	para.pathdepth_2=0;
	bool ret = areatree_visit(atree, (FUNC_START_T)start_mark_nav, NULL,&para);
    if(para.navarea){
    	if(para.navarea_2==NULL){
    		tag_area_func(para.navarea, AREA_FUNC_NAV);
    	}
    }
	return ret;
}

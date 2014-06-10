/*
 * easou_ahtml_baseinfo.h
 *
 *  Created on: 2011-11-18
 *      Author: ddt
 */

#ifndef EASOU_AHTML_BASEINFO_H_
#define EASOU_AHTML_BASEINFO_H_
#include "nodepool.h"
#include "easou_vhtml_basic.h"
#include "easou_ahtml_tree.h"

/*vnode节点链表*/
typedef struct vnode_list_t
{
	html_vnode_t * vnode ;
	struct vnode_list_t * next;
	struct vnode_list_t * pre;
} vnode_list_t ;

/**块的类型描述*/
/*块的外部信息*/
typedef struct _AOI4ST_extern_t
{
	int extern_area ; /*外部块面积*/
	vnode_list_t * extern_vnode_list_begin ;		  /**<  将外部节点链接起来      */
	vnode_list_t * extern_vnode_list_end ;			  /**<  将外部节点链接起来      */
}AOI4ST_extern_t;

/*块的交互信息*/
typedef struct _AOI4ST_interaction_t
{
	bool is_have_form;				//是否有form标签
	int textarea_num;				//textarea标签数
	int input_num ;  		  		/**<  input 标签个数      */
	int input_radio_num ;		    /**<有radio属性的input标签个数        */
	int input_radio_txtlen ;	    /**<有radio属性的input标签文本长度        */
	int select_num ;		        /**<select标签个数        */
	int option_num ;		        /**<option标签个数        */
	int option_txtlen ;		        /**<option标签文本长度        */
	int in_area ;		            /**<交互标签面积       */
	int cursor_num;								/**<具有cursor属性的标签个数       */
	int script_num;								/**<具有script标签的个数	*/
	int spec_word_num;		/**<具有交互块常见关键词的文本个数     */
	vnode_list_t * interaction_vnode_list_begin ; /**<将交互节点链接起来      */
	vnode_list_t * interaction_vnode_list_end ;   /**<将交互节点链接起来      */
}AOI4ST_interaction_t;

/*块的图片信息*/
typedef struct  _AOI4ST_pic_t
{
	int pic_num ;		  	  /**<img 标签个数        */
	int pic_area ;		      /**<图片面积        */
	int link_pic_num ; 		  /**<图片链接个数        */
	int link_pic_area ;		  /**<图片链接面积        */
	int size_fixed_num;			/**<图片长宽都被设定好的图片个数		*/
	vnode_list_t *pic_vnode_list_begin ; /**<  将图片节点链接起来      */
	vnode_list_t * pic_vnode_list_end ;  /**<  将图片节点链接起来      */
}AOI4ST_pic_t;

/*块的链接信息*/
typedef struct _AOI4ST_link_t
{
	vnode_list_t *url_vnode_list_begin ;/**<  将link节点链接起来      */
	vnode_list_t *url_vnode_list_end ;  /**<  将link节点链接起来      */
	int num ;		                    /**<  正常的超链个数       */
	int inner_num;		  				/**< 内链个数，根据主域判断       */
	int out_num;		  				/**< 外链个数，根据主域判断       */
	int other_num;		  				/**< "javascript:","mailto:"链接个数       */
	int anchor_size ;		            /**< anchor大小       */
	int link_area ;		                /**< 面积       */
	int anchor_size_before;		  		/**< 当前块之前(以深度优先遍历算)的anchor size       */
}AOI4ST_link_t ;

/*块的文本信息*/
typedef struct _AOI4ST_text_t
{
	vnode_list_t *cont_vnode_list_begin ; /**<  将pure_text节点链接起来      */
	vnode_list_t *cont_vnode_list_end ;   /**<  将pure_text节点链接起来      */
	int con_num ;		  				  /**<puretext节点个数        */
	int no_use_con_num ;		          /**<无意义的文本数量，例如连续空格等        */
	int con_size ;						  /**<文本长度        */
	int no_use_con_size;		          /**< 无意义的文本长度       */
	int cn_num;	/**< 中文汉字个数 */
	int text_area ;						  /**<文本块面积        */
	int no_use_text_area ;				  /**<无意义文本块面积        */
	int time_num ;						  /**<时间字符串个数，可能不太准确，不过做识别够了 */
	int con_size_before;		          /**< 当前块之前（以深度优先遍历算）的anchor size       */
	bool recommend_spec_word; /**< 是否含有推荐关键词 */
	float pure_text_rate; /**< 纯文本在当前块之前中所占的比例     */
}AOI4ST_text_t ;


/*块的基本信息,每一个分块都有这么一份信息*/
typedef struct _area_baseinfo_t{
	AOI4ST_extern_t extern_info ;		  /**<块的外部信息        */
	AOI4ST_interaction_t inter_info ;	  /**<块的交互信息        */
	AOI4ST_pic_t  pic_info ;		      /**<块的图片信息        */
	AOI4ST_link_t link_info ;		      /**<块的链接信息        */
	AOI4ST_text_t text_info ;		      /**<块的文本信息        */
}area_baseinfo_t;

/**
 * @brief A树的基本信息
 */
typedef struct _atree_baseinfo_t
{
	int max_text_area_no; /**< 具有最大纯文本比例的分块号      */
	float max_text_rate; /**< 最大纯文本比例      */
	int max_text_leaf_area_no; /**< 具有最大纯文本比例的分块号      */
	float max_text_rate_leaf; /**< 最大纯文本比例      */
} atree_baseinfo_t;

typedef struct _all_vnode_list_t
{
	vnode_list_t * ex_begin ;
	vnode_list_t * ex_end ;
	vnode_list_t * in_begin ;
	vnode_list_t * in_end ;
	vnode_list_t * pic_begin ;
	vnode_list_t * pic_end ;
	vnode_list_t * url_begin ;
	vnode_list_t * url_end  ;
	vnode_list_t * text_begin ;
	vnode_list_t * text_end ;
}all_vnode_list_t ;

/**
* @brief 管理分块标注的公用中间信息使用的内存管理器，atree级别
*/
typedef struct _area_baseinfo_mgr_t
{
	nodepool_t np_area_out_info ;
	nodepool_t np_vnode_list ;
	all_vnode_list_t  all_vnode_list ;
}area_baseinfo_mgr_t ;

/**
 * @brief 初始化baseinfo管理器
**/
int area_baseinfo_mgr_init(area_baseinfo_mgr_t * area_mgr , int m  ,int n);
/**
 * @brief 重置baseinfo管理器
**/
void area_baseinfo_mgr_reset(area_baseinfo_mgr_t * area_mgr);
/**
 * @brief 销毁baseinfo管理器
**/
void area_baseinfo_mgr_des( area_baseinfo_mgr_t * area_mgr );

/**
 * @brief 重置baseinfo
**/
void area_baseinfo_reset(area_baseinfo_mgr_t * area_mgr);
/**
 * @brief 获取baseinfo
**/
area_baseinfo_t * get_area_baseinfo_node(area_baseinfo_mgr_t * area_mgr);
/**
 * @brief 获取baseinfo中的list
**/
vnode_list_t * get_vnode_list_node(area_baseinfo_mgr_t * area_mgr);

/**
 * @brief 完善每个area中的baseinfo
**/
bool fill_base_info(area_tree_t * atree );

#endif /* EASOU_AHTML_BASEINFO_H_ */

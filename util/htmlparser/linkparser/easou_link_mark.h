/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_link_mark.h,v 1.2 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_link_mark.h
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 
 * 	定义链接类型标注的头文件, 内部使用
 **/

#ifndef  __EASOU_LINK_MARK_H_
#define  __EASOU_LINK_MARK_H_

#include "bbsparser.h"
#include "easou_url.h"
#include "easou_ahtml_tree.h"
#include "easou_vhtml_parser.h"
#include "easou_extract_blogtime.h"
#include "easou_link_timematch.h"

struct _vlink_t;

/**
 * @brief short description 标记链接类型用的输入资源
 */
typedef struct _lt_args_t
{
	html_vnode_t *root; //root 结点

	//html_area_t *html_area;	//网页分块信息
	//int area_count;		//网页分块数量
	area_tree_t *atree;

	_vlink_t *vlink; //网页链接集合
	int link_count; //网页链接个数

	char *url; //网页的url
	unsigned int pagetype; //网页类型
	html_vtree_t *vtree;
} lt_args_t;

/**
 * @brief short description 博客正文链接所用的资源
 */
typedef struct _blogmain_res_t
{
	extract_time_paras *pparas; //预编译的时间正则表达式
} blogmain_res_t;

/**
 * @brief short description 内容穿插链接使用的资源
 */
typedef struct _cont_interlude_res_t
{
	html_vnode_t **vnode_set; //vnode 集合
	int vnode_set_used; //vnode_set 使用的数量
	int vnode_set_size; //vnode_set 的大小
} cont_interlude_res_t;

/**
 * @brief short description 链接组信息
 */
typedef struct _group_link_t
{
	int text_len; /**<  文字长度      */
	int anchor_len; /**<  anchor长度      */
	int homepage_count; /**<  主页链接个数      */
	int link_count; /**<  链接个数      */
	int outer_count; /**<  外链个数      */
	int inner_count; /**<  内链个数      */
} group_link_t;

#define MAX_GROUP_NUM 1000

/**
 * @brief short description 网页链接组
 */
typedef struct _group_t
{
	group_link_t groups[MAX_GROUP_NUM]; /**<  链接分组      */
	int group_num; /**<  链接分组个数      */
} group_t;

struct _lt_area_info_t;

/**
 * @brief short description 标记链接所用的资源
 */
typedef struct _lt_res_t
{
	unsigned int flag; //需要标注的链接类型
	blogmain_res_t * res_blogmain; //博客正文资源
	cont_interlude_res_t *res_cont_interlude; //内容穿插链接资源
	timematch_pack_t *ptime_match; /**<  时间判断词典      */
	struct _lt_area_info_t *area_info; /**<  分块统计信息      */
	int *tag_code_len; /**<  按照tagcode索引的文字长度      */
	int tag_code_size; /**<  tagcode索引的最大size      */
	bbs_post_list_t *post_list; /**<  bbsparser解析结构   */
	group_t group; /**<  链接分组结构      */
} lt_res_t;

/**
 * @brief 创建博客正文链接包资源
 * @return  blogmain_res_t* 
 * @retval   NOT NULL	资源
 * @retval	NULL	创建失败
 **/
blogmain_res_t *blogmain_res_create();

/**
 * @brief 删除博客正文链接资源
 * @return  void 
 **/
void blogmain_res_del(blogmain_res_t *);

/**
 * @brief 标记博客正文链接
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常结束
 * @see 	=-1	函数出错
 **/
int mark_linktype_blogmain(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 标记博客回复链接
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_blogre(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 标记bbs回复链接
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_bbsre(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 创建内容穿插链接资源
 * @return  cont_interlude_res_t* 
 * @retval   
 **/
cont_interlude_res_t *cont_interlude_res_create();

/**
 * @brief 删除内容穿插资源
 * @return  void 
 **/
void cont_interlude_res_del(cont_interlude_res_t *);

/**
 * @brief 标记内容穿插链接
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_cont_interlude(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 标记导航链接
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_nav(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 标记版权链接
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_copyright(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 标记nofollow链接
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_nofollow(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 标记mypos链接
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_mypos(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 标记隐藏链接
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_hidden(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 标记css链接
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_css(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 标记图片链接
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_image(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 标记图片资源链接。
 *
 * @param [in/out] pargs   : lt_args_t*    传入参数包
 * @param [in] pres   : lt_res_t*      传入资源
 * @return  int 
 * @retval      >=0 函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_img_src(lt_args_t *pargs, lt_res_t *pres);
/**
 * @brief 标记嵌入资源链接。
 *
 * @param [in/out] pargs   : lt_args_t*    传入参数包
 * @param [in] pres   : lt_res_t*      传入资源
 * @return  int 
 * @retval      >=0 函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_embed_src(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 标记友情链接
 *
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_friend(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 通过链接分组信息标记友情链接
 *
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_friend_by_group(lt_args_t *pargs, lt_res_t *pvres);

/**
 * @brief 标记友情自助链接
 *
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_friendex(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 标记自助链接
 *
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_selfhelp(lt_args_t *pargs, lt_res_t *pres);
/**
 * @brief 标记不可见链接
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_invalid_and_control(lt_args_t *pargs, lt_res_t *pres);
/**
 * @brief 标记引文链接
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_quotation(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 标记穿插文本链接
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int 
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_text(lt_args_t *pargs, lt_res_t *pres);

/**
 * @brief 标记IFRAME链接
 * @param [in/out] pargs   : lt_args_t*	传入参数包
 * @param [in] pres   : lt_res_t*		传入资源
 * @return  int
 * @retval   	>=0	函数正常返回
 * @see 	=-1	函数出错
 **/
int mark_linktype_iframe(lt_args_t *pargs, lt_res_t *pres);

#endif  

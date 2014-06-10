/***************************************************************************
 *
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_link.h,v 1.0 2012/09/01 pageparse Exp $
 *
 **************************************************************************/

#ifndef __EASOU_LINK_H__
#define __EASOU_LINK_H__

#include "log.h"
#include "easou_url.h"
#include "easou_string.h"
#include "easou_vhtml_tree.h"
#include "easou_link_mark.h"
#include "easou_ahtml_tree.h"

#define EXTRACT_MERGE           0	//合并anchor中的空格
#define EXTRACT_NOMERGE         1	//不合并anchor中的空格
typedef int vlink_func_t;

#define VLINK_UNDEFINED 0x00		//未定义链接	0
#define VLINK_NAV	0x01		//导航链接	1<<0
#define VLINK_FRIEND	0x02		//友情链接	1<<1
#define VLINK_INVALID	0x04		//不可见的链接	1<<2
#define VLINK_CONTROLED	0x08		//通过css控制视觉的链接	1<<3
#define VLINK_IMAGE	0x10		//图片链接	1<<4
#define VLINK_FRIEND_EX 0x20		//友情自助链接	1<<5
#define VLINK_SELFHELP	0x40		//自组链接	1<<6
#define VLINK_BLOG_MAIN	0x80		//博客正文	1<<7
#define VLINK_CSS	0x100		//css链接	1<<8
#define VLINK_QUOTATION	0x200		//引文		1<<9
#define VLINK_CONT_INTERLUDE 0x400	//内容穿插	1<<10
#define VLINK_BBSRE	0x800		//bbs回复	1<<11
#define VLINK_BLOGRE	0x1000		//blog回复	1<<12
#define VLINK_IMG_SRC   0x2000      //图片资源链接	1<<13
#define VLINK_EMBED_SRC 0x4000      //嵌入资源链接	1<<14
#define VLINK_FROM_CONT 0x8000		//!!!!!!!!!!!ec中定义和实现的链接类型，防止冲突，加入	1<<15
#define VLINK_BBSCONT	0x10000		//	1<<16
#define VLINK_BBSSIG	0X20000		//	1<<17
#define VLINK_COPYRIGHT	0x40000		//	1<<18
#define VLINK_NOFOLLOW	0x80000		//nofollow	1<<19
#define VLINK_MYPOS	0x100000		//mypos	1<<20
#define VLINK_HIDDEN	0x200000	//	1<<21
#define VLINK_TEXT_LINK	0x400000	//穿插的链接，伪装成文本形式	1<<22
#define VLINK_FRIEND_SPAM	0x1000000	//预留给超链系统	1<<24
#define VLINK_IN_LINK	0x2000000		//预留给超链系统	1<<25
#define VLINK_TEXT  0x4000000   // 文字链接
#define VLINK_NEWS_ANCHOR 0x8000000	//新闻页正文中介绍的网站链接(预留给EC)
//add
#define VLINK_IFRAME 0x10000000 //iframe标签的链接
//modify
#define VLINK_ALL	0x3fffffff		//所有
//#define VLINK_ALL	0xfffffff		//所有
/**
 * @brief short description 链接内部属性
 */
typedef struct _vlink_info_t
{
	int xpos; //横坐标
	int ypos; //纵坐标
	int width; //宽度
	int height; //高度
	html_tag_type_t tag_type; //tag类型
	html_node_t *node; //链接指向的结点
	html_vnode_t *vnode; //链接指向的v结点
	int is_goodlink :1; //是否有效链接
	int is_outlink :1; //是否外链
	int link_set; // only for VLINK_BLOG_MAIN mark
	html_area_t *html_area; //链接属于哪个块
	int route;
	int area_left_link_count; //该块中还有多少链接
	int anchor_from_alt; //链接的anchor是否出自图片的alt
	int text_len;
	int is_for_screen :1; /**< for css link : 是否作用于屏幕媒体 */
} vlink_info_t;

/**
 * @brief short description 链接属性
 */
typedef struct _vlink_t
{
	char url[UL_MAX_URL_LEN]; //链接的url
	char text[UL_MAX_TEXT_LEN]; //链接的anchor
	char realanchor[UL_MAX_TEXT_LEN];
	// vlink marks
	html_area_abspos_mark_t position; //链接所在的位置
	int linkFunc; //链接的链接类型
	char nofollow; //是否为nofollow链接
	int group;
	// inner
	vlink_info_t inner; //链接内部属性
	int tag_code;
} vlink_t;

/**
 * @brief 图片类型
 */
enum img_type_t
{
	IMG_IN_ANCHOR,			//锚文中的图片
	IMG_NEAR_ANCHOR, 	//锚文相关的图片
	IMG_IN_CONTENT,		//内容相关的图片
};

/**
 * @brief 图片信息
 */
struct img_info_t
{
	img_type_t type;	//图片的类型
	char img_url[UL_MAX_URL_LEN]; //图片的url
	int img_wx;	//图片的宽度，没有设置时为0
	int img_hx;	//图片的高度，没有设置时为0
	int trust;		//图片的宽高可信度，0不可信，10最可信
	vlink_t *owner;	//图片对应的vlink，若为NULL，表示和当前解析页面的url对应
};

/**
 * @brief short description 网页基准url信息
 */
typedef struct _base_info_t
{
	char base_domain[UL_MAX_SITE_LEN]; //主域
	char base_port[UL_MAX_PORT_LEN]; //端口
	char base_path[UL_MAX_PATH_LEN]; //路径
} base_info_t;

/**
 * description : 
 * 	extract all of vlink in this area, every vlink marked postion and function
 * input :
 * 	root, the root vnode of htm vtree;
 * 	html_area, the area struct of html area;
 * 	base_url, the url of this web page;
 * 	base_info, the base url info, may be diff with base_url;
 * 	vlink : the array to store vlink;
 * 	num : the max num of vlink limit;
 * 	flag : merge or nomerge for anchor text
 * output :
 * 	vlink;
 * 	extra_info:
 * 		1.nofollow, if <meta content="nofollow"> defined  0x00000001
 * 		2.is_selfhelp_page, 0x00000002
 * return :
 * 	success : the number of vlinks
 * 	failed : -1
 * 	
 * Pre-required:
 * 	html_area : this area HAS marked position
 */
int html_area_extract_vlink(html_vtree_t *vtree, html_area_t *area, char *base_url, base_info_t *base_info,
		vlink_t *vlink, int num, char flag, char &extra_info);

/**
 * @brief 从html_vtree中解析链接，并且标记链接类型, 链接anchor合并空格
 * @param [in] vtree   : html_vtree_t*	可视树
 * @param [in] atree   : area_tree_t*	分块树
 * @param [in] base_url   : char*		网页的url
 * @param [in/out] vlink   : vlink_t*		用于存储解析出的link的buf
 * @param [in] maxnum   : int		最多解析的link数量
 * @return  int 
 * @retval   >=0 解析出来的link数量
 **/
int html_vtree_extract_vlink(html_vtree_t *vtree, area_tree_t *atree, char *base_url, vlink_t *vlink, int maxnum);

/**
 * @brief 从CSS中提取链接。
 * @param [in] css_text   : const char*	CSS的文本.
 * @param [in] base_url   : const char*	页面url,或用于拼接的base url.
 * @param [out] vlink   : vlink_t*	用于存储解析出的link的数组.
 * @param [in] maxnum   : int	最多容纳的LINK数量.
 * @param [bool] is_mark   : int	是否进行链接属性标注，默认为是.
 * @return  int -1:error; >=0:提取出的链接数量.
 **/
int css_extract_vlink(const char *css_text, const char *base_url, vlink_t *vlink, int maxnum, bool is_mark = true);

/**
 * @brief 从一段html文字中提取出文字链接
 *
 * @param [in] text   : const char* html文字
 * @param [in] text_len   : int html文字长度
 * @param [in/out] vlink   : vlink_t* 保存链接的数组
 * @param [in] maxnum   : int 最大能保存的链接数
 * @return  int 
 * @retval  >=0 成功提取的文字链接数；-1 错误
 **/
int text_extract_vlink(const char* text, int text_len, vlink_t *vlink, int maxnum);

/**
 * @brief 从html_vtree_t中解析链接，并且标记链接类型, 链接anchor不合并空格
 *
 * @param [in] vtree   : html_vtree_t*	网页的分块信息
 * @param [in] atree   : area_tree_t		网页的分块数量
 * @param [in] base_url   : char*		网页的url
 * @param [in/out] vlink   : vlink_t*		用于存储解析出的link的buf
 * @param [in] maxnum   : int		最多解析的link数量
 * @return  int 
 * @retval   >=0 解析出来的link数量
 **/
int html_vtree_extract_vlink_nomerge(html_vtree_t *vtree, area_tree_t *atree, char *base_url, vlink_t *vlink,
		int maxnum);

/**
 * @brief 标记链接类型
 * @param [in] vtree   : html_vtree_t* 	可视树
 * @param [in] atree   : area_tree_t*	分块树
 * @param [in] base_url   : char*	网页url
 * @param [in/out] vlink   : vlink_t*	提取出来的链接
 * @param [in] link_count   : int	链接个数
 * @param [in] pagetype   : unsigned int	页面类型
 * @param [in] marktype_flag   : unsigned int	需要标记的链接类型
 * @param [in] pvres   : vhp_res_t*	标记类型所需的buffer
 * @return  void 
 **/
void html_vtree_mark_vlink(html_vtree_t *vtree, area_tree_t *atree, char *base_url, vlink_t *vlink, int link_count,
		unsigned int pagetype, unsigned int marktype_flag, lt_res_t *pvres);

/**
 * @brief 提取图片信息
 * @param [in] url, 当前解析页面的url
 * @param [in] url_len, url的长度
 * @param [in] atree, 解析好的分块树
 * @param [in] vlinks, 已经抽取好的vlink
 * @param [in] num, 抽取到的vlink的个数
 * @param [in/out] imgs, 保存图片信息
 * @param [in] size, imgs的容量
 * @return 实际抽取到的图片信息个数
 * @author sue
 * @date 2013/04/14
 */
int extract_img_infos(const char *url, int url_len, area_tree_t* atree, vlink_t *vlinks, int num, img_info_t *imgs, int size);

/**
 * @brief 初始化标链所需的资源
 *
 * @param [in] marktype_flag   : int	需要标记的链接类型开关
 * @return  vhp_res_t* 
 * @retval   	NOT NULL	链接资源
 * @retval	NULL		初始化资源失败
 **/
lt_res_t * linktype_res_create(unsigned int marktype_flag);

/**
 * @brief 删除资源
 *
 * @return  void 
 * @retval   
 **/
void linktype_res_del(lt_res_t *);

/**
 * @brief 重置资源
 */
void linktype_res_reset(lt_res_t *pvres);

/**
 * @brief 释放正规表达式结构体
 * @return  void 
 * @retval   
 **/
void fin_url_pcre();

#endif

/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_link_common.h,v 1.2 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_link_common.h
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 	链接标注时使用的公共函数
 **/

#ifndef  __EASOU_LINK_COMMON_H_
#define  __EASOU_LINK_COMMON_H_

#include "easou_link.h"
#include "easou_mark_markinfo.h"

/**
 * @brief short description 块和链接对应的结构
 */
typedef struct _html_area_link_t
{
	html_area_t *html_area; //当前块
	vlink_t *vlink; //块中的链接集合
	int link_count; //块中的链接个数

	vlink_t * next_vlink; //辅助信息，用于查找下一个块的链接, 指向下一个块的开始位置
	int next_count; //辅助信息
} html_area_link_t;

// Input : url, Output : trunk
// Return : 1, success; 0, otherwise
int get_trunk_from_url(const char *url, char *trunk);

/**
 * @brief 获得一个块中所有链接的信息
 * @param [in] area   : html_area_t*	要获取链接的块
 * @param [in] vlink   : vlink_t*	网页中所有链接集合
 * @param [in] link_count   : int	网页中所有链接个数
 * @param [in/out] parea_link   : html_area_link_t*	保存块中链接信息
 * @return  void 
 **/
void get_area_link(html_area_t *area, vlink_t *vlink, int link_count, html_area_link_t *parea_link);

// Function : check url to see if in same trunk
// Return : 1, OK; 0, Not same trunk
int check_url_trunk(char *url, char *base_trunk);

int get_valid_text_len(html_vnode_t *pnode, int strict = 0);

/**
 * @brief 判断url是否含有协议头
 * @param [in] url   : char*	待检查的url
 * @return  int 
 * @retval   	0	不带协议头
 * @retval	1	带协议头
 **/
int is_url_has_protocol_head(const char *url);

/**
 * @brief 获得url协议头的长度， 带//的， 比如 http://
 * @param [in/out] url   : const char*	网页url
 * @return  int 
 * @retval   	>0	长度
 * @retval	0	不带协议头，长度默认为0
 **/
int url_protocol_head_len(const char *url);

/**
 * @brief 获得结点的tagcode
 */
int get_vnode_tag_code(html_vnode_t *vnode);

/**
 * @brief 获得子树最大的tag code
 */
int get_sub_tree_max_tag_code(html_vnode_t *start_vnode);

/**
 * @brief 是否主页
 */
bool is_homepage(const char *str);

/**
 * @brief 链接是否在area中
 */
int in_area(vlink_info_t *vlink, const area_list_t * plist);

/**
 * @brief 链接是否在area中
 */
int in_one_area(vlink_info_t *vlink, const html_area_t *parea);

/**
 * @brief 获得文字的中文个数
 */
int get_chword_len(const char *text);

/**
 * @brief 打印分块
 */
void print_area(FILE *fp, html_area_t *area);

/**
 * @brief 获得子树对应的tag code与文字长度的映射表
 */
void get_tag_code_text_len(html_vnode_t *root, int *tag_code_len, int size);

/**
 * @brief 获得子树最小的tag_code
 */
int get_sub_tree_min_tag_code(html_vnode_t *start_vnode);

#endif

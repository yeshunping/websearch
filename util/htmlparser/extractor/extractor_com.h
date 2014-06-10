/*
 * easou_extractor_com.h
 *
 *  Created on: 2012-1-11
 *      Author: xunwu
 */

#ifndef EASOU_EXTRACTOR_COM_H_
#define EASOU_EXTRACTOR_COM_H_

#include "easou_mark_srctype.h"
#include "easou_mark_func.h"
#include "easou_mark_sem.h"
#include "easou_mark_parser.h"

/**
 * @brief anchor的抽象
 **/
typedef struct _anchor_info
{
	char url[UL_MAX_URL_LEN];
	char text[UL_MAX_TEXT_LEN];
	unsigned long linktype;
} anchor_info;

#define IS_LINK_COPYRIGHT(flag) ((flag) & 0x01)
#define SET_LINK_COPYRIGHT(flag) ((flag)|= 0x01)
#define IS_LINK_NAV(flag) ((flag) & 0x02)
#define SET_LINK_NAV(flag) ((flag)|= 0x02)
#define IS_LINK_FRIEND(flag) ((flag) & 0x04)
#define SET_LINK_FRIEND(flag) ((flag)|= 0x04)
#define IS_LINK_RELATE_LINK(flag) ((flag) & 0x08)
#define SET_LINK_RELATE_LINK(flag) ((flag)|= 0x08)
#define IS_LINK_MYPOS(flag) ((flag) & 0x10)
#define SET_LINK_MYPOS(flag) ((flag)|= 0x10)
#define IS_LINK_ARTICLE_SIGN(flag) ((flag) & 0x20)
#define SET_LINK_ARTICLE_SIGN(flag) ((flag)|= 0x20)

/**
 * @brief 获取某资源类型分块的内容.
 **/
char *get_area_content(char *buf, int size, area_tree_t *atree, html_area_srctype_t func);

/**
 * @brief 获取某功能类型的分块的内容.
 **/
char *get_area_content(char *buf, int size, area_tree_t *atree, html_area_func_t func);

/**
 * @brief 获取某语义类型的分块的内容.
 **/
char *get_area_content(char *buf, int size, area_tree_t *atree, html_area_sem_t func);

/**
 * @brief 添加断点.
 **/
int addBreakInfo(char *buffer, int available, int end, const char *break_info);

/**
 * @brief 销毁所有树
 */
int destroyTree(html_tree_t *&html_tree, vtree_in_t *&vtree_in, html_vtree_t *&vtree, area_tree_t * &atree);

/**
 * @brief 创建所有树
 */
int createTree(html_tree_t *&html_tree, vtree_in_t *&vtree_in, html_vtree_t *&vtree, area_tree_t * &atree);

/**
 * @brief 复位所有树
 */
int resetTree(html_tree_t *&html_tree, vtree_in_t *&vtree_in, html_vtree_t *&vtree, area_tree_t * &atree);

/**
 * @brief 获取某语义类型的所有分块的内容.
 **/
char *get_all_area_content(char *buf, int size, area_tree_t *atree, html_area_sem_t sem);

/**
 * @brief 获取某功能类型的所有分块的内容.
 **/
char *get_all_area_content(char *buf, int size, area_tree_t *atree, html_area_func_t func);

/**
 * @brief 获取某资源类型所有分块的内容.
 **/
char *get_all_area_content(char *buf, int size, area_tree_t *atree, html_area_srctype_t srctype);

int getAnchorInfos(area_tree_t *atree, anchor_info * anchors, int anchorsize, const char *baseUrl);

#endif /* EASOU_EXTRACTOR_COM_H_ */


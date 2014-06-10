/***************************************************************************
 * 
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_link_tree.h,v 1.0 2012/09/01 pageparse Exp $
 * 
 **************************************************************************/

/**
 * @file easou_link_tree.h
 * @author (pageparse@staff.easou.com)
 * @date 2012/09/01
 * @version $Revision: 1.0 $
 * @brief 处理树的一些函数
 **/

#ifndef _EASOU_LINK_TREE_H
#define _EASOU_LINK_TREE_H

#include "easou_vhtml_parser.h"
#include "easou_ahtml_area.h"

/**
 * @brief 获取结点
 */
static inline html_tag_t *get_tag_from_vnode(html_vnode_t *vnode)
{
	return &vnode->hpNode->html_tag;
}

/**
 * @brief 获取结点类型
 */
static inline html_tag_type_t get_tag_type_from_vnode(html_vnode_t *vnode)
{
	html_tag_t *ptag = get_tag_from_vnode(vnode);
	return ptag->tag_type;
}

/**
 * @brief 获取结点编号
 */
static inline int get_tag_code_from_vnode(html_vnode_t *vnode)
{
	html_tag_t *ptag = get_tag_from_vnode(vnode);
	return ptag->tag_code;
}

#endif

/*
 * easou_mark_inner.cpp
 *
 *  Created on: 2011-11-20
 *      Author: xunwu_chen@staff.easou.com
 */
#include "easou_mark_switch.h"
#include "easou_mark_conf.h"
#include "easou_mark_inner.h"

/**
 * @brief
 */
const unsigned int mark_type_to_mask[] = {
	0x0, 0x1,
	0x1<<1,0x1<<2,0x1<<3,0x1<<4,0x1<<5,0x1<<6,0x1<<7,0x1<<8,
	0x1<<9,0x1<<10,0x1<<11,0x1<<12,0x1<<13,0x1<<14,0x1<<15,0x1<<16,
	0x1<<17,0x1<<18,0x1<<19,0x1<<20,0x1<<21,0x1<<22,0x1<<23,0x1<<24,
	0x1<<25,0x1<<26,0x1<<27,0x1<<28,0x1<<29,0x1<<30,0x1<<31
};

/**
 * @brief 获取变量的类型
 */
enum
{
	TYPE_INT, TYPE_AREA_SRCTYPE, TYPE_AREA_FUNC, TYPE_AREA_SEM
};

static const int NULL_ENUM_TYPE = -12321; /**< 空的枚举类型 */

inline int get_enum_type(const int)
{
	return TYPE_INT;
}

inline int get_undef_enum_type(const int)
{
	return NULL_ENUM_TYPE;
}

inline int get_last_enum_type(const int)
{
	return NULL_ENUM_TYPE;
}

inline int get_enum_type(const html_area_srctype_t)
{
	return TYPE_AREA_SRCTYPE;
}

inline html_area_srctype_t get_undef_enum_type(const html_area_srctype_t)
{
	return AREA_SRCTYPE_UNDEFINED;
}

inline html_area_srctype_t get_last_enum_type(const html_area_srctype_t)
{
	return AREA_SRCTYPE_LASTFLAG;
}

inline int get_enum_type(const html_area_func_t)
{
	return TYPE_AREA_FUNC;
}

inline html_area_func_t get_undef_enum_type(const html_area_func_t)
{
	return AREA_FUNC_UNDEFINED;
}

inline html_area_func_t get_last_enum_type(const html_area_func_t)
{
	return AREA_FUNC_LASTFLAG;
}

inline int get_enum_type(const html_area_sem_t)
{
	return TYPE_AREA_SEM;
}

inline html_area_sem_t get_undef_enum_type(const html_area_sem_t)
{
	return AREA_SEM_UNDEFINED;
}

inline html_area_sem_t get_last_enum_type(const html_area_sem_t)
{
	return AREA_SEM_LASTFLAG;
}

static bool is_selected_type_for_int(function_switch_t *function_switch, int type)
{
	switch (type)
	{
	case MARK_TYPE_POS:
		return function_switch->select_pos;
	case MARK_TYPE_BASE_INFO:
		return function_switch->select_base_info;
	default:
		assert(0);
		break;
	}
	assert(0);
	return false;
}

template<typename AREA_TYPE_T>
static bool is_selected_type(function_switch_t *function_switch, AREA_TYPE_T t)
{
	if (!function_switch->is_set_select)
	{
		/** if select-switch is not seted, then all types are regarded as selected. */
		return true;
	}
	switch (get_enum_type(t))
	{
	case TYPE_INT:
		return is_selected_type_for_int(function_switch, t);
	case TYPE_AREA_SRCTYPE:
		return function_switch->select_srctype[t];
	case TYPE_AREA_FUNC:
		return function_switch->select_func[t];
	case TYPE_AREA_SEM:
		return function_switch->select_sem[t];
	default:
		break;
	}
	assert(0);
}

/**
 * @brief 是否选择了某种类型的标注.
 **/
template<typename AREA_TYPE_T>
bool is_selected_type(area_tree_t *atree, AREA_TYPE_T t)
{
	return is_selected_type(atree->function_switch, t);
}

template bool is_selected_type<int>(area_tree_t *, int);
template bool is_selected_type<html_area_srctype_t>(area_tree_t *, html_area_srctype_t);
template bool is_selected_type<html_area_func_t>(area_tree_t *, html_area_func_t);
template bool is_selected_type<html_area_sem_t>(area_tree_t *, html_area_sem_t);

static bool is_shutted_type_for_int(function_switch_t *function_switch, int type)
{
	switch (type)
	{
	case MARK_TYPE_POS:
		return function_switch->shutted_pos;
	case MARK_TYPE_BASE_INFO:
		return function_switch->shutted_base_info;
	default:
		assert(0);
		break;
	}
	assert(0);
	return true;
}

template<typename AREA_TYPE_T>
static bool is_shutted_type(function_switch_t *function_switch, AREA_TYPE_T t)
{
	/**
	 * 显式关闭的类型都认为关闭.
	 * 如果选择了特定的类型,则没有选择的类型也认为关闭.
	 */
	switch (get_enum_type(t))
	{
	case TYPE_INT:
		return is_shutted_type_for_int(function_switch, t);
	case TYPE_AREA_SRCTYPE:
		if (function_switch->is_set_select)
		{
			return (!function_switch->select_srctype[t] || function_switch->shutted_srctype[t]);
		}
		else
		{
			return function_switch->shutted_srctype[t];
		}
	case TYPE_AREA_FUNC:
		if (function_switch->is_set_select)
		{
			return (!function_switch->select_func[t] || function_switch->shutted_func[t]);
		}
		else
		{
			return function_switch->shutted_func[t];
		}
	case TYPE_AREA_SEM:
		if (function_switch->is_set_select)
		{
			return (!function_switch->select_sem[t] || function_switch->shutted_sem[t]);
		}
		else
		{
			return function_switch->shutted_sem[t];
		}
	default:
		break;
	}
	assert(0);
}

/**
 * @brief 是否关闭了某种类型的标注.
 * @author xunwu
 * @date 2011/07/12
 **/
template<typename AREA_TYPE_T>
bool is_shutted_type(area_tree_t *atree, AREA_TYPE_T t)
{
	return is_shutted_type(atree->function_switch, t);
}

template bool is_shutted_type<int>(area_tree_t *, int);
template bool is_shutted_type<html_area_srctype_t>(area_tree_t *, html_area_srctype_t);
template bool is_shutted_type<html_area_func_t>(area_tree_t *, html_area_func_t);
template bool is_shutted_type<html_area_sem_t>(area_tree_t *, html_area_sem_t);

/**
 * @brief 获取先序深度优先遍历的下一个节点.
 * @param [in] visit_cmd   : int 可传入AREA_VISIT_NORMAL:正常遍历; AREA_VISIT_SKIP:忽略当前节点的子孙.
 * @return  html_area_t* 先序优先遍历的下一个节点.
 **/
html_area_t *html_area_iter_next(html_area_t *area, int visit_cmd)
{
	if (area == NULL)
	{
		return NULL;
	}
	if (area->subArea && visit_cmd != AREA_VISIT_SKIP)
	{
		return area->subArea;
	}
	do
	{
		if (area->nextArea)
		{
			return area->nextArea;
		}
		area = area->parentArea; // stack pop
	} while (area);
	return NULL;
}

void insert_area_list(area_list_t * area_list, area_list_node_t * area_node)
{
	assert(area_list !=NULL && area_node !=NULL );
	if (area_list->head == NULL)
	{
		assert(area_list->tail == NULL );
		area_list->head = area_node;
		area_list->tail = area_node;
	}
	else
	{
		area_list->tail->next = area_node;
		area_list->tail = area_node;
	}
	area_node->next = NULL;
	area_list->num++;
}

int insert_area_on_list(area_list_t area_list_arr[], int index, html_area_t *area, nodepool_t *area_pool)
{
	area_list_node_t * area_node = (area_list_node_t *) nodepool_get(area_pool);
	if (area_node == NULL)
	{
		Warn("nodepool_get fail at easou_mark_inner.cpp::insert_area_on_list");
		return -1;
	}
	area_node->area = area;
	area_node->next = NULL;
	insert_area_list(&area_list_arr[index], area_node);
	return 1;
}

/**
 * @brief 获取下一个可视块
 **/
html_area_t *next_valid_area(html_area_t *area)
{
	for (html_area_t *next_area = area->nextArea; next_area; next_area = next_area->nextArea)
	{
		if (next_area->isValid)
		{
			return next_area;
		}
	}
	return NULL;
}


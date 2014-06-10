/*
 * easou_mark_conf.h
 *
 *  Created on: 2011-11-22
 *      Author: ddt
 */

#ifndef EASOU_MARK_CONF_H_
#define EASOU_MARK_CONF_H_

#include "easou_ahtml_tree.h"
#include "easou_mark_switch.h"

#define LEVEL_POS     0
#define LEVEL_SRCTYPE 1
#define LEVEL_FUNC    2
#define LEVEL_SEM     3

/**
* @brief 标注函数定义.
*/
typedef bool (* SRCTYPE_MARK_T)(area_tree_t *) ;

/**
* @brief 资源类型的配置结构.
*/
typedef struct _mark_srctype_handler_t {
	SRCTYPE_MARK_T type_handler;		  /**< 对应的标注函数       */
	html_area_srctype_t srctype;		  /**< 标注的功能类型       */
	const char *name;		  			  /**< 语义类型名字       */
}mark_srctype_handler_t;

extern const int N_SRCTYPE;
extern const mark_srctype_handler_t srctype_mark_handler[];


/**
* @brief 功能标注函数定义.
*/
typedef bool (* FUNC_MARK_T)(area_tree_t *) ;
/**
* @brief 功能类型的配置结构.
* XXX:自动依赖关系生成(dependence_looker.sh)依赖此结构体，修改请注意.
*/
typedef struct _mark_func_handler_t{
	FUNC_MARK_T type_handler;		  /**< 对应的标注函数       */
	html_area_func_t func_type;		  /**< 标注的功能类型       */
	const char *name;		  /**< 标注的名字       */
}mark_func_handler_t;

extern const int N_FUNC;
extern const mark_func_handler_t func_mark_handler[];


/**
* @brief 语义标注函数定义.
*/
typedef bool (* SEM_MARK_T)(area_tree_t *) ;

/**
* @brief 语义标注的配置结构.
* XXX:自动依赖关系生成(dependence_looker.sh)依赖此结构体，修改请注意.
*/
typedef struct _mark_sem_handler_t{
	SEM_MARK_T 	type_handler;		  /**< 对应的标注函数       */
	html_area_sem_t	type;		  /**<  要标注的语义类型      */
	const char *name;		  /**< 语义类型名字       */
}mark_sem_handler_t;

extern const int N_SEM;
extern const mark_sem_handler_t sem_mark_handler[];

#endif /* EASOU_MARK_CONF_H_ */

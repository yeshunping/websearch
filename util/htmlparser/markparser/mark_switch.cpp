/*
 * easou_ahtml_switch.cpp
 *
 *  Created on: 2011-11-18
 *      Author: ddt
 */
#include <stdlib.h>
#include <stdio.h>
#include "log.h"
#include "easou_mark_switch.h"

using namespace EA_COMMON;


function_switch_t g_function_switch = {
	is_set_select:0,
	shutted_pos : 0,
	select_pos : 1,
	shutted_base_info : 0,
	select_base_info : 1,
	shutted_srctype : {0},
	select_srctype : {1,1,1,1,1,1,1},
	shutted_func : {0},
	select_func : {1,1,1,1,1,1,1,1,1,1},
	shutted_sem : {0},
	select_sem : {1,1}

//	is_set_select:0,
//	shutted_pos : 0,
//	select_pos : 1,
//	shutted_base_info : 0,
//	select_base_info : 1,
//	shutted_srctype : {0},
//	select_srctype : {1},
//	shutted_func : {0},
//	select_func : {1},
//	shutted_sem : {0},
//	select_sem : {1}
};

/**
 * @brief	初始化功能开关
**/
function_switch_t *function_switch_create()
{
	function_switch_t *fs = (function_switch_t *)calloc(1, sizeof(function_switch_t));
	if(fs == NULL){
		Fatal((char*)"alloc error!");
		goto ERR;
	}
	*fs = g_function_switch;
//	fs->is_set_select = 0;
//	fs->select_pos = 0;
//	fs->select_pos = 1;
//	fs->shutted_base_info = 0;
//	fs->select_base_info = 1;
//	fs->shutted_srctype[0] = 0;
//	fs->select_srctype[0] = 0;
//	fs->shutted_func[0] = 0;
//	fs->shutted_sem[0] = 0;
//	fs->select_sem[0] = 0;
	return fs;
ERR:
	return NULL;
}

/**
 * @brief	销毁功能开关
**/
void function_switch_destroy(function_switch_t *fs)
{
	free(fs);
}

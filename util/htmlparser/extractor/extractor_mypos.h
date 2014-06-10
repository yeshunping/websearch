/*
 * easou_extractor_mypos.h
 *
 *  Created on: 2012-1-11
 *      Author: xunwu
 */

#ifndef EASOU_EXTRACTOR_MYPOS_H_
#define EASOU_EXTRACTOR_MYPOS_H_
#include "easou_mark_parser.h"

#define MYPOS_BUF_LEN		256
#define MYPOS_MAX_ITEM_NUM	16

typedef struct _pos_item_t {
	char url[MAX_URL_LEN];
	char text[UL_MAX_TEXT_LEN];
}pos_item_t;

typedef struct _mypos_t {
	pos_item_t items[MYPOS_MAX_ITEM_NUM];
	char text[MYPOS_BUF_LEN];
	int len;									//text的长度
	int item_num;								//item数
	int output_type;							//输出类型：文本或(文本、链接)对
} mypos_t;

/**
 * @brief	从分块树中抽取MYPOS。 分块树已完成mark
**/
int html_atree_extract_mypos(mypos_t *mypos, const area_tree_t *area_tree, const char *page_url,int is_merge_text=1);

#endif /* EASOU_EXTRACTOR_MYPOS_H_ */

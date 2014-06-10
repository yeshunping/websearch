/**
 * easou_extractor_title.h
 *
 *  Created on: 2012-1-11
 *      Author: xunwu
 */

#ifndef EASOU_EXTRACTOR_TITLE_H_
#define EASOU_EXTRACTOR_TITLE_H_

#include "easou_html_tree.h"
#include "easou_ahtml_tree.h"

/**
 * @brief 提取网页的tagtitle，返回提取到的长度
 */
int html_tree_extract_tagtitle(html_tree_t *html_tree, char* title, int size);

/**
 * @brief 提取网页的realtitle，要求网页已经完成mark，返回提取到的长度
 */
int html_atree_extract_realtitle(area_tree_t *area_tree, char *realtitle, int size);

/**
 * @brief 提取网页的subtitle，要求网页已经完成mark，返回提取到的长度
 */
int html_atree_extract_subtitle(area_tree_t *area_tree, char *subtitle, int size);

/**
 * @brief easou_extract_realtitle方法的输入
 * @author sue
 * @date 2013-07-22
 */
struct realtitle_input_t
{
	area_tree_t* atree;	//已经标记好的分块树
	const char* url;	//网页的url
	int url_len;		//url的长度
	const char* tagtitle;	//网页的tagtitle
	int tagtitle_len;	//tagtitle的长度
};

/**
 * @brief 提取网页的realtitle，要求网页已经完成mark。同html_atree_extract_realtitle方法相比，该方法
 * 	会采用一些策略对特定需求进行优化，不完全依赖标题块的标记。
 * @param input [in], 抽取需要用到的信息
 * @param realtitle [in/out], 保存提取到的标题
 * @param size [in], realtitle的大小
 * @return 成功时返回提取到的长度(>=0)。<0表示失败。
 * @author sue
 * @date 2013-07-22
 */
int easou_extract_realtitle(realtitle_input_t* input, char* realtitle, int size);

#endif /* EASOU_EXTRACTOR_TITLE_H_ */

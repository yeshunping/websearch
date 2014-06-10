/*
 * easou_extractor_content.h
 *
 *  Created on: 2012-1-11
 *      Author: xunwu
 */

#ifndef EASOU_EXTRACTOR_CONTENT_H_
#define EASOU_EXTRACTOR_CONTENT_H_

#include "easou_mark_parser.h"
#include "easou_html_extractor.h"
#include "easou_extractor_mypos.h"

/**
 * @brief 抽取一个分块的content
 * @param area, [in], 分块
 * @param content, [in/out], 保存抽取到的内容
 * @param size, [in], content的大小
 * @return int, 实际抽取到的长度
 * @version 1.1(2012-10-09)
 **/
int html_area_extract_content(html_area_t *area, char *content, int size);

/**
 * @brief 抽取网页的content，注重召回率（抽取网页除版权块以外的所有内容）
 * @param area_tree, [in], 解析好的分块树
 * @param content, [in/out], 保存抽取到的正文
 * @param size, [in], content的大小
 * @param sizeignore_unavail, [in],
 * @return int, 实际抽取到的长度
 * @version 1.1(2012-10-09)
 **/
int html_atree_extract_content(area_tree_t *area_tree, char *content, int size, bool sizeignore_unavail);

/**
 * @brief 抽取网页的content，注重准确率
 * @param area_tree, [in], 解析好的分块树
 * @param content, [in/out], 保存抽取到的正文
 * @param size, [in], content的大小
 * @param url, [in], 网页的url
 * @return int, 实际抽取到的长度
 * @version 1.1(2012-10-09)
 **/
int html_atree_extract_main_content(area_tree_t *area_tree, char *content, int size, const char *url = NULL);

/**
 * @brief 抽取文章内容，注重准确率，主要对新闻和内容页有效
 * @param area_tree, [in], 解析好的分块树
 * @param content, [in/out], 保存抽取到的内容
 * @param size, [in], content的大小
 * @return int, 实际抽取到的长度
 * @author sue
 * @version 1.0(2012-10-29)
 **/
int html_atree_extract_article_content(area_tree_t *area_tree, char *content, int size);

void printAtreeTitle(area_tree_t * atree);
void printAtreeWithTitle(area_tree_t * atree, char *buf, int bufsize);
//int isHubType(const char *pcUrl, area_tree_t * atree, const link_t * pLink, int iLinkNum, const char *pcContent, int iContentLen,
//		const mypos_t *pMypos, const char *pcTitle, int iTitlenLen);

#endif /* EASOU_EXTRACTOR_CONTENT_H_ */

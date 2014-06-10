/**
 *  easou_extractor_stdata.h
 *  结构化抽取
 *
 *  Created on: 2013-7-19
 *      Author: round
 */

#ifndef EASOU_EXTRACTOR_STDATA_H_
#define EASOU_EXTRACTOR_STDATA_H_

#include "StructData_constants.h"
#include "StructData_types.h"

#include "easou_html_dom.h"
#include "easou_vhtml_basic.h"
#include "easou_ahtml_tree.h"

/**
 * @brief 抽取结构化数据
 * @param url [in], 页面url
 * @param url_len [in], url的长度
 * @param page_type [in], 页面类型
 * @param html_tree [in], 已经解析好的dom树
 * @param stdata [out], 存放抽取后的数据
 * @return 0 表示成功，其它表示失败
 * @author round
 * @date 2013-07-20      
 **/
int html_tree_extract_stdata(const char *url, int url_len, unsigned int pagetype, html_tree_t *html_tree, StructureData *stdata);



/**
 * @brief 抽取小说类结构化数据
 * @param url [in], 页面url
 * @param url_len [in], url的长度
 * @param page_type [in], 页面类型
 * @param html_tree [in], 已经解析好的dom树
 * @param html_vtree [in], 已经解析好的vtree树
 * @param html_vtree [in], 已经解析好的atree树
 * @param stdata [out], 存放抽取后的数据
 * @return 0 表示成功，其它表示失败
 * @author round
 * @date 2013-08-08      
 **/
int html_tree_extract_download_stdata(const char *url, int url_len, unsigned int pagetype, html_tree_t *html_tree, html_vtree_t *html_vtree, area_tree_t *atree, StructureData *stdata);


int html_tree_extract_music_stdata(const char *url, int url_len, unsigned int pagetype, html_tree_t *html_tree, html_vtree_t *html_vtree, area_tree_t *atree, StructureData *stdata);

/**
 * @brief 序列化结构化数据
 * @param stdata [in], 已经解析好的结构化数据
 * @param buf_ptr [in/out], 用于保存序列化后数据的缓存
 * @param sz [in], buf_ptr的大小
 * @return -1 表示失败，其它表示序列化后的长度
 * @author round
 * @date 2013-07-20      
 **/
int html_tree_extract_serial(StructureData *stdata, uint8_t *buf_ptr, uint32_t size);


/**
 * @brief 反序列化结构化数据
 * @param stdata [out], 存放解析后的结构化数据
 * @param buf_ptr [in], 序列化数据的缓存
 * @param sz [in], buf_ptr的大小
 * @return -1 表示失败，0表示反序列化成功.
 * @author round
 * @date 2013-07-20      
 **/
int html_tree_extract_deserial(StructureData *stdata, uint8_t *buf_ptr, uint32_t sz);


#endif /* EASOU_EXTRACTOR_STDATA_H_ */

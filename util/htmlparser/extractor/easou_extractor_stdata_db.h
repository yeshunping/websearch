/**
 *  easou_extractor_stdata_db.h
 *  结构化抽取-豆瓣类-KS
 *
 *  Created on: 2013-10-08
 *      Author: round
 */

#ifndef EASOU_EXTRACTOR_STDATA_DB_H_
#define EASOU_EXTRACTOR_STDATA_DB_H_

#include "StructData_constants.h"
#include "StructData_types.h"

#include "easou_html_dom.h"
#include "easou_vhtml_basic.h"
#include "easou_ahtml_tree.h"




/**
 * @brief 抽取豆瓣类-KS字段结构化数据
 * @param url [in], 页面url
 * @param url_len [in], url的长度
 * @param page_type [in], 页面类型
 * @param html_tree [in], 已经解析好的dom树
 * @param html_vtree [in], 已经解析好的vtree树
 * @param html_vtree [in], 已经解析好的atree树
 * @param stdata [out], 存放抽取后的数据
 * @return 0 表示成功，其它表示失败
 * @author round
 * @date 2013-10-08      
 **/

int html_tree_extract_db_stdata(const char *url, int url_len, unsigned int pagetype, html_tree_t *html_tree, html_vtree_t *html_vtree, area_tree_t *atree, StructureData *stdata);


#endif /* EASOU_EXTRACTOR_STDATA_DB_H_ */

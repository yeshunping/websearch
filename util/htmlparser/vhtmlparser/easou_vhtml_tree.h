/**
 * easou_vhtml_tree.h
 * Description: V树解析
 *  Created on: 2011-11-13
 * Last modify: 2012-10-26 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#ifndef EASOU_VHTML_TREE_H_
#define EASOU_VHTML_TREE_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nodepool.h"
#include "easou_html_tree.h"
#include "easou_css_parser.h"
#include "easou_css_utils.h"
#include "easou_vhtml_basic.h"
#include "easou_vhtml_parser.h"
#include "easou_vstruct_profiler.h"

/**
 * @brief	创建并初始化Vtree输入结构.
 * @author xunwu
 * @date 2011/06/27
 **/
vtree_in_t *vtree_in_create(int max_css_page_size = DEFAULT_MAX_CSS_PAGE_SIZE, int css_num = DEFAULT_CSS_NUM_INPOOL);

/**
 * @brief
 * @author xunwu
 * @date 2011/07/12
 **/
void vtree_in_destroy(vtree_in_t *vtree_in);

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
html_vtree_t *html_vtree_create_with_tree(html_tree_t *html_tree);

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
void html_vtree_clean(html_vtree_t *html_vtree);

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
void html_vtree_del(html_vtree_t *html_vtree);

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
void html_vtree_clean_with_tree(html_vtree_t *html_vtree);

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
void html_vtree_del_with_tree(html_vtree_t *html_vtree);

/**
 * @brief 计算V树节点的位置和长宽，以及一些基本信息。会使用到解析好的css信息。
 * @author xunwu
 * @date 2011/06/27
 **/
int html_vtree_parse_with_css(html_vtree_t *html_vtree, easou_css_pool_t* css_pool, int page_width = DEFAULT_PAGE_WX);

/**
 * @brief 在DOM树的基础上解析出V树
 * @param [in/out] html_vtree, 保存解析出来的V树
 * @param [in/out] vtree_in, 
 * @param [in] page_width, 页面默认宽度
 * @param [in] test_import, 是否测试外部css文件中import的css文件
 * @param [in] useoutcss,true:使用来自css服务器的css；false：不使用来自css服务器的css
 * @author xunwu
 * @date 2011/06/27
 * @last modify on 2012-10-26 sue_zhang@staff.easou.com
 **/
int html_vtree_parse_with_tree(html_vtree_t *html_vtree, vtree_in_t *vtree_in, const char *url, int page_width = DEFAULT_PAGE_WX, bool test_import = false,bool useoutcss=true);

/**
 * @brief 初始化cssserver
 * @author shuangwei
 * @date 2012/08/01
 * @param
 * 	config_path, [in], edb配置文件路径
 * 	log_dir, [in], 日志输出目录
 * 	timeout, [in], 获取css超时时间，默认10ms
 * 	thread_num, [in], 获取css的线程个数，默认20
 * 	cache_size, [in], css缓存数量，默认5W
 * @return
 * 	true，初始化成功
 * 	false,初始化失败
 */
bool init_css_server(char *config_path, char *log_dir, int timeout = 10, int thread_num = 20, int cache_size = 35000);

/**
 * @brief 释放css server相关资源。用一个全局变量计数，每init一次加1，每free一次减1，当值为0时，才会去真正释放。
 * @author sue
 * @date 2013/12/16
 */
void free_css_server();

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
int html_vnode_visit(html_vnode_t *html_vnode, int (*start_visit)(html_vnode_t *, void *), int (*finish_visit)(html_vnode_t *, void *), void *result);

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
int html_vtree_visit(html_vtree_t *html_vtree, int (*start_visit)(html_vnode_t *, void *), int (*finish_visit)(html_vnode_t *, void *), void *result);

/**
 * 复位vtree_in_t结构
 * @param [in] vtree_in:vtree_in_t *
 */
void vtree_in_reset(vtree_in_t *vtree_in);

/**
 *复位vhtml树，恢复初始状态
 */
void html_vtree_reset(html_vtree_t *html_vtree);

#endif /* EASOU_VHTML_TREE_H_ */

/*
 * easou_mark_com.h
 *
 *  Created on: 2011-11-22
 *      Author: ddt
 */

#ifndef EASOU_MARK_COM_H_
#define EASOU_MARK_COM_H_
#include "easou_ahtml_tree.h"
#include "easou_mark_baseinfo.h"
/**
 * @brief 是否是一个表示时间的节点
**/
bool is_time_vnode(const html_vnode_t * vnode );

/**
 * @brief 块太小
**/
bool is_too_little_area(html_area_t * area ,mark_area_info_t * g_info , int level );

/**
 * @brief 块太大
**/
bool is_too_big_area(html_area_t * area ,mark_area_info_t * g_info , int level ) ;

/**
 * @brief 块的层次太高
**/
bool is_too_up_area(html_area_t * area ,mark_area_info_t * g_info , int level ) ;

/**
 * @brief 块的层次太低
**/
bool is_too_down_area(html_area_t * area ,mark_area_info_t * g_info , int level );

/**
 * @brief	分块的深度
**/
int get_area_child_depth(const html_area_t * area );

/**
 * @brief	提取一个分块的内容.  返回提取的内容长度.
**/
int extract_area_content(char *cont, const int size, const html_area_t *area);

/**
 * @brief 快速的copy_html_text. 若当前文本不含&，则忽略转义.
**/
int copy_html_text(char *buffer, int available, int end, char *src);

/**
 * @brief 计算分块内的文本占据了几行.
**/
int get_text_line_num(const html_area_t *area);

/**
 * @brief 块内的文本是否处于同一行.
**/
bool is_text_in_line(const html_area_t *area);

/**
 * @brief 是否是带有anchor的节点.
**/
bool is_anchor(html_vnode_t * vnode);

/**
 * @brief 获取链接的类型：站内、站外、js
**/
int get_link_type(html_vnode_t * vnode , const char * base_url );

/**
 * @brief 是否是图片链接， 控制递归层数为2层，计算的数据不需要太准确
**/
bool is_pic_link(html_vnode_t *vnode);

/**
 * @brief 计算anchor的长度，汉字算一个字符，连续的字母算一个字符，连续的数字算一个字符
**/
int get_real_anchor_size(html_vnode_t * vnode);

/**
 * @brief 得到anchor
**/
int get_anchor_str(html_vnode_t * vnode  , char *buff , int buff_size);

/**
 * @brief 活的节点的puretext 孩子，暂时只获得一个，认为achor都出现在一个节点下
**/
html_vnode_t * get_pure_text_child_a(html_vnode_t * vnode);

/**
 * @brief 是否在一列内
**/
bool is_a_like_one_col(const html_area_t * area );

/**
 * @brief 是否在一行内
**/
bool is_a_like_one_row(const html_area_t * area );

/**
 * @brief 获取当前块同一行左边的分块.
**/
const html_area_t *get_left_area_inline(const html_area_t *area);

/**
 * @brief 获取当前块同一行右边的分块.
**/
const html_area_t *get_right_area_inline(const html_area_t *area);

/**
 * @brief	获取左边的有效文本长度.
**/
int get_left_cont_size(html_area_t *area);


/**
 * @brief	获取左边叶子块的有效文本长度.
**/
int get_left_leaf_cont_size(html_area_t *area);
/**
 * @brief	获取右边的有效文本长度.
**/
int get_right_con_size(html_area_t *area);

/**
 * @brief	获取右边叶子块的有效文本长度.
**/
int get_right_leaf_con_size(html_area_t *area);
/**
 * @brief 获取当前分块的字体信息.
 * @param [out] font   : font_t*[]  字体数组.
 * @param [in] size   : unsigned int 字体数组大小.
 * @param [in] min_contributing_size   : int 贡献颜色的最小文本大小.
 * @param [out] font_num   : unsigned int& 颜色数量.
 * @return  bool  颜色数量是否超出数组大小.
**/
bool get_area_font(html_area_t *area,font_t *font[],unsigned int size, int min_contributing_size, unsigned int &font_num);


/**
 * @brief 判断当前分块的字体/颜色是否突出。在当前分块的字体数小于等于size的前提下，判断当前分块的字体是否突出。
 * 如果当前分块的字体数>size，直接返回false.
 * @param [in] area   : html_area_t* 判断是否突出的分块。
 * @param [in] root_font   : font* 根节点的字体.
 * @param [out] font*   : font_t[]	字体数组。
 * @param [in] size   : unsigned int	字体数组大小.
 * @param [out] font_num   : unsigned int	输出的不同字体数量.
 * @return  bool
 * @retval   突出返回TRUE,否则返回FALSE.
 * 	font数组中返回当前分块的前size个字体.
**/
bool check_area_font_outstand(html_area_t *area, font_t *root_font, font_t *font[], unsigned int size, unsigned int &font_num);

/**
 * @brief 获取下一个有内容的节点.
**/
html_area_t *get_next_content_area(html_area_t *area);

/**
 * @brief	根据块内节点的tag判断，是否文本节点，而非block节点.
**/
bool is_text_area(const html_area_t *area);

/**
 * @brief 获取第一个标记为MYPOS的块.
**/
html_area_t *get_first_mypos_area(area_tree_t *atree);

typedef struct _same_urlpat_t {
	int num;
	int area;
} same_urlpat_t;

/**
 * @brief 获取相同urlpattern的链接面积.
**/
void get_same_pat_url_info(same_urlpat_t *spat, html_area_t *area, const char *page_url, const char *page_url_pat);

/**
 * @brief 块是否单独处于一行. 左右没有东西.
**/
bool is_single_inline(html_area_t *area);

/**
 * @brief 是不是类似首页的url
**/
int is_like_top_url(const char *url);

bool is_area_has_spec_word_by_index(const html_area_t * area  , const char * word_list[]);

/**
 * @brief 一个简单的hash函数，也许会有冲突，不过够用了.
**/
int simple_hash(const char * src);

html_area_t * get_first_left_valid_area(const html_area_t * area );

int ef_trans2pt(const char *url,char *pattern);

////获取页面内base tag所指定的URL
//int get_base_url(char *base_url, html_tree_t *html_tree);
/**
 * @brief 获取当前块同一page右边的分块.
**/
const html_area_t *get_right_area_in_page(const html_area_t *area);
/**
 * @brief 获取当前块同一page左边的分块.
**/
const html_area_t *get_left_area_in_page(const html_area_t *area);

#endif /* EASOU_MARK_COM_H_ */

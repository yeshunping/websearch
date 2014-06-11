/**
 * @file vhtml_com.h

 * @date 2011/06/20
 * @version 1.0(create)
 * @brief  内部使用的vtree分析工具:
 * 1, 解析style属性的函数;
 * 2, vTree非递归遍历函数;
 *  
 **/

#ifndef  __EASOU_VHTML_COM_H_
#define  __EASOU_VHTML_COM_H_

#include <ctype.h>
#include <string.h>
#include "util/htmlparser/utils/chinese.h"

#define DEFAULT_MAX_CSS_PAGE_SIZE	(256*1024)		  /**< 默认的CSS文本最大字节数  */
#define DEFAULT_CSS_NUM_INPOOL	8		  			/**< 默认分配的CSS结构数量  */

// ----------------------- //
// 以下为内部接口.解析style属性.
// 非Advanced User勿用.

#define MAX_ATTR_NAME_LENGTH    32
#define MAX_ATTR_VALUE_LENGTH   32

#define DEFAULT_CSS_PROP_NODE_NUM	512		  /**<  */

#define PX4WIDTH 15		  /**<  */
#define PX4HEIGHT 16		  /**<   */
#define INPUTWIDTH 150		  /**<  */
#define SELECTWIDTH 150		  /**<   */
#define DEFAULT_CX 8		  /**<   */
#define DEFAULT_CY 15		  /**< */

#define IS_CODE_NODE(vnode,code)	(vnode->hpNode->html_tag.tag_code==code)

#define MAX_TAG_NUM	256
#define UGLY_NUM	10000	//TODO:These codes using this num should be carefully reconsidered.
/**
 * @brief 分数
 */
typedef struct _fract_num_t
{
	int son; /**< 分子  */
	int mother; /**< 分母  */
} fract_num_t;

typedef struct _style_attr_t
{
	char name[MAX_ATTR_NAME_LENGTH];
	char value[MAX_ATTR_VALUE_LENGTH];
} style_attr_t;

/**
 * @brief 解析一个style代码段

 * @date 2011/06/20
 **/
int parse_style_attr(const char *style_str, style_attr_t *style_attr, int max_style_attr);

/**
 * @brief 得到属性值

 * @date 2011/06/20
 **/
int get_style_attr_value(const char *name, char *value, int vsize, style_attr_t *style_attr, int style_num);

/**
 * @brief 判断val是否给定的值.
 * @param [in/out] val   : const char* 待判断的属性值.
 * @param [in/out] dest   : const char* 给定的值.
 * @param [in/out] dest_len   : size_t 给定值的长度.
 **/
bool is_attr_value(const char *val, const char *dest, size_t dest_len);

/**
 * @brief 字符串转化为分数.
 * 为消除浮点数影响，将字符串化为分数进行计算.

 * @date 2011/06/20
 **/
void atofract(fract_num_t *fract, const char *val);

/**
 * 判断两个字符之间是否全是空格
 */
bool is_only_space_between(const char *begin, const char *end);

/**
 * @brief 计算文本大小,忽略空格.
 * 认为一个ASCII字符占一个文本大小单位,一个汉字占两个文本大小单位.
 * 文本大小结合字体可以算出文本的宽度.
 * 这个值与编码有关.
 **/
int getTextSize(const char *src);

/**
 * @brief	根据编码区间判断是否有用的字.
 * 跟编码相关.
 **/
int is_valid_word(const char *p);

/**
 * @brief	是否空白字符.
 **/
bool is_space_text(const char *text);

#endif  //__EASOU_VHTML_COM_H_

/**
 * easou_vhtml_basic.h
 * Description: vtree解析时以及对外的一些基本判断和设置操作,数据结构的定义
 */

#ifndef EASOU_VHTML_BASIC_H_
#define EASOU_VHTML_BASIC_H_

#include <sys/types.h>
#include <stdint.h>
#include "util/htmlparser/utils/nodepool.h"
#include "util/htmlparser/htmlparser/html_tree.h"
#include "util/htmlparser/cssparser/css_parser.h"
#include "util/htmlparser/cssparser/css_utils.h"
#include "util/htmlparser/vhtmlparser/vhtml_inner.h"
#include "util/htmlparser/vhtmlparser/vstruct_profiler.h"

#define SET_LINE_BREAK(prop) (prop |= 0x1)
#define IS_LINE_BREAK(prop) (prop & 0x1)
#define SET_TEXT_WARP(prop) (prop |= (0x1<<1))
#define IS_TEXT_WARP(prop) (prop & (0x1<<1))
#define SET_ABSOLUTE(prop) (prop |= (0x1<<2))
#define IS_ABSOLUTE(prop) (prop & (0x1<<2))
#define SET_STYLED(prop) (prop |= (0x1<<3))
#define IS_STYLED(prop) (prop & (0x1<<3))
#define SET_BREAK_BEFORE(prop) (prop |= (0x1<<4))
#define IS_BREAK_BEFORE(prop) (prop & (0x1<<4))
#define SET_LINE_BREAK_WARP(prop) (prop |= (0x1<<5))
#define IS_LINE_BREAK_WARP(prop) (prop & (0x1<<5))

#define IS_FLOAT_LEFT(prop)	(prop & (0x1<<6))
#define SET_FLOAT_LEFT(prop)	(prop |= (0x1<<6))
#define IS_FLOAT_RIGHT(prop)	(prop & (0x1<<7))
#define SET_FLOAT_RIGHT(prop)	(prop |= (0x1<<7))
#define IS_FLOAT(prop)	(prop & ((0x1<<6)|(0x1<<7)))
#define IS_CLEAR_LEFT(prop) (prop & (0x1<<8))
#define SET_CLEAR_LEFT(prop) (prop |= (0x1<<8))
#define IS_CLEAR_RIGHT(prop) (prop & (0x1<<9))
#define SET_CLEAR_RIGHT(prop) (prop |= (0x1<<9))

#define IS_TEXT_VNODE(prop)	(prop & (0x1<<10))
#define SET_TEXT_VNODE(prop)	(prop |= (0x1<<10))

#define IS_BLOCK_TAG(prop)	(prop & (0x1<<11))
#define SET_BLOCK_TAG(prop)	(prop |= (0x1<<11))

#define IS_EMBODY_NOWX_IMG(prop)	(prop & (0x1<<12))		  /**< 是否包含有无宽度的图片       */
#define SET_EMBODY_NOWX_IMG(prop)	(prop |= (0x1<<12))
#define CLEAR_EMBODY_NOWX_IMG(prop)	(prop &= ~(0x1<<12))

#define IS_INCLUDE_INTER(prop)	(prop & (0x1<<17))		  /**< 是否包含有交互标签       */
#define SET_INCLUDE_INTER(prop)	(prop |= (0x1<<17))
#define CLEAR_INCLUDE_INTER(prop)	(prop &= ~(0x1<<17))

#define IS_BORDER(prop)	(prop & (0x1<<18))		  /**< 是否有边框      */
#define SET_BORDER(prop)	(prop |= (0x1<<18))
#define CLEAR_BORDER(prop)	(prop &= ~(0x1<<18))

#define TOP_BIT 	(0x1<<28)
#define LEFT_BIT 	(0x1<<29)
#define BOTTOM_BIT	(0x1<<30)
#define RIGHT_BIT	(0x1<<31)

#define IS_IN_STYLE(prop)	(prop & (0x1<<13) )		  /**< 是否有属性在STYLE属性中 */
#define SET_IN_STYLE(prop)	(prop |= (0x1<<13) )		  /**< 有属性在STYLE属性中 */
#define HAS_HTML_FONT_ATTR(prop)	(prop & (0x1<<14) )		  /**< 是否HTML属性中有字体信息 */
#define SET_HAS_HTML_FONT_ATTR(prop)	(prop |= (0x1<<14) )		  /**< 设置HTML属性中含有字体信息 */

#define SET_REPEAT_STRUCT_PARENT(prop)	(prop |= (0x1<<15))
#define IS_REPEAT_STRUCT_PARENT(prop)	(prop & (0x1<<15))
#define SET_REPEAT_STRUCT_CHILD(prop)	(prop |= (0x1<<16))
#define IS_REPEAT_STRUCT_CHILD(prop)	(prop & (0x1<<16))
#define CLEAR_REPEAT_STRUCT_CHILD(prop)	(prop &= ~(0x1<<16))

#define VTREE_ERROR	(-1)
#define	VTREE_NORMAL	1
#define	VTREE_FETCH_CSS_FAIL	2

#define DEFAULT_FONT_SIZE	16		  			/**< 网页默认字体大小为16px  */
#define DEFAULT_BGCOLOR	(0xffffff)		  		/**< 网页默认背景颜色：白色  */
#define DEFAULT_COLOR	(0x000000)		  		/**< 网页默认前景颜色：黑色  */
#define DEFAULT_LINK_COLOR	(0x0000ff)		    /**< 链接默认前景颜色：蓝色  */

#define DEFAULT_IMG_SIZE	15		  			/**< 默认的图片大小      */
#define DEFAULT_PAGE_WX 1200		  			/**< 默认的页面宽度  */
#define MAX_TRUST_VALUE_FOR_VNODE	10		  	/**<  vnode->trust的最大值 */

#define COLOR_SIGN_MAX_COLLI_NUM	4		  /**< 签名相同的最大颜色数量  */
#define MAX_FONT_SIZE	10000

#define FONT_SIZE_TO_CHR_WX(px)	(px/2)		  /**< 根据字体大小计算单字符的宽度  */
#define CSS_PAGE_LEN 512000

/**
 * @brief 颜色的RGB表示.
 */
typedef struct _rgb_t {
  unsigned int r :8;
  unsigned int g :8;
  unsigned int b :8;
} rgb_t;

/**
 * @brief 节点的CSS属性.
 */
typedef struct _css_prop_node_t {
  easou_css_prop_type_t type; /**< 属性类型  */
  int priority; /**< 属性权值  */
  char *value; /**< 属性值  */
  struct _css_prop_node_t *next;
} css_prop_node_t;

typedef enum _text_align_t {
  VHP_TEXT_ALIGN_LEFT = 0, /**< 左对齐，默认  */
  VHP_TEXT_ALIGN_CENTER = 1, /**< 居中       */
  VHP_TEXT_ALIGN_RIGHT = 2 /**< 右对齐       */
} text_align_t;

/**
 * @brief vnode的节点类型
 */
typedef enum _vnode_type_t {
  TYPE_UNKNOWN = 0,  //未知
  TYPE_INTERACTION = 1,  //交互节点
} vnode_type_t;

/**
 * @brief 对边框的描述
 * @author sue
 * @date 2013/05/21
 */
struct border_t {
  unsigned int top; /**< 上边框的宽度 */
  unsigned int left; /**< 左边框的宽度 */
  unsigned int right; /**< 右边框的宽度 */
  unsigned int bottom; /**< 下边框的宽度 */
  unsigned int pad_top; /**< 上间距 */
  unsigned int pad_left; /**< 左间距 */
  unsigned int pad_right; /**< 右间距 */
  unsigned int pad_bottom;/**< 下间距 */
};

/**
 * @brief 对字体的描述
 */
typedef struct _font_t {
  unsigned int in_link :1; /**< 是否处在链接中，即是否是链接  */
  unsigned int align :2; /**< 文本对齐属性       */
  unsigned int header_size :3; /**< 标题字体：h1~h6,其对应header_size为1到6*/
  unsigned int is_bold :1; /**< 粗体  */
  unsigned int is_strong :1; /**< 重点强调	*/
  unsigned int is_big :1; /**< 较大字体  */
  unsigned int is_small :1; /**< 较小字体  */
  unsigned int is_italic :1; /**< 斜体  */
  unsigned int is_underline :1; /**< 下划线   */
  int size :20; /**< 字体大小，以px为单位，浏览器默认字体大小为16px  */
  int line_height :20; /**< 行高，以px为单位，默认值为当前字体大小  */
  unsigned int bgcolor :24; /**< 背景颜色，用RGB表示，如#ffffff   */
  unsigned int color :24; /**< 字体颜色(或前景颜色)，用RGB表示，如#000000  */
} font_t;

typedef struct _vhtml_struct_prof_t vhtml_struct_prof_t;
typedef struct _html_area_t html_area_t;

/**
 * @brief vtree上一个节点的描述.
 */
typedef struct _html_vnode_t {
  html_node_t *hpNode; /**< 该节点对应的html_tree上的节点  */
  unsigned int isValid :1; /**< 是否不	占据空间的节点，即是否可视  */
  unsigned int inLink :1; /**< 是否在链接内   */

  vnode_type_t type;  //节点类型
  font_t font; /**< 字体信息  */
  border_t border; /**< 边框信息 */
  int subtree_diff_font; /**< 子树有多少不同的中文字体，包括自己（只统计字体小于40的） */
  int subtree_max_font_size; /**< 子树的最大字体，包括自己 */
  int subtree_border_num; /**< 子树（包括自己）具有border属性的个数 */
  char fontSizes[40];
  int wx; /**< 节点的宽度  */
  int hx; /**< 节点的高度  */
  int xpos, ypos; /**< 节点左上角的X,Y坐标,页面左上角坐标为(0,0)   */
  int textSize; /**< 文本节点的文本大小  */
  int cn_num; /**< 文本节点的汉字个数 */
  int subtree_textSize; /**< 以当前节点为根节点的子树的textSize  */
  int subtree_anchorSize; /**< 以当前节点为根节点的子树的anchorSize  */
  int subtree_cn_num; /**< 以当前节点为根节点的子树具有的汉字个数 */
  int depth; /**< 节点的深度, 根节点深度为0 */
  int id; /**< 节点的唯一ID, 从0开始+1递增编号 */
  u_int property; /**< 节点按位设置的属性   */
  vstruct_info_t *struct_info; /**<  当前节点对应的子树结构信息 */
  css_prop_node_t *css_prop; /**< 当前节点对应的CSS属性链表  */
  html_area_t *hp_area; /**< 包含该节点的最小分块*/
  struct _html_vnode_t * firstChild; /**< 第一个孩子  */
  struct _html_vnode_t * prevNode; /**< 前一个兄弟  */
  struct _html_vnode_t * nextNode; /**< 后一个兄弟   */
  struct _html_vnode_t * upperNode; /**< 父亲节点 */

  short trust :5; /**< 大小或位置的可信度。
   对于图片节点来说，trust这样定义：
   a)	先令trust = 10;
   b)	若宽度未知，trust -= 3;
   c)	若高度未知, trust -= 2;
   d)	若宽度设为默认值（即未估算），trust -= 3；
   e)	若高度设为默认值（即未估算），trust -= 2。
   即：
   ====================================================================

   高度已知	高度未知且估算	高度未知且为默认值
   宽度已知		10				8				6
   宽度未知且估算		7				5				3
   宽度未知且为默认值		4				2				0
   ====================================================================
   */
  // for inner use
  short wp, hp; /**< 节点的百分比宽度和高度  */
  int min_wx; /**< 节点最小可能宽度  */
  int max_wx; /**< 节点最大能充满的宽度 */
  int colspan; /**< 表格的单元格的列跨度  */
  html_vtree_t *vtree;  //节点所属的vtree
  void *user_ptr;  //用户自定义指针，可以指向自己需要的结构
  //int subnodetype;//标示该节点的后裔节点包含的type，最后一位是否含有P，右边2是否含有DIV，右边3位是否含有table
  int whxy;  //css 是否指定宽度、高度、xpos、ypos
} html_vnode_t;

/**
 * @brief Vtree树结构.
 *   维护vtree的树结构,内存.
 */
typedef struct _html_vtree_t {
  html_tree_t *hpTree; /**<  vtree对应的html_tree  */
  html_vnode_t *root; /**<  vtree的根节点  */

  /** Below for inner use only */
  nodepool_t np; /**< html_vnode的节点池   */
  nodepool_t css_np; /**<  css_prop_node_t的节点池  */
  nodepool_t struct_np; /**< 一般结构信息的节点池 */
  unsigned int struct_np_inited :1; /**< 是否分配了结构信息的存储空间 */
  unsigned int normal_struct_info_added :1; /**< 是否已计算了一般的结构信息 */
  unsigned int repeat_struct_info_added :1; /**< 是否已计算了重复结构信息*/
  html_vnode_t *body; /**<  vtree的body节点  */
} html_vtree_t;

/**
 * @brief vtree输入结构.用于包装CSS相关的输入..
 *	每个VTREE对应这样一个结构.
 */
typedef struct _vtree_in_t {
  easou_css_env_t *css_env; /**< CSS解析环境 */
  char CSS_OUT_PAGE[CSS_PAGE_LEN];
  unsigned long long url_num;
  unsigned long long request_css_num;
  unsigned long long missing_css_num;
} vtree_in_t;

/**
 * @brief 将颜色的数值表示转化为RGB表示.
 *
 * @param [in] color   : unsigned int 颜色的数值表示，如0xffffff
 * @return  rgb_t
 **/
rgb_t int2rgb(unsigned int color);

bool is_same_font(font_t *a, font_t *b);

/**
 * @brief 是否灰色。
 * R,G,B值相同，但又不为黑或白，则为灰色.
 * @param [in/out] color   : unsigned int
 * @return  bool
 * @retval
 **/
bool is_gray_color(unsigned int color);

/**
 * @brief	解析各种单位的长度，统一输出为px为单位的长度。
 * @param [in] value   : const char*	长度字符串
 * @param [in] base_size   : int 基准长度，比例单位长度需要
 * @param [out] _unit   : const char** 返回原长度单位
 * @return  int	返回解析出来的px长度；若没有单位，返回-1。
 **/
int parse_length(const char *value, int base_size, const char **_unit);

int get_text_length(int font_size, int cn_num, int chr_num);

int html_vnode_visit_ex(html_vnode_t *html_vnode,
                        int (*start_visit)(html_vnode_t *, void *),
                        int (*finish_visit)(html_vnode_t *, void *),
                        void *result);

int html_vtree_visit_ex(html_vtree_t *html_vtree,
                        int (*start_visit)(html_vnode_t *, void *),
                        int (*finish_visit)(html_vnode_t *, void *),
                        void *result);

void print_font(font_t *pfont);

#endif /* EASOU_VHTML_BASIC_H_ */

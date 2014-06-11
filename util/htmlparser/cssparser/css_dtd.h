
#ifndef EASOU_CSS_DTD_H_
#define EASOU_CSS_DTD_H_

#define UL_MAX_SITE_LEN 1024
#define UL_MAX_PORT_LEN 1024
#define UL_MAX_PATH_LEN 1024

#include "util/htmlparser/htmlparser/html_dom.h"
#include "util/htmlparser/utils/simplehashmap.h"

/**
 * @brief CSS属性按功能划分类型
 */
typedef enum _css_prop_func_type
{
	CSS_PROP_HUNC_FONT, /* 字体属性 */
	CSS_PROP_HUNC_GEO, /* 几何属性 */
	CSS_PROP_HUNC_OTHER /* 其他属性  */
} css_prop_func_type;

extern short css_prop_first_char_map[]; /* CSS属性按首字母排序数组 */

/* css_property_name_array[]与prop_type_array[]每个元素一一对应 */
extern const char *css_property_name_array[];

extern short css_other_usefull_prop[]; /* 除字体，位置大小属性外其它有用的属性 */

/**
 * @brief 所有的CSS属性
 */
typedef enum _css_prop_type_t
{
	CSS_PROP_ACCELERATOR,
	CSS_PROP_AZIMUTH,
	CSS_PROP_BACKGROUND,
	CSS_PROP_BACKGROUND_ATTACHMENT,
	CSS_PROP_BACKGROUND_COLOR,
	CSS_PROP_BACKGROUND_IMAGE,
	CSS_PROP_BACKGROUND_POSITION,
	CSS_PROP_BACKGROUND_POSITION_X,
	CSS_PROP_BACKGROUND_POSITION_Y,
	CSS_PROP_BACKGROUND_REPEAT,
	CSS_PROP_BEHAVIOR,
	CSS_PROP_BORDER,
	CSS_PROP_BORDER_BOTTOM,
	CSS_PROP_BORDER_BOTTOM_COLOR,
	CSS_PROP_BORDER_BOTTOM_STYLE,
	CSS_PROP_BORDER_BOTTOM_WIDTH,
	CSS_PROP_BORDER_COLLAPSE,
	CSS_PROP_BORDER_COLOR,
	CSS_PROP_BORDER_LEFT,
	CSS_PROP_BORDER_LEFT_COLOR,
	CSS_PROP_BORDER_LEFT_STYLE,
	CSS_PROP_BORDER_LEFT_WIDTH,
	CSS_PROP_BORDER_RIGHT,
	CSS_PROP_BORDER_RIGHT_COLOR,
	CSS_PROP_BORDER_RIGHT_STYLE,
	CSS_PROP_BORDER_RIGHT_WIDTH,
	CSS_PROP_BORDER_SPACING,
	CSS_PROP_BORDER_STYLE,
	CSS_PROP_BORDER_TOP,
	CSS_PROP_BORDER_TOP_COLOR,
	CSS_PROP_BORDER_TOP_STYLE,
	CSS_PROP_BORDER_TOP_WIDTH,
	CSS_PROP_BORDER_WIDTH,
	CSS_PROP_BOTTOM,
	CSS_PROP_CAPTION_SIDE,
	CSS_PROP_CLEAR,
	CSS_PROP_CLIP,
	CSS_PROP_COLOR,
	CSS_PROP_CONTENT,
	CSS_PROP_COUNTER_INCREMENT,
	CSS_PROP_COUNTER_RESET,
	CSS_PROP_CUE,
	CSS_PROP_CUE_AFTER,
	CSS_PROP_CUE_BEFORE,
	CSS_PROP_CURSOR,
	CSS_PROP_DIRECTION,
	CSS_PROP_DISPLAY,
	CSS_PROP_ELEVATION,
	CSS_PROP_EMPTY_CELLS,
	CSS_PROP_FILTER,
	CSS_PROP_FLOAT,
	CSS_PROP_FONT,
	CSS_PROP_FONT_FAMILY,
	CSS_PROP_FONT_SIZE,
	CSS_PROP_FONT_SIZE_ADJUST,
	CSS_PROP_FONT_STRETCH,
	CSS_PROP_FONT_STYLE,
	CSS_PROP_FONT_VARIANT,
	CSS_PROP_FONT_WEIGHT,
	CSS_PROP_HEIGHT,
	CSS_PROP_IME_MODE,
	CSS_PROP_INCLUDE_SOURCE,
	CSS_PROP_LAYER_BACKGROUND_COLOR,
	CSS_PROP_LAYER_BACKGROUND_IMAGE,
	CSS_PROP_LAYOUT_FLOW,
	CSS_PROP_LAYOUT_GRID,
	CSS_PROP_LAYOUT_GRID_CHAR,
	CSS_PROP_LAYOUT_GRID_CHAR_SPACING,
	CSS_PROP_LAYOUT_GRID_LINE,
	CSS_PROP_LAYOUT_GRID_MODE,
	CSS_PROP_LAYOUT_GRID_TYPE,
	CSS_PROP_LEFT,
	CSS_PROP_LETTER_SPACING,
	CSS_PROP_LINE_BREAK,
	CSS_PROP_LINE_HEIGHT,
	CSS_PROP_LIST_STYLE,
	CSS_PROP_LIST_STYLE_IMAGE,
	CSS_PROP_LIST_STYLE_POSITION,
	CSS_PROP_LIST_STYLE_TYPE,
	CSS_PROP_MARGIN,
	CSS_PROP_MARGIN_BOTTOM,
	CSS_PROP_MARGIN_LEFT,
	CSS_PROP_MARGIN_RIGHT,
	CSS_PROP_MARGIN_TOP,
	CSS_PROP_MARKER_OFFSET,
	CSS_PROP_MARKS,
	CSS_PROP_MAX_HEIGHT,
	CSS_PROP_MAX_WIDTH,
	CSS_PROP_MIN_HEIGHT,
	CSS_PROP_MIN_WIDTH,
	CSS_PROP__MOZ_BINDING,
	CSS_PROP__MOZ_BORDER_BOTTOM_COLORS,
	CSS_PROP__MOZ_BORDER_LEFT_COLORS,
	CSS_PROP__MOZ_BORDER_RADIUS,
	CSS_PROP__MOZ_BORDER_RADIUS_BOTTOMLEFT,
	CSS_PROP__MOZ_BORDER_RADIUS_BOTTOMRIGHT,
	CSS_PROP__MOZ_BORDER_RADIUS_TOPLEFT,
	CSS_PROP__MOZ_BORDER_RADIUS_TOPRIGHT,
	CSS_PROP__MOZ_BORDER_RIGHT_COLORS,
	CSS_PROP__MOZ_BORDER_TOP_COLORS,
	CSS_PROP__MOZ_OPACITY,
	CSS_PROP__MOZ_OUTLINE,
	CSS_PROP__MOZ_OUTLINE_COLOR,
	CSS_PROP__MOZ_OUTLINE_STYLE,
	CSS_PROP__MOZ_OUTLINE_WIDTH,
	CSS_PROP__MOZ_USER_FOCUS,
	CSS_PROP__MOZ_USER_INPUT,
	CSS_PROP__MOZ_USER_MODIFY,
	CSS_PROP__MOZ_USER_SELECT,
	CSS_PROP_ORPHANS,
	CSS_PROP_OUTLINE,
	CSS_PROP_OUTLINE_COLOR,
	CSS_PROP_OUTLINE_STYLE,
	CSS_PROP_OUTLINE_WIDTH,
	CSS_PROP_OVERFLOW,
	CSS_PROP_OVERFLOW_X,
	CSS_PROP_OVERFLOW_Y,
	CSS_PROP_PADDING,
	CSS_PROP_PADDING_BOTTOM,
	CSS_PROP_PADDING_LEFT,
	CSS_PROP_PADDING_RIGHT,
	CSS_PROP_PADDING_TOP,
	CSS_PROP_PAGE,
	CSS_PROP_PAGE_BREAK_AFTER,
	CSS_PROP_PAGE_BREAK_BEFORE,
	CSS_PROP_PAGE_BREAK_INSIDE,
	CSS_PROP_PAUSE,
	CSS_PROP_PAUSE_AFTER,
	CSS_PROP_PAUSE_BEFORE,
	CSS_PROP_PITCH,
	CSS_PROP_PITCH_RANGE,
	CSS_PROP_PLAY_DURING,
	CSS_PROP_POSITION,
	CSS_PROP_QUOTES,
	CSS_PROP__REPLACE,
	CSS_PROP_RICHNESS,
	CSS_PROP_RIGHT,
	CSS_PROP_RUBY_ALIGN,
	CSS_PROP_RUBY_OVERHANG,
	CSS_PROP_RUBY_POSITION ,
	CSS_PROP_SCROLLBAR_3D_LIGHT_COLOR,
	CSS_PROP_SCROLLBAR_ARROW_COLOR,
	CSS_PROP_SCROLLBAR_BASE_COLOR,
	CSS_PROP_SCROLLBAR_DARK_SHADOW_COLOR,
	CSS_PROP_SCROLLBAR_FACE_COLOR,
	CSS_PROP_SCROLLBAR_HIGHLIGHT_COLOR,
	CSS_PROP_SCROLLBAR_SHADOW_COLOR,
	CSS_PROP_SCROLLBAR_TRACK_COLOR,
	CSS_PROP__SET_LINK_SOURCE,
	CSS_PROP_SIZE,
	CSS_PROP_SPEAK,
	CSS_PROP_SPEAK_HEADER,
	CSS_PROP_SPEAK_NUMERAL,
	CSS_PROP_SPEAK_PUNCTUATION,
	CSS_PROP_SPEECH_RATE,
	CSS_PROP_STRESS,
	CSS_PROP_TABLE_LAYOUT,
	CSS_PROP_TEXT_ALIGN,
	CSS_PROP_TEXT_ALIGN_LAST,
	CSS_PROP_TEXT_AUTOSPACE,
	CSS_PROP_TEXT_DECORATION,
	CSS_PROP_TEXT_INDENT,
	CSS_PROP_TEXT_JUSTIFY,
	CSS_PROP_TEXT_KASHIDA_SPACE,
	CSS_PROP_TEXT_OVERFLOW,
	CSS_PROP_TEXT_SHADOW,
	CSS_PROP_TEXT_TRANSFORM,
	CSS_PROP_TEXT_UNDERLINE_POSITION,
	CSS_PROP_TOP,
	CSS_PROP_UNICODE_BIDI,
	CSS_PROP__USE_LINK_SOURCE,
	CSS_PROP_VERTICAL_ALIGN,
	CSS_PROP_VISIBILITY,
	CSS_PROP_VOICE_FAMILY,
	CSS_PROP_VOLUME ,
	CSS_PROP_WHITE_SPACE,
	CSS_PROP_WIDOWS,
	CSS_PROP_WIDTH,
	CSS_PROP_WORD_BREAK,
	CSS_PROP_WORD_SPACING,
	CSS_PROP_WORD_WRAP,
	CSS_PROP_WRITING_MODE,
	CSS_PROP_UNKNOWN
} css_prop_type_t;

#define CSS_PROPERTY_NUM (CSS_PROP_UNKNOWN-CSS_PROP_ACCELERATOR)		  /**< CSS属性个数  */

/**
 * @brief CSS属性类型信息
 */
typedef struct _css_prop_type_info_t
{
	_css_prop_type_t type; /* 属性类型  */
	bool is_font_prop; /* 是否字体属性  */
	bool is_geo_prop; /* 是否几何属性  */
} css_prop_type_info_t;

/* css_property_name_array[]与prop_type_array[]每个元素一一对应 */
extern const css_prop_type_info_t prop_typeinfo_array[];

#define CSS_UNIVERSAL_SELECTOR	((html_tag_type_t)(-1))		  /**< 全局选择子，基本上是每个标签都可以被渲染  */

/**
 * @brief CSS属性选择子类型.
 */
typedef enum _css_attr_selector_type_t
{
	CSS_ATTR_SELECT_MATCH_NOVALUE, /* 无属性值匹配  */
	CSS_ATTR_SELECT_MATCH_EXACTLY, /* 属性严格匹配  */
	CSS_ATTR_SELECT_MATCH_ANY, /* 匹配其中一个 */
	CSS_ATTR_SELECT_MATCH_BEGIN, /* 匹配前缀  */
	CSS_ATTR_SELECT_OTHER /* 其他  */
} css_attr_selector_type_t;

/**
 * @brief CSS属性选择符.
 */
typedef struct _css_attr_selector_t
{
	css_attr_selector_type_t type; /* 属性选择子类型  */
	html_attribute_t attr; /* 属性/值  */
	struct _css_attr_selector_t *next; /* 下一个属性选择子  */
} css_attr_selector_t;

/**< CLASS选择子  */
typedef char* css_class_selector_t;
/**< ID选择子  */
typedef char* css_id_selector_t;
/**< 伪类选择子  */
typedef char* css_pseudo_selector_t;

/**
 * @brief CSS选择子组合类型
 */
typedef enum _css_selector_combinator_t
{
	CSS_NON_COMBINATOR, /**< 无组合  */
	CSS_DESCEND_COMBINATOR, /**< 后代选择子  */
	CSS_CHILD_COMBINATOR, /**< 子选择子  */
	CSS_ADJACENT_COMBINATOR /**< 相邻选择子  */
} css_selector_combinator_t;

enum simple_selector_type_t
{
	SIMPLE_SELECTOR_ALL,
	SIMPLE_SELECTOR_TAG,
	SIMPLE_SELECTOR_CLASS,
	SIMPLE_SELECTOR_ID,
	SIMPLE_SELECTOR_ATTR,
	SIMPLE_SELECTOR_TAGCLASS,
	SIMPLE_SELECTOR_TAGATTR,
	SIMPLE_SELECTOR_TAGID,
	SIMPLE_SELECTOR_PSEUDO,
	SIMPLE_SELECTOR_OTHER
};

/**
 * @brief 一般选择子.
 */
typedef struct _css_simple_selector_t
{
	simple_selector_type_t type;
	html_tag_type_t tag_type; /**< 类型选择子(转化为HTML_TAG类型)  */
	char *name; /**< 类型选择子对应的HTML_TAG名  */
	css_attr_selector_t *attr_selector; /**< 属性选择子  */
	css_class_selector_t class_selector; /**< CLASS选择子  */
	css_id_selector_t id_selector; /**<  ID选择子 */
	css_pseudo_selector_t pseudo_selector; /**< 伪类选择子  */
} css_simple_selector_t;

/**
 * @brief CSS选择子,由简单选择子组合而成.
 */
typedef struct _css_selector_t
{
	struct _css_selector_t *pre_selector; /**< 前选择子  */
	css_selector_combinator_t combinator; /**< 组合符号 */
	css_simple_selector_t simple_selector; /**< 简单选择子  */
} css_selector_t;

/**
 * @brief CSS属性
 */
typedef struct _css_property_t
{
	css_prop_type_t type; /**< 属性类型  */
	char *name; /**< 属性  */
	char *value; /**< 值  */
	struct _css_property_t *next;
} css_property_t;

/**
 * @brief CSS规则集
 */
typedef struct _css_ruleset_t
{
	int id;
	css_selector_t *selector; /**< 选择子  */
	css_property_t *all_property_list; /**< 所有属性列表  */
	css_property_t *font_prop_begin, *font_prop_end; /**< 字体属性的开始和结尾指针  */
	css_property_t *geo_prop_begin, *geo_prop_end; /**< 几何属性的开始和结尾指针  */
	css_property_t *other_prop_begin, *other_prop_end; /**< 其他属性的开始和结尾指针  */
	struct _css_ruleset_t *next; /**< 在CSS结构上的下一个规则集  */
	struct _css_ruleset_t *index_next; /**< 在索引上的下一个规则集  */
} css_ruleset_t;

/**
 * @brief 字符串堆结构,用于存放CSS解析的字符串.
 */
typedef struct _css_str_heap_t
{
	char *p_heap; /**< 分配的内存开始指针  */
	size_t heap_size; /**< 分配的内存大小  */
	char *p_heap_avail; /**< 当前可用的内存指针  */
} css_str_heap_t;

/**
 * @brief 内存管理节点.
 */
typedef struct _css_mem_node_t
{
	void *p_mem; /**< 内存块指针  */
	size_t mem_size; /**< 内存块大小  */
	struct _css_mem_node_t *next;
} css_mem_node_t;

/**
 * @brief 节点池.
 *	用于管理已分配的一系列内存块,并负责分配每一个节点的内存.
 */
typedef struct _css_nodepool_t
{
	css_mem_node_t *mem_node_list; /**< 已分配的内存链  */
	void *p_curr_mem; /**< 当前使用的内存块  */
	size_t p_curr_mem_size; /**< 当前内存的大小  */
	void *p_pool_avail; /**< 当前可用的内存开头处指针  */
} css_nodepool_t;

/**
 * @brief CSS解析使用的内部结构
 */
typedef struct _css_inner_t
{
	css_str_heap_t str_heap; /**< 字符串堆结构  */
	css_nodepool_t nodepool; /**< 节点池  */
} css_inner_t;

/**
 * @brief CSS索引结点
 */
typedef struct _css_index_node_t
{
	const char *key; /**< 索引键值  */
	void *data; /**< 索引的数据  */
	struct _css_index_node_t *next;
} css_index_node_t;

/**
 * @brief CSS简单选择子的索引结点
 */
typedef struct _simple_selector_index_node_t
{
	int selector_id;
	short start_pos;
	short pos;
	css_ruleset_t *ruleset;
	struct _simple_selector_index_node_t *next;
} simple_selector_index_node_t;

#define TAG_TYPE_NUM_LIMIT	256 		  /**< TAG类型的数量  */

/**
 * @brief CSS索引结构
 */
typedef struct _css_index_t
{
	css_index_node_t *type_index[TAG_TYPE_NUM_LIMIT]; /**< 每个TAG类型对应一个索引  */
	hashmap_t *class_map;
} css_index_t;

/**
 * @brief CSS解析出来的结构
 */
typedef struct _css_t
{
	css_inner_t css_inner; /**< CSS内部结构,用于内存管理  */
	css_ruleset_t *all_ruleset_list; /**< 所有规则集列表  */
	css_index_t index; /**< CSS索引结构  */
} css_t;

/**
 * @brief 扫描状态记录器
 *
 */
typedef enum _css_token_type_t
{
	CSS_TOKEN_URI, /**< URI token  */
	CSS_TOKEN_STRING, /**< 扫描到字符串  */
	CSS_TOKEN_NORMAL, /**< 一般扫描  */
	CSS_TOKEN_UNKNOWN /**< 未知扫描 */
} css_token_type_t;

/**
 * @brief 扫描状态记录器
 */
typedef enum _css_state_t
{
	CSS_STAT_SCAN_SELECTOR, /**< 扫描选择子  */
	CSS_STAT_SCAN_PROP_NAME, /**< 扫描属性的名称 */
	CSS_STAT_SCAN_PROP_VALUE /**< 扫描属性的值 */
} css_state_t;

/**
 * @brief css扫描器
 *
 */
typedef struct _css_scan_t
{
	char *p_curr_token; /**< 当前扫描到的字符  */
	int p_curr_token_len; /**< 当前扫描到的字符长度  */
	const char *p_next; /**<   */
	css_token_type_t type; /**< 当前正在扫描的字符类型  */
	css_state_t state; /**< 当前正在扫描的css类型  */
	const char *css_url; /**< 当前正在扫描的css的url  */
	bool test_import; /**< 是否下载css文件中import的css  */
} css_scan_t;

/**
 * @brief CSS属性信息
 */
typedef struct _css_prop_info_t
{
	char *value; /**< 属性值  */
	int prio_val; /**< 属性的优先级值  */
} css_prop_info_t;

/**
 * @brief CSS属性集
 */
typedef struct _css_property_set_t
{
	css_prop_info_t prop[CSS_PROPERTY_NUM];
} css_property_set_t;

/**
 * @brief 节点和css属性的结合体
 */
typedef struct _prop_set_param_t
{
	css_property_set_t *prop_set;
	const html_node_t *html_node;
	int prop_num;
	int order; //为css属性增加优先级，order为出现的顺序2012-05-02
} prop_set_param_t;

#endif /*EASOU_CSS_DTD_H_*/

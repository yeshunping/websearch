/*
 * easou_html_dom.h
 *
 *  Created on: 2011-11-8
 *      Author: ddt
 */

#ifndef EASOU_HTML_DOM_H_
#define EASOU_HTML_DOM_H_

#include <stddef.h>
#include "queue.h"
#include "html_pool.h"
#include "html_dtd.h"

#define VISIT_ERROR -1
#define VISIT_NORMAL 1
#define VISIT_FINISH 2
#define VISIT_SKIP_CHILD 3

#define	MAX_TITLE_SIZE         1024            //title的最大长度
#define	MAX_PAGE_SIZE          128000          //页面的最大长度
#define	MAX_CONTENT_SIZE       128000          //content的最大长度
#define MAX_LINK_NUM           300             //页面中link的最大数量
#define MAX_URL_SIZE           1024            //URL的最大长度
#define UL_MAX_URL_LEN 2048
#define UL_MAX_TEXT_LEN 1024
#define UL_MAX_PAGE_LEN 128*1024
#define MAX_ANNOTATION_LEN (64 * 1024)
#define MAX_START_TAG_NAME_LEN 16  /* 开始标签的最大长度 */
#define MAX_END_TAG_NAME_LEN 32    /* 结束标签的最大长度 */

#define POOLSZ 128*1024

#define MARK_DOMTREE_SUBTYPE(x)	((x)|=(0x1<<31))
#define IS_DOMTREE_SUBTYPE(x)	((x)&(0x1<<31))
#define MARK_DOMTREE_P_TAG(x)	((x)|=(0x1))
#define MARK_DOMTREE_DIV_TAG(x)	((x)|=(0x1<<1))
#define MARK_DOMTREE_H_TAG(x)	((x)|=(0x1<<2))
#define MARK_DOMTREE_TABLE_TAG(x)	((x)|=(0x1<<3))
#define MARK_DOMTREE_LIST_TAG(x)	((x)|=(0x1<<4))
#define MARK_DOMTREE_FORM_TAG(x)	((x)|=(0x1<<5))
#define MARK_DOMTREE_IMG_TAG(x)	((x)|=(0x1<<6))
#define IS_DOMTREE_P_TAG(x)	((x)&(0x1))
#define IS_DOMTREE_DIV_TAG(x)	((x)&(0x1<<1))
#define IS_DOMTREE_H_TAG(x)	((x)&(0x1<<2))
#define IS_DOMTREE_TABLE_TAG(x)	((x)&(0x1<<3))
#define IS_DOMTREE_LIST_TAG(x)	((x)&(0x1<<4))
#define IS_DOMTREE_FORM_TAG(x)	((x)&(0x1<<5))
#define IS_DOMTREE_IMG_TAG(x)	((x)&(0x1<<6))
/**
 * 文档类型定义
 */
enum html_doctype
{
	doctype_unknown = 0, doctype_wml = 1, // wml/wap1.0
	doctype_xhtml_MP = 2, // xhtml moible/wap2.0
	doctype_xhtml_BP = 3, //xhtml basic/web
	doctype_xhtml = 4, // xhtml/web
	doctype_html4 = 5, // html4.1/web
	doctype_html5 = 6,
// html5/web
};

/**
 * @brief Node type enum
 **/
enum dom_node_type_t
{
	ELEMENT_NODE = 1,
	ATTRIBUTE_NODE,
	TEXT_NODE,
	CDATA_SECTION_NODE,
	ENTITY_REFERENCE_NODE,
	ENTITY_NODE,
	PROCESSING_INSTRUCTION_NODE,
	COMMENT_NODE,
	DOCUMENT_NODE,
	DOCUMENT_TYPE_NODE,
	DOCUMENT_FRAGMENT_NODE,
	NOTATION_NODE
};

struct _html_tree_t;

/**
 * @brief HTML attribute node
 **/
typedef struct _html_attribute_t
{
	html_attr_type_t type; /**< 属性类型 */
	char *value; /**< 属性值 */
	int valuelength; //属性值长度
	const char *name; /**< 属性名称 */
	struct _html_attribute_t *next; /**< 下一个属性 */
} html_attribute_t;

typedef struct _html_node_t;

/**
 * @brief HTML标签的抽象
 **/
typedef struct _html_tag_t
{
	_html_node_t* html_node;
	const char *tag_name; /* 标签名称 */
	char *text; /* 标签在源代码中的表示 */
	int textlength; //节点的text长度
	html_attribute_t *attribute; /* 属性列表 */
	html_attribute_t *attr_class;
	html_attribute_t *attr_id;
	int tag_code; /* 标签编码 */
	int page_offset; /* 标签在页面中的偏移量,对于script生成的TAG,偏移量为-1 */
	int nodelength; //节点源码长度
	html_tag_type_t tag_type :16; /* 标签名称 */
	unsigned is_close_tag :1; /* 是否是结束标签 */
	unsigned is_self_closed :1; /* 是否是自结束标签 */
} html_tag_t;

/**
 * @brief HTML节点的抽象
 **/
typedef struct _html_node_t
{
	html_tag_t html_tag;
	struct _html_tree_t *owner; /* 拥有者 */
	struct _html_node_t *parent; /* 父节点 */
	struct _html_node_t *next; /* 下一个节点 */
	struct _html_node_t *prev; /* 前一个节点*/
	struct _html_node_t *child; /* 第一个儿子节点 */
	struct _html_node_t *last_child; /* 最后一个儿子节点 */
	unsigned int subnodetype; //标示该节点的后裔节点的type，最后一位是否含有P，右边2是否含有DIV，右边3是否含有H1-H6,右边4位是否含有table;右5是否含有ul或ol;右6是否含有form,右7 IMG
	unsigned int childnodetype; //标示该节点的儿子节点的type，最后一位是否含有P，右边2是否含有DIV，右边3是否含有H1-H6,右边4位是否含有table;右5是否含有ul或ol;右6是否含有form,右7 IMG
	void *user_ptr;//用户自定义指针，可以指向自己需要的结构
} html_node_t;

/**
 * @brief html节点的列表
 * @author sue
 * @date 2013-06-19
 */
typedef struct _html_node_list_t
{
	html_node_t* html_node;
	_html_node_list_t* next;
	_html_node_list_t* prev;
} html_node_list_t;

/**
 * @brief 关心tag的类型.
 */
typedef enum
{
	HTML_NORMAL_TAG, /**< 要解析的tag */
	HTML_IGNORE_TAG /**< 要忽略的tag, 这种类型tag包围内的所有内容忽略(尽管其中可能包含要解析的tag), 例如注释标签就是一例*/
} html_tag_config_type_t;

/**
 * @brief 关心的tag设置.
 */
typedef struct
{
	char start_tag_name[MAX_START_TAG_NAME_LEN]; /**< 如"<link","<!--" */
	char end_tag_name[MAX_END_TAG_NAME_LEN]; /**< 如"</link>", "-->".多个结束符以|分隔， 如"</iframe>|/>" */
	html_tag_config_type_t tag_type; /**< 这项配置的类型,是要解析还是要全部去掉 */
} html_interest_tag_t;

typedef struct _html_tree_debug_info_t
{
	//记录关闭当前节点的祖先的容错策略的触发情况
	int close_tag_err[HTML_TREE_TAG_TYPE_NUM][HTML_TREE_TAG_TYPE_NUM];
	//记录父子关系容错策略的触发情况
	int child_parent_err[HTML_TREE_TAG_TYPE_NUM][HTML_TREE_TAG_TYPE_NUM];
	int tag_count[HTML_TREE_TAG_TYPE_NUM];
	int cte_count;
	int cpe_count;
	int css_link_count;
	int script_count;
	int img_count;
} html_tree_debug_info_t;

/**
 *对dom树的抽象，其中包含了了dom树搭建过程中的一些特殊字段
 **/
typedef struct _html_tree_t
{
	html_node_t root; // HTML <root> node
	html_node_t *html; // HTML <html> node
	html_node_t *head; // HTML <head> node
	html_node_t *body; // HTML <body> node
	html_doctype doctype; /**< 文档类型 */
	unsigned int treeAttr; //dom树具有的属性，最后一位表示是否含有div节点；2位是否存在p节点；3位是否存在标题节点H1-H6；最高位表示是否标示节点的子孙节点类型
} html_tree_t;

struct _html_tokenizer_t;
typedef int (*token_state_t)(struct _html_tokenizer_t*, html_tree_t*);

/*html源代码读取器*/
typedef struct _html_tokenizer_t
{
	const char *ht_source; /*网页源码*/
	const char *ht_begin; /*网页code中开始遍历时的位置*/
	const char *ht_current; /*当前正在遍历的位置*/
	const char *ht_end; /*网页code的结束位置*/
	token_state_t ht_state; /*游标的一个状况函数指针*/
	html_node_t *ht_node; /*扫描出来的node的指针*/
	html_attribute_t *ht_attr; /*当前正在扫描的attribute的指针*/
	html_node_t *ht_opening; /*当前正在等待关闭的node*/
} html_tokenizer_t;

/**
 * @brief 用于装载当前等待关闭的节点的栈和栈节点
 **/
struct stack_item_t
{
	html_node_t *si_node;SLIST_ENTRY(stack_item_t) si_entries;
};
SLIST_HEAD(html_stack_t, stack_item_t);

/**
 * @brief Foster Stack Item
 **/
struct foster_item_t
{
	struct html_stack_t fi_stack;SLIST_ENTRY(foster_item_t) fi_entries;
};
SLIST_HEAD(html_foster_t, foster_item_t);

/**
 * @brief State handler
 **/
typedef int (*state_handler_t)(struct html_parser_t*, html_node_t*);

/**
 * @brief HTML5 Parser
 **/
struct html_parser_t
{
	struct mem_pool_t *hp_pool; //parser对应的内存池
	html_tree_t *hp_tree; //当前正在被解析的树
	html_tokenizer_t *hp_tokenizer; //当前的网页源代码读取器
	html_tokenizer_t *hp_nest_tokenizer; //备用的
	struct html_stack_t *hp_stack; //用于生成父子关系的栈
	struct html_stack_t *hp_foster_stack;
	struct html_stack_t *hp_actfmt_list;
	struct html_foster_t *hp_foster;
	html_node_t *hp_html; //html标签或者wml标签
	html_node_t *hp_body; //body标签或者card标签
	html_node_t *hp_form; //form标签
	char *hp_document_write; //动态生成的网页代码，主要在js解析时会用到
	state_handler_t hp_handler; //当前容错策略
	state_handler_t hp_last_handler; //备用handler
	struct slab_t *hp_stack_slab;
	struct slab_t *hp_foster_slab;
	unsigned hp_use_nest_tokenizer :1; //是否启用nest  tokenizer
	unsigned hp_ignore_space :1; //是否忽略空格
	unsigned hp_xml_compatible :1; //是否兼容xml文件
	unsigned hp_wml_compatible :1; //是否兼容wml文件
	unsigned hp_script_parsing :1; //是否对script进行分析
};

/**
 * @brief html dom树的实现环境
 **/
struct html_tree_impl_t
{
	html_tree_t ht_tree; /* public interface */
	struct mem_pool_t *ht_pool; /* memory pool */
	struct slab_t *ht_node_slab; /* node slab */
	struct html_parser_t *ht_parser; /* parser */
	int ht_tag_code; /* tag code variable */
};

/**
 * @brief Pre visitor
 **/
typedef int (*pre_visitor_t)(html_tag_t*, void*, int);

/**
 * @brief Post visitor
 **/
typedef int (*post_visitor_t)(html_tag_t*, void*);

/**
 * @brief Visit context
 **/
struct html_node_visit_ctx_t
{
	pre_visitor_t vc_pre_visitor;
	post_visitor_t vc_post_visitor;
	void *vc_data;
};

/**
 * @brief latin map
 **/
extern char g_latin_map[];
/**
 * @brief tag name special character map
 **/
extern char g_tag_name_map[];
/**
 * @brief attribute name special character map
 **/
extern char g_attribute_name_map[];
/**
 * @brief attribute value special character map
 **/
extern char g_attribute_value_uq_map[];

#endif /* EASOU_HTML_DOM_H_ */

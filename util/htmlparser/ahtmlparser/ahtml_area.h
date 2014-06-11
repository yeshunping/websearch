#ifndef EASOU_AHTML_AREA_H_
#define EASOU_AHTML_AREA_H_

#include <limits.h>
#include "util/htmlparser/utils/nodepool.h"
#include "util/htmlparser/vhtmlparser/vhtml_basic.h"
#include "util/htmlparser/vhtmlparser/vhtml_tree.h"

/** 块的最通用信息，坐标、长宽*/
typedef struct _area_info_t {
  int xpos;
  int ypos;
  int width;
  int height;
} area_info_t;

/**
 * @brief 分块的绝对位置，相对于页面
 */
typedef enum _html_area_abspos_mark_t {
  PAGE_UNDEFINED = 0,
  PAGE_HEADER,
  PAGE_LEFT,
  PAGE_RIGHT,
  PAGE_FOOTER,
  PAGE_MAIN,
  PAGE_MAIN_RELATED, /**< 暂时未用,老版本遗留       */
  PAGE_INVALID
} html_area_abspos_mark_t;

/**
 * @brief 分块的相对位置，相对于父分块
 */
typedef enum _html_area_relpos_mark_t {
  RELA_UNDEFINED = 0,
  RELA_HEADER,
  RELA_LEFT,
  RELA_RIGHT,
  RELA_FOOTER,
  RELA_MAIN
} html_area_relpos_mark_t;

/**
 * @brief 结合绝对位置和相对位置而标记出的每个分块在页面中的位置，该份块相对比较权威
 */
typedef enum _html_area_pos_plus_t {
  IN_PAGE_UNDEFINED = 0,
  IN_PAGE_HEADER,
  IN_PAGE_LEFT,
  IN_PAGE_RIGHT,
  IN_PAGE_FOOTER,
  IN_PAGE_MAIN
} html_area_pos_plus_t;

/**
 * @brief 分块的配置结构，用于在partition的过程中使用.
 */
typedef struct _area_config_t {
  int min_width; /**< 宽度小于此值的分块不再划分  */
  int min_height; /**< 高度小于此值的分块不再划分  */
  int min_size; /**< 面积小于此值的分块不再划分  */
  unsigned int max_depth; /**< 深度大于等于此值的分块不再划分  */
  const char *indivisible_tag_name; /**< 不可分的tag类型,传入tag名字符串,可设为NULL */
} area_config_t;

/**< 配置结构的初始值 */
static const area_config_t AREA_CONFIG_INIT_VALUE ={
  min_width : 0,
  min_height : 0,
  min_size : 0,
  max_depth : INT_MAX,
  indivisible_tag_name : NULL
};

/**
 * @brief 利用此结构体来表示用位操作的字段，防止直接用==判断.
 */
typedef struct {
  unsigned int _mark_bits;
} bits_field_t;

typedef struct _area_tree_t area_tree_t;
typedef struct _area_uid_binfo_t area_uid_binfo_t;
typedef struct _area_baseinfo_t area_baseinfo_t;

/**
 * @brief 分块节点结构.
 */
typedef struct _html_area_t {
  /**分块的位置属性*/
  html_vnode_t *begin; /**< 分块开始节点  */
  html_vnode_t *end; /**< 分块的结束节点  */
  html_vnode_t *main_vnode;
  bool isValid :1; /**< 分块是否可见  */
  bool is_pos_partition_level :1; /**< 它和它的兄弟与其父节点的位置标注(pos_plus)不完全一致 */
  area_info_t area_info; /**< 分块的物理属性 */
  html_area_abspos_mark_t abspos_mark; /**< 相对于页面的位置 */
  html_area_relpos_mark_t pos_mark; /**< 相对于父分块的位置 */
  html_area_pos_plus_t pos_plus; /**< 结合绝对位置和相对父分块的位置而进行的位置标识，更准确，建议使用 */

  /**分块的关系属性*/
  unsigned int depth; /**< 当前分块在分块树中的深度,根分块(对应整个页面)深度为0  */
  unsigned int max_depth;
  unsigned int subArea_num; /**< 子分块的数量  */
  unsigned int valid_subArea_num; /**< 可见子分块的数量  */

  struct _html_area_t *parentArea; /**< 父分块  */
  struct _html_area_t *subArea; /**< 第一个子分块  */
  struct _html_area_t *nextArea; /**< 下一个分块  */
  struct _html_area_t *prevArea; /**< 前一个分块  */

  /**分块的资源、功能、语义属性*/
  bits_field_t srctype_mark; /**< 当前块的资源类型标注  */
  bits_field_t func_mark; /**< 当前块的功能类型标注  */
  bits_field_t sem_mark; /**< 当前块的语义类型标注  */

  bits_field_t subtree_srctype_mark; /**< 以当前块为根节点的子树中含有的资源类型标注  */
  bits_field_t subtree_func_mark; /**< 以当前块为根节点的子树含有的功能类型标注  */
  bits_field_t subtree_sem_mark; /**< 以当前块为根节点的子树含有的语义类型标注  */

  bits_field_t upper_srctype_mark; /**< 当前块及当前块上层的块的资源类型标注       */
  bits_field_t upper_func_mark; /**< 当前块及当前块上层的功能类型标注       */
  bits_field_t upper_sem_mark; /**< 当前块及当前块上层的语义类型标注       */

  /**base info*/
  char *uid; /**<  用户名      */
  area_uid_binfo_t *uidbinfo; /**<  指向用户名绑定提前计算的基本信息,与mark相关      */
  area_baseinfo_t *baseinfo; /**< 指向块标注前提前计算的基本信息,对于valid的块有此属性，mark相关*/
  unsigned int no; /**< 每个分块的唯一标识号，从0开始+1递增，可用来作为用户自定义数组的下标  */
  int order; /**< 同一个分块层上的序号 */
  area_tree_t *area_tree;
  int areaattr;                        // area attribute
  int nodeTypeOfArea;  //构成块的节点类型最后一位是否含有P，右边2是否含有DIV，右边3是否含有H1-H6,右边4位是否含有table;右5是否含有ul或ol;右6是否含有form,
  struct _html_area_t *titleArea; /**描述该块的标题块  */
} html_area_t;

#define SET_LEAF_AREA(x)	((x)|=0x1)
#define IS_LEAF_AREA(x)	((x)&0x1)
#endif /* EASOU_AHTML_AREA_H_ */

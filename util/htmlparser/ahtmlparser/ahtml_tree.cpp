
#include <pthread.h>
#include "util/htmlparser/htmlparser/html_attr.h"
#include "util/htmlparser/vhtmlparser/vhtml_tree.h"
#include "util/htmlparser/ahtmlparser/ahtml_tree.h"
#include "util/htmlparser/ahtmlparser/ahtml_dtd.h"
#include "util/htmlparser/vhtmlparser/vstruct_profiler.h"
#include "util/htmlparser/utils/debug.h"

#define BIG_SHORT_INT	30000
#define DEFAULT_AREA_BLOCK_SIZE	100

#define WX_PERCENT_BE_DIVIDE	55		  /**< 一个节点宽度与父分块比例大于此值,且高度也大于一比例,则需要划分  */
#define HX_PERCENT_BE_DIVIDE	40		  /**< 一个节点高度与父分块比例大于此值,且宽度也大于一比例,则需要划分  */
#define	WX_PERCENT_DIVE_IN	100		  /**< 若一个节点宽度与父分块比例小于此值,将采用较粗糙的分块方式*/
#define HX_PERCENT_DIVE_IN	100		  /**< 若一个节点高度与父分块比例小于此值,将采用较粗糙的分块方式 */
#define SMALL_HX_VAL	1		  /**< 对于高度小于此值的节点,将考虑与其他节点组合成块  */
#define SMALL_WX_VAL	1		  /**< 对于宽度小于此值,将认为太小  */

/**
 * @brief	创建一棵分块树.
 * @param [in] cfg   : area_config_t*	用于配置分块树的分块粒度.可为NULL.
 * @return   area_tree_t*	已分配空间的分块树.

 * @date 2011/07/05
 **/
area_tree_t * area_tree_create(area_config_t *cfg) {
  area_tree_t *atree = (area_tree_t *) malloc(sizeof(area_tree_t));
  if (atree == NULL) {
    goto FAIL;
  }
  memset(atree, 0, sizeof(area_tree_t));
  if (!nodepool_init(&(atree->np), sizeof(html_area_t),
                     DEFAULT_AREA_BLOCK_SIZE)) {
    goto FAIL;
  }
  area_tree_clean(atree);
  area_tree_setConfig(atree, cfg);
  return atree;

  FAIL: if (atree) {
    area_tree_del(atree);
  }
  return NULL;
}

/**
 * @brief 清除一棵分块树.

 * @date 2011/07/12
 **/
void area_tree_clean(area_tree_t *atree) {
  nodepool_reset(&atree->np);
  atree->hp_vtree = NULL;
  atree->root = NULL;
  atree->mark_status = 0;
}

/**
 * @brief 删除分块树.

 * @date 2011/07/05
 **/
void area_tree_del(area_tree_t *atree) {
  if (atree == NULL) {
    return;
  }
  nodepool_destroy(&atree->np);
  free(atree);
  atree = 0;
}

/**
 * @brief 设置分块树的分块粒度.

 * @date 2011/07/05
 **/
void area_tree_setConfig(area_tree_t *atree, area_config_t *cfg) {
  if (cfg) {
    atree->config = *cfg;
  } else {
    atree->config = AREA_CONFIG_INIT_VALUE;
  }
}

static html_area_t *createAreaNode(nodepool_t *np) {
  html_area_t *area = (html_area_t *) nodepool_get(np);
  if (area == NULL) {
    return NULL;
  }
  memset(area, 0, sizeof(html_area_t));
  area->isValid = true;
  return area;
}

/**
 * @brief 划分分块过程中保存的信息,相当于分块游标，主要记录一个正在形成的块的当前信息.
 */
typedef struct _dividing_t {
  area_tree_t *atree; /**< 所属的分块树*/
  html_vnode_t *begin; /**< 分块的开始节点  */
  html_vnode_t *end; /**< 分块的结束节点  */
  int xpos; /**< 当前块的X坐标  */
  int ypos; /**< 当前块的Y坐标  */
  int wx; /**< 当前块的宽度  */
  int hx; /**< 当前块的高度  */
  unsigned int depth; /**< 当前块的在分块树中的深度  */
  html_vnode_t * last_valid; /**< 上一个有效的节点  */
  html_vnode_t * last_validword; /**< 上一个有效且子树文字长度大于0的节点  */
  html_area_t *parent_area; /**< 父分块  */
  html_area_t **cur_tail; /**< 新块需要悬挂的指针  */
  html_area_t *prev_area; /**< 前一个兄弟分块  */
  nodepool_t *np; /**< 分块的节点池  */
  const area_config_t *config; /**< 对分块功能的外部配置 */
  bool wap_page; /**< 是否为wap页面 */
} dividing_t;

/**
 * @brief	清除分块过程中保存的信息.

 * @date 2011/07/05
 **/
static void dividing_collect_clr(dividing_t *divider) {
  divider->begin = NULL;
  divider->end = NULL;
  divider->xpos = -1;
  divider->ypos = -1;
  divider->wx = 0;
  divider->hx = 0;
  divider->last_valid = NULL;
  divider->last_validword = NULL;
}

/**
 * @brief 清空游标.

 * @date 2011/07/05
 **/
static void dividing_clr(dividing_t *divider) {
  dividing_collect_clr(divider);
  divider->depth = 0;
  divider->cur_tail = NULL;
  divider->prev_area = NULL;
  divider->wap_page = false;
}

/**
 * @brief 根据收集的信息创建分块节点.

 * @date 2011/07/05
 **/
static html_area_t *blockToAreaNode(dividing_t *divider) {
  html_area_t *aNode = createAreaNode(divider->np);
  if (aNode == NULL) {
    return NULL;
  }
  aNode->area_tree = divider->atree;
  aNode->begin = divider->begin;
  aNode->end = divider->end;
  aNode->area_info.xpos = divider->xpos;
  aNode->area_info.ypos = divider->ypos;
  aNode->area_info.width = divider->wx;
  aNode->area_info.height = divider->hx;
  aNode->depth = divider->depth;
  aNode->parentArea = divider->parent_area;
  aNode->prevArea = divider->prev_area;

  if (divider->last_valid) {
    aNode->isValid = true;
  } else {
    aNode->isValid = false;
  }

  // update parent area
  aNode->parentArea->subArea_num++;
  if (aNode->isValid) {
    aNode->parentArea->valid_subArea_num++;
  }

  // update the dividing structure
  *(divider->cur_tail) = aNode;
  divider->cur_tail = &(aNode->nextArea);
  divider->prev_area = aNode;
  return aNode;
}

#define SET_TO_RIGHT(x)	((x)|=0x1)
#define IS_TO_RIGHT(x)	((x)&0x1)
#define SET_TO_LEFT(x)	((x)|=0x2)
#define IS_TO_LEFT(x)	((x)&0x2)
#define SET_TO_UPPER(x)	((x)|=0x4)
#define IS_TO_UPPER(x)	((x)&0x4)
#define	SET_TO_LOWER(x)	((x)|=0x8)
#define	IS_TO_LOWER(x)	((x)&0x8)

/**
 * @brief 判断当前节点与前面节点的位置关系.

 * @date 2011/07/05
 **/
static unsigned int check_pos(html_vnode_t *vnode, dividing_t *divider) {
  int pos = 0;
  int xdiff = vnode->xpos - divider->xpos;
  if (xdiff >= divider->wx && xdiff > 0) {
    SET_TO_RIGHT(pos);
  } else if (xdiff <= 0 - vnode->wx && xdiff < 0) {
    SET_TO_LEFT(pos);
  }
  int ydiff = vnode->ypos - divider->ypos;
  if ((ydiff >= divider->hx || (divider->last_valid && divider->last_valid->hpNode->html_tag.tag_type == TAG_BR))
      && ydiff > 0) {
    SET_TO_LOWER(pos);
  } else if (ydiff <= 0 - vnode->hx && ydiff < 0) {
    SET_TO_UPPER(pos);
  }
  return pos;
}

#define	NO_FIT		0
#define FIT_LOWER	1
#define FIT_RIGHT	2
#define FIT_LEFT	3

#define AREA_TOBE_DIVIDE	0		    /**< 当前节点需要划分  */
#define AREA_NEW_BEGIN	1		  	 	/**< 以当前节点开始,创建新的分块  */
#define	AREA_END	2		  			/**< 以当前节点作为当前分块的结束  */
#define AREA_END_BOTH	3		  		/**< 之前收集的结点作为一个分块,当前节点成为一个分块  */
#define AREA_CONTINUE	4		  		/**< 继续收集节点  */

//观察者
typedef struct _observer_t {
  unsigned int fit_pos :16; /**< 相对位置  */
  unsigned int act :16; /**< 采取的分块行为  */
} observer_t;

/**
 * @brief 判断当前节点与前面收集的节点的位置适配关系.

 * @date 2011/07/05
 **/
static observer_t check_fit_prev(html_vnode_t *vnode, dividing_t *divider) {
  observer_t obsv = { 0, 0 };
  if (vnode->hpNode->html_tag.tag_type == TAG_BR) {
    if (divider->wap_page == true) {
      //当前节点是br，WAP不继续融合
      debuginfo(DIVIDING_AREA,
                "[depth:%d] node(id=%d)<%s> FIT_LOWER & AREA_NEW_BEGIN",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      obsv.fit_pos = FIT_LOWER;
      obsv.act = AREA_NEW_BEGIN;
    } else {
      //当前节点是br，WEB继续融合
      if (divider->last_valid && divider->last_valid->ypos <= vnode->ypos) {
        debuginfo(DIVIDING_AREA,
                  "[depth:%d] node(id=%d)<%s> FIT_LOWER & AREA_CONTINUE",
                  divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
        obsv.fit_pos = FIT_LOWER;
        obsv.act = AREA_CONTINUE;
      }

    }
    return obsv;
  }
  //如果当前分块还没有节点，那么与前面收集的节点不适配
  if (divider->begin == NULL) {
    obsv.fit_pos = NO_FIT;
    debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> NO_FIT",
              divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
    return obsv;
  }
  //查看节点与当前分块的相对位置（上、下、左、右）
  unsigned int pos = check_pos(vnode, divider);
  //当前节点在当前分块的下面
  if (IS_TO_LOWER(pos) && divider->last_valid) {
    //如果不是正下方，那么不适配
    if (IS_TO_LEFT(pos) || IS_TO_RIGHT(pos)) {
      obsv.fit_pos = NO_FIT;
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> NO_FIT",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return obsv;
    }
    int wxdiff = divider->wx - vnode->wx;
    //如果当前节点和当前分块的宽度之差小于当前分块宽度的30%或者两者之一的宽度为0的话，可以适配的当前分块的下面
    if (divider->last_valid
        && (IS_TEXT_VNODE(divider->last_valid->property)
            || divider->last_valid->hpNode->html_tag.tag_type == TAG_BR)
        && IS_TEXT_VNODE(vnode->property)) {
      obsv.fit_pos = FIT_LOWER;
      //obsv.act = AREA_CONTINUE;
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> act AREA_CONTINUE",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return obsv;
    }

    if (abs(wxdiff) * 100 <= divider->wx * 30 || 0 == divider->wx
        || 0 == vnode->wx) {
      obsv.fit_pos = FIT_LOWER;
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> FIT_LOWER",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return obsv;
    }
  }
  //同样的策略应对左、右两种情况
  if (IS_TO_RIGHT(pos)) {
    if ((IS_TO_LOWER(pos) || IS_TO_UPPER(pos))&&divider->last_valid) {
      obsv.fit_pos = NO_FIT;
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> NO_FIT",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return obsv;
    }
    int hxdiff = divider->hx - vnode->hx;
    if (vnode->hx < 40 && divider->hx < 40) {
      obsv.fit_pos = FIT_RIGHT;
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> FIT_RIGHT",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return obsv;
    } else if (abs(hxdiff) * 100 <= divider->hx * 30 || 0 == divider->hx
        || 0 == vnode->hx) {
      obsv.fit_pos = FIT_RIGHT;
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> FIT_RIGHT",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return obsv;
    }
  } else if (IS_TO_LEFT(pos)) {
    if ((IS_TO_LOWER(pos) || IS_TO_UPPER(pos))&&divider->last_valid) {
      obsv.fit_pos = NO_FIT;
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> NO_FIT",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return obsv;
    }
    int hxdiff = divider->hx - vnode->hx;
    if (abs(hxdiff) * 100 <= divider->hx * 30 || 0 == divider->hx
        || 0 == vnode->hx) {
      obsv.fit_pos = FIT_LEFT;
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> FIT_LEFT",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return obsv;
    }
  }
  //当前节点在当前分块的上面，无法适配
  obsv.fit_pos = NO_FIT;
  debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> NO_FIT", divider->depth,
            vnode->id, vnode->hpNode->html_tag.tag_name);
  return obsv;
}

/**
 * @brief 大小是否合适以成为一个单独的分块.

 * @date 2011/07/05
 **/
static bool is_suite_size(html_vnode_t *vnode, html_area_t *parentArea) {
  if (vnode->wx * 100 >= parentArea->area_info.width * 50) {
    if (vnode->hx >= SMALL_HX_VAL
        && vnode->hx * 100 >= parentArea->area_info.height * 10) {
      return true;
    }
  }

  if (vnode->hx * 100 >= parentArea->area_info.height * 50) {
    if (vnode->wx >= SMALL_HX_VAL
        && vnode->wx * 100 >= parentArea->area_info.width * 10) {
      return true;
    }
  }

  return false;
}

static inline bool is_link(html_vnode_t *vnode) {
  if (vnode->hpNode->html_tag.tag_type
      == TAG_A && get_attribute_value(&vnode->hpNode->html_tag,ATTR_HREF) != NULL) {return true;
}

return false;
}

  /**
   * @brief 这些TAG应该作为一个分块的整体存在.

   * @date 2011/07/05
   **/
static bool is_structure_tag(const html_tag_type_t tag_type) {
  if (tag_type == TAG_ROOT || tag_type == TAG_HTML || tag_type == TAG_BODY
      || tag_type == TAG_TABLE || tag_type == TAG_TBODY || tag_type == TAG_DIV
      || tag_type == TAG_TR || tag_type == TAG_TD || tag_type == TAG_TH
      || tag_type == TAG_COLGROUP || tag_type == TAG_CENTER
      || tag_type == TAG_FORM) {
    return true;
  }

  return false;
}

/**
 * @brief 这个节点是否应该在当前层拆分为更细的结构.

 * @date 2011/07/05
 **/
static bool can_vnode_split(html_vnode_t *vnode) {
  /**
   * 当前节点为结构型标签，且其子节点含有结构型标签，
   * 且含有高度较大的子节点，
   * 则可以拆分为更细的结构.
   */
  if (!is_structure_tag(vnode->hpNode->html_tag.tag_type)) {
    return false;
  }
  bool has_struct_tag = false;
  //判断该节点中的子节点是否含有结构型标签（结构型标签最好是作为一个分块的整体存在）
  for (html_vnode_t *child = vnode->firstChild; child; child =
      child->nextNode) {
    if (child->isValid && is_structure_tag(child->hpNode->html_tag.tag_type)) {
      has_struct_tag = true;
      break;
    }
  }
  //如果含有结构型标签，还需要分析子节点的其他属性：可视子节点个数小于10个，有高度大于当前节点40%的子节点存在
  if (has_struct_tag) {
    bool has_high_child = false;
    int valid_child_num = 0;
    for (html_vnode_t *child = vnode->firstChild; child; child =
        child->nextNode) {
      if (child->isValid) {
        if (child->hx * 100 >= vnode->hx * 40) {
          has_high_child = true;
        }
        if (++valid_child_num >= 10) {
          return false;
        }
      }
    }
    if (has_high_child) {
      return true;
    }
  }
  return false;
}

/*
 static int isMyposPosition(int page_height,int page_width,html_vnode_t *vnode){
 int header_value = 0;
 // it seems small page rule is not applicable after 100 equality sample tes
 if (page_height>=1200){
 //header_value = 600;//
 header_value = page_height*3/7;//shuangwei modify,根据	CASEPS-169
 }
 else if(page_height <= 300){
 header_value = page_height / 2;
 }
 else{
 header_value = page_height*3/5;
 }
 if (vnode->ypos<header_value){
 return (vnode->xpos <= page_width/2);
 }
 else{
 return 0;
 }
 }
 */

/**
 * @brief 是否主要为table
 * @date 2012-10-28
 * @author sue
 */
static bool is_main_table_vnode(html_vnode_t *vnode) {
  if (vnode->hpNode->html_tag.tag_type == TAG_TABLE) {
    return true;
  }
  if (!vnode->firstChild || vnode->firstChild->nextNode
      || vnode->firstChild->hpNode->html_tag.tag_type != TAG_TABLE) {
    return false;
  }
  return true;
}

/**
 * @brief 是否主要为列表
 * @date 2012-10-27
 * @author sue
 */
static bool is_main_ul_vnode(html_vnode_t *vnode) {
  if (vnode->hpNode->html_tag.tag_type == TAG_UL) {
    return true;
  }
  if (!vnode->firstChild || vnode->firstChild->nextNode
      || vnode->firstChild->hpNode->html_tag.tag_type != TAG_UL) {
    return false;
  }
  return true;
}

/**
 * @brief 判断节点是否符合<div><h[1-4]/><ul/></div>结构
 * @date 2012-10-27
 * @author sue
 */
static bool is_h_ul_vnode(html_vnode_t *vnode) {
  if (vnode->hpNode->html_tag.tag_type != TAG_DIV) {
    return false;
  }
  if (vnode->hx > 200 || vnode->wx > 800) {
    return false;
  }
  if (vnode->subtree_diff_font > 2) {
    return false;
  }
  if (!vnode->firstChild
      || !ahtml_small_header_map[vnode->firstChild->hpNode->html_tag.tag_type]) {
    return false;
  }
  if (!vnode->firstChild->nextNode
      || vnode->firstChild->nextNode->hpNode->html_tag.tag_type != TAG_UL) {
    return false;
  }
  return true;
}

/**
 *  @brief 是否包含某个节点
 */
static bool is_contain_tag(html_vnode_t *vnode, html_tag_type_t tag_type) {
  for (html_vnode_t *child = vnode->firstChild; child; child =
      child->nextNode) {
    if (!child->isValid) {
      continue;
    }
    if (child->hpNode->html_tag.tag_type == tag_type) {
      return true;
    }
    bool flag = is_contain_tag(child, tag_type);
    if (flag) {
      return flag;
    }
  }
  return false;
}

/**
 * @brief 当前节点是否需要划分，注意策略的优先级。一是尽量保持分块树的层次感，二是使叶子分块不至于分太细。

 * @date 2011/07/05
 **/
static bool is_need_divide(html_vnode_t *vnode, html_area_t *pArea,
                           bool is_divid_text, dividing_t *divider) {
  html_tag_type_t tag_type = vnode->hpNode->html_tag.tag_type;
  //叶子节点无法进行划分
  debuginfo(
      DIVIDING_AREA,
      "[depth:%d] node(id=%d)<%s> ,subnodetype=%x,areawidth=%d,areaheight=%d, divide",
      divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name,
      vnode->hpNode->subnodetype, pArea->area_info.width,
      pArea->area_info.height, __FILE__, __LINE__, __FUNCTION__);
  if (vnode->firstChild == NULL) {
    debuginfo(DIVIDING_AREA,
              "[depth:%d] node(id=%d)<%s> don't have child, not divide",
              divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
    return false;
  }
  if (vnode->struct_info->valid_child_num <= 2) {  //防止分块层次不好
    if (vnode->hpNode->subnodetype & (1 << 2)) {  //子树上含有H（1－6）标签，继续划分（防止realtitle分块粒度过大）
      debuginfo(
          DIVIDING_AREA,
          "[depth:%d] node(id=%d)<%s> ,subnodetype=%x,areawidth=%d,areaheight=%d, to be divide",
          divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name,
          vnode->hpNode->subnodetype, pArea->area_info.width,
          pArea->area_info.height, __FILE__, __LINE__, __FUNCTION__);
      return true;
    }
  }
  if (IS_BORDER(vnode->property) && vnode->subtree_border_num == 1
      && vnode->subtree_diff_font < 3) {  //具有border属性，且子树（包括自己）中只有自己具有border属性，且子树字体数目不超过2的不再继续划分 (sue)
    if (vnode->hx < 400 && vnode->wx < 800) {
      debuginfo(DIVIDING_AREA,
                "[depth:%d] node(id=%d)<%s> not divide for it has border",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return false;
    }
  }
  if (vnode->wx * 2 < vnode->vtree->root->wx
      && (is_h_ul_vnode(vnode) || is_main_ul_vnode(vnode))) {  //防止被分得太细了
    debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> not divide",
              divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
    return false;
  }
  if (vnode->subtree_textSize < 300 && is_main_table_vnode(vnode)
      && !is_contain_tag(vnode, TAG_TABLE) && divider->depth > 2
      && vnode->hx < 400) {  //防止table被分得太细了
    if (tag_type == TAG_TABLE) {
      if (vnode->subtree_diff_font > 1) {
        debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> divide",
                  divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
        return true;
      }
    }
    debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> not divide",
              divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
    return false;
  }
  //防止回复块分细了
  char *class_value = get_attribute_value(&vnode->hpNode->html_tag, ATTR_CLASS);
  char *id_value = get_attribute_value(&vnode->hpNode->html_tag, ATTR_ID);
  if (vnode->struct_info->interaction_tag_num > 1
      && (class_value != NULL || id_value != NULL)) {
    bool flag = false;
    if (class_value && strstr(class_value, "comment")) {
      flag = true;
    }
    if (!flag && id_value && strstr(id_value, "comment")) {
      flag = true;
    }
    if (flag) {
      vnode->type = TYPE_INTERACTION;
      debuginfo(
          DIVIDING_AREA,
          "[depth:%d] node(id=%d)<%s> not divide for it is interaction node",
          divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return false;
    }
  }
  if (divider->depth == 1 && vnode->hpNode->html_tag.tag_type == TAG_TABLE
      && vnode->upperNode
      && vnode->upperNode->hpNode->html_tag.tag_type == TAG_BODY) {
    if (vnode->hx < vnode->upperNode->hx - 20
        || vnode->wx < vnode->upperNode->wx - 20) {
      //第一层分块时，遇到table标签认为其是布局用的，不要分碎了
      debuginfo(DIVIDING_AREA,
                "[depth:%d] node(id=%d)<%s> is table of root, not divide",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return false;
    }
  }
  if (divider->depth > 2) {
    //如果可能是realtitle块，分块粒度则需要细一些
    if (vnode->ypos >= 50 && vnode->ypos <= 360 && vnode->hx < 250
        && vnode->hx >= 30 && vnode->wx >= 250) {
      if (vnode->hx < 50 && vnode->hpNode->html_tag.tag_type == TAG_TD
          && vnode->subtree_diff_font == 1) {
        debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> not divide",
                  divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
        return false;
      }
      bool flag = false;
      if (vnode->hpNode->html_tag.tag_type == TAG_P
          && vnode->subtree_diff_font <= 2) {
        flag = true;
      }
      if (!flag) {
        debuginfo(
            DIVIDING_AREA,
            "[depth:%d] node(id=%d)<%s> divide for it maybe realtitle area",
            divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
        return true;
      }
    }
    if (vnode->struct_info->hr_num > 0) {
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> divide",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return true;
    }
    if (vnode->hpNode->html_tag.tag_type == TAG_TABLE && divider->depth > 1
        && vnode->subtree_textSize > 400) {
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> divide",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return true;
    }
    if (vnode->subtree_diff_font > 1 && divider->depth > 1) {
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> divide",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return true;
    }
  }
  if (divider->depth == 1) {
    //第一层中间块防止粒度太大
    int page_width = vnode->vtree->root->wx;
    if (vnode->hx * 5 > page_width * 2 && vnode->wx > 900) {
      debuginfo(DIVIDING_AREA,
                "[depth:%d] node(id=%d)<%s> divide for it is too big",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return true;
    }
  }
  //防止<h1>节点分细了
  int child_num = 0;
  for (html_vnode_t *child = vnode->firstChild; child; child =
      child->nextNode) {
    child_num++;
  }
  if (vnode->hpNode->html_tag.tag_type == TAG_H1 && child_num <= 3
      && vnode->subtree_diff_font == 1) {
    debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> H1 not divide",
              divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
    return false;
  }
  //文本节点无需划分
  if (!is_divid_text && IS_TEXT_VNODE(vnode->property)) {
    debuginfo(DIVIDING_AREA,
              "[depth:%d] node(id=%d)<%s> is text node, not divide",
              divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
    return false;
  }
  //链接和会晃动的文字无需划分
  if (is_link(vnode) || vnode->hpNode->html_tag.tag_type == TAG_MARQUEE) {
    debuginfo(DIVIDING_AREA,
              "[depth:%d] node(id=%d)<%s> is marrquee node, not divide",
              divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
    return false;
  }
  //textarea节点无需划分
  if (vnode->hpNode->html_tag.tag_type == TAG_TEXTAREA) {
    debuginfo(DIVIDING_AREA,
              "[depth:%d] node(id=%d)<%s> is textarea node, not divide",
              divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
    return false;
  }
  //shuangwei add 20120605
  if ((vnode->hpNode->html_tag.tag_type == TAG_TABLE)
      && IS_DOMTREE_SUBTYPE(vnode->hpNode->owner->treeAttr)
      && !(vnode->hpNode->subnodetype & 31)) {
    int colNum = 0;
    html_vnode_t *ptdNode = vnode;
    if (ptdNode && ptdNode->firstChild
        && ptdNode->firstChild->hpNode->html_tag.tag_type == TAG_THEAD) {
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> not divide",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return false;
    }
    while (ptdNode && ptdNode->firstChild) {
      if (ptdNode->firstChild->hpNode->html_tag.tag_type == TAG_TD) {
        ptdNode = ptdNode->firstChild;
        break;
      }
      ptdNode = ptdNode->firstChild;
    }
    if (ptdNode) {
      char *pcolspan = html_node_get_attribute_value(ptdNode->hpNode,
                                                     "colspan");
      if (pcolspan
          && (((*pcolspan) > '1' && (*pcolspan) <= '9') || strlen(pcolspan) > 1)) {
        debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> not divide",
                  divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
        return false;
      }
    }
    while (ptdNode && ptdNode->hpNode->html_tag.tag_type == TAG_TD) {
      colNum++;
      ptdNode = ptdNode->nextNode;
    }
    if (colNum > 1) {
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> not divide",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return false;
    }
  }
  if ((vnode->hpNode->html_tag.tag_type == TAG_UL
      || vnode->hpNode->html_tag.tag_type == TAG_OL)
      && IS_DOMTREE_SUBTYPE(vnode->hpNode->owner->treeAttr)
      && !(vnode->hpNode->subnodetype & 15)) {
    debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> not divide",
              divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
    return false;
  }
  if (vnode->hpNode->html_tag.tag_type == TAG_DL
      && vnode->subtree_diff_font == 1) {
    debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> not divide",
              divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
    return false;
  }
  if ((vnode->hpNode->html_tag.tag_type == TAG_FORM) && vnode->wx < 400
      && vnode->hx < 400) {
    debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> not divide",
              divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
    return false;
  }
  if (vnode->subtree_max_font_size >= 20 && vnode->subtree_diff_font == 2) {
    char *class_value = get_attribute_value(&vnode->hpNode->html_tag,
                                            ATTR_CLASS);
    if (class_value && strstr(class_value, "title")) {
      debuginfo(DIVIDING_AREA,
                "[depth:%d] node(id=%d)<%s> divide for it maybe realtitle",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return true;
    }
  }
  if (vnode->hpNode->html_tag.tag_type == TAG_P) {
    if (vnode->subtree_max_font_size >= 20 && vnode->subtree_diff_font == 2) {
      debuginfo(DIVIDING_AREA,
                "[depth:%d] node(id=%d)<%s> divide for it maybe realtitle",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return true;
    }
    if (IS_DOMTREE_SUBTYPE(vnode->hpNode->owner->treeAttr)
        && !(vnode->hpNode->subnodetype & 31)
        && (vnode->hpNode->owner->treeAttr & 5) != 0) {
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> not divide",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return false;
    }
  }
  if (tag_type == TAG_TABLE) {
    if (divider->depth > 1 && vnode->subtree_diff_font > 1) {
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> divide",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return true;
    }
    if (vnode->wx <= 600 && vnode->hx <= 200 && vnode->prevNode
        && vnode->nextNode && (vnode->ypos > 350 || vnode->ypos < 80)) {
      debuginfo(DIVIDING_AREA,
                "[depth:%d] node(id=%d)<%s> is table, not divide",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return false;
    }
  }
  //节点的长宽和父分块一样大小，则该节点需要划分
  if (vnode->wx == pArea->area_info.width
      && vnode->hx == pArea->area_info.height
      && IS_DOMTREE_SUBTYPE(vnode->hpNode->owner->treeAttr)
      && (vnode->hpNode->subnodetype & 127)) {
    bool flag = false;  //是否豁免该策略

    html_tag_type_t tag_type = vnode->hpNode->html_tag.tag_type;
    if ((tag_type == TAG_TABLE || tag_type == TAG_DIV)
        && !(vnode->hpNode->subnodetype & 32) && vnode->hx < 500
        && vnode->wx < 500) {  //正文中会出现一些表格，不希望这样的表格被分细了。但对于布局用的表格，有的是需要继续划分的
      flag = true;
    }
    if (!flag) {
      debuginfo(DIVIDING_AREA,
                "[depth:%d] node(id=%d)<%s> is equal area, divide",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return true;
    }
  }
  //如果该节点下属的子树结构是一个重复结构，那么该节点无须划分
  unsigned int limit = 1;
  if (vnode->hx > 600) {
    limit = 2;
  }
  if (divider->depth > limit) {
    if (vnode->struct_info && vnode->struct_info->is_self_repeat
        && vnode->struct_info->self_similar_value > 90) {
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> not divide",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return false;
    }
  }
  //如果当前节点的长宽与当前分块的比例过大，或者节点本身面积过大
  if (vnode->wx * 100 > pArea->area_info.width * WX_PERCENT_BE_DIVIDE
      && vnode->hx * 100 > pArea->area_info.height * HX_PERCENT_BE_DIVIDE
      && vnode->wx >= 400 && vnode->hx >= 200 && can_vnode_split(vnode)) {
    if (vnode->hpNode->html_tag.tag_type != TAG_TABLE) {
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> divide",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return true;
    }
  }
  if (vnode->hx < 800) {
    if (vnode->subtree_diff_font > 1 && divider->depth > 1
        && !vnode->struct_info->is_self_repeat
        && !vnode->struct_info->is_repeat_with_sibling) {
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> divide",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return true;
    }
  }
  if (vnode->hx < 60 && vnode->wx > 600 && vnode->subtree_diff_font > 1) {
    debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> divide",
              divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
    return true;
  }
//	if (vnode->struct_info->valid_child_num > 1 && vnode->hx > 30)
//	{
//		debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> divide", divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
//		return true;
//	}
//	if(divider->depth == 2 && vnode->hx > 600 && vnode->wx > 250)
//	{
//		debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> divide", divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
//		return true;
//	}

  //如果是wap网页，只要高度够高，就需要继续划分(sue)
  if (divider->wap_page == true) {
    if (vnode->hx > 30) {
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> divide",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return true;
    }
  }

  debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> not divide by default",
            divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
  return false;
}

/**
 * @brief	判断分块是否由文本节点构成;false:该块不是文本块，含有块型节点

 * @date 2011/07/05
 **/
static bool is_divide_text_block(html_area_t *area) {
  html_vnode_t *vnode = area->begin;
  if (vnode->hpNode->html_tag.tag_type == TAG_BR) {
    vnode = vnode->nextNode;
  }
  //遍历该分块中的可视节点，一旦出现非文本的而且不是BR节点时，就可以认定该分块不是由文本节点构成
  for (; vnode; vnode = vnode->nextNode) {
    if (vnode->isValid) {
      if (!IS_TEXT_VNODE(vnode->property)
          && (vnode != area->end || vnode->hpNode->html_tag.tag_type != TAG_BR))
        return false;
    }
    if (vnode == area->end) {
      break;
    }
  }
  return true;
}

static bool is_divisible_by_config(html_vnode_t *vnode,
                                   const area_config_t *config) {
  if (config && config->indivisible_tag_name && vnode->hpNode->html_tag.tag_name
      && strcasecmp(vnode->hpNode->html_tag.tag_name,
                    config->indivisible_tag_name) == 0) {
    return false;
  }

  return true;
}

/**
 * @brief 根据当前节点判断将采取的行动.

 * @date 2011/07/05
 **/
static observer_t check_area(html_vnode_t *vnode, dividing_t *divider) {
  observer_t obsv = { 0, 0 };
  html_area_t *pArea = divider->parent_area;
  bool is_divid_text = is_divide_text_block(pArea);
  //判断该节点是否可以被划分
  if (is_divisible_by_config(vnode, divider->config)
      && is_need_divide(vnode, pArea, is_divid_text, divider)) {
    obsv.fit_pos = NO_FIT;
    obsv.act = AREA_TOBE_DIVIDE;
    return obsv;
  }
  //如果是root节点(sue)
  html_tag_type_t tag_type = vnode->hpNode->html_tag.tag_type;
  if (tag_type == TAG_ROOT || tag_type == TAG_HTML || tag_type == TAG_BODY
      || tag_type == TAG_WAP_CARD) {
    obsv.fit_pos = NO_FIT;
    obsv.act = AREA_TOBE_DIVIDE;
    return obsv;
  }
  //判断当前节点与当前分块的适配程度
  obsv = check_fit_prev(vnode, divider);
  if (obsv.act != 0) {
    return obsv;
  }
  obsv.act = 0;
  if (divider->last_valid) {
    //相邻两个节点，具有不同的class值时，不被分到同一个分块中
    char *class_value = get_attribute_value(&vnode->hpNode->html_tag,
                                            ATTR_CLASS);
    char *last_class_value = get_attribute_value(
        &divider->last_valid->hpNode->html_tag, ATTR_CLASS);
    if (class_value && last_class_value
        && strcmp(class_value, last_class_value) != 0) {
      obsv.act = AREA_NEW_BEGIN;
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> act AREA_NEW_BEGIN",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return obsv;
    }
  }
  if (vnode->hpNode->html_tag.tag_type == TAG_H1) {
    obsv.act = AREA_END_BOTH;
    debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> act AREA_END_BOTH",
              divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
    return obsv;
  }
  if (divider->depth == 1 && vnode->hpNode->html_tag.tag_type == TAG_TABLE) {
    obsv.act = AREA_NEW_BEGIN;
    debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> act AREA_NEW_BEGIN",
              divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
    return obsv;
  }
  unsigned int depth_limit = 1;
  if (vnode->hx > 600) {
    depth_limit = 2;
  }
  if (divider->depth > depth_limit && vnode->hpNode->html_tag.tag_type != TAG_P
      && vnode->struct_info->is_repeat_with_sibling && vnode->hx < 200) {
    if (!divider->last_valid
        || divider->last_valid->hpNode->html_tag.tag_type
            == vnode->hpNode->html_tag.tag_type) {
      if (divider->last_valid && vnode->hx >= 20
          && vnode->hx != divider->last_valid->hx) {
        obsv.act = AREA_NEW_BEGIN;
        debuginfo(DIVIDING_AREA,
                  "[depth:%d] node(id=%d)<%s> act AREA_NEW_BEGIN",
                  divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
        return obsv;
      }
      if (divider->last_valid
          && vnode->subtree_max_font_size
              != divider->last_valid->subtree_max_font_size) {
        obsv.act = AREA_NEW_BEGIN;
        debuginfo(DIVIDING_AREA,
                  "[depth:%d] node(id=%d)<%s> act AREA_NEW_BEGIN",
                  divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
        return obsv;
      }
      obsv.act = AREA_CONTINUE;
      debuginfo(
          DIVIDING_AREA,
          "[depth:%d] node(id=%d)<%s> act AREA_CONTINUE for it is repeat with sibling",
          divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return obsv;
    }
  }
  //如果当前分块的父分块不是由文本节点构成，而且当前节点不是文本节点
  if (!is_divid_text && IS_TEXT_VNODE(vnode->property)) {
    //当前节点的上一个可视节点是文本节点，则当前节点融入当前分块中，继续收集节点
    if (divider->last_valid) {
      if (vnode->hpNode->html_tag.tag_type == TAG_FONT) {
        if (divider->last_valid->depth == vnode->depth) {
          if (divider->last_valid->subtree_max_font_size
              != vnode->subtree_max_font_size) {
            obsv.act = AREA_NEW_BEGIN;
            debuginfo(DIVIDING_AREA,
                      "[depth:%d] node(id=%d)<%s> act AREA_NEW_BEGIN",
                      divider->depth, vnode->id,
                      vnode->hpNode->html_tag.tag_name);
            return obsv;
          }
        }
      }
      if (IS_TEXT_VNODE(divider->last_valid->property)) {
        obsv.act = AREA_CONTINUE;
        debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> act AREA_CONTINUE",
                  divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
        return obsv;
      }
    } else {  //如果当前分块还没有一个可视节点，那么当前节点作为一个新分块的开始
      obsv.act = AREA_NEW_BEGIN;
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> act AREA_NEW_BEGIN",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return obsv;
    }
  }
  if (divider->last_valid && is_divid_text
      && (IS_TEXT_VNODE(divider->last_valid->property)
          || divider->last_valid->hpNode->html_tag.tag_type == TAG_BR)
      && IS_TEXT_VNODE(vnode->property)) {
    obsv.act = AREA_CONTINUE;
    debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> act AREA_CONTINUE",
              divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
    return obsv;
  }
  //讨论适配情况
  switch (obsv.fit_pos) {
    case NO_FIT:
      if (vnode->subtree_textSize == 0
          && vnode->hpNode->html_tag.tag_type != TAG_IMG
          && vnode->hpNode->html_tag.tag_type != TAG_DIV) {
        debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> act AREA_CONTINUE",
                  divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
        obsv.act = AREA_CONTINUE;
        return obsv;
      }
      //如果不适配，那么如果当前节点适合作为一个单独分块的话，形成单独分块，否则作为一个新分块的开始
      if (vnode->nextNode
          && vnode->nextNode->hpNode->html_tag.tag_type == TAG_BR) {
        obsv.act = AREA_NEW_BEGIN;
        debuginfo(DIVIDING_AREA,
                  "[depth:%d] node(id=%d)<%s> act AREA_NEW_BEGIN",
                  divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      } else if (is_suite_size(vnode, pArea)
          && (divider->last_valid || IS_BLOCK_TAG(vnode->property))
          && vnode->hpNode->html_tag.tag_type != TAG_LI) {
        obsv.act = AREA_END_BOTH;
        debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> act AREA_END_BOTH",
                  divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      } else {
        obsv.act = AREA_NEW_BEGIN;
        debuginfo(DIVIDING_AREA,
                  "[depth:%d] node(id=%d)<%s> act AREA_NEW_BEGIN",
                  divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      }
      return obsv;
    case FIT_LOWER:
      if (divider->last_valid) {
        //子树字体数不同的块不进行融合
        if (divider->last_valid->subtree_max_font_size
            != vnode->subtree_max_font_size) {
          debuginfo(DIVIDING_AREA,
                    "[depth:%d] node(id=%d)<%s> act AREA_CONTINUE",
                    divider->depth, vnode->id,
                    vnode->hpNode->html_tag.tag_name);
          obsv.act = AREA_NEW_BEGIN;
          return obsv;
        }
        //子树文字长度为0的继续融合
        if (vnode->subtree_textSize == 0
            && vnode->hpNode->html_tag.tag_type != TAG_IMG) {
          debuginfo(DIVIDING_AREA,
                    "[depth:%d] node(id=%d)<%s> act AREA_CONTINUE",
                    divider->depth, vnode->id,
                    vnode->hpNode->html_tag.tag_name);
          obsv.act = AREA_CONTINUE;
          return obsv;
        }
        //对交互属性不同的块不进行融合
        bool last_inter = IS_INCLUDE_INTER(divider->last_valid->property);
        bool this_inter = IS_INCLUDE_INTER(vnode->property);
        if (last_inter != this_inter) {
          debuginfo(DIVIDING_AREA,
                    "[depth:%d] node(id=%d)<%s> act AREA_CONTINUE",
                    divider->depth, vnode->id,
                    vnode->hpNode->html_tag.tag_name);
          obsv.act = AREA_NEW_BEGIN;
          return obsv;
        }
        //把多行链接分到一起
        if (divider->last_validword) {
          if (vnode->hx < 200
              && vnode->hpNode->html_tag.tag_type
                  == divider->last_validword->hpNode->html_tag.tag_type) {
            if (vnode->firstChild && !vnode->firstChild->nextNode
                && divider->last_validword->firstChild
                && !divider->last_validword->firstChild->nextNode) {
              if (vnode->firstChild->hpNode->html_tag.tag_type
                  == divider->last_validword->firstChild->hpNode->html_tag
                      .tag_type) {
                debuginfo(DIVIDING_AREA,
                          "[depth:%d] node(id=%d)<%s> act AREA_CONTINUE",
                          divider->depth, vnode->id,
                          vnode->hpNode->html_tag.tag_name);
                obsv.act = AREA_CONTINUE;
                return obsv;
              }
            }
          }
        }
      }
      //如果当前节点在当前分块的下方，如果高度满足条件的话，之前收集的节点作为一个分块，当前节点作为一个分块，否则当前分块继续融合
      if (vnode->hx >= SMALL_HX_VAL
          && vnode->hpNode->html_tag.tag_type != TAG_BR
          && vnode->hpNode->html_tag.tag_type != TAG_PURETEXT) {
        obsv.act = AREA_END_BOTH;
        debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> act AREA_END_BOTH",
                  divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
        return obsv;
      }
      obsv.act = AREA_CONTINUE;
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> act AREA_CONTINUE",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return obsv;
    case FIT_RIGHT:
      if (vnode->hpNode->html_tag.tag_type != TAG_LI) {
        if (vnode->wx >= SMALL_WX_VAL && (IS_BLOCK_TAG(vnode->property))) {
          obsv.act = AREA_END_BOTH;
          debuginfo(DIVIDING_AREA,
                    "[depth:%d] node(id=%d)<%s> act AREA_END_BOTH",
                    divider->depth, vnode->id,
                    vnode->hpNode->html_tag.tag_name);
          return obsv;
        }
        if (vnode->wx >= SMALL_WX_VAL
            && (vnode->hpNode->html_tag.tag_type == TAG_BR)) {
          obsv.act = AREA_END;
          debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> act AREA_END",
                    divider->depth, vnode->id,
                    vnode->hpNode->html_tag.tag_name);
          return obsv;
        }
      }
      obsv.act = AREA_CONTINUE;
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> act AREA_CONTINUE",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return obsv;
    case FIT_LEFT:
      if (vnode->wx >= SMALL_WX_VAL && (IS_BLOCK_TAG(vnode->property))) {
        obsv.act = AREA_END_BOTH;
        debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> act AREA_END_BOTH",
                  divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
        return obsv;
      }
      obsv.act = AREA_CONTINUE;
      debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> act AREA_CONTINUE",
                divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
      return obsv;
    default:
      assert(0);
      break;
  }
  return obsv;
}

/**
 * @brief 当前节点挂在已有块的下边.主要是更新当前块的xpos与ypos

 * @date 2011/07/05
 **/
static void block_append_lower(dividing_t *divider, html_vnode_t *vnode) {
  divider->end = vnode;
  divider->hx = vnode->ypos + vnode->hx - divider->ypos;
  int right_xpos = divider->xpos + divider->wx;
  if (right_xpos < vnode->xpos + vnode->wx) {
    right_xpos = vnode->xpos + vnode->wx;
  }
  if (vnode->xpos < divider->xpos) {
    divider->xpos = vnode->xpos;
  }
  divider->wx = right_xpos - divider->xpos;
}

/**
 * @brief 当前节点挂在已有块的左边.主要是更新当前块的xpos与ypos

 * @date 2011/07/05
 **/
static void block_append_left(dividing_t *divider, html_vnode_t *vnode) {
  divider->end = vnode;
  divider->wx = divider->xpos + divider->wx - vnode->xpos;
  int bot_ypos = divider->ypos + divider->hx;
  if (bot_ypos < vnode->ypos + vnode->hx)
    bot_ypos = vnode->ypos + vnode->hx;
  if (vnode->ypos < divider->ypos)
    divider->ypos = vnode->ypos;
  divider->hx = bot_ypos - divider->ypos;
  divider->xpos = vnode->xpos;
}

/**
 * @brief	当前节点挂在已有块的右边.主要是更新当前块的xpos与ypos

 * @date 2011/07/05
 **/
static void block_append_right(dividing_t *divider, html_vnode_t *vnode) {
  divider->end = vnode;
  divider->wx = vnode->xpos + vnode->wx - divider->xpos;
  int bot_ypos = divider->ypos + divider->hx;
  if (bot_ypos < vnode->ypos + vnode->hx)
    bot_ypos = vnode->ypos + vnode->hx;
  if (vnode->ypos < divider->ypos)
    divider->ypos = vnode->ypos;
  divider->hx = bot_ypos - divider->ypos;
}

/**
 * @brief 当前节点与已有块在位置上不适配.

 * @date 2011/07/05
 **/
static void block_append_nofit(dividing_t *divider, html_vnode_t *vnode) {
  divider->end = vnode;
  int bot_ypos = divider->ypos + divider->hx;
  int right_xpos = divider->xpos + divider->wx;
  if (vnode->xpos < divider->xpos) {
    divider->xpos = vnode->xpos;
  }
  if (vnode->ypos < divider->ypos) {
    divider->ypos = vnode->ypos;
  }
  if (bot_ypos < vnode->ypos + vnode->hx) {
    bot_ypos = vnode->ypos + vnode->hx;
  }
  divider->hx = bot_ypos - divider->ypos;
  if (right_xpos < vnode->xpos + vnode->wx) {
    right_xpos = vnode->xpos + vnode->wx;
  }
  divider->wx = right_xpos - divider->xpos;
}

/**
 * @brief 当前节点挂在已有分块上.

 * @date 2011/07/05
 **/
static void block_append(dividing_t *divider, html_vnode_t *vnode,
                         unsigned int fit_pos) {
  //该分块目前为空，则填充开始和结束节点
  if (divider->begin == NULL) {
    divider->begin = vnode;
    divider->end = vnode;
  }
  //如果目前还没有可视节点，则填充分块的长宽等信息，并且不存在外挂的概念。
  if (divider->last_valid == NULL) {
    divider->xpos = vnode->xpos;
    divider->ypos = vnode->ypos;
    divider->wx = vnode->wx;
    divider->hx = vnode->hx;
    divider->last_valid = vnode;
    if (vnode->subtree_textSize > 0) {
      divider->last_validword = vnode;
    }
    return;
  }
  divider->last_valid = vnode;
  if (vnode->subtree_textSize > 0) {
    divider->last_validword = vnode;
  }
  //更新当前块的xpos和ypos
  switch (fit_pos) {
    case FIT_LOWER:
      block_append_lower(divider, vnode);
      break;
    case FIT_LEFT:
      block_append_left(divider, vnode);
      break;
    case FIT_RIGHT:
      block_append_right(divider, vnode);
      break;
    case NO_FIT:
      block_append_nofit(divider, vnode);
      break;
    default:
      ;
  }
}

/**
 * @brief 是否太小的分块,将忽略.

 * @date 2011/07/05
 **/
static bool is_little_block(dividing_t *divider) {
  return false;
}

/**
 * @brief 是否对父分块啥也没分出来.

 * @date 2011/07/05
 **/
static bool is_divide_nothing(html_vnode_t *tail_vnode,
                              html_area_t *parentArea) {
  if (parentArea->valid_subArea_num == 0 && parentArea->end == tail_vnode) {
    return true;
  }
  return false;
}

/**
 * @brief 根据已收集的分块创建分块节点.并清除已收集的分块.

 * @date 2011/07/05
 **/
static int old_collect_pass_away(dividing_t *divider) {
  if (divider->begin != NULL && !is_little_block(divider)
      && !is_divide_nothing(divider->end, divider->parent_area)) {
    html_area_t *aNode = blockToAreaNode(divider);
    if (aNode == NULL) {
      return -1;
    }
  }
  dividing_collect_clr(divider);
  return 1;
}

/**
 * @brief 收集无效节点.

 * @date 2011/07/05
 static void collect_invalid_vnode(dividing_t *divider, html_vnode_t *vnode)
 {
 if(divider->begin == NULL){
 divider->begin = vnode;
 }
 divider->end = vnode;
 }
 **/

/**
 * @brief	访问VTREE节点并分块.

 * @date 2011/07/05
 **/
static int visit_for_divide(html_vnode_t *vnode, void *result) {
  dividing_t *divider = (dividing_t *) result;
  if (!vnode->isValid) {
    return VISIT_SKIP_CHILD;
  }
  //确定分块的决策，该步确定当前节点与当前分块到底是形成一个分块还是怎么滴。。。
  observer_t obsv = check_area(vnode, divider);
  switch (obsv.act) {
    case AREA_CONTINUE: {
      //继续融合
      block_append(divider, vnode, obsv.fit_pos);
      return VISIT_SKIP_CHILD;
    }
    case AREA_NEW_BEGIN: {
      //新产生一个分块，结束旧分块，开始新分块
      if (old_collect_pass_away(divider) == -1) {
        goto FAIL;
      }
      block_append(divider, vnode, obsv.fit_pos);
      return VISIT_SKIP_CHILD;
    }
    case AREA_END_BOTH: {
      //之前收集的结点作为一个分块,当前节点成为一个分块
      if (old_collect_pass_away(divider) == -1) {
        goto FAIL;
      }
      if (!is_divide_nothing(vnode, divider->parent_area)) {  //don't do worthless divide
        block_append(divider, vnode, obsv.fit_pos);
        html_area_t *aNode = blockToAreaNode(divider);
        if (aNode == NULL) {
          goto FAIL;
        }
      }
      dividing_collect_clr(divider);
      return VISIT_SKIP_CHILD;
    }
    case AREA_END: {
      //以当前节点作为当前分块的结束
      if (!is_divide_nothing(vnode, divider->parent_area)) {  //don't do worthless divide
        block_append(divider, vnode, obsv.fit_pos);
        html_area_t *aNode = blockToAreaNode(divider);
        if (aNode == NULL) {
          goto FAIL;
        }
      }
      dividing_collect_clr(divider);
      return VISIT_SKIP_CHILD;
    }
    case AREA_TOBE_DIVIDE: {
      //当前节点需要划分
      if (old_collect_pass_away(divider) == -1) {
        goto FAIL;
      }
      return VISIT_NORMAL;
    }
    default:
      assert(0);
      break;
  }
  FAIL: return VISIT_ERROR;
}

/**
 * @brief	访问完一棵子树时,已收集的分块要么创建一个新的分块节点,要么丢掉.
 *

 * @date 2011/07/05
 **/
static int end_visit_for_divide(html_vnode_t *vnode, void *result) {
  if (!vnode->isValid) {
    return VISIT_NORMAL;
  }
  dividing_t *divider = (dividing_t *) result;
  if (divider->end == vnode) {/**并非从子树返回*/
    return VISIT_NORMAL;
  }
  if (old_collect_pass_away(divider) == -1) {
    return VISIT_ERROR;
  }
  debuginfo(DIVIDING_AREA, "[depth:%d] node(id=%d)<%s> old_collect_pass_away",
            divider->depth, vnode->id, vnode->hpNode->html_tag.tag_name);
  return VISIT_NORMAL;
}

/**
 * @brief 丢掉只有一个有效分块的分块层.

 * @date 2011/07/05
 **/
static void cut_single_area_level(html_area_t *area, nodepool_t *np) {
  html_area_t *pArea = area->parentArea;
  html_area_t *validArea = area;
//	for(;!validArea->isValid;validArea=validArea->nextArea);
  html_area_t *prevArea = validArea->prevArea;
  html_area_t *nextArea = validArea->nextArea;

  if (validArea->subArea == NULL) {
    html_area_t *next = NULL;
    for (html_area_t *a = pArea->subArea; a; a = next) {
      next = a->nextArea;
      nodepool_put(np, a);
    }
    pArea->subArea = NULL;
    pArea->subArea_num = 0;
    pArea->valid_subArea_num = 0;
  } else {
    if (prevArea) {
      prevArea->nextArea = validArea->subArea;
      validArea->subArea->prevArea = prevArea;
    }
    html_area_t *lastSubArea = NULL;
    // hang subArea on its parent
    for (html_area_t *subArea = validArea->subArea; subArea;
        subArea = subArea->nextArea) {
      subArea->parentArea = pArea;
      lastSubArea = subArea;
    }
    if (nextArea) {
      lastSubArea->nextArea = nextArea;
      nextArea->prevArea = lastSubArea;
    }
    // update parent area
    pArea->subArea_num += validArea->subArea_num - 1;
    pArea->valid_subArea_num = validArea->valid_subArea_num;
    if (prevArea == NULL) {
      pArea->subArea = validArea->subArea;
    }
    nodepool_put(np, validArea);
  }
}

/**
 * @brief 丢掉无效分块，或者说，坏分块.

 * @date 2011/07/05
 **/
static void cut_invalid_level(html_area_t *area, nodepool_t *np) {
  html_area_t *pArea = area->parentArea;
  html_area_t *next = NULL;
  for (html_area_t *a = pArea->subArea; a; a = next) {
    next = a->nextArea;
    nodepool_put(np, a);
  }
  pArea->subArea = NULL;
  pArea->subArea_num = 0;
  pArea->valid_subArea_num = 0;
}

/**
 * @brief 当前分块是否在分块粒度配置的范围内.

 * @date 2011/07/05
 **/
static inline bool is_in_limit(const html_area_t *area, unsigned int depth,
                               const area_config_t *cfg) {
  int awx = area->area_info.width;
  int ahx = area->area_info.height;
  if (awx < cfg->min_width) {
    return false;
  }
  if (ahx < cfg->min_height) {
    return false;
  }
  if (depth > cfg->max_depth) {
    return false;
  }
  if (awx < BIG_SHORT_INT && ahx < BIG_SHORT_INT && awx * ahx < cfg->min_size) {
    return false;
  }
  return true;
}

/**
 * @brief 当前分块是否只有一个有效子分块，并且不是第一层分块，这种情况出现，说明该层分块根本没起到作用

 * @date 2011/07/05
 **/
static bool is_to_cut_valid_level(html_area_t *area) {
  if (area->valid_subArea_num == 1 && area->depth >= 1) {
    return true;
  }
  return false;
}
/**
 * @brief 校准area的size.

 * @date 2011/07/05
 **/
static void adjust_area_size(html_area_t *area) {
  int most_left_x = area->area_info.xpos + area->area_info.width;
  int most_right_x = area->area_info.xpos;

  for (html_area_t *subarea = area->subArea; subarea;
      subarea = subarea->nextArea) {
    if (!subarea->isValid) {
      continue;
    }
    int leftsidex = subarea->area_info.xpos;
    int rightsidex = subarea->area_info.xpos + subarea->area_info.width;

    if (leftsidex < most_left_x) {
      most_left_x = leftsidex;
    }

    if (rightsidex > most_right_x) {
      most_right_x = rightsidex;
    }
  }
  int sub_xspan = most_right_x - most_left_x;
  if (sub_xspan > 0 && sub_xspan < area->area_info.width) {
    area->area_info.width = sub_xspan;
  }
}

/**
 * @brief 划分一个分块.
 * @param [in] area   :  html_area_t*	待划分的分块.
 * @param [in] cfg   : const area_config_t*	分块的粒度配置.
 * @param [in/out] np   : nodepool_t*	分块节点池.
 * @param [in] depth   : unsigned int	待划分的分块的深度.
 * @return  int 
 * @retval   -1:分块出错.1:成功.

 * @date 2011/07/05
 **/
int areaNode_divide(html_area_t *area, const area_config_t *cfg, nodepool_t *np,
                    unsigned int depth) {
  /*创建游标，并进行初始化*/
  dividing_t divider;
  dividing_clr(&divider);
  divider.atree = area->area_tree;
  divider.depth = depth;
  divider.parent_area = area;
  divider.cur_tail = &(area->subArea);
  divider.np = np;
  divider.config = cfg;
  int doc_type = area->begin->vtree->hpTree->doctype;
  if (doc_type == 1 || doc_type == 2)
    divider.wap_page = true;

  unsigned int new_depth = 0;
  //该字段标识某个分层是否需要去掉
  bool is_cut_vlevel = false;
  //判断当前分块是否还有必要再分：主要是宽度、高度、深度几个方面
  if (cfg != NULL && !is_in_limit(area, depth, cfg)) {
    return 1;
  }
  if (area->begin != area->end && depth > 3) {
    debuginfo(DIVIDING_AREA, "[depth:%d] area(node id=%d) not divide",
              area->depth, area->begin->id);
    return 1;
  }

  //如果只有一个有文字的叶子节点，不再划分
  int for_partition_area_num = 0;
  html_vnode_t *for_partition_area = NULL;
  for (html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode) {
    if (vnode->textSize > 0) {
      for_partition_area_num++;
      for_partition_area = vnode;
    }
    if (vnode == area->end) {
      break;
    }
  }
  if (for_partition_area_num <= 1) {
    if (for_partition_area
        && for_partition_area->struct_info->valid_leaf_num == 1) {
      debuginfo(DIVIDING_AREA, "[depth:%d] area(node id=%d) not divide",
                area->depth, area->begin->id);
      return 1;
    }
  }

  if (area->begin == area->end) {
    html_vnode_t *vnode = area->begin;
    char *class_value = get_attribute_value(&vnode->hpNode->html_tag,
                                            ATTR_CLASS);
    char *id_value = get_attribute_value(&vnode->hpNode->html_tag, ATTR_ID);
    int hx = vnode->hx;
    int wx = vnode->wx;
    if (class_value
        && (strstr(class_value, "copyright") || strstr(class_value, "footer")
            || strstr(class_value, "discuss"))) {  //可能是版权的分块不再细分
      debuginfo(DIVIDING_AREA, "[depth:%d] area(node id=%d) not divide",
                area->depth, area->begin->id);
      return 1;
    }
    if (depth > 2) {
      if (hx <= 50 && (class_value != NULL || id_value != NULL)) {  //高度在一定范围内，且具有class或id属性的不再继续划分
        debuginfo(DIVIDING_AREA, "[depth:%d] area(node id=%d) not divide",
                  area->depth, area->begin->id);
        return 1;
      }
    }
    if (depth > 2) {
      if (vnode->struct_info->is_self_repeat
          && vnode->struct_info->self_similar_value >= 96) {  //重复父结构不再继续划分
        debuginfo(
            DIVIDING_AREA,
            "[depth:%d] area(node id=%d) not divide for it is self repeat area",
            area->depth, area->begin->id);
        return 1;
      }
    }
    unsigned int subtree_node_num = 0;
    for (html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode) {
      subtree_node_num += vnode->struct_info->valid_node_num;
      if (vnode == area->end) {
        break;
      }
    }
    if (!(vnode->hpNode->subnodetype && (1 << 5))) {  //不包含form节点
      if (vnode->hpNode->html_tag.tag_type == TAG_TABLE && wx >= 700 && hx <= 40
          && subtree_node_num < 100) {  //长条形状的table节点，且不包含form节点的，不再细分 (sue)
        debuginfo(
            DIVIDING_AREA,
            "[depth:%d] area(node id=%d) not divide for it is like navigation",
            area->depth, area->begin->id);
        return 1;
      }
    }
  }
  // 访问vtree获取分块
  for (html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode) {
    int ret = html_vnode_visit(vnode, visit_for_divide, end_visit_for_divide,
                               &divider);
    if (ret == VISIT_ERROR) {
      goto FAIL;
    }
    if (vnode == area->end) {
      if (area->valid_subArea_num > 0
          && old_collect_pass_away(&divider) == -1) {
        goto FAIL;
      }
      break;
    }
  }
  new_depth = depth + 1;
  is_cut_vlevel = is_to_cut_valid_level(area);
  //本层有效分块少于1个，将丢弃
  if (is_cut_vlevel
      || (area->valid_subArea_num == 0 && area->subArea_num > 0)) {
    new_depth--;
  }
  // 继续划分子分块
  for (html_area_t *subArea = area->subArea; subArea;
      subArea = subArea->nextArea) {
    if (!subArea->isValid) {
      continue;
    }
    int ret = areaNode_divide(subArea, cfg, np, new_depth);
    if (ret == VISIT_ERROR) {
      goto FAIL;
    }
  }
  if (is_cut_vlevel) {
    //丢弃只有一个有效分块的层
    //由于试图让第一层分块展示网页结构,对第一层分块不进行丢弃.
    cut_single_area_level(area->subArea, np);
  } else if (area->valid_subArea_num == 0 && area->subArea_num > 0) {  //没有有效分块的层
    cut_invalid_level(area->subArea, np);
  }
  if (area->depth == 0) {
    adjust_area_size(area);
  }
  return 1;

  FAIL: return -1;
}

/**
 * @brief 获取根分块,即页面的真正的宽度.

 * @date 2011/07/05
 **/
static int real_page_width(const html_area_t *root) {
  if (root->subArea == NULL) {
    return root->area_info.width;
  }
  int wx = 0;
  for (html_area_t *a = root->subArea; a; a = a->nextArea) {
    int tmp = a->area_info.xpos + a->area_info.width;
    if (tmp > wx) {
      wx = tmp;
    }
  }
  return wx;
}

static int visit_for_add_no(html_area_t *area, void *data) {
  unsigned int *no = (unsigned int *) data;
  area->no = (*no)++;
  if (area->isValid
      && area->depth >= (unsigned int) area->area_tree->max_depth) {
    area->area_tree->max_depth = area->depth;
  }
  return AREA_VISIT_NORMAL;
}

static int visit_vnode_for_add_vnode2area(html_vnode_t *vnode, void *data) {
  /**
   * 对一个area内部遍历到的vnode填充其对应的最小分块。
   * 若已被填充，说明它已对应到更小的分块，因此跳过对其子树的遍历。
   */
  html_area_t *area = (html_area_t *) data;
  if (vnode->hp_area == NULL) {
    vnode->hp_area = area;
    //SET_LEAF_AREA(area->areaattr);
  } else {
    return VISIT_SKIP_CHILD;
  }
  return AREA_VISIT_NORMAL;
}

static int finish_visit_for_add_vnode2area(html_area_t *area, void *data) {
  for (html_vnode_t *vnode = area->begin;; vnode = vnode->nextNode) {
    if (IS_DOMTREE_SUBTYPE(area->area_tree->hp_vtree->hpTree->treeAttr)) {
      switch (vnode->hpNode->html_tag.tag_type) {
        case TAG_P:
          MARK_DOMTREE_P_TAG(area->nodeTypeOfArea);
          break;
        case TAG_DIV:
          MARK_DOMTREE_DIV_TAG(area->nodeTypeOfArea);
          break;
        case TAG_TABLE:
          MARK_DOMTREE_TABLE_TAG(area->nodeTypeOfArea);
          break;
        case TAG_H1:
        case TAG_H2:
        case TAG_H3:
        case TAG_H4:
        case TAG_H5:
        case TAG_H6:
          MARK_DOMTREE_H_TAG(area->nodeTypeOfArea);
          break;
        case TAG_UL:
        case TAG_OL:
          MARK_DOMTREE_LIST_TAG(area->nodeTypeOfArea);
          break;
        case TAG_FORM:
          MARK_DOMTREE_FORM_TAG(area->nodeTypeOfArea);
          break;
        default:
          break;
      }
      area->nodeTypeOfArea = area->nodeTypeOfArea | vnode->hpNode->subnodetype;
    }
    html_vnode_visit(vnode, visit_vnode_for_add_vnode2area, NULL, area);
    if (vnode == area->end) {
      break;
    }
  }
  return AREA_VISIT_NORMAL;
}

int area_partition(area_tree_t *atree, html_vtree_t *vtree,
                   const char *base_url) {
  debuginfo_on (DIVIDING_AREA);
  timeinit();
  timestart();

  unsigned int no = 0;
  int ret, doc_type;
  area_tree_clean(atree);
  /** add repeat struct info */
  vhtml_struct_prof(vtree);

  //初始化根节点、hp_vtree等属性
  atree->root = createAreaNode(&atree->np);
  if (atree->root == NULL) {
    goto FAIL;
  }

  //确定文档类型
  determine_doctype(vtree->hpTree, base_url);
  //如果页面宽度很窄，也当成wap页面处理(sue)
  doc_type = vtree->hpTree->doctype;
  if (doc_type != 1 && doc_type != 2 && vtree->root->wx < 250) {
    vtree->hpTree->doctype = doctype_xhtml_MP;
  }

  atree->hp_vtree = vtree;
  atree->root->area_tree = atree;
  atree->root->begin = vtree->root;
  atree->root->end = vtree->root;
  atree->root->area_info.xpos = vtree->root->xpos;
  atree->root->area_info.ypos = vtree->root->ypos;
  atree->root->area_info.width = vtree->root->wx;
  atree->root->area_info.height = vtree->root->hx;
  atree->root->depth = 0;

  ret = areaNode_divide(atree->root, &atree->config, &atree->np, 1);
  if (ret == -1) {
    goto FAIL;
  }
  // 校准页面宽度
  atree->root->area_info.width = real_page_width(atree->root);
  // 为每个area添加uid，并且更新vnode指向area的指针
  areatree_visit(atree, visit_for_add_no, finish_visit_for_add_vnode2area, &no);
  atree->area_num = no;

  timeend("area", "");
  dumpdebug(DIVIDING_AREA, DIVIDING_AREA);
  return 1;
  FAIL: timeend("area", "");
  dumpdebug(DIVIDING_AREA, DIVIDING_AREA);
  return -1;
}

/**
 * @brief 分块树的遍历函数，同 html_tree_visit 类似
 * @param [in/out] atree   :  area_tree_t*
 * @param [in/out] start   : FUNC_START_T
 * @param [in/out] finish   : FUNC_FINISH_T
 * @param [in/out] data   : void*
 * @return  bool 
 * @retval   成功 true ，失败false
 * @see 

 * @date 2011/07/05
 **/
bool areatree_visit(area_tree_t * atree, FUNC_START_T start,
                    FUNC_FINISH_T finish, void * data) {
  int ret = areatree_visit(atree->root, start, finish, data);
  if (ret == AREA_VISIT_ERR) {
    return false;
  }
  return true;
}

/**
 * @brief	分块树遍历的非递归实现.

 * @date 2011/07/05
 **/
int areatree_visit(html_area_t * area, FUNC_START_T start, FUNC_FINISH_T finish,
                   void * data) {
  int ret = AREA_VISIT_NORMAL;
  html_area_t *iter = area;

  while (iter) {
    int local_ret = AREA_VISIT_NORMAL;
    if (start != NULL) {
      local_ret = start(iter, data);
      if (local_ret == AREA_VISIT_ERR || local_ret == AREA_VISIT_FINISH) {
        ret = local_ret;
        goto FINISH;
      }
    }
    if (iter->subArea && local_ret != AREA_VISIT_SKIP) {
      iter = iter->subArea;
      continue;
    }

    /**
     * 如果该节点为叶节点或AREA_VISIT_SKIP，
     * 立即对其进行finish_visit()
     */
    if (finish != NULL) {
      local_ret = finish(iter, data);
      if (local_ret == AREA_VISIT_ERR || local_ret == AREA_VISIT_FINISH) {
        ret = local_ret;
        goto FINISH;
      }
      assert(local_ret == AREA_VISIT_NORMAL);
    }
    if (iter == area) {
      goto FINISH;
    }

    /**向右
     */
    if (iter->nextArea) {
      iter = iter->nextArea;
      continue;
    }

    /**向上
     */
    while (1) {
      iter = iter->parentArea;
      if (iter == NULL) {
        break;
      }
      if (finish != NULL) {
        local_ret = (*finish)(iter, data);
        if (local_ret == AREA_VISIT_ERR || local_ret == AREA_VISIT_FINISH) {
          ret = local_ret;
          goto FINISH;
        }
        assert(local_ret == AREA_VISIT_NORMAL);
      }
      if (iter == area) {
        goto FINISH;
      }

      /**向右
       */
      if (iter->nextArea) {
        iter = iter->nextArea;
        break;
      }
    }
  }

  FINISH: return ret;
}
void printArea(html_area_t * area, int level) {
}
void printAtree(area_tree_t * atree) {
  if (atree) {
    printArea(atree->root, 0);
  }

}
void printSingleArea(html_area_t * area) {
}


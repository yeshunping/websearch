
#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "base/logging.h"
#include "base/string_util.h"
#include "util/htmlparser/htmlparser/html_attr.h"
#include "util/htmlparser/vhtmlparser/vhtml_basic.h"
#include "util/htmlparser/utils/debuginfo.h"

bool is_same_font(font_t *a, font_t *b) {
  if (a->align == b->align && a->header_size == b->header_size
      && a->is_bold == b->is_bold && a->is_strong == b->is_strong
      && a->is_big == b->is_big && a->is_small == b->is_small
      && a->is_italic == b->is_italic && a->is_underline == b->is_underline
      && a->size == b->size && a->line_height == b->line_height
      && a->bgcolor == b->bgcolor && a->color == b->color) {
    return true;
  }

  return false;
}

/**
 * @brief 将颜色的数值表示转化为RGB表示.
 *
 * @param [in] color   : unsigned int 颜色的数值表示，如0xffffff
 * @return  rgb_t
 * @retval
 * @see

 * @date 2011/06/27
 **/
rgb_t int2rgb(unsigned int color) {
  rgb_t rgb;

  rgb.r = (color & (0xff0000)) >> 16;
  rgb.g = (color & (0x00ff00)) >> 8;
  rgb.b = color & (0x0000ff);

  return rgb;
}

/**
 * @brief 是否灰色。
 * R,G,B值相同，但又不为黑或白，则为灰色.
 * @param [in/out] color   : unsigned int
 * @return  bool
 * @retval
 * @see

 * @date 2011/06/27
 **/
bool is_gray_color(unsigned int color) {
  if (color == 0x000000 || color == 0xffffff)
    return false;

  rgb_t rgb = int2rgb(color);

  if (rgb.r == rgb.g && rgb.g == rgb.b)
    return true;

  return false;
}

/**
 * @brief	解析各种单位的长度，统一输出为px为单位的长度。
 * @param [in] value   : const char*	长度字符串
 * @param [in] base_size   : int 基准长度，比例单位长度需要
 * @param [out] _unit   : const char** 返回原长度单位
 * @return  int	返回解析出来的px长度；若没有单位，返回-1。

 * @date 2011/06/20
 **/
int parse_length(const char *value, int base_size, const char **_unit) {
  const char *unit = value;
  while (isdigit(*unit) || *unit == '.' || *unit == '+' || *unit == '-') {
    unit++;
  }
  *_unit = unit;

  /**
   * CSS属性在解析时已转化为小写.
   */
  if (is_attr_value(unit, "px", 2)) {
    return atoi(value);
  } else if (is_attr_value(unit, "pt", 2)) {
    return atoi(value) * 4 / 3;
  } else if (is_attr_value(unit, "in", 2)) {
    /** 1 in = 96 px
     */
    fract_num_t fract;
    atofract(&fract, value);
    return (fract.son * 96 / fract.mother);
  } else if (is_attr_value(unit, "cm", 2)) {
    /** 1 cm = 37.77 px
     */
    fract_num_t fract;
    atofract(&fract, value);
    return (fract.son * 3777 / 100 / fract.mother);
  } else if (is_attr_value(unit, "mm", 2)) {
    /** 1 mm = 3.78 px
     */
    fract_num_t fract;
    atofract(&fract, value);
    return (fract.son * 378 / 100 / fract.mother);
  } else if (is_attr_value(unit, "pc", 2)) {
    /** 1 pc=16 pt
     */
    fract_num_t fract;
    atofract(&fract, value);
    return (fract.son * 16 / fract.mother);
  } else if (is_attr_value(unit, "em", 2)) {
    /**相对大小*/
    fract_num_t fract;
    atofract(&fract, value);
    return (fract.son * base_size / fract.mother);
  } else if (is_attr_value(unit, "ex", 2)) {
    /**相对大小*/
    fract_num_t fract;
    atofract(&fract, value);
    return (fract.son * base_size / 2 / fract.mother);
  } else if (*unit == '%') {
    return (atoi(value) * base_size / 100);
  }

  return -1;
}

/**
 * @brief 遍历vTree子树的非递归实现.
 * 遍历一次比递归遍历要快40%.
 * 对遍历到的任何节点都会访问两次：start_visit()和finish_visit().
 * @return  int VISIT_NORMAL|VISIT_ERROR|VISIT_FINISH
 * @retval
 * @see

 * @date 2011/06/20
 **/
int html_vnode_visit_ex(html_vnode_t *html_vnode,
                        int (*start_visit)(html_vnode_t *, void *),
                        int (*finish_visit)(html_vnode_t *, void *),
                        void *result) {
  int ret = VISIT_NORMAL;
  html_vnode_t *vnode = html_vnode;
  while (vnode) {
    int local_ret = VISIT_NORMAL;
    if (start_visit != NULL) {
      local_ret = (*start_visit)(vnode, result);
      if (local_ret == VISIT_ERROR || local_ret == VISIT_FINISH) {
        ret = local_ret;
        goto FINISH;
      }
    }
    if (vnode->firstChild && local_ret != VISIT_SKIP_CHILD) {
      vnode = vnode->firstChild;
      continue;
    }
    /**
     * 如果该节点为叶节点或不访问其子节点，
     * 立即对其进行finish_visit()
     */
    if (finish_visit != NULL) {
      local_ret = (*finish_visit)(vnode, result);
      if (local_ret == VISIT_ERROR || local_ret == VISIT_FINISH) {
        ret = local_ret;
        goto FINISH;
      }
      assert(local_ret == VISIT_NORMAL);
    }
    if (vnode == html_vnode)
      goto FINISH;
    /**向右
     */
    if (vnode->nextNode) {
      vnode = vnode->nextNode;
      continue;
    }

    /**向上
     */
    while (1) {
      vnode = vnode->upperNode;
      if (vnode == NULL)
        break;
      if (finish_visit != NULL) {
        local_ret = (*finish_visit)(vnode, result);
        if (local_ret == VISIT_ERROR || local_ret == VISIT_FINISH) {
          ret = local_ret;
          goto FINISH;
        }
        assert(local_ret == VISIT_NORMAL);
      }

      if (vnode == html_vnode)
        goto FINISH;
      /**向右
       */
      if (vnode->nextNode) {
        vnode = vnode->nextNode;
        break;
      }
    }
  }

  FINISH: return ret;
}

/**
 * @brief 遍历vtree，非递归。
 * @return  int
 * @retval
 * @see

 * @date 2011/06/20
 **/
int html_vtree_visit_ex(html_vtree_t *html_vtree,
                        int (*start_visit)(html_vnode_t *, void *),
                        int (*finish_visit)(html_vnode_t *, void *),
                        void *result) {
  int ret;
  ret = html_vnode_visit_ex(html_vtree->root, start_visit, finish_visit,
                            result);
  if (ret == VISIT_ERROR)
    return 0;

  return 1;
}

void print_font(font_t *pfont) {
  if (pfont) {
    LOG(INFO) << StringPrintf(
        "font info:is_bold=%d,is_strong=%d,is_big=%d,size=%d,color=%d\n",
        pfont->is_bold, pfont->is_strong, pfont->is_big, pfont->size, pfont->color);
  }
}

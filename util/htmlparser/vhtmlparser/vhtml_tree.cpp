/**
 * easou_vhtml_tree.cpp
 * Description: V树解析
 *  Created on: 2011-11-13
 * Last modify: 2012-10-26 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#include "util/htmlparser/cssparser/css_utils.h"
#include "util/htmlparser/vhtmlparser/vhtml_tree.h"
#include "util/htmlparser/htmlparser/html_extractor.h"
#include "util/htmlparser/utils/debuginfo.h"
#include "util/htmlparser/utils/debug.h"

#include <sys/time.h>
#include <unistd.h>
#include <string>

#ifdef PARSER_SAVECSS
#include "sign.h"
#include "common_utils.h"
#include <sys/stat.h>
#endif

#include <vector>
//#include "batchreader.h"
//#include "MemManager.h"
//#include "TblRowReador.h"
//#include "ClientConfig.h"

using namespace std;
//using namespace nsPageDB;
//
//CLog g_Log;  //this declared as extern variable in pagedb libs
//static CClientConfig *g_fromConf = NULL;
//static CBatchReader *g_reader = NULL;
//static bool g_cssserverok = false;
//static int g_cssserver_init_count = 0;
//static pthread_mutex_t g_css_init_lock = PTHREAD_MUTEX_INITIALIZER;

#define CSS_COUNT 20
/**
 * @brief	创建并初始化Vtree输入结构.

 * @date 2011/06/27
 **/
vtree_in_t *vtree_in_create(int max_css_page_size, int css_num) {
  vtree_in_t *vtree_in = (vtree_in_t *) calloc(1, sizeof(vtree_in_t));
  if (vtree_in == NULL) {
    goto FAIL;
  }
  vtree_in->css_env = css_env_create(max_css_page_size, css_num);
  if (vtree_in->css_env == NULL) {
    goto FAIL;
  }
  vtree_in->url_num = 0;
  vtree_in->request_css_num = 0;
  vtree_in->missing_css_num = 0;
  return vtree_in;
  FAIL: if (vtree_in) {
    vtree_in_destroy(vtree_in);
  }
  return NULL;
}

/**
 * @brief

 * @date 2011/07/12
 **/
void vtree_in_destroy(vtree_in_t *vtree_in) {
  if (vtree_in) {
    if (vtree_in->css_env) {
      css_env_destroy(vtree_in->css_env);
    }
    free(vtree_in);
    vtree_in = 0;
  }
}

/**
 * @brief

 * @date 2011/06/27
 **/
html_vtree_t *html_vtree_create_with_tree(html_tree_t *html_tree) {
  html_vtree_t *html_vtree = NULL;
  assert(html_tree);
  html_vtree = (html_vtree_t *) calloc(1, sizeof(html_vtree_t));
  if (!html_vtree) {
    goto FAIL;
  }
  if (!nodepool_init(&html_vtree->np, sizeof(html_vnode_t))) {
    goto FAIL;
  }
  if (!nodepool_init(&html_vtree->css_np, sizeof(css_prop_node_t),
                     DEFAULT_CSS_PROP_NODE_NUM)) {
    goto FAIL;
  }
  html_vtree->struct_np_inited = 0;
  html_vtree->hpTree = html_tree;
  html_vtree->root = NULL;

#ifdef PARSER_SAVECSS
  if (check_path("./cssdir") == 1)
  {  //not exist
    mkdir("./cssdir", 0777);
  }
#endif

  return html_vtree;

  FAIL: if (html_vtree) {
    html_vtree_del_with_tree(html_vtree);
  }
  return NULL;
}

/**
 * @brief

 * @date 2011/06/27
 **/
void html_vtree_clean(html_vtree_t *html_vtree) {
  html_vtree_clean_with_tree(html_vtree);
  html_tree_reset_no_destroy((html_tree_impl_t*) (html_vtree->hpTree));
}

/**
 * @brief

 * @date 2011/06/27
 **/
void html_vtree_del(html_vtree_t *html_vtree) {
  if (html_vtree) {
    html_vtree_del_with_tree(html_vtree);
    html_vtree = 0;
  }
}

/**
 * @brief

 * @date 2011/06/27
 **/
void html_vtree_del_with_tree(html_vtree_t *html_vtree) {
  nodepool_destroy(&html_vtree->np);
  nodepool_destroy(&html_vtree->css_np);
  vhtml_struct_prof_del(html_vtree);
  free(html_vtree);
  html_vtree = 0;
}

/**
 * @brief

 * @date 2011/06/27
 **/
void html_vtree_clean_with_tree(html_vtree_t *html_vtree) {
  nodepool_reset(&html_vtree->np);
  nodepool_reset(&html_vtree->css_np);
  html_vtree->root = NULL;
  html_vtree->normal_struct_info_added = 0;
  html_vtree->repeat_struct_info_added = 0;
}

void free_css_server() {
  // TODO(shunping) :
}

bool init_css_server(char * config_path, char *log_dir, int timeout,
                     int thread_num, int cache_size) {
  // TODO(shunping) :
  return true;
}

int getVtreeCssInfo(html_vtree_t *html_vtree, vtree_in_t *vtree_in,
                    const char *url, char *csspage_conversion,
                    int csspagelength) {
  if (!html_vtree || !html_vtree->hpTree
      || !csspage_conversion || !vtree_in) {
    return 0;
  }
  vtree_in->url_num++;
  int csspos = 0;
  int linknum = CSS_COUNT;
  link_t links[CSS_COUNT];  //保存链接信息
  memset(links, 0, sizeof(links));
  int reallinknum = html_tree_extract_csslink(html_vtree->hpTree, url, links,
                                              linknum);
  dprintf("outer css num:%d\n", reallinknum);
  if (reallinknum <= 0) {
    return 0;
  }
  vector<string> allkeys;
  for (int i = 0; i < CSS_COUNT && i < reallinknum; i++) {
    if (strlen(links[i].url) == 0) {
      break;
    }
    char httpurl[2096] = { 0 };
    if (strstr(links[i].url, "://") == NULL) {
      sprintf(httpurl, "http://%s", links[i].url);
    } else {
      sprintf(httpurl, "%s", links[i].url);
    }
    allkeys.push_back(string(httpurl));
  }
  if (allkeys.size() <= 0) {
    return 0;
  }
  vtree_in->request_css_num += allkeys.size();

  for (size_t i = 0; i < allkeys.size(); i++) {
    string css_content;
    // TODO(shunping) : Get css content by url.
    memcpy(csspage_conversion + csspos, css_content.data(), css_content.size());
    *(csspage_conversion + csspos + css_content.size()) = '\0';

    dprintf("outer css:%s get success\n", links[i].url);
    add_out_style_text(&(vtree_in->css_env->page_css),
                       csspage_conversion + csspos, links[i].url);
    csspos = csspos + css_content.size() + 1;
  }

  allkeys.clear();
  return csspos;
}

int html_vtree_parse_with_tree(html_vtree_t *html_vtree, vtree_in_t *vtree_in,
                               const char *url, int page_width,
                               bool test_import, bool useoutcss) {
  assert(html_vtree->hpTree);
  if (vtree_in) {
    //从css服务器获取外部css
    if (useoutcss) {
      memset(vtree_in->CSS_OUT_PAGE, 0, sizeof(vtree_in->CSS_OUT_PAGE));
      getVtreeCssInfo(html_vtree, vtree_in, url, vtree_in->CSS_OUT_PAGE,
                      sizeof(vtree_in->CSS_OUT_PAGE));
    }

    //解析css
    get_parse_css_inpage(vtree_in->css_env, html_vtree->hpTree, url,
                         test_import);
    if (!vtree_in->css_env) {
      goto FAIL;
    }
    //创建V树，计算节点的位置和长宽
    if (html_vtree_parse_with_css(html_vtree, &(vtree_in->css_env->css_pool),
                                  page_width) == VTREE_ERROR) {
      goto FAIL;
    }
  } else {
    if (html_vtree_parse_with_css(html_vtree, NULL, page_width) == VTREE_ERROR) {
      goto FAIL;
    }
  }
  return VTREE_NORMAL;

  FAIL: html_vtree_clean_with_tree(html_vtree);
  return VTREE_ERROR;
}

int html_vtree_parse_with_css(html_vtree_t *html_vtree,
                              easou_css_pool_t *css_pool, int page_width) {
  debuginfo_on(CALC_VTREE); timeinit(); timestart();

  int ret = VTREE_NORMAL;
  space_mgr_t space_mgr;
  table_col_t default_col[DEFAULT_TABLE_COL_NUM];
  html_vtree_init();
  //重置V树
  html_vtree_clean_with_tree(html_vtree);
  //创建V树
  int id = 0;
  html_vtree->root = construct_vtree(&html_vtree->np, &html_vtree->hpTree->root,
                                     0, id, html_vtree);
  if (!html_vtree->root) {
    goto FAIL;
  }
  html_vtree->body = html_vtree->root;
  //根据html标签的属性设置V树节点的属性
  html_vtree_get_html_property(html_vtree);  //css 属性优先级，节点>内css>外部css
  //根据css设置V树节点的属性
  if (css_pool) {
    html_vtree_add_info_with_css2(html_vtree, css_pool);
  }
  //计算V树节点的字体，并设置其它的一些基本属性
  html_vtree_parse_font(html_vtree);
  //html_vtree_parse_border(html_vtree);
  //主要计算V树节点的最大宽，最小宽。初步计算V树节点的宽度
  //trans_down_top_for_property(html_vtree->root, page_width);
  trans_down_top(html_vtree->root, page_width);
  //设置根节点的宽度
  get_root_wx(html_vtree->root, page_width);
  //计算V树节点的宽度
  if (html_vtree_compute_wx(html_vtree->root, default_col) == PARSE_ERROR) {
    goto FAIL;
  }
  //由子节点开始，计算各个节点的位置，及节点的高度
  layout_down_top(html_vtree->root, &space_mgr, 0);
  html_vtree->root->xpos = 0;
  html_vtree->root->ypos = 0;
  //计算绝对位置
  compute_absolute_pos(html_vtree->root);
  //调整各个节点及页面宽度
  get_page_width(html_vtree->root);

  timeend("vhtmlparser", "parse"); dumpdebug(CALC_VTREE, CALC_VTREE);
  return ret;

  FAIL: html_vtree_clean(html_vtree);

  timeend("vhtmlparser", "parse"); dumpdebug(CALC_VTREE, CALC_VTREE);
  return VTREE_ERROR;
}

/**
 * @brief 遍历vTree子树的非递归实现.
 * 遍历一次比递归遍历要快40%.
 * 对遍历到的任何节点都会访问两次：start_visit()和finish_visit().
 * @return  int VISIT_NORMAL|VISIT_ERROR|VISIT_FINISH

 * @date 2011/06/20
 **/
int html_vnode_visit(html_vnode_t *html_vnode,
                     int (*start_visit)(html_vnode_t *, void *),
                     int (*finish_visit)(html_vnode_t *, void *),
                     void *result) {
  return html_vnode_visit_ex(html_vnode, start_visit, finish_visit, result);
}

/**
 * @brief 遍历vtree，非递归。
 * @return  int

 * @date 2011/06/20
 **/
int html_vtree_visit(html_vtree_t *html_vtree,
                     int (*start_visit)(html_vnode_t *, void *),
                     int (*finish_visit)(html_vnode_t *, void *),
                     void *result) {
  return html_vtree_visit_ex(html_vtree, start_visit, finish_visit, result);
}

/**
 *复位vhtml树，恢复初始状态
 */
void html_vtree_reset(html_vtree_t *html_vtree) {
  html_vtree_clean_with_tree(html_vtree);
  //html_tree_clean(html_vtree->hpTree);
}

void vtree_in_reset(vtree_in_t *vtree_in) {
  if (vtree_in) {
    if (vtree_in->css_env) {
      css_env_reset(vtree_in->css_env);
    }
  }
}

#ifndef __EASOU_VHTML_UTILS_H_
#define __EASOU_VHTML_UTILS_H_

/**
 * @brief print vtree
 * @return 写入到buf的长度
 */
int print_vtree_html(html_vtree_t *html_vtree, char* buf, int size, const char* base);

/**
 * @brief 递归打印当前子树的节点信息
 * @return 写入到buf的长度
 */
int print_vnode_html(html_vnode_t *html_vnode, char* buf, int size, int& avail, const char* base);

/**
 * @brief print vtree
 * @return 写入到buf的长度
 */
int print_vtree(html_vtree_t *html_vtree, char* buf, int size);

/**
 * @brief 递归打印当前子树的节点信息
 * @return 写入到buf的长度
 */
int print_vnode(html_vnode_t *html_vnode, char* buf, int size, int& avail);

/**
 * @brief 打印单个V树节点信息
 * @param [in] vnode,
 * @param [in/out] p, 缓存
 * @param [in] bufLen, p的长度
 * @param [in] space_len, 打印空格数
 * @param [in] type, 2，父；3，子
 * @return 写入到p的长度
 */
int print_vnode_info(html_vnode_t *vnode, char *p, int bufLen, int space_len, int type);

#endif

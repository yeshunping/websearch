

#ifndef EASOU_HTML_PARSER_H_
#define EASOU_HTML_PARSER_H_

#include "html_dom.h"

/**
 * @brief 从parser中获取内存
 **/
void* html_parser_palloc(struct html_parser_t*, size_t);

/**
 * @brief 从tree中获取内存，实际上还是才能够parser上获取
 **/
void* html_tree_palloc(html_tree_t* tree, size_t size);

/**
 * @brief 从tree中获取内存，同时进行初始化，实际上还是才能够parser上获取
 */
char* html_tree_strndup(html_tree_t *tree, const char *text, size_t len);

/**
 * @brief 创建html parser
 * @return 成功返回parser
 **/
struct html_parser_t* html_parser_create();

/**
 * @brief Destroy HTML Parser
 **/
void html_parser_destroy(struct html_parser_t *parser);

/**
 * @brief Get XML compatible state from Parser
 * @param[in] parser The HTML Parser
 * @return If XML compatible is enabled return 1, otherwize return 0
 **/
int html_parser_get_xml_compatible(struct html_parser_t *parser);

/**
 * @brief Set XML compatible state from Parser
 * @param[in] parser The HTML Parser
 * @param[in] enable The new state value
 * @note enable = 1 means to enable XML compatible, 0 to disable, default is disable
 **/
void html_parser_set_xml_compatible(struct html_parser_t *parser, int enable);

/**
 * @brief Get WML compatible state from Parser
 * @param[in] parser The HTML Parser
 * @return If WML compatible is enabled return 1, otherwize return 0
 **/
int html_parser_get_wml_compatible(struct html_parser_t*);

/**
 * @brief Set WML compatible state from Parser
 * @param[in] parser The HTML Parser
 * @param[in] enable The new state value
 * @note enable = 1 means to enable WML compatible, 0 to disable, default is disable
 **/
void html_parser_set_wml_compatible(struct html_parser_t*, int);

/**
 * @brief Get script parsing state from Parser
 * @param[in] parser The HTML Parser
 * @return If script parsing is enabled return 1, otherwize return 0
 **/
int html_parser_get_script_parsing(struct html_parser_t*);

/**
 * @brief Set script parsing state from Parser
 * @param[in] parser The HTML Parser
 * @param[in] enable The new state value
 * @note enable = 1 means to enable script parsing, 0 to disable, default is enable
 **/
void html_parser_set_script_parsing(struct html_parser_t*, int);

/**
 * @brief 将html网页解析成dom树
 **/
int html_parse(struct html_parser_t *parser, html_tree_t *tree, const char *page, size_t size, int ignore_space);

//int html_parser_reset_all(struct html_parser_t *parser);
#endif /* EASOU_HTML_PARSER_H_ */

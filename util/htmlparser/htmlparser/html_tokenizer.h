/**
 * @file html_tokenizer.h

 * @date 2011/08/02
 * @version 1.0
 * @brief html源代码读取器
 *
 **/

#ifndef EASOU_HTML_TOKENIZER_H_
#define EASOU_HTML_TOKENIZER_H_

#include <stddef.h>
#include "html_dom.h"

/**
 * @brief Create HTML Tokenizer
 * @param[in] pool Memory pool
 * @return If success return the tokenizer, otherwize return NULL
 **/
html_tokenizer_t* html_tokenizer_create(struct mem_pool_t *pool);

/**
 * @brief Reset the HTML Tokenizer
 * @param[in] tokenizer HTML Tokenizer to be reset
 * @param[in] html HTML source code to be tokenized
 * @param[in] size Length of HTML source code
 **/
void html_tokenizer_reset(html_tokenizer_t *tokenizer, const char *html, size_t size);

/**
 * @brief Tokenize HTML source into nodes
 **/
html_node_t* html_tokenize(html_tokenizer_t*, html_tree_t*);

/**
 * @brief switch tokenizer state to parsing RCDATA
 **/
void html_tokenizer_switch_to_rcdata(html_tokenizer_t*);

#endif /* EASOU_HTML_TOKENIZER_H_ */



#ifndef HTML_TEXT_UTILS_H_
#define HTML_TEXT_UTILS_H_

/**
 * @brief 字符串copy,只处理GB18030编码的字符.
**/
int html_tree_copy_html_text_gb18030(char *buffer, int available, int end, char *src);

/**
 * @brief 快速的copy_html_text. 若当前文本不含&，则忽略转义.
**/
int copy_html_text(char *buffer, int available, int end, char *src);

int html_deref_to_gb18030_str(char* psrc, char* pdes, int slen, int dlen);

#endif /* HTML_TEXT_UTILS_H_ */

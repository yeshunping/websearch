
#ifndef EASOU_STRING_H_
#define EASOU_STRING_H_

#include <stddef.h>

#define MAX_TIME_STR_LEN 128
#define MAX_TIME_UNIT_NUM 8

extern int CHAR_SPACE[];
extern char legal_char_set[];
extern char url_eng_set[];
extern char legal_word_set[];

/**
 * @brief whitespace map
 **/
extern char g_whitespace_map[];

/**
 *  判断给定的字符是否为空格\n
 *  字符串存放到dststr中，srcstr保持不变
 *  @param[in]  ch 待检测的字符
 *  @return 返回判断结果
 *  - 0 表示不是空格
 *  - 1 表示是空格
 *  @note 注意space字符包括'\0',这与函数isspace不同
 */
#define isspace(ch) CHAR_SPACE[(unsigned char)(ch)]

/*
 * 字符大小写转换
 */
char tolower(unsigned char chr);

/**
 * 字符串大小写转换
 */
int trans2lower(char* lower, char* upper);

/**
 * @brief 转化为半角. 输出和输入buffer可相同.
 * @return int, 转换后的长度
 **/
int trans2bj(const char *in, char *out);

/**
 * @brief 转化为半角和小写. 输出和输入buffer可相同.
 **/
void trans2bj_lower(const char *in, char *out);

/**
 * @brief 计算文本大小、宽度.
 *		 	认为一个ASCII字符占一个文本大小单位,一个汉字占两个文本大小单位.
 * 		文本大小结合字体可以算出文本的宽度.
 * 		这个值与编码有关.
 * 	@param cn_num [in/out], 中文汉字个数

 * @date 2011/06/27
 **/
int getTextSize(const char *src, int &cn_num);

/**
 * @brief	根据编码区间判断是否有用的字. 跟编码相关.
 **/
int is_valid_word(const char *p);

/**
 * @brief 字符是否数字. 实验表明，这样写比试过的其他方式要快.
 **/
bool q_isdigit(const char ch);

/**
 * @brief	是否空白字符串.
 **/
bool is_space_text(const char *text);

/**
 * @brief 判断两个字符之间是否全是空格
 */
bool is_only_space_between(const char *begin, const char *end);

/**
 * @brief 扫描空格
 * @param [in] pstr   : const char* 字符指针.
 * @return  const char* 从输入指针开始的第一个非空格字符指针.
 * @retval
 * @see

 * @date 2011/06/20
 **/
const char *skip_space(const char *pstr);

/**
 * @brief	是否是一个表示时间的字符串
 **/
bool is_time_str(const char * str_time);

/**
 * @brief 	字符串拷贝
 * @param [out] dst   : char*	目标缓冲区
 * @param [int] src   : const char*	src 源缓冲区
 * @param [in] siz   : size_t 目标缓冲区的大小
 * @return  size_t 返回应该期望的字符串长度，注意可能会超过缓冲区大小
 **/
size_t strlcpy(char *dst, const char *src, size_t siz);

/**
 * @brief 最长公共子串. 返回公共子串的长度，可能与编码有关.
 **/
int longest_common_substring(const char *l, const char *r);

/**
 * @brief 过掉字符串内的空白，英文单词间空白除外. 返回转换后的长度.
 **/
int trim_space(char *buf);

/**
 * @brief 裁剪字符串两边的空格， 如果中间有空格则会截取到中间的第1个空格处。在原字符串上修改。
 * @return int, 返回转换后的长度
 **/
int str_trim_side_space(char *str);

/**
 * @brief	是否含有空白字符
 **/
bool is_has_special_word(const char * spec_words[], const char * word);

/**
 * @brief 去掉字符串结尾处的\n, \r, \t
 * @return 返回值< 0，失败
 */
int remove_tail_rnt(char *line);

/**
 * @brief 将字符串中的全角字符转换为半角,并过滤乱码
 * @return int, >=0 转换后的长度； <0  src is NULL
 */
int trans_full_to_half(const char *src, const int srcLen, char *dest,
                       int *destLen);

/**
 * @brief 获取下一个gb18030字符的长度，如果为半角字符和数字，返回1；如果为全角字符返回2；如果为汉字，返回2或则4
 */
int get_next_gb18030_bytes(unsigned char* s);

/**
 * @brief 只保留中文汉字
 * @param src [in], 输入字符串
 * @param srcLen [in], src的长度
 * @param dest [in/out], 保存只保留汉字的字符串
 * @param destLen [out], dest的长度
 * @return dest保存的字符串的长度
 */
int remain_only_chinese(const char *src, const int srcLen, char *dest,
                        int *destLen);

#endif /* EASOU_STRING_H_ */

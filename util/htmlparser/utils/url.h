
#ifndef EASOU_URL_H_
#define EASOU_URL_H_

#include "string_util.h"

/**
 * @brief href属性的类型
 **/
#define IS_LINK_INNER 1 /**站内url*/
#define IS_LINK_OUT 2	/**站外url*/
#define IS_LINK_ANCHOR 3	/**站外url*/
#define IS_LINK_ERR (-1)/**其他，一般指js*/

#define MAX_URL_LEN			2048
#define MAX_SITE_LEN		256
#define MAX_PATH_LEN		1600
#define MAX_PORT_LEN		7 //64k 5+:+1
#define MAX_ANCHOR_LEN		256

/*
 * @breif 是否是绝对url
 */
int is_url(const char *url);

/**
 * @brief 获取主域名，不包括最后的com
 */
const char * fetch_maindomain_from_url(const char * url, char * domain, int size);

/**
 * @brief 获取主域名，包括最后的com
 */
const char * fetch_domain_l_from_url(const char * url, char * domain, int size);

/**
 * @brief 判断href的类型：站内，站外，js（ERR）
 **/
int get_href_type(const char *phref, const char *base_url);

/**
 * @brief 根据url解析出其中的各个部分
 * @param input 输入的url
 * @param[out] site 站点名缓冲区
 * @param[out] port 端口
 * @param[out] path 路径
 * @return 1正常，0无效url格式
 */
int parse_url(const char *input, char *site, char *port, char *path);

/**
 * @brief 归一化URL路径
 * @param[in,out] path 路径
 * @return 1正常，0无效url格式
 */
int single_path(char *path);

/**
 * @brief 根据url解析出其中的路径部分
 * @param url 输入的url
 * @param[out] path 路径
 * @return NULL失败，否则为指向path的指针
 */
char *get_path(const char *url, char *path);

/**
 * @brief 根据url解析出其中的站点名部分
 * @param url 输入的url
 * @param[out] site 站点名( be sure it is enough,larger than UL_MAX_SITE_LEN)
 * @return NULL失败，否则为指向site的指针
 */
char *get_site(const char *url, char *site);

/**
 * @brief 规范化路径的形式,对'\\', './', '/.', '../', '/..', '//'等形式进行规范化
 * @param[in,out] path 待转化的路径
 */
void normalize_path(char *path);

/**
 * @brief get directory of path
 */
void remove_path_file_name(char *path);

/**
 * @brief 从url中获取端口信息
 * @param url 输入的url
 * @param[out] pport 端口
 * @return 1成功，0失败
 */
int get_port(const char* url, int* pport);

/**
 * @brief 从url中获取静态的部分（?或者;之前的部分）
 * @param url 输入的url
 * @param[out] staticurl 静态部分缓冲区
 */
void get_static_part(const char *url, char *staticurl);

//int parse_url(char *input,char *site,char *port,char *path);
//char *get_path(char *url, char *path);
//char *get_site(char *url, char *site);
//int get_port(char* url,int* pport);
//void get_static_part(char *url, char *staticurl);

/**
 * @brief 判断url是否是动态url
 * @returnu 0不是动态url，非0，是动态url
 */
int isdyn(const char* str);

/**
 * @brief 判断url是否合法，这个方法在某种情况下会修改url输入，调用时需要注意。
 * @param url 输入url
 * @returnu 1合法，0不合法
 */
int check_url(char* url);

/**
 * @brief 从站点名中获取主干部分, 比如"www.easou.com"将获得"easou"
 * @param site 站点名
 * @param[out] trunk 存放主干部分的缓冲区
 * @param size 缓冲区大小
 * @return 1成功，-1未知错误，-2站点没有主干部分，-3站点不包含'.'
 */
int fetch_trunk(const char* site, char *trunk, int size);

/**
 * @brief 检查站点名是否是IP地址
 * @param sitename 站点名
 * @return 0不是，非0是
 */
int is_dotip(const char *sitename);

/**
 * @brief 从站点名中获取主干部分，功能类同@ref fetch_trunk()
 * @param site 站点名
 * @param[out] domain 存放主干部分的缓冲区
 * @param size 缓冲区大小
 * @return NULL失败，否则为指向site主干部分的指针
 */
const char* fetch_maindomain(const char* site, char *domain, int size);

/**
 * @brief 检查url是否规范化
 * @param url 检查的url
 * @return 1是，0不是
 */
int isnormalized_url(const char *url);

/**
 * @brief 将url转化为统一的形式\n
 * 执行@ref normalize_site, @ref normalize_port, @ref single_path, @ref normalize_path
 * @param url 待转化的url
 * @param[out] buf 转化后的url缓冲区
 * @return 1成功，0无效url
 * @note you can use normalize_url(buf, buf) to save an extra buffer.
 */
int normalize_url(const char* url, char* buf);

/**
 * @brief 将站点名进行规范化（大写转为小写）
 * @param site 站点名
 * @return 1成功，0失败
 */
int normalize_site(char *site);

/**
 * 将端口字符串进行规范化（检查端口范围合法性，并去掉80端口的字符串）
 * @param port 指向的端口的指针
 * @return 1成功，0失败
 */
int normalize_port(char *port);

/**
 *  根据url解析出其中的各个部分,支持加长的url，最高可支持到1024，path最长可到800，site最长可到128
 *
 *  @param[in]  input          输入的url
 *  @param[in]  site           site字段的存储buf指针
 *  @param[in]  site_size      site缓冲区的大小，可根据此字段设置合理的site长度,默认为128,使用时可适当调小.
 *  @param[in]  port           port字段的存储buf指针
 *  @param[in]  port_size      port字段的大小
 *  @param[in]  path           path字段的存储buf指针
 *  @param[in]  max_path_size  path字段的大小,可根据此字段设置合理的path长度,默认为800,使用时可适当调小.
 *  @param[out] site           site值
 *  @param[out] port           port值
 *  @param[out] path           path路径
 *  @return 函数操作结果
 *  - 1   表示正常
 *  - 0  无效url格式
 *  - @note 为保证程序安全,传入的buf请小于默认最大值
 */
int parse_url_ex(const char *input, char *site, size_t site_size, char *port, size_t port_size, char *path, size_t max_path_size);

/**
 *  根据url解析出其中的路径部分,支持加长的url，最高可支持到1024，path最长可到800，site最长可到128
 *
 *  @param[in]  url          输入的url
 *  @param[in]  path         site字段的存储buf指针
 *  @param[in]  path_size    path字段的大小,可根据此字段设置合理的path长度,默认为800,使用时可适当调小.
 *  @param[out] path         path路径
 *  @return 函数操作结果
 *  - 非NULL   指向路径的指针
 *  - NULL     表示失败
 *  - @note 为保证程序安全,传入的path_size请小于默认最大值
 */
char *get_path_ex(const char *url, char *path, size_t path_size);

/**
 *  根据url解析出其中的站点名部分,支持加长的url，最高可支持到1024，path最长可到800，site最长可到128
 *
 *  @param[in]  url            输入的url
 *  @param[in]  site           site字段的存储buf指针
 *  @param[in]  site_size      site缓冲区的大小，可根据此字段设置合理的site长度,默认为128,使用时可适当调小.
 *  @param[out] site           site值
 *  @return 函数操作结果
 *  - 非NULL   指向site的指针
 *  - NULL     表示失败
 *  - @note 为保证程序安全,传入的site_size请小于默认最大值
 */
char *get_site_ex(const char *url, char *site, size_t site_size);

/**
 *  从url中获取端口信息,支持加长的url，最高可支持到1024，path最长可到800，site最长可到128
 *
 *  @param[in]  input          输入的url
 *  @param[in]  pport          port字段的存储buf指针
 *  @param[out] pport          port值
 *  @return 函数操作结果
 *  - 1   表示成功
 *  - 0   表示失败
 */
int get_port_ex(const char* url, int* pport);

/**
 *  将url转化为统一的形式\n,支持加长的url，最高可支持到1024，path最长可到800，site最长可到128
 *  执行@ref normalize_site, @ref normalize_port, @ref single_path, @ref normalize_path
 *
 *  @param[in]  url           待转化的url
 *  @param[in]  buf           转化后的url缓冲区
 *  @param[in]  buf_size      buf的大小
 *  @param[out] buf           转化后的url
 *  @return 函数操作结果
 *  - 1   成功
 *  - 0   无效url
 *  - @note 为保证程序安全,传入的site_size请小于默认最大值
 */
int normalize_url_ex(const char* url, char* buf, size_t buf_size);

/**
 *  从url中获取静态的部分（?或者;之前的部分）,支持加长的url，最高可支持到1024，path最长可到800，site最长可到128
 *
 *  @param[in]  url                 输入的url
 *  @param[in]  staticurl           静态部分缓冲区
 *  @param[in]  staticurl_size      buf的大小
 *  @param[out] staticurl           静态部分
 *  @return 无
 */
void get_static_part_ex(const char *url, char *staticurl, size_t staticurl_size);

/**
 *  检查url是否规范化,支持加长的url，最高可支持到1024，path最长可到800，site最长可到128
 *
 *  @param[in]  url                 检查的url
 *  @param[out] 无
 *  @return 返回判断结果
 *  - 1   是
 *  - 0   不是
 */
int isnormalized_url_ex(const char *url);

/**
 *  规范化路径的形式\n,支持加长的url，最高可支持到1024，path最长可到800，site最长可到128
 * 对'\\', './', '/.', '../', '/..', '//'等形式进行规范化
 *
 *  @param[in]  path           待转化的路径
 *  @param[out] path           转化后的路径
 *  @return 无
 */
void normalize_path_ex(char *path);

/**
 *  归一化URL路径,支持加长的url，最高可支持到1024，path最长可到800，site最长可到128
 *
 *  @param[in]  path         path路径
 *  @param[out] path         归一化过的路径
 *  @return 返回归一化结果
 *  - 1   正常
 *  - 0   无效url格式路径
 */
int single_path_ex(char *path);

/**
 *  判断url是否合法,支持加长的url，最高可支持到1024，path最长可到800，site最长可到128
 *
 *  @param[in]  url           待转化的url
 *  @param[out] 无
 *  @return 函数操作结果
 *  - 1   合法
 *  - 0   不合法
 */
int check_url_ex(char *url);

/**
 *  根据url解析出其中的各个部分,支持加长的url，最高可支持到2048，path最长可到1600，site最长可到256
 *
 *  @param[in]  input          输入的url
 *  @param[in]  site           site字段的存储buf指针
 *  @param[in]  site_size      site缓冲区的大小，可根据此字段设置合理的site长度,默认为256,使用时可适当调小.
 *  @param[in]  port           port字段的存储buf指针
 *  @param[in]  port_size      port字段的大小
 *  @param[in]  path           path字段的存储buf指针
 *  @param[in]  max_path_size  path字段的大小,可根据此字段设置合理的path长度,默认为1600,使用时可适当调小.
 *  @param[out] site           site值
 *  @param[out] port           port值
 *  @param[out] path           path路径
 *  @return 函数操作结果
 *  - 1   表示正常
 *  - 0  无效url格式
 *  - @note 为保证程序安全,传入的buf请小于默认最大值
 */
int parse_url_ex2(const char *input, char *site, size_t site_size, char *port, size_t port_size, char *path, size_t max_path_size);
/**
 *  根据url解析出其中的路径部分,支持加长的url，最高可支持到2048，path最长可到1600，site最长可到256
 *
 *  @param[in]  url          输入的url
 *  @param[in]  path         site字段的存储buf指针
 *  @param[in]  path_size    path字段的大小,可根据此字段设置合理的path长度,默认为1600,使用时可适当调小.
 *  @param[out] path         path路径
 *  @return 函数操作结果
 *  - 非NULL   指向路径的指针
 *  - NULL     表示失败
 *  - @note 为保证程序安全,传入的path_size请小于默认最大值
 */
char *get_path_ex2(const char *url, char *path, size_t path_size);

/**
 *  根据url解析出其中的站点名部分,支持加长的url，最高可支持到2048，path最长可到1600，site最长可到256
 *
 *  @param[in]  url            输入的url
 *  @param[in]  site           site字段的存储buf指针
 *  @param[in]  site_size      site缓冲区的大小，可根据此字段设置合理的site长度,默认为256,使用时可适当调小.
 *  @param[out] site           site值
 *  @return 函数操作结果
 *  - 非NULL   指向site的指针
 *  - NULL     表示失败
 *  - @note 为保证程序安全,传入的site_size请小于默认最大值
 */
char *get_site_ex2(const char *url, char *site, size_t site_size);

/**
 *  从url中获取端口信息,支持加长的url，最高可支持到2048，path最长可到1600，site最长可到256
 *
 *  @param[in]  input          输入的url
 *  @param[in]  pport          port字段的存储buf指针
 *  @param[out] pport          port值
 *  @return 函数操作结果
 *  - 1   表示成功
 *  - 0   表示失败
 */
int get_port_ex2(const char* url, int* pport);

/**
 *  将url转化为统一的形式\n,支持加长的url，最高可支持到2048，path最长可到1600，site最长可到256
 *  执行@ref normalize_site, @ref normalize_port, @ref single_path, @ref normalize_path
 *
 *  @param[in]  url           待转化的url
 *  @param[in]  buf           转化后的url缓冲区
 *  @param[in]  buf_size      buf的大小
 *  @param[out] buf           转化后的url
 *  @return 函数操作结果
 *  - 1   成功
 *  - 0   无效url
 *  - @note 为保证程序安全,传入的site_size请小于默认最大值
 */
int normalize_url_ex2(const char* url, char* buf, size_t buf_size);

/**
 *  从url中获取静态的部分（?或者;之前的部分）,支持加长的url，最高可支持到2048，path最长可到1600，site最长可到256
 *
 *  @param[in]  url                 输入的url
 *  @param[in]  staticurl           静态部分缓冲区
 *  @param[in]  staticurl_size      buf的大小
 *  @param[out] staticurl           静态部分
 *  @return 无
 */
void get_static_part_ex2(const char *url, char *staticurl, size_t staticurl_size);

/**
 *  检查url是否规范化,支持加长的url，最高可支持到2048，path最长可到1600，site最长可到256
 *
 *  @param[in]  url                 检查的url
 *  @param[out] 无
 *  @return 返回判断结果
 *  - 1   是
 *  - 0   不是
 */
int isnormalized_url_ex2(const char *url);

/**
 *  规范化路径的形式\n,支持加长的url，最高可支持到2048，path最长可到1600，site最长可到256
 * 对'\\', './', '/.', '../', '/..', '//'等形式进行规范化
 *
 *  @param[in]  path           待转化的路径
 *  @param[out] path           转化后的路径
 *  @return 无
 */
void normalize_path_ex2(char *path);

/**
 *  归一化URL路径,支持加长的url，最高可支持到2048，path最长可到1600，site最长可到256
 *
 *  @param[in]  path         path路径
 *  @param[out] path         归一化过的路径
 *  @return 返回归一化结果
 *  - 1   正常
 *  - 0   无效url格式路径
 */
int single_path_ex2(char *path);

/**
 *  判断url是否合法,支持加长的url，最高可支持到2048，path最长可到1600，site最长可到256
 *
 *  @param[in]  url           待转化的url
 *  @param[out] 无
 *  @return 函数操作结果
 *  - 1   合法
 *  - 0   不合法
 */
int check_url_ex2(char *url);

/**
 * @brief 获得url深度
 */
int get_url_depth(const char * url);

/**
 * @brief 页面内的相对URL拼成一个绝对URL
 */
int combine_url(char *result_url, const char *base_url, const char *relative_url);
void combine_url_inner(char *url, char *domain, char *port, char *path);

/**
 * @brief 是不是类似首页的url
 **/
int is_like_top_url(const char *url);

#define MAX_DIR_NUM 20
#define MAX_PARAM_NUM 10

typedef struct _url_t
{
	const char *site;
	int site_len;

	const char *port;
	int port_len;

	const char *path;
	int path_len;

	int dir_num;
	const char *dir[MAX_DIR_NUM];
	int dir_len[MAX_DIR_NUM];

	int param_num;
	const char *name[MAX_PARAM_NUM];
	int name_len[MAX_PARAM_NUM];
	const char *value[MAX_PARAM_NUM];
	int value_len[MAX_PARAM_NUM];
} url_t;

#define MAX_ELE_LEN 200
#define MAX_DELIM_LEN 256
#define MAX_PATTERN_NUM 32

#define DIGIT 1
#define ALPHA 2
#define CHN_1 3
#define CHN_2 4
#define DELIM 5
#define EASOU_URL_OTHER 6

#define ELE_DIR 0
#define ELE_FILE 1
#define ELE_NAME 2
#define ELE_VALUE 3
#define MAX_SHIFT_NUM 1
#define MAX_SEPS_NUM 200

typedef struct _p_element_t
{
	int type;
	char raw[MAX_ELE_LEN];
	char regular[MAX_ELE_LEN];
	int pattern_len;
} element;

typedef struct _p_url_t
{
	char site[MAX_SITE_LEN];
	int site_len;
	char port[MAX_PORT_LEN];
	int port_len;
	element dir[MAX_DIR_NUM];
	int dir_num;
	bool has_file;
	element file;
	int param_num;
	element name[MAX_PARAM_NUM];
	element value[MAX_PARAM_NUM];
} p_url;

const char *parse_url_inner(url_t *url, const char *url_buffer, int length);

int single_path_inner(char *path);

/**
 * @brief 判断是否首页
 */
bool is_home_page(const char* url, const int urlLen);

/**
 * @brief 判断url是否是目录
 * @return int
 * @retval 1, 目录
 * @retval 0, 非目录
 */
int is_dir_url(const char *url);

/**
 * @brief url中是否存在非ascii 字符
 */
bool has_not_ascii_char(const unsigned char* url);

/**
 * @brief 把url转换成正则字符串
 * @return 0, 成功; -1, 失败
 */
int urlstr_to_regexstr(const char *url, int url_len, char *buf, int buf_len);

#endif /* EASOU_URL_H_ */

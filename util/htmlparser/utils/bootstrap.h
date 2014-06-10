/**
 * @file tools/bootstrap.h
 * @brief bootstrap训练算法框架.
 *  
 **/


#ifndef  __BOOTSTRAP_H_
#define  __BOOTSTRAP_H_
#include <unistd.h>
#include "simplehashmap.h"
#include "nodepool.h"

#define SEED_FEATURE "^SEED_FT^"		  /**< 种子特征的名字 */
#define MAX_FT_CONF_FILE_LEN (256 * 1024)		  /**< 特征管理配置字符串的最大长度 */
#define MAX_FEATURE_NAME_LEN (64)		  /**< 特征名的最大长度 */

struct ft_manager_t;		  /**< 特征管理器 */

/**
 * @brief 根据配置文件初始化特征管理器.
 * @param [in] conf_file   : const char* 特征配置文件.
 * @param [in] isTraining   : bool 是否处于训练阶段.
 * @retval   若失败返回NULL,否则返回创建的特征管理器.
**/
ft_manager_t *ft_manager_init(const char *conf_file, bool isTraining);

/**
 * @brief 根据配置字符串初始化特征管理器.
 * @param [in] conf_str   : const char* 特征配置文本.
 * @param [in] isTraining   : bool 是否处于训练阶段.
 * @retval   若失败返回NULL,否则返回创建的特征管理器.
**/
ft_manager_t *ft_manager_init_from_string(const char *conf_str, bool isTraining);

/**
 * @brief 添加一个特征.
 * @param [in/out] fev   : ft_manager_t* 特征管理器.
 * @param [in] ft_name   : const char* 特征名.
 * @param [in] isContinue   : bool 是否连续.
 * @param [in] value   : int 特征的值. 对于非连续特征,调用此接口即表示特征出现.
**/
void ft_manager_add(ft_manager_t *fev, const char *ft_name, bool isContinue, int value);

/**
 * @brief 当前个体是否属于目标类别.
 * @param [in] fev   : ft_manager_t* 特征管理器.
 * @param [in] isNormalize   : bool 是否根据特征数量进行规范化, 以避免偏袒特征多的个体.
**/
bool is_target_type(ft_manager_t *fev, bool isNormalize);

/**
 * @brief 获取属于目标类别的概率.
 * 	XXX:此接口在调用is_target_type()之后调用才有意义.
 * @param [in] fmgr   : ft_manager_t* 特征管理器.
 * @param [in] isNormalize   : bool 是否根据特征数量对概率进行规范化.
 * @return  double 返回概率值, 介于0到1.
**/
double get_target_type_probability(ft_manager_t *fmgr, bool isNormalize);

/**
 * @brief 调用此接口,表明已完成当前个体信息的收集,进行必要的统计工作.
 * @param [in] is_target_type   : bool 当前个体是否目标类型.
**/
void ft_manager_finish_an_entity(ft_manager_t *fev, bool is_target_type);

/**
 * @brief 清空对当前个体收集的信息, 表明当前个体的情况不参于统计.
**/
void ft_manager_new_fts_clean(ft_manager_t *fev);

/**
 * @brief 终止特征管理.
 *  在训练阶段, 输出这一轮迭代得到的特征.
 *  销毁特征管理结构.
 * @param [in/out] out_file   : const char* 用于训练阶段将训练出的信息输出到文件.
**/
void ft_manager_end(const char *out_file, ft_manager_t *fev);

/**
 * @brief 是否处于bootstrap迭代过程.
**/
bool is_bootstrap(ft_manager_t *fmgr);

/**
 * @brief 获取当前分类器的概率阈值.
**/
double get_p_bootstrap(ft_manager_t *fmgr);

/**
 * @brief 获取最小条件概率值.
**/
double get_min_ft_cond_probability(ft_manager_t *fmgr);

/**
 * @brief 窥视一下当前的分类结果, 但不改变内部状态.
**/
bool peek_target_type(ft_manager_t *fev, bool isNormalize);

#endif  //__TOOLS/BOOTSTRAP_H_


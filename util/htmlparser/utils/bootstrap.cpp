/**
 * @file tools/bootstrap.cpp
 * @brief bootstrap训练算法框架.
 *  
 **/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include "log.h"
#include "bootstrap.h"

using namespace EA_COMMON;

#define ALLOC_FAIL_THEN_GOTO(obj,flag) do {\
	if(NULL==(obj)) {\
		goto flag;\
	}\
} while (0)

static const int LOCAL_DEFAULT_NODEPOOL_SIZE = 256;		  /**< 当前文件使用nodepool的默认大小 */

/**
* @brief 连续属性的观测值.
*/
typedef struct _value_item_t {
	int value;		  /**< 连续属性的值 */
	double ft_cnt;		  /**< 在该值上的目标样本数量 */
	double no_ft_cnt;		  /**< 在该值上的非目标样本数量 */
	struct _value_item_t *next;
} value_item_t;

/**
* @brief 连续属性的分段结构.
*/
typedef struct _discret_item_t {
	int upper_bound;		  /**< 当前分段的上界 */
	double freq;		  /**< 当前分段的样本数量 */
	double cond_freq;		  /**< 当前分段的目标样本数量 */
	struct _discret_item_t *next;
} discret_item_t;

/**
* @brief 一个特征.
*/
typedef struct {
	char ft_name[MAX_FEATURE_NAME_LEN];		  /**< 特征名 */
	double freq;		  /**< 特征出现频率 */
	double cond_freq;		  /**< 特征在目标样本上的出现频率 */
	bool is_continue;		  /**< 特征是否连续 */
	int observ_value;		  /**< 该特征在当前样本上观察到的值 */
	value_item_t value_item_head;		  /**< 该特征的连续属性观察值 */
	discret_item_t discret_item_head;		  /**< 该特征的连续属性分段 */
	int discret_num;		  /**< 连续属性分段数量 */
	int index;
	unsigned selected:1;
} ft_element_t;

typedef struct {
	double pt_freq;		  /**< 目标样本数量 */
	double uni_freq;		  /**< 全集样本数量 */
	int ft_num;		  /**< 特征数量 */
	nodepool_t ft_nodepool;		  /**< 特征节点池 */
	nodepool_t vi_nodepool;		  /**< 连续属性值节点池 */
	nodepool_t dis_nodepool;		  /**< 连续属性分段节点池 */
	hashmap_t *ft_map;		  /**< hashmap用于快速存取特征 */
} ft_collector_t;

typedef struct {
	double positive_probability;		  /**< 属于目标样本的概率 */
	double negative_probability;		  /**< 不属于目标样本的概率 */
} probability_val_t;

/**
* @brief 特征管理器.
* 用于训练阶段的特征演化及线上运行阶段的特征管理.
*/
struct ft_manager_t {
	unsigned int isBootstrap;		  /**< 是否自举迭代阶段 */
	unsigned int isClassic;	/**< 在非经典模式下,对离散特征只在该特征值出现时才对似然值产生影响 */
	unsigned int isTraining:1;		  /**< 是否训练阶段 */
	unsigned int is_collect_ft_vector;
	int hidden_num;					/**< 符合先验分布的非可见样本个数 */
	int min_freq_to_discret;		/**< 对连续属性分段时,区间内的样本个数须大于此值才继续划分 */
	int min_ft_freq;				/**< 特征出现次数须大于此值,才能成为有效特征 */
	double positive_weight;		  /**< 正样本的重要度 */
	double negative_weight;		  /**< 负样本的重要度 */
	double min_ft_cond_probability;	/**< 为bootstrap过程需要,特征的条件概率若小于此值将纠正为最小值*/
	probability_val_t probability_val;		  /**< 用于概率计算 */
	double p_bootstrap;		  /**< 目标样本的概率阈值 */
	double ft_threshold;		  /**< 特征选取的阈值 */
	ft_collector_t *last_fts;		  /**< 上一次迭代的特征 */
	ft_collector_t *new_fts;		  /**< 正在训练的新的特征 */
	hashmap_t *new_ft_buffer;		  /**< 缓存的原始特征 */
	nodepool_t buffer_nodepool;		  /**< 用于缓存特征节点 */
};

bool is_bootstrap(ft_manager_t *fmgr)
{
	return fmgr->isBootstrap;
}

double get_p_bootstrap(ft_manager_t *fmgr)
{
	return fmgr->p_bootstrap;
}

double get_min_ft_cond_probability(ft_manager_t *fmgr)
{
	return fmgr->min_ft_cond_probability;
}

static void ft_collector_destroy(ft_collector_t *cc)
{
	if(cc){
		nodepool_destroy(&cc->ft_nodepool);
		nodepool_destroy(&cc->vi_nodepool);
		nodepool_destroy(&cc->dis_nodepool);
		if(cc->ft_map){
			hashmap_destroy(cc->ft_map);
		}
		free(cc);
	}
}

static ft_collector_t *ft_collector_create()
{
	ft_collector_t *cc = (ft_collector_t *)calloc(1, sizeof(ft_collector_t));
	ALLOC_FAIL_THEN_GOTO(cc, ERR);
	if(nodepool_init(&cc->ft_nodepool, sizeof(ft_element_t), LOCAL_DEFAULT_NODEPOOL_SIZE) == 0){
		goto ERR;
	}
	if(nodepool_init(&cc->vi_nodepool, sizeof(value_item_t), LOCAL_DEFAULT_NODEPOOL_SIZE) == 0){
		goto ERR;
	}
	if(nodepool_init(&cc->dis_nodepool, sizeof(discret_item_t), LOCAL_DEFAULT_NODEPOOL_SIZE) == 0){
		goto ERR;
	}
	cc->ft_map = hashmap_create();
	if(NULL == cc->ft_map){
		goto ERR;
	}

	return cc;
ERR:
	ft_collector_destroy(cc);
	return NULL;
}

static void ft_manager_destroy(ft_manager_t *fev)
{
	if(fev){
		ft_collector_destroy(fev->last_fts);
		ft_collector_destroy(fev->new_fts);
		hashmap_destroy(fev->new_ft_buffer);
		nodepool_destroy(&fev->buffer_nodepool);
		//ft_vector_set_destroy(&fev->ft_vector_set);
		free(fev);
	}
}

static ft_manager_t *ft_manager_create(bool isTraining)
{
	ft_manager_t *fev = (ft_manager_t *)calloc(1, sizeof(ft_manager_t));
	ALLOC_FAIL_THEN_GOTO(fev, ERR);
	fev->last_fts = ft_collector_create();
	ALLOC_FAIL_THEN_GOTO(fev->last_fts, ERR);

	fev->isTraining = isTraining;
	
	fev->new_ft_buffer = hashmap_create(137);
	ALLOC_FAIL_THEN_GOTO(fev->new_ft_buffer, ERR);
	if(nodepool_init(&fev->buffer_nodepool, sizeof(ft_element_t), LOCAL_DEFAULT_NODEPOOL_SIZE) == 0){
		goto ERR;
	}

	if(isTraining){
		fev->new_fts = ft_collector_create();
		ALLOC_FAIL_THEN_GOTO(fev->new_fts, ERR);
	}

	return fev;
ERR:
	ft_manager_destroy(fev);
	return NULL;
}

static void add_value_item(ft_element_t *ele, int value, bool is_pt, ft_collector_t *cc, 
		ft_manager_t *fmgr)
{
	value_item_t *last_value_item = &ele->value_item_head;
	for(value_item_t *vi = ele->value_item_head.next; vi; vi=vi->next){
		if(vi->value == value){
			if(is_pt){
				vi->ft_cnt += fmgr->positive_weight;
			}
			else{
				vi->no_ft_cnt += fmgr->negative_weight;
			}
			return;
		}
		else if(vi->value > value){
			break;
		}
		last_value_item = vi;
	}

	value_item_t *new_vi = (value_item_t *)nodepool_get(&cc->vi_nodepool);
	if(new_vi){
		memset(new_vi, 0, sizeof(value_item_t));
		new_vi->value = value;
		if(is_pt){
			new_vi->ft_cnt += fmgr->positive_weight;
		}
		else{
			new_vi->no_ft_cnt += fmgr->negative_weight;
		}
		new_vi->next = last_value_item->next;
		last_value_item->next = new_vi;
	}
}

static ft_element_t *ft_collector_get(ft_collector_t *cc, const char *ft_name)
{
	return (ft_element_t *)hashmap_get(cc->ft_map, ft_name);
}

static ft_element_t *ft_manager_add_into_buffer(ft_manager_t *fev, const char *ft_name)
{
	if(ft_name[0] == '\0'){
		return NULL;
	}

	int i_bucket = key_to_hash_bucket_index(ft_name, fev->new_ft_buffer);

	ft_element_t *ft_elem = (ft_element_t *)hashmap_get(fev->new_ft_buffer, i_bucket, ft_name);

	if(NULL == ft_elem){
		ft_elem = (ft_element_t *)nodepool_get(&fev->buffer_nodepool);
		if(ft_elem){
			if(fev->isTraining){
				memset(ft_elem, 0, sizeof(ft_element_t));
			}
			snprintf(ft_elem->ft_name, sizeof(ft_elem->ft_name), "%s", ft_name);
			hashmap_put(fev->new_ft_buffer, i_bucket, ft_elem->ft_name, ft_elem);
		}
	}

	return ft_elem;
}

static ft_element_t *ft_collector_add(ft_collector_t *cc, const char *ft_name)
{
	if(ft_name[0] == '\0'){
		return NULL;
	}

	ft_element_t *ft_elem = (ft_element_t *)hashmap_get(cc->ft_map, ft_name);
	if(NULL == ft_elem){
		ft_elem = (ft_element_t *)nodepool_get(&cc->ft_nodepool);
		assert(ft_elem);
		memset(ft_elem, 0, sizeof(ft_element_t));
		snprintf(ft_elem->ft_name, sizeof(ft_elem->ft_name), "%s", ft_name);
		hashmap_put(cc->ft_map, ft_elem->ft_name, ft_elem);
		cc->ft_num ++;
	}
	return ft_elem;
}

static ft_element_t *ft_collector_add(ft_collector_t *cc, 
		const char *ft_name, 
		bool is_pt,
		bool is_continue, int value,
		ft_manager_t *fmgr)
{
	ft_element_t *ft_elem = ft_collector_add(cc, ft_name);

	if(is_pt){
		ft_elem->freq += fmgr->positive_weight;
		ft_elem->cond_freq += fmgr->positive_weight;
	}
	else{
		if(strcmp(ft_elem->ft_name, "N_LINE") == 0){
			//printf("%lf + %lf = ", ft_elem->freq, fmgr->negative_weight);
		}
		ft_elem->freq += fmgr->negative_weight;
		if(strcmp(ft_elem->ft_name, "N_LINE") == 0){
			//printf("%lf\n", ft_elem->freq);
		}
	}

	if(strcmp(ft_elem->ft_name, "N_LINE") == 0){
		//printf("__%s\n", ft_elem->ft_name);
		//printf("%lf %lf\n", ft_elem->cond_freq, ft_elem->freq);
		for(value_item_t *vi = ft_elem->value_item_head.next; vi; vi = vi->next){
			//printf("%d %lf %lf\n", vi->value, vi->ft_cnt, vi->no_ft_cnt);
		}
	}

	if(is_continue){
		ft_elem->is_continue = 1;
		add_value_item(ft_elem, value, is_pt, cc, fmgr);
	}

	return ft_elem;
}

static void get_continue_bound(double &cond_freq, double &freq, ft_element_t *ft_elem, int value)
{
	for(discret_item_t *di = ft_elem->discret_item_head.next; di; di=di->next){
		if(value <= di->upper_bound){
			cond_freq = di->cond_freq;
			freq = di->freq;
			return;
		}
	}
}

static void prob_val_update(probability_val_t *ft_val, ft_element_t *ft_elem, ft_collector_t *fts,
		double min_ft_cond_probability, bool isClassic, bool is_clean_observ_value)
{
	if(ft_elem->freq <= 0){
		return;
	}

	double cond_freq = 0;
	double freq = 0;
	if(ft_elem->is_continue){
		get_continue_bound(cond_freq, freq, ft_elem, ft_elem->observ_value);
	}
	else{
		if(ft_elem->observ_value > 0){
			freq = ft_elem->freq;
			cond_freq = ft_elem->cond_freq;
		}
		else{
			if(isClassic){
				freq = fts->uni_freq - ft_elem->freq;
				cond_freq = fts->pt_freq - ft_elem->cond_freq;
			}
			else{
				return;
			}
		}
	}

	/** 在目标类别上的条件概率 */
	double px_on_d = (double)cond_freq;
	px_on_d /= (double)fts->pt_freq;

	/** 在非目标类别上的条件概率 */
	double px_on_nd = (double)freq - cond_freq;
	px_on_nd /= fts->uni_freq - fts->pt_freq;
	
	Debug("[%s] %d like_val:[%lf/%lf]%lf, unlikely_val:[%lf/%lf]%lf",
			ft_elem->ft_name, ft_elem->observ_value, 
			cond_freq, fts->pt_freq, px_on_d,
			freq - cond_freq, fts->uni_freq - fts->pt_freq, px_on_nd);

	if(px_on_d < min_ft_cond_probability){
		px_on_d = min_ft_cond_probability;
	}

	ft_val->positive_probability *= 10 * px_on_d;
	ft_val->negative_probability *= 10 * px_on_nd;
	Debug("[%s] %d current prob:%lf", ft_elem->ft_name, ft_elem->observ_value,
			ft_val->positive_probability/(ft_val->positive_probability+ft_val->negative_probability));

	if(is_clean_observ_value){
		ft_elem->observ_value = 0;
	}
}

/**
 * @brief 添加一个特征.
 * @param [in/out] fev   : ft_manager_t* 特征管理器.
 * @param [in] ft_name   : const char* 特征名.
 * @param [in] isContinue   : bool 是否连续.
 * @param [in] value   : int 特征的值. 对于非连续特征,调用此接口表示特征出现.
**/
void ft_manager_add(ft_manager_t *fev, const char *ft_name, bool isContinue, int value)
{
	if(strcmp(ft_name, SEED_FEATURE) != 0){
		ft_element_t *new_ft_elem = ft_manager_add_into_buffer(fev, ft_name);
		if(new_ft_elem && isContinue){
			new_ft_elem->is_continue = 1;
			new_ft_elem->observ_value = value;
		}
	}

	ft_element_t *ft_elem = ft_collector_get(fev->last_fts, ft_name);
	if(ft_elem){
		if(isContinue){
			ft_elem->observ_value = value;
		}
		else{
			ft_elem->observ_value = 1; 
		}
	}
}

static void probability_val_init(probability_val_t *ft_val, ft_collector_t *fts)
{
	ft_val->positive_probability = 0.5;
	if(fts->uni_freq > 0 && fts->pt_freq > 0){
		ft_val->positive_probability = (double)fts->pt_freq / fts->uni_freq;
	}

	ft_val->negative_probability = 1.0 - ft_val->positive_probability;
	ft_val->negative_probability *= 1000;
	ft_val->positive_probability *= 1000;
}

static void compute_likelihood(probability_val_t *pval, ft_manager_t *fev, bool is_clean_observ_value)
{
	probability_val_init(pval, fev->last_fts);

	hashmap_iter_begin(fev->last_fts->ft_map);
	while(1){
		ft_element_t *ft_elem = (ft_element_t *)hashmap_iter_next(fev->last_fts->ft_map);
		if(NULL == ft_elem){
			break;
		}
		
		prob_val_update(pval, ft_elem, fev->last_fts, fev->min_ft_cond_probability, fev->isClassic,
				is_clean_observ_value);
	}
}

static bool has_seed_feature(ft_manager_t *fmgr)
{
	ft_element_t *ft_elem = (ft_element_t *) hashmap_get(fmgr->last_fts->ft_map, SEED_FEATURE);
	if (ft_elem && ft_elem->observ_value == 1){
		ft_elem->observ_value = 0;
		return true;
	}

	return false;
}

/**
 * @brief 当前个体是否属于目标类别.
 * @param [in] fev   : ft_manager_t* 特征管理器.
 * @param [in] isNormalize   : bool 是否根据特征数量进行规范化, 以避免偏袒特征多的个体.
**/
static bool is_target_type(ft_manager_t *fev, bool isNormalize, bool is_clean_observ_value)
{
	if(fev->isTraining && !fev->isBootstrap){
		return has_seed_feature(fev);
	}

	bool is_target = false;

	probability_val_t *pval = &fev->probability_val;

	compute_likelihood(pval, fev, is_clean_observ_value);

	int ft_num = hashmap_get_element_num(fev->new_ft_buffer);
	pval->negative_probability += 1e-38; /** 加一个极小的浮点数,避免为0 */

	if(isNormalize && ft_num > 0){
		double normal_prob = log(pval->positive_probability) - log(pval->negative_probability);
		normal_prob /= ft_num;
		double threshold = log(fev->p_bootstrap) - log(1.0 - fev->p_bootstrap);
		Debug("normal_prob=%lf threshold=%lf", normal_prob, threshold);
		if(normal_prob > threshold){
			is_target = true;
		}
	}
	else{
		double prob 
			= pval->positive_probability / (pval->positive_probability + pval->negative_probability);
		Debug("probability=%lf", prob);

		if(prob > fev->p_bootstrap){
			is_target = true;
		}
	}

	return is_target;
}

bool is_target_type(ft_manager_t *fev, bool isNormalize)
{
	return is_target_type(fev, isNormalize, true);
}

bool peek_target_type(ft_manager_t *fev, bool isNormalize)
{
	return is_target_type(fev, isNormalize, false);
}

/**
 * @brief 获取属于目标类别的概率.
 * 	XXX:此接口在调用is_target_type()之后调用才有意义.
 * @param [in] fmgr   : ft_manager_t* 特征管理器.
 * @param [in] isNormalize   : bool 是否根据特征数量对概率进行规范化.
 * @return  double 返回概率值, 介于0到1.
**/
double get_target_type_probability(ft_manager_t *fmgr, bool isNormalize)
{
	probability_val_t *pval = &fmgr->probability_val;
	if(isNormalize){
		double normal_prob = pval->positive_probability / pval->negative_probability;
		normal_prob = pow(normal_prob, 1.0/(double)hashmap_get_element_num(fmgr->new_ft_buffer));
		normal_prob = normal_prob / (1.0 + normal_prob);
		return normal_prob;
	}
	else{
		double prob 
			= pval->positive_probability / (pval->positive_probability + pval->negative_probability);
		return prob;
	}
}

/**
 * @brief 清空对当前个体收集的信息, 表明当前个体的情况不参于统计.
**/
void ft_manager_new_fts_clean(ft_manager_t *fev)
{
	hashmap_clean(fev->new_ft_buffer);
	nodepool_reset(&fev->buffer_nodepool);
}

static void ft_manager_buffer_to_new_fts(ft_manager_t *fev, bool is_pt)
{
	hashmap_iter_begin(fev->new_ft_buffer);
	while(1){
		ft_element_t *ft_elem = (ft_element_t *)hashmap_iter_next(fev->new_ft_buffer);
		if(ft_elem == NULL){
			break;
		}
		ft_collector_add(fev->new_fts, ft_elem->ft_name, is_pt, 
				ft_elem->is_continue, ft_elem->observ_value, fev);
	}

	ft_manager_new_fts_clean(fev);
}

/**
 * @brief 调用此接口,表明已完成当前个体信息的收集,进行必要的统计工作.
 * @param [in] is_target_type   : bool 当前个体是否目标类型.
**/
void ft_manager_finish_an_entity(ft_manager_t *fev, bool is_target_type)
{
	if(fev->isTraining){
		if(is_target_type){
			fev->new_fts->uni_freq += fev->positive_weight;
			fev->new_fts->pt_freq += fev->positive_weight;
		}
		else{
			fev->new_fts->uni_freq += fev->negative_weight;
		}
		ft_manager_buffer_to_new_fts(fev, is_target_type);
	}

	ft_manager_new_fts_clean(fev);
}

static const char *scan_non_space(const char *str)
{
	while(*str && !isspace(*str)){
		str++;
	}
	return str;
}

static const char *scan_space(const char *str)
{
	while(isspace(*str)){
		str++;
	}
	return str;
}

/**
 * @brief 扫描n个非空字符串.
**/
static const char *scanf_n(const char *str, int n)
{
	while(*str && n > 0){
		if(!isspace(*str)){
			str = scan_non_space(str);
			n--;
		}
		else{
			str ++;
		}
	}

	str = scan_space(str);

	return str;
}

static const char *SPACE_ESCAPE_CHAR = "&nbsp;";		  /**< 空格的转义字符串 */

/**
 * @brief 对字符串中的特殊字符进行转义.
**/
static void reference_chars(char *des_str, int des_size, const char *src)
{
	int i = 0;
	for(const char *p = src; *p ; p++){
		if(i >= des_size - 1){
			break;
		}
		if(*p == ' '){
			if(i < des_size - (int)strlen(SPACE_ESCAPE_CHAR)){
				snprintf(des_str+i, des_size-i, "%s", SPACE_ESCAPE_CHAR);
				i += strlen(SPACE_ESCAPE_CHAR);
			}
		}
		else{
			des_str[i++] = *p;
		}
	}
	des_str[i] = '\0';
}

/**
 * @brief 将含转义字符的字符串进行反转义.
**/
static void inverse_reference_chars(char *des_str, int des_size, const char *src)
{
	Debug("before inver refer:[%s]", src);
	int i = 0;
	for(const char *p = src; *p; ){
		if(i >= des_size - 1){
			break;
		}
		if(strncmp(p, SPACE_ESCAPE_CHAR, strlen(SPACE_ESCAPE_CHAR)) == 0){
			des_str[i++] = ' ';
			p += strlen(SPACE_ESCAPE_CHAR);
		}
		else{
			des_str[i++] = *p++;
		}
	}
	des_str[i] = '\0';
	Debug("after inver refer:[%s]", des_str);
}

static int buffer_sscanf(const char * &src, const char *fmt, ...)
{
    va_list args;
    va_start(args,fmt);
	
	int n_scan = vsscanf(src, fmt, args);

    va_end(args);

	src = scanf_n(src, n_scan);

    return n_scan;
}

static bool is_conf_key(const char *stream, const char *key)
{
	return strncmp(stream, key, strlen(key)) == 0;
}

static void read_evolution_conf(ft_manager_t *fev, const char *conf_str)
{
	int ft_num = 0;
	double pdx = 0;
	double pxd = 0;
	double pxnd = 0;
	const char *p_conf_str = conf_str;
	
	buffer_sscanf(p_conf_str, "isBootstrap=%d\np_bootstrap=%lf\nft_threshold=%lf\nisClassic=%d\n", 
			&fev->isBootstrap, &fev->p_bootstrap, &fev->ft_threshold, &fev->isClassic);
	buffer_sscanf(p_conf_str, "min_freq_to_discret=%d\nhidden_num=%d\nmin_ft_freq=%d min_cond_p=%lf\n", 
			&fev->min_freq_to_discret,&fev->hidden_num,&fev->min_ft_freq,&fev->min_ft_cond_probability);
	if(is_conf_key(p_conf_str, "positive_weight")){
		buffer_sscanf(p_conf_str, "positive_weight=%lf\n", &fev->positive_weight);
	}
	else{
		fev->positive_weight = 1;
	}
	if(is_conf_key(p_conf_str, "negative_weight")){
		buffer_sscanf(p_conf_str, "negative_weight=%lf\n", &fev->negative_weight);
	}
	else{
		fev->negative_weight = 1;
	}
	ft_collector_t *fts = fev->last_fts;
	buffer_sscanf(p_conf_str, "posive_freq=%lf\nfreq=%lf\nraw_ft_num=%d\n", &fts->pt_freq, &fts->uni_freq, &ft_num);

	int index = 1;
	for(; ; ){
		char ft_name[MAX_FEATURE_NAME_LEN];
		ft_name[0] = '\0';
		int is_continue = 0;

		int n_scan = buffer_sscanf(p_conf_str, "%s %d", ft_name, &is_continue);
		if(n_scan <= 0){
			break;
		}
		char name_buffer[MAX_FEATURE_NAME_LEN];
		name_buffer[0] = '\0';
		inverse_reference_chars(name_buffer, sizeof(name_buffer), ft_name);

		ft_element_t *ft_elem = ft_collector_add(fts, name_buffer);
		ft_elem->is_continue = is_continue;
		ft_elem->index = index ++;

		if(!ft_elem->is_continue){
			buffer_sscanf(p_conf_str, "%lf %lf %lf %lf %lf\n", &(ft_elem->freq), &(ft_elem->cond_freq),
					&pdx,&pxd,&pxnd);
		}
		else{
			buffer_sscanf(p_conf_str, "%d", &(ft_elem->discret_num));
			discret_item_t *tail = &(ft_elem->discret_item_head);
			for(int j = 0; j < ft_elem->discret_num; j++){
				int upper_bound = 0;
				double freq = 0, cond_freq = 0;
				double cond_on_freq = 0.0;
				buffer_sscanf(p_conf_str, "%d %lf %lf %lf", &upper_bound, &freq, &cond_freq,
						&cond_on_freq);
				discret_item_t *di = (discret_item_t *)nodepool_get(&fts->dis_nodepool);
				assert(di);
				di->upper_bound = upper_bound;
				di->freq = freq;
				di->cond_freq = cond_freq;
				di->next = tail->next;
				tail->next = di;
				tail = di;
				ft_elem->freq += freq;
			}
		}
	}
}

/**
 * @brief 计算熵.
**/
static double get_class_ent(double pt_num, double npt_num)
{
	double ppt = (double)pt_num / (pt_num + npt_num);
	double pnpt = 1-ppt;
	double entS = 0.0;
	if(pt_num > 0){
		entS += (0-ppt) * log(ppt);
	}
	if(npt_num > 0){
		entS += (0-pnpt) * log(pnpt); 
	}
	return entS;
}

static bool is_discrtable_between(value_item_t *last_vi, value_item_t *vi)
{
	if(last_vi->ft_cnt > 0 && last_vi->no_ft_cnt > 0){
		return true;
	}

	if(vi->ft_cnt > 0 && vi->no_ft_cnt > 0){
		return true;
	}

	if(last_vi->ft_cnt == 0 && vi->ft_cnt > 0){
		return true;
	}

	if(last_vi->no_ft_cnt == 0 && vi->no_ft_cnt > 0){
		return true;
	}

	return false;
}

static int mid_prefer_min(int a, int b)
{
	switch(a - b){
		case -1:
			return a;
		case 1:
			return b;
		default:
			return (a + b) / 2;
	}
}

static discret_item_t *discret_range(ft_element_t *ft_ele, int lower_bound, int upper_bound,
		ft_collector_t *fts, int min_freq_to_discret, ft_manager_t *fmgr)
{
	double pt_num = 0;
	double npt_num = 0;
	int cnt_val = 0;
	value_item_t *lower_vi = NULL;
	value_item_t *upper_vi = NULL;
	for(value_item_t *vi = ft_ele->value_item_head.next; vi; vi=vi->next){
		if(vi->value > upper_bound){
			break;
		}
		if(vi->value > lower_bound){
			if(lower_vi == NULL){
				lower_vi = vi;
			}
			pt_num += vi->ft_cnt;
			npt_num += vi->no_ft_cnt;
			cnt_val ++;
		}
		upper_vi = vi;
	}

	double entS = get_class_ent(pt_num, npt_num);

	if(lower_vi == NULL || lower_vi == upper_vi){
		return NULL;
	}

	value_item_t *last_vi = lower_vi;
	double sub_pt_num = lower_vi->ft_cnt;
	double sub_npt_num = lower_vi->no_ft_cnt;
	double max_ent_gain = 0.0;
	discret_item_t *max_gain_di = NULL;
	for(value_item_t *vi = lower_vi->next; vi; vi=vi->next){
		if(is_discrtable_between(last_vi, vi)){
			double ent_s1 = get_class_ent(sub_pt_num, sub_npt_num);
			double ent_s2 = get_class_ent(pt_num-sub_pt_num, npt_num-sub_npt_num);
			double s1_num = sub_pt_num + sub_npt_num;
			double s2_num = pt_num + npt_num - s1_num;
			double s_num = s1_num + s2_num;
			double ent_sep = ent_s1 * s1_num / s_num + ent_s2 * s2_num / s_num;
			int k1 = 1;
			int k2 = 1;
			if(sub_pt_num > 0 && sub_npt_num > 0){
				k1 = 2;
			}
			if(pt_num - sub_pt_num > 0 && npt_num - sub_npt_num > 0){
				k2 = 2;
			}

			double ent_gain = entS - ent_sep;
			double threshold = log((double)(cnt_val-1)) / s_num
				+ (log((double)(3*3-2)) - 2 * entS + k1 * ent_s1 + k2 * ent_s2)/s_num;
			if(ent_gain > threshold && ent_gain > max_ent_gain 
					&& s1_num >= min_freq_to_discret && s2_num >= min_freq_to_discret){
				discret_item_t *di = (discret_item_t *)nodepool_get(&fts->dis_nodepool);
				if(di){
					di->cond_freq = sub_pt_num;
					di->freq = sub_pt_num + sub_npt_num;
					di->upper_bound = mid_prefer_min(last_vi->value, vi->value);
					di->next = NULL;
					if(max_gain_di){
						nodepool_put(&fts->dis_nodepool, max_gain_di);
					}
					max_gain_di = di;
					max_ent_gain = ent_gain;
				}
			}
		}

		sub_pt_num += vi->ft_cnt;
		sub_npt_num += vi->no_ft_cnt;

		if(vi == upper_vi){
			break;
		}
		last_vi = vi;
	}

	return max_gain_di;
}

static void discret_ft_print(ft_element_t *ft_ele, ft_collector_t *fts)
{
	double pd = 0;
	if(fts->uni_freq > 0){
		pd = (double)fts->pt_freq / fts->uni_freq;
	}
	printf("_%s:", ft_ele->ft_name);
	for(discret_item_t *di = ft_ele->discret_item_head.next;
			di; di=di->next){
		double px = 0;
		if(fts->uni_freq > 0){
			px = (double)di->freq / fts->uni_freq;
		}
		double pxd = 0; 
		if(fts->pt_freq > 0){
			pxd = (double)di->cond_freq / fts->pt_freq;
		}
		double pdx = pxd * pd/px;
		printf("<=%d: %lf %lf %lf %lf\n", di->upper_bound, di->cond_freq, di->freq,
				pdx, pdx/pd);
	}
	printf("\n");
}

/**
 * @brief 对连续属性, 利用基于信息增益的办法对其进行分段.
**/
static void discret_ft_ele(ft_element_t *ft_ele, ft_collector_t *fts, int min_freq_to_discret,
		ft_manager_t *fmgr)
{
	discret_item_t *tail_di = (discret_item_t *)nodepool_get(&fts->dis_nodepool);
	tail_di->upper_bound = INT_MAX;
	tail_di->freq = ft_ele->freq;
	tail_di->cond_freq = ft_ele->cond_freq;
	tail_di->next = ft_ele->discret_item_head.next;
	assert(tail_di->next == NULL);
	ft_ele->discret_num = 1;
	ft_ele->discret_item_head.next = tail_di;
	//printf("%s\n", ft_ele->ft_name);
	//printf("%lf %lf\n", ft_ele->cond_freq, ft_ele->freq);
	for(value_item_t *vi = ft_ele->value_item_head.next; vi; vi = vi->next){
		//printf("%d %lf %lf\n", vi->value, vi->ft_cnt, vi->no_ft_cnt);
	}
	//printf("~~~\n");
	while(1){
		int lower_bound = INT_MAX+1;
		discret_item_t *last_di = &ft_ele->discret_item_head;
		bool is_dis = false;
		for(discret_item_t *di = ft_ele->discret_item_head.next; di; di=di->next){
			int upper_bound = di->upper_bound;
			discret_item_t * new_di 
				= discret_range(ft_ele, lower_bound, upper_bound, fts, min_freq_to_discret, fmgr);
			if(new_di){
				new_di->next = di;
				last_di->next = new_di;
				di->cond_freq -= new_di->cond_freq;
				di->freq -= new_di->freq;
				is_dis = true;
				ft_ele->discret_num ++;
			}
			last_di = di;
			lower_bound = last_di->upper_bound;
		}
		for(discret_item_t *di = ft_ele->discret_item_head.next; di; di=di->next){
			//printf("<=%d: %lf %lf\n", di->upper_bound, di->cond_freq, di->freq);
		}
		//printf("------\n");
		if(!is_dis){
			break;
		}
	}

	discret_ft_print(ft_ele, fts);
}

static void discret_continue_ft(ft_collector_t *fts, int min_freq_to_discret, ft_manager_t *fmgr)
{
	hashmap_iter_begin(fts->ft_map);
	while(1){
		ft_element_t *ele = (ft_element_t *)hashmap_iter_next(fts->ft_map);
		if(ele == NULL){
			break;
		}
		if(ele->is_continue){
			discret_ft_ele(ele, fts, min_freq_to_discret, fmgr);
		}
	}
}

/**
 * @brief 计算X方值.
 * 	X方值是计算一个特征出现与否对条件概率改变的大小.
 * 	在一个特征出现与不出现的情况下, 类别的概率变化越大, 则这个特征越重要.
 * 	这里利用与理想特征X方值的比值进行了归一化, 即这里计算出的值的范围为[0,1].
 * 	理想特征是指这样的特征:样本属于目标类别当且仅当该特征出现.
**/
static double get_x2_value(ft_collector_t *fts, ft_element_t *ft_elem)
{
	double a = (double)ft_elem->cond_freq;
	double b = (double)ft_elem->freq - a;
	double c = (double)(fts->pt_freq - ft_elem->cond_freq);
	double d = (double)fts->uni_freq - ft_elem->freq - c;
	
	double x2 = 0;
	if(a + b > 0 && c + d > 0){
		double s = a * d - b * c;
		x2 = s / (a+b);
		x2 *= s / (c+d);
		x2 /= (a + c);
		x2 /= (b + d);
	}

	fprintf(stderr, "%s %lf %lf %lf %lf %lf\n", ft_elem->ft_name, x2, a, b, c, d);
	return x2;
}

/**
 * @brief 输出新的分类规则.
**/
static void print_newfts(const char *newfts_file, ft_manager_t *fev)
{
	FILE *f = fopen(newfts_file, "w");
	assert(f);

	ft_collector_t *new_fts = fev->new_fts;

	fprintf(f, "isBootstrap=%d\np_bootstrap=%lf\nft_threshold=%lf\nisClassic=%d\n", 
			1, fev->p_bootstrap, fev->ft_threshold, fev->isClassic);
	fprintf(f, "min_freq_to_discret=%d\nhidden_num=%d\nmin_ft_freq=%d\nmin_cond_p=%lf\n", fev->min_freq_to_discret, fev->hidden_num, fev->min_ft_freq, fev->min_ft_cond_probability);
	fprintf(f, "positive_weight=%lf\n", fev->positive_weight);
	fprintf(f, "negative_weight=%lf\n", fev->negative_weight);
	fprintf(f, "posive_freq=%lf\nfreq=%lf\nraw_ft_num=%d\n", new_fts->pt_freq, new_fts->uni_freq, new_fts->ft_num);

	double pd = 0;
	if(new_fts->uni_freq > 0){
		pd = (double)new_fts->pt_freq / new_fts->uni_freq;
	}

	new_fts->pt_freq += fev->hidden_num *pd;
	new_fts->uni_freq += fev->hidden_num;

	int ft_cnt = 0;
	hashmap_iter_begin(new_fts->ft_map);

	while(1){
		ft_element_t *ft_elem = (ft_element_t *)hashmap_iter_next(new_fts->ft_map);
		if(ft_elem == NULL){
			break;
		}
		ft_cnt ++;

		if(ft_elem->freq < fev->min_ft_freq){
			continue;
		}

		ft_elem->freq += fev->hidden_num;
		ft_elem->cond_freq += fev->hidden_num * pd;

		double presc_ins = get_x2_value(new_fts, ft_elem);

		double px = 0;
		if(new_fts->uni_freq > 0){
			px = (double)ft_elem->freq / new_fts->uni_freq;
		}
		double pxd = 0; 
		if(new_fts->pt_freq > 0){
			pxd = (double)ft_elem->cond_freq / new_fts->pt_freq;
		}
		double pdx = pxd * pd/px;
		bool choose_ft = false;
		if(ft_elem->is_continue && ft_elem->discret_num > 1){
			choose_ft = true;
		}
		if(!ft_elem->is_continue && presc_ins >= fev->ft_threshold){
			choose_ft = true;
		}

		char name_buffer[MAX_FEATURE_NAME_LEN];
		name_buffer[0] = '\0';
		reference_chars(name_buffer, sizeof(name_buffer), ft_elem->ft_name);

		fprintf(stderr, "%s %lf %lf\n", name_buffer, presc_ins, pdx/pd);

		if(choose_ft){
			if(!ft_elem->is_continue){
				fprintf(f, "%s 0 %lf %lf %lf %lf %lf\n", 
						name_buffer, 
						ft_elem->freq, ft_elem->cond_freq,
						pdx, pxd, (px-pxd*pd)/(1-pd));
			}
			else{
				fprintf(f, "%s 1 %d\n", name_buffer, ft_elem->discret_num);
				for(discret_item_t *di = ft_elem->discret_item_head.next; di; di = di->next){
					fprintf(f, "\t%d %lf %lf %lf\n", di->upper_bound, di->freq, di->cond_freq,
							(double)di->cond_freq/di->freq);
				}
			}
			ft_elem->selected = 1;
		}
	}

	assert(ft_cnt == new_fts->ft_num);

	fclose(f);
}

/**
 * @brief 根据配置字符串初始化特征管理器.
 * @param [in] conf_str   : const char* 特征配置文本.
 * @param [in] isTraining   : bool 是否处于训练阶段.
 * @retval   若失败返回NULL,否则返回创建的特征管理器.
**/
ft_manager_t *ft_manager_init_from_string(const char *conf_str, bool isTraining)
{
	ft_manager_t *ft_manager = ft_manager_create(isTraining);
	if(NULL == ft_manager){
		Fatal("ft_manager create error!");
		return NULL;
	}

	read_evolution_conf(ft_manager, conf_str);
	
	if(!ft_manager->isBootstrap && isTraining){
		ft_collector_add(ft_manager->last_fts, SEED_FEATURE); // add the seed feature.
	}

	//ft_manager->svm = svm_load_model("data.t.model");
	//ft_manager->x = (svm_node *)calloc(ft_manager->last_fts->ft_num + 1, sizeof(svm_node));

	return ft_manager;
}

/**
 * @brief 根据配置文件初始化特征管理器.
 * @param [in] conf_file   : const char* 特征配置文件.
 * @param [in] isTraining   : bool 是否处于训练阶段.
 * @retval   若失败返回NULL,否则返回创建的特征管理器.
**/
ft_manager_t *ft_manager_init(const char *conf_file, bool isTraining)
{
	char conf_str[MAX_FT_CONF_FILE_LEN];
	conf_str[0] = '\0';

	FILE *fin = fopen(conf_file, "r");
	assert(fin);

	int l_read = fread(conf_str, 1, MAX_FT_CONF_FILE_LEN, fin);
	conf_str[l_read] = '\0';
	
	fclose(fin);

	return ft_manager_init_from_string(conf_str, isTraining);
}

/**
 * @brief 终止特征管理.
 *  在训练阶段, 输出这一轮迭代得到的特征.
 *  销毁特征管理结构.
 * @param [in/out] out_file   : const char* 用于训练阶段将训练出的信息输出到文件.
**/
void ft_manager_end(const char *out_file, ft_manager_t *fev)
{
	if(fev->isTraining){
		discret_continue_ft(fev->new_fts, fev->min_freq_to_discret, fev);
		print_newfts(out_file, fev);
	}

	ft_manager_destroy(fev);
}


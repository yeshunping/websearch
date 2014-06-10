/**
 * easou_ahtml_switch.h
 * Description:
 *  Created on: 2011-11-18
 * Last modify: 2012-11-10 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#ifndef EASOU_AHTML_SWITCH_H_
#define EASOU_AHTML_SWITCH_H_

/**
* @brief 分块的资源类型.
* XXX:命名规则为AREA_SRCTYPE_*.
*/
typedef enum _html_area_srctype_t {
	AREA_SRCTYPE_UNDEFINED=0 , 	//必须是第一个
	AREA_SRCTYPE_OUT ,		  	/**< 外部块       */
	AREA_SRCTYPE_INTERACTION, 	/**< 交互块       */
	AREA_SRCTYPE_PIC,		  	/**< 图片块       */
	AREA_SRCTYPE_LINK,		  	/**< 链接块:索引块的子集,链接所占比例较大的索引块 */
	AREA_SRCTYPE_HUB,		  	/**< 索引块:以链接为中心的重复结构  */
	AREA_SRCTYPE_HUB_SUBUNIT,	/**< 组成索引块的子重复结构 */
	AREA_SRCTYPE_TEXT,		  	/**< 文本块       */
	AREA_SRCTYPE_LASTFLAG  		//必须是最后一个
}html_area_srctype_t;

/**
* @brief 分块的功能类型.
* XXX:命名规则为AREA_FUNC_*.
*/
typedef enum _html_area_func_t
{
	AREA_FUNC_UNDEFINED=0 ,//必须是第一个
	AREA_FUNC_TIME,		  /**< 时间      */
	AREA_FUNC_TURNPAGE,		/**< 翻页链接块		*/
	AREA_FUNC_COPYRIGHT,		  /**< 版权块      */
	AREA_FUNC_NAV,		  /**< 导航条       */
	AREA_FUNC_FRIEND,		  /**< 友情链接       */
	AREA_FUNC_RELATE_LINK,		  /**< 相关链接       */
	AREA_FUNC_MYPOS,		  /**< 大名鼎鼎的MYPOS       */
	AREA_FUNC_ARTICLE_SIGN,		  /**< 标题下方的文章来源、作者、时间等       */
	AREA_FUNC_SUBTITLE,		  /**< 小标题       */
	AREA_FUNC_SUBTIT_4RT,		  /**<　辅助realtitle的小标题       */
	AREA_FUNC_CANDI_SUBTIT_4RT,	 /**< 辅助realtitle小标题       */
	AREA_FUNC_ARTICLE_CONTENT, /**< 文章内容块       */
	AREA_FUNC_LASTFLAG	//必须是最后一个
}html_area_func_t ;

/**
* @brief 分块的语义类型.
* XXX:命名规则为AREA_SEM_*.
*/
typedef enum _html_area_sem_t {
	AREA_SEM_UNDEFINED=0 ,		//必须是第一个
	AREA_SEM_REALTITLE,		  	/**< 网页内容中的标题  */
	AREA_SEM_CENTRAL,		  	/**< 核心内容块       */
	AREA_SEM_HUB_CENTRAL,       /**hub 核心内容块*/
	AREA_SEM_LASTFLAG 			//必须是最后一个
}html_area_sem_t;

static const int N_AREA_SRC_TYPE = AREA_SRCTYPE_LASTFLAG - AREA_SRCTYPE_UNDEFINED;/**< 资源类型的个数 */
static const int N_AREA_FUNC_TYPE = AREA_FUNC_LASTFLAG - AREA_FUNC_UNDEFINED;/**< 功能类型的个数 */
static const int N_AREA_SEM_TYPE = AREA_SEM_LASTFLAG - AREA_SEM_UNDEFINED;/**< 语义类型的个数 */
/**
* @brief 功能开关.
*/
typedef struct _function_switch_t {
	unsigned char is_set_select:1;		  /**< 是否设置了选择项 */
	unsigned char shutted_pos:1;		  /**< 是否关闭了位置标注 */
	unsigned char select_pos:1;		      /**< 是否选择了位置标注 */
	unsigned char shutted_base_info:1;		  /**< 是否关闭了基本信息计算 */
	unsigned char select_base_info:1;		  /**< 是否选择了基本信息计算 */
	unsigned char shutted_srctype[N_AREA_SRC_TYPE];		  /**< 是否关闭了特定的资源类型 */
	unsigned char select_srctype[N_AREA_SRC_TYPE];		  /**< 是否选择了特定的资源类型 */
	unsigned char shutted_func[N_AREA_FUNC_TYPE];		  /**< 是否关闭了特定的功能类型 */
	unsigned char select_func[N_AREA_FUNC_TYPE];		  /**< 是否选择了特定的功能类型 */
	unsigned char shutted_sem[N_AREA_SEM_TYPE];		  /**< 是否关闭了特定的语义类型 */
	unsigned char select_sem[N_AREA_SEM_TYPE];		  /**< 是否选择了特定的语义类型 */
} function_switch_t;

extern function_switch_t g_function_switch;

/**
 * @brief	初始化功能开关
**/
function_switch_t *function_switch_create();
/**
 * @brief	销毁功能开关
**/
void function_switch_destroy(function_switch_t *fs);

#endif /* EASOU_AHTML_SWITCH_H_ */

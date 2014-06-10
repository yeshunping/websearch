/**
 * easou_mark_conf.cpp
 * Description:
 *  Created on: 2011-11-22
 * Last modify: 2012-11-10 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.1
 */
#include "easou_mark_srctype.h"
#include "easou_mark_func.h"
#include "easou_mark_sem.h"
#include "easou_mark_conf.h"

/**
* @brief 增加新的资源标注类型，修改此定义.
*/
const mark_srctype_handler_t srctype_mark_handler[] = {
	{NULL, AREA_SRCTYPE_UNDEFINED, "undefine"},
	{mark_other_srctypes, AREA_SRCTYPE_OUT, "out"},
	{mark_srctype_interaction, AREA_SRCTYPE_INTERACTION, "interaction"},
	{mark_srctype_picture, AREA_SRCTYPE_PIC, "pic"},
	{mark_srctype_link, AREA_SRCTYPE_LINK, "link"},
	{mark_srctype_link, AREA_SRCTYPE_HUB, "hub"},
	{mark_srctype_link, AREA_SRCTYPE_HUB_SUBUNIT, "subunit"},
	{mark_other_srctypes, AREA_SRCTYPE_TEXT, "text"},
};

const int N_SRCTYPE = sizeof(srctype_mark_handler) / sizeof(mark_srctype_handler_t);


/**
* @brief 要增加新的功能标注类型，修改此定义.
* 注意添加的顺序，若某标注类型依赖于别的标注类型，则此类型须在其依赖的标注类型之后.
*/
const mark_func_handler_t func_mark_handler[] = {
	{NULL,	AREA_FUNC_UNDEFINED,	"undefine"},
	{mark_func_time,	AREA_FUNC_TIME,	"time"},  //时间
	{mark_func_turnpage, AREA_FUNC_TURNPAGE, "turn_page"}, //翻页块
	{mark_func_copyright,	AREA_FUNC_COPYRIGHT,	"copyright"}, //版权
	{mark_func_mypos,	AREA_FUNC_MYPOS,	"mypos"}, //mypos
	{mark_func_relate_link,	AREA_FUNC_RELATE_LINK,	"relate-link"},//相关链接
	{mark_func_nav,	AREA_FUNC_NAV,	"nav"},	//nav
	{mark_func_friend,	AREA_FUNC_FRIEND,	"friend"}, //友情链接
	{mark_func_article_sign,	AREA_FUNC_ARTICLE_SIGN,	"article_sign"}, //文章来源，作者等
	{mark_func_subtitle,	AREA_FUNC_SUBTITLE,	"subtitle"},  //小标题
	{mark_func_subtit_forrt,	AREA_FUNC_SUBTIT_4RT,	"subtitle_forrt"},//辅助realtitle的小标题
	{mark_func_subtit_forrt,	AREA_FUNC_CANDI_SUBTIT_4RT,	"candi_subtitle_forrt"},//辅助realtitle的小标题
	{mark_func_article_content, AREA_FUNC_ARTICLE_CONTENT, "article_content" }, //文章内容块
};

/**
* @brief 配置的数量.
*/
const int N_FUNC = sizeof(func_mark_handler) / sizeof(mark_func_handler_t);

/**
* @brief 要增加新的语义标注类型，修改此定义.
* 注意添加的顺序，若某标注类型依赖于别的标注类型，则此类型须在其依赖的标注类型之后.
*/
const mark_sem_handler_t sem_mark_handler[] = {
	{NULL,	AREA_SEM_UNDEFINED,	"undefine"},
	{mark_sem_realtitle,	AREA_SEM_REALTITLE,	"realtitle"},
	{mark_sem_central,	AREA_SEM_CENTRAL,	"central"},
	{mark_sem_hub_central,	AREA_SEM_HUB_CENTRAL,	"hub_central"},
};

/**
* @brief 配置的数量.
*/
const int N_SEM = sizeof(sem_mark_handler) / sizeof(mark_sem_handler_t);

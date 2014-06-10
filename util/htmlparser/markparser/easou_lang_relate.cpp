/**
 * @file easou_lang_relate.cpp
 * @brief 语言相关部分.
 *
 **/
#include "easou_lang_relate.h"

const char * for_interaction[]=
{
	"登录" ,
	"注册" ,
	"密码" ,
	"用户" ,
	"用户名" ,
	"姓名" ,
	"电话" ,
	"手机" ,
	"email" ,
	"查询" ,
	"搜索" ,
	"搜寻" ,
	"留言" ,
	"选择" ,
	"关键字" ,
	"发表" ,
	"评论" ,
	"分享",
	0
} ;
//int for_interaction_size = sizeof(for_interaction)/sizeof(char *) ;

//友情链接包含以下关键字，可能性增大
const char * for_friend_link_inanchor[]=
{
	"网",
	"站",
	"论坛",
	"在线",
	"家",
	"公司",
	"热线",
	0
} ;
//int for_friend_link_inanchor_size = sizeof(for_friend_link_inanchor)/sizeof(char *) ;

//友情链接包含一下提示信息，可能性增大
const char * for_friend_link_title[]=
{
	"友情" ,
	"合作" ,
	"伙伴" ,
	0
};
//int for_friend_link_title_size = sizeof(for_friend_link_title)/sizeof(char *) ;

//友情链接中包含以下关键字，粒度划分不够
const char * for_friend_link_no[]=
{
	"copyright" ,
	"Copyright" ,
	"版权" ,
	"ICP" ,
	"Powered" ,
	"Version" ,
	"所有",
	0
} ;
//int for_friend_link_no_size = sizeof(for_friend_link_no)/sizeof(char *) ;


//相关链接包含以下关键字，可能性增大
const char * for_rel_link[]=
{
	"相关",
	"推荐",
	"热门",
	"精彩",
	"关于",//根据http://news.163.com/12/0509/10/812A2SCF0001121M.html
	0
} ;
//int for_rel_link_size = sizeof(for_rel_link)/sizeof(char *) ;

const char * hot_rank[]=
{
	"热门",
	"热点",
	"最新",
	"Top",
	"排行",
	0
} ;
//int hot_rank_size = sizeof(hot_rank)/sizeof(char *) ;

// mypos中的过滤词
const char *mypos_filter_words[] = {"打印本页","登录","注册","回复","发表",0};

const char *mypos_hint[] = {"位置",0};

const char *mypos_mark_filter_words[]
	= {"返回","打印","复制","收藏本页","收藏到","]]>","/>","下一条","下一页","上一条","上一页",0};

const char * topanchor[]
={"首页","门户",0} ;

const char * typeword[]
={"：","类型",":","：",0} ;

const char * no_nav_anchor[]
={"上一","下一","前一","后一","时间","更新","上篇","下篇","前篇","后篇","收藏","注册","登录","全部","不限","：","老师","店长","加为好友","发短消息","(",")","[","]","网站导航","返回首页","！","字号:","字体:",0} ;

const char *ARTI_SIGN_FEAT_TERM[] =
{ "作者", "时间", "日期", "发表", "来源", "來源", "稿源", "发布", "浏览", "电视台", ".com", ".cn", "报", " 网", 0 };

const char *realtit_plus_word[] = {"主题","标题","查看完整版本","查看完全版本", 0};

const char *COPYRIGHT_ANCHOR_TERM[]={
"联系我们","关于我们","友情链接","广告服务","网站地图","免责声明","帮助中心","联系方式",
"法律声明","服务条款","设为首页","加入收藏","诚聘英才","友情连接","版权声明","合作伙伴",
"站点地图","意见反馈","隐私声明","客服中心","网站导航","人才招聘","招聘信息","广告投放",
"渠道招商","会员服务","使用帮助","关于本站","版权申明","意见建议","常见问题","合作推广",
"管理登录","关于酷讯","网站简介","隐私保护","快捷面板","公司简介","交流论坛","站点存档",
"免责条款","空间列表","媒体报道","联络我们","付款方式","联系站长","服务项目","广告联系",
"客户服务","使用条款","用户协议","收藏本站","招贤纳士","广告合作","公司介绍","商务合作",
"网站律师","会员注册","人才招募","腾讯招聘","关于腾讯","隐私政策","使用协议","加入我们",
"公司动态","广告刊登","公告列表","法律顾问","图片主页","服务协议","用户注册","人员招聘",
"访客留言","欢迎投稿","网站声明","广告业务","网站首页","联系方法","广告报价","关於我们",
"配送方式","和讯部落","网站建设","工作机会","在线留言","返回首页","服务指南","联系合作",
"网站公告","站点导航","用户反馈","产品展厅","收费标准","新手上路","网站帮助","刊登广告",
"最新供应","隐私条款","网站合作",0};

const char * COPYRIGHT_BAD_TEAM[]={"上一","下一","下载地址",0};

const char * COPYRIGHT_FEAT_TERM[]={//文字,链接,链接组,文字链接
	"电话:(0000)","电话(传真):","传真:00","电话:00","联系电话:00","传真号码:00",
	"邮编:000","e-mail:","e_mail:","厂商网址:","厂商电话:","地址:",

	"processedin0second(s),0queries.","allrightsreserved版权所有poweredby","http://www.miibeian.gov.cn/",
	"processedin0.0000second.","copyright&copy;2000-2000","增值电信业务经营许可证沪b0-00000000",
	"&copy;2000-2000","公司copyright&copy2000","allrightsrserved2000","copyright2000-2000?","copyright2000-2000",
	"icp证:浙b0-00000000","icp证:沪b0-00000000","allrightsreserved","网站公安备案编号:000000",
	"传真:000-00000000","粤icp备00000000号","豫icp备00000000号","渝icp备00000000号","湘icp备00000000号","皖icp备00000000号",
	"苏icp备00000000号","蜀icp备00000000号","陕icp备00000000号","闽icp备00000000号","鲁icp备00000000号","辽icp备00000000号",
	"京icp备00000000号","晋icp备00000000号","冀icp备00000000号","吉icp备00000000号","沪icp备00000000号","黑icp备00000000号",
	"桂icp备00000000号","赣icp备00000000号","鄂icp备00000000号","mailto:server@","copyright?2000","2000-2000","京icp证000000号",
	"version0.0.0","e-mail:@.com","&copy;2000-","comsenzinc","2000&copy;","清除cookies","poweredby","未经授权请勿转载",
	"公安局备案编号:","公安备案编号:","特别声明:本站","经营许可证编号","本站所有权归","有限责任公司","&copy;","投诉与建议",
	"私隐权声明","使用本网站","免责说明:","版权所有:","版权所有",
	0
};



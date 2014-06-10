/*
 * mark_func_time.cpp
 *
 *  Created on: 2011-11-24
 *      Author: ddt
 */
#include <math.h>
#include "easou_mark_baseinfo.h"
#include "easou_mark_com.h"
#include "easou_mark_conf.h"
#include "easou_mark_func.h"
#include "debuginfo.h"
#define _isdigit(ch)    (ch>='0'&&ch<='9')
#define _YEAR "1"
#define _MONTH "2"
#define _DAY "3"
#define _HOUR "4"
#define _MIN "5"
#define _SEC "6"
#define _ENG_MONTH "7"
static const char g_month_list[][4]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static const char g_digit_list[][3]={"零","一","二","三","四","五","六","七","八","九","十","初"};
static const char g_digit_list2[][3]={"０","１","２","３","４","５","６","７","８","９","０","１"};
static const char g_pattern[][30]={
	/*0*/   _YEAR   "年"    _MONTH  "月"    _DAY    "日||||"        _HOUR   "点"    _MIN    "分"    _SEC    "秒",
	/*1*/   _YEAR   "年"    _MONTH  "月"    _DAY    "日||||"        _HOUR   "时"    _MIN    "分"    _SEC    "秒",
	/*2*/   _YEAR   "年"    _MONTH  "月"    _DAY    "日||||"        _HOUR   ":"     _MIN    ":"     _SEC ,/* |表示中英文或空 */
	/*3*/   _DAY    _ENG_MONTH _YEAR        _HOUR   ":"     _MIN    ":"     _SEC	"GMT",
	/*4*/   _ENG_MONTH      _DAY    _HOUR   ":"     _MIN    ":"     _SEC    _YEAR	"GMT",
	/*5*/   _YEAR   "#"     _MONTH  "#"     _DAY    "||"    _HOUR   ":"     _MIN    ":" _SEC ,/*       #表示-/.       */
	/*6*/   _MONTH  "/"     _DAY    "/"     _YEAR   _HOUR   ":"     _MIN    ":"     _SEC	"|m",/*       +表示任意一个字符       */
	/*7*/   _ENG_MONTH      "|||||||"       _DAY    "||||"  _YEAR   _HOUR   ":"     _MIN ":" _SEC,
	/*8*/   _MONTH  "月"    _DAY    "日"    _HOUR   ":"     _MIN    ":"     _SEC,
	/*9*/   _MONTH  "月"    _DAY    "日"    _HOUR   "时"     _MIN    "分"     _SEC,
	/*10*/  _MONTH  "#"     _DAY    _HOUR   ":"     _MIN    ":"     _SEC,
	/*11*/  _HOUR	":"	_MIN	":"	_SEC,
	/*12*/	_HOUR 	":"	_MIN,
};
static int g_pattern_len[MAX_PATTERN_NUM];
static int g_pat_num=sizeof(g_pattern)/sizeof(g_pattern[0]);
static const int g_min_month[]={0,1900,1,1,0,0,0};
static const int g_max_month[]={0,2043,12,31,24,60,60};
static int g_hash_time[1000]={0};

/**
 * 时间初始化
 */
void time_init()
{
	const char str[]="0123456789年月日aepuco/.-";
	int str_len=strlen(str);
	int i;
	unsigned int j;
	for(i=0;i<g_pat_num;i++)
		g_pattern_len[i]=strlen(g_pattern[i]);

	int month_list_num = sizeof(g_month_list)/sizeof(g_month_list[0]);
	int pow1[]={1,1,1};
	int pow2[]={5,3,2};
	for(i=0;i<month_list_num;i++)
	{
		int value1=0,value2=0;
		for(j=0;j<strlen(g_month_list[i]);j++)
		{
			unsigned int v = (unsigned char)(g_month_list[i][j]);
			g_hash_time[v]=1;
			value1+=v*pow1[j];
			value2+=v*pow2[j];
		}
		g_hash_time[value1]=i+1;
		g_hash_time[value2]=i+1;
	}

	for(i=0;i<str_len;i++)
	{
		unsigned int id= (unsigned char)(str[i]);
		g_hash_time[id]=1;
	}

	int g_digit_list_num = sizeof(g_digit_list)/sizeof(g_digit_list[0]);
	for(i=0;i<g_digit_list_num;i++)
	{
		for(j=0;j<2;j++)
		{
			unsigned int id;
			id = (unsigned char)(g_digit_list[i][j]);
			g_hash_time[id]=2;
		}
	}
	g_hash_time[999]=1;
	return;
}

/**
 * 计算输入字符串匹配时间模式字符串的长度
 */
int get_time_len(const char *str,int offset)
{
	if(!_isdigit(*str)&&(*str>'S'||*str<'A')&&g_hash_time[(unsigned char)(*str)]!=2){
		return 0;
	}
	if(g_hash_time[(unsigned char)(*(str+1))]==0){
		return 0;
	}
	if(offset>0&&_isdigit(*(str-1))){
		return 0;
	}
	int i,j,k;
	for(i=0;i<g_pat_num;i++)
	{
		int cur_chinese_num=0,space_num=0;
		const char *pos=str;
		int pattern_len=g_pattern_len[i];
		for(j=0;j<pattern_len;j++)
		{
			if(*pos=='\0')
				break;

			while(*pos==' '||*pos=='\t'||*pos=='\n')
				pos++,space_num++;

			if(*pos=='\0')
				break;

			while(memcmp(pos,"&nbsp;",6)==0)
				pos+=6,space_num+=6;

			if(*pos=='\0')
				break;

			if((g_pattern[i][j]=='+')||(g_pattern[i][j]=='#'&&(*pos=='-'||*pos=='/'||*pos=='.'/*||*pos=='_'*/)))
				pos++;
			else if(g_pattern[i][j]=='|')
			{
				if(*pos!='\0' && !_isdigit(*pos))
					pos++;
			}
			else if(g_pattern[i][j] == *_YEAR ||g_pattern[i][j] == *_MONTH || g_pattern[i][j] == *_DAY||
					g_pattern[i][j] == *_HOUR ||g_pattern[i][j] == *_MIN   || g_pattern[i][j] == *_SEC)
			{
				int num=-1;
				if(_isdigit(*pos))
					num=atoi(pos);
				else
				{
					int date[5]={0},nd=0;
					do
					{
						for(k=0;k<=11;k++)
							if((*pos==g_digit_list[k][0]&&*(pos+1)==g_digit_list[k][1])||
									(*pos==g_digit_list2[k][0]&&*(pos+1)==g_digit_list2[k][1]))
							{
								date[nd++]=k;
								pos+=2;
								break;
							}
					}
					while(k<=11&&nd<5);
					if(nd==4&&date[0]*100+date[1]*10+date[2]<=204&&date[0]*100+date[1]*10+date[2]>=198)
						num=date[0]*1000+date[1]*100+date[2]*10+date[3];/*二零零八*/
					else if(nd==1&&date[0]<=10&&date[0]>=1)
						num=date[0];/*一二．．． 十*/
					else if(date[0]==11&&date[1]<=10&&date[1]>=1&&nd==2)
						num=date[1];/*初一 初十*/
					else if(date[0]==10&&nd==2&&date[1]>=1&&date[1]<=9)
						num=10+date[1];/*十一 十九*/
					else if(nd==2&&date[1]==10)
						num=10*date[0];/*二十 三十*/
					else if(nd==3&&date[0]==2&&date[1]==10&&date[2]>=1&&date[2]<=9)
						num=20+date[2];/*二十一*/
					else if(nd==2)
						num=date[0]*10+date[1];
				}
				if(g_pattern[i][j]=='1'&&_isdigit(*pos)&&_isdigit(*(pos+1))&&!_isdigit(*(pos+2))&&num<100)/*80-99
															    00-20*/
				{
					num += ( (num<20&&num>=0) ? 2000 : (num>80?1900:0) );
				}
				if(num<g_min_month[g_pattern[i][j]-'0']||num>g_max_month[g_pattern[i][j]-'0'])
					break;

				k=0;
				while(k<4&&_isdigit(*pos))
					k++,pos++;
			}
			else if(g_pattern[i][j]==*_ENG_MONTH)
			{
				if(*pos>'S'||*pos<'A'){
					break;
				}
				int off1 = (unsigned char)(*pos)+(unsigned char)(*(pos+1))+(unsigned char)(*(pos+2));
				int off2 = (unsigned char)(*pos)*5+(unsigned char)(*(pos+1))*3+(unsigned char)(*(pos+2))*2;
				if(off1 > 999 || off2 > 999){
					return 0;
				}
				int k1=g_hash_time[(unsigned char)(*pos)+(unsigned char)(*(pos+1))+(unsigned char)(*(pos+2))];
				int k2=g_hash_time[(unsigned char)(*pos)*5+(unsigned char)(*(pos+1))*3+(unsigned char)(*(pos+2))*2];


				if(k1!=0&&k1==k2)
					pos+=3;
				else
					break;
			}
			else if(*pos==g_pattern[i][j])
			{
				if(*pos<0)
					cur_chinese_num++;
				pos++;
			}
			else
				break;
		}
		if(j==pattern_len||pos-str-space_num>=8||cur_chinese_num>1||(i==10&&j==3))
		{
			if((i<2&&j==14)||(i==4&&*(pos-1)=='.')||(i==8&&j==7)||(i==7&&j<=8))
				continue;

			int len=strlen(pos);
			if(i==10&&j==3&&len>4)
				continue;

			return pos-str;
		}
	}

	return 0;
}

#define TEXT_BUF_LEN 1024
typedef struct _marking_info_t
{
	bool is_time;
}marking_info_t;

void fill_time(int time_info[],int text_len,int cur)
{
	if(cur<0 || cur>=text_len || time_info[cur]!=2)
		return;
	time_info[cur]=1;
	fill_time(time_info,text_len,cur+1);
	fill_time(time_info,text_len,cur-1);
	fill_time(time_info,text_len,cur+2);
	fill_time(time_info,text_len,cur-2);
}
static int is_text_time(double m0,double m1,double t1,double t2)
{

	if(m1<3)
		return 0;
	if(m1>2*m0)
		return 1;
	if(m0>2*m1)
		return 0;
	if(log(0.01+m1+t1+t2)*m1>2.95*m0)
		return 1;
	return 0;
}
static int judge_text_time(int time_info[],unsigned int text_len,char text[])
{
	char good_word[][10]={
		"日期","时间","发布","发表","更新","最后","现在","上次","注册","登录","在","于","於","楼",
		"time","Post","星期","年","月","日","一","二","三","四","五","六","Date","：","上午","下午",
		"七","八","九","200","199","报时","今天","是","当前","时区"};
	char bad_word[][10]={
		"版权","页","元","作者","来源","版","本","米","下载","集","电话","层",
		"页","次","万","亿","千","个","条","M","v","V","m","k","K",".","日记",
		"期","/","￥","[","]"};
	unsigned int good_num=sizeof(good_word)/sizeof(good_word[0]);
	unsigned int bad_num=sizeof(bad_word)/sizeof(bad_word[0]);
	unsigned int i,j;
	unsigned int k;
	for(i=0;i<text_len;i++)
		if(time_info[i]==0)
		{
			for(j=0;j<good_num;j++)
			{
				if(memcmp(text+i,good_word[j],strlen(good_word[j]))==0)
				{
					for(k=0;k<strlen(good_word[j]);k++)
						time_info[i+k]=2;//将匹配良好词的地方写2
					i+=strlen(good_word[j])-1;
					break;
				}
			}
			if(j==good_num)
				for(j=0;j<bad_num;j++)
				{
					if(memcmp(text+i,bad_word[j],strlen(bad_word[j]))==0)
					{
						for(k=0;k<strlen(bad_word[j]);k++){
							marktreeprintfs(MARK_TIME,"march bad word =%s at %s(%d)-%s\r\n",bad_word[j],__FILE__,__LINE__,__FUNCTION__);
							time_info[i+k]=3;//将匹配坏词的地方写3
							}
						i+=strlen(bad_word[j])-1;
						break;
					}
				}

		}
	for(i=0;i<text_len;i++)
		if(time_info[i]==1)
		{
			for(j=i-4;j<i+4;j++)
				if(j>=0 && j<text_len && time_info[j]==2)
					fill_time(time_info,text_len,j);//将匹配时间模式附近的良好词位置也写成1
		}
	for(i=0;i<text_len;i++)
		if(time_info[i]==3)
		{
			for(j=i;j<text_len;j++)
				if(time_info[j]!=3)
					break;
			for(k=i-4;k<i+4;k++)
				if(k>=0&&k<text_len)
					time_info[k]=0;//将坏词左右4个位置写0，认为为非时间
		}
	double max_zero=0,max_one=0,cur_zero=0,cur_one=0,total_one=0,total_two=0;
	for(i=0;i<text_len;i++)
	{
		if(time_info[i]==0)
		{
			cur_one=0;
			cur_zero++;
			if(cur_zero>max_zero)
				max_zero=cur_zero;
		}
		else if(time_info[i]==1)
		{
			cur_zero=0;
			cur_one++;
			total_one++;
			if(cur_one>max_one)
				max_one=cur_one;
		}
		else if(time_info[i]==2)
			total_two++;
	}
	marktreeprintfs(MARK_TIME,"max_zero=%f,max_one=%f,total_one=%f,total_two=%f at %s(%d)-%s\r\n",max_zero,max_one,total_one,total_two,__FILE__,__LINE__,__FUNCTION__);
	return is_text_time(max_zero,max_one,total_one,total_two);
}
static int visit_for_time(html_vnode_t *vnode, void *data)
{
	if(!vnode->isValid)
		return VISIT_SKIP_CHILD;

	marking_info_t *minfo = (marking_info_t *)data;
	if(vnode->hpNode->html_tag.tag_type != TAG_PURETEXT){
		marktreeprintfs(MARK_TIME,"the node  is not text node ,it is not fit at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return VISIT_NORMAL;
	}
    // shuangwei add for check node position
    if(vnode->ypos*100/(vnode->hp_area->area_tree->root->area_info.height+1)*85){
    	marktreeprintfs(MARK_TIME,"its xpos =%d,the page height = %d,the node is too low ,it is not fit and skip it at %s(%d)-%s\r\n",vnode->ypos,vnode->hp_area->area_tree->root->area_info.height,__FILE__,__LINE__,__FUNCTION__);
    	return VISIT_SKIP_CHILD;
    }

	char text[TEXT_BUF_LEN]="\0";
	copy_html_text(text, 0, sizeof(text)-1, vnode->hpNode->html_tag.text);
	int text_len=strlen(text);
	if(text_len>80){
		marktreeprintfs(MARK_TIME,"the text length of the node  is >80 ,it is not fit at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return VISIT_NORMAL;
	}
	int text_info[80]={0};


	const char *pos;
	for(pos=text;*pos!='\0';pos++)
	{
		int time_len=get_time_len(pos,pos-text);
		if(time_len>0)
		{
			for(int i=0;i<time_len;i++)
				text_info[(pos-text)+i]=1;//将匹配时间字符串的地方写1
			pos+=time_len-1;
		}
	}
	if(1==judge_text_time(text_info,text_len,text)){

		minfo->is_time=true;
		marktreeprintfs(MARK_TIME,"judge (%s)  is time at %s(%d)-%s\r\n",text,__FILE__,__LINE__,__FUNCTION__);
	}


	return VISIT_NORMAL;
}
static int visit_for_mark_time_srctype(html_area_t *area, void *data)
{

	assert(area!=NULL);
	if(MARK_TIME==g_EASOU_DEBUG){
		printlines("mark time");
		printNode(area->begin->hpNode);
	}
	if(area->isValid == false){
		marktreeprintfs(MARK_TIME,"it is not valid ,it is not src_time and skip it at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP ;
	}
	// shuangwei add 20120511
		if(!IS_LEAF_AREA(area->areaattr)){
			marktreeprintfs(MARK_TIME,"it is not leaf area ,it is not src_time  at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
				return AREA_VISIT_NORMAL;
		}

	// shuangwei add for check area position, page footer skip
	if(area->abspos_mark==PAGE_FOOTER){
		marktreeprintfs(MARK_TIME,"it is at foot of page ,it is not src_time and skip it at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_SKIP ;
	}
	area_baseinfo_t * paoi = (area_baseinfo_t * )area->baseinfo;
	if(paoi->text_info.con_num!=1){
		marktreeprintfs(MARK_TIME,"the text node number (%d)!=1 ,it is not src_time at %s(%d)-%s\r\n",paoi->text_info.con_num,__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_NORMAL;
	}

	marking_info_t minfo;
	minfo.is_time=0;
	for(html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode)
	{
		html_vnode_visit(vnode,visit_for_time,NULL,&minfo);
		if(vnode == area->end)
			break;
	}
	if(minfo.is_time){
		//todo
//		char buf[1024];
//		memset(&buf, 0, 1024);
//		extract_area_content(buf, sizeof(buf), area);
//		printf("time:	%s\n", buf);
//        bool isvalid=is_valid_time_area(area,4);
//        if(!isvalid){
//        	html_area_t *parentarea=area->parentArea;
//        	isvalid=is_valid_time_area(parentarea,3);
//        	 if(!isvalid&&parentarea)
//        	 {
//        	    	html_area_t *pparea=parentarea->parentArea;
//        	       	isvalid=is_valid_time_area(pparea,3);
//
//        	 }
//        }
//        if(isvalid){
//        	tag_area_func(area, AREA_FUNC_TIME);
//        }
		tag_area_func(area, AREA_FUNC_TIME);

	}
	return AREA_VISIT_NORMAL ;
}

bool mark_func_time(area_tree_t *atree)
{
	if(g_hash_time[999]==0)
		time_init();
	mark_area_info_t * mark_info = atree->mark_info ;

	areatree_visit(atree ,
			(FUNC_START_T)visit_for_mark_time_srctype,
			NULL,mark_info ) ;

	return true;
}



/*
 * mark_func_copyright.cpp
 *
 *  Created on: 2011-11-22
 *      Author: ddt
 */
#include "easou_html_attr.h"
#include "easou_mark_com.h"
#include "easou_lang_relate.h"
#include "easou_mark_baseinfo.h"
#include "easou_mark_func.h"
#include "debuginfo.h"

static const int TEXT_BUF_LEN = 256;
static const int MAX_NODE = 200;
static const int AREA_MAX_NUM = 10;

static const int TYPE_SEPARATOR = 1;
static const int TYPE_LINK_GROUP = 2;

typedef struct _marking_info_t{
	int count;		  		/**< 当前个数       */
	int size;			  	/**< 最大个数       */
	int in_link;		  		/**< 当前是否在链接中       */
	int is_miibeian;		  	/**< 是否是备案链接       */
	char text[MAX_NODE][TEXT_BUF_LEN];	/**< 链接描述       */
	int text_len[MAX_NODE];		  	/**< 链接长度       */
	int is_in_link[MAX_NODE];		/**< 是否链接       */
	int text_type[MAX_NODE];		/**< 节点类型       */
}marking_info_t;

typedef struct _area_copyright_info_t{
	int page_height;		  	/**< 网页总高度       */
	int page_width;		  		/**< 网页总宽度       */
	html_area_t * last_area[AREA_MAX_NUM];	/**< 最后10个分块       */
	int con_size[AREA_MAX_NUM];		/**< 文字个数       */
	int xpos[AREA_MAX_NUM];		  	/**< 横坐标       */
	int ypos[AREA_MAX_NUM];		  	/**< 纵坐标       */
	int width[AREA_MAX_NUM];		/**< 宽度       */
	int height[AREA_MAX_NUM];		/**< 高度       */
	int cur_area_num;		  	/**< 当前块的编号       */
}area_copyright_info_t;

/**
 * @brief 将src字符串复制到buffer中,
**/
static int copy_text(char *buffer,int end,char *src)
{
	int nbsp_len = 6;
	int count = 0;

	while( *src!='\0' )
	{
		if( isspace(*src) )
			src ++;
		else if( memcmp(src,"&nbsp;",nbsp_len) == 0 )
			src += nbsp_len;
		else break;
	}

	while(*src!='\0')
	{
		if(isupper(*src))
		{
			*(buffer+count) = *src - 'A' + 'a';
			count ++;
			src ++;
		}
		else if( isdigit(*src) )
		{
			int num = atoi(src);
			if(num>1990 && num <2020)
			{
				memcpy(buffer+count,"2000",4);
				src += 4;
				count += 4;
			}
			else
			{
				*(buffer+count) = '0';
				src ++ ;
				count ++ ;
			}
		}
		else if( isspace(*src) )
			src++;
		else if(memcmp(src,"&nbsp;",nbsp_len)==0)
			src += nbsp_len;
		else if(memcmp(src,"：",2)==0)
		{
			*(buffer+count)=':';
			src+=2;
			count++;
		}
		else
		{
			*(buffer+count)=*src;
			src++;
			count++;
		}
		if(count>end-10)
		{
			*(buffer+count)='\0';
			break;
		}

	}
	*(buffer+count)='\0';
	return count;
}

/**
 * @brief
**/
static int visit_for_copyright(html_vnode_t *vnode, void *data)
{
	if(!vnode->isValid)
		return VISIT_SKIP_CHILD;

	marking_info_t *minfo = (marking_info_t *)data;
	html_tag_t *html_tag = &(vnode->hpNode->html_tag);

	if(html_tag->tag_type == TAG_PURETEXT && vnode->textSize > 0)
	{
		if(minfo->count<0 || minfo->count >= minfo->size)
			return VISIT_FINISH;

		minfo->is_in_link[minfo->count] = minfo->in_link;
		minfo->text_len[minfo->count] = copy_text(minfo->text[minfo->count],TEXT_BUF_LEN,html_tag->text);

		minfo->text_type[minfo->count] = 0;
      // shuangwei add"┊" by sina
		if(minfo->in_link == 0 &&
		   ( strcmp(minfo->text[minfo->count],"|")==0 || strcmp(minfo->text[minfo->count],"｜")==0|| strcmp(minfo->text[minfo->count],"┊")==0 ) )
			minfo->text_type[minfo->count] = TYPE_SEPARATOR;

		else if(minfo->in_link == 1 && minfo->text_len[minfo->count] ==8)
			minfo->text_type[minfo->count] = TYPE_LINK_GROUP;
		minfo->count++;

	}

	if(html_tag->tag_type == TAG_A)
	{
		char * href = get_attribute_value(html_tag , ATTR_HREF);
		if(href!=NULL )
		{
			minfo->in_link = 1;
			if(strstr(href,"mailto:")!=NULL
			  ||strstr(href,"http://www.miibeian.gov.cn")!=NULL)
			minfo->is_miibeian =1;
		}
	}
	return VISIT_NORMAL;;
}

/**
 * @brief
**/
static int visit_end_for_copyright(html_vnode_t *vnode, void *data)
{
	marking_info_t *minfo = (marking_info_t *)data;

	html_tag_t *html_tag = &(vnode->hpNode->html_tag);

	if(html_tag->tag_type == TAG_A && get_attribute_value(html_tag,ATTR_HREF) != NULL)
		minfo->in_link = 0;

	return VISIT_NORMAL;
}

/**
 * @brief 遍历树,获得符合版权块位置的最后若干个分块,并把分块信息保存起来
**/
static int visit_for_mark_copyright(html_area_t * area ,area_copyright_info_t* mark_info )
{
	//定义了页面总高度和总宽度
	int page_height = mark_info->page_height;

	//如果分块底部的纵坐标值太小,则跳过该分块
	if( 10*(area->area_info.ypos + area->area_info.height) < 9*page_height - 1200 ){
		marktreeprintfs(MARK_COPYRIGHT,"it is not copyright for the bottom of the area is small at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
	       return AREA_VISIT_SKIP;
	}

	//如果分块头部的纵坐标值太小,则返回该分块的子块
	if( 100*area->area_info.ypos < 85*page_height - 12000 ){
		marktreeprintfs(MARK_COPYRIGHT,"it is not copyright for the ypos of the area is small at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_NORMAL;
	}

	//将分块的信息保存起来
	mark_info->con_size[mark_info->cur_area_num] = area->baseinfo->text_info.con_size;
	mark_info->ypos[mark_info->cur_area_num] = area->area_info.ypos;
	mark_info->height[mark_info->cur_area_num] = area->area_info.height;
	mark_info->last_area[mark_info->cur_area_num++] = area;

	//如果分块的个数太多,则覆盖比较旧的分块
	if(mark_info->cur_area_num >= AREA_MAX_NUM){
		mark_info->cur_area_num = 0;
		marktreeprintfs(MARK_COPYRIGHT,"marked copyright ,the area is too many ,new will cover old at %s(%d)-%s\r\n",__FILE__,__LINE__,__FUNCTION__);
	}
	return AREA_VISIT_SKIP;
}

/**
 * @brief 获得分块是版权块的分数
**/
static int get_copyright_point(html_area_t * area,area_copyright_info_t *mark_info)
{
	int page_height = mark_info->page_height;

	marking_info_t minfo;		  /**<        */
	minfo.in_link = 0;
	minfo.size = MAX_NODE;
	minfo.count = 0;
	minfo.is_miibeian = 0;

	for(html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode)
	{
		html_vnode_visit(vnode,visit_for_copyright,visit_end_for_copyright,&minfo);
		if(vnode == area->end)
			break;
	}
	if(minfo.is_miibeian ==1)
	{
		return 1;
	}
	int i=0;
	int j=0;
	int len=0;
	int link_group_point = 0;
	int text_point = 0;
	int anchor_point = 0;

	for(i = 0; i <minfo.count ; i++)
	{
		//shuangwei modify,sina copyright first
//		if(minfo.text_type[i] == TYPE_LINK_GROUP && (i>0 && minfo.text_type[i-1] == TYPE_SEPARATOR)
//				||(i+1<minfo.count && minfo.text_type[i+1] == TYPE_SEPARATOR))
		if(minfo.text_type[i] == TYPE_LINK_GROUP &&((i>0 && minfo.text_type[i-1] == TYPE_SEPARATOR)
						||(i+1<minfo.count && minfo.text_type[i+1] == TYPE_SEPARATOR)))
		{
			link_group_point += (minfo.text_len[i]>8 ? 12-minfo.text_len[i] : minfo.text_len[i] -4);

		}
		for(j=0;COPYRIGHT_FEAT_TERM[j];j++)
		{
			len = strlen(COPYRIGHT_FEAT_TERM[j]);
			char *pos = strstr(minfo.text[i],COPYRIGHT_FEAT_TERM[j]);

			if(pos == NULL)
				continue;

			int weight=1;
			if(j>11)
				weight=8;

			text_point += (pos-minfo.text[i] < 2 ? len+len : len) * weight;
			text_point += (pos-minfo.text[i] + len + 4 > minfo.text_len[i] ? len+len: len) * weight;
		}

		for(j=0;COPYRIGHT_BAD_TEAM[j];j++)
			if(strstr(minfo.text[i],COPYRIGHT_BAD_TEAM[j])!=NULL)
			{
				return 0;
			}
		if(minfo.is_in_link[i]==1 && minfo.text_len[i]<12)
		{
			for(j=0;COPYRIGHT_ANCHOR_TERM[j];j++)
			{
				char *pos = strstr(minfo.text[i],COPYRIGHT_ANCHOR_TERM[j]);
				if(pos!=NULL)
					anchor_point +=1;
			}
		}
	}
	int height_value = 10 * (area->area_info.height+area->area_info.ypos) / (1 + page_height);

	if(height_value < 8 || link_group_point + text_point + anchor_point <3)
	{
		marktreeprintfs(MARK_COPYRIGHT,"it is not copyright for height(%d)<8 or link_group_point(%d) + text_point(%d) + anchor_point(%d) <3 at %s(%d)-%s\r\n",height_value,link_group_point, text_point , anchor_point,__FILE__,__LINE__,__FUNCTION__);
		return 0;
	}
	return 1;
}

/**
 * @brief 将保存在area_copyright_info中的分块中,挑出最适合当版权的分块
**/
static int get_area_copyright(area_copyright_info_t *mark_info)
{
	int i = 0;
	//存放倒序的分块
	int area_info[AREA_MAX_NUM];
	//存放的个数
	int area_count=0;

	//将保存在mark_info的若干个分块,倒序存放到area_info数组中
	for(i=0;i<AREA_MAX_NUM;i++) {
		int cur_num = ( AREA_MAX_NUM + mark_info->cur_area_num - i - 1) % AREA_MAX_NUM;
		if( mark_info->last_area[cur_num] == NULL )
			break;

		area_info[ area_count++ ] = cur_num;
	}
	//todo
//	char buf[1024];
	//从倒序的块中,挑选出版权块
	for(i=0;i<area_count;i++)
	{
		html_area_t * area = mark_info->last_area[area_info[i]];//mark_info->last_area[i];

		int point = get_copyright_point(area,mark_info);
		if(point>0) {
//			memset(&buf, 0, 1024);
//			extract_area_content(buf, sizeof(buf), area);
//			printf("copyright:	%s\n", buf);
			if(MARK_COPYRIGHT==g_EASOU_DEBUG){
						printNode(area->begin->hpNode);
						marktreeprintfs(MARK_COPYRIGHT,"the %d area is copyright for its point is >0 at %s(%d)-%s\r\n",area->no,__FILE__,__LINE__,__FUNCTION__);
			}
			tag_area_func(area,AREA_FUNC_COPYRIGHT);
			//shuangwei 连续区两头是，认为中间也是，原来到代码在循环存储后，有内存访问错误
			//if(i-2 >= 0 && is_func_area(mark_info->last_area[i-2],AREA_FUNC_COPYRIGHT) && mark_info->con_size[i-1]<4 ) {
			if(i-2 >= 0 && is_func_area(mark_info->last_area[area_info[i-2]],AREA_FUNC_COPYRIGHT) && mark_info->con_size[i-1]<4 ) {
//				memset(&buf, 0, 1024);
//				extract_area_content(buf, sizeof(buf), mark_info->last_area[i-2]);
//				printf("copyright:	%s\n", buf);
				marktreeprintfs(MARK_COPYRIGHT,"the %d area is copyright for the previous and next area is copyright at %s(%d)-%s\r\n",area->no,__FILE__,__LINE__,__FUNCTION__);
				tag_area_func(mark_info->last_area[area_info[i-1]], AREA_FUNC_COPYRIGHT);
			}
		}
		else{
			if(MARK_COPYRIGHT==g_EASOU_DEBUG){
									printNode(area->begin->hpNode);
									marktreeprintfs(MARK_COPYRIGHT,"the %d area is copyright for its point is < 0 at %s(%d)-%s\r\n",area->no,__FILE__,__LINE__,__FUNCTION__);
			}
		}
	}
	return 1;
}

/**
 * @brief
**/
bool mark_func_copyright(area_tree_t *atree)
{
	//版权块基本信息
	area_copyright_info_t mark_info;

	memset(&mark_info,0,sizeof(mark_info));

	//获得网页总宽度及总高度
	mark_info.page_width = atree->root->area_info.width;
	mark_info.page_height= atree->root->area_info.height;

	mark_info.cur_area_num=0;

	//遍历树,获得最后若干个分块
	areatree_visit(atree,(FUNC_START_T)visit_for_mark_copyright, NULL, &mark_info );

	//获得版权块
	get_area_copyright(&mark_info);

	return true;
}

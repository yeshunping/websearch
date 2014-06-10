/*
 * easou_mark_com.cpp
 *
 *  Created on: 2011-11-22
 *      Author: ddt
 */
#include "easou_string.h"
#include "easou_url.h"
#include "easou_html_attr.h"
#include "easou_mark_conf.h"
#include "easou_mark_srctype.h"
#include "easou_mark_func.h"
#include "easou_mark_sem.h"
#include "easou_mark_com.h"
#include "easou_debug.h"

/**
 * @brief 是否是一个表示时间的文本节点
**/
bool is_time_vnode(const html_vnode_t * vnode )
{
	if(vnode == NULL ){
		return false ;
	}
	if(vnode->hpNode->html_tag.tag_type != TAG_PURETEXT ){
		return false ;
	}
	return is_time_str(vnode->hpNode->html_tag.text ) ;
}

/**
 * @brief	分块的深度从本块到叶子块的最大深度
**/
int get_area_child_depth(const html_area_t * area )
{
	assert(area !=NULL ) ;
	if(area->subArea == NULL )
		return 0 ;
	html_area_t * sub = NULL ;
	int max = 0 ;
	for(sub = area->subArea ;sub!=NULL ; sub = sub->nextArea )
	{
		int c_len = 0 ;
		if(sub->isValid == false )
			continue ;
		if( (c_len = get_area_child_depth(sub)) > max)
		{
			max = c_len ;
		}
	}
	return max+1 ;
}

/**
 * @brief 块太小
**/
bool is_too_little_area( html_area_t * area ,mark_area_info_t * g_info , int level )
{
	assert(area!=NULL && g_info !=NULL && g_info->area_tree !=NULL ) ;
	area_tree_t * atree = g_info->area_tree ;
	int page_h = atree->root->area_info.height ;
	int page_w = atree->root->area_info.width ;
	int area_h = area->area_info.height ;
	int area_w = area->area_info.width ;

	switch (level)
	{
		case LEVEL_POS :
			break ;
		case LEVEL_SRCTYPE :
			if(area_h <= 40 && area_w <= 40 )
			{
				return true ;
			}
			if(area_h <= 10 || area_w <= 10 )
			{
				return true ;
			}
			if(area_h*50 < page_h && area_w * 50 < page_w )
			{
				return true ;
			}
			if(area_h*100 < page_h || area_w * 100 < page_w )
			{
				return true ;
			}
			return false ;
		case LEVEL_FUNC :
			if(area_h <= 40 && area_w <= 40 )
			{
				return true ;
			}
			if(area_h <= 15 || area_w <= 15 )
			{
				return true ;
			}
			if(area_h*50 < page_h && area_w * 50 < page_w )
			{
				return true ;
			}
			if(area_h*100 < page_h || area_w * 100 < page_w )
			{
				return true ;
			}
			return false ;
		case LEVEL_SEM :
			if(area_h <= 40 && area_w <= 40 )
			{
				return true ;
			}
			if(area_h <= 15 || area_w <= 15 )
			{
				return true ;
			}
			if(area_h*50 < page_h && area_w * 50 < page_w )
			{
				return true ;
			}
			if(area_h*100 < page_h || area_w * 100 < page_w )
			{
				return true ;
			}
			return false ;
		default :
			assert(0) ;
	}
	return false ;
}

/**
 * @brief 块太大
**/
bool is_too_big_area(html_area_t * area , mark_area_info_t * g_info , int level )
{
	assert(area!=NULL && g_info !=NULL && g_info->area_tree !=NULL ) ;
	area_tree_t * atree = g_info->area_tree ;
	int page_h = atree->root->area_info.height ;
	int page_w = atree->root->area_info.width ;
	int area_h = area->area_info.height ;
	int area_w = area->area_info.width ;

	switch (level)
	{
		case LEVEL_POS :
			break ;
		case LEVEL_SRCTYPE :
			if(area_h*2 > page_h && area_w * 2 > page_w ) {
				return true ;
			}
			return false ;
		case LEVEL_FUNC :
			if(area_h*2 > page_h && area_w * 2 > page_w ) {
				return true ;
			}
			return false ;
		case LEVEL_SEM :
			if(area_h*2 > page_h && area_w * 2 > page_w ) {
				return true ;
			}
			return false ;
		default :
			assert(0) ;
	}
	return false ;
}

/**
 * @brief 块的层次太高
**/
bool is_too_up_area(html_area_t * area ,mark_area_info_t * g_info , int level )
{
	assert(area!=NULL && g_info !=NULL && g_info->area_tree !=NULL ) ;
	switch(level)
	{
		case LEVEL_POS :
			if(area->depth <=0 )
			{
				return true ;
			}
			return false ;
		case LEVEL_SRCTYPE :
			if(area->depth == 0 )
			{
				return true ;
			}
			if(area->depth ==1 && get_area_child_depth(area) >=2)
			{
				return true ;
			}
			return false ;
		case LEVEL_FUNC :
			if(area->depth <=0 )
			{
				return true ;
			}
			return false ;
		case LEVEL_SEM :
			if(area->depth <=1 )
			{
				return true ;
			}
			return false ;
		default :
			assert(0) ;
	}
	return false ;
}

/**
 * @brief 块的层次太低
**/
bool is_too_down_area(html_area_t * area ,mark_area_info_t * g_info , int level )
{

	assert(area!=NULL && g_info !=NULL && g_info->area_tree !=NULL ) ;
	switch(level)
	{
		case LEVEL_POS :
			return false ;
		case LEVEL_SRCTYPE :
			if(area->depth >=5 )
			{
				return true ;
			}
			return false ;
		case LEVEL_FUNC :
			if(area->depth >=4 )
			{
				return true ;
			}
			return false ;
		case LEVEL_SEM :
			if(area->depth >=4 )
			{
				return true ;
			}
			return false ;
		default :
			assert(0) ;
	}
	return false ;
}

/**
 * @brief	提取一个分块的内容. 返回提取的内容长度.
**/
int extract_area_content(char *cont, const int size, const html_area_t *area)
{
	int avail = 0;
	cont[0] = '\0';
	vnode_list_t *vlist = area->baseinfo->text_info.cont_vnode_list_begin;
	for(; vlist; vlist = vlist->next){
		html_vnode_t *vnode = vlist->vnode;
		if(vnode->isValid && vnode->textSize > 0){
			avail = copy_html_text(cont, avail, size-1, vnode->hpNode->html_tag.text);
			if(avail >= size-1){
				break;
			}
		}
		if(vlist == area->baseinfo->text_info.cont_vnode_list_end)
			break;
	}
	return avail;
}


/**
 * @brief 块内的文本是否处于同一行.
 **/
bool is_text_in_line(const html_area_t *area)
{
	int prev_ypos = -1000;
	int prev_height = 0;
	int is_inline = true;
	vnode_list_t *vlist = area->baseinfo->text_info.cont_vnode_list_begin;
	for (; vlist; vlist = vlist->next)
	{
		html_vnode_t *vnode = vlist->vnode;
		if (vnode->isValid && vnode->textSize > 0)
		{
			if (prev_ypos >= 0)
			{
				if (vnode->ypos >= prev_ypos + prev_height)
				{
					is_inline = false;
					break;
				}
			}
			else
			{
				prev_ypos = vnode->ypos;
				prev_height = vnode->hx;
			}
		}
		if (vlist == area->baseinfo->text_info.cont_vnode_list_end)
		{
			break;
		}
	}

	return is_inline;
}

/**
 * @brief 计算分块内的文本占据了几行.
**/
int get_text_line_num(const html_area_t *area)
{
	int prev_ypos = -1000;
	int prev_height = 0;
	int line_num = 0;
	vnode_list_t *vlist = area->baseinfo->text_info.cont_vnode_list_begin;
	for(; vlist; vlist = vlist->next){
		html_vnode_t *vnode = vlist->vnode;
		if(vnode->isValid && vnode->textSize > 0){
			if(prev_ypos >= 0){
				if(vnode->ypos >= prev_ypos + prev_height){
					line_num++;
				}
			}
			else{
				line_num = 1;
			}
			prev_ypos = vnode->ypos;
			prev_height = vnode->hx;
		}
		if(vlist == area->baseinfo->text_info.cont_vnode_list_end){
			break;
		}
	}
	return line_num;
}

/**
 * @brief 是否是带有anchor的节点.3代祖先是否为A节点
**/
bool is_anchor(html_vnode_t * vnode)
{
	if(vnode!=NULL && vnode->hpNode->html_tag.tag_type==TAG_PURETEXT)
	{
		int i = 0 ;
		html_vnode_t * parent  = vnode->upperNode ;
		while( i<3 ) {
			if(parent == NULL ){
				return false ;
			}
			if( parent->hpNode->html_tag.tag_type==TAG_A ){
				return true ;
			}
			parent = parent->upperNode ;
			i++ ;
		}
	}
	return false ;
}

/**
 * @brief 获取链接的类型：站内、站外、js
**/
int get_link_type(html_vnode_t * vnode , const char * base_url )
{
	assert(vnode->hpNode->html_tag.tag_type == TAG_A ) ;
	char * phref = get_attribute_value(&vnode->hpNode->html_tag, ATTR_HREF ) ;
	if(phref&&*phref=='#'){
			return IS_LINK_ANCHOR;
		}
	return get_href_type(phref, base_url);
}

/**
 * @brief 是否是图片链接， 控制递归层数为2层，计算的数据不需要太准确
**/
bool is_pic_link(html_vnode_t *vnode)
{
	assert(vnode->hpNode->html_tag.tag_type == TAG_A ) ;
	html_vnode_t * looper = vnode->firstChild ;
	while(looper)
	{
		if(looper->hpNode->html_tag.tag_type == TAG_IMG)
		{
			return true ;
		}
		html_vnode_t * looper2 = looper->firstChild ;

		while(looper2)
		{
			if(looper2->hpNode->html_tag.tag_type == TAG_IMG)
			{
				return true ;
			}
			looper2= looper2->nextNode ;
		}

		looper = looper->nextNode ;
	}
	return false ;
}


/**
 * @brief 抽取anchor结构
**/
typedef struct _anchor_t
{
	char * anchor_buf ;
	int pos ;
}anchor_t ;

/**
 * 提取A节点下 的文本信息
 */
static int start_visit_for_anchor(html_tag_t *html_tag, void *result, int flag)
{
	anchor_t * anchor = (anchor_t *)result ;
	if(html_tag->tag_type == TAG_PURETEXT ){
		if(html_tag->text !=NULL ){
			anchor->pos = copy_html_text(anchor->anchor_buf , anchor->pos , MAX_ANCHOR_LEN-1, html_tag->text);
		}
	}
	return VISIT_NORMAL ;
}

/**
 * @brief 得到anchor
**/
int get_anchor_str(html_vnode_t * vnode  , char *buff , int buff_size)
{
	assert(vnode->hpNode->html_tag.tag_type==TAG_A) ;
	if(buff_size < MAX_ANCHOR_LEN ) {
		return -1 ;
	}
	anchor_t anchor ;
	buff[0] = 0 ;
	anchor.anchor_buf = buff ;
	anchor.pos = 0 ;
	html_node_visit(vnode->hpNode , start_visit_for_anchor ,NULL , &anchor ,0 ) ;
	assert(anchor.pos >= 0 && anchor.pos <= MAX_ANCHOR_LEN ) ;
	return anchor.pos ;
}

static const char BLANK_CHR = ' ';

/**
 * @brief 计算anchor的长度，汉字算一个字符，连续的字母算一个字符，连续的数字算一个字符
**/
int get_real_anchor_size(html_vnode_t * vnode)
{
	assert(vnode!=NULL && vnode->hpNode->html_tag.tag_type == TAG_A) ;

	char buff[MAX_ANCHOR_LEN];
	buff[0] = '\0';
	int len = get_anchor_str(vnode , buff , MAX_ANCHOR_LEN) ;
	buff[len] = '\0';

	int num = 0 ;

	for(const char *p = buff; *p;){
		int l = GET_CHR_LEN(p);
		if(l >= 2){
			if(!IS_GB_SPACE(p))
				num += 2;
			p += l;
			continue;
		}

		if(*p == BLANK_CHR){
			p ++;
			continue;
		}

		if(isalpha(*p)){
			p++;
			num++;
			while(isalpha(*p))
				p++;
			continue;
		}

		if(q_isdigit(*p)){
			p++;
			num++;
			while(q_isdigit(*p))
				p++;
			continue;
		}

		p++;
	}
	return num ;
}


/**
 * @brief 获得该节点的第一个puretext 孩子，暂时只获得一个，认为achor都出现在一个节点下
**/
static html_vnode_t * get_pure_text_child(html_vnode_t * vnode)
{
	assert(vnode!=NULL ) ;
	html_vnode_t * child  = vnode->firstChild ;
	for( ;child!=NULL ; child=child->nextNode ) {
		if(!(child->isValid)){
			continue;
		}
		if(child->hpNode->html_tag.tag_type==TAG_PURETEXT) {
			return child ;
		}
		html_vnode_t * sub_child = get_pure_text_child(child) ;
		if(sub_child != NULL ) {
			return sub_child ;
		}
	}
	return NULL ;
}

/**
 * @brief 活的节点的puretext 孩子，暂时只获得一个，认为achor都出现在一个节点下
**/
html_vnode_t * get_pure_text_child_a(html_vnode_t * vnode)
{
	assert(vnode!=NULL && vnode->hpNode->html_tag.tag_type == TAG_A) ;
	return get_pure_text_child(vnode) ;
}

/**
 * @brief 是否在一行内
**/
bool is_a_like_one_row(const html_area_t * area )
{
	if(area==NULL || area->isValid == false ){
		return false ;
	}
	int min = 65536 ;
	int max = -1 ;
	int num = 0 ;
	int nodecount=0;
	vnode_list_t * vnode = area->baseinfo->link_info.url_vnode_list_begin ;
	int lineheight=65536;
	while(vnode){
		//忽略位置计算不准确的值
		if(!(vnode->vnode)){
			break;
		}
		if(vnode->vnode->ypos<0 ){
			vnode = vnode->next ;
			continue ;
		}
		if(vnode->vnode->ypos > max ) {
			max = vnode->vnode->ypos ;
			num++ ;
		}
		if(vnode->vnode->ypos < min ) {
			min = vnode->vnode->ypos ;
			num++ ;
		}
		if(vnode->vnode->hx<lineheight){
			lineheight=vnode->vnode->hx;
		}
		nodecount++;
		if(vnode->vnode == area->baseinfo->link_info.url_vnode_list_end->vnode ){
			break ;
		}
		vnode = vnode->next ;
	}
	if( nodecount <=3 ) {
		return false ;
	}
	if(num>4){
		return false ;
	}
    if(nodecount<8&&num>2){
    	return false ;
    }
	if( max - min <= 5+(num-2)*lineheight&&nodecount/(num-1)>10 ) {
		return true ;
	}
	if( max - min <= 5 ) {
			return true ;
		}
	return false ;
}

/**
 * @brief 是否在一列内
**/
bool is_a_like_one_col(const html_area_t * area )
{
	if(area==NULL || area->isValid == false ){
		return false ;
	}
	int min = 65536 ;
	int max = 0 ;
	int num = 0 ;

	vnode_list_t * vnode = area->baseinfo->link_info.url_vnode_list_begin ;
	while(vnode){
			//忽略位置计算不准确的值
			if(!(vnode->vnode)){
				break;
			}
			if(vnode->vnode->xpos<0 ){
				vnode = vnode->next ;
				continue ;
			}
			if(vnode->vnode->xpos > max ) {
				max = vnode->vnode->xpos ;

			}
			if(vnode->vnode->xpos < min ) {
				min = vnode->vnode->xpos ;

			}

			num++ ;
			if(vnode->vnode == area->baseinfo->link_info.url_vnode_list_end->vnode ){
				break ;
			}
			vnode = vnode->next ;
		}
//	html_vnode_t * vnode = area->begin ;
//	while(vnode){
//		//忽略位置计算不准确的值
//		if(vnode->xpos<0 ){
//			vnode = vnode->nextNode ;
//			continue ;
//		}
//		if(vnode->xpos > max ){
//			max = vnode->xpos ;
//		}
//		if(vnode->xpos < min ){
//			min = vnode->xpos ;
//		}
//		num++ ;
//		if(vnode == area->end ){
//			break ;
//		}
//		vnode = vnode->nextNode ;
//	}
	if( num <=2 ){
		return false ;
	}
	if( max - min <= 5 ){
		return true ;
	}
	return false ;
}


/**
 * @brief 获取当前块同一行左边的分块.
**/
const html_area_t *get_left_area_inline(const html_area_t *area)
{
	int line_ypos = area->area_info.ypos;
	int my_xpos = area->area_info.xpos;

	const html_area_t *cur_area = area;

	while(cur_area){
		const html_area_t *iter = cur_area->prevArea;

		while(iter){
			if(iter->isValid){
				if(iter->area_info.ypos == line_ypos){
					if(iter->area_info.xpos < my_xpos){
						return iter;
					}
				}
				else
					return NULL;
			}

			iter = iter->prevArea;
		}

		cur_area = cur_area->parentArea;
	}

	return NULL;
}

/**
 * @brief 获取当前块同一行右边的分块.
**/
const html_area_t *get_right_area_inline(const html_area_t *area)
{
	int line_ypos = area->area_info.ypos;
	int my_xpos = area->area_info.xpos;

	const html_area_t *cur_area = area;

	while(cur_area){
		const html_area_t *iter = cur_area->nextArea;
		while(iter){
			if(iter->isValid){
				if(iter->area_info.ypos == line_ypos){
					if(iter->area_info.xpos > my_xpos){
						return iter;
					}
				}
				else
					return NULL;
			}

			iter = iter->nextArea;
		}

		cur_area = cur_area->parentArea;
	}

	return NULL;
}

/**
 * @brief	根据块内节点的tag判断，是否文本节点，而非block节点.
**/
bool is_text_area(const html_area_t *area)
{
	for(html_vnode_t *vnode = area->begin; vnode; vnode = vnode->nextNode){
		if(vnode->isValid){
			if(!IS_TEXT_VNODE(vnode->property)){
				return false;
			}
		}
		if(vnode == area->end){
			break;
		}
	}
	return true;
}

/**
 * @brief	获取左边的有效文本长度.
**/
int get_left_cont_size(html_area_t *area)
{
	const html_area_t *left = get_left_area_inline(area);

	int left_con_size = 0;

	int area_hi = area->area_info.height;

	while(left){
		if((left->area_info.height - area_hi)*20 > area_hi && left->area_info.height - area_hi > 30
				&& !is_text_area(left) && left->depth < area->depth){
			break;
		}
		left_con_size += left->baseinfo->text_info.con_size
			- left->baseinfo->text_info.no_use_con_size;

		left = get_left_area_inline(left);
	}

	return left_con_size;
}

/**
 * @brief	获取左边叶子块的有效文本长度.
 **/
int get_left_leaf_cont_size(html_area_t *area)
{
	const html_area_t *left = get_left_area_inline(area);

	int left_con_size = 0;

	int area_hi = area->area_info.height;

	while (left)
	{
		if ((left->area_info.height - area_hi) * 20 > area_hi && left->area_info.height - area_hi > 30
				&& !is_text_area(left) && left->depth < area->depth)
		{
			break;
		}
		if (left->valid_subArea_num < 1)
		{
			left_con_size += left->baseinfo->text_info.con_size - left->baseinfo->text_info.no_use_con_size;
		}

		left = get_left_area_inline(left);
	}

	return left_con_size;
}

/**
 * @brief	获取右边的有效文本长度.
**/
int get_right_con_size(html_area_t *area)
{
	const html_area_t *right = get_right_area_inline(area);

	int right_con_size = 0;

	int area_hi = area->area_info.height;

	while(right){
		if((right->area_info.height - area_hi)*20 > area_hi
				&& right->area_info.height - area_hi > 30
				&& !is_text_area(right)
				&& right->depth < area->depth){
			break;
		}

		right_con_size += right->baseinfo->text_info.con_size
			- right->baseinfo->text_info.no_use_con_size;

		right = get_right_area_inline(right);
	}

	return right_con_size;
}

/**
 * @brief	获取右边叶子块的有效文本长度.
**/
int get_right_leaf_con_size(html_area_t *area)
{
	const html_area_t *right = get_right_area_inline(area);

	int right_con_size = 0;

	int area_hi = area->area_info.height;

	while(right){
		if((right->area_info.height - area_hi)*20 > area_hi
				&& right->area_info.height - area_hi > 30
				&& !is_text_area(right)
				&& right->depth < area->depth){
			break;
		}
        if(right->valid_subArea_num<1){
        	right_con_size += right->baseinfo->text_info.con_size
        				- right->baseinfo->text_info.no_use_con_size;

        }

		right = get_right_area_inline(right);
	}

	return right_con_size;
}

/**
 * @brief 是否默认的链接颜色.
**/
bool is_normal_link_color(const font_t *font)
{
	return font->in_link && font->color == DEFAULT_LINK_COLOR;
}

static bool is_normal_font(font_t *font, font_t *root_font)
{
	if(font->header_size == 0
			&& font->is_bold == 0
			&& font->is_strong == 0
			&& font->is_big == 0
			&& font->is_italic == 0
			&& (font->is_underline == 0 || font->in_link)
			&& font->size <= root_font->size
			&& font->bgcolor == root_font->bgcolor
			&& (font->color == root_font->color
				|| is_gray_color(font->color)
				|| is_normal_link_color(font))
			&& font->align != VHP_TEXT_ALIGN_CENTER){
		return true;
	}

	return false;
}

static bool has_normal_font(font_t *font[], unsigned int font_num, font_t *root_font)
{
	for(unsigned int i = 0; i < font_num; i++){
		if(is_normal_font(font[i],root_font))
			return true;
	}

	return false;
}

typedef struct _area_font_t{
	html_area_t *upper;
	html_area_t *area;
	font_t **font;
	unsigned int num;
	int non_outstand_size;
	bool is_outstand;
}area_font_t;

static bool header_outstander_than(const font_t *font, const font_t *other_font)
{
	if(other_font->header_size == 0){
		if(font->header_size > 0)
			return true;
	}
	else{
		if(font->header_size < other_font->header_size)
			return true;
	}

	return false;
}

static bool is_font_outstander_than(const font_t *font, const font_t *other_font, int ypos, int other_ypos)
{
	if (font->size >= other_font->size || font->header_size > 0 || font->is_bold > other_font->is_bold
			|| font->is_strong > other_font->is_strong)
	{
		;
	}
	else
		return false;

	if (header_outstander_than(font, other_font) || font->is_bold > other_font->is_bold
			|| font->is_strong > other_font->is_strong || font->is_big > other_font->is_big
			|| (font->size >= 20 && font->size > other_font->size) || font->is_italic > other_font->is_italic
			|| (font->line_height >= 30 && font->line_height * 2 >= other_font->line_height * 3)
			|| (font->bgcolor != DEFAULT_BGCOLOR && font->bgcolor != other_font->bgcolor)
			|| (font->color != other_font->color && !is_gray_color(font->color))
			|| (font->align == VHP_TEXT_ALIGN_CENTER && other_font->align != VHP_TEXT_ALIGN_CENTER && ypos != other_ypos))
	{
		return true;
	}

	return false;
}

static void cmp_font(html_vnode_t *vnode, area_font_t *area_font)
{
	for (unsigned int i = 0; i < area_font->num; i++)
	{
		if (!is_font_outstander_than(area_font->font[i], &vnode->font, area_font->area->area_info.ypos, vnode->ypos))
		{
			debuginfo(MARK_REALTITLE, "cmp_font, vnode->ypos:%d, vnode->textSize:%d, vnode->font{header_size:%d,is_bold:%d,is_strong:%d,align:%d,color:%d,size:%d,line-height:%d}", vnode->ypos, vnode->textSize, vnode->font.header_size, vnode->font.is_bold, vnode->font.is_strong, vnode->font.align, vnode->font.color, vnode->font.size, vnode->font.line_height);
			area_font->non_outstand_size += vnode->textSize;
			if (area_font->non_outstand_size >= 2 * area_font->area->baseinfo->text_info.con_size
					|| area_font->non_outstand_size >= area_font->upper->baseinfo->text_info.con_size / 10)
			{
				area_font->is_outstand = false;
				return;
			}
		}
	}
}

static int visit_for_cmp_font(html_area_t *area, area_font_t *area_font)
{
	if (!area->isValid)
		return AREA_VISIT_SKIP;

	if (area == area_font->area)
		return AREA_VISIT_SKIP;

	if (area->valid_subArea_num == 0)
	{
		vnode_list_t *vlist = area->baseinfo->text_info.cont_vnode_list_begin;

		for (; vlist; vlist = vlist->next)
		{
			html_vnode_t *vnode = vlist->vnode;
			if (vnode->textSize > 0)
			{
				cmp_font(vnode, area_font);
				if (area_font->is_outstand == false)
					return AREA_VISIT_FINISH;
			}

			if (vlist == area->baseinfo->text_info.cont_vnode_list_end)
				break;
		}

		return AREA_VISIT_SKIP;
	}

	return AREA_VISIT_NORMAL;
}

static bool is_outstand_with_upper_area(html_area_t *area, font_t *font[], unsigned int font_num)
{
	html_area_t *upper_area = area->parentArea;
	int my_size = area->baseinfo->text_info.con_size - area->baseinfo->text_info.no_use_con_size;
	debuginfo(MARK_REALTITLE, "area id:%d, depth:%d, my_size:%d", area->no, area->depth, my_size);
	html_vnode_t *upper_vnode = area->begin->upperNode;
	int up_size;
	while (upper_area)
	{
		//在H1或则H2节点下面
		if (upper_area->begin == upper_area->end)
		{
			html_tag_type_t tag_type = upper_area->begin->hpNode->html_tag.tag_type;
			if (tag_type == TAG_H1 || tag_type == TAG_H2)
			{
				return true;
			}
		}
		html_tag_type_t upper_vnode_tagtype = upper_vnode->hpNode->html_tag.tag_type;
		if (upper_vnode_tagtype == TAG_H6 || upper_vnode_tagtype == TAG_H5 || upper_vnode_tagtype == TAG_H4)
		{
			return false;
		}
		up_size = upper_area->baseinfo->text_info.con_size - upper_area->baseinfo->text_info.no_use_con_size;
		debuginfo(MARK_REALTITLE, "upper_area id:%d, depth:%d, up_size:%d", upper_area->no, upper_area->depth, up_size);
		if (up_size > my_size * 2)
			break;

		if (upper_area->parentArea == NULL)
			break;

		upper_area = upper_area->parentArea;
		upper_vnode = upper_vnode->upperNode;
	}

	if (upper_area == NULL)
	{
		debuginfo(MARK_REALTITLE, "not outstand with upper area for upper_area is NULL");
		return false;
	}

	if (upper_area->no == 0)
	{
		debuginfo(MARK_REALTITLE, "not outstand with upper area for upper_area no is 0");
		return false;
	}

	area_font_t area_font;
	area_font.upper = upper_area;
	area_font.area = area;
	area_font.font = font;
	area_font.num = font_num;
	area_font.non_outstand_size = 0;
	area_font.is_outstand = true;

	areatree_visit(upper_area, (FUNC_START_T) visit_for_cmp_font, NULL, &area_font);

	return area_font.is_outstand;
}

static bool is_meet_new_font(font_t *font, font_t *font_arr[], unsigned int font_num)
{
	for(unsigned int i = 0; i < font_num; i++){
		if(is_same_font(font,font_arr[i]))
			return false;
	}
	return true;
}

/**
 * @brief 获取当前分块的字体信息.
 * @param [out] font   : font_t*[]  字体数组.
 * @param [in] size   : unsigned int 字体数组大小.
 * @param [in] min_contributing_size   : int 贡献颜色的最小文本大小.
 * @param [out] font_num   : unsigned int& 颜色数量.
 * @return  bool  颜色数量是否超出数组大小.
**/
bool get_area_font(html_area_t *area,font_t *font[],unsigned int size, int min_contributing_size, unsigned int &font_num)
{
	font_num = 0;
	if (!area->isValid)
		return false;

	vnode_list_t *vlist = area->baseinfo->text_info.cont_vnode_list_begin;
	bool font_num_exceed = false;
	for (; vlist; vlist = vlist->next)
	{
		html_vnode_t *vnode = vlist->vnode;
		if (!vnode->isValid)
			goto _CONTINUE;
		if (vnode->textSize >= min_contributing_size
			&& !is_space_text(vnode->hpNode->html_tag.text))
		{
			if (is_meet_new_font(&vnode->font, font, font_num))
			{
				if (font_num < size)
				{
					font[font_num] = &vnode->font;
					font_num++;
				}
				else
				{
					font_num_exceed = true;
					break;
				}
			}
		}
		_CONTINUE:
		if (vlist == area->baseinfo->text_info.cont_vnode_list_end)
			break;
	}
	return font_num_exceed;
}

/**
 * @brief 判断当前分块的字体/颜色是否突出。
 * 在当前分块的字体数小于等于size的前提下，判断当前分块的字体是否突出。
 * 如果当前分块的字体数>size，直接返回false.
 * @param [in] area   : html_area_t* 判断是否突出的分块。
 * @param [in] root_font   : font* 根节点的字体.
 * @param [out] font*   : font_t[]	字体数组。
 * @param [in] size   : unsigned int	字体数组大小.
 * @param [out] font_num   : unsigned int	输出的不同字体数量.
 * @return  bool
 * @retval   突出返回TRUE,否则返回FALSE.
 * 	font数组中返回当前分块的前size个字体.
**/
bool check_area_font_outstand(html_area_t *area, font_t *root_font, font_t *font[], unsigned int size, unsigned int &font_num)
{
	static int MIN_CONTRIBUTING_SIZE = 3;

	bool font_num_exceed = get_area_font(area, font, size, MIN_CONTRIBUTING_SIZE, font_num);

	//是H1或者H2节点，或则子孙节点中有H1或者H2节点
	if(area->begin == area->end){
		html_tag_type_t tag_type = area->begin->hpNode->html_tag.tag_type;
		if(tag_type == TAG_H1 || tag_type == TAG_H2){
			return true;
		}
		if(tag_type == TAG_H6 || tag_type == TAG_H5 || tag_type == TAG_H4){
			return false;
		}
		html_vnode_t *vnode = area->begin;
		while(vnode->firstChild && vnode->firstChild->nextNode == NULL){
			vnode = vnode->firstChild;
			html_tag_type_t tag_type = vnode->hpNode->html_tag.tag_type;
			if(tag_type == TAG_H1 || tag_type == TAG_H2){
				return true;
			}
		}
	}

	if (font_num_exceed)
	{
		debuginfo(MARK_REALTITLE, "area(id=%d)'s font not outstand for it has more than %d different font", area->no, font_num);
		return false;
	}

	if (has_normal_font(font, font_num, root_font))
	{
		debuginfo(MARK_REALTITLE, "area(id=%d)'s font not outstand for it has normal font", area->no);
		return false;
	}

	//当前分块的最大字体等于前后兄弟分块的最大字体，则认为字体不突出
	int prevAreaMaxFont = -1;
	if (area->prevArea && area->prevArea->isValid)
	{
		for (html_vnode_t *vnode = area->prevArea->begin; vnode; vnode = vnode->nextNode)
		{
			if(vnode->isValid)
			{
				if(vnode->subtree_max_font_size > prevAreaMaxFont)
				{
					prevAreaMaxFont = vnode->subtree_max_font_size;
				}
			}
			if (vnode == area->prevArea->end)
			{
				break;
			}
		}
	}
	int nextAreaMaxFont = -1;
	if (area->nextArea && area->nextArea->isValid)
	{
		for (html_vnode_t *vnode = area->nextArea->begin; vnode; vnode = vnode->nextNode)
		{
			if(vnode->subtree_max_font_size > nextAreaMaxFont)
			{
				nextAreaMaxFont = vnode->subtree_max_font_size;
			}
			if (vnode == area->nextArea->end)
			{
				break;
			}
		}
	}
	int curAreaMaxFont = -1;
	for (int i = 0; i < font_num; i++)
	{
		if (font[i]->size > curAreaMaxFont)
		{
			curAreaMaxFont = font[i]->size;
		}
	}
	if (curAreaMaxFont != -1 && (curAreaMaxFont == prevAreaMaxFont || curAreaMaxFont == nextAreaMaxFont))
	{
		debuginfo(MARK_REALTITLE, "area(id=%d)'s font is not outstand [ cur_area_max_font:%d prev_area_max_font:%d next_area_max_font:%d ]", area->no, curAreaMaxFont, prevAreaMaxFont, nextAreaMaxFont);
		return false;
	}

	if (area->depth <= 1)
	{
		for (int i = 0; i < font_num; i++)
		{
			if (font[i] && font[i]->size > 20)
			{
				debuginfo(MARK_REALTITLE, "area(id=%d) font is outstand with upper", area->no);
				return true;
			}
		}
		debuginfo(MARK_REALTITLE, "area(id=%d) font is not outstand with upper", area->no);
		return false;
	}

	if (!is_outstand_with_upper_area(area, font, font_num))
	{
		debuginfo(MARK_REALTITLE, "area(id=%d)'s font not outstand for it not outstand with upper area", area->no);
		return false;
	}

	return true;
}

/**
 * @brief 获取下一个有内容的area.
**/
html_area_t *get_next_content_area(html_area_t *area)
{
	for(html_area_t *n = area->nextArea;n;n=n->nextArea){
		if(n->isValid && n->baseinfo->text_info.con_size > 0){
			return n;
		}
	}
	return NULL;
}

/**
 * @brief 获取第一个标记为MYPOS的块.
**/
html_area_t *get_first_mypos_area(area_tree_t *atree)
{
	/**获取标记为mypos的节点列表*/
	const area_list_t *mypos_area_list = get_func_mark_result(atree, AREA_FUNC_MYPOS);
	if(mypos_area_list == NULL || mypos_area_list->num  < 1){
		return NULL;
	}
	return mypos_area_list->head->area;
}


int is_chn_1(unsigned char c)
{
	if(c>=0xB0)
	{
		return 1;
	}
	return 0;
}

int is_chn_2(unsigned char c)
{
	if(c>=0XA1)
	{
		return 1;
	}
	return 0;
}

int is_digit(char c)
{
	if(c>='0' && c<='9')
	{
		return 1;
	}
	return 0;
}

int is_alpha(char c)
{
	if((c>='a' && c<='z')||(c>='A' && c<='Z'))
	{
		return 1;
	}
	return 0;
}

char delims[]=",./~;()-_";

int is_delim(char c)
{
	int ret=0;
	for(unsigned int i=0;i<strlen(delims);i++)
	{
		if(c==delims[i])
		{
			ret=1;
			break;
		}
	}
	return ret;
}

int char_type(char *c)
{
	unsigned char p=*c;
	if(is_delim(p))
	{
		return DELIM;
	}
	if(is_digit(p))
	{
		return DIGIT;
	}
	if(is_alpha(p))
	{
		return ALPHA;
	}
	if(is_chn_1(p))
	{
		return CHN_1;
	}
	if(is_chn_2(p))
	{
		return CHN_2;
	}
	else
	{
		return EASOU_URL_OTHER;
	}
}

//parse url 中的某个元素：如目录、file等
//元素内部以分隔符划分，每个基本单元有以下几种状态：
//0 : begin/end
//1 : # 纯数字
//2 : #$ 连续数字和字母
//3 : $# 连续字母和数字
//4 : $ 连续字母
//5 : chn_0 半中文状态
//6 : 中文状态
//7 : ^ 混合模式,各种基本单元的混合
int parse_pattern(char *raw_str,char *regular_str,int ele_type)
{
	char *p=NULL,*q=NULL,*cur=NULL;
	char *regular_p=NULL;
	char tmp[MAX_ELE_LEN];
	int i=0;
	int k=0;//表示元素中是否出现混合模式的单元
	int flag[MAX_SEPS_NUM];
	int seps_num=0;
	int pattern_len=0;
	int status=0;//0:begin/end 1:# 2:#$ 3:$# 4:$ 5.chn_0 6:chn 7:^
	//element is too long
	if(strlen(raw_str)>MAX_ELE_LEN-1)
	{
//		fprintf(stderr,"parse_pattern error:%s is too long element(%d)\n",raw_str,MAX_ELE_LEN);
		return -1;
	}
	p=raw_str;
	regular_p=regular_str;
	while(*p)
	{
		switch(char_type(p))
		{
			case DELIM:
				 //分隔符,表示上一个段的结束，重新整理该段的模式
				if(status==7 || status==5)
				{
					k=1;
					tmp[0]='^';
					tmp[1]=*p;
					tmp[2]='\0';
				}
				else
				{
					tmp[i++]=*p;
					tmp[i]='\0';
				}
				strncpy(regular_p,tmp,strlen(tmp));
				regular_p+=strlen(tmp);
				pattern_len+=strlen(tmp);
				if(pattern_len>=MAX_ELE_LEN)
				{
					fprintf(stderr,"too long subpart!(%s)\n",raw_str);
					return -1;
				}
				if(seps_num>=MAX_SEPS_NUM)
				{
					fprintf(stderr,"too long successive septs!\n");
					return -1;
				}
				//记录当前单元的最终状态
				flag[seps_num++]=status;
				//重新初始化
				i=0;
				status=0;
				break;
			case DIGIT:
				//当前字符为数字
				switch(status)
				{
					//上一个状态是混合模式
					case 7:
					//上一个状态是中文状态
					case 6:
					//上一个状态是半中文状态
					case 5:
					//上一个状态为#$
					case 2:
						status=7;
						break;
					//上一个状态是字母模式
					case 4:
						tmp[i]='#';
						status=3;
						i++;
						break;
					//上一个状态也是数字模式,维持当前模式
					case 1:
					case 3:
						break;
					//上一个状态是分割符模式，表示一个段的开始
					case 0:
						i=0;
						tmp[i]='#';
						i++;
						status=1;
						break;
					default:
						status=7;
						break;
				};
		                break;
			case ALPHA:
				//当前字符为字母
				switch(status)
				{
					//上一个状态是混合模式
					case 7:
          				//上一个状态是中文状态
					case 6:
					//上一个状态是半中文状态
					case 5:
					//上一个状态是$#
					case 3:
						status=7;
						break;
					//上一个状态也是字母模式
					case 2:
						tmp[i]=*p;
						i++;
						status=4;
						break;
					case 4:
						tmp[i]=*p;
						i++;
						break;
					//上一个状态是数字模式，表示出现一次切换
					case 1:
						tmp[i]=*p;
						i++;
						status=2;
						break;
					//上一个状态是分割符模式，表示一个段的开始
					case 0:
						i=0;
						tmp[i]=*p;
						i++;
						status=4;
						break;
					default:
						status=7;
						break;
				};
				break;
			case CHN_1:
			case CHN_2:
				switch(status)
				{
					case 0:
						i=0;
						tmp[i]=*p;
						i++;
						status=5;
						break;
					case 5:
						tmp[i]=*p;
						i++;
						status=6;
						break;
					case 6:
						tmp[i]=*p;
						i++;
						status=5;
						break;
					default:
						status=7;
						break;

				}
				break;
			case EASOU_URL_OTHER:
				status=7;
				break;
		}
		p++;
	}
	//最后一段的处理
	if(status==7 || status==5)
	{
		k=1;
		tmp[0]='^';
		tmp[1]=*p;
		tmp[2]='\0';
	}
	else
	{
		tmp[i++]=*p;
		tmp[i]='\0';
	}
	if(seps_num>=MAX_SEPS_NUM)
	{
		fprintf(stderr,"too long successive septs!\n");
		return -1;
	}
	flag[seps_num++]=status;
	strncpy(regular_p,tmp,strlen(tmp));
	pattern_len+=strlen(tmp);
	if(pattern_len>=MAX_ELE_LEN)
	{
		fprintf(stderr,"too long subpart!(%s)\n",raw_str);
		return -1;
	}

	regular_str[pattern_len]='\0';
	//最后统一各单元的pattern粒度
	p=regular_str;
	q=regular_str;
	cur=tmp;
	i=0;
	while(*p)
	{
		if(is_delim(*p))
		{
			//该单元属于半混合状态,为了保证同一元素内的各单元粒度一致,改写为混合模式
			if((flag[i]==2 || flag[i]==3) && k==1)
			{
				*(cur++)='^';
				*(cur++)=*p;
			}
			else
			{
				//value中的中文字符模式会改写成混合模式
				if(ele_type==ELE_VALUE && flag[i]==6)
				{
					*(cur++)='^';
					*(cur++)=*p;
				}
				else
				{
					strncpy(cur,q,p-q+1);
					cur+=(p-q+1);
				}
			}
			q=p+1;
			i++;
		}
		p++;
	}
	if((flag[i]==2 || flag[i]==3) && k==1)
	{
		*(cur++)='^';
		*(cur++)=*p;
	}
	else
	{
		strncpy(cur,q,p-q+1);
	}
	strcpy(regular_str,tmp);
	return strlen(regular_str);
}

int parse_url(const url_t *url,p_url *urlinfo)
{
	int url_type=0;
	int i=0;
	if((url->param_num==1 && url->name_len[0]==0)||url->param_num==0)
	{
		url_type=1;//静态url
	}
	//get site
	strncpy(urlinfo->site,url->site,url->site_len);
	urlinfo->site[url->site_len]=0;
	urlinfo->site_len=url->site_len;


	strncpy(urlinfo->port,url->port,url->port_len);
	urlinfo->port[url->port_len]=0;
	urlinfo->port_len=url->port_len;

	//get dir
	urlinfo->dir_num = url->dir_num-1;
	for (i = 0; i < urlinfo->dir_num; i++){
//		if(url->dir_len[i]>MAX_ELE_LEN-1)
//		{
//			fprintf(stderr,"parse url err : too long dir:%s\n",url->site);
//		}
		strncpy(urlinfo->dir[i].raw, url->dir[i], url->dir_len[i]);
		urlinfo->dir[i].raw[url->dir_len[i]] = '\0';
		//parse dir pattern
		urlinfo->dir[i].pattern_len=parse_pattern(urlinfo->dir[i].raw,urlinfo->dir[i].regular,ELE_DIR);
		if(urlinfo->dir[i].pattern_len==-1)
		{
			return -1;
		}
	}
	if (url->dir_len[i] == 0)
		urlinfo->has_file = 0;
	else
		urlinfo->has_file = 1;
	if(urlinfo->has_file)
	{
		//get file
		if(url->dir_len[i]>MAX_ELE_LEN-1)
		{
//			fprintf(stderr,"parse url err : too long filename:%s\n",url->dir[i]);
			return -1;
		}
		strncpy(urlinfo->file.raw, url->dir[i], url->dir_len[i]);
		urlinfo->file.raw[url->dir_len[i]] = '\0';
		if(url_type==1)
		{
			//parse file pattern
			urlinfo->file.pattern_len=parse_pattern(urlinfo->file.raw,urlinfo->file.regular,ELE_FILE);
			if(urlinfo->file.pattern_len==-1)
			{
				return -1;
			}
		}
		else
		//动态url的cgi部分不解析
		{
			strcpy(urlinfo->file.regular,urlinfo->file.raw);
			urlinfo->file.pattern_len=strlen(urlinfo->file.raw);
		}
	}
	//get param
	urlinfo->param_num = url->param_num;
	for (i = 0; i < urlinfo->param_num; i++)
	{
		if(url->name_len[i]>MAX_ELE_LEN-1)
		{
//			fprintf(stderr,"parse url err : too long paramname:%s\n",url->name[i]);
			return -1;
		}
		strncpy(urlinfo->name[i].raw,url->name[i],url->name_len[i]);
		urlinfo->name[i].raw[url->name_len[i]]=0;
		//don't parse name pattern
		strcpy(urlinfo->name[i].regular,urlinfo->name[i].raw);
		urlinfo->name[i].pattern_len=url->name_len[i];
		//parse value pattern
		if(url->value_len[i]>MAX_ELE_LEN-1)
		{
//			fprintf(stderr,"parse url err : too long value:%s\n",url->value[i]);
			return -1;
		}
		strncpy(urlinfo->value[i].raw, url->value[i], url->value_len[i]);
		urlinfo->value[i].raw[url->value_len[i]] = '\0';
		if(url_type==1)
		{
			urlinfo->value[i].pattern_len=parse_pattern(urlinfo->value[i].raw,urlinfo->value[i].regular,ELE_VALUE);
			if(urlinfo->value[i].pattern_len==-1)
			{
				return -1;
			}
		}
		else
		{
			urlinfo->value[i].regular[0]='*';
			urlinfo->value[i].regular[1]='\0';
			urlinfo->value[i].pattern_len=1;
		}
	}
	return 0;
}

int parse_url(const char *url,p_url *urlinfo)
{
	url_t tmp;
	const char *nor_url = url;
	if (0 == strncmp(nor_url, "http://", 7))
		nor_url += 7;

	if (NULL == parse_url_inner(&tmp, nor_url, strlen(nor_url))) {
		return -1;
	}
	return parse_url(&tmp,urlinfo);
}

int build_pattern(char *buf, p_url *url)
{
	int i;
	int pos = 0;
	int bFirst = 1;

	if (buf != NULL)
		memcpy(buf +pos, url->site,url->site_len);
	pos += url->site_len;

	if (url->port_len>0)
	{
		memcpy(buf+pos,":",1);
		memcpy(buf+pos+1, url->port,url->port_len);
		pos+=(url->port_len+1);
	}

	for(i=0; i<url->dir_num; i++)
	{
		if (url->dir[i].pattern_len == 0)
			continue;
		if (buf != NULL)
			buf[pos] = '/';
		pos++;
		if (buf != NULL)
			memcpy(buf+pos, url->dir[i].regular, url->dir[i].pattern_len);
		pos += url->dir[i].pattern_len;
	}
	buf[pos] = '/';
	pos++;
	if(url->has_file == 1)
	{
		if(buf != NULL)
		{
			memcpy(buf+pos,url->file.regular,url->file.pattern_len);
			pos += url->file.pattern_len;
		}

	}
	if(url->param_num != 0)
	{
		if (buf != NULL)
			buf[pos] = '?';
		pos++;
	}
	for(i=0; i<url->param_num; i++)
	{
		if (url->name[i].pattern_len == 0 && url->value[i].pattern_len == 0)
			continue;
		if (bFirst == 1)
			bFirst = 0;
		else
		{
			if (buf != NULL)
				buf[pos] = '&';
			pos++;
		}
		if (buf != NULL )
			memcpy(buf+pos, url->name[i].regular, url->name[i].pattern_len);
		pos += url->name[i].pattern_len;
		if (buf != NULL && url->name[i].pattern_len>0)
		{
			buf[pos] = '=';
			pos++;
		}
		if (buf != NULL)
			memcpy(buf+pos, url->value[i].regular, url->value[i].pattern_len);
		pos += url->value[i].pattern_len;
	}
	if (buf != NULL)
		buf[pos] = 0;
	pos++;
	return pos;
}

int ef_trans2pt(const char *url,char *pattern)
{

	p_url urlinfo;
	if(parse_url(url,&urlinfo)==0) {
		build_pattern(pattern,&urlinfo);
		return 1;
	}
	else {
		return 0;
	}

}

static bool is_same_urlpat_link(const char *link, const char *page_url_pat)
{
	char pat[UL_MAX_URL_LEN];
	pat[0] = '\0';//todo
	ef_trans2pt(link, pat);
	if(strcmp(pat, page_url_pat) == 0){
		return true;
	}

	return false;
}

static bool is_same_urlpat_vnode(html_vnode_t *vnode, const char *page_url, const char *page_url_pat)
{
	const char *phref = get_attribute_value(&vnode->hpNode->html_tag, ATTR_HREF);

	if(phref != NULL && strchr(phref,'#') == NULL){
		char link[UL_MAX_URL_LEN];
		link[0] = '\0';
		if(easou_combine_url(link, page_url, phref) == -1 || link[0] == '\0'){
			return false;
		}

		if(strcmp(link, page_url) == 0){
			return false;
		}

		return is_same_urlpat_link(link, page_url_pat);
	}

	return false;
}

/**
 * @brief
**/
int get_same_pat_url_num(html_area_t *area, const char *page_url, const char *page_url_pat)
{
	int sp_num = 0;

	AOI4ST_link_t *link_info = &area->baseinfo->link_info;

	vnode_list_t *vlist = link_info->url_vnode_list_begin;

	for(; vlist; vlist = vlist->next){
		html_vnode_t *vnode = vlist->vnode;

		if(is_same_urlpat_vnode(vnode, page_url, page_url_pat)){
			sp_num ++;
		}

		if(vlist == link_info->url_vnode_list_end)
			break;
	}

	return sp_num;
}

/**
 * @brief 获取相同urlpattern的链接面积.
**/
void get_same_pat_url_info(same_urlpat_t *spat, html_area_t *area, const char *page_url, const char *page_url_pat)
{
	spat->num = 0;
	spat->area = 0;

	AOI4ST_link_t *link_info = &area->baseinfo->link_info;

	vnode_list_t *vlist = link_info->url_vnode_list_begin;

	for(; vlist; vlist = vlist->next){
		html_vnode_t *vnode = vlist->vnode;

		if(is_same_urlpat_vnode(vnode, page_url, page_url_pat)){
			spat->num ++;
			spat->area += vnode->wx * vnode->hx;
		}

		if(vlist == link_info->url_vnode_list_end)
			break;
	}
}

/**
 * @brief 块是否单独处于一行. 左右没有东西.
**/
bool is_single_inline(html_area_t *area)
{
	static const int LEFT_RIGHT_NOISE_SIZE_MIN = 2;

	const html_area_t *left = get_left_area_inline(area);

	int left_right_con_size = 0;

	int area_hi = area->area_info.height;

	while(left){
		if(abs(left->area_info.height - area_hi)*10 > area_hi
				&& !is_text_area(left))
			break;

		left_right_con_size += left->baseinfo->text_info.con_size;

		if(left_right_con_size >= LEFT_RIGHT_NOISE_SIZE_MIN)
			return false;

		left = get_left_area_inline(left);
	}

	const html_area_t *right = get_right_area_inline(area);
	while(right){
		if(abs(right->area_info.height - area_hi)*10 > area_hi
				&& !is_text_area(right))
			break;

		left_right_con_size += right->baseinfo->text_info.con_size;

		if(left_right_con_size >= LEFT_RIGHT_NOISE_SIZE_MIN)
			return false;

		right = get_right_area_inline(right);
	}

	return true;
}

/**
 * @brief
**/
bool is_area_has_spec_word_by_index(const html_area_t * area  , const char * word_list[])
{
	assert(area!=NULL && area->baseinfo !=NULL ) ;
	area_baseinfo_t * paoi = (area_baseinfo_t *) area->baseinfo ;
    vnode_list_t * looper = paoi->text_info.cont_vnode_list_begin ;
	while(looper)
	{
		assert(looper->vnode->hpNode->html_tag.tag_type == TAG_PURETEXT) ;
		if(!is_anchor(looper->vnode))
		{
			const char * cont = looper->vnode->hpNode->html_tag.text ;
			if(is_has_special_word(word_list ,cont ))
			{
				return true ;
			}
		}
		if(looper == paoi->text_info.cont_vnode_list_end)
			break ;
		looper = looper->next ;
	}
	return false ;
}

/**
 * @brief 一个简单的hash函数，也许会有冲突，不过够用了.
**/
int simple_hash(const char * src)
{
	unsigned int hash = 0 ;
	const char * looper = src ;
	while(*looper)
	{
		hash+=(unsigned int )(*looper) ;
		looper++ ;
	}
	return hash%255 + 1 ;//空着0位
}

html_area_t * get_first_left_valid_area(const html_area_t * area )
{
	const html_area_t * finder = area ;
	if( finder == NULL || finder->prevArea == NULL)
		return NULL ;
	finder = finder->prevArea ;
	while(finder!=NULL)
	{
		if(finder->isValid == false )
		{
			finder = finder->prevArea ;
			continue ;
		}
		return (html_area_t * )finder ;
	}
	return NULL ;
}

//static int start_visit_for_base_url(html_tag_t *html_tag, void *result, int flag)
//{
//	char domain[MAX_SITE_LEN];
//	char port[MAX_PORT_LEN];
//	char path[MAX_PATH_LEN];
//	if(html_tag->tag_type == TAG_BASE){
//		char *base_url = get_attribute_value(html_tag, ATTR_HREF);
//		if(NULL == base_url || !easou_is_url(base_url) || strlen(base_url) >= UL_MAX_URL_LEN ||
//			!easou_parse_url(base_url, domain, port, path) || !easou_single_path(path)){
//			return VISIT_FINISH;
//		}
//		snprintf((char *)result, MAX_URL_LEN, "%s", base_url);
//		return VISIT_FINISH;
//	}
//	return VISIT_NORMAL;
//}
//
//static int finish_visit_for_base_url(html_tag_t *html_tag, void *result)
//{
//	if(html_tag->tag_type == TAG_HEAD)
//		return VISIT_FINISH;
//	return VISIT_NORMAL;
//}
//
////获取页面内base tag所指定的URL
//int get_base_url(char *base_url, html_tree_t *html_tree)
//{
//	int ret = html_tree_visit(html_tree, &start_visit_for_base_url, &finish_visit_for_base_url, base_url, 0);
//	return ret;
//}

/**
 * @brief 获取当前块同一page左边的分块.
**/
const html_area_t *get_left_area_in_page(const html_area_t *area)
{
	int line_ypos = area->area_info.ypos;
	int my_xpos = area->area_info.xpos;

	const html_area_t *cur_area = area;

	while(cur_area){
		const html_area_t *iter = cur_area->prevArea;

		while(iter){
			if(iter->isValid){
				if((iter->area_info.ypos==line_ypos)||(iter->area_info.ypos < line_ypos)&&(iter->area_info.ypos+iter->area_info.height > line_ypos)){
					if((iter->area_info.xpos+iter->area_info.width) <=(my_xpos+10)){
						debuginfo(MARK_POS, "mark the area(id=%d) has left area(%d)", area->no,iter->no);
						return iter;
					}
					else{
						debuginfo(MARK_POS, "mark the area(id=%d) has not left area(%d)", area->no,iter->no);
						return NULL;
					}
				}

			}

			iter = iter->prevArea;
		}

		cur_area = cur_area->parentArea;
	}

	return NULL;
}

/**
 * @brief 获取当前块同一page右边的分块.
**/
const html_area_t *get_right_area_in_page(const html_area_t *area)
{
	int line_ypos = area->area_info.ypos;
	int my_xpos = area->area_info.xpos;

	const html_area_t *cur_area = area;

	while(cur_area){
		const html_area_t *iter = cur_area->nextArea;
		while(iter){
			if(iter->isValid){
				if(iter->area_info.ypos==line_ypos||(iter->area_info.ypos > line_ypos)&&(iter->area_info.ypos<  line_ypos+area->area_info.height))
				{
					if((iter->area_info.xpos+10) >=( my_xpos+area->area_info.width)){
						debuginfo(MARK_POS, "mark the area(id=%d) has right area(%d)", area->no,iter->no);
						return iter;
					}
					else
					{
						debuginfo(MARK_POS, "mark the area(id=%d) has not right area(%d)", area->no,iter->no);
						return NULL;
					}
				}

			}

			iter = iter->nextArea;
		}

		cur_area = cur_area->parentArea;
	}

	return NULL;
}

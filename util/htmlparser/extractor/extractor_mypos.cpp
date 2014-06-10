/*
 * easou_extractor_mypos.cpp
 *
 *  Created on: 2012-1-11
 *      Author: xunwu
 */
#include "easou_url.h"
#include "html_text_utils.h"
#include "easou_html_attr.h"
#include "easou_mark_baseinfo.h"
#include "easou_extractor_com.h"
#include "easou_extractor_mypos.h"
#include "easou_html_extractor.h"


/****************************************************************************************************************/
#define MYPOS_LEN_LIMIT		160						//MYPOS文本的最大长度限制
#define MYPOS_EXCEED_Y_PROPORTION(ypos,page_height)	(10*(ypos) > 6*(page_height)) //MYPOS文本在页面中的纵坐标比例限制
#define MYPOS_EXCEED_X_PROPORTION(xpos,page_width)	(10*(xpos) > 5*(page_width)) //MYPOS文本在页面中的横坐标比例限制
#define MYPOS_Y_SPAN_LIMIT	60						//MYPOS文本在页面中跨越的纵向最大高度限制
#define MYPOS_VALID_ITEM_LEN	2					//MYPOS的每一个item最少需含有的有效字符长度
#define MYPOS_MAX_SIZE_PER_ITEM	35					//MYPOS的每一个item平均最大长度限制

#define MYPOS_OUTPUT_ONLY_TEXT	0x00000001
#define MYPOS_OUTPUT_ONLY_ITEMS	0x00000002
#define MYPOS_OUTPUT_ALL		0x00000003

#define MYPOS_IS_OUTPUT_TEXT(x)	((x)&MYPOS_OUTPUT_ONLY_TEXT)
#define MYPOS_IS_OUTPUT_ITEMS(x) ((x)&MYPOS_OUTPUT_ONLY_ITEMS)

static html_area_t *get_mypos_area(const area_tree_t *atree)
{
	const area_list_t *alist = get_func_mark_result((area_tree_t *)atree , AREA_FUNC_MYPOS) ;
	if(alist && alist->num > 0){
		return alist->head->area;
	}
	return NULL;
}

/**
 * @brief	得到当前子树文本，不判断是否满足MYPOS限制
 * @return  失败返回-1，否则返回0
**/
static int recursive_get_tree_txt_without_check (char *txt, int &txt_len, const html_vnode_t *node)
{
	html_tag_t *html_tag = &(node->hpNode->html_tag);

	if(html_tag->tag_type == TAG_PURETEXT && node->textSize > 0){
		int len =  strlen(html_tag->text);
		if(txt_len+len >= MYPOS_BUF_LEN)
			return -1;
		memcpy(txt+txt_len,html_tag->text,len);
		txt_len += len;
	}
	for(html_vnode_t *chld = node->firstChild; chld; chld=chld->nextNode){
		int ret = recursive_get_tree_txt_without_check (txt, txt_len, chld);
		if (-1 == ret)
			return -1;
	}
	return 0;
}

static int get_area_txt(char *text, int &len, html_area_t *area)
{
	for(html_vnode_t *vnode = area->begin; vnode; vnode=vnode->nextNode){
		if(vnode->isValid){
			int ret = recursive_get_tree_txt_without_check(text,len,vnode);
			if(ret == -1)
				return -1;
		}
		if(vnode == area->end)
			break;
	}
	return 0;
}

/**
 * @brief	若为空格，返回其长度，否则返回零
**/
static int check_lbl_space(const char *str)
{
	if(*str == '&'){
		if(strncmp(str+1,"nbsp;",5) == 0)
			return 6;
		if(*(str+1) == '#'){
			if(strncmp(str+2,"x20;",4) == 0)
				return 6;
			if(strncmp(str+2,"32;",3) == 0)
				return 5;
		}
	}
    return 0;
}

static int check_lsharp_and_getlen(const char *str)
{
	if(*str == '&' && *(str+1) == 'l'){
		if(strncmp(str+2,"t;",2) == 0)
			return 4;
		if(strncmp(str+2,"aquo;",5) == 0)
			return 7;
	}
	if(strncmp(str,"←",2) == 0)
		return 2;
	if(strncmp(str,"＜",2) == 0)
		return 2;
	if(*str == '<')
		return 1;
    return 0;
}


// 若为右尖括号，返回长度，否则返回0
static int check_rsharp_and_getlen(const char *str)
{
    if(*str == '&'){
        if(strncmp(str+1,"gt;",strlen("gt;")) == 0)
            return strlen("gt;")+1;
        if(strncmp(str+1,"#62;",strlen("#62;")) == 0)
            return strlen("#62;")+1;
        if(strncmp(str+1,"raquo;",strlen("raquo;")) == 0)
            return strlen("raquo;")+1;
        if(strncmp(str+1,"#187;",strlen("#187;")) == 0)
            return strlen("#187;")+1;
        if(strncmp(str+1,"#8250;",strlen("#8250;")) == 0)
            return strlen("#8250;")+1;
    }
    if(strncmp(str,"→",strlen("→")) == 0)
        return strlen("→");
    if(strncmp(str,"＞",strlen("＞")) == 0)
        return strlen("＞");
    if(strncmp(str,"\x81\x30\x86\x33",strlen("\x81\x30\x86\x33")) == 0)
        return strlen("\x81\x30\x86\x33");
    if(*str == '>')
        return 1;
    return 0;
}

/**
 * @brief	将HTML空格标号替换为空格
**/
static void uniform_spaces(char *txt)
{
	char *result_pt = txt;
    for(char *pt = txt; *pt; result_pt++,pt++){
        int l = check_lbl_space(pt);
        if(l > 0){
            *result_pt = ' ';
            pt += l-1;
			continue;
        }
		*result_pt = *pt;
    }
	*result_pt = '\0';
}

static const char Uni_Lsharp = '<';
static const char Uni_Rsharp = '>';

static void uniform_sharps(char *txt)
{
	char *result_pt = txt;
    for(char *pt = txt; *pt; ){
        int sharp_len = check_lsharp_and_getlen(pt);
        if(sharp_len > 0){
            *result_pt = Uni_Lsharp;
			result_pt++;
            pt += sharp_len;
            continue;
        }
        sharp_len = check_rsharp_and_getlen(pt);
        if(sharp_len > 0){
            *result_pt = Uni_Rsharp;
			result_pt++;
            pt += sharp_len;
            continue;
        }
		unsigned int skip_l = GET_CHR_LEN(pt);
		memmove(result_pt,pt,skip_l);
		result_pt += skip_l;
		pt += skip_l;
    }
	*result_pt = '\0';
}

static const char trivial_char[] = { //琐碎的无意义字符: '\r','\n', '\t', ' '.
//  0  1  2  3  4  5  6  7  8  9 10  11 12 13 14 15
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static void compact_txt (char *txt)
{
	///将\r,\n,\t以空格替代
	for(char *pt = txt; *pt;){
		int chr_l = GET_CHR_LEN(pt);
		if(chr_l >= 2){
			pt += chr_l;
			continue;
		}
		if(trivial_char[(unsigned char)*pt])
			*pt = ' ';
		pt++;
	}
}

static void tame_mypos(mypos_t *mypos, int is_merge_text)
{
	uniform_spaces (mypos->text);
	uniform_sharps (mypos->text);
	compact_txt (mypos->text);
	if(is_merge_text)
		copy_html_text(mypos->text, 0, MYPOS_BUF_LEN-1, mypos->text);
	else{//todo
		copy_html_text(mypos->text, 0, MYPOS_BUF_LEN-1, mypos->text);
//		html_deref_to_gb18030_str(mypos->text, mypos->text, 0, MYPOS_BUF_LEN-1);
	}
}

static int is_chn_word(const char *p)
{
	unsigned char h = (unsigned char)*p;
	unsigned char l = (unsigned char)*(p+1);
	if(h>=0xB0 && h <= 0xF7 && l >= 0xA1 && l <= 0xFE)	//GBK
		return 1;
	if(h>=0x81 && h <= 0xA0 && l >= 0x40 && l <= 0xFE)	//CJK
		return 1;
	if(h>=0xAA && h <= 0xFE && l >= 0x40 && l <= 0xA0)	//
		return 1;
	return 0;
}

static int has_valid_words(const char *text)
{
	int l = 0;
	for(const char *pt=text;*pt;){
		if((l=check_lbl_space(pt)))
			pt += l;
		else if((l=check_lsharp_and_getlen(pt)))
			pt += l;
		else if((l=check_rsharp_and_getlen(pt)))
			pt += l;
		else if(isalnum(*pt))
			return 1;
		else if(is_chn_word(pt))
			return 1;
		else{
			pt += GET_CHR_LEN(pt);
		}
	}
	return 0;
}


// {{{　获取当前子树的文本和其对应的URL（若有的话）
static int recursive_get_tree_txt_and_link (mypos_t *mypos, const html_vnode_t *node, const char *page_url)
{
	if (mypos->item_num >= MYPOS_MAX_ITEM_NUM-1)
		return -1;
	int keep_item_num = 0;

	html_tag_t *html_tag = &(node->hpNode->html_tag);
	if(html_tag->tag_type == TAG_A){
		char *link = get_attribute_value(html_tag,ATTR_HREF);
		if(link != NULL){
			if(strlen(link) < MAX_URL_LEN){
				strncpy(mypos->items[mypos->item_num].url,link, MAX_URL_LEN-1);
			}
			else{
//				Warn("%s:%d too long link in page %s." ,__FILE__,__LINE__,page_url);
			}
		}
	}
	else if(html_tag->tag_type == TAG_PURETEXT){
		if(has_valid_words(html_tag->text)){
			strncpy(mypos->items[mypos->item_num].text, html_tag->text, UL_MAX_TEXT_LEN-1);
			mypos->item_num++;
			mypos->items[mypos->item_num].url[0] = '\0';
			mypos->items[mypos->item_num].text[0] = '\0';
		}
	}
	keep_item_num = mypos->item_num;

	for(html_vnode_t *chld = node->firstChild; chld; chld=chld->nextNode){
		int ret = recursive_get_tree_txt_and_link(mypos, chld, page_url);
		if(-1 == ret)
			return -1;
	}

	if(html_tag->tag_type == TAG_A && keep_item_num == mypos->item_num){
		//TAG_A结束，若它没有anchor文字：
		mypos->items[mypos->item_num].url[0] = '\0';
		mypos->items[mypos->item_num].text[0] = '\0';
	}
	return 0;
}

static int get_area_txt_and_link(mypos_t *mypos, html_area_t *area, const char *page_url)
{
	for(html_vnode_t *vnode = area->begin; vnode; vnode=vnode->nextNode){
		if(vnode->isValid){
			int ret = recursive_get_tree_txt_and_link (mypos,vnode,page_url);
			if(ret == -1)
				return -1;
		}
		if(vnode == area->end)
			break;
	}

	return 0;
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
//static int get_base_url(char *base_url, html_tree_t *html_tree)
//{
//	int ret = html_tree_visit(html_tree, &start_visit_for_base_url, &finish_visit_for_base_url, base_url, 0);
//	return ret;
//}

static void tame_mypos_item(mypos_t *mypos, const html_vtree_t *html_vtree, const char *page_url, int is_merge_text)
{
	const char *base_url = page_url;
	char base_in_page[MAX_URL_LEN];
	if(mypos->item_num > 0){
		base_in_page[0] = '\0';
		get_base_url(base_in_page, html_vtree->hpTree);
		if(base_in_page[0] != '\0'){
			base_url = base_in_page;
		}
	}
	for(int i = 0; i < mypos->item_num;i++){
		compact_txt(mypos->items[i].text);
		if(is_merge_text)
			copy_html_text (mypos->items[i].text, 0, UL_MAX_TEXT_LEN-1, mypos->items[i].text);
		else{
			copy_html_text (mypos->items[i].text, 0, UL_MAX_TEXT_LEN-1, mypos->items[i].text);
		}
		if(mypos->items[i].url[0] != '\0'){
			if (easou_combine_url(mypos->items[i].url, base_url,mypos->items[i].url) == -1)
				mypos->items[i].url[0] = '\0';
		}
	}
}


/**
 * @brief	从分块树中抽取MYPOS。 分块树已完成MYPOS标注。
 * @return  int 提取出错，返回<0；没有提取出MYPOS，返回=0；提取MYPOS成功，返回>0.
**/
int html_atree_extract_mypos(mypos_t *mypos, const area_tree_t *area_tree, const char *page_url,int is_merge_text)
{
	if (NULL == mypos || NULL == area_tree){
		return -1;
	}

	//避免空输出
	if(!MYPOS_IS_OUTPUT_TEXT(mypos->output_type) && !MYPOS_IS_OUTPUT_ITEMS(mypos->output_type)){
		mypos->output_type = MYPOS_OUTPUT_ALL;
	}

	//清空
	mypos->len = 0;
	mypos->text[0] = '\0';
	mypos->item_num = 0;

	//获取mypos块
	html_area_t *mypos_area = get_mypos_area(area_tree);
	if(mypos_area == NULL){
		return 0;
	}
	int len = 0;
	if (MYPOS_IS_OUTPUT_TEXT(mypos->output_type)){
		get_area_txt(mypos->text, len, mypos_area);
		mypos->text[len] = 0;
		tame_mypos(mypos,is_merge_text);
	}

	if (MYPOS_IS_OUTPUT_ITEMS(mypos->output_type)){
		mypos->items[0].url[0] = '\0';
		mypos->items[0].text[0] = '\0';
		get_area_txt_and_link (mypos,mypos_area,page_url);
		tame_mypos_item(mypos,area_tree->hp_vtree,page_url,is_merge_text);
	}
	mypos->len = strlen(mypos->text);
	return mypos->len;
}


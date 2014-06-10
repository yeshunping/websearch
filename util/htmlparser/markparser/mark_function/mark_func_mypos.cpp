/**
 * mark_func_mypos.cpp
 * Description: 标记mypos块
 *  Created on: 2011-11-22
 * Last modify: 2012-10-26 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#include "chinese.h"
#include "easou_html_attr.h"
#include "easou_lang_relate.h"
#include "easou_mark_com.h"
#include "easou_mark_func.h"
#include "easou_mark_srctype.h"
#include "debuginfo.h"

#define MYPOS_HX_LIMIT	60		  /**<  MYPOS块的高度限制 */
#define MYPOS_MIN_TEXTSIZE	7		  /**<  MYOPS块的最小text size */
#define MYPOS_MAX_TEXTSIZE	160		  /**< MYPOS块的最大text size */
#define MYPOS_BUF_SIZE	1024		  /**< MYPOS文本最大buffer长度 */
#define MYPOS_VALID_ITEM_LEN    2	/**< MYPOS的每一个item最少需含有的有效字符长度 */
#define	THRESHOLD_OF_MYPOS	2           /**< 满足成为MYPOS串的字符串的强度应大于等于此值 */

static const char Uni_Lsharp = '<';
static const char Uni_Rsharp = '>';

/**
 *查看分块是否在页面的上半部分
 */
static int is_in_page_upside(int page_height, const html_area_t *area){
	int header_value = 0;
	// it seems small page rule is not applicable after 100 equality sample tes
	if (page_height>=1200){
		//header_value = 600;//
		header_value = page_height*3/7;//shuangwei modify,根据	CASEPS-169
	}
	else if(page_height <= 300){
		header_value = page_height / 2;
	}
	else{
		header_value = page_height*3/5;
	}
	if (area->area_info.ypos<header_value){
		return 1;
	}
	else{
		return 0;
	}
}

/**
 * 判断是否在页面的左半部分
 */
static int is_in_page_left(int page_width,const html_area_t *area){
	return (area->area_info.xpos <= page_width/2);
}

static bool is_proper_position(const html_area_t *area, mark_area_info_t * mark_info){
	/**页面高度和宽度*/
	int page_height = mark_info->area_tree->root->area_info.height;
	int page_width = mark_info->area_tree->root->area_info.width;
	return (is_in_page_upside(page_height, area) && is_in_page_left(page_width, area));
}

// 若为左尖括号，返回长度，否则返回0
static int check_lsharp_and_getlen(const char *str)
{
    if(*str == '&' && *(str+1) == 'l'){
        if(strncmp(str+2,"t;",strlen("t;")) == 0)
            return strlen("t;") + 2;
        if(strncmp(str+2,"aquo;",strlen("aquo;")) == 0)
            return strlen("aquo;") + 2;
    }
    if(strncmp(str,"←",strlen("←")) == 0)
        return strlen("←");
    if(strncmp(str,"＜",strlen("＜")) == 0)
        return strlen("＜");
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
        if(strncmp(str+1,"rsaquo;",strlen("rsaquo;")) == 0)
                    return strlen("rsaquo;")+1;
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

static bool is_text_has_mypos_feature (const char *txt)
{
    for (const char *pt = txt; *pt; pt += GET_CHR_LEN(pt)){
        if (check_rsharp_and_getlen (pt)){
            return true;
        }
    }
    return false;
}

static bool is_tree_has_mypos_feature(const html_vnode_t *vnode,bool in_link)
{
	html_tag_t *html_tag = &(vnode->hpNode->html_tag);
		marktreeprintfs(MARK_MYPOS,"easou print:node content=%s ,is_in_link=%d,node name=%s at %s(%d)-%s\r\n",html_tag->text,in_link,html_tag->tag_name,__FILE__,__LINE__,__FUNCTION__);
	if(html_tag->tag_type == TAG_PURETEXT && vnode->textSize > 0 ){
		/**看看是否有右尖括号*/
		if(is_text_has_mypos_feature(html_tag->text)){
			marktreeprintfs(MARK_MYPOS,"easou print:node content=%s ,is_in_link=%d,node name=%s has mypos feature at %s(%d)-%s\r\n",html_tag->text,in_link,html_tag->tag_name,__FILE__,__LINE__,__FUNCTION__);
			return true;
		}
	}
	bool is_in_link = in_link || (html_tag->tag_type == TAG_A && get_attribute_value(html_tag,ATTR_HREF) != NULL);

	for(html_vnode_t *chld = vnode->firstChild;chld;chld=chld->nextNode){
		if(chld->isValid){
			if(is_tree_has_mypos_feature(chld, is_in_link)){
				return true;
			}
		}
	}
	return false;
}

static bool is_area_has_mypos_feature(const html_area_t *area)
{
	for(html_vnode_t *v = area->begin;v;v=v->nextNode){
		if(is_tree_has_mypos_feature(v,false)){
			return true;
		}
		if(v == area->end){
			break;
		}
	}
	return false;
}

static int get_tree_txt(char *txt, int &txt_len, const html_vnode_t *node, int buf_size)
{
	html_tag_t *html_tag = &(node->hpNode->html_tag);

	if(html_tag->tag_type == TAG_PURETEXT && node->textSize > 0){
		int len =  strlen(html_tag->text);
		if(txt_len+len >= buf_size){
			return -1;
		}
		memcpy(txt+txt_len,html_tag->text,len);
		txt_len += len;
	}

	for(html_vnode_t *chld = node->firstChild; chld; chld=chld->nextNode){
		int ret = get_tree_txt(txt, txt_len, chld, buf_size);
		if(-1 == ret){
			return -1;
		}
	}

	return 0;
}

static int get_area_txt(char *txt, const html_area_t *area, int buf_size)
{
	int len = 0;
	for(html_vnode_t *v = area->begin; v; v=v->nextNode){
		if(get_tree_txt(txt, len, v, buf_size) == -1){
			txt[len] = '\0';
			return -1;
		}
		if(v == area->end){
			break;
		}
	}
	txt[len] = '\0';
	return len;
}

//若为空格，返回其长度，否则返回零
static int check_lbl_space(const char *str)
{
	if(*str == '&'){
		if(strncmp(str+1,"nbsp;",strlen("nbsp;")) == 0)
			return 6;
		if(*(str+1) == '#'){
			if(strncmp(str+2,"x20;",strlen("x20;")) == 0)
				return 6;
			if(strncmp(str+2,"32;",strlen("32;")) == 0)
				return 5;
		}
	}
	return 0;
}

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

// 将特征字符统一 彻底与编码无关!
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

static void desert_filter_words(char *txt)
{
	char *p = NULL;
	for(int i = 0; mypos_mark_filter_words[i]; i++){
		p = txt;
		while((p = strstr(p, mypos_mark_filter_words[i])) != NULL){
			int l = strlen(mypos_mark_filter_words[i]);
			memset(p,' ',l);
			p += l;
		}
	}
}

//获取特征当前文本的MYPOS特征字符：Uni_Lsharp还是Uni_Rsharp;将互相抵消的符号以空格替换
static char get_feature_char (char *txt)
{
    char *lsharp_pos_stack[MYPOS_MAX_TEXTSIZE];
    int num_net_Lsharp = 0;
    int num_net_Rsharp = 0;
    for (char *pt = txt; *pt; pt += GET_CHR_LEN(pt)){
        if (Uni_Lsharp == *pt){
            if(num_net_Lsharp < MYPOS_MAX_TEXTSIZE){
                lsharp_pos_stack[num_net_Lsharp] = pt;
                num_net_Lsharp++;
            }
        }
        else if (Uni_Rsharp == *pt){
            if(num_net_Lsharp > 0){
                num_net_Lsharp--;
                *(lsharp_pos_stack[num_net_Lsharp]) = ' ';
                *pt = ' ';
            }
            else{
                num_net_Rsharp++;
            }
        }
    }
    if (num_net_Rsharp > 0){
        return Uni_Rsharp;
    }
    if (num_net_Lsharp > 0){
        return Uni_Lsharp;
    }
    return 0;
}


static int get_valid_item_num(char *txt)
{
	const char feat_char = get_feature_char (txt);
    if (feat_char != Uni_Rsharp)
        return 0;

	// 计算被特征字符分隔的有效item数
	int valid_item_num = 0;
	int valid_len = 0;
	for(const char *p = txt;*p;p += GET_CHR_LEN(p)){
		if (feat_char == *p){
			valid_item_num += ( valid_len >= MYPOS_VALID_ITEM_LEN? 1:0);
			valid_len = 0;
		}else if(Uni_Lsharp==*p){
			valid_item_num=valid_item_num-1;//过滤书名号
		}
		else if (isalnum(*p)){
			valid_len++;
		}
		else if (is_valid_word(p)){
			valid_len += 2;
		}
	}
	valid_item_num += ( valid_len >= MYPOS_VALID_ITEM_LEN? 1:0);
	return valid_item_num;
}

static int get_area_mypos_strength(const html_area_t *area)
{
	char txt[MYPOS_BUF_SIZE];
	get_area_txt(txt, area, MYPOS_BUF_SIZE);
	/**移除空格*/
	uniform_spaces (txt);
	/**统一特殊字符*/
	uniform_sharps (txt);
	/**过滤掉特殊词*/
	desert_filter_words (txt);
	// 计算被特征字符分隔的有效item数
	return get_valid_item_num(txt);
}

static bool is_too_many_link_num(int area_strength, const html_area_t *area)
{
	int link_num = area->baseinfo->link_info.num;
	if(area_strength < THRESHOLD_OF_MYPOS + 1){
		if(link_num > area_strength + 1)
			return true;
	}
	else{
		if(link_num >= area_strength*2)
			return true;
	}
	return false;
}

static bool is_same_consize_with_sub(const html_area_t *area)
{
	int use_size = area->baseinfo->text_info.con_size - area->baseinfo->text_info.no_use_con_size;
	for(html_area_t *s = area->subArea; s; s = s->nextArea){
		if(s->isValid){
			int s_use_size = s->baseinfo->text_info.con_size - s->baseinfo->text_info.no_use_con_size;
			if(s_use_size == use_size)
				return true;
		}
	}

	return false;
}

static bool has_mypos_hint(const html_area_t *area)
{
	static const int HINT_MAX_DIS = 32;

	int line_num = get_text_line_num(area);
	if(line_num >= 2){
		marktreeprintfs(MARK_MYPOS,"easou print:too many line judge the area(id=%d) is not mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	if(is_same_consize_with_sub(area)){
		marktreeprintfs(MARK_MYPOS,"easou print:is_same_consize_with_sub judge the area(id=%d) is not mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	if(area->baseinfo->link_info.num <= 1
			&& area->baseinfo->pic_info.pic_num - area->baseinfo->pic_info.link_pic_num <= 2){
		marktreeprintfs(MARK_MYPOS,"easou print:area->baseinfo->link_info.num <= 1 judge the area(id=%d) is not mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	if(area->baseinfo->link_info.num >= 8){
		marktreeprintfs(MARK_MYPOS,"the area(id=%d) is not mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	char buf[HINT_MAX_DIS];
	extract_area_content(buf,sizeof(buf),area);

	for(int i = 0; mypos_hint[i]; i++){
		int l = strlen(mypos_hint[i]);
		const char *p = strstr(buf,mypos_hint[i]);
		if(p != NULL){
			if(p[l] == ':'
					|| strncmp(p+l,"：",strlen("：")) == 0){
				marktreeprintfs(MARK_MYPOS,"the area(id=%d) is mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
				return true;
			}
		}
	}

	marktreeprintfs(MARK_MYPOS,"the area(id=%d) is not mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
	return false;
}
/**
 * @brief 是否在一行内
**/
bool is_a_like_one_rowformypos(const html_area_t * area )
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

	if(num>4){
		return false ;
	}
	if( max - min <= 5 ) {
			return true ;
		}
	return false ;
}
static bool is_proper_mypos(const html_area_t *area)
{
	//判断MYPOS特征
//	if(!is_a_like_one_rowformypos(area)){
//		return false ;
//	}
	int area_strength = -1;
	/**判断是否有mypos的特征，主要是看有没有>和是否有链接*/
	if(is_area_has_mypos_feature(area)){
		area_strength = get_area_mypos_strength(area);
	}

	if(area_strength < THRESHOLD_OF_MYPOS){
		return has_mypos_hint(area);
	}

	if(is_too_many_link_num(area_strength,area)){
		marktreeprintfs(MARK_MYPOS,"easou print:because too many link judge the area(id=%d) is not mypos [area_strength:%d] at %s(%d)-%s\r\n", area->no, area_strength, __FILE__,__LINE__,__FUNCTION__);
		return false;
	}

	//与子分块比较,确定粒度
	for(html_area_t *sub = area->subArea; sub; sub=sub->nextArea){
		if(is_area_has_mypos_feature(sub)){
			int sub_strength = get_area_mypos_strength(sub);
			if(sub_strength >= area_strength){
				marktreeprintfs(MARK_MYPOS,"subarea is more fit, judge the area(id=%d) is not mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
				return false;
			}
		}
	}

	return true;
}

static bool is_to_skip_subtree(html_area_t *area)
{
	/**深度为1、2，且处于父分块的左右两边，其实也就是页面的左右两边了，直接跳过*/
	if(area->depth == 1){
		if(area->pos_mark == RELA_LEFT){
			marktreeprintfs(MARK_MYPOS,"skip the area(id=%d) at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
			return true;
		}
		if(area->pos_mark == RELA_RIGHT){
			marktreeprintfs(MARK_MYPOS,"skip the area(id=%d) at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
			return true;
		}
	}
	if(area->depth == 2){
		if(area->pos_mark == RELA_RIGHT){
			marktreeprintfs(MARK_MYPOS,"skip the area(id=%d) at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
			return true;
		}
		/**高度大于60，跳过*/
		if(area->pos_mark == RELA_LEFT && area->area_info.height >= 60){
			marktreeprintfs(MARK_MYPOS,"skip the area(id=%d) at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
			return true;
		}
	}
	/**有意义的文本太少，直接跳过*/
	int use_size = area->baseinfo->text_info.con_size - area->baseinfo->text_info.no_use_con_size;
	if(use_size < MYPOS_MIN_TEXTSIZE){
		marktreeprintfs(MARK_MYPOS,"skip the area(id=%d) at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return true;
	}

	return false;
}

static bool is_to_filter_by_url(html_area_t *area)
{
	/**连接数量少于4， 不会是mypos的父分块（有可能就是mypos）*/
	if(area->baseinfo->link_info.num < 4){
		return false;
	}
	AOI4ST_link_t *link_info = &(area->baseinfo->link_info);

	int last_url_len = 0;
	bool same_url_len = true;

	for(vnode_list_t *vlist = link_info->url_vnode_list_begin; vlist; vlist = vlist->next){
		html_vnode_t *vnode = vlist->vnode;
		int url_l = 0;
		const char *href = get_attribute_value(&vnode->hpNode->html_tag, ATTR_HREF);
		if(href == NULL || *href == '#' || *href == '\0'
			|| strncasecmp(href,"javascript:",strlen("javascript:")) == 0
			|| strncasecmp(href,"mailto:",strlen("mailto:")) == 0){
			goto _NEXT;
		}
		url_l = strlen(href);
		if(last_url_len > 0 && url_l - last_url_len != 0){
			same_url_len = false;
			break;
		}
		last_url_len = url_l;

_NEXT:
		if(vlist == link_info->url_vnode_list_end){
			break;
		}
	}
	/**所有链接都相同，可能是mypos的父分块*/
	if(same_url_len){
		return true;
	}
	return false;
}

static bool is_to_skip_normal(html_area_t *area, mark_area_info_t *mark_info)
{
	/**有意义的文本数量必须足够 大于7，查看子分块*/
	int use_size = area->baseinfo->text_info.con_size - area->baseinfo->text_info.no_use_con_size;
	if(use_size > MYPOS_MAX_TEXTSIZE){
		marktreeprintfs(MARK_MYPOS,"easou print:%d>160  judge the area(id=%d) is not mypos at %s(%d)-%s\r\n",use_size, area->no, __FILE__,__LINE__,__FUNCTION__);
		return true;
	}
	/**分块过高，且不是在一行，查看子分块*/
	if(area->area_info.height > MYPOS_HX_LIMIT && !is_text_in_line(area)){
		marktreeprintfs(MARK_MYPOS,"easou print:line height %d>60  judge the area(id=%d) is not mypos at %s(%d)-%s\r\n",area->area_info.height, area->no, __FILE__,__LINE__,__FUNCTION__);
		return true;
	}
	/**有链接，且连接比例较高，查看子分块*/
	if(area->baseinfo->link_info.num > 0){
		int per_anchor_size = area->baseinfo->link_info.anchor_size / area->baseinfo->link_info.num;
		if(per_anchor_size < 2){
			marktreeprintfs(MARK_MYPOS,"per_anchor_size <2  judge the area(id=%d) is not mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
			return true;
		}
	}
	/**js链接的数量过多，查看子分块*/
	if(area->baseinfo->link_info.other_num >= 1){
		if(area->baseinfo->link_info.other_num >= 4){
			marktreeprintfs(MARK_MYPOS,"link_info.other_num >=4 judge the area(id=%d) is not mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
			return true;
		}
//		if(area->baseinfo->link_info.other_num > area->baseinfo->link_info.num){
//			marktreeprintfs(MARK_MYPOS,"link_info.other_num >link_info.num, judge the area(id=%d) is not mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
//			return true;
//		}
	}
	/**连接数量大于12，查看子分块*/
	if(area->baseinfo->link_info.num >= 12){
		marktreeprintfs(MARK_MYPOS,"link_info.num >=12, judge the area(id=%d) is not mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return true;
	}
	/**含有过多选择框，输入框等交互元素，查看子分块*/
	int inter_num = area->baseinfo->inter_info.input_num
			+ area->baseinfo->inter_info.select_num + area->baseinfo->inter_info.option_num;
	if(inter_num >= 2 && !is_srctype_area(area,AREA_SRCTYPE_LINK)){
		marktreeprintfs(MARK_MYPOS,"input+select+option >=2, judge the area(id=%d) is not mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return true;
	}

	/**根据位置过滤*/
//	if(!is_proper_position(area, mark_info)){
//		marktreeprintfs(MARK_MYPOS,"easou print:position is not proper  judge the area(id=%d) is not mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
//		return true;
//	}
	/**通过链接来过滤*/
	if(is_to_filter_by_url(area)){
		marktreeprintfs(MARK_MYPOS,"easou print:is_to_filter_by_url is not proper  judge the area(id=%d) is not mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return true;
	}
	return false;
}

static int start_mark_mypos(html_area_t *area, mark_area_info_t *mark_info)
{

	if(!area->isValid){
		return AREA_VISIT_SKIP;
	}
	if(g_EASOU_DEBUG==MARK_MYPOS){
		printlines("mark mypos");
		printNode(area->begin->hpNode);
		}
	/**第一遍过滤，看是否适合作为mypos候选，从位置判断*/
	if(is_to_skip_subtree(area)){
		return AREA_VISIT_SKIP;
	}
	/**第二遍过滤，主要是判断分块是否过大*/
	if(is_to_skip_normal(area, mark_info)){
		marktreeprintfs(MARK_MYPOS,"is_to_skip_normal() is fit , judge the area(id=%d) is not mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		return AREA_VISIT_NORMAL;
	}
	/**判断是否是mypos*/
	if(is_proper_mypos(area)){
		marktreeprintfs(MARK_MYPOS,"the area  is fit , judge the area(id=%d) is  mypos at %s(%d)-%s\r\n", area->no, __FILE__,__LINE__,__FUNCTION__);
		tag_area_func(area, AREA_FUNC_MYPOS);
		return AREA_VISIT_SKIP;
	}
	return AREA_VISIT_NORMAL;
}

bool mark_func_mypos(area_tree_t *atree)
{
	bool ret = areatree_visit(atree, (FUNC_START_T)start_mark_mypos, NULL,atree->mark_info);
	return ret;
}


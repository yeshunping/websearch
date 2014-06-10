/**
 * easou_vhtml_parser.cpp
 * Description: 计算节点的坐标和长宽
 *  Created on: 2011-06-27
 * Last modify: 2012-10-26 sue_zhang@staff.easou.com shuangwei_zhang@staff.easou.com
 *      Author: xunwu_chen@staff.easoucom
 *     Version: 1.2
 */
#include "log.h"
#include "easou_string.h"
#include "easou_html_attr.h"
#include "easou_vhtml_parser.h"
#include "easou_vhtml_basic.h"
#include "easou_debug.h"

#include <math.h>
#include <ctype.h>
#include <limits.h>
#include <pthread.h>

using namespace EA_COMMON;

static const int ILLEGAL_TABLE_ATTR = 1; /**<        */
static const int ILLEGAL_TABLE_STRUCTURE = 2; /**<        */

static char g_block_tag_map[MAX_TAG_NUM];
static char g_inline_tag_map[MAX_TAG_NUM];
static bool has_vtree_init = false;

static const char *START_TAG_FOR_CSS[] =
{ "<style", "<link", "<base", };
static const html_tag_type_t TAG_TYPE_FOR_CSS[] =
{ TAG_STYLE, TAG_LINK, TAG_BASE, };

static int N_CSS_TAGS = sizeof(START_TAG_FOR_CSS) / sizeof(char *);
static char g_csstag_skip_len[256] =
{ 0 };
static int g_csstag_min_len = INT_MAX;
#define CASE_MARK	(0x20)

#define CSS_STATUS_INVALID 	10
#define CSS_SELECTOR_MAX	5000

typedef struct _css_pre_status_t
{
	int selector_id;
	int old_status;
	_css_pre_status_t *next;
} css_pre_status_t;

#define CSS_PRE_STATUS_MAX 10000

// inner function define
static html_vnode_t *first_cell_list_in_table(html_vnode_t *begin_vnode);
static html_vnode_t *next_cell_list(html_vnode_t *cell_list, int *FLAG_is_bad_table);
static html_vnode_t *next_cell(html_vnode_t *cell);

/**
 * @brief 签名与在颜色字符串列表中位置的对应关系。
 */
const short g_sign_to_pos[] =
{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, -1, -1, -1, -1, -1, -1, -1, 3, -1, 4, 6, 8, -1, -1, 10, -1, -1, -1, -1, 11, 13, 14, 15, -1, 16, -1, 17, 18, -1, 20, -1, 23, -1, 25, -1, 28, 30, -1, -1, 31, 32, 33, 36, 39, 40, 43, -1, 45, 47, 51, -1,
		-1, 53, 55, 57, -1, 60, 61, 62, 63, 67, 68, 71, 73, -1, 75, 78, 80, 81, 82, 83, 86, 87, 88, 89, -1, -1, 90, -1, -1, -1, 92, 94, 95, -1, 97, 98, 99, 100, 101, 102, -1, 104, 106, 109, -1, 110, -1, -1, 112, 113, 115, -1, -1, 116, 117, 120, 121, -1, 122, -1, 123, 124, -1, 125, 126, 128, -1, 129,
		-1, -1, -1, 130, -1, 131, 132, -1, -1, -1, -1, 133, -1, -1, -1, -1, 134, -1, -1, -1, -1, -1, 135, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 137, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 138, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, 139 };

/**
 * @brief 网页预定义颜色字符串列表
 *   按颜色字符串的签名排列.
 *   签名方式为:字符串每个字符值-'a'相加.
 *
 */
const char *g_color_str_arr[] =
{ "beige", "black", "red", "tan", "gold", "teal", "khaki", "lime", "aqua", "blue", "cyan", "coral", "green", "oldlace", "pink", "gray", "linen", "orchid", "indigo", "wheat", "darkred", "magenta", "orange", "peru", "sienna", "navy", "olive", "plum", "fuchsia", "white", "aliceblue", "cadetblue",
		"darkkhaki", "azure", "darkblue", "seagreen", "bisque", "brown", "snow", "salmon", "darkcyan", "indianred", "moccasin", "dimgray", "maroon", "deeppink", "firebrick", "chocolate", "lavender", "peachpuff", "seashell", "darkgreen", "palegreen", "darkgray", "violet", "orangered", "tomato",
		"limegreen", "olivedrab", "silver", "darkorchid", "purple", "dodgerblue", "crimson", "darkmagenta", "darkorange", "ivory", "goldenrod", "hotpink", "thistle", "yellow", "lightblue", "mintcream", "skyblue", "slateblue", "aquamarine", "lawngreen", "lightcyan", "gainsboro", "honeydew",
		"steelblue", "cornsilk", "blanchedalmond", "lightcoral", "lightgreen", "mediumblue", "darkseagreen", "lightpink", "darksalmon", "slategray", "lightgrey", "royalblue", "papayawhip", "saddlebrown", "darkviolet", "chartreuse", "lemonchiffon", "mediumorchid", "powderblue", "midnightblue",
		"blueviolet", "deepskyblue", "darkgoldenrod", "palegoldenrod", "lightseagreen", "navajowhite", "darkslateblue", "floralwhite", "whitesmoke", "lightsalmon", "forestgreen", "springgreen", "ghostwhite", "mediumseagreen", "sandybrown", "burlywood", "darkslategray", "greenyellow",
		"lavenderblush", "yellowgreen", "palevioletred", "darkolivegreen", "mistyrose", "turquoise", "lightyellow", "lightskyblue", "antiquewhite", "rosybrown", "mediumpurple", "lightsteelblue", "mediumslateblue", "mediumaquamarine", "lightslategray", "cornflowerblue", "mediumvioletred",
		"darkturquoise", "paleturquoise", "mediumspringgreen", "mediumturquoise", "lightgoldenrodyellow" };

/**
 * @brief 网页预定义颜色对应的颜色编码，与上表排列顺序相同.
 *
 */
const unsigned int g_color_value[] =
{ 0xF5F5DC, 0x000000, 0xFF0000, 0xD2B48C, 0xFFD700, 0x008080, 0xF0E68C, 0x00FF00, 0x00FFFF, 0x0000FF, 0x00FFFF, 0xFF7F50, 0x008000, 0xFDF5E6, 0xFFC0CB, 0x808080, 0xFAF0E6, 0xDA70D6, 0x4B0082, 0xF5DEB3, 0x8B0000, 0xFF00FF, 0xFFA500, 0xCD853F, 0xA0522D, 0x000080, 0x808000, 0xDDA0DD, 0xFF00FF,
		0xFFFFFF, 0xF0F8FF, 0x5F9EA0, 0xBDB76B, 0xF0FFFF, 0x00008B, 0x2E8B57, 0xFFE4C4, 0xA52A2A, 0xFFFAFA, 0xFA8072, 0x008B8B, 0xCD5C5C, 0xFFE4B5, 0x696969, 0x800000, 0xFF1493, 0xB22222, 0xD2691E, 0xE6E6FA, 0xFFDAB9, 0xFFF5EE, 0x006400, 0x98FB98, 0xA9A9A9, 0xEE82EE, 0xFF4500, 0xFF6347, 0x32CD32,
		0x6B8E23, 0xC0C0C0, 0x9932CC, 0x800080, 0x1E90FF, 0xDC143C, 0x8B008B, 0xFF8C00, 0xFFFFF0, 0xDAA520, 0xFF69B4, 0xD8BFD8, 0xFFFF00, 0xADD8E6, 0xF5FFFA, 0x87CEEB, 0x6A5ACD, 0x7FFFD4, 0x7CFC00, 0xE0FFFF, 0xDCDCDC, 0xF0FFF0, 0x4682B4, 0xFFF8DC, 0xFFEBCD, 0xF08080, 0x90EE90, 0x0000CD, 0x8FBC8B,
		0xFFB6C1, 0xE9967A, 0x708090, 0xD3D3D3, 0x4169E1, 0xFFEFD5, 0x8B4513, 0x9400D3, 0x7FFF00, 0xFFFACD, 0xBA55D3, 0xB0E0E6, 0x191970, 0x8A2BE2, 0x00BFFF, 0xB8860B, 0xEEE8AA, 0x20B2AA, 0xFFDEAD, 0x483D8B, 0xFFFAF0, 0xF5F5F5, 0xFFA07A, 0x228B22, 0x00FF7F, 0xF8F8FF, 0x3CB371, 0xF4A460, 0xDEB887,
		0x2F4F4F, 0xADFF2F, 0xFFF0F5, 0x9ACD32, 0xDB7093, 0x556B2F, 0xFFE4E1, 0x40E0D0, 0xFFFFE0, 0x87CEFA, 0xFAEBD7, 0xBC8F8F, 0x9370DB, 0xB0C4DE, 0x7B68EE, 0x66CDAA, 0x778899, 0x6495ED, 0xC71585, 0x00CED1, 0xAFEEEE, 0x00FA9A, 0x48D1CC, 0xFAFAD2 };

// the tags which are block tags, rendering linebreaks before and after,
// note : the following is not the complete list
const html_tag_type_t blockTagList[] =
{ TAG_BLOCKQUOTE, TAG_CENTER, TAG_DD, TAG_DIV, TAG_DL, TAG_DT, TAG_FORM, TAG_H1, TAG_H2, TAG_H3, TAG_H4, TAG_H5, TAG_H6, TAG_HR, TAG_IFRAME, TAG_LI, TAG_OL, TAG_P, TAG_PRE, TAG_TABLE, TAG_TR, TAG_UL,
// added
		TAG_HTML, TAG_BODY, TAG_MARQUEE, TAG_TBODY, TAG_THEAD, TAG_TFOOT };

// define inline container tags
const html_tag_type_t inlineContainerTagList[] =
{ TAG_A, TAG_FONT, TAG_SPAN, // frequent used
		TAG_ABBR, TAG_ACRONYM, TAG_TT, TAG_I, TAG_B, TAG_U, TAG_S, TAG_STRIKE, TAG_BIG, TAG_SMALL, TAG_BLINK, TAG_EM, TAG_STRONG, TAG_DFN, TAG_CODE, TAG_SAMP, TAG_KBD, TAG_VAR, TAG_CITE, TAG_Q, TAG_SUB, TAG_SUP, TAG_BDO };

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
const char *init_block_tag_map()
{
	memset(g_block_tag_map, 0, sizeof(g_block_tag_map));
	int size = sizeof(blockTagList) / sizeof(html_tag_type_t);
	for (int i = 0; i < size; i++)
	{
		g_block_tag_map[blockTagList[i]] = 1;
	}
	return g_block_tag_map;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
const char *init_inline_tag_map()
{
	memset(g_inline_tag_map, 0, sizeof(g_inline_tag_map));
	int size = sizeof(inlineContainerTagList) / sizeof(html_tag_type_t);
	for (int i = 0; i < size; i++)
	{
		g_inline_tag_map[inlineContainerTagList[i]] = 1;
	}
	return g_inline_tag_map;
}

/**
 * @brief 是否为块级元素
 */
int inline is_block_tag(html_tag_type_t tagType)
{
	return g_block_tag_map[tagType];
}

int inline is_inline_container_tag(html_tag_type_t tagType)
{
	return g_inline_tag_map[tagType];
}

static void skip_len_init()
{
	// get min pat len
	int min_len = INT_MAX;
	for (int i = 0; i < N_CSS_TAGS; i++)
	{
		int i_len = strlen(START_TAG_FOR_CSS[i]);
		if (i_len < min_len)
		{
			min_len = i_len;
		}
	}
	g_csstag_min_len = min_len;
	// init skip_arr
	g_csstag_skip_len[0] = 1;
	for (int i = 1; i < 256; i++)
	{
		g_csstag_skip_len[i] = 0; //min_len;
	}
	// compute skip len
	for (int i = N_CSS_TAGS - 1; i >= 0; i--)
	{
		for (int j = 0; j < min_len; j++)
		{
			unsigned char ch = START_TAG_FOR_CSS[i][j];
			g_csstag_skip_len[ch] = i + 1;
			unsigned char chup = toupper(START_TAG_FOR_CSS[i][j]);
			g_csstag_skip_len[chup] = i + 1;
		}
	}
}

static void html_vtree_init_route()
{
	init_block_tag_map();
	init_inline_tag_map();
	skip_len_init();
	has_vtree_init = true;
}

int html_vtree_init()
{
	static pthread_once_t once_control = PTHREAD_ONCE_INIT;
	pthread_once(&once_control, html_vtree_init_route);
	return 1;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static bool flex_bigger(int num1, int num2)
{
	if (num1 > num2)
	{
		if (num2 == 0)
			return true;
		if ((num1 - num2) * 100 > num2 * 40)
			return true;
	}

	return false;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static inline int pt2px(int ptval)
{
	return ptval * 3 / 4;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static char *parse_expr(char *expr)
{
	static const char SINGLE_QUOTE = '\'';
	static const char DOUBLE_QUOTE = '\"';
	char *p = strchr(expr, SINGLE_QUOTE);
	if (p != NULL)
	{
		char *q = expr;
		p++;
		while (*p && *p != SINGLE_QUOTE)
		{
			*q++ = *p++;
		}
		*q = '\0';
	}
	else if ((p = strchr(expr, DOUBLE_QUOTE)) != NULL)
	{
		char *q = expr;
		p++;
		while (*p && *p != DOUBLE_QUOTE)
		{
			*q++ = *p++;
		}
		*q = '\0';
	}
	return expr;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static int get_pxlen(const char *val)
{
	static const short BUF_SIZE = 256;
	char valbuf[BUF_SIZE];

	const char *value = NULL;

	if (strstr(val, "expression") != NULL)
	{
		snprintf(valbuf, sizeof(valbuf), "%s", val);
		value = parse_expr(valbuf);
	}
	else
	{
		value = val;
	}
	const char *unit = NULL;
	int pxval = parse_length(value, 10, &unit);
	//printf("val=%s,length=%d\n",val,pxval);
//	if(pxval<0){
//		return -1;
//	}
	int val_len = unit - value;

#define STR_SIGN(val)	(*(val)=='-'?-1:1)		  /**<     */

	if (val_len > 8 && pxval != -1)
	{
		pxval = UGLY_NUM * STR_SIGN(value);
	}
	if (unit[0] == '\0' && val_len >= 1)
	{
		if (val_len > 8)
		{
			pxval = UGLY_NUM * STR_SIGN(value);
		}
		else
		{
			pxval = atoi(value);
		}
	}

#undef SIGN_FUNC
	return pxval;
}

static int is_space_string(char *p)
{
	while (*p)
	{
		if (!g_whitespace_map[(unsigned char) *p])
			return 0;
		p++;
	}
	return 1;
}

static html_vnode_t *next_valid_node(html_vnode_t *vnode)
{
	html_vnode_t *tmp_vnode = vnode->nextNode;
	while (tmp_vnode && !tmp_vnode->isValid)
		tmp_vnode = tmp_vnode->nextNode;
	return tmp_vnode;
}

static html_vnode_t *last_valid_node(html_vnode_t *vnode)
{
	html_vnode_t *tmp_vnode = vnode->prevNode;
	while (tmp_vnode && !tmp_vnode->isValid)
	{
		tmp_vnode = tmp_vnode->prevNode;
	}
	return tmp_vnode;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
inline bool is_rect_vnode(html_vnode_t *vnode)
{
	if (IS_BLOCK_TAG(vnode->property))
		return true;
	switch (vnode->hpNode->html_tag.tag_type)
	{
	case TAG_IMG:
	case TAG_TD:
	case TAG_INPUT:
	case TAG_TEXTAREA:
	case TAG_FORM:
	case TAG_BR:
	case TAG_ROOT:
	case TAG_OBJECT:
	case TAG_A:
		return true;
	default:
		return false;
	}
}

#define NEXT_CSS_PROP_TYPE(t)	((easou_css_prop_type_t)((int)(t)+1))

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static int create_css_prop_list(html_vnode_t *vnode, const easou_css_property_set_t *prop_set, const int numof_css_prop, nodepool_t *css_np)
{
	int current_prop_num = 0;
	for (easou_css_prop_type_t iter = CSS_PROP_ACCELERATOR; current_prop_num < numof_css_prop; iter = NEXT_CSS_PROP_TYPE(iter))
	{
		if (prop_set->prop[iter].value != NULL)
		{
			current_prop_num++;
			css_prop_node_t *css_node = (css_prop_node_t *) nodepool_get(css_np);
			if (css_node == NULL)
			{
				goto FAIL;
			}
			css_node->type = iter;
			css_node->priority = prop_set->prop[iter].prio_val;
			css_node->value = prop_set->prop[iter].value;
			css_node->next = vnode->css_prop;
			vnode->css_prop = css_node;
		}
	}
	return 1;
	FAIL: return -1;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static int is_valid_by_style(const char *aValue)
{
	style_attr_t sattr[10];
	char value[MAX_ATTR_VALUE_LENGTH];
	if (aValue)
	{
		int nstyle = parse_style_attr(aValue, sattr, sizeof(sattr) / sizeof(style_attr_t));
		if (nstyle > 0 && get_style_attr_value("display", value, sizeof(value), sattr, nstyle) > 0)
		{
			if (is_attr_value(value, "none", strlen("none")))
				return 0;
		}
		if (nstyle > 0 && get_style_attr_value("visibility", value, sizeof(value), sattr, nstyle) > 0)
		{
			if (is_attr_value(value, "hidden", strlen("hidden")))
				return 0;
		}
	}
	return 1;
}

static int is_display_by_style(const char *aValue)
{
	style_attr_t sattr[10];
	char value[MAX_ATTR_VALUE_LENGTH];
	if (aValue)
	{
		int nstyle = parse_style_attr(aValue, sattr, sizeof(sattr) / sizeof(style_attr_t));
		if (nstyle > 0 && get_style_attr_value("display", value, sizeof(value), sattr, nstyle) > 0)
		{
			if (is_attr_value(value, "none", strlen("none")))
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
		if (nstyle > 0 && get_style_attr_value("visibility", value, sizeof(value), sattr, nstyle) > 0)
		{
			if (is_attr_value(value, "hidden", strlen("hidden")))
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
		return 2;
	}
	return 2;
}

static bool is_valid_by_css(easou_css_property_set_t *prop_set)
{
	const char *aValue = NULL;
	if ((aValue = prop_set->prop[CSS_PROP_DISPLAY].value) != NULL)
	{
		int l = strlen("none");
		if (is_attr_value(aValue, "none", l))
		{
			return false;
		}
	}
	if ((aValue = prop_set->prop[CSS_PROP_VISIBILITY].value) != NULL)
	{
		int l = strlen("hidden");
		if (is_attr_value(aValue, "hidden", l))
		{
			return false;
		}
	}
	return true;
}

static int is_display_node(html_node_t *node)
{
	for (html_attribute_t *attr = node->html_tag.attribute; attr; attr = attr->next)
	{
		switch (attr->type)
		{
		case ATTR_STYLE:
			return (is_display_by_style(attr->value));
			break;
		default:
			break;
		}
	}
	return 2;
}

static bool is_valid_html_node(html_node_t *node)
{
	switch (node->html_tag.tag_type)
	{
	case TAG_ROOT:
		return true;
	case TAG_DOCTYPE:
	case TAG_HEAD:
	case TAG_SCRIPT:
	case TAG_NOSCRIPT:
	case TAG_OPTION:
	case TAG_COMMENT:
	case TAG_STYLE:
	case TAG_TITLE:
	case TAG_META:
	case TAG_LINK:
	case TAG_BASE:
	case TAG_NOEMBED:
	case TAG_INSTRUCTION:
		return false;
	case TAG_PURETEXT:
		if (node->html_tag.text && !is_space_string(node->html_tag.text))
		{
			return true;
		}
		return false;
	case TAG_EMBED:
	{
		for (html_node_t *child = node->child; child; child = child->next)
		{
			if (child->html_tag.tag_type == TAG_PURETEXT && is_space_string(child->html_tag.text))
			{
			}
			else
				return true;
		}
		break;
	}
	case TAG_MARQUEE:
	{
		const char *aValue = get_attribute_value(&node->html_tag, ATTR_WIDTH);
		if (aValue != NULL && strcmp(aValue, "0") == 0)
			return false;
		break;
	}
	default:
		break;
	}

	for (html_attribute_t *attr = node->html_tag.attribute; attr; attr = attr->next)
	{
		switch (attr->type)
		{
		case ATTR_STYLE:
			if (!is_valid_by_style(attr->value))
				return false;
			break;
		case ATTR_TYPE:
			if (attr->value && strcasecmp(attr->value, "hidden") == 0)
				return false;
			break;
//		case ATTR_CLASS:
//			if (attr->value && strstr(attr->value, "hidden") != 0)
//				return false;
//			break;
		default:
			break;
		}
	}
	return true;
}

static void vnode_clear(html_vnode_t *vnode)
{
	// -1 stands for the field is uninitialized
	vnode->wx = -1;
	vnode->hx = -1;
	vnode->wp = -1;
	vnode->hp = -1;
	vnode->xpos = -1;
	vnode->ypos = -1;
	vnode->textSize = -1;
	vnode->subtree_textSize = -1;
	vnode->subtree_border_num = 0;

	vnode->inLink = 0;
	memset(&(vnode->font), 0, sizeof(font_t));

	vnode->min_wx = -1;
	vnode->max_wx = -1;

	vnode->property = 0;

	//pointers initialize to NULL
	vnode->firstChild = NULL;
	vnode->prevNode = NULL;
	vnode->nextNode = NULL;
	vnode->upperNode = NULL;

	vnode->struct_info = NULL;
	vnode->css_prop = NULL;
	vnode->hp_area = NULL;

	vnode->colspan = -1;

	vnode->subtree_diff_font = 0;
	vnode->subtree_max_font_size = 0;
	memset(vnode->fontSizes, 0, sizeof(vnode->fontSizes));
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static inline void set_vnode_width(html_vnode_t *vnode, const char *value)
{
	if(value&&strcasecmp(value,"auto")==0){
			return;
		}

	if (strchr(value, '%'))
	{
		vnode->wp = atoi(value);
		if(vnode->wp>0&&vnode->wp<UGLY_NUM){
			vnode->whxy=vnode->whxy|8;
				}
		if (vnode->wp <= 0)
		{
			vnode->wp = -1;
		}
		if (vnode->wp > UGLY_NUM)
		{
			vnode->wp = UGLY_NUM;
		}
	}
	else
	{
		int wx = get_pxlen(value);
		if(wx>=0&&wx<UGLY_NUM){
			vnode->whxy=vnode->whxy|8;
		}
		if ((unsigned) wx <= UGLY_NUM)
		{
			vnode->wx = wx;
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
		}
		else if (vnode->wx < 0)
		{
			vnode->wx = wx <= UGLY_NUM ? wx : UGLY_NUM;
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
		}
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static inline void set_vnode_height(html_vnode_t *vnode, const char *value)
{

	if(value&&strcasecmp(value,"auto")==0){
		return;
	}
	if (strchr(value, '%'))
	{
		vnode->hp = atoi(value);
		if(vnode->hp>0&&vnode->hp<UGLY_NUM){
			vnode->whxy=vnode->whxy|4;
		}
		if (vnode->hp <= 0)
			vnode->hp = -1;
		if (vnode->hp > UGLY_NUM)
			vnode->hp = UGLY_NUM;
	}
	else
	{
		int hx = get_pxlen(value);
		if(hx>=0&&hx<UGLY_NUM){
			vnode->whxy=vnode->whxy|4;
		}
		if ((unsigned) hx <= UGLY_NUM)
		{
			vnode->hx = hx;
			debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d", vnode->id, vnode->hx);
		}
		else if (vnode->hx < 0)
		{
			vnode->hx = hx <= UGLY_NUM ? hx : UGLY_NUM;
			debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d", vnode->id, vnode->hx);
		}
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void set_vnode_by_css(html_vnode_t *vnode, easou_css_property_set_t *prop_set)
{
	char *value = NULL;
	if ((value = prop_set->prop[CSS_PROP_WIDTH].value) != NULL)
	{
		set_vnode_width(vnode, value);
		debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
	}
	if ((value = prop_set->prop[CSS_PROP_HEIGHT].value) != NULL)
	{
		set_vnode_height(vnode, value);
		debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d", vnode->id, vnode->hx);
	}
	if ((value = prop_set->prop[CSS_PROP_FLOAT].value) != NULL)
	{
		if (is_attr_value(value, "left", strlen("left")))
		{
			SET_FLOAT_LEFT(vnode->property);
		}
		else if (is_attr_value(value, "right", strlen("right")))
		{
			SET_FLOAT_RIGHT(vnode->property);
		}
	}
	if ((value = prop_set->prop[CSS_PROP_CLEAR].value) != NULL)
	{
		if (is_attr_value(value, "left", strlen("left")))
		{
			SET_CLEAR_LEFT(vnode->property);
			SET_BREAK_BEFORE(vnode->property);
		}
		else if (is_attr_value(value, "right", strlen("right")))
		{
			SET_CLEAR_RIGHT(vnode->property);
			SET_LINE_BREAK(vnode->property);
		}
		else if (is_attr_value(value, "both", strlen("both")))
		{
			SET_CLEAR_LEFT(vnode->property);
			SET_BREAK_BEFORE(vnode->property);
			SET_CLEAR_RIGHT(vnode->property);
			SET_LINE_BREAK(vnode->property);
		}
	}
	if ((value = prop_set->prop[CSS_PROP_POSITION].value) != NULL)
	{
		if (is_attr_value(value, "absolute", strlen("absolute")))
			SET_ABSOLUTE(vnode->property);
		else if (is_attr_value(value, "relative", strlen("relative")))
			vnode->trust = 2;
	}
	int pxval = 0;
	// add use of "top", "left" attr
	if ((value = prop_set->prop[CSS_PROP_TOP].value) != NULL)
	{
		pxval = get_pxlen(value);
		if (IS_ABSOLUTE(vnode->property)&&pxval>=0)
		{ // only deal with "position:absolute" now
			vnode->ypos = pxval;
			if (vnode->ypos > UGLY_NUM)
				vnode->ypos = UGLY_NUM;
			vnode->property |= TOP_BIT;
		}
	}
	if ((value = prop_set->prop[CSS_PROP_LEFT].value) != NULL)
	{
		pxval = get_pxlen(value);
		if (IS_ABSOLUTE(vnode->property)&&pxval>=0)
		{
			vnode->xpos = pxval;
			if (vnode->xpos > UGLY_NUM)
				vnode->xpos = UGLY_NUM;
			vnode->property |= LEFT_BIT;
		}
	}
	// add use of "bottom", "right" attr
	if ((value = prop_set->prop[CSS_PROP_BOTTOM].value) != NULL)
	{
		pxval = get_pxlen(value);
		if (IS_ABSOLUTE(vnode->property)&&pxval>=0)
		{ // only deal with "position:absolute" now
			if ((vnode->property & TOP_BIT) == 0)
			{
				vnode->ypos = pxval;
				if (vnode->ypos > UGLY_NUM)
					vnode->ypos = UGLY_NUM;
				vnode->property |= BOTTOM_BIT;
			}
		}
	}
	if ((value = prop_set->prop[CSS_PROP_RIGHT].value) != NULL)
	{
		pxval = get_pxlen(value);
		if (IS_ABSOLUTE(vnode->property)&&pxval>=0)
		{
			if ((vnode->property & LEFT_BIT) == 0)
			{
				vnode->xpos = pxval;
				if (vnode->xpos > UGLY_NUM)
					vnode->xpos = UGLY_NUM;
				vnode->property |= RIGHT_BIT;
			}
		}
	}
}

/**
 * @brief assert the tag of the node checked is a block tag
 */
static bool is_inline_element(easou_css_property_set_t *prop_set, int numof_css_prop, html_vnode_t *vnode)
{
	if (numof_css_prop == 0)
	{
		return false;
	}
	const char *value = NULL;
	if ((value = prop_set->prop[CSS_PROP_DISPLAY].value) != NULL)
	{
		if (is_attr_value(value, "inline", strlen("inline")))
		{
			return true;
		}
		if (is_attr_value(value, "inline-block", strlen("inline-block")))
		{
			for (html_vnode_t *child_vnode = vnode->firstChild; child_vnode; child_vnode = child_vnode->nextNode)
			{
				SET_LINE_BREAK(child_vnode->property);
			}
			return true;
		}
	}
	return false;
}

static void set_vnode_by_style(html_vnode_t *vnode, const char *attrValue)
{
	style_attr_t sattr[10];
	char value[MAX_ATTR_VALUE_LENGTH];
	if (attrValue)
	{
		int nstyle = parse_style_attr(attrValue, sattr, sizeof(sattr) / sizeof(style_attr_t));
		if (nstyle > 0 && get_style_attr_value("position", value, sizeof(value), sattr, nstyle) > 0)
		{
			if (!strncmp(value, "absolute", strlen("absolute")))
			{
				SET_ABSOLUTE(vnode->property);
			}
			else if (!strncmp(value, "relative", strlen("relative")))
			{
				vnode->trust = 2;
			}
		}
		if (nstyle > 0 && get_style_attr_value("width", value, sizeof(value), sattr, nstyle) > 0)
		{
			set_vnode_width(vnode, value);
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
		}
		if (nstyle > 0 && get_style_attr_value("height", value, sizeof(value), sattr, nstyle) > 0)
		{
			set_vnode_height(vnode, value);
			debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d", vnode->id, vnode->hx);
		}
		// add use of "top", "left" attr
		int pxval = 0;
		if (nstyle > 0 && get_style_attr_value("top", value, sizeof(value), sattr, nstyle) > 0)
		{
			pxval = get_pxlen(value);
			if (IS_ABSOLUTE(vnode->property)&&pxval>=0)
			{ // only deal with "position:absolute" now
				vnode->ypos = pxval;
				if (vnode->ypos > UGLY_NUM)
				{
					vnode->ypos = UGLY_NUM;
				}
				vnode->property |= TOP_BIT;
			}
		}
		if (nstyle > 0 && get_style_attr_value("left", value, sizeof(value), sattr, nstyle) > 0)
		{
			pxval = get_pxlen(value);
			if (IS_ABSOLUTE(vnode->property)&&pxval>=0)
			{
				vnode->xpos = pxval;
				if (vnode->xpos > UGLY_NUM)
				{
					vnode->xpos = UGLY_NUM;
				}
				vnode->property |= LEFT_BIT;
			}
		}
		// add use of "bottom", "right" attr
		if (nstyle > 0 && get_style_attr_value("bottom", value, sizeof(value), sattr, nstyle) > 0)
		{
			pxval = get_pxlen(value);
			if (IS_ABSOLUTE(vnode->property)&&pxval>=0)
			{ // only deal with "position:absolute" now
				if ((vnode->property & TOP_BIT) == 0)
				{
					vnode->ypos = pxval;
					if (vnode->ypos > UGLY_NUM)
					{
						vnode->ypos = UGLY_NUM;
					}
					vnode->property |= BOTTOM_BIT;
				}
			}
		}
		if (nstyle > 0 && get_style_attr_value("right", value, sizeof(value), sattr, nstyle) > 0)
		{
			pxval = get_pxlen(value);
			if (IS_ABSOLUTE(vnode->property)&&pxval>=0)
			{
				if ((vnode->property & LEFT_BIT) == 0)
				{
					vnode->xpos = pxval;
					if (vnode->xpos > UGLY_NUM)
					{
						vnode->xpos = UGLY_NUM;
					}
					vnode->property |= RIGHT_BIT;
				}
			}
		}

		if (nstyle > 0 && get_style_attr_value("float", value, sizeof(value), sattr, nstyle) > 0)
		{
			if (is_attr_value(value, "left", strlen("left")))
			{
				SET_FLOAT_LEFT(vnode->property);
			}
			else if (is_attr_value(value, "right", strlen("right")))
			{
				SET_FLOAT_RIGHT(vnode->property);
			}
		}
		if (nstyle > 0 && get_style_attr_value("clear", value, sizeof(value), sattr, nstyle) > 0)
		{
			if (is_attr_value(value, "left", strlen("left")))
			{
				SET_CLEAR_LEFT(vnode->property);
				SET_BREAK_BEFORE(vnode->property);
			}
			else if (is_attr_value(value, "right", strlen("right")))
			{
				SET_CLEAR_RIGHT(vnode->property);
				SET_LINE_BREAK(vnode->property);
			}
			else if (is_attr_value(value, "both", strlen("both")))
			{
				SET_CLEAR_LEFT(vnode->property);
				SET_BREAK_BEFORE(vnode->property);
				SET_CLEAR_RIGHT(vnode->property);
				SET_LINE_BREAK(vnode->property);
			}
		}
		if (nstyle > 0 && get_style_attr_value("border", value, sizeof(value), sattr, nstyle) > 0)
		{
			int border_size = 0;
			if(isdigit(*value))
			{
				border_size = atoi(value);
			}
			if(border_size > 0)
			{
				vnode->subtree_border_num++;
				SET_BORDER(vnode->property);
			}
		}
	}
}

static void deal_pre_tag(html_vnode_t *vnode)
{
	html_vnode_t *child = vnode->firstChild;
	html_vnode_t *prev = NULL;
	while (child)
	{
		if (child->hpNode->html_tag.tag_type == TAG_PURETEXT)
		{
			if (strchr(child->hpNode->html_tag.text, '\n'))
			{
				if (child->isValid)
				{
					//child->isTextWrap = 1;
					if (child->nextNode && child->nextNode->hpNode->html_tag.tag_type != TAG_BR)
						SET_LINE_BREAK(child->property);
				}
				// break before effect
				prev = child->prevNode;
				while (prev && !prev->isValid)
					prev = prev->prevNode;
				if (prev)
					SET_LINE_BREAK(prev->property);
			}
		}
		else
			deal_pre_tag(child);
		child = child->nextNode;
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static inline void trans_text_size(html_vnode_t *vnode)
{
	int textSize = 0;
	int cn_num = 0;
	int anchorSize = 0;
	vnode->subtree_anchorSize = 0;
	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (child->isValid)
		{
			textSize += child->subtree_textSize;
			cn_num += child->subtree_cn_num;
			if (child->subtree_anchorSize >= 0)
			{
				anchorSize += child->subtree_anchorSize;
			}
		}
	}
	vnode->subtree_textSize = textSize;
	if (vnode->hpNode->html_tag.tag_type == TAG_A)
	{
		if (vnode->subtree_textSize >= 0)
		{
			vnode->subtree_anchorSize = vnode->subtree_textSize;
		}
		else
		{
			vnode->subtree_anchorSize = 0;
		}
	}

	if (vnode->hpNode->html_tag.tag_type != TAG_A)
	{
		vnode->subtree_anchorSize = anchorSize;
	}

	vnode->subtree_cn_num = cn_num;
}

html_vnode_t *construct_vtree(nodepool_t *np, html_node_t *node, int depth, int &id, html_vtree_t * vtree)
{
	html_vnode_t *prev = NULL;
	html_vnode_t *root = NULL;

	if (NULL == node || NULL == np)
	{
		goto ERR;
	}
	root = (html_vnode_t*) nodepool_get(np);
	if (NULL == root)
	{
		goto ERR;
	}
	vnode_clear(root);
	root->hpNode = node;
	root->depth = depth;
	root->id = id++;
	root->vtree = vtree;
//	printf("%d	tag_name:%s tag_txt:%s	tag_code:%d	offset:%d\n", depth, node->html_tag.tag_name, node->html_tag.text, node->html_tag.tag_code, node->html_tag.page_offset);

	for (html_node_t *child = node->child; child; child = child->next)
	{
		html_vnode_t *subroot = construct_vtree(np, child, depth + 1, id, vtree);
		if (NULL == subroot)
		{
			goto ERR;
		}
		subroot->upperNode = root;
		subroot->prevNode = prev;
		if (prev)
		{
			prev->nextNode = subroot;
		}
		else
		{
			root->firstChild = subroot;
		}
		subroot->vtree = vtree;
		prev = subroot;
	}
	return root;
	ERR:
	Fatal((char*) "construct vtree error!");
	return NULL;
}

bool is_child_in_aline(html_vnode_t *vnode)
{
	bool has_valid_before = false;
	bool has_break_before = false;
	int wx_spare = vnode->wx;

	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (!child->isValid)
			continue;

		if (has_break_before)
		{
			return false;
		}

		if (IS_BREAK_BEFORE(child->property) && has_valid_before)
		{
			return false;
		}

		wx_spare -= child->wx;
		if (wx_spare < 0)
		{
			return false;
		}

		if (IS_LINE_BREAK(child->property))
		{
			has_break_before = true;
		}

		has_valid_before = true;
	}

	return true;
}

/**
 * @brief 根据祖先节点的高度估算当前节点的高度
 * @author xunwu
 * @date 2011/06/27
 **/
static int estimate_hx_by_upper(html_vnode_t *vnode)
{
	for (html_vnode_t *upper = vnode->upperNode; upper; upper = upper->upperNode)
	{
		if (!is_child_in_aline(upper))
		{
			return -1;
		}

		if (upper->hx > 0)
		{
			return upper->hx;
		}
	}

	return -1;
}

/**
 * @brief 计算叶子节点的高度
 * @author xunwu
 * @date 2011/06/27
 **/
static void compute_leaf_hx(html_vnode_t *vnode)
{
	if (vnode->hpNode->html_tag.tag_type == TAG_BR)
	{
		vnode->hx = vnode->font.line_height;
		debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d that the same with its line height", vnode->id, vnode->font.line_height);
		return;
	}

	if (vnode->hx >= 0)
		return;

	switch (vnode->hpNode->html_tag.tag_type)
	{
	case TAG_INPUT:
		vnode->hx = PX4HEIGHT;
		debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d by tag type", vnode->id, vnode->hx);
		return;
	case TAG_SELECT:
		vnode->hx = PX4HEIGHT;
		debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d by tag type", vnode->id, vnode->hx);
		return;
	case TAG_IMG:
	{
		int esti_hx = estimate_hx_by_upper(vnode);
		if (esti_hx > 0)
		{
			vnode->hx = esti_hx;
			vnode->trust += 2;
			debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d that estimate by upper node", vnode->id, vnode->hx);
			return;
		}
		if (vnode->wx <= 100)
		{
			int tmpval = vnode->wx / 3;
			vnode->hx = (tmpval < DEFAULT_IMG_SIZE ? DEFAULT_IMG_SIZE : tmpval);
			debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d that estimate by width", vnode->id, vnode->hx);
		}
		else
		{
			vnode->hx = DEFAULT_IMG_SIZE;
			debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d that use img default height", vnode->id, vnode->hx);
		}
		return;
	}
	case TAG_TEXTAREA:
	{
		const char *val = get_attribute_value(&vnode->hpNode->html_tag, ATTR_ROWS);
		int rows = 0;
		if (val)
			rows = atoi(val);
		if (rows > 0)
		{
			vnode->hx = DEFAULT_CY * rows;
			debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d that calculated by rows", vnode->id, vnode->hx);
		}
		else
		{
			vnode->hx = 2 * DEFAULT_CY;
			debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d that use textarea default height", vnode->id, vnode->hx);
		}
		return;
	}
	case TAG_PURETEXT:
		if (vnode->wx > 0)
		{
			vnode->hx = ((vnode->max_wx + vnode->wx - 1) / vnode->wx) * vnode->font.line_height;
			debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d that calculated by text size and line height", vnode->id, vnode->hx);
		}
		return;
	default:
		vnode->hx = 0;
		debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d that by default", vnode->id, vnode->hx);
		break;
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void push_space_mgr(space_mgr_t *space_mgr, int x, int y, int width)
{
	if (space_mgr->top >= MAX_AVAIL_SPACE_NUM - 1)
	{
		Warn("space stack full!");
		return;
	}
	int cur_top = ++(space_mgr->top);
	space_mgr->space[cur_top].x = x;
	space_mgr->space[cur_top].y = y;
	space_mgr->space[cur_top].width = width;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static avail_space_t pop_space_mgr(space_mgr_t *space_mgr)
{
	int cur_top = space_mgr->top;
	avail_space_t space;
	if (cur_top >= 0)
	{
		space.x = space_mgr->space[cur_top].x;
		space.y = space_mgr->space[cur_top].y;
		space.width = space_mgr->space[cur_top].width;
		space_mgr->top--;
	}
	else
	{
		Warn("space stack is empty!");
		space.x = -1;
		space.y = -1;
		space.width = -1;
	}
	return space;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static avail_space_t select_space(space_mgr_t *space_mgr, int wx)
{
	while (space_mgr->space[space_mgr->top].width < wx)
	{
		if (space_mgr->top == 0)
		{
			break;
		}
		pop_space_mgr(space_mgr);
	}
	return space_mgr->space[space_mgr->top];
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void space_clear(space_mgr_t *space_mgr, u_int property, int parent_wx)
{
	if (IS_CLEAR_LEFT(property))
	{
		while (space_mgr->space[space_mgr->top].x > 0)
		{
			pop_space_mgr(space_mgr);
		}
	}

	if (IS_CLEAR_RIGHT(property))
	{
		while (space_mgr->space[space_mgr->top].x + space_mgr->space[space_mgr->top].width < parent_wx)
		{
			pop_space_mgr(space_mgr);
		}
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void upd_space_mgr_float_left(space_mgr_t *space_mgr, html_vnode_t *node)
{
	int node_bot = node->ypos + node->hx;
	int i = space_mgr->top;
	for (; i >= 0; i--)
	{
		if (node_bot <= space_mgr->space[i].y)
			break;
	}
	i++;
	assert(i <= space_mgr->top && i >= 0);
	avail_space_t space_left = space_mgr->space[i];
	if (i == 0 || space_mgr->space[i - 1].y > node_bot)
	{
		space_mgr->space[i].y = node_bot;
		debuginfo(CALC_VTREE, "[space_mgr] set space[%d].y to %d", i, space_mgr->space[i].y);
		i++;
	}
	int newtop = i;
	int split_x = node->xpos + node->wx;
	space_left.width = space_left.x + space_left.width - split_x;
	space_left.x = split_x;
	for (; i <= space_mgr->top; i++)
	{
		avail_space_t *spc = &(space_mgr->space[i]);
		int split_width = spc->x + spc->width - split_x;
		if (split_width > 0)
		{
			if (split_width == space_left.width)
			{
				space_left.y = spc->y;
			}
			else
			{
				space_mgr->space[newtop++] = space_left;
				space_left.y = spc->y;
				space_left.width = split_width;
			}
		}
		else
			break;
	}
	space_mgr->space[newtop] = space_left;
	debuginfo(CALC_VTREE, "[space_mgr] space[%d] x=%d y=%d width=%d", newtop, space_left.x, space_left.y, space_left.width);
	space_mgr->top = newtop;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void upd_space_mgr_float_right(space_mgr_t *space_mgr, html_vnode_t *node)
{
	int node_bot = node->ypos + node->hx;
	int i = space_mgr->top;
	for (; i >= 0; i--)
	{
		if (space_mgr->space[i].y >= node_bot)
			break;
	}

	i++;
	int keep_i = i;
	avail_space_t space_left = space_mgr->space[i];
	if (i == 0 || space_mgr->space[i - 1].y > node_bot)
	{
		space_mgr->space[i].y = node_bot;
		debuginfo(CALC_VTREE, "[space_mgr] set space[%d].y to %d", i, space_mgr->space[i].y);
		i++;
	}

	space_left.width = node->xpos - space_left.x;
	debuginfo(CALC_VTREE, "[space_mgr] set space[%d].width to %d by node(id=%d)->xpos(%d)-space[%d].x(%d) -", keep_i, space_mgr->space[keep_i].width, node->id, node->xpos, keep_i, space_mgr->space[keep_i].x);
	for (; i <= space_mgr->top; i++)
	{
		avail_space_t spc_tmp = space_mgr->space[i];
		space_mgr->space[i] = space_left;
		debuginfo(CALC_VTREE, "[space_mgr] space[%d].y is %d", i, space_mgr->space[i].y);
		space_left = spc_tmp;
		space_left.width = node->xpos - space_left.x;
	}
	if (space_left.width > 0)
		push_space_mgr(space_mgr, space_left.x, space_left.y, space_left.width);
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void upd_space_mgr_line(space_mgr_t *space_mgr, html_vnode_t *node, int max_line_ypos)
{
	avail_space_t space_left = space_mgr->space[space_mgr->top];
	int i = space_mgr->top;
	for (; i >= 0; i--)
	{
		if (space_mgr->space[i].y >= max_line_ypos)
			break;
	}
	if (i == -1 || space_mgr->space[i].y > max_line_ypos)
	{
		i++;
		space_mgr->space[i].y = max_line_ypos;
		debuginfo(CALC_VTREE, "[space_mgr] set space[%d].y to %d", i, space_mgr->space[i].y);
	}
	space_mgr->top = i;
	if (!IS_LINE_BREAK(node->property))
	{
		int new_x = space_left.x;
		int new_width = space_left.width;
		if (node->wx > 0)
		{
			new_x += node->wx;
			new_width -= node->wx;
		}
		if (new_width > 0)
			push_space_mgr(space_mgr, new_x, space_left.y, new_width);
	}
}

static void eval_hx_in_marquee(html_vnode_t *vnode)
{
	if (vnode->hx < 0)
	{
		vnode->hx = vnode->font.size;
		debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d", vnode->id, vnode->hx);
	}
	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		eval_hx_in_marquee(child);
	}
}

/**
 * @brief
 **/
static void lay_marquee(html_vnode_t *vnode)
{
	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		eval_hx_in_marquee(child);
	}
	int max_hi = 0;
	html_vnode_t *child = vnode->firstChild;
	for (; child; child = child->nextNode)
	{
		if (child->isValid)
		{
			child->xpos = 0;
			child->ypos = 0;
			if (child->hx > max_hi)
			{
				max_hi = child->hx;
			}
		}
	}
	if (max_hi > vnode->hx)
	{
		vnode->hx = max_hi;
		debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d", vnode->id, vnode->hx);
	}
}

/**
 * @brief 推算文本节点的高度
 **/
static int deduce_txtnode_height(int origin_xpos, int origin_ypos, int current_xpos, int current_ypos, int limitWx, unsigned int line_height)
{
	int hi = 0;
	if (current_ypos > origin_ypos)
	{
		hi += current_ypos - origin_ypos;
		if (current_xpos > 0)
			hi += line_height;
	}
	else
	{
		if (current_xpos > origin_xpos)
			hi = line_height;
	}

	return hi;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void build_txt_vnode(html_vnode_t *pbVnode, html_vnode_t *txtVnode, int limitWx)
{
	if (txtVnode->textSize > 0)
	{
		int wxNoWrap = pbVnode->textSize + (txtVnode->textSize) * FONT_SIZE_TO_CHR_WX(txtVnode->font.size);

		if (wxNoWrap >= limitWx)
		{
			pbVnode->wx = limitWx;
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", pbVnode->id, pbVnode->wx);
		}
		else if (wxNoWrap > pbVnode->wx)
		{
			pbVnode->wx = wxNoWrap;
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", pbVnode->id, pbVnode->wx);
		}
		if (limitWx > 0)
		{
			while (wxNoWrap > limitWx)
			{
				wxNoWrap -= limitWx;
				pbVnode->hx += txtVnode->font.line_height;
			}
		}

		pbVnode->textSize = wxNoWrap;
	}
	if (txtVnode->firstChild == NULL && IS_LINE_BREAK(txtVnode->property))
	{
		pbVnode->hx += txtVnode->font.line_height;
		pbVnode->textSize = 0;
	}

	for (html_vnode_t *child = txtVnode->firstChild; child; child = child->nextNode)
	{
		if (!child->isValid)
			continue;
		// text node pos
		int ori_xpos = pbVnode->textSize;
		int ori_ypos = pbVnode->hx;
		child->xpos = ori_xpos;
		child->ypos = ori_ypos;

		build_txt_vnode(pbVnode, child, limitWx);

		child->xpos -= txtVnode->xpos;
		child->ypos -= txtVnode->ypos;

		// text node height
		if (child->hx <= 0)
		{
			child->hx = deduce_txtnode_height(ori_xpos, ori_ypos, pbVnode->textSize, pbVnode->hx, limitWx, child->font.line_height);
			debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d", child->id, child->hx);
		}
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static html_vnode_t *create_text_vnode(html_vnode_t *pbVnode, html_vnode_t *startVnode, int limitWx)
{
	html_vnode_t *endVnode = startVnode;
	while (1)
	{
		html_vnode_t *nextVnode = next_valid_node(endVnode);
		if (nextVnode && IS_TEXT_VNODE(nextVnode->property))
		{
			endVnode = nextVnode;
		}
		else
			break;
	}

	pbVnode->wx = 0;
	pbVnode->hx = 0;
	pbVnode->textSize = 0;
	pbVnode->property = 0;
	for (html_vnode_t *vnode = startVnode;; vnode = vnode->nextNode)
	{
		if (vnode->isValid)
		{
			// text node pos
			vnode->xpos = pbVnode->textSize;
			vnode->ypos = pbVnode->hx;
			build_txt_vnode(pbVnode, vnode, limitWx);
			// text node height
			if (vnode->hx <= 0)
			{
				vnode->hx = deduce_txtnode_height(vnode->xpos, vnode->ypos, pbVnode->textSize, pbVnode->hx, limitWx, vnode->font.line_height);
				debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d", vnode->id, vnode->hx);
			}
		}
		if (vnode == endVnode)
			break;
	}

	if (pbVnode->textSize > 0)
		pbVnode->hx += endVnode->font.line_height;

	if (IS_BREAK_BEFORE(startVnode->property))
	{
		SET_BREAK_BEFORE(pbVnode->property);
	}

	if (IS_LINE_BREAK(endVnode->property))
	{
		SET_LINE_BREAK(pbVnode->property);
	}

	return endVnode;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void txt_block_layout(space_mgr_t *space_mgr, html_vnode_t *txVnode, int *ll_max_ypos)
{
	avail_space_t avail_space = select_space(space_mgr, txVnode->wx);
	txVnode->xpos = avail_space.x;
	txVnode->ypos = avail_space.y;
	debuginfo(CALC_VTREE, "vnode(id=%d) set xpos to %d and ypos to %d", txVnode->id, txVnode->xpos, txVnode->ypos);
	if (txVnode->ypos + txVnode->hx > *ll_max_ypos)
	{
		*ll_max_ypos = txVnode->ypos + txVnode->hx;
		debuginfo(CALC_VTREE, "[ll_max_ypos] set to %d", *ll_max_ypos);
	}
	if (txVnode->wx >= 0 && txVnode->hx >= 0)
		upd_space_mgr_line(space_mgr, txVnode, *ll_max_ypos);
}

/**
 * @brief 获取单元格的指定宽度
 */
static int get_cell_width(html_vnode_t *vnode)
{
	if (!vnode)
	{
		return -1;
	}
	int width = -1;
	char *value = get_attribute_value(&vnode->hpNode->html_tag, ATTR_WIDTH);
	if (value)
	{
		if (isdigit(*value))
		{
			width = atoi(value);
		}
	}
	return width;
}

/**
 * @brief 获取rowspan值
 */
static int get_cell_rowspan(html_vnode_t *vnode)
{
	if (!vnode)
	{
		return -1;
	}
	int rowspan = 1;
	char *rowspan_value = get_attribute_value(&vnode->hpNode->html_tag, ATTR_ROWSPAN);
	if (rowspan_value)
	{
		if (isdigit(*rowspan_value))
		{
			rowspan = atoi(rowspan_value);
			if (rowspan < 1)
			{
				rowspan = 1;
			}
		}
	}
	return rowspan;
}

/**
 * @brief 对齐表格的每一行
 * @author xunwu
 * @date 2011/06/27
 **/
static void align_each_row(html_vnode_t *vnode)
{
	int wx_left = vnode->wx;
	int not_assign_width_num = 0; //没有指定宽度的单元格个数
	html_vnode_t *cell_list = first_cell_list_in_table(vnode->firstChild);
	for (; cell_list; cell_list = next_cell_list(cell_list, NULL))
	{
		int hi = 0; //计算行的最大高度，row_span为1的
		int max_hi = 0; //计算行的最大高度，包括row_span不为1的
		for (html_vnode_t *cell = cell_list; cell; cell = next_cell(cell))
		{
			int row_span = get_cell_rowspan(cell);
			if (cell->hx > hi && row_span == 1)
			{
				hi = cell->hx;
			}
			if (cell->hx > max_hi)
			{
				max_hi = cell->hx;
			}
			int assigned_width = get_cell_width(cell);
			if (assigned_width != -1)
			{
				wx_left -= assigned_width;
			}
			else
			{
				not_assign_width_num++;
			}
		}
		//设置单元格的高度为行的最大高度
		for (html_vnode_t *cell = cell_list; cell; cell = next_cell(cell))
		{
			int row_span = get_cell_rowspan(cell);
			if (row_span == 1)
			{ //对齐row_span为1的
				cell->hx = hi;
				debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d while align height for row of table", cell->id, cell->hx);
			}
			else
			{ //对齐row_span不为1的
				cell->hx = max_hi;
				debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d while align height for row of table", cell->id, cell->hx);
			}
			//对齐单元格的宽度
			int assigned_width = get_cell_width(cell);
			if (assigned_width == -1 && not_assign_width_num == 1)
			{
				if (wx_left > cell->max_wx)
				{
					cell->max_wx = wx_left;
					cell->wx = wx_left;
					debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d and max width to %d while align width for row of table", cell->id, cell->wx, cell->max_wx);
				}
			}
		}
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void txt_vnode_pos_trans(html_vnode_t *start_tvnode, html_vnode_t *end_tvnode, html_vnode_t *vitual_tvnode)
{
	for (html_vnode_t *t = start_tvnode;; t = t->nextNode)
	{
		t->xpos += vitual_tvnode->xpos;
		t->ypos += vitual_tvnode->ypos;
		debuginfo(CALC_VTREE, "vnode(id=%d) set xpos to %d and ypos to %d", t->id, t->xpos, t->ypos);
		if (t == end_tvnode)
			break;
	}
}

/**
 * @brief 计算相对布局下节点的位置及长宽
 * @author xunwu
 * @date 2011/06/27
 **/
static void relative_layout(html_vnode_t * vnode, space_mgr_t *space_mgr)
{
	if (!vnode->isValid)
	{
		return;
	}
	if (vnode->wx < vnode->min_wx)
	{
		if(vnode->whxy&8==0){
			vnode->wx = vnode->min_wx;
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d that the same with its minimum width", vnode->id, vnode->wx);

		}
	}
	if (IS_TEXT_VNODE(vnode->property))
	{
		return;
	}
	//计算叶子节点的高度
	if (!vnode->firstChild)
	{
		compute_leaf_hx(vnode);
		return;
	}

	switch (vnode->hpNode->html_tag.tag_type)
	{
	case TAG_MARQUEE:
		lay_marquee(vnode);
		return;
	case TAG_TABLE:
		align_each_row(vnode);
		break;
	default:
		break;
	}
	space_mgr->top = -1;
	push_space_mgr(space_mgr, 0, 0, vnode->wx);
	avail_space_t avail_space =
	{ -1, -1, -1 };
	int ll_max_ypos = 0;

	html_vnode_t *child = vnode->firstChild;
	for (; child; child = child->nextNode)
	{
		if (!child->isValid)
		{
			continue;
		}
		if (IS_TEXT_VNODE(child->property))
		{
			html_vnode_t txVnode;
			html_vnode_t *endvnode = create_text_vnode(&txVnode, child, vnode->wx);
			txt_block_layout(space_mgr, &txVnode, &ll_max_ypos);
			txt_vnode_pos_trans(child, endvnode, &txVnode);
			child = endvnode;
			continue;
		}

		if (IS_ABSOLUTE(child->property))
		{
		}
		else
		{
			if (IS_CLEAR_LEFT(child->property) || IS_CLEAR_RIGHT(child->property))
				space_clear(space_mgr, child->property, vnode->wx);
			avail_space = select_space(space_mgr, child->wx);

			if (IS_FLOAT_RIGHT(child->property))
			{
				child->xpos = avail_space.x + avail_space.width - child->wx;
				child->ypos = avail_space.y;
				debuginfo(CALC_VTREE, "vnode(id=%d) set xpos to %d and ypos to %d", child->id, child->xpos, child->ypos);
				if (child->wx > 0 && child->hx > 0)
				{
					upd_space_mgr_float_right(space_mgr, child);
				}
			}
			else if (IS_FLOAT_LEFT(child->property))
			{
				child->xpos = avail_space.x;
				child->ypos = avail_space.y;
				debuginfo(CALC_VTREE, "vnode(id=%d) set xpos to %d and ypos to %d", child->id, child->xpos, child->ypos);
				if (child->wx > 0 && child->hx > 0)
					upd_space_mgr_float_left(space_mgr, child);
			}
			else
			{
				child->xpos = avail_space.x;
				child->ypos = avail_space.y;
				debuginfo(CALC_VTREE, "vnode(id=%d) set xpos to %d and ypos to %d", child->id, child->xpos, child->ypos);
				if (child->ypos + child->hx > ll_max_ypos)
				{
					ll_max_ypos = child->ypos + child->hx;
					debuginfo(CALC_VTREE, "[ll_max_ypos] set to %d", ll_max_ypos);
				}
				if (child->wx >= 0 && child->hx >= 0)
					upd_space_mgr_line(space_mgr, child, ll_max_ypos);
			}
		}
	}
	if (space_mgr->space[0].y > vnode->hx && vnode->hx <= 0)
	{
		vnode->hx = space_mgr->space[0].y;
		debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d that calculated by relative layout", vnode->id, vnode->hx);
	}
}

void layout_down_top(html_vnode_t *vnode, space_mgr_t *space_mgr, int depth)
{
	depth++;
	if (depth > 30000)
	{
		return;
	}
	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (child->isValid)
		{
			layout_down_top(child, space_mgr, depth);
		}
	}
	relative_layout(vnode, space_mgr);
}

void compute_absolute_pos(html_vnode_t *vnode)
{
	int baseX = vnode->xpos;
	int baseY = vnode->ypos;
	int adjust_hx = 0;

	for (html_vnode_t * child = vnode->firstChild; child; child = child->nextNode)
	{
		if (!child->isValid)
		{
			continue;
		}
		if (IS_ABSOLUTE(child->property))
		{
			if ((child->property & LEFT_BIT) > 0)
			{
				child->xpos += baseX;
			}
			else if ((child->property & RIGHT_BIT) > 0)
			{
				child->xpos = baseX + vnode->wx - child->wx - child->xpos;
			}
			else
			{
				child->xpos = baseX;
			}
			if ((child->property & TOP_BIT) > 0)
			{
				child->ypos += baseY;
			}
			else if ((child->property & BOTTOM_BIT) > 0)
			{
				child->ypos = baseY + vnode->hx - child->hx - child->ypos;
			}
			else
			{
				child->ypos = baseY;
			}
		}
		else
		{
			child->xpos += baseX;
			child->ypos += baseY;
		}
		compute_absolute_pos(child);
		if (child->hx > 0)
		{
			int hx_span = child->ypos + child->hx - baseY;
			if (hx_span > adjust_hx)
				adjust_hx = hx_span;
		}
	}

	if (adjust_hx > vnode->hx&&!(vnode->whxy&4))
	{
		vnode->hx = adjust_hx;
	}
}

/**
 * @brief 根据宽度百分比设置节点的宽度
 * @author xunwu
 * @date 2011/06/27
 **/
static void get_wx_by_wp(html_vnode_t *vnode, int upLimitWx)
{
	short wp = (vnode->wp >= 100 ? 100 : vnode->wp);
	vnode->wx = ((int) upLimitWx * wp) / 100;
	debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
	if (vnode->wx < vnode->min_wx)
	{
		vnode->wx = vnode->min_wx;
		debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void get_this_wx(html_vnode_t *vnode, int upLimitWx)
{
	if (vnode->wp < 0 && IS_LINE_BREAK(vnode->property) && IS_BREAK_BEFORE(vnode->property) && !IS_ABSOLUTE(vnode->property))
	{ //没有设置宽度百分比，且相对于父节点独自为1行，且不为布局不为绝对位置
		if (upLimitWx < vnode->min_wx)
		{
			vnode->wx = vnode->min_wx;
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
		}
		else if (upLimitWx > vnode->max_wx)
		{
			if (IS_BLOCK_TAG(vnode->property))
			{
				vnode->wx = upLimitWx;
				debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
			}
			else
			{
				vnode->wx = vnode->max_wx;
				debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
			}
		}
		else
		{
			vnode->wx = upLimitWx;
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
		}
		return;
	}

	if (vnode->wp >= 0)
	{
		get_wx_by_wp(vnode, upLimitWx);
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static int getColSpan(html_tag_t *html_tag, int *FLAG_is_bad_table)
{
	const char *val = NULL;
	for (html_attribute_t *attr = html_tag->attribute; attr; attr = attr->next)
	{
		switch (attr->type)
		{
		case ATTR_COLSPAN:
			val = attr->value;
			break;
		case ATTR_ROWSPAN:
		{
			if (attr->value && isdigit(attr->value[0]))
			{
				if (atoi(attr->value) != 1)
					*FLAG_is_bad_table = ILLEGAL_TABLE_ATTR;
			}
		}
			break;
		default:
			break;
		}
	}

	if (val && isdigit(val[0]))
	{
		int i = atoi(val);
		if (i > 0)
			return i;
	}
	return 1;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static html_vnode_t *cell_list_in_tr(html_vnode_t *begin_vnode)
{
	for (html_vnode_t *n = begin_vnode; n; n = n->nextNode)
	{
		if (!n->isValid)
			continue;
		switch (n->hpNode->html_tag.tag_type)
		{
		case TAG_FORM:
		{
			html_vnode_t *cell_list = cell_list_in_tr(n->firstChild);
			if (cell_list)
				return cell_list;
			break;
		}
		case TAG_TD:
		case TAG_TH:
			return n;
		default:
			break;
		}
	}
	return NULL;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static html_vnode_t *first_cell_list_in_table_group(html_vnode_t *begin_vnode)
{
	for (html_vnode_t *n = begin_vnode; n; n = n->nextNode)
	{
		if (!n->isValid)
			continue;
		switch (n->hpNode->html_tag.tag_type)
		{
		case TAG_TR:
		{
			html_vnode_t *cell_list = cell_list_in_tr(n->firstChild);
			if (cell_list)
				return cell_list;
			break;
		}
		case TAG_FORM:
		{
			html_vnode_t *cell_list = first_cell_list_in_table_group(n->firstChild);
			if (cell_list)
				return cell_list;
			break;
		}
		case TAG_THEAD:
		case TAG_TBODY:
		case TAG_TFOOT:
		{
			html_vnode_t *cell_list = first_cell_list_in_table(n->firstChild);
			if (cell_list)
				return cell_list;
			break;
		}
		case TAG_TD:
		case TAG_TH:
			return n;
		default:
			break;
		}
	}

	return NULL;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static html_vnode_t *first_cell_list_in_table(html_vnode_t *begin_vnode)
{
	for (html_vnode_t *n = begin_vnode; n; n = n->nextNode)
	{
		if (!n->isValid)
			continue;
		switch (n->hpNode->html_tag.tag_type)
		{
		case TAG_THEAD:
		case TAG_TBODY:
		case TAG_TFOOT:
		{
			html_vnode_t *cell_list = first_cell_list_in_table_group(n->firstChild);
			if (cell_list)
				return cell_list;
			break;
		}
		case TAG_TR:
		{
			html_vnode_t *cell_list = cell_list_in_tr(n->firstChild);
			if (cell_list)
				return cell_list;
			break;
		}
		case TAG_FORM:
		{
			html_vnode_t *cell_list = first_cell_list_in_table(n->firstChild);
			if (cell_list)
				return cell_list;
			break;
		}
		case TAG_TD:
		case TAG_TH:
			return n;
		default:
			break;
		}
	}

	return NULL;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void set_other_wx_in_tr(html_vnode_t *begin_vnode)
{
	for (html_vnode_t *n = begin_vnode; n; n = n->nextNode)
	{
		if (!n->isValid)
			continue;
		switch (n->hpNode->html_tag.tag_type)
		{
		case TAG_FORM:
		{
			set_other_wx_in_tr(n->firstChild);
			int wx = 0;
			for (html_vnode_t *child = n->firstChild; child; child = child->nextNode)
			{
				if (child->isValid)
					wx += child->wx;
			}
			n->wx = wx;
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", n->id, n->wx);
			break;
		}
		default:
			break;
		}
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void set_other_wx_in_table(html_vnode_t *begin_vnode, int tablewx)
{
	for (html_vnode_t *n = begin_vnode; n; n = n->nextNode)
	{
		if (!n->isValid)
			continue;
		switch (n->hpNode->html_tag.tag_type)
		{
		case TAG_THEAD:
		case TAG_TBODY:
		case TAG_TFOOT:
		{
			n->wx = tablewx;
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", n->id, n->wx);
			set_other_wx_in_table(n->firstChild, tablewx);
			break;
		}
		case TAG_TR:
		{
			n->wx = tablewx;
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", n->id, n->wx);
			set_other_wx_in_tr(n->firstChild);
			break;
		}
		case TAG_FORM:
		{
			n->wx = tablewx;
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", n->id, n->wx);
			set_other_wx_in_table(n->firstChild, tablewx);
			break;
		}
		default:
			break;
		}
	}
}

#define IN_TR			1
#define IN_TABLE_GROUP	2
#define IN_TABLE		3

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static html_vnode_t *next_cell_list(html_vnode_t *curr_cell_list, int *FLAG_is_bad_table)
{
	html_vnode_t *my_body = curr_cell_list->upperNode;
	int stat = IN_TR;
	while (my_body)
	{
		switch (my_body->hpNode->html_tag.tag_type)
		{
		case TAG_TR:
		{
			stat = IN_TABLE_GROUP;
			html_vnode_t *cell_list = first_cell_list_in_table_group(my_body->nextNode);
			if (cell_list)
				return cell_list;
			break;
		}
		case TAG_THEAD:
		case TAG_TBODY:
		case TAG_TFOOT:
		{
			stat = IN_TABLE;
			html_vnode_t *cell_list = first_cell_list_in_table(my_body->nextNode);
			if (cell_list)
				return cell_list;
			break;
		}
		case TAG_FORM:
		{
			html_vnode_t *cell_list = NULL;
			if (stat == IN_TABLE_GROUP)
			{
				cell_list = first_cell_list_in_table_group(my_body->nextNode);
				if (cell_list)
					return cell_list;
			}
			else if (stat == IN_TABLE)
			{
				cell_list = first_cell_list_in_table(my_body->nextNode);
				if (cell_list)
					return cell_list;
			}
			break;
		}
		case TAG_TABLE:
			if (stat == IN_TR && FLAG_is_bad_table != NULL)
			{
				*FLAG_is_bad_table = ILLEGAL_TABLE_STRUCTURE;
			}
			return NULL;
		default:
			break;
		}
		my_body = my_body->upperNode;
	}
	return NULL;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static html_vnode_t *next_cell(html_vnode_t *cell)
{
	html_vnode_t *my_body = cell;
	while (my_body)
	{
		switch (my_body->hpNode->html_tag.tag_type)
		{
		case TAG_TD:
		case TAG_TH:
		{
			html_vnode_t *cell = cell_list_in_tr(my_body->nextNode);
			if (cell)
				return cell;
			break;
		}
		case TAG_FORM:
		{
			html_vnode_t *cell = cell_list_in_tr(my_body->nextNode);
			if (cell)
				return cell;
			break;
		}
		case TAG_TABLE:
		case TAG_TR:
		case TAG_THEAD:
		case TAG_TBODY:
		case TAG_TFOOT:
			return NULL;
		default:
			break;
		}
		my_body = my_body->upperNode;
	}
	return NULL;
}

/**
 * @brief 获取表格的列数
 * @author xunwu
 * @date 2011/06/27
 **/
static int get_col_num(html_vnode_t *vnode, int *FLAG_is_bad_table)
{
	int table_col_num = 0;
	int row_num = 0;
	html_vnode_t *cell_list = first_cell_list_in_table(vnode->firstChild);
	for (; cell_list; cell_list = next_cell_list(cell_list, FLAG_is_bad_table))
	{
		row_num++;
		int col_num = 0;
		for (html_vnode_t *cell = cell_list; cell; cell = next_cell(cell))
		{
			cell->colspan = getColSpan(&cell->hpNode->html_tag, FLAG_is_bad_table);
			if (cell->colspan > MAX_COLSPAN_NUM)
			{
				cell->colspan = MAX_COLSPAN_NUM;
			}
			col_num += cell->colspan;
		}
		if (col_num > table_col_num)
			table_col_num = col_num;
	}
	return table_col_num;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void table_col_clear(table_col_t *tcol, int col_num)
{
	for (int i = 0; i < col_num; i++)
	{
		tcol[i].wx = -1;
		tcol[i].wxlimit = -1;
		tcol[i].span = 1;
		tcol[i].min_wx = -1;
		tcol[i].max_wx = -1;
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void update_col_wx(table_col_t *tcol, int col_no, int wx, int colspan, int *FLAG_is_bad_table)
{
	if (tcol[col_no].wxlimit < 0)
	{
		tcol[col_no].wxlimit = wx;
		tcol[col_no].span = colspan;
		if (colspan == 1)
		{
			if (tcol[col_no].wx >= 0 && tcol[col_no].wx != wx)
				*FLAG_is_bad_table = ILLEGAL_TABLE_ATTR;
			tcol[col_no].wx = wx;
		}
		return;
	}

	if (colspan == tcol[col_no].span)
	{
		if (wx > tcol[col_no].wxlimit)
		{
			tcol[col_no].wxlimit = wx;
			if (colspan == 1)
			{
				if (tcol[col_no].wx >= 0 && tcol[col_no].wx != wx)
					*FLAG_is_bad_table = ILLEGAL_TABLE_ATTR;
				tcol[col_no].wx = wx;
			}
		}
	}
	else if (colspan < tcol[col_no].span)
	{
		int spare_wx = tcol[col_no].wxlimit - wx;
		int spare_colno = col_no - colspan;
		int spare_colspan = tcol[col_no].span - colspan;
		if (spare_wx >= 0)
		{
			update_col_wx(tcol, spare_colno, spare_wx, spare_colspan, FLAG_is_bad_table);
		}
		tcol[col_no].wxlimit = wx;
		tcol[col_no].span = colspan;
		if (colspan == 1)
		{
			if (tcol[col_no].wx >= 0 && tcol[col_no].wx != wx)
				*FLAG_is_bad_table = ILLEGAL_TABLE_ATTR;
			tcol[col_no].wx = wx;
		}
	}
	else
	{ //colspan > tcol[col_no].span
		int spare_wx = wx - tcol[col_no].wxlimit;
		int spare_colno = col_no - tcol[col_no].span;
		int spare_colspan = colspan - tcol[col_no].span;
		if (spare_wx >= 0)
		{
			update_col_wx(tcol, spare_colno, spare_wx, spare_colspan, FLAG_is_bad_table);
		}
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void cmp_cell_list(table_col_t *tcol, html_vnode_t *cell_list, int wxlimit, int *FLAG_is_bad_table)
{
	int col_no = 0;

	for (html_vnode_t *cell = cell_list; cell; cell = next_cell(cell), col_no++)
	{
		col_no += cell->colspan - 1;

		if (cell->wx < 0 && cell->wp >= 0 && cell->wp < 100)
		{
			get_wx_by_wp(cell, wxlimit);
		}

		if (cell->wx >= 0)
		{
			update_col_wx(tcol, col_no, cell->wx, cell->colspan, FLAG_is_bad_table);
		}

		if (cell->colspan == 1)
		{
			if (cell->min_wx > tcol[col_no].min_wx)
				tcol[col_no].min_wx = cell->min_wx;
			if (cell->max_wx > tcol[col_no].max_wx)
				tcol[col_no].max_wx = cell->max_wx;
		}
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void compute_cols(table_col_t *tcol, html_vnode_t *vnode, int *FLAG_is_bad_table)
{
	html_vnode_t *cell_list = first_cell_list_in_table(vnode->firstChild);
	for (; cell_list; cell_list = next_cell_list(cell_list, NULL))
	{
		cmp_cell_list(tcol, cell_list, vnode->wx, FLAG_is_bad_table);
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void eval_noval_cols(table_col_t *tcol, int begin, int end, int sparewx, int tot_flex)
{
	if (tot_flex <= 0)
		tot_flex = 1;
	if (sparewx < 0)
		sparewx = 0;
	for (int i = begin; i <= end; i++)
	{
		if (tcol[i].wx < 0)
		{
			int my_flex = tcol[i].max_wx - tcol[i].min_wx + 1;
			int flex_wx = (sparewx * my_flex) / tot_flex;
			tcol[i].wx = tcol[i].min_wx + flex_wx;
		}
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void evalColsWx(table_col_t *tcol, int col_num, int table_wx)
{
	int noval_num = 0;

	// for those colspan value is too large
	for (int i = 0; i < col_num; i++)
	{
		if (tcol[i].min_wx < 0)
			tcol[i].min_wx = 0;
		if (tcol[i].max_wx < 0)
			tcol[i].max_wx = 1;
	}

	for (int i = 0; i < col_num; i++)
	{
		if (tcol[i].wx < 0)
		{
			noval_num++;
		}
		if (tcol[i].span > 1)
		{
			int cur_tot_min = 0;
			int cur_tot_flex = 0;
			int cur_tot_wx = 0;
			int i_begin = i - tcol[i].span + 1;
			for (int j = i_begin; j <= i; j++)
			{
				if (tcol[j].wx < 0)
				{
					cur_tot_min += tcol[j].min_wx;
					cur_tot_flex += tcol[j].max_wx - tcol[j].min_wx + 1;
					noval_num--;
				}
				else
				{
					cur_tot_wx += tcol[j].wx;
				}
			}
			int sparewx = tcol[i].wxlimit - cur_tot_wx - cur_tot_min;
			eval_noval_cols(tcol, i_begin, i, sparewx, cur_tot_flex);
		}
	}

	if (noval_num > 0)
	{
		int cur_tot_min = 0;
		int cur_tot_flex = 0;
		int cur_tot_wx = 0;
		for (int j = 0; j < col_num; j++)
		{
			if (tcol[j].wx < 0)
			{
				cur_tot_min += tcol[j].min_wx;
				cur_tot_flex += tcol[j].max_wx - tcol[j].min_wx + 1;
			}
			else
			{
				cur_tot_wx += tcol[j].wx;
			}
		}
		int sparewx = table_wx - cur_tot_wx - cur_tot_min;
		eval_noval_cols(tcol, 0, col_num - 1, sparewx, cur_tot_flex);
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static int evalCellWx(html_vnode_t *vnode, table_col_t *tcol, table_col_t *default_col)
{
	html_vnode_t *cell_list = first_cell_list_in_table(vnode->firstChild);

	for (; cell_list; cell_list = next_cell_list(cell_list, NULL))
	{
		int col_no = 0;
		for (html_vnode_t *cell = cell_list; cell; cell = next_cell(cell))
		{
			int tmpwx = 0;
			for (int i = 0; i < cell->colspan; i++)
			{
				tmpwx += tcol[col_no].wx;
				col_no++;
			}
			cell->wx = tmpwx;
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", cell->id, cell->wx);
		}
	}

	cell_list = first_cell_list_in_table(vnode->firstChild);

	for (; cell_list; cell_list = next_cell_list(cell_list, NULL))
	{
		for (html_vnode_t *cell = cell_list; cell; cell = next_cell(cell))
		{
			if (html_vtree_compute_wx(cell, default_col) == PARSE_ERROR)
				return PARSE_ERROR;
		}
	}

	return PARSE_SUCCESS;
}

static int evalCellWx_badtable(html_vnode_t *vnode, table_col_t *default_col)
{
	html_vnode_t *cell_list = first_cell_list_in_table(vnode->firstChild);

	for (; cell_list; cell_list = next_cell_list(cell_list, NULL))
	{
		int spare_wx = vnode->wx;
		int tot_flex = 0;
		//each row
		for (html_vnode_t *cell = cell_list; cell; cell = next_cell(cell))
		{
			spare_wx -= cell->min_wx;
			tot_flex += cell->max_wx - cell->min_wx;
		}
		if (tot_flex <= 0)
			tot_flex = 1;
		for (html_vnode_t *cell = cell_list; cell; cell = next_cell(cell))
		{
			int flex = cell->max_wx - cell->min_wx;
			cell->wx = cell->min_wx + (spare_wx * flex) / tot_flex;
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", cell->id, cell->wx);
			if (html_vtree_compute_wx(cell, default_col) == PARSE_ERROR)
				return PARSE_ERROR;
		}
	}

	return PARSE_SUCCESS;
}

/**
 * @brief 根据表格的列宽调整表格的宽度
 * @author xunwu
 * @date 2011/06/27
 **/
static void adjust_table_wx(html_vnode_t *vnode, table_col_t *tcol, int col_num)
{
	int tot_colwx = 0;
	for (int i = 0; i < col_num; i++)
	{
		tot_colwx += tcol[i].wx;
	}
	if (tot_colwx > vnode->wx)
	{
		vnode->wx = tot_colwx;
		debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
	}
}

/**
 * @brief 检查表格的列宽，看是否为正确的表格
 */
static void check_cols_wx(table_col_t *tcol, int col_num, html_vnode_t *vnode, int *FLAG_is_bad_table)
{
	int wx_cum = 0;
	int has_noval = 0;
	for (int i = 0; i < col_num; i++)
	{
		if (tcol[i].wx >= 0)
		{
			wx_cum += tcol[i].wx;
			if (tcol[i].wx < tcol[i].min_wx)
			{
				*FLAG_is_bad_table = ILLEGAL_TABLE_ATTR;
				return;
			}
		}
		else
			has_noval = 1;
	}
	if (wx_cum > vnode->wx)
	{
		*FLAG_is_bad_table = ILLEGAL_TABLE_ATTR;
		return;
	}
	if (!has_noval && wx_cum < vnode->wx - 1)
	{
		*FLAG_is_bad_table = ILLEGAL_TABLE_ATTR;
		return;
	}
}

/**
 * @brief 计算表格的宽度
 * @author xunwu
 * @date 2011/06/27
 **/
static int html_table_compute_wx(html_vnode_t *vnode, table_col_t *default_col)
{
	int FLAG_is_bad_table = 0;
	int col_num = get_col_num(vnode, &FLAG_is_bad_table);

	if (FLAG_is_bad_table == ILLEGAL_TABLE_STRUCTURE)
	{
		return PARSE_TABLE_FAIL;
	}
	table_col_t *tcol = NULL;
	if (col_num > DEFAULT_TABLE_COL_NUM)
	{
		tcol = (table_col_t *) malloc(col_num * sizeof(table_col_t));
		if (tcol == NULL)
		{
			goto FAIL;
		}
	}
	else
	{
		tcol = default_col;
	}
	table_col_clear(tcol, col_num);
	compute_cols(tcol, vnode, &FLAG_is_bad_table);
	check_cols_wx(tcol, col_num, vnode, &FLAG_is_bad_table);

	if (FLAG_is_bad_table == ILLEGAL_TABLE_ATTR)
	{
		if (evalCellWx_badtable(vnode, default_col) == PARSE_ERROR)
		{
			goto FAIL;
		}
	}
	else
	{
		evalColsWx(tcol, col_num, vnode->wx);
		adjust_table_wx(vnode, tcol, col_num);
		if (evalCellWx(vnode, tcol, default_col) == PARSE_ERROR)
		{
			goto FAIL;
		}
	}
	set_other_wx_in_table(vnode->firstChild, vnode->wx);
	if (tcol != default_col)
	{
		free(tcol);
	}
	return PARSE_SUCCESS;

	FAIL: if (tcol != NULL && tcol != default_col)
	{
		free(tcol);
	}
	return PARSE_ERROR;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void estimate_my_wx(html_vnode_t *mynode, int wxLimit)
{
	int spare_wx = wxLimit;

	for (html_vnode_t *prev = mynode->prevNode; prev; prev = prev->prevNode)
	{
		if (!prev->isValid)
		{
			continue;
		}

		if (IS_LINE_BREAK(prev->property))
		{
			break;
		}

		spare_wx -= prev->wx;

		if (spare_wx < mynode->min_wx)
		{
			goto OUT;
		}

		if (IS_BREAK_BEFORE(prev->property))
		{
			break;
		}
	}

	for (html_vnode_t *next = mynode->nextNode; next; next = next->nextNode)
	{
		if (!next->isValid)
		{
			continue;
		}

		if (IS_BREAK_BEFORE(next->property))
		{
			break;
		}

		spare_wx -= next->wx;

		if (spare_wx < mynode->min_wx)
		{
			goto OUT;
		}

		if (IS_LINE_BREAK(next->property))
		{
			break;
		}
	}

	if (spare_wx == wxLimit)
	{
		mynode->wx = spare_wx;
		debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", mynode->id, mynode->wx);
		mynode->trust += 3;
	}

	OUT: return;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void compute_child_wx(html_vnode_t *vnode)
{
	int wxLimit = vnode->wx;
	int spareWx = wxLimit;
	html_vnode_t *img_vnode_nowx = NULL;
	bool is_esti = true;

	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{

		if (!child->isValid)
		{
			continue;
		}
		if(IS_BLOCK_TAG(child->property)){
			spareWx = wxLimit;
		}
		if (child->wx < 0)
		{
			get_this_wx(child, wxLimit);
		}
		if (IS_ABSOLUTE(child->property))
		{
			if (child->wx < 0)
			{
				if (vnode->wx <= child->min_wx)
				{
					child->wx = child->min_wx;
					debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", child->id, child->wx);
				}
				else if (vnode->wx <= child->max_wx)
				{
					child->wx = vnode->wx;
					debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", child->id, child->wx);
				}
				else
				{
					child->wx = child->max_wx;
					debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", child->id, child->wx);
				}
			}
			continue;
		}
		if (child->wx < 0 && IS_EMBODY_NOWX_IMG(child->property))
		{
			if (img_vnode_nowx == NULL)
			{
				img_vnode_nowx = child;
			}
			else
			{
				is_esti = false;
			}
		}

		if (spareWx < child->min_wx)
		{
			spareWx = wxLimit;
		}
		if (child->wx < 0)
		{
			if (IS_TEXT_VNODE(child->property))
			{
				if (child->max_wx >= wxLimit)
				{
					child->wx = wxLimit;
					debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", child->id, child->wx);
				}
				else
				{
					child->wx = child->max_wx;
					debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", child->id, child->wx);
				}
			}
			else
			{
				if (spareWx <= child->min_wx)
				{
					char *overflow_value = get_css_attribute(child, CSS_PROP_OVERFLOW);
					if (!overflow_value || !strstr(overflow_value, "hidden"))
					{
						child->wx = child->min_wx;
						debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", child->id, child->wx);
					}
				}
				else if (spareWx >= child->max_wx)
				{
					child->wx = child->max_wx;
					debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", child->id, child->wx);
				}
				else
				{
					child->wx = spareWx;
					debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", child->id, child->wx);
				}
			}
		}

		spareWx -= child->wx;

		if (spareWx <= 0)
		{
			spareWx = wxLimit;
		}
		else if (IS_LINE_BREAK(child->property))
		{
			spareWx = wxLimit;
			if (img_vnode_nowx != NULL && is_esti)
			{
				estimate_my_wx(img_vnode_nowx, wxLimit);
			}
			img_vnode_nowx = NULL;
			is_esti = true;
		}
	}
	if (img_vnode_nowx != NULL && is_esti)
	{
		estimate_my_wx(img_vnode_nowx, wxLimit);
	}
}

int html_vtree_compute_wx(html_vnode_t *vnode, table_col_t *default_col)
{
	compute_child_wx(vnode);

	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (!child->isValid)
		{
			continue;
		}
		if (child->hpNode->html_tag.tag_type == TAG_TABLE)
		{
			int ret = html_table_compute_wx(child, default_col);
			if (ret == PARSE_TABLE_FAIL)
			{
				ret = html_vtree_compute_wx(child, default_col);
			}
			if (ret == PARSE_ERROR)
			{
				return ret;
			}
		}
		else
		{
			int ret = html_vtree_compute_wx(child, default_col);
			if (ret == PARSE_ERROR)
			{
				return ret;
			}
		}
	}

	return PARSE_SUCCESS;
}

/**
 * @brief 计算textarea的最大宽度
 * @author xunwu
 * @date 2011/06/27
 **/
static void compute_textarea_wx_limit(html_vnode_t *vnode)
{
	static const int DEFAULT_TEXTAREA_COLS = 22;

	const char *val = get_attribute_value(&vnode->hpNode->html_tag, ATTR_COLS);

	int cols = 0;
	if (val)
		cols = atoi(val);
	if (cols > 0)
	{
		vnode->max_wx = cols * DEFAULT_CX;
	}
	else
	{
		vnode->max_wx = DEFAULT_TEXTAREA_COLS * DEFAULT_CX;
	}

	vnode->min_wx = 0;
}

/**
 * @brief 计算input节点的最大宽度
 * @author xunwu
 * @date 2011/06/27
 **/
static void compute_input_wx_limit(html_vnode_t *vnode)
{
	const char *attrVal = get_attribute_value(&vnode->hpNode->html_tag, ATTR_TYPE);

	if (attrVal == NULL || strcasecmp(attrVal, "text") == 0 || strcasecmp(attrVal, "password") == 0)
	{
		const char *sizeval = get_attribute_value(&vnode->hpNode->html_tag, ATTR_SIZE);
		if (sizeval && isdigit(sizeval[0]))
		{
			int size = atoi(sizeval);
			vnode->max_wx = size * DEFAULT_CX;
		}
		else
			vnode->max_wx = INPUTWIDTH;
	}
	else if (strcasecmp(attrVal, "hidden") == 0)
	{
		vnode->max_wx = 0;
	}
	else if (strcasecmp(attrVal, "checkbox") == 0 || strcasecmp(attrVal, "radio") == 0 || strcasecmp(attrVal, "image") == 0)
	{
		vnode->max_wx = PX4WIDTH;
	}
	else if (strcasecmp(attrVal, "submit") == 0 || strcasecmp(attrVal, "reset") == 0)
	{
		const char *val = get_attribute_value(&vnode->hpNode->html_tag, ATTR_VALUE);
		if (val)
		{
			vnode->max_wx = DEFAULT_CX * strlen(val);
		}
		else
		{
			vnode->max_wx = PX4WIDTH;
		}
	}
	else
		vnode->max_wx = INPUTWIDTH;

	vnode->min_wx = 0;
}

static bool is_only_one_child(html_vnode_t *vnode)
{
	int child_num = 0;

	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (!child->isValid)
			continue;
		child_num++;
		if (child_num >= 2)
		{
			return false;
		}
	}

	return true;
}

static inline bool is_break_both_side(html_vnode_t *vnode)
{
	if (IS_BREAK_BEFORE(vnode->property) && IS_LINE_BREAK(vnode->property))
	{
		return true;
	}

	return false;
}

/**
 * @brief 根据父节点的宽度确定当前节点的宽度
 */
static int estimate_wx_by_upper(html_vnode_t *vnode)
{
	html_vnode_t *upper = vnode->upperNode;
	html_vnode_t *last = vnode;

	while (upper)
	{
		if (!is_break_both_side(last) && !is_only_one_child(upper))
		{
			return -1;
		}

		if (upper->wx > 0)
		{
			return upper->wx;
		}

		last = upper;
		upper = upper->upperNode;
	}

	return -1;
}

/**
 * @brief 计算叶子节点的最大宽度和最小宽度
 * @author xunwu
 * @date 2011/06/27
 **/
static void compute_leaf_wx_limit(html_vnode_t *vnode, int page_width)
{
	switch (vnode->hpNode->html_tag.tag_type)
	{
	case TAG_TEXTAREA:
		compute_textarea_wx_limit(vnode);
		debuginfo(CALC_VTREE, "vnode(id=%d) set min_wx to %d and max_wx to %d", vnode->id, vnode->min_wx, vnode->max_wx);
		break;
	case TAG_INPUT:
		compute_input_wx_limit(vnode);
		debuginfo(CALC_VTREE, "vnode(id=%d) set min_wx to %d and max_wx to %d", vnode->id, vnode->min_wx, vnode->max_wx);
		break;
	case TAG_SELECT:
		vnode->max_wx = SELECTWIDTH;
		vnode->min_wx = 0;
		debuginfo(CALC_VTREE, "vnode(id=%d) set min_wx to %d and max_wx to %d", vnode->id, vnode->min_wx, vnode->max_wx);
		break;
	case TAG_IMG:
	{
		int esti_wx = estimate_wx_by_upper(vnode);
		if (esti_wx > 0)
		{
			vnode->max_wx = esti_wx;
			vnode->trust += 3;
			debuginfo(CALC_VTREE, "vnode(id=%d) set max_wx to %d", vnode->id, vnode->max_wx);
		}
		else if (vnode->wp > 0)
		{
			vnode->max_wx = page_width;
			debuginfo(CALC_VTREE, "vnode(id=%d) set max_wx to %d", vnode->id, vnode->max_wx);
		}
		else
		{
			vnode->max_wx = DEFAULT_IMG_SIZE;
			debuginfo(CALC_VTREE, "vnode(id=%d) set max_wx to %d", vnode->id, vnode->max_wx);
		}

		if (vnode->max_wx >= DEFAULT_IMG_SIZE)
		{
			vnode->min_wx = DEFAULT_IMG_SIZE;
			debuginfo(CALC_VTREE, "vnode(id=%d) set min_wx to %d", vnode->id, vnode->min_wx);
		}
		else
		{
			vnode->min_wx = vnode->max_wx;
			debuginfo(CALC_VTREE, "vnode(id=%d) set min_wx to %d", vnode->id, vnode->min_wx);
		}
		break;
	}
	case TAG_PURETEXT:
	{
		int chr_wx = FONT_SIZE_TO_CHR_WX(vnode->font.size);
		if (vnode->textSize >= 2)
		{
			vnode->min_wx = chr_wx * 2;
			vnode->max_wx = chr_wx * vnode->textSize;
			debuginfo(CALC_VTREE, "vnode(id=%d) set min_wx to %d and max_wx to %d", vnode->id, vnode->min_wx, vnode->max_wx);
		}
		else
		{
			vnode->min_wx = chr_wx * vnode->textSize;
			vnode->max_wx = chr_wx * vnode->textSize;
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d and max_wx to %d", vnode->id, vnode->min_wx, vnode->max_wx);
		}
		break;
	}
	default:
		vnode->min_wx = 0;
		vnode->max_wx = 0;
		debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d and max_wx to %d", vnode->id, vnode->min_wx, vnode->max_wx);
		break;
	}
}

/**
 * @brief 判断子节点中是否含有TD或者TH节点
 * @author xunwu
 * @date 2011/06/27
 **/
static bool has_td(html_vnode_t *vnode)
{
	for (html_vnode_t *n = vnode; n; n = n->nextNode)
	{
		if (n->isValid)
		{
			if (n->hpNode->html_tag.tag_type == TAG_TD || n->hpNode->html_tag.tag_type == TAG_TH)
			{
				return true;
			}
			else
				return false;
		}
	}
	return false;
}

/**
 * @brief 判断是否为TD标签列表
 * @author xunwu
 * @date 2011/06/27
 **/
static bool is_table_cell_list(html_vnode_t *vnode)
{
	for (html_vnode_t *n = vnode; n; n = n->nextNode)
	{
		if (n->isValid)
		{
			switch (n->hpNode->html_tag.tag_type)
			{
			case TAG_TR:
				return false;
			case TAG_FORM:
				if (has_td(n->firstChild))
					return true;
				break;
			case TAG_TD:
				return true;
			default:
				return false;
			}
		}
	}
	return false;
}

/**
 * @brief 判断节点是否为表格中的TR
 * @author xunwu
 * @date 2011/06/27
 **/
static bool is_table_row(html_vnode_t *vnode)
{
	switch (vnode->hpNode->html_tag.tag_type)
	{
	case TAG_TR:
		return true;
	case TAG_TABLE:
	case TAG_THEAD:
	case TAG_TFOOT:
	case TAG_TBODY:
		return is_table_cell_list(vnode->firstChild);
	case TAG_FORM:
		return has_td(vnode->firstChild);
	default:
		return false;
	}
}

/**
 * @brief 计算非叶子节点的最大宽度和最小宽度
 * @author xunwu
 * @date 2011/06/27
 **/
static void compute_wx_limit_by_child(html_vnode_t *vnode)
{
	if (vnode->wx > 0)
	{
		//已经指定了节点的宽度，且overflow属性为hidden
		char *overflow_value = get_css_attribute(vnode, CSS_PROP_OVERFLOW);
		if (overflow_value && strstr(overflow_value, "hidden"))
		{
			vnode->min_wx = vnode->wx;
			vnode->max_wx = vnode->wx;
			debuginfo(CALC_VTREE, "vnode(id=%d) set min_wx to %d and max_wx to %d", vnode->id, vnode->min_wx, vnode->max_wx);
			return;
		}
	}
	int wrap_linewx = 0;
	int nowrap_linewx = 0; //不换行的宽度
	int line_nowx_img_num = 0;
	int max_linewx = 0;
	int max_line_nowx_img_num = 0;
	int max_abswx = 0;
	bool isTableRow = is_table_row(vnode);

	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (!child->isValid)
			continue;
		if (IS_ABSOLUTE(child->property))
		{
			if (child->min_wx > max_abswx)
				max_abswx = child->min_wx;
			continue;
		}

		// update wrap_linewx and nowrap_linewx
		if (isTableRow)
		{
			wrap_linewx += child->min_wx;
		}
		else
		{
			if (child->min_wx > wrap_linewx)
				wrap_linewx = child->min_wx;
		}

		nowrap_linewx += child->max_wx;

		// record no wx img
		if (IS_EMBODY_NOWX_IMG(child->property))
		{
			line_nowx_img_num++;
		}

		// line break now ..
		if (IS_LINE_BREAK(child->property) && !isTableRow)
		{
			if (nowrap_linewx > max_linewx)
			{
				max_linewx = nowrap_linewx;
				max_line_nowx_img_num = line_nowx_img_num;
			}
			nowrap_linewx = 0;
			line_nowx_img_num = 0;
		}
	}

	if (nowrap_linewx > max_linewx)
	{
		max_linewx = nowrap_linewx;
		max_line_nowx_img_num = line_nowx_img_num;
	}

	if (max_line_nowx_img_num == 1)
	{
		SET_EMBODY_NOWX_IMG(vnode->property);
	}

	if (max_linewx <= 0)
	{
		vnode->min_wx = max_abswx;
		vnode->max_wx = max_abswx;
		debuginfo(CALC_VTREE, "vnode(id=%d) set min_wx to %d and max_wx to %d", vnode->id, vnode->min_wx, vnode->max_wx);
	}
	else
	{
		vnode->min_wx = wrap_linewx;
		vnode->max_wx = max_linewx;
		debuginfo(CALC_VTREE, "vnode(id=%d) set min_wx to %d and max_wx to %d", vnode->id, vnode->min_wx, vnode->max_wx);
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static void special_width(html_vnode_t *vnode)
{
	switch (vnode->hpNode->html_tag.tag_type)
	{
	case TAG_SELECT:
		vnode->min_wx = SELECTWIDTH;
		vnode->max_wx = SELECTWIDTH;
		debuginfo(CALC_VTREE, "vnode(id=%d) set min_wx to %d and max_wx to %d", vnode->id, vnode->min_wx, vnode->max_wx);
		break;
	default:
		break;
	}
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
static inline void trans_visit(html_vnode_t *vnode, int page_width)
{
	bool hasRectChild = false;
	bool hasBlockChild = false;
	html_vnode_t *first_valid = NULL; //第一个有效子节点
	html_vnode_t *last_valid = NULL; //最后一个有效子节点
	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (child->isValid)
		{
			if (!IS_TEXT_VNODE(child->property))
			{
				hasRectChild = true;
			}
			if (IS_BLOCK_TAG(child->property))
			{
				hasBlockChild = true;
			}
			last_valid = child;
			if (first_valid == NULL)
				first_valid = child;
		}
	}

	if (hasBlockChild)
		SET_BLOCK_TAG(vnode->property);

	if (!hasRectChild && !is_rect_vnode(vnode))
		SET_TEXT_VNODE(vnode->property);

	if (is_inline_container_tag(vnode->hpNode->html_tag.tag_type))
	{
		if (first_valid && IS_BREAK_BEFORE(first_valid->property))
			SET_BREAK_BEFORE(vnode->property);
		if (first_valid != last_valid && IS_LINE_BREAK(first_valid->property))
			SET_BREAK_BEFORE(vnode->property);
		if (last_valid && IS_LINE_BREAK(last_valid->property))
			SET_LINE_BREAK(vnode->property);
	}

	if (vnode->hpNode->html_tag.tag_type == TAG_IMG)
	{
		vnode->trust = MAX_TRUST_VALUE_FOR_VNODE;

		if (vnode->wx < 0 && vnode->wp < 0)
		{
			vnode->trust -= 3;
			vnode->trust -= 3;
		}

		if (vnode->hx < 0)
		{
			vnode->trust -= 2;
			vnode->trust -= 2;
		}
		if (vnode->wx > 0 && vnode->hx < 0 && vnode->hp < 0)
		{
			//如果图片的高度未设置，则使用其宽度作为高度
//			vnode->hx = vnode->wx;
//			debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d", vnode->id, vnode->hx);
//			vnode->trust += 2;
		}
		else if (vnode->hx > 0 && vnode->wx < 0 && vnode->wp < 0)
		{
			//如果图片的宽度未设置，则使用其高度作为宽度
			vnode->wx = vnode->hx;
			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
			vnode->trust += 3;
		}
		if (vnode->wx < 0 && vnode->wp < 0)
		{
			SET_EMBODY_NOWX_IMG(vnode->property);
		}
	}

	//计算叶子节点的最大宽度和最小宽度
	if (vnode->firstChild == NULL && vnode->wx < 0)
	{
		compute_leaf_wx_limit(vnode, page_width);
		return;
	}

	//计算非叶子节点的最大宽度和最小宽度
	compute_wx_limit_by_child(vnode);
	//非叶子节点的最大宽和最小宽不根据子节点计算的
	special_width(vnode);

	if (vnode->wx >= 0)
	{
		if (flex_bigger(vnode->min_wx, vnode->wx))
		{ //最小宽比宽度大40%以上，或者宽度为0
			if((vnode->whxy&8)==0){
				vnode->wx = -1;
			}

			debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
		}
		else
		{
			vnode->max_wx = vnode->wx;
			vnode->min_wx = vnode->wx;
			debuginfo(CALC_VTREE, "vnode(id=%d) set min_wx to %d and max_wx to %d", vnode->id, vnode->min_wx, vnode->max_wx);
		}
	}

	if (vnode->wx > 0 || vnode->wp > 0)
	{
		CLEAR_EMBODY_NOWX_IMG(vnode->property);
	}
}

void trans_down_top(html_vnode_t *root, int page_width)
{
	html_vnode_t *vnode = root;
	bool down = true;
	while (vnode)
	{
		if (down)
		{
			while (vnode->isValid && vnode->firstChild)
				vnode = vnode->firstChild;
		}

		if (vnode->isValid)
			trans_visit(vnode, page_width);

		if (vnode->nextNode != NULL)
		{
			vnode = vnode->nextNode;
			down = true;
		}
		else
		{
			vnode = vnode->upperNode;
			down = false;
		}
	}
}

/**
 * @brief 设置V树根节点的宽度
 * @author xunwu
 * @date 2011/06/27
 **/
void get_root_wx(html_vnode_t *root, int page_width)
{
	if (page_width < root->min_wx)
	{
		root->wx = root->min_wx;
		debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", root->id, root->wx);
	}
	else
	{
		root->wx = page_width;
		debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", root->id, root->wx);
	}
}

void get_page_width(html_vnode_t *vnode)
{
	int pgwidth = 0;
	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (!vnode->isValid)
			continue;
		switch (child->hpNode->html_tag.tag_type)
		{
		case TAG_BODY:
		case TAG_WAP_CARD:
			child->vtree->body = child;
		case TAG_HTML:
			get_page_width(child);
			// continue to execute the code below
		default:
			if (child->xpos + child->wx > pgwidth)
				pgwidth = child->xpos + child->wx;
			break;
		}
	}

	if (pgwidth < vnode->wx)
	{
		vnode->wx = pgwidth;
		debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
	}
}

static inline void mark_vnode_invalid(html_vnode_t *vnode)
{
	vnode->isValid = 0;
	vnode->trust = -1;
}

static inline void mark_vnode_valid(html_vnode_t *vnode)
{
	vnode->isValid = 1;
	vnode->trust = MAX_TRUST_VALUE_FOR_VNODE;
}

static int mark_subtree_invalid(html_vnode_t *vnode, void *data)
{
	vnode->wx = -1;
	vnode->hx = -1;
	vnode->wp = -1;
	vnode->hp = -1;
	vnode->textSize = -1;
	vnode->cn_num = -1;
	vnode->subtree_textSize = -1;
	vnode->subtree_border_num = 0;
	vnode->subtree_cn_num = -1;
	mark_vnode_invalid(vnode);
	return VISIT_NORMAL;
}

struct css_context_t
{
	easou_css_pool_t *csspool;
	nodepool_t *css_np;
	short selectors_st[CSS_SELECTOR_MAX]; //用来保存各个选择子的状态
	css_pre_status_t pre_st[CSS_PRE_STATUS_MAX]; //用来保存节点的css状态更新情况
	int pre_st_avail;
};

static bool is_important(const char *val)
{
	const char *p = NULL;
	if ((p = strchr(val, '!')) == NULL)
		return false;
	p++;
	while (*p == ' ')
		p++;
	if (strncmp(p, "important", strlen("important")) == 0)
		return true;
	return false;
}

int get_prop_values(easou_css_property_set_t *prop_set, easou_css_property_t *prop_begin, easou_css_property_t *prop_end, int pri_val)
{
	int added_prop_num = 0;
	for (easou_css_property_t *prop = prop_begin; prop != NULL; prop = prop->next)
	{
		int this_pri_val = pri_val;
		if (is_important(prop->value))
			this_pri_val = 2;
		easou_css_prop_info_t *prop_info = &(prop_set->prop[prop->type]);
		if (prop_info->value == NULL)
		{
			added_prop_num++;
			prop_info->value = prop->value;
			prop_info->prio_val = this_pri_val;
		}
		else if (this_pri_val >= prop_info->prio_val)
		{
			prop_info->value = prop->value;
			prop_info->prio_val = this_pri_val;
		}
		if (prop == prop_end)
			break;
	}
	return added_prop_num;
}

/**
 * @brief 向下遍历树时，更新（进化）css匹配的状态，如果完成匹配则保存css规则
 * @author sue
 * @date 2013/04/12
 */
static int update_css_down_status(css_context_t *context, char* key, easou_css_property_set_t* prop_set, html_vnode_t *vnode)
{
	void *value = hashmap_get(context->csspool->hm, key);
	if (value == NULL)
		return 0;

	int numof_css_prop = 0;

	simple_selector_index_node_t *simple = (simple_selector_index_node_t*) value;
	while (simple)
	{
		if (simple->selector_id >= CSS_SELECTOR_MAX)
			goto _CONTINUE;

		if (context->selectors_st[simple->selector_id] == CSS_STATUS_INVALID)
		{
			if (simple->start_pos == simple->pos)
			{
				if (context->pre_st_avail < CSS_PRE_STATUS_MAX)
				{
					css_pre_status_t* pre = &context->pre_st[context->pre_st_avail++];
					pre->selector_id = simple->selector_id;
					pre->old_status = CSS_STATUS_INVALID;
					pre->next = (css_pre_status_t*) vnode->user_ptr;
					vnode->user_ptr = pre;

					context->selectors_st[simple->selector_id] = simple->start_pos;
				}
			}
		}
		else
		{
			if (simple->pos == context->selectors_st[simple->selector_id] + 1)
			{
				if (context->pre_st_avail < CSS_PRE_STATUS_MAX)
				{
					css_pre_status_t* pre = &context->pre_st[context->pre_st_avail++];
					pre->selector_id = simple->selector_id;
					pre->old_status = context->selectors_st[simple->selector_id];
					pre->next = (css_pre_status_t*) vnode->user_ptr;
					vnode->user_ptr = pre;

					context->selectors_st[simple->selector_id] = simple->pos;
				}
			}
		}

		if (context->selectors_st[simple->selector_id] == 0)
			numof_css_prop += get_prop_values(prop_set, simple->ruleset->all_property_list, NULL, -simple->start_pos);

		_CONTINUE: simple = simple->next;
	}

	return numof_css_prop;
}

/**
 * @brief 查询某个节点的css属性，该方法用于遍历树的过程中进行查找
 * @author sue
 * @date 2013/04/12
 */
static int get_css_props(css_context_t *context, easou_css_property_set_t* prop_set, html_vnode_t* vnode)
{
	vnode->user_ptr = NULL;
	html_tag_t* tag = &(vnode->hpNode->html_tag);
	if (tag->tag_name == NULL)
		return 0;

	int name_len = strlen(tag->tag_name);
	if (name_len >= 1024)
		return 0;

	int ret;
	int numof_css_prop = 0;

	char selector_str[1024];
	memcpy(selector_str, tag->tag_name, name_len);
	selector_str[name_len] = 0;

	numof_css_prop += update_css_down_status(context, selector_str, prop_set, vnode);

	if (tag->attr_class)
	{
		char class_str[1024];

		char *begin = tag->attr_class->value;
		char *end = begin;
		int len = 0;
		while (*end)
		{
			while (*end && !easou_isspace(*end))
			{
				end++;
				len++;
			}
			if (len > 0 && len < 256)
			{
				memcpy(class_str, begin, len);
				class_str[len] = 0;

				ret = snprintf(selector_str, 256, "%s.%s", tag->tag_name, class_str);
				if (ret <= 0)
					return numof_css_prop;
				selector_str[ret] = 0;
				numof_css_prop += update_css_down_status(context, selector_str, prop_set, vnode);

				ret = snprintf(selector_str, 256, ".%s", class_str);
				if (ret <= 0)
					return numof_css_prop;
				selector_str[ret] = 0;
				numof_css_prop += update_css_down_status(context, selector_str, prop_set, vnode);
			}
			while (*end && easou_isspace(*end))
			{
				end++;
			}
			begin = end;
			len = 0;
		}
	}

	if (tag->attr_id && tag->attr_id->value)
	{
		int value_len = strlen(tag->attr_id->value);
		if (value_len < 1023)
		{
			ret = sprintf(selector_str, "#%s", tag->attr_id->value);
			if (ret <= 0)
				return numof_css_prop;
			selector_str[ret] = 0;

			numof_css_prop += update_css_down_status(context, selector_str, prop_set, vnode);
		}

		if (value_len + name_len < 1023)
		{
			ret = sprintf(selector_str, "%s#%s", tag->tag_name, tag->attr_id->value);
			if (ret <= 0)
				return numof_css_prop;
			selector_str[ret] = 0;
			numof_css_prop += update_css_down_status(context, selector_str, prop_set, vnode);
		}
	}

	return numof_css_prop;
}

/**
 * @brief 在向上遍历树时，更新（退化）css匹配的状态信息
 * @authro sue
 * @date 2013/04/12
 */
static void up_update_node_css_status(css_context_t *context, html_vnode_t* vnode)
{
	if (vnode->user_ptr == NULL)
		return;

	css_pre_status_t *pre_st = (css_pre_status_t*) vnode->user_ptr;
	while (pre_st != NULL)
	{
		context->selectors_st[pre_st->selector_id] = pre_st->old_status;
		pre_st = pre_st->next;
	}
	vnode->user_ptr = NULL;
}

static int start_add_info_with_css2(html_vnode_t *vnode, void *data)
{
	struct css_context_t *context = (struct css_context_t *) data;

	html_node_t *node = vnode->hpNode;

	easou_css_property_set_t prop_set;
	memset(&prop_set, 0, sizeof(prop_set));

	int numof_css_prop = get_css_props(context, &prop_set, vnode);

	int display = is_display_node(vnode->hpNode);
	if (display == 0)
	{
		html_vnode_visit_ex(vnode, mark_subtree_invalid, NULL, NULL);
		return VISIT_SKIP_CHILD;
	}
	if (numof_css_prop > 0)
	{
		if (create_css_prop_list(vnode, &prop_set, numof_css_prop, context->css_np) == -1)
		{
			return VISIT_ERROR;
		}
		if ((display == 2) && !is_valid_by_css(&prop_set))
		{
			html_vnode_visit_ex(vnode, mark_subtree_invalid, NULL, NULL);
			return VISIT_SKIP_CHILD;
		}
		set_vnode_by_css(vnode, &prop_set);
	}
	if (IS_IN_STYLE(vnode->property))
	{
		for (html_attribute_t *attr = node->html_tag.attribute; attr; attr = attr->next)
		{
			if (ATTR_STYLE == attr->type)
			{
				set_vnode_by_style(vnode, attr->value);
			}
		}
	}
	if (is_block_tag(node->html_tag.tag_type))
	{
		SET_BLOCK_TAG(vnode->property);
		if (is_inline_element(&prop_set, numof_css_prop, vnode))
		{
			if (!IS_FLOAT_RIGHT(vnode->property))
			{
				SET_FLOAT_LEFT(vnode->property);
			}
		}
		if (!IS_FLOAT(vnode->property))
		{
			SET_LINE_BREAK(vnode->property);
		}
	}
	return VISIT_NORMAL;
}

static int finish_add_info_with_css(html_vnode_t *vnode, void *data)
{
	if (!vnode->isValid)
	{
		return VISIT_NORMAL;
	}
	html_tag_type_t tag_type = vnode->hpNode->html_tag.tag_type;
	if (vnode->firstChild == NULL)
	{ //叶子节点
		if (interaction_tag_types[tag_type])
		{
			SET_INCLUDE_INTER(vnode->property);
		}
	}
	html_node_t *node = vnode->hpNode;
	html_vnode_t *firstValid = NULL;
	html_vnode_t *lastValid = NULL;
	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (child->isValid)
		{
			if (IS_INCLUDE_INTER(child->property))
			{ //如果子节点具有交互属性，则传递到当前节点
				SET_INCLUDE_INTER(vnode->property);
			}
			//找第一个有效子节点和最后一个有效子节点
			if (firstValid == NULL)
			{
				firstValid = child;
			}
			lastValid = child;
		}
	}
	if (firstValid && firstValid->hpNode->html_tag.tag_type == TAG_BR)
	{ // first valid node is tag_br
		if (is_inline_container_tag(node->html_tag.tag_type) && firstValid != lastValid)
		{
			SET_BREAK_BEFORE(vnode->property);
		}
	}
	if (lastValid && lastValid->hpNode->html_tag.tag_type == TAG_BR)
	{
		if (is_inline_container_tag(node->html_tag.tag_type))
		{
			SET_LINE_BREAK(vnode->property);
		}
	}

	if (vnode->upperNode && vnode->trust == MAX_TRUST_VALUE_FOR_VNODE && node->html_tag.tag_type != TAG_IMG)
	{
		vnode->trust = vnode->upperNode->trust;
	}
	if (vnode->hpNode->html_tag.tag_type == TAG_PRE)
	{
		deal_pre_tag(vnode);
	}
	html_vnode_t *lastValidBrother = last_valid_node(vnode);
	/*	if(IS_BLOCK_TAG(vnode->property)){
	 if(!IS_FLOAT(vnode->property)){
	 if(!(lastValidBrother && IS_FLOAT(lastValidBrother->property))){
	 SET_BREAK_BEFORE(vnode->property);
	 }
	 }
	 }*/
	if (IS_BREAK_BEFORE(vnode->property))
	{
		if (lastValidBrother && !IS_FLOAT(lastValidBrother->property))
		{
			SET_LINE_BREAK(lastValidBrother->property);
		}
	}
	if (vnode->subtree_textSize < 0)
	{
		trans_text_size(vnode);
	}
	return VISIT_NORMAL;
}

static int finish_add_info_with_css2(html_vnode_t *vnode, void *data)
{
	html_tag_type_t tag_type = vnode->hpNode->html_tag.tag_type;
	if (vnode->firstChild == NULL)
	{ //叶子节点
		if (interaction_tag_types[tag_type])
		{
			SET_INCLUDE_INTER(vnode->property);
		}
	}
	html_node_t *node = vnode->hpNode;
	html_vnode_t *firstValid = NULL;
	html_vnode_t *lastValid = NULL;
	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (child->isValid)
		{
			if (IS_INCLUDE_INTER(child->property))
			{ //如果子节点具有交互属性，则传递到当前节点
				SET_INCLUDE_INTER(vnode->property);
			}
			//找第丄1�7个有效子节点和最后一个有效子节点
			if (firstValid == NULL)
			{
				firstValid = child;
			}
			lastValid = child;
		}
	}
	if (firstValid && firstValid->hpNode->html_tag.tag_type == TAG_BR)
	{ // first valid node is tag_br
		if (is_inline_container_tag(node->html_tag.tag_type) && firstValid != lastValid)
		{
			SET_BREAK_BEFORE(vnode->property);
		}
	}
	if (lastValid && lastValid->hpNode->html_tag.tag_type == TAG_BR)
	{
		if (is_inline_container_tag(node->html_tag.tag_type))
		{
			SET_LINE_BREAK(vnode->property);
		}
	}

	if (vnode->upperNode && vnode->trust == MAX_TRUST_VALUE_FOR_VNODE && node->html_tag.tag_type != TAG_IMG)
	{
		vnode->trust = vnode->upperNode->trust;
	}
	if (vnode->hpNode->html_tag.tag_type == TAG_PRE)
	{
		deal_pre_tag(vnode);
	}
	html_vnode_t *lastValidBrother = last_valid_node(vnode);
	/*	if(IS_BLOCK_TAG(vnode->property)){
	 if(!IS_FLOAT(vnode->property)){
	 if(!(lastValidBrother && IS_FLOAT(lastValidBrother->property))){
	 SET_BREAK_BEFORE(vnode->property);
	 }
	 }
	 }*/
	if (IS_BREAK_BEFORE(vnode->property))
	{
		if (lastValidBrother && !IS_FLOAT(lastValidBrother->property))
		{
			SET_LINE_BREAK(lastValidBrother->property);
		}
	}
	if (vnode->subtree_textSize < 0)
	{
		trans_text_size(vnode);
	}

	struct css_context_t *context = (struct css_context_t *) data;

	up_update_node_css_status(context, vnode);

	return VISIT_NORMAL;
}

void html_vtree_add_info_with_css2(html_vtree_t *html_vtree, easou_css_pool_t *css_pool)
{
	struct css_context_t css_context;
	css_context.csspool = css_pool;
	css_context.css_np = &html_vtree->css_np;
	css_context.pre_st_avail = 0;

	for (int i = 0; i < CSS_SELECTOR_MAX; i++)
		css_context.selectors_st[i] = CSS_STATUS_INVALID;

	html_vnode_visit_ex(html_vtree->root, start_add_info_with_css2, finish_add_info_with_css2, &css_context);
}

static void vnode_set_html_attributes(html_vnode_t *vnode)
{
	html_node_t *node = vnode->hpNode;
	for (html_attribute_t *attr = node->html_tag.attribute; attr; attr = attr->next)
	{
		switch (attr->type)
		{
		case ATTR_STYLE:
			SET_IN_STYLE(vnode->property);
			break;
		case ATTR_BGCOLOR:
			SET_STYLED(vnode->property);
			break;
		case ATTR_COLOR:
		case ATTR_ALIGN:
			SET_HAS_HTML_FONT_ATTR(vnode->property);
			break;
		case ATTR_WIDTH:
			if (attr->value && vnode->wx < 0)
			{
				set_vnode_width(vnode, attr->value);
				//vnode->whxy=vnode->whxy|8;
				debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
			}
			break;
		case ATTR_HEIGHT:
			if (attr->value && vnode->hx < 0)
			{
				set_vnode_height(vnode, attr->value);
				//vnode->whxy=vnode->whxy|4;
				debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d", vnode->id, vnode->hx);
			}
			break;
		default:
			break;
		}
	}
	switch (node->html_tag.tag_type)
	{
	case TAG_TABLE:
	{
		const char *attrValue = get_attribute_value(&node->html_tag, ATTR_ALIGN);
		if (attrValue)
		{
			if (strcasecmp(attrValue, "left") == 0)
			{
				SET_FLOAT_LEFT(vnode->property);
			}
			else if (strcasecmp(attrValue, "right") == 0)
			{
				SET_FLOAT_RIGHT(vnode->property);
			}
		}
		break;
	}
	case TAG_BR:
	{
		SET_LINE_BREAK(vnode->property);
		vnode->wx = 0;
		debuginfo(CALC_VTREE, "vnode(id=%d) set width to %d", vnode->id, vnode->wx);
		vnode->hx = DEFAULT_CY;
		debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d", vnode->id, vnode->hx);
		break;
	}
	case TAG_HR:
	{
		if (vnode->wp == -1)
		{
			vnode->wp = 100;
		}
		vnode->hx = DEFAULT_CY;
		debuginfo(CALC_VTREE, "vnode(id=%d) set height to %d", vnode->id, vnode->hx);
		break;
	}
	default:
		break;
	}
}

static int start_for_html_property(html_vnode_t *vnode, void *data)
{
	html_vnode_t *parent = vnode->upperNode;
	html_node_t *node = vnode->hpNode;
	if ((parent && !parent->isValid) || !is_valid_html_node(node))
	{
		mark_vnode_invalid(vnode);
		return VISIT_NORMAL;
	}
	mark_vnode_valid(vnode);
	vnode_set_html_attributes(vnode);
	return VISIT_NORMAL;
}

static int finish_for_html_property(html_vnode_t *vnode, void *data)
{
	if (!vnode->isValid)
	{
		return VISIT_NORMAL;
	}
	html_node_t *node = vnode->hpNode;
	if (node->html_tag.tag_type == TAG_PURETEXT && node->html_tag.text)
	{
		vnode->textSize = getTextSize(node->html_tag.text, vnode->cn_num);
		vnode->subtree_textSize = vnode->textSize;
		vnode->subtree_cn_num = vnode->cn_num;
	}

	return VISIT_NORMAL;
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
void html_vtree_get_html_property(html_vtree_t *html_vtree)
{
	html_vnode_visit_ex(html_vtree->root, start_for_html_property, finish_for_html_property, NULL);
}

/**
 * @brief
 * @author xunwu
 * @date 2011/06/27
 **/
char *get_css_attribute(html_vnode_t *html_vnode, easou_css_prop_type_t type)
{
	for (css_prop_node_t *cnode = html_vnode->css_prop; cnode; cnode = cnode->next)
	{
		if (cnode->type == type)
		{
			return cnode->value;
		}
	}
	return NULL;
}

static unsigned int color_sign(const char *val)
{
	unsigned int sign = 0;
	for (const char *p = val; *p && *p != ' ' && *p != '!'; p++)
	{
		sign += *p - 'a';
	}
	return sign;
}

static int color_to_rgb(const char *value)
{
	unsigned int sign = color_sign(value);
	unsigned int n = sizeof(g_sign_to_pos) / sizeof(short);

	if (sign >= n)
		return -1;

	int pos = g_sign_to_pos[sign];

	if (pos == -1)
		return -1;

	unsigned int ncolor = sizeof(g_color_str_arr) / sizeof(char *);

	for (unsigned int i = pos; i < ncolor && i < (unsigned int) pos + COLOR_SIGN_MAX_COLLI_NUM; i++)
	{
		if (is_attr_value(value, g_color_str_arr[i], strlen(g_color_str_arr[i])))
		{
			return g_color_value[i];
		}
	}

	return -1;
}

static bool is_font_size(const char *val)
{
	const char *unit = val;
	while (isdigit(*unit) || *unit == '.')
	{
		unit++;
	}

	if (is_attr_value(unit, "px", strlen("px")) || is_attr_value(unit, "pt", strlen("pt")) || is_attr_value(unit, "in", strlen("in")) || is_attr_value(unit, "cm", strlen("cm")) || is_attr_value(unit, "mm", strlen("mm")) || is_attr_value(unit, "pc", strlen("pc"))
			|| is_attr_value(unit, "em", strlen("em")) || is_attr_value(unit, "ex", strlen("ex")) || is_attr_value(unit, "%", strlen("%")))
	{
		return true;
	}

	return false;
}

static void rgb_convert_to_standard(char *st_val, const char *val)
{
	sprintf(st_val, "%c%c%c%c%c%c", val[1], val[1], val[2], val[2], val[3], val[3]);
}

static int parse_color(const char *val)
{
	if (val[0] == '#')
	{
		int l = strlen(val);
		if (l == 4)
		{
			/**
			 * 将#abc形式变为#aabbcc形式
			 */
			char st_val[32];
			st_val[0] = '\0';
			rgb_convert_to_standard(st_val, val);
			return strtol(st_val, NULL, 16);
		}
		else if (l <= 7)
		{
			return strtol(val + 1, NULL, 16);
		}
		else
		{
			char st_val[32];
			st_val[0] = '\0';
			snprintf(st_val, 7, "%s", val + 1);
			return strtol(st_val, NULL, 16);
		}
	}

	if (strncasecmp(val, "rgb", strlen("rgb")) == 0)
	{
		unsigned int rgb_val[3] =
		{ 0 };
		const char *p_val = val + strlen("rgb");
		for (int iter = 0; *p_val && iter < 3; p_val++)
		{
			if (easou_isspace(*p_val))
				continue;
			if (*p_val == '(' && iter == 0)
				continue;
			if (*p_val == ',' && iter >= 1 && iter <= 2)
				continue;
			if (*p_val == ')')
				break;
			if (!isdigit(*p_val))
				break;

			rgb_val[iter++] = atoi(p_val);
			do
			{
				p_val++;
			} while (isdigit(*p_val));
			p_val--; /**退到最后一个数字，避免越界*/
		}

		int color = (rgb_val[0] << 16) + (rgb_val[1] << 8) + rgb_val[2];

		return color;
	}
	return color_to_rgb(val);
}

static void parse_line_height(font_t *font, const char *value)
{
	assert(font->size > 0);

	const char *unit = NULL;

	int size = parse_length(value, font->size, &unit);

	if (size > MAX_FONT_SIZE)
		size = MAX_FONT_SIZE;

	if (size >= 0)
	{
		font->line_height = size;
	}
	else if (unit && *unit == '\0')
	{
		fract_num_t fract;
		atofract(&fract, value);
		if (fract.son < 0)
			fract.son = 0;
		int l = fract.son * font->size / fract.mother;
		if (l > MAX_FONT_SIZE)
			l = MAX_FONT_SIZE;
		font->line_height = l;
	}

	if (font->line_height < font->size)
	{
		font->line_height = font->size;
	}
}

static void parse_size(font_t *font, const char *value, int base_size)
{
	const char *unit = NULL;
	int size = parse_length(value, base_size, &unit);

	if (size > MAX_FONT_SIZE)
		size = MAX_FONT_SIZE;

	if (size >= 0)
	{
		font->size = size;
	}
	else if (unit && *unit == '\0')
	{
		int l = atoi(value);
		if (l > MAX_FONT_SIZE)
			l = MAX_FONT_SIZE;
		font->size = l;
	}
	else
	{
		font->size = base_size;
	}

	if (font->size < 0)
		font->size = base_size;
	if (font->size < 2)
		font->size = 2;
}

static void parse_font_style(font_t *font, const char *value)
{
	if (is_attr_value(value, "italic", strlen("italic")) || is_attr_value(value, "oblique", strlen("oblique")))
	{
		font->is_italic = 1;
	}
	else if (is_attr_value(value, "normal", strlen("normal")))
	{
		font->is_italic = 0;
	}
}

static void parse_font_weight(font_t *font, const char *value)
{
	if (is_attr_value(value, "bold", strlen("bold")) || is_attr_value(value, "bolder", strlen("bolder")))
	{
		font->is_bold = 1;
		return;
	}

	if (is_attr_value(value, "normal", strlen("normal")) || is_attr_value(value, "lighter", strlen("lighter")))
	{
		font->is_bold = 0;
		return;
	}

	if (atoi(value) >= 600)
	{
		/**
		 * 根据对浏览器的实验得出的值,font-weight值大于600
		 * 即出现粗体,这个值更大时，字体粗细似乎无明显改变.
		 */
		font->is_bold = 1;
	}
}

static void parse_font(font_t *font, const char *value, int base_size)
{
#define SCAN_LINE_HEIGHT_NONE	0
#define SCAN_LINE_HEIGHT_NEXT	1
#define SCAN_LINE_HEIGHT_NOW	2
	char val_buf[MAX_ATTR_VALUE_LENGTH];
	snprintf(val_buf, sizeof(val_buf), "%s", value);

	bool over = false;
	int scan_line_hi = SCAN_LINE_HEIGHT_NONE;

	for (char *pval = val_buf; *pval;)
	{
		char *p_end = pval;
		while (!easou_isspace(*p_end) && *p_end != '/' && *p_end != '\0')
		{
			p_end++;
		}
		if (*p_end == '\0')
		{
			over = true;
		}
		else if (*p_end == '/')
		{
			scan_line_hi = SCAN_LINE_HEIGHT_NEXT;
			*p_end = '\0';
		}
		else
		{
			*p_end = '\0';
		}

		if (SCAN_LINE_HEIGHT_NOW == scan_line_hi)
		{
			parse_line_height(font, pval);
		}
		else if (is_font_size(pval))
		{
			parse_size(font, pval, base_size);
		}
		else
		{
			parse_font_style(font, pval);
			parse_font_weight(font, pval);
		}

		if (over)
		{
			break;
		}
		else
		{
			pval = p_end + 1;
			while (easou_isspace(*pval) && *pval != '\0')
			{
				pval++;
			}
		}

		if (scan_line_hi == SCAN_LINE_HEIGHT_NOW)
		{
			scan_line_hi = SCAN_LINE_HEIGHT_NONE;
		}
		else if (scan_line_hi == SCAN_LINE_HEIGHT_NEXT)
		{
			scan_line_hi = SCAN_LINE_HEIGHT_NOW;
		}
	}
#undef SCAN_LINE_HEIGHT_NONE
#undef SCAN_LINE_HEIGHT_NEXT
#undef SCAN_LINE_HEIGHT_NOW
}

static void parse_background(font_t *font, const char *val)
{
	enum
	{
		OUT_BRACKET = 0, /**< 不在括号内       */
		IN_BRACKET = 1, /**< 在括号内       */
	};

	char color_buf[32];
	color_buf[0] = '\0';
	const char *p = val;
	char *q = color_buf;

	const char *p_bufend = color_buf + sizeof(color_buf) - 1;

	/**截出background属性的第一个被空格分开的部分：背景颜色
	 * 只可能在这个部分.
	 */
	for (int stat = OUT_BRACKET; *p && q < p_bufend; p++)
	{
		switch (*p)
		{
		case '(':
			stat = IN_BRACKET;
			break;
		case ')':
			stat = OUT_BRACKET;
			break;
		default:
			break;
		}

		if (!easou_isspace(*p))
			*q++ = *p;
		else if (stat == OUT_BRACKET)
			break;
	}
	*q = '\0';

	int color = parse_color(color_buf);
	if (color >= 0)
		font->bgcolor = color;
}

static void parse_text_decoration(font_t *font, const char *val)
{
	if (is_attr_value(val, "underline", strlen("underline")))
	{
		font->is_underline = 1;
	}

	if (is_attr_value(val, "none", strlen("none")))
	{
		font->is_underline = 0;
	}
}

static int parse_align(const char *val)
{
	if (is_attr_value(val, "center", strlen("center")))
		return VHP_TEXT_ALIGN_CENTER;

	if (is_attr_value(val, "right", strlen("right")))
		return VHP_TEXT_ALIGN_RIGHT;

	if (is_attr_value(val, "left", strlen("left")))
		return VHP_TEXT_ALIGN_LEFT;

	return -1;
}

void get_style_font_info(html_vnode_t *vnode, int base_size, bool &given_line_height)
{
	const char *style_val = get_attribute_value(&(vnode->hpNode->html_tag), ATTR_STYLE);
	if (style_val == NULL)
		return;

	style_attr_t sattr[16];

	const char *line_height_val = NULL;

	int nstyle = parse_style_attr(style_val, sattr, sizeof(sattr) / sizeof(style_attr_t));

	for (int i = 0; i < nstyle; i++)
	{
		if (strcmp(sattr[i].name, "text-align") == 0)
		{
			int ali = parse_align(sattr[i].value);
			if (ali >= 0)
				vnode->font.align = ali;
			continue;
		}

		if (strcmp(sattr[i].name, "text-decoration") == 0)
		{
			parse_text_decoration(&(vnode->font), sattr[i].value);
			continue;
		}

		if (strcmp(sattr[i].name, "background") == 0)
		{
			parse_background(&(vnode->font), sattr[i].value);
			continue;
		}

		if (strcmp(sattr[i].name, "line-height") == 0)
		{
			line_height_val = sattr[i].value;
			continue;
		}

		if (strcmp(sattr[i].name, "background-color") == 0)
		{
			int color = parse_color(sattr[i].value);
			if (color >= 0)
				vnode->font.bgcolor = color;
			continue;
		}

		if (strcmp(sattr[i].name, "color") == 0)
		{
			int color = parse_color(sattr[i].value);
			if (color >= 0)
				vnode->font.color = color;
			continue;
		}

		if (strncmp(sattr[i].name, "font", strlen("font")) != 0)
		{
			continue;
		}

		if (strcmp(sattr[i].name, "font-size") == 0)
		{
			parse_size(&(vnode->font), sattr[i].value, base_size);
			continue;
		}

		if (strcmp(sattr[i].name, "font-style") == 0)
		{
			parse_font_style(&(vnode->font), sattr[i].value);
			continue;
		}

		if (strcmp(sattr[i].name, "font-weight") == 0)
		{
			parse_font_weight(&(vnode->font), sattr[i].value);
			continue;
		}

		if (strcmp(sattr[i].name, "font") == 0)
		{
			parse_font(&(vnode->font), sattr[i].value, base_size);
			continue;
		}
	}

	/**
	 * line-height必须在font-size之后进行解析.
	 */
	if (line_height_val != NULL)
	{
		parse_line_height(&(vnode->font), line_height_val);
		given_line_height = true;
	}
}

void get_css_font_info(html_vnode_t *vnode, int base_size, bool &given_line_height)
{
	const char *line_height_val = NULL;

	for (css_prop_node_t *cssprop = vnode->css_prop; cssprop; cssprop = cssprop->next)
	{
		switch (cssprop->type)
		{
		case CSS_PROP_TEXT_ALIGN:
		{
			int ali = parse_align(cssprop->value);
			if (ali >= 0)
				vnode->font.align = ali;
			break;
		}
		case CSS_PROP_FONT_SIZE:
			parse_size(&(vnode->font), cssprop->value, base_size);
			break;
		case CSS_PROP_FONT_STYLE:
			parse_font_style(&(vnode->font), cssprop->value);
			break;
		case CSS_PROP_FONT_WEIGHT:
			parse_font_weight(&(vnode->font), cssprop->value);
			break;
		case CSS_PROP_FONT:
			parse_font(&(vnode->font), cssprop->value, base_size);
			break;
		case CSS_PROP_LINE_HEIGHT:
			line_height_val = cssprop->value;
			break;
		case CSS_PROP_BACKGROUND:
			parse_background(&(vnode->font), cssprop->value);
			break;
		case CSS_PROP_BACKGROUND_COLOR:
		{
			int color = parse_color(cssprop->value);
			if (color >= 0)
				vnode->font.bgcolor = color;
			break;
		}
		case CSS_PROP_COLOR:
		{
			int color = parse_color(cssprop->value);
			if (color >= 0)
				vnode->font.color = color;
			break;
		}
		case CSS_PROP_TEXT_DECORATION:
			parse_text_decoration(&(vnode->font), cssprop->value);
			break;
		default:
			break;
		}
	}

	/**
	 * line-height必须在font-size之后进行解析.
	 */
	if (line_height_val != NULL)
	{
		parse_line_height(&(vnode->font), line_height_val);
		given_line_height = true;
		if (vnode->font.line_height > vnode->hx)
		{
			vnode->font.line_height = vnode->hx;
		}
	}
}

static int parse_font_tag_size(const char *size_str)
{
	int size_num = atoi(size_str);
	/**
	 * 规一化字体大小的两种表示:(-3到+4)表示法->(1到7)表示法.
	 */
	if ((size_num <= 0 && size_num >= -3) || size_str[0] == '+')
	{
		size_num += 3;
	}
	/*
	 * 异常处理.
	 */
	if (size_num < 1)
		size_num = 1;
	if (size_num > 7)
		size_num = 7;

	/**
	 * font size转化为像素大小.
	 */
	static const int SIZE_TO_PX[] =
	{ 0, 10, 13, 16, 18, 24, 32, 48 };

	return SIZE_TO_PX[size_num];
}

static const unsigned int HEADER_FONT_SIZE[] =
{ 0, 33, 25, 19, 17, 14, 11 };
static const unsigned int HEADER_FONT_LINE_HEIGHT[] =
{ 0, 74, 66, 60, 53, 43, 33 };

static void get_tag_font_info(html_vnode_t *vnode)
{
	/**
	 * 根据tag类型判断
	 */
	html_tag_t *html_tag = &(vnode->hpNode->html_tag);
	switch (html_tag->tag_type)
	{
	case TAG_CENTER:
		vnode->font.align = VHP_TEXT_ALIGN_CENTER;
		break;
	case TAG_H1:
		vnode->font.header_size = 1;
		vnode->font.size = HEADER_FONT_SIZE[1];
		break;
	case TAG_H2:
		vnode->font.header_size = 2;
		vnode->font.size = HEADER_FONT_SIZE[2];
		break;
	case TAG_H3:
		vnode->font.header_size = 3;
		vnode->font.size = HEADER_FONT_SIZE[3];
		break;
	case TAG_H4:
		vnode->font.header_size = 4;
		vnode->font.size = HEADER_FONT_SIZE[4];
		break;
	case TAG_H5:
		vnode->font.header_size = 5;
		vnode->font.size = HEADER_FONT_SIZE[5];
		break;
	case TAG_H6:
		vnode->font.header_size = 6;
		vnode->font.size = HEADER_FONT_SIZE[6];
		break;
	case TAG_B:
		vnode->font.is_bold = 1;
		break;
	case TAG_STRONG:
		vnode->font.is_strong = 1;
		break;
	case TAG_BIG:
		vnode->font.is_big = 1;
		break;
	case TAG_SMALL:
		vnode->font.is_small = 1;
		break;
	case TAG_EM:
	case TAG_I:
		vnode->font.is_italic = 1;
		break;
	case TAG_U:
		vnode->font.is_underline = 1;
		break;
	case TAG_FONT:
	{
		const char *size = get_attribute_value(html_tag, ATTR_SIZE);
		if (size)
		{
			vnode->font.size = parse_font_tag_size(size);
		}
		break;
	}
	default:
		break;
	}

	if (!vnode->isValid || HAS_HTML_FONT_ATTR(vnode->property))
	{
		for (html_attribute_t *attr = html_tag->attribute; attr; attr = attr->next)
		{
			switch (attr->type)
			{
			char valbuf[32];
		case ATTR_BGCOLOR:
			if (attr->value)
			{
				snprintf(valbuf, sizeof(valbuf), "%s", attr->value);
				easou_trans2lower(valbuf, valbuf);
				int color = parse_color(valbuf);
				if (color >= 0)
					vnode->font.bgcolor = color;
			}
			break;
		case ATTR_COLOR:
			if (attr->value)
			{
				snprintf(valbuf, sizeof(valbuf), "%s", attr->value);
				easou_trans2lower(valbuf, valbuf);
				int color = parse_color(valbuf);
				if (color >= 0)
					vnode->font.color = color;
			}
			break;
		case ATTR_ALIGN:
			if (attr->value)
			{
				if (strcasecmp(attr->value, "center") == 0)
					vnode->font.align = VHP_TEXT_ALIGN_CENTER;
				else if (strcasecmp(attr->value, "right") == 0)
					vnode->font.align = VHP_TEXT_ALIGN_RIGHT;
				else if (strcasecmp(attr->value, "left") == 0)
					vnode->font.align = VHP_TEXT_ALIGN_LEFT;
			}
			break;
		default:
			break;
			}
		}
	}
}

static bool is_link_tag(html_vnode_t *vnode)
{
	/**
	 * 当前节点是否链接标签
	 */
	html_vnode_t *upper_node = vnode->upperNode;
	if (upper_node && upper_node->inLink)
	{
		return false;
	}

	return vnode->inLink;
}

static void inherit_font(html_vnode_t *vnode)
{
	html_vnode_t *upper_node = vnode->upperNode;
	if (upper_node != NULL)
	{
		vnode->font = upper_node->font;
	}
	else
	{
		vnode->font.size = DEFAULT_FONT_SIZE;
		vnode->font.line_height = DEFAULT_FONT_SIZE;
		vnode->font.bgcolor = DEFAULT_BGCOLOR;
		vnode->font.color = DEFAULT_COLOR;
	}

	if (is_link_tag(vnode))
	{
		/**
		 * 对于链接，其默认颜色为蓝色且有下划线
		 * 且此特征不继承自父节点。
		 */
		vnode->font.in_link = 1;
		vnode->font.color = DEFAULT_LINK_COLOR;
		vnode->font.is_underline = 1;
	}
}

static void compu_line_height(html_vnode_t *vnode, bool given_line_height)
{
	/**
	 * 若指定了line_height,则不能根据标题字体来计算line_height
	 */
	if (!given_line_height && vnode->font.header_size > 0)
	{
		vnode->font.line_height = HEADER_FONT_LINE_HEIGHT[vnode->font.header_size];
	}

	/**
	 * line-height必须大于等于font-size
	 */
	if (vnode->font.line_height < vnode->font.size)
	{
		vnode->font.line_height = vnode->font.size;
	}
}

static int start_visit_for_font(html_vnode_t *vnode, void *data)
{
	if (vnode->hpNode->html_tag.tag_type == TAG_A && get_attribute_value(&vnode->hpNode->html_tag, ATTR_HREF) != NULL)
	{
		vnode->inLink = 1;
	}
	else if (vnode->upperNode)
	{
		vnode->inLink = vnode->upperNode->inLink;
	}
	inherit_font(vnode);
	int base_size = vnode->font.size;
	bool given_line_height = false;
	if (vnode->font.line_height > base_size) /**继承自父节点的line_height*/
		given_line_height = true;
	get_tag_font_info(vnode);
	get_css_font_info(vnode, base_size, given_line_height);
	if (!vnode->isValid || IS_IN_STYLE(vnode->property))
	{
		get_style_font_info(vnode, base_size, given_line_height);
	}
	compu_line_height(vnode, given_line_height);
	if (vnode->hpNode->html_tag.tag_type == TAG_TABLE)
	{
		//对于表格,其对齐属性不作用于文本.
		vnode->font.align = VHP_TEXT_ALIGN_LEFT;
	}
	return VISIT_NORMAL;
}

static int finish_visit_for_font(html_vnode_t *vnode, void *data)
{
	if (!vnode->isValid)
	{
		return VISIT_NORMAL;
	}
	html_vnode_t *valid_child = NULL;
	int valid_child_num = 0;
	int border_num = 0;
	for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
	{
		if (!child->isValid)
			continue;
		valid_child = child;
		valid_child_num++;
		border_num += child->subtree_border_num;
		if (child->subtree_textSize == vnode->subtree_textSize && child->subtree_textSize > 0)
		{
			/**
			 * 如果一个节点的子节点所在子树包含的文本就是
			 * 这个节点所在的子树包含的文本，则更新这个节
			 * 点的字体大小为其子节点的字体大小，这样做是
			 * 为了方便得到一个区域或整个页面的基本字体大小.
			 */
			vnode->font.size = child->font.size;
		}
	}
	/**
	 * 如果一个节点只有一个有效子节点，则其某些字体属性等于其子元素字体属性.
	 */
	if (valid_child_num == 1)
	{
		vnode->font.bgcolor = valid_child->font.bgcolor;
		/**
		 * 对于链接来说，颜色不能向上传递
		 */
		!is_link_tag(valid_child) && ((vnode->font.color = valid_child->font.color), 1);
		vnode->font.line_height = valid_child->font.line_height;
	}

	//计算vnode->subtree_max_font_size，子树(包括自己)的最大字体
	if (vnode->firstChild == NULL)
	{
		vnode->subtree_max_font_size = vnode->font.size;
		if (vnode->font.size < 40 && vnode->hpNode->html_tag.tag_type == TAG_PURETEXT && vnode->cn_num > 0)
		{
			html_vnode_t *upper = vnode;
			while (upper)
			{
				upper->fontSizes[vnode->font.size] = 1;
				upper = upper->upperNode;
			}
			vnode->subtree_diff_font = 1;
		}
	}
	else
	{
		for (html_vnode_t *child = vnode->firstChild; child; child = child->nextNode)
		{
			if (child->subtree_max_font_size > vnode->subtree_max_font_size)
			{
				vnode->subtree_max_font_size = child->subtree_max_font_size;
			}
		}
		if (vnode->font.size > vnode->subtree_max_font_size)
		{
			vnode->subtree_max_font_size = vnode->font.size;
		}
		for (int i = 0; i < 40; i++)
		{
			if (vnode->fontSizes[i])
			{
				vnode->subtree_diff_font++;
			}
		}
	}

	//计算子树边框个数，设置节点的边框属性
	vnode->subtree_border_num += border_num;
	if (!IS_BORDER(vnode->property))
	{
		char *border_value = get_css_attribute(vnode, CSS_PROP_BORDER);
		if (border_value)
		{
			SET_BORDER(vnode->property);
			vnode->subtree_border_num++;
		}
	}
	return VISIT_NORMAL;
}

int html_vtree_parse_font(html_vtree_t *html_vtree)
{
	html_vnode_visit_ex(html_vtree->root, start_visit_for_font, finish_visit_for_font, NULL);
	return 1;
}


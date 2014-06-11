/*
 * html_dtd.h
 *
 *  Created on: 2011-11-8
 *      Author: ddt
 */

#ifndef EASOU_HTML_DTD_H_
#define EASOU_HTML_DTD_H_

#define HTML_TREE_ATTR_TYPE_NUM (ATTR_UNKNOWN + 1)
#define HTML_TREE_TAG_TYPE_NUM (TAG_UNKNOWN + 1)

typedef enum _html_tag_type_t
{
	TAG_COMMENT, //0
	TAG_DOCTYPE, //1
	TAG_ROOT, //2
	TAG_INSTRUCTION, //3
	TAG_PURETEXT, //4
	TAG_WHITESPACE, //5
	TAG_UNKNOWN, //6
	TAG_A, //7
	TAG_ABBR, //8
	TAG_WAP_ACCESS, //9
	TAG_ACRONYM, //10
	TAG_ADDRESS, 	//11
	TAG_WAP_ANCHOR, //12
	TAG_APPLET, //13
	TAG_AREA, //14
	TAG_B, //15
	TAG_BASE, //16
	TAG_BASEFONT, //17
	TAG_BDO, //18
	TAG_BGSOUND, //19
	TAG_BIG, //20
	TAG_BLINK, //21
	TAG_BLOCKQUOTE, //22
	TAG_BODY, //23
	TAG_BR, //24
	TAG_BUTTON, //25
	TAG_CAPTION, //26
	TAG_WAP_CARD, //27
	TAG_CENTER, //28
	TAG_CITE, //29
	TAG_CODE, //30
	TAG_COL, //31
	TAG_COLGROUP, //32
	TAG_COMMENT2, //33
	TAG_DD, //34
	TAG_DEL, //35
	TAG_DFN, //36
	TAG_DIR, //37
	TAG_DIV, //38
	TAG_DL, //39
	TAG_WAP_DO, //40
	TAG_DT, //41
	TAG_EM, //42
	TAG_EMBED, //43
	TAG_FIELDSET, //44
	TAG_FONT, //45
	TAG_FORM, //46
	TAG_FRAME, //47
	TAG_FRAMESET, //48
	TAG_WAP_GO, //49
	TAG_H1, //50
	TAG_H2, //51
	TAG_H3, //52
	TAG_H4, //53
	TAG_H5, //54
	TAG_H6, //55
	TAG_HEAD, //56
	TAG_HR, //57
	TAG_HTML, //58
	TAG_I, //59
	TAG_IFRAME, //60
	TAG_IMG, //61
	TAG_INPUT, //62
	TAG_INS, //63
	TAG_ISINDEX, //64
	TAG_KBD, //65
	TAG_LABEL, //66
	TAG_LAYER, //67
	TAG_LEGEND, //68
	TAG_LI, //69
	TAG_LINK, //70
	TAG_MAP, //71
	TAG_MARQUEE, //72
	TAG_MENU, //73
	TAG_META, //74
	TAG_MULTICOL, //75
	TAG_NOBR, //76
	TAG_NOEMBED, //77
	TAG_NOFRAME, //78
	TAG_NOLAYER, //79
	TAG_WAP_NOOP, //80
	TAG_NOSCRIPT, //81
	TAG_OBJECT, //82
	TAG_OL, //83
	TAG_WAP_ONEVENT, //84
	TAG_OPTGROUP, //85
	TAG_OPTION, //86
	TAG_P, //87
	TAG_PARAM, //88
	TAG_WAP_POSTFIELD, //89
	TAG_PRE, //90
	TAG_WAP_PREV, //91
	TAG_Q, //92
	TAG_WAP_REFRESH, //93
	TAG_S, //94
	TAG_SAMP, //95
	TAG_SCRIPT, //96
	TAG_SELECT, //97
	TAG_WAP_SETVAR, //98
	TAG_SMALL, //99
	TAG_SOUND, //100
	TAG_SPACER, //101
	TAG_SPAN, //102
	TAG_STRIKE, //103
	TAG_STRONG, //104
	TAG_STYLE, //105
	TAG_SUB, //106
	TAG_SUP, //107
	TAG_TABLE, //108
	TAG_TBODY, //109
	TAG_TD, //110
	TAG_WAP_TEMPLATE, //111
	TAG_TEXTAREA, //112
	TAG_TFOOT, //113
	TAG_TH, //114
	TAG_THEAD, //115
	TAG_WAP_TIMER, //116
	TAG_TITLE, //117
	TAG_TR, //118
	TAG_TT, //119
	TAG_U, //120
	TAG_UL, //121
	TAG_VAR, //122
	TAG_WBR, //123
	TAG_WAP_WML //124
} html_tag_type_t;

typedef enum _html_attr_type_t
{
	ATTR_ABBR,
	ATTR_ACCEPT,
	ATTR_ACCEPTCHARSET,
	ATTR_ACCESSKEY,
	ATTR_ACTION,
	ATTR_ALIGN,
	ATTR_ALINK,
	ATTR_ALT,
	ATTR_ARCHIVE,
	ATTR_AXIS,
	ATTR_BACKGROUND,
	ATTR_BGCOLOR,
	ATTR_BORDER,
	ATTR_CELLPADDING,
	ATTR_CELLSPACING,
	ATTR_CHAR,
	ATTR_CHAROFF,
	ATTR_CHARSET,
	ATTR_CHECKED,
	ATTR_CITE,
	ATTR_CLASS,
	ATTR_CLASSID,
	ATTR_CLEAR,
	ATTR_CODE,
	ATTR_CODEBASE,
	ATTR_CODETYPE,
	ATTR_COLOR,
	ATTR_COLS,
	ATTR_COLSPAN,
	ATTR_COMPACT,
	ATTR_CONTENT,
	ATTR_COORDS,
	ATTR_DATA,
	ATTR_DATETIME,
	ATTR_DECLARE,
	ATTR_DEFER,
	ATTR_DIR,
	ATTR_DISABLED,
	ATTR_ENCTYPE,
	ATTR_FACE,
	ATTR_FOR,
	ATTR_FRAME,
	ATTR_FRAMEBORDER,
	ATTR_HEADERS,
	ATTR_HEIGHT,
	ATTR_HREF,
	ATTR_HREFLANG,
	ATTR_HSPACE,
	ATTR_HTTPEQUIV,
	ATTR_ID,
	ATTR_ISMAP,
	ATTR_LABEL,
	ATTR_LANG,
	ATTR_LANGUAGE,
	ATTR_LINK,
	ATTR_LONGDESC,
	ATTR_MARGINHEIGHT,
	ATTR_MARGINWIDTH,
	ATTR_MAXLENGTH,
	ATTR_MEDIA,
	ATTR_METHOD,
	ATTR_MULTIPLE,
	ATTR_NAME,
	ATTR_NOHREF,
	ATTR_NORESIZE,
	ATTR_NOSHADE,
	ATTR_NOWRAP,
	ATTR_OBJECT,
	ATTR_ONBLUR,
	ATTR_ONCHANGE,
	ATTR_ONCLICK,
	ATTR_ONDBLCLICK,
	ATTR_ONFOCUS,
	ATTR_ONKEYDOWN,
	ATTR_ONKEYPRESS,
	ATTR_ONKEYUP,
	ATTR_ONLOAD,
	ATTR_ONMOUSEDOWN,
	ATTR_ONMOUSEMOVE,
	ATTR_ONMOUSEOUT,
	ATTR_ONMOUSEOVER,
	ATTR_ONMOUSEUP,
	ATTR_ONRESET,
	ATTR_ONSELECT,
	ATTR_ONSUBMIT,
	ATTR_ONUNLOAD,
	ATTR_PROFILE,
	ATTR_PROMPT,
	ATTR_READONLY,
	ATTR_REL,
	ATTR_REV,
	ATTR_ROWS,
	ATTR_ROWSPAN,
	ATTR_RULES,
	ATTR_SCHEME,
	ATTR_SCOPE,
	ATTR_SCROLLING,
	ATTR_SELECTED,
	ATTR_SHAPE,
	ATTR_SIZE,
	ATTR_SPAN,
	ATTR_SRC,
	ATTR_STANDBY,
	ATTR_START,
	ATTR_STYLE,
	ATTR_SUMMARY,
	ATTR_TABINDEX,
	ATTR_TARGET,
	ATTR_TEXT,
	ATTR_TITLE,
	ATTR_TYPE,
	ATTR_USEMAP,
	ATTR_VALIGN,
	ATTR_VALUE,
	ATTR_VALUETYPE,
	ATTR_VERSION,
	ATTR_VLINK,
	ATTR_VSPACE,
	ATTR_WIDTH,
	ATTR_UNKNOWN,
} html_attr_type_t;

typedef struct _html_tag_info_t
{
	char * name;
	html_tag_type_t type;
	char mixed, empty, etag, pre, break_before, break_after;
	char *parents[104];
} html_tag_info_t;

typedef struct _html_tag_pair_t
{
	char * name;
	html_tag_type_t type;
} html_tag_pair_t;

typedef struct _html_attr_pair_t
{
	char * name;
	html_attr_type_t type;
} html_attr_pair_t;

struct html_tag_category_t
{
	char tc_name[50];
	html_tag_type_t tc_type;
	unsigned tc_is_special :1;
	unsigned tc_is_formatting :1;
	unsigned tc_is_scope :1;
};

/*
 * table of tag info
 */
extern html_tag_info_t html_tag_info_array[];

/*
 * table of tag pair ,sorted by str ,for search
 */
extern html_tag_pair_t html_tag_pair_array[];

/*
 * table of html attribute label
 */
extern html_attr_pair_t html_attr_pair_array[];

/*
 * size of table of tag info
 */
extern int html_tag_info_size;

/*
 * size of table of tag pair
 */
extern int html_tag_pair_size;

/*
 * size of attribute array
 */
extern int html_attr_pair_size;

html_tag_type_t get_tag_type(const char *name, size_t len);

const char* get_tag_name(html_tag_type_t type);

html_attr_type_t get_attr_type(const char *name, size_t len);

const char* get_attr_name(html_attr_type_t type);

#endif /* EASOU_HTML_DTD_H_ */

#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "easou_html_dtd.h"
#include<stdio.h>
#define MAX_HTML_LABEL_LEN 128
#define LATIN_CHARACTERS_COUNT 26
#define NUMERIC_CHARACTERS_COUNT 10
#define TAG_MAP_BRANCHES (LATIN_CHARACTERS_COUNT + NUMERIC_CHARACTERS_COUNT + 1)
#define TAG_MAP_LEAF (TAG_MAP_BRANCHES - 1)
#define ATTR_MAP_BRANCHES (LATIN_CHARACTERS_COUNT + 1)
#define ATTR_MAP_LEAF (ATTR_MAP_BRANCHES - 1)

//the array is sorted by sign(str[0]*128 + str[1] + ... + str[len-1])
html_attr_pair_t g_html_attr_pair_array[] =
{
	{"abbr",ATTR_ABBR},
	{"accept",ATTR_ACCEPT},
	{"accept-charset",ATTR_ACCEPTCHARSET},
	{"accesskey",ATTR_ACCESSKEY},
	{"action",ATTR_ACTION},
	{"align",ATTR_ALIGN},
	{"alink",ATTR_ALINK},
	{"alt",ATTR_ALT},
	{"archive",ATTR_ARCHIVE},
	{"axis",ATTR_AXIS},
	{"background",ATTR_BACKGROUND},
	{"bgcolor",ATTR_BGCOLOR},
	{"border",ATTR_BORDER},
	{"cellpadding",ATTR_CELLPADDING},
	{"cellspacing",ATTR_CELLSPACING},
	{"char",ATTR_CHAR},
	{"charoff",ATTR_CHAROFF},
	{"charset",ATTR_CHARSET},
	{"checked",ATTR_CHECKED},
	{"cite",ATTR_CITE},
	{"class",ATTR_CLASS},
	{"classid",ATTR_CLASSID},
	{"clear",ATTR_CLEAR},
	{"code",ATTR_CODE},
	{"codebase",ATTR_CODEBASE},
	{"codetype",ATTR_CODETYPE},
	{"color",ATTR_COLOR},
	{"cols",ATTR_COLS},
	{"colspan",ATTR_COLSPAN},
	{"compact",ATTR_COMPACT},
	{"content",ATTR_CONTENT},
	{"coords",ATTR_COORDS},
	{"data",ATTR_DATA},
	{"datetime",ATTR_DATETIME},
	{"declare",ATTR_DECLARE},
	{"defer",ATTR_DEFER},
	{"dir",ATTR_DIR},
	{"disabled",ATTR_DISABLED},
	{"enctype",ATTR_ENCTYPE},
	{"face",ATTR_FACE},
	{"for",ATTR_FOR},
	{"frame",ATTR_FRAME},
	{"frameborder",ATTR_FRAMEBORDER},
	{"headers",ATTR_HEADERS},
	{"height",ATTR_HEIGHT},
	{"href",ATTR_HREF},
	{"hreflang",ATTR_HREFLANG},
	{"hspace",ATTR_HSPACE},
	{"http-equiv",ATTR_HTTPEQUIV},
	{"id",ATTR_ID},
	{"ismap",ATTR_ISMAP},
	{"label",ATTR_LABEL},
	{"lang",ATTR_LANG},
	{"language",ATTR_LANGUAGE},
	{"link",ATTR_LINK},
	{"longdesc",ATTR_LONGDESC},
	{"marginheight",ATTR_MARGINHEIGHT},
	{"marginwidth",ATTR_MARGINWIDTH},
	{"maxlength",ATTR_MAXLENGTH},
	{"media",ATTR_MEDIA},
	{"method",ATTR_METHOD},
	{"multiple",ATTR_MULTIPLE},
	{"name",ATTR_NAME},
	{"nohref",ATTR_NOHREF},
	{"noresize",ATTR_NORESIZE},
	{"noshade",ATTR_NOSHADE},
	{"nowrap",ATTR_NOWRAP},
	{"object",ATTR_OBJECT},
	{"onblur",ATTR_ONBLUR},
	{"onchange",ATTR_ONCHANGE},
	{"onclick",ATTR_ONCLICK},
	{"ondblclick",ATTR_ONDBLCLICK},
	{"onfocus",ATTR_ONFOCUS},
	{"onkeydown",ATTR_ONKEYDOWN},
	{"onkeypress",ATTR_ONKEYPRESS},
	{"onkeyup",ATTR_ONKEYUP},
	{"onload",ATTR_ONLOAD},
	{"onmousedown",ATTR_ONMOUSEDOWN},
	{"onmousemove",ATTR_ONMOUSEMOVE},
	{"onmouseout",ATTR_ONMOUSEOUT},
	{"onmouseover",ATTR_ONMOUSEOVER},
	{"onmouseup",ATTR_ONMOUSEUP},
	{"onreset",ATTR_ONRESET},
	{"onselect",ATTR_ONSELECT},
	{"onsubmit",ATTR_ONSUBMIT},
	{"onunload",ATTR_ONUNLOAD},
	{"profile",ATTR_PROFILE},
	{"prompt",ATTR_PROMPT},
	{"readonly",ATTR_READONLY},
	{"rel",ATTR_REL},
	{"rev",ATTR_REV},
	{"rows",ATTR_ROWS},
	{"rowspan",ATTR_ROWSPAN},
	{"rules",ATTR_RULES},
	{"scheme",ATTR_SCHEME},
	{"scope",ATTR_SCOPE},
	{"scrolling",ATTR_SCROLLING},
	{"selected",ATTR_SELECTED},
	{"shape",ATTR_SHAPE},
	{"size",ATTR_SIZE},
	{"span",ATTR_SPAN},
	{"src",ATTR_SRC},
	{"standby",ATTR_STANDBY},
	{"start",ATTR_START},
	{"style",ATTR_STYLE},
	{"summary",ATTR_SUMMARY},
	{"tabindex",ATTR_TABINDEX},
	{"target",ATTR_TARGET},
	{"text",ATTR_TEXT},
	{"title",ATTR_TITLE},
	{"type",ATTR_TYPE},
	{"usemap",ATTR_USEMAP},
	{"valign",ATTR_VALIGN},
	{"value",ATTR_VALUE},
	{"valuetype",ATTR_VALUETYPE},
	{"version",ATTR_VERSION},
	{"vlink",ATTR_VLINK},
	{"vspace",ATTR_VSPACE},
	{"width",ATTR_WIDTH},
};

int g_html_attr_pair_size = sizeof(g_html_attr_pair_array) / sizeof(html_attr_pair_t);

struct html_tag_category_t g_html_tag_category_map[] =
{
	    { "#comment",   TAG_COMMENT,  0, 0, 0 },
	    { "#doctype",   TAG_DOCTYPE,  0, 0, 0 },
	    { "#document",  TAG_ROOT,     0, 0, 0 },
	    { "#processing-instruction",    TAG_INSTRUCTION, 0, 0, 0 },
	    { "#text",      TAG_PURETEXT, 0, 0, 0 },
	    { "#text",      TAG_WHITESPACE, 0, 0, 0 },
	    { "#unknown",   TAG_UNKNOWN,      0, 0, 0 },
	    { "a",          TAG_A,        0, 1, 0 },
	    { "abbr",       TAG_ABBR,     0, 0, 0 },
	    { "access",     TAG_WAP_ACCESS,   0, 0, 0 },
	    { "acronym",    TAG_ACRONYM,  0, 0, 0 },
	    { "address",    TAG_ADDRESS,  1, 0, 0 },
	    { "anchor",     TAG_WAP_ANCHOR,   0, 0, 0 },
	    { "applet",     TAG_APPLET,   1, 0, 0 },
	    { "area",       TAG_AREA,     1, 0, 0 },
	    { "b",          TAG_B,        0, 1, 0 },
	    { "base",       TAG_BASE,     1, 0, 0 },
	    { "basefont",   TAG_BASEFONT, 1, 0, 0 },
	    { "bdo",        TAG_BDO,      0, 0, 0 },
	    { "bgsound",    TAG_BGSOUND,  1, 0, 0 },
	    { "big",        TAG_BIG,      0, 1, 0 },
	    { "blink",      TAG_BLINK,    0, 0, 0 },
	    { "blockquote", TAG_BLOCKQUOTE, 1, 0, 0 },
	    { "body",       TAG_BODY,     1, 0, 0 },
	    { "br",         TAG_BR,       1, 0, 0 },
	    { "button",     TAG_BUTTON,   1, 0, 0 },
	    { "caption",    TAG_CAPTION,  1, 0, 0 },
	    { "card",       TAG_WAP_CARD,     0, 0, 0 },
	    { "center",     TAG_CENTER,   1, 0, 0 },
	    { "cite",       TAG_CITE,     0, 0, 0 },
	    { "code",       TAG_CODE,     0, 1, 0 },
	    { "col",        TAG_COL,      1, 0, 0 },
	    { "colgroup",   TAG_COLGROUP, 1, 0, 0 },
	    { "comment",    TAG_COMMENT2, 0, 0, 0 },
	    { "dd",         TAG_DD,       1, 0, 0 },
	    { "del",        TAG_DEL,      0, 0, 0 },
	    { "dfn",        TAG_DFN,      0, 0, 0 },
	    { "dir",        TAG_DIR,      1, 0, 0 },
	    { "div",        TAG_DIV,      1, 0, 0 },
	    { "dl",         TAG_DL,       1, 0, 0 },
	    { "do",         TAG_WAP_DO,       0, 0, 0 },
	    { "dt",         TAG_DT,       1, 0, 0 },
	    { "em",         TAG_EM,       0, 1, 0 },
	    { "embed",      TAG_EMBED,    1, 0, 0 },
	    { "fieldset",   TAG_FIELDSET, 1, 0, 0 },
	    { "font",       TAG_FONT,     0, 1, 0 },
	    { "form",       TAG_FORM,     1, 0, 0 },
	    { "frame",      TAG_FRAME,    1, 0, 0 },
	    { "frameset",   TAG_FRAMESET, 1, 0, 0 },
	    { "go",         TAG_WAP_GO,       0, 0, 0 },
	    { "h1",         TAG_H1,       1, 0, 0 },
	    { "h2",         TAG_H2,       1, 0, 0 },
	    { "h3",         TAG_H3,       1, 0, 0 },
	    { "h4",         TAG_H4,       1, 0, 0 },
	    { "h5",         TAG_H5,       1, 0, 0 },
	    { "h6",         TAG_H6,       1, 0, 0 },
	    { "head",       TAG_HEAD,     1, 0, 0 },
	    { "hr",         TAG_HR,       1, 0, 0 },
	    { "html",       TAG_HTML,     1, 0, 1 },
	    { "i",          TAG_I,        0, 1, 0 },
	    { "iframe",     TAG_IFRAME,   1, 0, 0 },
	    { "img",        TAG_IMG,      1, 0, 0 },
	    { "input",      TAG_INPUT,    1, 0, 0 },
	    { "ins",        TAG_INS,      0, 0, 0 },
	    { "isindex",    TAG_ISINDEX,  0, 0, 0 },
	    { "kbd",        TAG_KBD,      0, 0, 0 },
	    { "label",      TAG_LABEL,    0, 0, 0 },
	    { "layer",      TAG_LAYER,    0, 0, 0 },
	    { "legend",     TAG_LEGEND,   0, 0, 0 },
	    { "li",         TAG_LI,       1, 0, 0 },
	    { "link",       TAG_LINK,     1, 0, 0 },
	    { "map",        TAG_MAP,      0, 0, 0 },
	    { "marquee",    TAG_MARQUEE,  1, 0, 0 },
	    { "menu",       TAG_MENU,     1, 0, 0 },
	    { "meta",       TAG_META,     1, 0, 0 },
	    { "multicol",   TAG_MULTICOL, 0, 0, 0 },
	    { "nobr",       TAG_NOBR,     0, 1, 0 },
	    { "noembed",    TAG_NOEMBED,  1, 0, 0 },
	    { "noframes",   TAG_NOFRAME,  1, 0, 0 },
	    { "nolayer",    TAG_NOLAYER,  0, 0, 0 },
	    { "noop",       TAG_WAP_NOOP,     0, 0, 0 },
	    { "noscript",   TAG_NOSCRIPT, 1, 0, 0 },
	    { "object",     TAG_OBJECT,   1, 0, 0 },
	    { "ol",         TAG_OL,       1, 0, 0 },
	    { "onevent",    TAG_WAP_ONEVENT,  0, 0, 0 },
	    { "optgroup",   TAG_OPTGROUP, 0, 0, 0 },
	    { "option",     TAG_OPTION,   0, 0, 0 },
	    { "p",          TAG_P,        1, 0, 0 },
	    { "param",      TAG_PARAM,    1, 0, 0 },
	    { "postfield",  TAG_WAP_POSTFIELD, 0, 0, 0 },
	    { "pre",        TAG_PRE,      1, 0, 0 },
	    { "prev",       TAG_WAP_PREV,     0, 0, 0 },
	    { "q",          TAG_Q,        0, 0, 0 },
	    { "refresh",    TAG_WAP_REFRESH,  0, 0, 0 },
	    { "s",          TAG_S,        0, 1, 0 },
	    { "samp",       TAG_SAMP,     0, 0, 0 },
	    { "script",     TAG_SCRIPT,   1, 0, 0 },
	    { "select",     TAG_SELECT,   1, 0, 0 },
	    { "setvar",     TAG_WAP_SETVAR,   0, 0, 0 },
	    { "small",      TAG_SMALL,    0, 1, 0 },
	    { "sound",      TAG_SOUND,    0, 0, 0 },
	    { "spacer",     TAG_SPACER,   0, 0, 0 },
	    { "span",       TAG_SPAN,     0, 0, 0 },
	    { "strike",     TAG_STRIKE,   0, 1, 0 },
	    { "strong",     TAG_STRONG,   0, 1, 0 },
	    { "style",      TAG_STYLE,    1, 0, 0 },
	    { "sub",        TAG_SUB,      0, 0, 0 },
	    { "sup",        TAG_SUP,      0, 0, 0 },
	    { "table",      TAG_TABLE,    1, 0, 1 },
	    { "tbody",      TAG_TBODY,    1, 0, 0 },
	    { "td",         TAG_TD,       1, 0, 1 },
	    { "template",   TAG_WAP_TEMPLATE, 0, 0, 0 },
	    { "textarea",   TAG_TEXTAREA, 0, 0, 0 },
	    { "tfoot",      TAG_TFOOT,    1, 0, 0 },
	    { "th",         TAG_TH,       1, 0, 1 },
	    { "thead",      TAG_THEAD,    1, 0, 0 },
	    { "timer",      TAG_WAP_TIMER,    0, 0, 0 },
	    { "title",      TAG_TITLE,    1, 0, 0 },
	    { "tr",         TAG_TR,       1, 0, 0 },
	    { "tt",         TAG_TT,       0, 1, 0 },
	    { "u",          TAG_U,        0, 1, 0 },
	    { "ul",         TAG_UL,       1, 0, 0 },
	    { "var",        TAG_VAR,      0, 0, 0 },
	    { "wbr",        TAG_WBR,      1, 0, 0 },
	    { "wml",        TAG_WAP_WML,      0, 0, 0 }
    /* XMP is HTML4 element but special behavior, not support yet
    { "xmp",        TAG_XMP,      0, 0, 0 }, */

    /* HTML5 new elements, not support yet
    { "article",    TAG_ARTICLE, 0, 0, 0 },
    { "aside",      TAG_ASIDE, 0, 0, 0 },
    { "audio",      TAG_AUDIO, 0, 0, 0 },
    { "bdi",        TAG_BDI, 0, 0, 0 },
    { "canvas",     TAG_CANVAS, 0, 0, 0 },
    { "command",    TAG_COMMAND, 0, 0, 0 },
    { "datalist",   TAG_DATALIST, 0, 0, 0 },
    { "details",    TAG_DETAILS, 0, 0, 0 },
    { "figcaption", TAG_FIGCAPTION, 0, 0, 0 },
    { "figure",     TAG_FIGURE, 0, 0, 0 },
    { "footer",     TAG_FOOTER, 0, 0, 0 },
    { "header",     TAG_HEADER, 0, 0, 0 },
    { "hgroup",     TAG_HGROUP, 0, 0, 0 },
    { "keygen",     TAG_KEYGEN, 0, 0, 0 },
    { "mark",       TAG_MARK, 0, 0, 0 },
    { "meter",      TAG_METER, 0, 0, 0 },
    { "nav",        TAG_NAV, 0, 0, 0 },
    { "output",     TAG_OUTPUT, 0, 0, 0 },
    { "progress",   TAG_PROGRESS, 0, 0, 0 },
    { "rp",         TAG_RP, 0, 0, 0 },
    { "rt",         TAG_RT, 0, 0, 0 },
    { "ruby",       TAG_RUBY, 0, 0, 0 },
    { "section",    TAG_SECTION, 0, 0, 0 },
    { "source",     TAG_SOURCE, 0, 0, 0 },
    { "summary",    TAG_SUMMARY, 0, 0, 0 },
    { "time",       TAG_TIME, 0, 0, 0 },
    { "track",      TAG_TRACK, 0, 0, 0 },
    { "video",      TAG_VIDEO, 0, 0, 0 },
     * HTML5 new elements end here */
};

int g_html_tag_category_size = sizeof(g_html_tag_category_map) / sizeof(struct html_tag_category_t);

/* a-z => 0-25, A-Z => 0-25, 0-9 => 26-36 */
char g_tag_char_map[] =
{
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

/* a-z => 0-25, A-Z => 0-25 */
char g_attr_char_map[] =
{
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

/* a-zA-Z */
char g_latin_map[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* whitespace > / */
char g_tag_name_map[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
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
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* whitespace / = > */
char g_attribute_name_map[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
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
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* whitespace > */
char g_attribute_value_uq_map[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
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
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

html_tag_type_t get_tag_type(const char *name, size_t len)
{
	if (len > MAX_HTML_LABEL_LEN)
	{
		return TAG_UNKNOWN;
	}
	int highIndex = g_html_tag_category_size - 1;
	int lowIndex = 0;
	int mid = 0;
	while (true)
	{
		mid = (highIndex - lowIndex) / 2 + lowIndex;
		size_t tmp = strlen(g_html_tag_category_map[mid].tc_name);
		int cmplen = len > tmp ? tmp : len;
		int ret = strncasecmp(g_html_tag_category_map[mid].tc_name, name, cmplen);
		if (ret == 0 && tmp == len)
		{
			return g_html_tag_category_map[mid].tc_type;
		}
		else if (ret > 0 || (ret == 0 && tmp > len))
		{
			highIndex = mid - 1;
			if (highIndex < lowIndex)
			{
				break;
			}
		}
		else if (ret < 0 || (ret == 0 && tmp < len))
		{
			lowIndex = mid + 1;
			if (lowIndex > highIndex)
			{
				break;
			}
		}
	}

	/*
	 for(int i = 0; i < g_html_tag_category_size; i++){
	 size_t tmp = strlen(g_html_tag_category_map[i].tc_name);
	 tmp = len > tmp ? len:tmp;
	 if(0 == strncasecmp(g_html_tag_category_map[i].tc_name, name, tmp)){
	 return g_html_tag_category_map[i].tc_type;
	 }
	 }
	 */
	return TAG_UNKNOWN;
}

const char* get_tag_name(html_tag_type_t type)
{
	return g_html_tag_category_map[type].tc_name;
}

html_attr_type_t get_attr_type(const char *name, size_t len)
{
	if (len > MAX_HTML_LABEL_LEN)
	{
		return ATTR_UNKNOWN;
	}
	int highIndex = g_html_attr_pair_size - 1;
	int lowIndex = 0;
	int mid = 0;
	while (true)
	{
		mid = (highIndex - lowIndex) / 2 + lowIndex;
		size_t tmp = strlen(g_html_attr_pair_array[mid].name);
		tmp = len > tmp ? len : tmp;
		int ret = strncasecmp(g_html_attr_pair_array[mid].name, name, tmp);
		if (ret == 0)
		{
			return g_html_attr_pair_array[mid].type;
		}
		else if (ret > 0)
		{
			highIndex = mid - 1;
			if (highIndex < lowIndex)
			{
				break;
			}
		}
		else if (ret < 0)
		{
			lowIndex = mid + 1;
			if (lowIndex > highIndex)
			{
				break;
			}
		}
	}

	/*
	 for(int i = 0; i < g_html_attr_pair_size; i++){
	 size_t tmp = strlen(g_html_attr_pair_array[i].name);
	 tmp = len > tmp ? len:tmp;
	 if(0 == strncasecmp(g_html_attr_pair_array[i].name, name, tmp)){
	 return g_html_attr_pair_array[i].type;
	 }
	 }
	 */
	return ATTR_UNKNOWN;
}

const char* get_attr_name(html_attr_type_t type)
{
	if (type < g_html_attr_pair_size)
	{
		return g_html_attr_pair_array[type].name;
	}
	else
	{
		return NULL;
	}
}

//shuangwei add 2012-04-23 鐢ㄤ簬寮�鍙戜汉鍛樿皟璇�
int g_EASOU_DEBUG = 0;
FILE * g_logfile = 0;
char * g_debugbuf = NULL;
int g_debugbuflen = 0;

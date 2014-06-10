/*
 * vtree_print_utils.cpp
 *
 *  Created on: 2013-7-1
 *      Author: sue
 */

#include "vtree_print_utils.h"
#include "easou_html_extractor.h"

struct print_visit_t
{
	char* buf;
	int avail;
	int size;
	int page_hx;
	int page_wx;
	int node_depth;
	int depth;
};

#define assert_printf(visit, format, ...) \
{ \
	int ret = snprintf(visit->buf + visit->avail, visit->size - visit->avail, format, ##__VA_ARGS__); \
	assert(ret < (visit->size - visit->avail));	\
	visit->avail += ret; \
	visit->buf[visit->avail] = 0; \
}

static void print_vnode_font(html_vnode_t* vnode, print_visit_t* data)
{
	font_t font = vnode->font;
	assert_printf(data, "\t[font] size:%u color:%u\n", font.size, font.color);
}

static int start_visit_for_printinfo(html_vnode_t* vnode, void* result)
{
	if (vnode->hpNode->html_tag.tag_type == TAG_ROOT)
		return VISIT_NORMAL;

	assert(vnode->struct_info);
	if (vnode->isValid == false)
		return VISIT_SKIP_CHILD;

	print_visit_t* data = (print_visit_t*) result;
	data->node_depth++;
	for (int i = 0; i < data->node_depth; i++)
		assert_printf(data, " ");

	html_tag_t& tag = vnode->hpNode->html_tag;
	if (vnode->hpNode->html_tag.tag_type == TAG_PURETEXT)
	{
		assert_printf(data,
		"<span id=%d no=\"no%d\" class=\"%s\" id=\"%s\" title=\"x:%d,y:%d,"
		"valid:%d, property:%d, float_left:%d, float_right:%d, break_before:%d, line_break:%d, is_text:%d, "
		"repeat:%d,similar:%d,repeat_type_num:%d,"
		"padding-top:%d-right:%d-bottom:%d-left:%d,"
		"valid_leaf_num:%d,valid_child_num:%d\""
		" style=\"width:%dpx;height:%dpx;position:absolute;left:%dpx;top:%dpx;line-height:%dpx;\">%s</span>\n",
		vnode->id, vnode->hpNode->html_tag.tag_code,
		tag.attr_class ? tag.attr_class->value : "", tag.attr_id ? tag.attr_id->value : "",
		vnode->xpos, vnode->ypos,
		vnode->isValid,
		vnode->property, 
		IS_FLOAT_LEFT(vnode->property) ? 1 : 0,
		IS_FLOAT_RIGHT(vnode->property) ? 1 : 0,
		IS_BREAK_BEFORE(vnode->property) ? 1 : 0,
		IS_LINE_BREAK(vnode->property) ? 1 : 0,
		IS_TEXT_VNODE(vnode->property) ? 1 : 0,
		vnode->struct_info->is_self_repeat, vnode->struct_info->self_similar_value, vnode->struct_info->repeat_type_num,
	  vnode->border.pad_top, vnode->border.pad_right, vnode->border.pad_bottom, vnode->border.pad_left,
	  vnode->struct_info->valid_leaf_num, vnode->struct_info->valid_child_num,
		vnode->wx, vnode->hx, vnode->xpos - vnode->upperNode->xpos, vnode->ypos - vnode->upperNode->ypos,
		vnode->font.line_height,
		vnode->hpNode->html_tag.text ? vnode->hpNode->html_tag.text : "");

		print_vnode_font(vnode, data);
	}
	else if (!vnode->hpNode->html_tag.is_self_closed)
	{
		assert_printf(data, "<%s id=%d code=\"no%d\" class=\"%s\" id=\"%s\" title=\"x:%d,y:%d,"
		"valid:%d, property:%d, float_left:%d, float_right:%d, break_before:%d, line_break:%d, is_text:%d, "
		"repeat:%d,similar:%d,repeat_type_num:%d,"
		"repeat_sibling:%d,"
		"padding-top:%d-right:%d-bottom:%d-left:%d,"
		"border-top:%d-right:%d-bottom:%d-left:%d,"
		"subtree_text_size:%d,subtree_cn_num:%d,"
		"valid_leaf_num:%d,valid_child_num:%d\""
		" style=\"width:%dpx;height:%dpx;position:absolute;left:%dpx;top:%dpx;line-height:%dpx\">",
		vnode->hpNode->html_tag.tag_name, vnode->id, vnode->hpNode->html_tag.tag_code,
		tag.attr_class ? tag.attr_class->value : "", tag.attr_id ? tag.attr_id->value : "",
		vnode->xpos, vnode->ypos,
		vnode->isValid,
		vnode->property,
		IS_FLOAT_LEFT(vnode->property) ? 1 : 0,
		IS_FLOAT_RIGHT(vnode->property) ? 1 : 0,
		IS_BREAK_BEFORE(vnode->property) ? 1 : 0,
		IS_LINE_BREAK(vnode->property) ? 1 : 0,
		IS_TEXT_VNODE(vnode->property) ? 1 : 0,
		vnode->struct_info->is_self_repeat, vnode->struct_info->self_similar_value, vnode->struct_info->repeat_type_num,
		vnode->struct_info->is_repeat_with_sibling,
		vnode->border.pad_top, vnode->border.pad_right, vnode->border.pad_bottom, vnode->border.pad_left,
		vnode->border.top, vnode->border.right, vnode->border.bottom, vnode->border.left,
		vnode->subtree_textSize,vnode->subtree_cn_num,
		vnode->struct_info->valid_leaf_num, vnode->struct_info->valid_child_num,
		vnode->wx, vnode->hx, vnode->xpos - vnode->upperNode->xpos, vnode->ypos - vnode->upperNode->ypos,
		vnode->font.line_height);
	}
	else if (vnode->hpNode->html_tag.is_self_closed)
	{
		assert_printf(data,
		"<%s no=\"no%d\" title=\"x:%d,y:%d\" style=\"width:%dpx;height:%dpx;position:absolute;left:%dpx;top:%dpx\"/>\n",
		vnode->hpNode->html_tag.tag_name,
		vnode->hpNode->html_tag.tag_code,
		vnode->xpos, vnode->ypos, vnode->wx, vnode->hx,
		vnode->xpos - vnode->upperNode->xpos,
		vnode->ypos - vnode->upperNode->ypos);
	}
	return VISIT_NORMAL;
}

static int finish_visit_for_printinfo(html_vnode_t* vnode, void* result)
{
	if (vnode->isValid == false)
		return VISIT_NORMAL;

	print_visit_t* data = (print_visit_t*) result;
	for (int i = 0; i < data->node_depth; i++)
		assert_printf(data, " ");
	data->node_depth--;

	if (vnode->hpNode->html_tag.tag_type == TAG_PURETEXT)
	{
		assert_printf(data, "\n");
	}
	else if (!vnode->hpNode->html_tag.is_self_closed)
	{
		assert_printf(data, "</%s no=\"no%d\">\n", vnode->hpNode->html_tag.tag_name, vnode->hpNode->html_tag.tag_code);
	}
	return VISIT_NORMAL;
}

int vhtml_print_info(html_vnode_t* vnode, char* buf, int size, int& avail)
{
	print_visit_t visit;
	memset(&visit, 0, sizeof(print_visit_t));
	visit.buf = buf;
	visit.size = size;
	visit.avail = avail;
	visit.depth = 3;

	assert_printf((&visit),
	"<html>\n <head>\n  <style type=\"text/css\">\nli {list-style-type:none;}\nbody,strong,h1,h2,h3,h4,h5,h6 {font-size:12px;}\n  </style>\n </head>\n");
	html_vnode_visit(vnode, start_visit_for_printinfo, finish_visit_for_printinfo, &visit);
	assert_printf((&visit), "</html>\n");

	buf[visit.avail] = 0;
	avail = visit.avail;
	return visit.avail;
}


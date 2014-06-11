#include "util/htmlparser/htmlparser/html_attr.h"
#include "util/htmlparser/vhtmlparser/vhtml_tree.h"
#include "util/htmlparser/vhtmlparser/vhtml_utils.h"
#include "util/htmlparser/htmlparser/html_extractor.h"
#include "util/htmlparser/utils/url.h"
#include "util/htmlparser/utils/debug.h"

int print_vnode_cssinfo(html_vnode_t *vnode, char *p, int bufLen, int space_len, int type)
{
	html_tag_type_t tag_type = vnode->hpNode->html_tag.tag_type;
	if (tag_type == TAG_BODY || tag_type == TAG_HTML)
	{
		return 0;
	}
	bool has_background = false;
	bool ignore_border = false;
	if (tag_type == TAG_INPUT || tag_type == TAG_TEXTAREA || tag_type == TAG_SELECT || tag_type == TAG_IMG)
	{
		ignore_border = true;
	}
	else if (tag_type == TAG_IFRAME)
		has_background = true;
	else
	{
		for (css_prop_node_t* prop = vnode->css_prop; prop; prop = prop->next)
		{
			if (prop->type == CSS_PROP_BACKGROUND)
				has_background = true;
			else if (prop->type == CSS_PROP_BACKGROUND_COLOR)
				has_background = true;
		}
		if (get_attribute_value(&vnode->hpNode->html_tag, ATTR_BGCOLOR))
			has_background = true;
		else if (get_attribute_value(&vnode->hpNode->html_tag, ATTR_BACKGROUND))
			has_background = true;
	}
	if (IS_FLOAT_RIGHT(vnode->property))
		has_background = true;

	char *pos = "fixed";
	int left = vnode->xpos;
	int top = vnode->ypos;
	if (vnode->upperNode)
	{
		left -= vnode->upperNode->xpos;
		top -= vnode->upperNode->ypos;
		pos = "absolute";
		if (vnode->upperNode->hpNode->html_tag.tag_type == TAG_BODY)
			left += 40;
	}
	char *pp = p;
	pp += sprintf(pp, 
			"list-style:none;margin:0px;"
			"position:%s;left:%dpx;top:%dpx;width:%dpx;height:%dpx;font-size:%dpx;"
			"border-top:%dpx solid #ff0000;"
			"border-bottom:%dpx solid #ff0000;"
			"border-right:%dpx solid #ff0000;"
			"border-left:%dpx solid #ff0000;"
			"%s;"
			,pos ,left, top, vnode->wx, vnode->hx, vnode->font.size,
			ignore_border ? 0 : vnode->border.top, ignore_border ? 0 : vnode->border.bottom, ignore_border ? 0 : vnode->border.right, ignore_border ? 0 : vnode->border.left,
			has_background ? "border:1px solid #00ff00" : ""
		     );
	return pp - p;
}

int print_vnode_html(html_vnode_t *html_vnode, char* buf, int size, int& avail, const char* base)
{
	if (html_vnode->firstChild == NULL && html_vnode->textSize <= 0 && html_vnode->depth > 2)
		return 0;
	if (size <= 0 || buf == NULL || html_vnode == NULL || !html_vnode->isValid || html_vnode->hx == 0 || html_vnode->wx == 0)
		return 0;

	char nodeinfo[10240] = "";
	int nodeinfo_len = print_vnode_cssinfo(html_vnode, nodeinfo, 10240, 0, 0);

	html_attribute_t *attribute;
	html_vnode_t *child;
	html_node_t *html_node = html_vnode->hpNode;

	int level = html_vnode->depth;
	if (avail + level >= size)
		return avail;
	for (int i = 0; i < level; i++)
		avail += sprintf(buf + avail, " ");

	html_tag_type_t type = html_node->html_tag.tag_type;
	if (type == TAG_ROOT || type == TAG_LINK || type == TAG_SCRIPT || type == TAG_STYLE)
	{
	}
	else if (html_node->html_tag.tag_type == TAG_DOCTYPE)
	{
		if (avail + 12 + nodeinfo_len + html_node->html_tag.textlength >= size)
			return avail;
		avail += sprintf(buf + avail, "<!DOCTYPE %s>\n", html_node->html_tag.text);
	}
	else if (html_node->html_tag.tag_type == TAG_COMMENT)
	{
		if (avail + 12 + nodeinfo_len + html_node->html_tag.textlength >= size)
			return avail;
		avail += sprintf(buf + avail, "<!-- %s -->\n", html_node->html_tag.text);
	}
	else if (html_node->html_tag.tag_type == TAG_PURETEXT)
	{
		if (avail + 12 + nodeinfo_len + html_node->html_tag.textlength >= size)
			return avail;
		avail += sprintf(buf + avail, "<span id=%d style=\"%s\">%s</span>\n", html_vnode->id, nodeinfo, html_node->html_tag.text);
		//avail += sprintf(buf + avail, "%s", html_node->html_tag.text);
	}
	else
	{
		if (html_node->html_tag.tag_name)
		{
			avail += sprintf(buf + avail, "<%s id=%d style=\"%s\"", html_node->html_tag.tag_name, html_vnode->id, nodeinfo);
		}
		for (attribute = html_node->html_tag.attribute; attribute != NULL; attribute = attribute->next)
		{
			if (avail + 4 + strlen(attribute->name) + attribute->valuelength >= size)
				return avail;
			if (attribute->value != NULL)
			{
				if(attribute->type == ATTR_HREF || attribute->type == ATTR_SRC)
				{
					char domain[MAX_SITE_LEN] = { '\0' };
					char port[MAX_PORT_LEN] = { '\0' };
					char path[MAX_PATH_LEN] = { '\0' };
					int ret = easou_parse_url(base, domain, port, path);
					assert(ret == 1);
					char href_buf[2048];
					if(html_combine_url(href_buf, attribute->value, domain, path, port) > 0)
					{
						/*
						   if (html_vnode->hpNode->html_tag.tag_type == TAG_IMG)
						   avail += sprintf(buf + avail, " src=\"http://%s\"", href_buf);
						   else
						 */
						avail += sprintf(buf + avail, " href=\"http://%s\"", href_buf);
					}
				}
				else
					avail += sprintf(buf + avail, " %s=\"%s\"", attribute->name, attribute->value);
			}
		}
		if (avail + 2 + nodeinfo_len >= size)
			return avail;
		avail += sprintf(buf + avail, ">\n");
	}

	for (child = html_vnode->firstChild; child != NULL; child = child->nextNode)
	{
		print_vnode_html(child, buf, size, avail, base);
		if (avail >= size)
			return avail;
	}

	if (type == TAG_ROOT || type == TAG_LINK || type == TAG_COMMENT || type == TAG_DOCTYPE || type == TAG_PURETEXT || type == TAG_SCRIPT || type == TAG_STYLE)
	{
	}
	else if (html_node->html_tag.is_self_closed == false)
	{
		if (avail + level >= size)
			return avail;
		for (int i = 0; i < level; i++)
		{
			avail += sprintf(buf + avail, " ");
		}
		if (avail + 8 + nodeinfo_len + strlen(html_node->html_tag.tag_name) >= size)
			return avail;
		avail += sprintf(buf + avail, "</%s>\n", html_node->html_tag.tag_name);
	}
}

int print_vtree_html(html_vtree_t *html_vtree, char* buf, int size, const char* base)
{
	if (size <= 0 || buf == NULL)
		return 0;
	buf[0] = 0;
	if (html_vtree == NULL)
		return 0;
	int avail = 0;
	print_vnode_html(html_vtree->root, buf, size, avail, base);
	return avail;
}

int print_vnode_info(html_vnode_t *vnode, char *p, int bufLen, int space_len, int type)
{
	char *pp = p;
	int len = bufLen;
	for (int i = 0; i < space_len; i++)
	{
		pp += snprintf(pp, len, " ");
		len--;
	}
        char *desp = "";
        if (type == 1)
        {
                desp = "";
        }
        else if (type == 2)
        {
                desp = "父";
        }
        else if (type == 3)
        {
                desp = "子";
        }
	bool inter_prop = false;
	if (IS_INCLUDE_INTER(vnode->property))
	{
		inter_prop = true;
	}
	bool border_prop = false;
	if (IS_BORDER(vnode->property))
	{
		border_prop = true;
	}
	bool txt_prop = false;
	if (IS_TEXT_VNODE(vnode->property))
	{
		txt_prop = true;
	}
	char prop_buf[1024] = "";
	char *p_prop = prop_buf;
	int parentid=-1;
	if(vnode->upperNode){
		parentid=vnode->upperNode->id;
	}
	for (css_prop_node_t *prop = vnode->css_prop; prop; prop = prop->next)
	{
		p_prop += sprintf(p_prop, "%s=%s;", css_property_name_array[prop->type], prop->value);
	}
	pp += snprintf(pp, len, "[%svnode id=%d parent=%d level=%d pid=%d %s 有效=%d 横坐标=%d 纵坐标=%d 可信=%d 宽=%d 高=%d 最小宽=%d 最大宽=%d "
			"上边框=%d 下边框=%d 左边框=%d 右边框=%d "
			"上边距=%d 下边距=%d 左边距=%d 右边距=%d "
			"字体大小=%d 子树最大字体=%d 子树字体数=%d 文本长度=%d 子树文本长度=%d 汉字数=%d 子树汉字数=%d 子树border数=%d 叶子节点数=%d 自重复=%d 兄弟重复=%d 自重复度=%d 自重复标签类型=%d 兄弟重复数=%d 交互属性=%d 边框属性=%d 文本属性=%d "
			"后换行=%d 前换行=%d "
			"CSS属性=%s]\n",
			desp, vnode->id, vnode->upperNode ? vnode->upperNode->id : -1, vnode->depth,parentid, vnode->hpNode->html_tag.tag_name == NULL ? "txt" : vnode->hpNode->html_tag.tag_name, vnode->isValid, vnode->xpos, vnode->ypos, vnode->trust, vnode->wx, vnode->hx, vnode->min_wx, vnode->max_wx,
			vnode->border.top, vnode->border.bottom, vnode->border.left, vnode->border.right,
			vnode->border.pad_top, vnode->border.pad_bottom, vnode->border.pad_left, vnode->border.pad_right, 
			vnode->font.size, vnode->subtree_max_font_size, vnode->subtree_diff_font, vnode->textSize, vnode->subtree_textSize, vnode->cn_num, vnode->subtree_cn_num, vnode->subtree_border_num, vnode->struct_info->valid_leaf_num, vnode->struct_info->is_self_repeat, vnode->struct_info->is_repeat_with_sibling, vnode->struct_info->self_similar_value, vnode->struct_info->repeat_type_num, vnode->struct_info->repeat_num, inter_prop, border_prop, txt_prop, 
			IS_LINE_BREAK(vnode->property), IS_BREAK_BEFORE(vnode->property), 
			prop_buf);
	return pp - p;
}

int print_vnode(html_vnode_t *html_vnode, char* buf, int size, int& avail)
{
	if (size <= 0 || buf == NULL || html_vnode == NULL || !html_vnode->isValid)
		return 0;

	char nodeinfo[10240] = "";
	int nodeinfo_len = print_vnode_info(html_vnode, nodeinfo, 10240, 0, 1);

	html_attribute_t *attribute;
	html_vnode_t *child;
	html_node_t *html_node = html_vnode->hpNode;

	int level = html_vnode->depth;
	if (avail + level >= size)
		return avail;
	for (int i = 0; i < level; i++)
		avail += sprintf(buf + avail, " ");

	if (html_node->html_tag.tag_type == TAG_DOCTYPE)
	{
		if (avail + 12 + nodeinfo_len + html_node->html_tag.textlength >= size)
			return avail;
		avail += sprintf(buf + avail, "<!DOCTYPE %s> %s", html_node->html_tag.text, nodeinfo);
	}
	else if (html_node->html_tag.tag_type == TAG_COMMENT)
	{
		if (avail + 12 + nodeinfo_len + html_node->html_tag.textlength >= size)
			return avail;
		avail += sprintf(buf + avail, "<!-- %s --> %s", html_node->html_tag.text, nodeinfo);
	}
	else if (html_node->html_tag.tag_type == TAG_PURETEXT)
	{
		if (avail + 12 + nodeinfo_len + html_node->html_tag.textlength >= size)
			return avail;
		avail += sprintf(buf + avail, "%s%s", html_node->html_tag.text, nodeinfo);
	}
	else
	{
		if (html_node->html_tag.tag_name)
		{
			avail += sprintf(buf + avail, "<%s", html_node->html_tag.tag_name);
		}
		else
		{
			if (html_node->html_tag.tag_type == TAG_ROOT)
			{
				avail += sprintf(buf + avail, "<ROOT");
			}
			else
			{
				avail += sprintf(buf + avail, "<%d", html_node->html_tag.tag_type);
			}
		}
		for (attribute = html_node->html_tag.attribute; attribute != NULL; attribute = attribute->next)
		{
			if (avail + 4 + strlen(attribute->name) + attribute->valuelength >= size)
				return avail;
			avail += sprintf(buf + avail, " %s", attribute->name);
			if (attribute->value != NULL)
			{
				avail += sprintf(buf + avail, "=\"%s\"", attribute->value);
			}
		}
		char prop_buf[1024] = "";
		char *p_prop = prop_buf;
		for (css_prop_node_t *prop = html_vnode->css_prop; prop; prop = prop->next)
		{
			if (avail + 2 + strlen(css_property_name_array[prop->type]) + strlen(prop->value) >= size)
				return avail;
			p_prop += sprintf(p_prop, "%s=%s;", css_property_name_array[prop->type], prop->value);
		}

		if (avail + 2 + nodeinfo_len >= size)
			return avail;
		avail += sprintf(buf + avail, "> %s", nodeinfo);
	}

	for (child = html_vnode->firstChild; child != NULL; child = child->nextNode)
	{
		print_vnode(child, buf, size, avail);
		if (avail >= size)
			return avail;
	}

	if (html_node->html_tag.tag_type == TAG_STYLE && html_node->html_tag.text)
	{
		if (avail + html_node->html_tag.textlength >= size)
			return avail;
		avail += sprintf(buf + avail, "%s", html_node->html_tag.text);
	}
	if (html_node->html_tag.tag_type != TAG_COMMENT && html_node->html_tag.tag_type != TAG_DOCTYPE && html_node->html_tag.tag_type != TAG_PURETEXT && !html_node->html_tag.is_self_closed)
	{
		if (avail + level >= size)
			return avail;
		for (int i = 0; i < level; i++)
		{
			avail += sprintf(buf + avail, " ");
		}
		if (html_node->html_tag.tag_type == TAG_ROOT)
		{
			if (avail + 8 + nodeinfo_len >= size)
				return avail;
			avail += sprintf(buf + avail, "</ROOT> %s", nodeinfo);
		}
		else
		{
			if (avail + 8 + nodeinfo_len + strlen(html_node->html_tag.tag_name) >= size)
				return avail;
			avail += sprintf(buf + avail, "</%s> %s", html_node->html_tag.tag_name, nodeinfo);
		}
	}
}

int print_vtree(html_vtree_t *html_vtree, char* buf, int size)
{
	if (size <= 0 || buf == NULL)
		return 0;
	buf[0] = 0;
	if (html_vtree == NULL)
		return 0;
	int avail = 0;
	print_vnode(html_vtree->root, buf, size, avail);
	return avail;
}

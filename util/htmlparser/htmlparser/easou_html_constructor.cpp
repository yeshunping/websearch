#include <assert.h>
#include <stdio.h>
#include "easou_html_dtd.h"
#include "easou_html_tree.h"
#include "easou_html_dom.h"
#include "easou_html_parser.h"
#include "easou_html_operate.h"
#include "easou_html_constructor.h"

int on_xml_after(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_DOCTYPE:
			ret = do_error(parser, next);
			return ret;
		case TAG_WHITESPACE:
			ret = do_ignore(parser, next);
			return ret;
		case TAG_PURETEXT:
			ret = do_error(parser, next);
			return ret;
		default:
			ret = do_error(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		default:
			ret = do_error(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_xml(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_DOCTYPE:
			ret = do_error(parser, next);
			return ret;
		case TAG_WHITESPACE:
			ret = do_ignore(parser, next);
			return ret;
		case TAG_PURETEXT:
			ret = do_append_no_stack(parser, next);
			return ret;
		default:
			ret = do_append(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		default:
			ret = do_is_match(parser, next);
			if (ret == 0)
			{
				ret = do_popup(parser, next);
			}
			else
			{
				ret = do_error(parser, next);
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_is_root(parser, next);
			if (ret == 0)
			{
				ret = do_switch(parser, next, on_xml_after);
			}
			else
			{
				ret = 0;
			}
			return ret;
		}
	}
	return ret;
}

int on_xml_root(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_DOCTYPE:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_WHITESPACE:
			ret = do_ignore(parser, next);
			return ret;
		case TAG_PURETEXT:
			ret = do_error(parser, next);
			return ret;
		default:
			ret = do_switch(parser, next, on_xml);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		default:
			ret = do_error(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_after_after_frameset(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_DOCTYPE:
			ret = do_forward(parser, next, on_in_body);
			return ret;
		case TAG_WHITESPACE:
			ret = do_forward(parser, next, on_in_body);
			return ret;
		case TAG_HTML:
			ret = do_forward(parser, next, on_in_body);
			return ret;
		case TAG_NOFRAME:
			ret = do_forward(parser, next, on_in_head);
			return ret;
		default:
			ret = do_warning(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		default:
			ret = do_warning(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_after_after_body(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_DOCTYPE:
			ret = do_forward(parser, next, on_in_body);
			return ret;
		case TAG_WHITESPACE:
			ret = do_forward(parser, next, on_in_body);
			return ret;
		case TAG_HTML:
			ret = do_forward(parser, next, on_in_body);
			return ret;
		case TAG_NOFRAME:
			ret = do_forward(parser, next, on_in_head);
			return ret;
		default:
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_body);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		default:
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_body);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_after_frameset(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_WHITESPACE:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_DOCTYPE:
			ret = do_warning(parser, next);
			return ret;
		case TAG_HTML:
			ret = do_forward(parser, next, on_in_body);
			return ret;
		case TAG_NOFRAME:
			ret = do_forward(parser, next, on_in_head);
			return ret;
		default:
			ret = do_warning(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		case TAG_HTML:
			ret = do_switch(parser, next, on_after_after_frameset);
			return ret;
		default:
			ret = do_warning(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_in_frameset(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_WHITESPACE:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_DOCTYPE:
			ret = do_warning(parser, next);
			return ret;
		case TAG_HTML:
			ret = do_forward(parser, next, on_in_body);
			return ret;
		case TAG_FRAMESET:
			ret = do_append(parser, next);
			return ret;
		case TAG_FRAME:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_NOFRAME:
			ret = do_forward(parser, next, on_in_head);
			return ret;
		default:
			ret = do_warning(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		case TAG_FRAMESET:
			ret = do_is(parser, next, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_warning(parser, next);
			}
			else
			{
				ret = do_popup(parser, next);
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_is(parser, next, TAG_FRAMESET);
			if (ret != 0)
			{
				ret = do_switch(parser, next, on_after_frameset);
			}
			return ret;
		default:
			ret = do_warning(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_after_body(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_WHITESPACE:
			ret = do_forward(parser, next, on_in_body);
			return ret;
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_DOCTYPE:
			ret = do_warning(parser, next);
			return ret;
		case TAG_HTML:
			ret = do_forward(parser, next, on_in_body);
			return ret;
		default:
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_body);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		case TAG_HTML:
			ret = do_switch(parser, next, on_after_after_body);
			return ret;
		default:
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_body);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_in_select_in_table(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_CAPTION:
		case TAG_TABLE:
		case TAG_TBODY:
		case TAG_TFOOT:
		case TAG_THEAD:
		case TAG_TR:
		case TAG_TD:
		case TAG_TH:

			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_as_if_end(parser, next, TAG_SELECT);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		default:
			ret = do_forward(parser, next, on_in_select);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		case TAG_CAPTION:
		case TAG_TABLE:
		case TAG_TBODY:
		case TAG_TFOOT:
		case TAG_THEAD:
		case TAG_TR:
		case TAG_TD:
		case TAG_TH:
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_has_this_tag_in_table_scope(parser, next);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_SELECT);
				if (ret == 0)
				{
					ret = do_redo(parser, next);
				}
			}
			else
			{
				ret = do_ignore(parser, next);
			}
			return ret;
		default:
			ret = do_forward(parser, next, on_in_select);
			return ret;
		}
	}
	return ret;
}

int on_in_select(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_WHITESPACE:

			ret = do_append_no_stack(parser, next);

			return ret;

		case TAG_PURETEXT:

			ret = do_append_no_stack(parser, next);

			return ret;

		case TAG_COMMENT:

			ret = do_append_no_stack(parser, next);

			return ret;

		case TAG_DOCTYPE:

			ret = do_warning(parser, next);

			return ret;

		case TAG_HTML:

			ret = do_forward(parser, next, on_in_body);

			return ret;

		case TAG_OPTION:

			ret = do_is(parser, next, TAG_OPTION);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_OPTION);
			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);

			return ret;

		case TAG_OPTGROUP:

			ret = do_is(parser, next, TAG_OPTION);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_OPTION);
			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_is(parser, next, TAG_OPTGROUP);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_OPTGROUP);
			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);

			return ret;

		case TAG_SELECT:

			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_as_if_end(parser, next, TAG_SELECT);

			return ret;

		case TAG_INPUT:

			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_has_these_tags_in_table_scope(parser, next, TAG_SELECT, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_SELECT);
				if (ret == 0)
				{
					ret = do_redo(parser, next);
				}

			}
			else
			{
				ret = do_ignore(parser, next);

			}

			return ret;

		case TAG_SCRIPT:

			ret = do_forward(parser, next, on_in_head);

			return ret;

		default:

			ret = do_warning(parser, next);

			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		case TAG_OPTGROUP:
			ret = do_proc_select_optgroup_end(parser, next);
			return ret;
		case TAG_OPTION:

			ret = do_is(parser, next, TAG_OPTION);
			if (ret == 0)
			{
				ret = do_popup(parser, next);
			}
			else
			{
				ret = do_warning(parser, next);
			}

			return ret;

		case TAG_SELECT:

			ret = do_has_this_tag_in_table_scope(parser, next);
			if (ret == 0)
			{
				ret = do_close_tag(parser, next);
			}
			else
			{
				ret = do_warning(parser, next);
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_reset(parser, next);

			return ret;

		default:

			ret = do_warning(parser, next);

			return ret;

		}
	}
	return ret;
}

int on_in_cell(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_CAPTION:
		case TAG_COL:
		case TAG_COLGROUP:
		case TAG_TBODY:
		case TAG_TD:
		case TAG_TFOOT:
		case TAG_TH:
		case TAG_THEAD:
		case TAG_TR:

			ret = do_has_these_tags_in_table_scope(parser, next, TAG_TH, TAG_TD, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_close_cell(parser, next);
				if (ret == 0)
				{
					ret = do_redo(parser, next);
				}

			}
			else
			{
				ret = do_warning(parser, next);

			}

			return ret;

		default:

			ret = do_forward(parser, next, on_in_body);

			return ret;

		}
	}
	else
	{
		switch (tag)
		{
		case TAG_TD:
		case TAG_TH:

			ret = do_has_this_tag_in_table_scope(parser, next);
			if (ret == 0)
			{
				ret = do_close_tag(parser, next);
				if (ret == 0)
				{
					ret = do_clear_actfmt_list(parser, next);
					if (ret == 0)
					{
						ret = do_assert_in_tag(parser, next, TAG_TR);
						if (ret == 0)
						{
							ret = do_switch(parser, next, on_in_row);
						}
					}
				}

			}
			else
			{
				ret = do_warning(parser, next);

			}

			return ret;

		case TAG_BODY:
		case TAG_CAPTION:
		case TAG_COL:
		case TAG_COLGROUP:
		case TAG_HTML:

			ret = do_warning(parser, next);

			return ret;

		case TAG_TABLE:
		case TAG_TBODY:
		case TAG_TFOOT:
		case TAG_THEAD:
		case TAG_TR:

			ret = do_has_this_tag_in_table_scope(parser, next);
			if (ret == 0)
			{
				ret = do_close_cell(parser, next);
				if (ret == 0)
				{
					ret = do_redo(parser, next);
				}

			}
			else
			{
				ret = do_warning(parser, next);

			}

			return ret;

		default:

			ret = do_forward(parser, next, on_in_body);

			return ret;

		}
	}
	return ret;
}

int on_in_row(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_TH:
		case TAG_TD:

			ret = do_clear_to_table_row_context(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_cell);

			return ret;

		case TAG_CAPTION:
		case TAG_COL:
		case TAG_COLGROUP:
		case TAG_TBODY:
		case TAG_TFOOT:
		case TAG_THEAD:
		case TAG_TR:

			ret = do_as_if_end(parser, next, TAG_TR);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);

			return ret;

		default:

			ret = do_forward(parser, next, on_in_table);

			return ret;

		}
	}
	else
	{
		switch (tag)
		{
		case TAG_TR:

			ret = do_has_this_tag_in_table_scope(parser, next);
			if (ret == 0)
			{
				ret = do_clear_to_table_row_context(parser, next);
				if (ret == 0)
				{
					ret = do_is(parser, next, TAG_TR);
					if (ret == 0)
					{
						ret = do_popup(parser, next);
						if (ret == 0)
						{
							ret = do_switch(parser, next, on_in_table_body);
						}
					}
				}

			}
			else
			{
				ret = do_warning(parser, next);

			}

			return ret;

		case TAG_TABLE:

			ret = do_as_if_end(parser, next, TAG_TR);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);

			return ret;

		case TAG_TBODY:
		case TAG_TFOOT:
		case TAG_THEAD:

			ret = do_has_this_tag_in_table_scope(parser, next);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_TR);
				if (ret == 0)
				{
					ret = do_redo(parser, next);
				}

			}
			else
			{
				ret = do_warning(parser, next);

			}

			return ret;

		case TAG_BODY:
		case TAG_CAPTION:
		case TAG_COL:
		case TAG_COLGROUP:
		case TAG_HTML:
		case TAG_TD:
		case TAG_TH:

			ret = do_warning(parser, next);

			return ret;

		default:

			ret = do_forward(parser, next, on_in_table);

			return ret;

		}
	}
	return ret;
}

int on_in_table_body(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_TR:

			ret = do_clear_to_table_body_context(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_row);

			return ret;

		case TAG_TH:
		case TAG_TD:

			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_as_if(parser, next, TAG_TR);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);

			return ret;

		case TAG_CAPTION:
		case TAG_COL:
		case TAG_COLGROUP:
		case TAG_TBODY:
		case TAG_TFOOT:
		case TAG_THEAD:

			ret = do_has_these_tags_in_table_scope(parser, next, TAG_TBODY, TAG_TFOOT, TAG_THEAD, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_clear_to_table_body_context(parser, next);

			}
			else
			{
				ret = do_warning(parser, next);

			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_as_if_end_this(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);

			return ret;

		default:

			ret = do_forward(parser, next, on_in_table);

			return ret;

		}
	}
	else
	{
		switch (tag)
		{
		case TAG_TBODY:
		case TAG_TFOOT:
		case TAG_THEAD:

			ret = do_has_this_tag_in_table_scope(parser, next);
			if (ret == 0)
			{
				ret = do_clear_to_table_body_context(parser, next);
				if (ret == 0)
				{
					ret = do_popup(parser, next);
					if (ret == 0)
					{
						ret = do_switch(parser, next, on_in_table);
					}
				}

			}
			else
			{
				ret = do_warning(parser, next);

			}

			return ret;

		case TAG_TABLE:

			ret = do_has_these_tags_in_table_scope(parser, next, TAG_TBODY, TAG_TFOOT, TAG_THEAD, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_clear_to_table_body_context(parser, next);
			}
			else
			{
				ret = do_warning(parser, next);
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_as_if_end_this(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);

			return ret;

		case TAG_BODY:
		case TAG_CAPTION:
		case TAG_COL:
		case TAG_COLGROUP:
		case TAG_HTML:
		case TAG_TD:
		case TAG_TH:
		case TAG_TR:

			ret = do_warning(parser, next);

			return ret;

		default:

			ret = do_forward(parser, next, on_in_table);

			return ret;

		}
	}
	return ret;
}

int on_in_column_group(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_WHITESPACE:

			ret = do_append_no_stack(parser, next);

			return ret;

		case TAG_COMMENT:

			ret = do_append_no_stack(parser, next);

			return ret;

		case TAG_DOCTYPE:

			ret = do_warning(parser, next);

			return ret;

		case TAG_HTML:

			ret = do_forward(parser, next, on_in_body);

			return ret;

		case TAG_COL:

			ret = do_append_no_stack(parser, next);

			return ret;

		default:

			ret = do_as_if_end(parser, next, TAG_COLGROUP);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);

			return ret;

		}
	}
	else
	{
		switch (tag)
		{
		case TAG_COLGROUP:

			ret = do_is(parser, next, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_warning(parser, next);

			}
			else
			{
				ret = do_popup(parser, next);
				if (ret == 0)
				{
					ret = do_switch(parser, next, on_in_table);
				}

			}

			return ret;

		case TAG_COL:

			ret = do_warning(parser, next);

			return ret;

		default:

			ret = do_as_if_end(parser, next, TAG_COLGROUP);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);

			return ret;

		}
	}
	return ret;
}

int on_in_caption(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_CAPTION:
		case TAG_COL:
		case TAG_COLGROUP:
		case TAG_TBODY:
		case TAG_TD:
		case TAG_TFOOT:
		case TAG_TH:
		case TAG_THEAD:
		case TAG_TR:

			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_as_if_end(parser, next, TAG_CAPTION);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);

			return ret;

		default:

			ret = do_forward(parser, next, on_in_body);

			return ret;

		}
	}
	else
	{
		switch (tag)
		{
		case TAG_CAPTION:

			ret = do_has_this_tag_in_table_scope(parser, next);
			if (ret == 0)
			{
				ret = do_close_tag(parser, next);
				if (ret == 0)
				{
					ret = do_clear_actfmt_list(parser, next);
					if (ret == 0)
					{
						ret = do_switch(parser, next, on_in_table);
					}
				}

			}
			else
			{
				ret = do_warning(parser, next);

			}

			return ret;

		case TAG_TABLE:

			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_as_if_end(parser, next, TAG_CAPTION);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);

			return ret;

		case TAG_BODY:
		case TAG_COL:
		case TAG_COLGROUP:
		case TAG_HTML:
		case TAG_TBODY:
		case TAG_TD:
		case TAG_TFOOT:
		case TAG_TH:
		case TAG_THEAD:
		case TAG_TR:

			ret = do_warning(parser, next);

			return ret;

		default:

			ret = do_forward(parser, next, on_in_body);

			return ret;

		}
	}
	return ret;
}

int on_in_table(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_WHITESPACE:

			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_is_in_these_tags(parser, next, TAG_TABLE, TAG_TBODY, TAG_TFOOT, TAG_THEAD, TAG_TR, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_enter_foster(parser, next);
				if (ret == 0)
				{
					ret = do_forward(parser, next, on_in_body);
					if (ret == 0)
					{
						ret = do_exit_foster(parser, next);
					}
				}

			}
			else
			{
				ret = do_ignore(parser, next);

			}

			return ret;

		case TAG_PURETEXT:

			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_is_in_these_tags(parser, next, TAG_TABLE, TAG_TBODY, TAG_TFOOT, TAG_THEAD, TAG_TR, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_enter_foster(parser, next);
				if (ret == 0)
				{
					ret = do_forward(parser, next, on_in_body);
					if (ret == 0)
					{
						ret = do_exit_foster(parser, next);
					}
				}

			}
			else
			{
				ret = do_ignore(parser, next);

			}

			return ret;

		case TAG_COMMENT:

			ret = do_append_no_stack(parser, next);

			return ret;

		case TAG_DOCTYPE:

			ret = do_warning(parser, next);

			return ret;

		case TAG_CAPTION:

			ret = do_clear_to_table_context(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_caption);

			return ret;

		case TAG_COLGROUP:

			ret = do_clear_to_table_context(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_column_group);

			return ret;

		case TAG_COL:

			ret = do_as_if(parser, next, TAG_COLGROUP);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);

			return ret;

		case TAG_TBODY:
		case TAG_TFOOT:
		case TAG_THEAD:

			ret = do_clear_to_table_context(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_table_body);

			return ret;

		case TAG_TD:
		case TAG_TH:
		case TAG_TR:

			ret = do_as_if(parser, next, TAG_TBODY);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);

			return ret;

		case TAG_TABLE:

			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_as_if_end(parser, next, TAG_TABLE);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);

			return ret;

		case TAG_STYLE:
		case TAG_SCRIPT:

			ret = do_forward(parser, next, on_in_head);

			return ret;

		case TAG_INPUT:

			ret = do_proc_table_input(parser, next);

			return ret;

		case TAG_FORM:

			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_has_form(parser, next);
			if (ret == 0)
			{
				ret = do_ignore(parser, next);

			}
			else
			{
				ret = do_append_no_stack(parser, next);

			}

			return ret;

		default:

			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_is_in_these_tags(parser, next, TAG_TABLE, TAG_TBODY, TAG_TFOOT, TAG_THEAD, TAG_TR, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_enter_foster(parser, next);
				if (ret == 0)
				{
					ret = do_forward(parser, next, on_in_body);
					if (ret == 0)
					{
						ret = do_exit_foster(parser, next);
					}
				}

			}
			else
			{
				ret = do_ignore(parser, next);

			}

			return ret;

		}
	}
	else
	{
		switch (tag)
		{
		case TAG_TABLE:

			ret = do_has_this_tag_in_table_scope(parser, next);
			if (ret == 0)
			{
				ret = do_close_tag(parser, next);
				if (ret == 0)
				{
					ret = do_merge_foster(parser, next);
					if (ret == 0)
					{
						ret = do_reset(parser, next);
					}
				}

			}
			else
			{
				ret = do_warning(parser, next);

			}

			return ret;

		case TAG_BODY:
		case TAG_CAPTION:
		case TAG_COL:
		case TAG_COLGROUP:
		case TAG_HTML:
		case TAG_TBODY:
		case TAG_TD:
		case TAG_TFOOT:
		case TAG_TH:
		case TAG_THEAD:
		case TAG_TR:

			ret = do_warning(parser, next);

			return ret;

		default:

			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_is_in_these_tags(parser, next, TAG_TABLE, TAG_TBODY, TAG_TFOOT, TAG_THEAD, TAG_TR, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_enter_foster(parser, next);
				if (ret == 0)
				{
					ret = do_forward(parser, next, on_in_body);
					if (ret == 0)
					{
						ret = do_exit_foster(parser, next);
					}
				}

			}
			else
			{
				ret = do_ignore(parser, next);

			}

			return ret;

		}
	}
	return ret;
}

int on_in_body(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_WHITESPACE:
			//ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_PURETEXT:
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_DOCTYPE:
			ret = do_warning(parser, next);
			return ret;
		case TAG_HTML:
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_merge_attr(parser, next);
			return ret;
		case TAG_BASE:
		case TAG_BASEFONT:
		case TAG_BGSOUND:
		case TAG_LINK:
		case TAG_META:
		case TAG_NOFRAME:
		case TAG_SCRIPT:
		case TAG_STYLE:
		case TAG_TITLE:
			ret = do_forward(parser, next, on_in_head);
			return ret;
		case TAG_BODY:
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_merge_attr(parser, next);
			return ret;
		case TAG_FRAMESET:
			ret = do_warning(parser, next);
			return ret;
		case TAG_ADDRESS:
		case TAG_BLOCKQUOTE:
		case TAG_CENTER:
		case TAG_DIR:
		case TAG_DIV:
			next->owner->treeAttr = next->owner->treeAttr | 1;
		case TAG_DL:
		case TAG_FIELDSET:
		case TAG_MENU:
		case TAG_OL:
		case TAG_P:
			next->owner->treeAttr = next->owner->treeAttr | 2;
		case TAG_UL:
			ret = do_has_p_in_button_scope(parser, next);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_P);
			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			return ret;
		case TAG_H1:
		case TAG_H2:
		case TAG_H3:
		case TAG_H4:
		case TAG_H5:
		case TAG_H6:
			next->owner->treeAttr = next->owner->treeAttr | 4;
			ret = do_has_p_in_button_scope(parser, next);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_P);
			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_is_in_these_tags(parser, next, TAG_H1, TAG_H2, TAG_H3, TAG_H4, TAG_H5, TAG_H6, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_warning(parser, next);
				if (ret == 0)
				{
					ret = do_popup(parser, next);
				}

			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);

			return ret;

		case TAG_PRE:
			ret = do_has_p_in_button_scope(parser, next);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_P);
			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_eat_next_newline(parser, next);
			return ret;

		case TAG_FORM:
			ret = do_proc_body_form(parser, next);
			return ret;

		case TAG_LI:
			ret = do_proc_body_li(parser, next);
			return ret;

		case TAG_DD:
		case TAG_DT:
			ret = do_proc_body_dd_dt(parser, next);
			return ret;

		case TAG_A:
			ret = do_proc_body_a(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_actfmt_list(parser, next);
			return ret;

		case TAG_B:
		case TAG_BIG:
		case TAG_CODE:
		case TAG_EM:
		case TAG_FONT:
		case TAG_I:
		case TAG_S:
		case TAG_SMALL:
		case TAG_STRIKE:
		case TAG_STRONG:
		case TAG_TT:
		case TAG_U:

			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_actfmt_list(parser, next);
			return ret;

		case TAG_NOBR:
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_is_in_tag(parser, next, TAG_NOBR);
			if (ret == 0)
			{
				ret = do_warning(parser, next);
				if (ret == 0)
				{
					ret = do_as_if_end(parser, next, TAG_NOBR);
				}

			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_actfmt_list(parser, next);
			return ret;

		case TAG_BUTTON:
			ret = do_has_these_tags_in_scope(parser, next, TAG_BUTTON, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_warning(parser, next);
				if (ret == 0)
				{
					ret = do_as_if_this_end(parser, next);
					if (ret == 0)
					{
						ret = do_redo(parser, next);
					}
				}
			}
			else
			{
				ret = do_reconstruct_actfmt_list(parser, next);
				if (ret == 0)
				{
					ret = do_append(parser, next);
					if (ret == 0)
					{
						ret = do_append_actfmt_list_marker(parser, next);
					}
				}
			}
			return ret;

		case TAG_APPLET:
		case TAG_MARQUEE:
		case TAG_OBJECT:
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_actfmt_list_marker(parser, next);
			return ret;

		case TAG_TABLE:
			ret = do_has_p_in_button_scope(parser, next);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_P);
			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_table);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_prepare_foster(parser, next);
			return ret;

		case TAG_AREA:
		case TAG_BR:
		case TAG_EMBED:
		case TAG_IMG:
		case TAG_INPUT:
		case TAG_WBR:
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_no_stack(parser, next);
			return ret;

		case TAG_PARAM:
			ret = do_append_no_stack(parser, next);
			return ret;

		case TAG_HR:
			ret = do_has_p_in_button_scope(parser, next);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_P);
			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_no_stack(parser, next);
			return ret;

		case TAG_TEXTAREA:

			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_eat_next_newline(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_parse_rcdata(parser, next);
			return ret;

		case TAG_IFRAME:
		case TAG_NOSCRIPT:
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_parse_rcdata(parser, next);
			return ret;

		case TAG_SELECT:
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_save(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_proc_body_select(parser, next);
			return ret;

		case TAG_OPTGROUP:
		case TAG_OPTION:
			ret = do_is_in_tag(parser, next, TAG_OPTION);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_OPTION);
			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);

			return ret;

		case TAG_CAPTION:
		case TAG_COL:
		case TAG_COLGROUP:
		case TAG_FRAME:
		case TAG_HEAD:
		case TAG_TBODY:
		case TAG_TD:
		case TAG_TFOOT:
		case TAG_TH:
		case TAG_THEAD:
		case TAG_TR:
		case TAG_WAP_CARD: //todo
			ret = do_warning(parser, next);
			return ret;

		default:
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		case TAG_BODY:
			cal_node_length(parser, next);
			ret = do_has_these_tags_in_scope(parser, next, TAG_BODY, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_switch(parser, next, on_after_body);
			}
			else
			{
				ret = do_warning(parser, next);
			}
			return ret;

		case TAG_HTML:
			//todo
//			ret = do_as_if_end(parser, next, TAG_BODY);
//			if (ret != 0) {
//				return ret;
//			}
//			ret = do_redo(parser, next);
			cal_node_length(parser, next);
			ret = do_warning(parser, next);
			return ret;

		case TAG_ADDRESS:
		case TAG_BLOCKQUOTE:
		case TAG_BUTTON:
		case TAG_CENTER:
		case TAG_DIR:
		case TAG_DIV:
		case TAG_DL:
		case TAG_FIELDSET:
		case TAG_MENU:
		case TAG_OL:
		case TAG_PRE:
		case TAG_UL:
			cal_node_length(parser, next);
			ret = do_has_this_tag_in_scope(parser, next);
			if (ret == 0)
			{
				ret = do_close_tag(parser, next);
			}
			else
			{
				ret = do_warning(parser, next);
			}

			return ret;

		case TAG_FORM:
			cal_node_length(parser, next);
			ret = do_proc_body_form_end(parser, next);

			return ret;

		case TAG_P:
			cal_node_length(parser, next);
			ret = do_has_p_in_button_scope(parser, next);
			if (ret == 0)
			{
				ret = do_close_tag(parser, next);
			}
			else
			{
				ret = do_warning(parser, next);
			}

			return ret;

		case TAG_LI:
			cal_node_length(parser, next);
			ret = do_has_this_tag_in_list_item_scope(parser, next);
			if (ret == 0)
			{
				ret = do_close_tag(parser, next);
			}
			else
			{
				ret = do_warning(parser, next);
			}

			return ret;

		case TAG_DD:
		case TAG_DT:
			cal_node_length(parser, next);
			ret = do_has_this_tag_in_scope(parser, next);
			if (ret == 0)
			{
				ret = do_close_tag(parser, next);
			}
			else
			{
				ret = do_warning(parser, next);
			}

			return ret;

		case TAG_H1:
		case TAG_H2:
		case TAG_H3:
		case TAG_H4:
		case TAG_H5:
		case TAG_H6:
			cal_node_length(parser, next);
			ret = do_has_these_tags_in_scope(parser, next, TAG_H1, TAG_H2, TAG_H3, TAG_H4, TAG_H5, TAG_H6, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_close_these_tags(parser, next, TAG_H1, TAG_H2, TAG_H3, TAG_H4, TAG_H5, TAG_H6, TAG_ROOT);
			}
			else
			{
				ret = do_warning(parser, next);
			}

			return ret;

		case TAG_A:
		case TAG_B:
		case TAG_BIG:
		case TAG_CODE:
		case TAG_EM:
		case TAG_FONT:
		case TAG_I:
		case TAG_NOBR:
		case TAG_S:
		case TAG_SMALL:
		case TAG_STRIKE:
		case TAG_STRONG:
		case TAG_TT:
		case TAG_U:
			cal_node_length(parser, next);
			ret = do_proc_body_formatting_elements_end(parser, next);

			return ret;

		case TAG_APPLET:
		case TAG_MARQUEE:
		case TAG_OBJECT:
			cal_node_length(parser, next);
			ret = do_has_this_tag_in_scope(parser, next);
			if (ret == 0)
			{
				ret = do_close_tag(parser, next);
				if (ret == 0)
				{
					ret = do_clear_actfmt_list(parser, next);
				}

			}
			else
			{
				ret = do_warning(parser, next);
			}
			return ret;
		case TAG_BR:
			cal_node_length(parser, next);
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_as_if(parser, next, TAG_BR);
			return ret;
		case TAG_CAPTION:
		case TAG_COL:
		case TAG_COLGROUP:
		case TAG_FRAME:
		case TAG_HEAD:
		case TAG_TBODY:
		case TAG_TD:
		case TAG_TFOOT:
		case TAG_TH:
		case TAG_THEAD:
		case TAG_TR:
			cal_node_length(parser, next);
			ret = do_warning(parser, next);
			return ret;
		case TAG_TABLE:
			cal_node_length(parser, next);
			ret = do_warning(parser, next);
			return ret;
		default:
			cal_node_length(parser, next);
			ret = do_proc_body_end(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_after_head(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_WHITESPACE:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_DOCTYPE:
			ret = do_warning(parser, next);
			return ret;
		case TAG_HTML:
			ret = do_forward(parser, next, on_in_body);
			return ret;
		case TAG_WAP_WML:
			ret = do_forward(parser, next, on_in_card);
			return ret;
		case TAG_BODY:
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_body);
			return ret;
		case TAG_WAP_CARD:
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_card);
			return ret;
		case TAG_FRAMESET:
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_frameset);
			return ret;
		case TAG_BASE:
		case TAG_BASEFONT:
		case TAG_BGSOUND:
		case TAG_LINK:
		case TAG_META:
		case TAG_NOFRAME:
		case TAG_SCRIPT:
		case TAG_STYLE:
		case TAG_TITLE:
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_enter_head(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_forward(parser, next, on_in_head);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_exit_head(parser, next);
			return ret;
		case TAG_HEAD:
			ret = do_warning(parser, next);
			return ret;
		default:
			// 根据gsp.kf.73.ppmt.info/picture/chapter?fid=108&pid=201111122020138695&bkid=1&lx=0&page=2&orgfid=0&rnd=8179 修改 shuangwei 2012-04-10
			if (parser->hp_html && parser->hp_html->html_tag.tag_type == TAG_WAP_WML)
			{
				ret = do_warning(parser, next);
				return ret;
			}
			else
			{
				ret = do_as_if(parser, next, TAG_BODY);
				if (ret != 0)
				{
					return ret;
				}
				ret = do_redo(parser, next);
				return ret;
			}

		}
	}
	else
	{
		switch (tag)
		{
		case TAG_BODY:
		case TAG_HTML:
		case TAG_BR:
			ret = do_as_if(parser, next, TAG_BODY);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		default:
			ret = do_warning(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_in_head(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_BR:
		case TAG_WHITESPACE:
			ret = do_ignore(parser, next);
			return ret;
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_HTML:
			ret = do_forward(parser, next, on_in_body);
			return ret;
		case TAG_BASE:
		case TAG_BASEFONT:
		case TAG_BGSOUND:
		case TAG_LINK:
		case TAG_META:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_TITLE:
		case TAG_NOFRAME:
		case TAG_STYLE:
		case TAG_SCRIPT:
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_parse_rcdata(parser, next);
			return ret;
		case TAG_HEAD:
			ret = do_warning(parser, next);
			return ret;
		default:
			ret = do_as_if_end(parser, next, TAG_HEAD);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		case TAG_HEAD:
			cal_node_length(parser, next);
			ret = do_assert_in_tag(parser, next, TAG_HEAD);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_popup(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_after_head);
			return ret;
		case TAG_BODY:
		case TAG_HTML:
		case TAG_BR:
			ret = do_as_if_end(parser, next, TAG_HEAD);
			if (ret != 0)
			{
				return ret;
			}
			//cal_node_length(parser,next);
			ret = do_redo(parser, next);
			return ret;
		default:
			cal_node_length(parser, next);
			ret = do_warning(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_before_head(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_WHITESPACE:
			ret = do_ignore(parser, next);
			return ret;
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_BR:
		case TAG_DOCTYPE:
			ret = do_warning(parser, next);
			return ret;
		case TAG_HTML:
			ret = do_forward(parser, next, on_in_body);
		case TAG_WAP_WML:
			ret = do_forward(parser, next, on_in_card);
			return ret;
		case TAG_HEAD:
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_record_head(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_head);
			return ret;
		default:
			ret = do_as_if(parser, next, TAG_HEAD);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		case TAG_HEAD:
		case TAG_BODY:
		case TAG_HTML:
		case TAG_BR:
			ret = do_as_if(parser, next, TAG_HEAD);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		default:
			ret = do_warning(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_before_html(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_DOCTYPE:
			ret = do_warning(parser, next);
			return ret;
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_BR:
		case TAG_WHITESPACE:
			ret = do_ignore(parser, next);
			return ret;
		case TAG_HTML:
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_before_head);
			return ret;
		case TAG_WAP_WML:
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_before_head);
			return ret;
		default:
			ret = do_as_if(parser, next, TAG_HTML);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		case TAG_HEAD:
		case TAG_BODY:
		case TAG_HTML:
		case TAG_BR:
			ret = do_as_if(parser, next, TAG_HTML);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		default:
			ret = do_warning(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_initial(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	/**是开始标签*/
	if (!close)
	{
		switch (tag)
		{
		case TAG_DOCTYPE:
			ret = do_append_no_stack(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_before_html);
			return ret;
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_BR:
		case TAG_WHITESPACE:
		case TAG_PURETEXT: //delete file head
			ret = do_ignore(parser, next);
			return ret;
		default:
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_before_html);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		}
	}
	else
	{/**是结束标签*/
		switch (tag)
		{
		default:
			cal_node_length(parser, next);
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_before_html);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_in_card(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_WHITESPACE:
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_PURETEXT:
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_DOCTYPE:
			ret = do_warning(parser, next);
			return ret;
		case TAG_HTML:
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_merge_attr(parser, next);
			return ret;
		case TAG_BASE:
		case TAG_BASEFONT:
		case TAG_BGSOUND:
		case TAG_LINK:
		case TAG_META:
		case TAG_NOFRAME:
		case TAG_SCRIPT:
		case TAG_STYLE:
		case TAG_TITLE:
			ret = do_forward(parser, next, on_in_head);
			return ret;
		case TAG_BODY:
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_merge_attr(parser, next);
			return ret;
		case TAG_FRAMESET:
			ret = do_warning(parser, next);
			return ret;
		case TAG_ADDRESS:
		case TAG_BLOCKQUOTE:
		case TAG_CENTER:
		case TAG_DIR:
		case TAG_DIV:
			next->owner->treeAttr = next->owner->treeAttr | 1;
		case TAG_DL:
		case TAG_FIELDSET:
		case TAG_MENU:
		case TAG_OL:
		case TAG_P:
			next->owner->treeAttr = next->owner->treeAttr | 2;
		case TAG_UL:
			ret = do_has_p_in_button_scope(parser, next);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_P);
			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			return ret;
		case TAG_H1:
		case TAG_H2:
		case TAG_H3:
		case TAG_H4:
		case TAG_H5:
		case TAG_H6:
			next->owner->treeAttr = next->owner->treeAttr | 4;
			ret = do_has_p_in_button_scope(parser, next);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_P);
			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_is_in_these_tags(parser, next, TAG_H1, TAG_H2, TAG_H3, TAG_H4, TAG_H5, TAG_H6, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_warning(parser, next);
				if (ret == 0)
				{
					ret = do_popup(parser, next);
				}

			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);

			return ret;

		case TAG_PRE:
			ret = do_has_p_in_button_scope(parser, next);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_P);
			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_eat_next_newline(parser, next);
			return ret;

		case TAG_FORM:
			ret = do_proc_body_form(parser, next);
			return ret;

		case TAG_LI:
			ret = do_proc_body_li(parser, next);
			return ret;

		case TAG_DD:
		case TAG_DT:
			ret = do_proc_body_dd_dt(parser, next);
			return ret;

		case TAG_A:
			ret = do_proc_body_a(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_actfmt_list(parser, next);
			return ret;

		case TAG_B:
		case TAG_BIG:
		case TAG_CODE:
		case TAG_EM:
		case TAG_FONT:
		case TAG_I:
		case TAG_S:
		case TAG_SMALL:
		case TAG_STRIKE:
		case TAG_STRONG:
		case TAG_TT:
		case TAG_U:

			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_actfmt_list(parser, next);
			return ret;

		case TAG_NOBR:
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_is_in_tag(parser, next, TAG_NOBR);
			if (ret == 0)
			{
				ret = do_warning(parser, next);
				if (ret == 0)
				{
					ret = do_as_if_end(parser, next, TAG_NOBR);
				}

			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_actfmt_list(parser, next);
			return ret;

		case TAG_BUTTON:
			ret = do_has_these_tags_in_scope(parser, next, TAG_BUTTON, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_warning(parser, next);
				if (ret == 0)
				{
					ret = do_as_if_this_end(parser, next);
					if (ret == 0)
					{
						ret = do_redo(parser, next);
					}
				}
			}
			else
			{
				ret = do_reconstruct_actfmt_list(parser, next);
				if (ret == 0)
				{
					ret = do_append(parser, next);
					if (ret == 0)
					{
						ret = do_append_actfmt_list_marker(parser, next);
					}
				}
			}
			return ret;

		case TAG_APPLET:
		case TAG_MARQUEE:
		case TAG_OBJECT:
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_actfmt_list_marker(parser, next);
			return ret;

		case TAG_TABLE:
			ret = do_has_p_in_button_scope(parser, next);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_P);
			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_table);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_prepare_foster(parser, next);
			return ret;

		case TAG_AREA:
		case TAG_BR:
		case TAG_EMBED:
		case TAG_IMG:
		case TAG_INPUT:
		case TAG_WBR:
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_no_stack(parser, next);
			return ret;

		case TAG_PARAM:
			ret = do_append_no_stack(parser, next);
			return ret;

		case TAG_HR:
			ret = do_has_p_in_button_scope(parser, next);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_P);
			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append_no_stack(parser, next);
			return ret;

		case TAG_TEXTAREA:

			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_eat_next_newline(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_parse_rcdata(parser, next);
			return ret;

		case TAG_IFRAME:
		case TAG_NOSCRIPT:
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_parse_rcdata(parser, next);
			return ret;

		case TAG_SELECT:
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_save(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_proc_body_select(parser, next);
			return ret;

		case TAG_OPTGROUP:
		case TAG_OPTION:
			ret = do_is_in_tag(parser, next, TAG_OPTION);
			if (ret == 0)
			{
				ret = do_as_if_end(parser, next, TAG_OPTION);
			}
			else
			{
				ret = 0;
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);

			return ret;

		case TAG_CAPTION:
		case TAG_COL:
		case TAG_COLGROUP:
		case TAG_FRAME:
		case TAG_HEAD:
		case TAG_TBODY:
		case TAG_TD:
		case TAG_TFOOT:
		case TAG_TH:
		case TAG_THEAD:
		case TAG_TR:
			ret = do_warning(parser, next);
			return ret;

		default:
			ret = do_reconstruct_actfmt_list(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_append(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		case TAG_WAP_CARD:
			cal_node_length(parser, next);
			ret = do_has_these_tags_in_scope(parser, next, TAG_WAP_CARD, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_close_tag(parser, next);
				//ret = do_switch(parser, next, on_after_card);
			}
			else
			{
				ret = do_warning(parser, next);
			}
			ret = do_switch(parser, next, on_after_card);
			return ret;

		case TAG_HTML:
			cal_node_length(parser, next);
			ret = do_as_if_end(parser, next, TAG_WAP_CARD);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;

		case TAG_ADDRESS:
		case TAG_BLOCKQUOTE:
		case TAG_BUTTON:
		case TAG_CENTER:
		case TAG_DIR:
		case TAG_DIV:
		case TAG_DL:
		case TAG_FIELDSET:
		case TAG_MENU:
		case TAG_OL:
		case TAG_PRE:
		case TAG_UL:
			cal_node_length(parser, next);
			ret = do_has_this_tag_in_scope(parser, next);
			if (ret == 0)
			{
				ret = do_close_tag(parser, next);
			}
			else
			{
				ret = do_warning(parser, next);
			}

			return ret;

		case TAG_FORM:
			cal_node_length(parser, next);
			ret = do_proc_body_form_end(parser, next);

			return ret;

		case TAG_P:

			ret = do_has_p_in_button_scope(parser, next);
			if (ret == 0)
			{
				ret = do_close_tag(parser, next);
			}
			else
			{
				ret = do_warning(parser, next);
			}

			return ret;
		case TAG_LI:
			ret = do_has_this_tag_in_list_item_scope(parser, next);
			if (ret == 0)
			{
				ret = do_close_tag(parser, next);
			}
			else
			{
				ret = do_warning(parser, next);
			}
			return ret;

		case TAG_DD:
		case TAG_DT:
			cal_node_length(parser, next);
			ret = do_has_this_tag_in_scope(parser, next);
			if (ret == 0)
			{
				ret = do_close_tag(parser, next);
			}
			else
			{
				ret = do_warning(parser, next);
			}
			return ret;

		case TAG_H1:
		case TAG_H2:
		case TAG_H3:
		case TAG_H4:
		case TAG_H5:
		case TAG_H6:
			cal_node_length(parser, next);
			ret = do_has_these_tags_in_scope(parser, next, TAG_H1, TAG_H2, TAG_H3, TAG_H4, TAG_H5, TAG_H6, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_close_these_tags(parser, next, TAG_H1, TAG_H2, TAG_H3, TAG_H4, TAG_H5, TAG_H6, TAG_ROOT);
			}
			else
			{
				ret = do_warning(parser, next);
			}

			return ret;

		case TAG_A:
		case TAG_B:
		case TAG_BIG:
		case TAG_CODE:
		case TAG_EM:
		case TAG_FONT:
		case TAG_I:
		case TAG_NOBR:
		case TAG_S:
		case TAG_SMALL:
		case TAG_STRIKE:
		case TAG_STRONG:
		case TAG_TT:
		case TAG_U:
			cal_node_length(parser, next);
			ret = do_proc_body_formatting_elements_end(parser, next);
			return ret;

		case TAG_APPLET:
		case TAG_MARQUEE:
		case TAG_OBJECT:
			ret = do_has_this_tag_in_scope(parser, next);
			if (ret == 0)
			{
				ret = do_close_tag(parser, next);
				if (ret == 0)
				{
					ret = do_clear_actfmt_list(parser, next);
				}

			}
			else
			{
				ret = do_warning(parser, next);
			}
			return ret;
		case TAG_BR:
			cal_node_length(parser, next);
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_as_if(parser, next, TAG_BR);
			return ret;
		case TAG_CAPTION:
		case TAG_COL:
		case TAG_COLGROUP:
		case TAG_FRAME:
		case TAG_HEAD:
		case TAG_TBODY:
		case TAG_TD:
		case TAG_TFOOT:
		case TAG_TH:
		case TAG_THEAD:
		case TAG_TR:
			cal_node_length(parser, next);
			ret = do_warning(parser, next);
			return ret;
		case TAG_TABLE:
			cal_node_length(parser, next);
			ret = do_warning(parser, next);
			return ret;
		default:
			cal_node_length(parser, next);
			ret = do_proc_body_end(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_after_card(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_WHITESPACE:
			ret = do_forward(parser, next, on_in_card);
			return ret;
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_DOCTYPE:
			ret = do_warning(parser, next);
			return ret;
		case TAG_HTML:
			ret = do_forward(parser, next, on_in_card);
			return ret;
		case TAG_WAP_CARD:

			ret = do_has_these_tags_in_scope(parser, next, TAG_WAP_CARD, TAG_ROOT);
			if (ret == 0)
			{
				ret = do_close_these_tags(parser, next, TAG_WAP_CARD, TAG_ROOT);
				ret = do_append(parser, next);
				if (ret != 0)
				{
					return ret;
				}
				ret = do_switch(parser, next, on_in_card);
				return ret;
			}
			else
			{
				ret = do_warning(parser, next);
			}
			return ret;
		default:
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_card);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		case TAG_HTML:
			ret = do_switch(parser, next, on_after_after_card);
			return ret;
		case TAG_WAP_WML:
			ret = do_switch(parser, next, on_after_after_card);
			return ret;
		default:
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_card);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_after_after_card(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_DOCTYPE:
			ret = do_forward(parser, next, on_in_card);
			return ret;
		case TAG_WHITESPACE:
			ret = do_forward(parser, next, on_in_card);
			return ret;
		case TAG_HTML:
			ret = do_forward(parser, next, on_in_card);
			return ret;
		case TAG_NOFRAME:
			ret = do_forward(parser, next, on_in_head);
			return ret;
		default:
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_card);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		default:
			ret = do_warning(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_in_card);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_redo(parser, next);
			return ret;
		}
	}
	return ret;
}

int on_wml(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_DOCTYPE:
			ret = do_error(parser, next);
			return ret;
		case TAG_WHITESPACE:
			ret = do_ignore(parser, next);
			return ret;
		case TAG_PURETEXT:
			ret = do_append_no_stack(parser, next);
			return ret;
		default:
			ret = do_append(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		default:
			ret = do_is_match(parser, next);
			if (ret == 0)
			{
				ret = do_popup(parser, next);
			}
			else
			{
				ret = do_error(parser, next);
			}
			if (ret != 0)
			{
				return ret;
			}
			ret = do_is_root(parser, next);
			if (ret == 0)
			{
				ret = do_switch(parser, next, on_xml_after);
			}
			else
			{
				ret = 0;
			}
			return ret;
		}
	}
	return ret;
}

int on_wml_root(struct html_parser_t *parser, html_node_t *next)
{
	int ret = 0;
	html_tag_type_t tag = TAG_UNKNOWN;
	int close = 0;
	tag = next->html_tag.tag_type;
	close = next->html_tag.is_close_tag;
	if (!close)
	{
		switch (tag)
		{
		case TAG_COMMENT:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_DOCTYPE:
			ret = do_append_no_stack(parser, next);
			return ret;
		case TAG_WHITESPACE:
			ret = do_ignore(parser, next);
			return ret;
		case TAG_PURETEXT:
			ret = do_error(parser, next);
			return ret;
		case TAG_WAP_WML:
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_before_head);
			return ret;
		case TAG_HTML:
			ret = do_append(parser, next);
			if (ret != 0)
			{
				return ret;
			}
			ret = do_switch(parser, next, on_before_head);
		default:
			ret = do_error(parser, next);
			return ret;
		}
	}
	else
	{
		switch (tag)
		{
		default:
			ret = do_error(parser, next);
			return ret;
		}
	}
	return ret;
}


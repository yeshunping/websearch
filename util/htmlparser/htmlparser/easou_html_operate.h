#ifndef EASOU_HTML_OPERATE_H_
#define EASOU_HTML_OPERATE_H_

#include "queue.h"
#include "easou_html_dtd.h"
#include "easou_html_dom.h"
/**
 * @brief State handler
 **/
typedef int (*state_handler_t)(struct html_parser_t*, html_node_t*);

/**
 * @brief HTML5 constructor meta code append_no_stack
 **/
int do_append_no_stack(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code append
 **/
int do_append(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code switch
 **/
int do_switch(struct html_parser_t *parser, html_node_t *next, state_handler_t handler);
/**
 * @brief HTML5 constructor meta code popup
 **/
int do_popup(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code error
 **/
int do_error(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code warning
 **/
int do_warning(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code ignore
 **/
int do_ignore(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code as_if
 **/
int do_as_if(struct html_parser_t *parser, html_node_t *next, html_tag_type_t type);
/**
 * @brief HTML5 constructor meta code as_if_end
 **/
int do_as_if_end(struct html_parser_t *parser, html_node_t *next, html_tag_type_t type);
/**
 * @brief HTML5 constructor meta code as_if_end_this
 **/
int do_as_if_end_this(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code redo
 **/
int do_redo(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code record_head
 **/
int do_record_head(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code forward
 **/
int do_forward(struct html_parser_t *parser, html_node_t *next, state_handler_t handler);
/**
 * @brief HTML5 constructor meta code assert_in_tag
 **/
int do_assert_in_tag(struct html_parser_t *parser, html_node_t *next, html_tag_type_t type);
/**
 * @brief HTML5 constructor meta code save
 **/
int do_save(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code restore
 **/
int do_restore(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code enter_head
 **/
int do_enter_head(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code exit_head
 **/
int do_exit_head(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code as_if_this_end
 **/
int do_as_if_this_end(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code close_tag
 **/
int do_close_tag(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code close_these_tags
 **/
int do_close_these_tags(struct html_parser_t *parser, html_node_t *next, ...);
/**
 * @brief HTML5 constructor meta code eat_next_newline
 **/
int do_eat_next_newline(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code is_in_tag
 **/
int do_is_in_tag(struct html_parser_t *parser, html_node_t *next, html_tag_type_t type);
/**
 * @brief HTML5 constructor meta code is_in_these_tags
 **/
int do_is_in_these_tags(struct html_parser_t *parser, html_node_t *next, ...);
/**
 * @brief HTML5 constructor meta code merge_attr
 **/
int do_merge_attr(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code proc_body_a
 **/
int do_proc_body_a(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code proc_body_dd_dt
 **/
int do_proc_body_dd_dt(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code proc_body_end
 **/
int do_proc_body_end(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code proc_body_form
 **/
int do_proc_body_form(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code proc_body_formatting_elements_end
 **/
int do_proc_body_formatting_elements_end(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code proc_body_form_end
 **/
int do_proc_body_form_end(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code proc_body_li
 **/
int do_proc_body_li(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code proc_body_select
 **/
int do_proc_body_select(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code proc_select_optgroup_end
 **/
int do_proc_select_optgroup_end(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code close_cell
 **/
int do_close_cell(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code close_to
 **/
int do_close_to(struct html_parser_t *parser, html_node_t *next, ...);
/**
 * @brief HTML5 constructor meta code clear_to_table_context
 **/
int do_clear_to_table_context(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code clear_to_table_row_context
 **/
int do_clear_to_table_row_context(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code clear_to_table_body_context
 **/
int do_clear_to_table_body_context(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code has_form
 **/
int do_has_form(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code has_this_tag_in_scope
 **/
int do_has_this_tag_in_scope(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code has_these_tags_in_scope
 **/
int do_has_these_tags_in_scope(struct html_parser_t *parser, html_node_t *next, ...);
/**
 * @brief HTML5 constructor meta code has_this_tag_in_list_item_scope
 **/
int do_has_this_tag_in_list_item_scope(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code has_p_in_button_scope
 **/
int do_has_p_in_button_scope(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code has_this_tag_in_table_scope
 **/
int do_has_this_tag_in_table_scope(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code has_these_tags_in_table_scope
 **/
int do_has_these_tags_in_table_scope(struct html_parser_t *parser, html_node_t *next, ...);
/**
 * @brief HTML5 constructor meta code is
 **/
int do_is(struct html_parser_t *parser, html_node_t *next, html_tag_type_t type);
/**
 * @brief HTML5 constructor meta code proc_table_input
 **/
int do_proc_table_input(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code reset
 **/
int do_reset(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code prepare_foster
 **/
int do_prepare_foster(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code merge_foster
 **/
int do_merge_foster(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code enter_foster
 **/
int do_enter_foster(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code exit_foster
 **/
int do_exit_foster(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code reconstruct_actfmt_list
 **/
int do_reconstruct_actfmt_list(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code append_actfmt_list
 **/
int do_append_actfmt_list(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code append_actfmt_list_marker
 **/
int do_append_actfmt_list_marker(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code clear_actfmt_list
 **/
int do_clear_actfmt_list(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code parse_rcdata
 **/
int do_parse_rcdata(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code is_match
 **/
int do_is_match(struct html_parser_t *parser, html_node_t *next);
/**
 * @brief HTML5 constructor meta code is_root
 **/
int do_is_root(struct html_parser_t *parser, html_node_t *next);

/*
 * 计算节点长度
 */
int cal_node_length(const html_parser_t *parser, html_node_t *next);

#endif /* EASOU_HTML_OPERATE_H_ */

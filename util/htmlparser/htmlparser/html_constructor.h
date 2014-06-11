#ifndef __EASOU_HTML_CONSTRUCTOR_H_
#define __EASOU_HTML_CONSTRUCTOR_H_

#include "html_parser.h"
#include "html_dom.h"

int on_initial(struct html_parser_t*, html_node_t*);
int on_before_html(struct html_parser_t*, html_node_t*);
int on_before_head(struct html_parser_t*, html_node_t*);
int on_in_head(struct html_parser_t*, html_node_t*);
int on_after_head(struct html_parser_t*, html_node_t*);
int on_in_body(struct html_parser_t*, html_node_t*);
int on_in_table(struct html_parser_t*, html_node_t*);
int on_in_caption(struct html_parser_t*, html_node_t*);
int on_in_column_group(struct html_parser_t*, html_node_t*);
int on_in_table_body(struct html_parser_t*, html_node_t*);
int on_in_row(struct html_parser_t*, html_node_t*);
int on_in_cell(struct html_parser_t*, html_node_t*);
int on_in_select(struct html_parser_t*, html_node_t*);
int on_in_select_in_table(struct html_parser_t*, html_node_t*);
int on_after_body(struct html_parser_t*, html_node_t*);
int on_in_frameset(struct html_parser_t*, html_node_t*);
int on_after_frameset(struct html_parser_t*, html_node_t*);
int on_after_after_body(struct html_parser_t*, html_node_t*);
int on_after_after_frameset(struct html_parser_t*, html_node_t*);
int on_xml_root(struct html_parser_t*, html_node_t*);
int on_xml(struct html_parser_t*, html_node_t*);
int on_xml_after(struct html_parser_t*, html_node_t*);
int on_wml(struct html_parser_t*, html_node_t*);
int on_wml_root(struct html_parser_t*, html_node_t*);
int on_in_card(struct html_parser_t *parser, html_node_t*);
int on_after_card(struct html_parser_t *parser, html_node_t*);
int on_after_after_card(struct html_parser_t *parser, html_node_t*);

#endif /*__EASOU_HTML_CONSTRUCTOR_H_*/


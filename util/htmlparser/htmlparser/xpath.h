

#ifndef EASOU_XPATH_H_
#define EASOU_XPATH_H_
#define ATTRLISTLENGTH 5

#include "html_dom.h"
#include <assert.h>
struct xpathattr
{
	int attr_type;
	char * name;
	size_t namelength;
	char * value;
	size_t valuelength;
	xpathattr * next;
};
struct xpathnode
{
	int tag_type;
	char * name;
	size_t namelength;
	int isfather; //1: 父亲;2:祖先;0:当前节点的属性
	int index;
	xpathattr * attrlist[ATTRLISTLENGTH]; //同一个属性列表中的是与的关系
	xpathnode * pre;
};

struct XPATH_RESULT_NODE
{
	//int isvalid;
	int type; //0:html_tag;1:html_attr
	void * node; //符合条件的节点

};
/**
 * 将xpath字符串解析成xpath表达式,解析结果必须用freexpath释放
 * xpathval：in，xpath字符串表达式
 * xpathlist：out，xpath表达式
 * xpathlength：in，out，xpath表达式个数
 * 结果：-1：参数错误；0：成功
 */
int parserxpath(char * xpathval, xpathnode ** xpathlist, int * xpathlength);
/**
 * 倒序打印xpath解析表达式
 * xpathlist：in
 * xpathlength：in
 * 结果：-1：参数错误；0：正确
 */
int printxpath(xpathnode ** xpathlist, int xpathlength);
/**
 * 释放xpath字符串解析的xpath表达式
 * xpathlist：in,out
 *  结果：-1：参数错误；0：成功
 */
int freexpath(xpathnode ** xpathlist);

/**
 * 利用单个xpath表达式dom树节点
 * htmlnode：in
 * xpathlist：in
 * result：in，out
 * resultlen：in，out
 * 结果：满足条件的节点数
 */
int xpathselectnode(html_node_t* htmlnode, xpathnode * xpathlist, XPATH_RESULT_NODE * result, int & resultlen);
/**
 * 利用多个xpath表达式dom树节点
 * htmlnode：in
 * xpathlist：in
 * xpathlength：in
 * result：in，out
 * resultlen：in，out
 * 结果：满足条件的节点数
 */
int xpathselect(html_node_t* htmlnode, xpathnode ** xpathlist, int xpathlength, XPATH_RESULT_NODE *result, int & resultlen);
#endif /* EASOU_XPATH_H_ */


#include "xpath.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "html_attr.h"
#include "html_dtd.h"

#define NODESTATUS 1
#define ATTRNAMESTATUS 2
#define ATTRVALSTATUS 3

struct XPATHVISIT
{
	int cur_pos;
	int length;
	html_node_t *self_node;
	XPATH_RESULT_NODE *nodelist;
	xpathnode * tmppxpath;
};

/**
 * attrlist [in ,out]:释放属性空间
 * 结果：-1：参数错误；0：成功
 */
int freeattr(xpathattr ** attrlist)
{
	if (attrlist == NULL)
	{
		return -1;
	}
	int j = 0;
	while (attrlist[j])
	{
		xpathattr * attr = attrlist[j];
		while (attr)
		{

			xpathattr *temp = attr;
			attr = attr->next;
			free(temp);
		}
		j++;
	}
	return 0;

}

/**
 * 释放xpath字符串解析的xpath表达式
 * xpathlist：in,out
 *  结果：-1：参数错误；0：成功
 */
int freexpath(xpathnode ** xpathlist)
{
	if (xpathlist == NULL)
	{
		return -1;
	}
	int i = 0;
	while (xpathlist[i])
	{
		xpathnode * xpathnode0 = xpathlist[i];
		while (xpathnode0)
		{
			freeattr(xpathnode0->attrlist);
			xpathnode * temp = xpathnode0;
			xpathnode0 = xpathnode0->pre;
			free(temp);
		}
		xpathlist[i] = NULL;
		i++;
	}
	return 0;

}
/**
 * 对xpath解析式的node节点进行反转
 * xpathlist：in,out
 * 结果：-1：参数错误；0：成功
 */
int xpath_reverse(xpathnode ** xpathlist, int xpathlength)
{

	if (xpathlist == NULL)
	{
		return -1;
	}
	int i = 0;
	xpathnode * xpatha[5000] =
	{ 0 };
	while (xpathlist[i] && i < xpathlength)
	{
		xpathnode * pxpathnode = xpathlist[i];
		memset(xpatha, 0, sizeof(xpatha));
		int j = 0;
		while (pxpathnode)
		{
			xpatha[j++] = pxpathnode;
			pxpathnode = pxpathnode->pre;

		}
		xpathlist[i] = xpatha[j - 1];
		for (j--; j >= 1; j--)
		{
			xpatha[j]->pre = xpatha[j - 1];
		}
		xpatha[0]->pre = NULL;
		;
		i++;
	}
	return 0;

}
/**
 * 将xpath字符串解析成xpath表达式
 * xpathval：in，xpath字符串表达式
 * xpathlist：out，xpath表达式
 * xpathlength：in，out，xpath表达式个数
 * 结果：-1：参数错误；0：成功
 */
int parserxpath(char * xpathval, xpathnode ** xpathlist, int * xpathlength)
{
	if (xpathval == NULL || xpathlist == NULL || xpathlength == NULL)
	{
		return -1;
	}
	int oldxpathsize = *xpathlength;
	char *pxpath = xpathval;
	int slapcount = 0;
	bool run = true;
	int xpathlistcount = 0;
	int chartype = 0;
	char *pnodename = NULL;
	char *pattrname = NULL;
	char *pattrvalue = NULL;
	xpathnode * pxpathnode = NULL;
	xpathattr * pattr = NULL;
	int attrindex = 0;
	char * lastpos = NULL;
	while ((*pxpath) && run)
	{
		//printf("process c=%c",*pxpath);
		if (lastpos == pxpath)
		{
			run = false;
			break;
		}
		else
		{
			lastpos = pxpath;
		}
		if (*pxpath == ' ' || *pxpath == '\t')
		{
			pxpath++;
			continue;
		}
		if (*pxpath == '/')
		{
			slapcount++;
			pxpath++;
			attrindex = 0;
			pxpathnode = NULL;
			chartype = NODESTATUS;
			if (slapcount > 2)
			{
				run = false;
				break;
			}
			continue;
		}
		if (*pxpath == '|')
		{
			xpathlistcount++;
			if (xpathlistcount >= oldxpathsize)
			{
				run = false;
				break;
			}
			pxpath++;
			chartype = NODESTATUS;
			slapcount = 0;
			if (*pxpath == '|')
			{
				run = false;
				break;
			}

			continue;
		}

		if ((*pxpath) == '=')
		{
			if (chartype == ATTRVALSTATUS)
			{

				pxpath++;
				while (*pxpath == ' ' || *pxpath == '\t')
				{
					pxpath++;
				}
				char separator = 0;
				if (*pxpath == '\'' || *pxpath == '"')
				{

					separator = *pxpath;
					pxpath++;
					pattrvalue = pxpath;
				}
				while (*pxpath != separator && *pxpath != 0)
				{
					pxpath++;
				}
				if (*pxpath == 0)
				{
					run = false;
					break;
				}
				if (*pxpath == separator)
				{
					pattr->value = pattrvalue;
					pattr->valuelength = pxpath - pattrvalue;
					pxpath++;
				}
				while (*pxpath == ' ' || *pxpath == '\t')
				{
					pxpath++;
				}
				if (*pxpath == ']')
				{
					pxpath++;
					chartype = ATTRNAMESTATUS;
				}

				if (*pxpath == 'a' || *pxpath == 'A')
				{
					pxpath = pxpath + 3;
					chartype = ATTRNAMESTATUS;
				}
				if (*pxpath == 'o' || *pxpath == 'O')
				{
					pxpath++;
					pxpath++;
					attrindex++;
					chartype = ATTRNAMESTATUS;
					if (attrindex >= ATTRLISTLENGTH)
					{
						run = false;
						break;
					}
				}
				continue;

			}
			else
			{
				run = false;
				break;
			}

		}
		if (*pxpath == '[')
		{
			// attribute begin
			pxpath++;
			if (*pxpath == '@')
			{
				pxpath++;
				chartype = ATTRNAMESTATUS;
				continue;
			}
			if (*pxpath >= '0' && *pxpath <= '9')
			{
				int pos = 0;
				while (*pxpath >= '0' && *pxpath <= '9')
				{
					pos = pos * 10 + (*pxpath - '0');
					pxpath++;
				}
				if (pos > 0)
				{
					pxpathnode->index = pos;
				}
				if (*pxpath == ']')
				{
					pxpath++;
					chartype = ATTRNAMESTATUS;
					continue;
				}
			}
		}
		if (*pxpath == '@' && chartype == ATTRNAMESTATUS)
		{
			pxpath++;
			continue;
		}
		if (*pxpath == '@' && NODESTATUS == chartype)
		{
			pxpath++;
			if (slapcount == 1)
			{

				pnodename = pxpath;
				{
					pxpathnode = (xpathnode*) malloc(sizeof(xpathnode));
					if (!pxpathnode)
					{
						run = false;
						break;
					}

				}
				pxpathnode->isfather = 0;
				pxpathnode->index = 0;
				pxpathnode->pre = xpathlist[xpathlistcount];
				xpathlist[xpathlistcount] = pxpathnode;
				while (('a' <= *pxpath && *pxpath <= 'z') || ('A' <= *pxpath && *pxpath <= 'Z'))
				{
					pxpath++;
				}
				pxpathnode->name = pnodename;
				pxpathnode->namelength = pxpath - pnodename;
				pxpathnode->tag_type = get_attr_type((const char *) (pxpathnode->name), pxpathnode->namelength);
				slapcount = 0;
			}
			continue;
		}
		if (('a' <= *pxpath && *pxpath <= 'z') || ('A' <= *pxpath && *pxpath <= 'Z') || (*pxpath == '*'))
		{

			if (NODESTATUS == chartype)
			{ //process node name
				if (slapcount == 1)
				{
					{
						pxpathnode = (xpathnode*) malloc(sizeof(xpathnode));
						if (!pxpathnode)
						{
							run = false;
							break;
						}

					}
					pxpathnode->index = 0;
					pxpathnode->isfather = 1;
					pxpathnode->pre = xpathlist[xpathlistcount];
					xpathlist[xpathlistcount] = pxpathnode;
					slapcount = 0;
				}
				if (slapcount == 2)
				{
					{
						pxpathnode = (xpathnode*) malloc(sizeof(xpathnode));
						if (!pxpathnode)
						{
							run = false;
							break;
						}

					}
					pxpathnode->index = 0;
					pxpathnode->isfather = 2;
					pxpathnode->pre = xpathlist[xpathlistcount];
					xpathlist[xpathlistcount] = pxpathnode;
					slapcount = 0;
				}
				pnodename = pxpath;
				while (('a' <= *pxpath && *pxpath <= 'z') || ('A' <= *pxpath && *pxpath <= 'Z') || ('(' == *pxpath) || (')' == *pxpath)
						|| (*pxpath == '*') || (*pxpath >= '0' && *pxpath <= '9'))
				{
					pxpath++;
				}
				pxpathnode->name = pnodename;
				pxpathnode->namelength = pxpath - pnodename;
				pxpathnode->tag_type = get_tag_type((const char *) (pxpathnode->name), pxpathnode->namelength);
				if (*pxpathnode->name == '*')
				{
					pxpathnode->tag_type = -1;
				}
				if (strncasecmp(pxpathnode->name, "text()", pxpathnode->namelength) == 0)
				{
					pxpathnode->tag_type = TAG_PURETEXT;
				}
				chartype = ATTRNAMESTATUS;
				continue;
			}
			if (chartype == ATTRNAMESTATUS) //process attribute name
			{
				{
					pattr = (xpathattr*) malloc(sizeof(xpathattr));
					if (!pattr)
					{
						run = false;
						break;
					}
					pattr->attr_type = -1;
				}
				pattrname = pxpath;
				while (('a' <= *pxpath && *pxpath <= 'z') || ('A' <= *pxpath && *pxpath <= 'Z'))
				{
					pxpath++;
				}
				if (*pxpath == 0)
				{
					run = false;
					break;
				}
				pattr->name = pattrname;
				pattr->namelength = pxpath - pattrname;
				pattr->attr_type = get_attr_type((const char *) (pattr->name), pattr->namelength);
				pattr->next = pxpathnode->attrlist[attrindex];
				pxpathnode->attrlist[attrindex] = pattr;
				if (*pxpath == ']')
				{
					pxpath++;
					chartype = ATTRNAMESTATUS;
				}
				else
				{
					if (*pxpath == '=')
					{
						chartype = ATTRVALSTATUS;
					}
					else if (*pxpath == ' ' || *pxpath == '\t')
					{
						chartype = ATTRNAMESTATUS;
						while (*pxpath == ' ' || *pxpath == '\t')
						{
							pxpath++;
						}
						if (*pxpath == 'a' || *pxpath == 'A')
						{
							pxpath = pxpath + 3;
						}
						if (*pxpath == 'o' || *pxpath == 'O')
						{
							pxpath++;
							pxpath++;
							attrindex++;
							if (attrindex >= ATTRLISTLENGTH)
							{
								run = false;
								break;
							}
						}
					}
				}
			}
			continue;
		}
		if (*pxpath == ' ' || *pxpath == '\t')
		{
			continue;
		}
//		printf("xpath string:%s ; current char is %c ,pos=%d\n",xpathval,*pxpath,pxpath-xpathval);
//		run=false;
//		break;

	}
	if (!run)
	{
		//printf("xpath string:%s; current char is %c,ascii=%d ,pos=%d\n",xpathval,*pxpath,*pxpath,pxpath-xpathval);
		freexpath(xpathlist);
		if (xpathlistcount >= oldxpathsize)
		{
			return 2;
		}
		if (attrindex >= ATTRLISTLENGTH)
		{
			return 3;
		}
		return 1;
	}
	else
	{
		*xpathlength = (xpathlistcount + 1);
		xpath_reverse(xpathlist, (xpathlistcount + 1));
		return 0;
	}
}

/**
 * 判断单个节点是否满足xpath单个节点
 * xpath：in
 * node：in
 * 结果：0：满足条件；其他：不满足条件
 */
int is_node_for_single_xpath(const xpathnode * xpath, html_node_t *node)
{

	// printf("node type=%d,xpath type=%d\n",node->html_tag.tag_type,xpath->tag_type);
	int result = 1;
	if (node != NULL && xpath)
	{
		if ((xpath->tag_type == -1 || xpath->tag_type == (int) node->html_tag.tag_type) && xpath->isfather > 0)
		{
			int i = 0;
			if (xpath->attrlist[0] == NULL)
			{
				return 0;
			}
			while (xpath->attrlist[i])
			{
				xpathattr * pattr = xpath->attrlist[i];
				result = 0;
				while (pattr)
				{
					char name[2000] =
					{ 0 };
					char value[1024] =
					{ 0 };
					memcpy(name, pattr->name, pattr->namelength);
					html_attribute_t *attr = html_node_get_attribute_by_name(node, (const char *) name);
					if (!attr)
					{
						result = 1;
						break;
					}
					else
					{
						if (pattr->value && pattr->valuelength)
						{
							memcpy(value, pattr->value, pattr->valuelength);
							if (attr->value == NULL || strcmp(attr->value, value) != 0)
							{
								result = 1;
								break;
							}
						}
					}

					pattr = pattr->next;
				}
				if (result == 0)
				{
					return 0;
				}
				i++;
			}
			result = 1;

		}
	}
	else
	{
		result = 1;
	}
	return result;
}

/**
 * 倒序打印xpath解析表达式
 * xpathlist：in
 * xpathlength：in
 * 结果：-1：参数错误；0：正确
 */
int printRxpath(xpathnode ** xpathlist, int xpathlength)
{

	if (xpathlist == NULL || xpathlength < 1)
	{
		return -1;
	}
	int i = 0;
	xpathnode * xpatha[5000] =
	{ 0 };
	while (xpathlist[i] && i < xpathlength)
	{
		xpathnode * pxpathnode = xpathlist[i];
		memset(xpatha, 0, sizeof(xpatha));
		int j = 0;
		while (pxpathnode)
		{
			xpatha[j++] = pxpathnode;
			pxpathnode = pxpathnode->pre;

		}
		for (j--; j >= 0; j--)
		{
			if (xpatha[j]->isfather == 0)
			{
				printf("/@");
			}
			if (xpatha[j]->isfather == 1)
			{
				printf("/");
			}
			if (xpatha[j]->isfather == 2)
			{
				printf("//");
			}
			unsigned int cc = 0;
			for (; cc < xpatha[j]->namelength; cc++)
			{
				printf("%c", xpatha[j]->name[cc]);
			}
			if (xpatha[j]->index > 0)
			{
				printf("[%d]", xpatha[j]->index);
			}
			if (xpatha[j]->attrlist[0])
			{
				printf("[");
				int mm = 0;
				while (xpatha[j]->attrlist[mm])
				{
					xpathattr * pattr = xpatha[j]->attrlist[mm];
					while (pattr)
					{
						cc = 0;
						if (pattr->namelength > 0)
						{
							printf("@");
						}
						cc = 0;
						for (; cc < pattr->namelength; cc++)
						{
							printf("%c", pattr->name[cc]);
						}
						if (pattr->valuelength > 0)
						{
							printf("='");
							cc = 0;
							for (; cc < pattr->valuelength; cc++)
							{
								printf("%c", pattr->value[cc]);
							}
							printf("'");
						}
						pattr = pattr->next;
						if (pattr)
						{
							printf(" and ");
						}
					}
					if (xpatha[j]->attrlist[mm + 1])
					{
						printf(" or ");
					}

					mm++;
				}
				printf("]");
			}
		}
		if (i + 1 < xpathlength)
		{
			printf("|");
		}

		i++;
	}
	return 0;

}

/**
 * @brief Pre visitor
 **/
typedef int (*start_visit_tforxpath)(html_node_t *html_node, void* result, int flag);

/**
 * @brief Post visitor
 **/
typedef int (*finish_visit_tforxpath)(html_node_t *html_node, void* result);

/**
 * @brief 遍历dom树遍历节点
 **/
int html_node_visitforxpath(html_node_t *html_node, start_visit_tforxpath start_visit, finish_visit_tforxpath finish_visit, void *result,
		int flag)
{
	int ret = VISIT_NORMAL;
	int lret = VISIT_NORMAL;
	html_node_t *current = NULL;

	current = html_node;
	while (current)
	{
		if (start_visit)
		{
			//printf("start visit current name=%s ,current=%x\n",current->html_tag.tag_name,current);
			lret = start_visit(current, result, flag);
			if (lret == VISIT_ERROR || lret == VISIT_FINISH)
			{
				ret = lret;
				//XPATHVISIT * pvisit=(XPATHVISIT *)result;
				// printf("ret=%d\n",ret);
				return ret;
			}
		}
		if (current->child && lret != VISIT_SKIP_CHILD)
		{
			//printf(" current = current->child; current name=%s ,current=%x\n",current->html_tag.tag_name,current);
			current = current->child;
			continue;
		}
		if (finish_visit && lret != VISIT_SKIP_CHILD)
		{
			lret = finish_visit(current, result);
			if (lret == VISIT_ERROR || lret == VISIT_FINISH)
			{
				// printf("finish ret=%d\n",ret);
				ret = lret;
				return ret;
			}
			assert(lret == VISIT_NORMAL);
		}
		if (current == html_node)
		{
			return ret;
		}
		if (current->next)
		{
			//	printf("current = current->next;; current name=%s ,current=%x\n",current->html_tag.tag_name,current);
			current = current->next;
			continue;
		}
		while (1)
		{
			current = current->parent;
			// printf("current = current->parent;;; current name=%s ,current=%x\n",current->html_tag.tag_name,current);
			if (!current)
			{
				break;
			}
			if (finish_visit)
			{
				lret = finish_visit(current, result);
				if (lret == VISIT_ERROR || lret == VISIT_FINISH)
				{
					ret = lret;
					return ret;
				}
				assert(lret == VISIT_NORMAL);
			}
			if (current == html_node)
			{
				return ret;
			}

			if (current->next)
			{
				// 	printf("current = current->next;;;; current name=%s ,current=%x\n",current->html_tag.tag_name,current);
				current = current->next;
				break;
			}
		}
	}
	return ret;
}

//int is_node_for_xpath(xpathnode ** xpathlist,int  xpathlength,html_node_t *node){
//	int result=1;
//
//	if((xpathlist == NULL)||(xpathlength < 1)||(node == NULL)){
//		return -1;
//	}
//
//	int i=0;
//	while(xpathlist[i]&&result!=0&&i<xpathlength){
//		result=is_node_for_single_xpath(xpathlist[i],node);
//		if(result==0){
//			return 0;
//		}
//		i++;
//	}
//	return result;
//}

int start_visit_xpath(html_node_t* node, void *result, int flag)
{
	XPATHVISIT * pvisit = (XPATHVISIT *) result;
	xpathnode * pxpath = pvisit->tmppxpath;
	if (pvisit->cur_pos >= pvisit->length)
	{
		printf("cur_pos=%d,length=%d\n", pvisit->cur_pos, pvisit->length);
		return VISIT_FINISH;
	}
	int isfit = 0;
	//printf("start visit node name=%s,node=%x\n",node->html_tag.tag_name,node);
	if (flag > 0 && pvisit->self_node == node)
	{
		return VISIT_NORMAL;
	}
	// printf("start visit node name=%s,node=%x\n",node->html_tag.tag_name,node);
	if (pxpath->isfather == 1)
	{

		if (pxpath->tag_type != node->html_tag.tag_type && pxpath->tag_type != -1)
		{
			return VISIT_SKIP_CHILD;
		}
		//printf("node name=%s\n",node->html_tag.tag_name);
		int pos = 0;
		html_node_t* parent = node->parent;
		if (pxpath->index > 0 && parent)
		{
			for (html_node_t* child = parent->child; child; child = child->next)
			{

				if (child->html_tag.tag_type == pxpath->tag_type || pxpath->tag_type == -1)
				{
					pos++;
					if (pxpath->index == pos && child == node)
					{
						isfit = is_node_for_single_xpath(pxpath, node);
						if (isfit == 0)
						{
							xpathnode * pnext = pxpath->pre;
							if (pnext && pnext->isfather == 0)
							{
								char name[2000] =
								{ 0 };
								memcpy(name, pnext->name, pnext->namelength);
								html_attribute_t *attr = html_node_get_attribute_by_name(node, (const char *) name);
								if (attr)
								{
									//printf("copy attr=%s\n",attr->name);
									if ((pvisit->cur_pos && pvisit->nodelist[pvisit->cur_pos - 1].node != attr) || (pvisit->cur_pos == 0))
									{
										pvisit->nodelist[pvisit->cur_pos].node = attr;
										pvisit->nodelist[pvisit->cur_pos].type = 1;
										pvisit->cur_pos++;
									}
								}
							}
							else
							{
								if (pvisit->cur_pos && pvisit->nodelist[pvisit->cur_pos - 1].node != node || pvisit->cur_pos == 0)
								{
									pvisit->nodelist[pvisit->cur_pos].node = node;
									pvisit->nodelist[pvisit->cur_pos].type = 0;
									pvisit->cur_pos++;
								}
							}
							return VISIT_SKIP_CHILD;
						}
						else
						{
							return VISIT_SKIP_CHILD;
						}

					}
				}
			}
		}
		else
		{
			isfit = is_node_for_single_xpath(pxpath, node);
			if (isfit == 0)
			{
				xpathnode * pnext = pxpath->pre;
				if (pnext && pnext->isfather == 0)
				{
					char name[2000] =
					{ 0 };
					memcpy(name, pnext->name, pnext->namelength);
					html_attribute_t *attr = html_node_get_attribute_by_name(node, (const char *) name);
					if (attr)
					{
						//printf("copy attr=%s\n",attr->name);
						if ((pvisit->cur_pos && pvisit->nodelist[pvisit->cur_pos - 1].node != attr) || (pvisit->cur_pos == 0))
						{
							pvisit->nodelist[pvisit->cur_pos].node = attr;
							pvisit->nodelist[pvisit->cur_pos].type = 1;
							pvisit->cur_pos++;
						}
					}
				}
				else
				{
					//printf("copy node=%s\n",node->html_tag.tag_name);
					if ((pvisit->cur_pos && pvisit->nodelist[pvisit->cur_pos - 1].node != node) || (pvisit->cur_pos == 0))
					{
						pvisit->nodelist[pvisit->cur_pos].node = node;
						pvisit->nodelist[pvisit->cur_pos].type = 0;
						pvisit->cur_pos++;
					}
					else
					{

					}

				}
				return VISIT_SKIP_CHILD;
			}
			else
			{
				return VISIT_SKIP_CHILD;
			}
		}
		return VISIT_SKIP_CHILD;
	}
	if (pxpath->isfather == 2)
	{

		if (pxpath->tag_type != node->html_tag.tag_type && pxpath->tag_type != -1)
		{
			return VISIT_NORMAL;
		}

		int pos = 0;
		//printf("");
		html_node_t* parent = node->parent;
		if (pxpath->index > 0 && parent)
		{

			for (html_node_t* child = parent->child; child; child = child->next)
			{

				if ((child->html_tag.tag_type == pxpath->tag_type) || (pxpath->tag_type == -1))
				{

					pos++;
					//printf("node name=%s,pos=%d,index=%d,child=%x,node=%x\n",node->html_tag.tag_name,pos,pxpath->index,child,node);
					//printf("%d child=%x ,node=%x ,node name =%s\n",pos,child,node,node->html_tag.tag_name);
					if (pxpath->index == pos && child == node)
					{

						isfit = is_node_for_single_xpath(pxpath, node);
						if (isfit == 0)
						{
							xpathnode * pnext = pxpath->pre;
							if (pnext && pnext->isfather == 0)
							{
								char name[2000] =
								{ 0 };
								memcpy(name, pnext->name, pnext->namelength);
								html_attribute_t *attr = html_node_get_attribute_by_name(node, (const char *) name);
								if (attr)
								{

									//printf("copy attr=%s\n",attr->name);
									if ((pvisit->cur_pos && pvisit->nodelist[pvisit->cur_pos - 1].node != attr) || (pvisit->cur_pos == 0))
									{
										pvisit->nodelist[pvisit->cur_pos].node = attr;
										pvisit->nodelist[pvisit->cur_pos].type = 1;
										pvisit->cur_pos++;
									}

								}
							}
							else
							{
								if ((pvisit->cur_pos && pvisit->nodelist[pvisit->cur_pos - 1].node != node) || (pvisit->cur_pos == 0))
								{
									pvisit->nodelist[pvisit->cur_pos].node = node;
									pvisit->nodelist[pvisit->cur_pos].type = 0;
									pvisit->cur_pos++;
								}
							}
							return VISIT_NORMAL;
						}
						else
						{
							return VISIT_NORMAL;
						}

					}
				}
			}
		}
		else
		{
			isfit = is_node_for_single_xpath(pxpath, node);
			if (isfit == 0)
			{
				xpathnode * pnext = pxpath->pre;
				if (pnext && pnext->isfather == 0)
				{
					char name[2000] =
					{ 0 };
					memcpy(name, pnext->name, pnext->namelength);
					html_attribute_t *attr = html_node_get_attribute_by_name(node, (const char *) name);
					if (attr)
					{
						//printf("copy attr=%s\n",attr->name);
						if ((pvisit->cur_pos && pvisit->nodelist[pvisit->cur_pos - 1].node != attr) || (pvisit->cur_pos == 0))
						{
							pvisit->nodelist[pvisit->cur_pos].node = attr;
							pvisit->nodelist[pvisit->cur_pos].type = 1;
							pvisit->cur_pos++;
						}
					}
				}
				else
				{
					if ((pvisit->cur_pos && pvisit->nodelist[pvisit->cur_pos - 1].node != node) || (pvisit->cur_pos == 0))
					{
						pvisit->nodelist[pvisit->cur_pos].node = node;
						pvisit->nodelist[pvisit->cur_pos].type = 0;
						pvisit->cur_pos++;
					}
				}
				return VISIT_NORMAL;
			}
			else
			{
				return VISIT_NORMAL;
			}
		}
		return VISIT_NORMAL;
	}
	if (pxpath->isfather == 0)
	{
		return VISIT_ERROR;
	}
	if (pxpath->isfather < 0 || pxpath->isfather > 2)
	{
		printf("pxpath->isfather=%d\n", pxpath->isfather);
	}
	return VISIT_NORMAL;

}

/**
 * 利用单个xpath表达式dom树节点
 * htmlnode：in
 * xpathlist：in
 * result：in，out
 * resultlen：in，out
 * 结果：满足条件的节点数
 */
int xpathselectnode(html_node_t* htmlnode, xpathnode * xpathlist, XPATH_RESULT_NODE *result, int & resultlen)
{

	XPATH_RESULT_NODE tmpresult[5000] =
	{
	{ 0, 0 } };
	html_node_t *tmpresult1[5000] =
	{ 0 };
	XPATHVISIT visit;
	visit.cur_pos = 0;
	visit.length = 5000;
	visit.nodelist = tmpresult;
	visit.tmppxpath = xpathlist;
	int flag = 0;
	int nodelength = 1;
	tmpresult1[0] = htmlnode;
	bool run = true;
	while (xpathlist && run && xpathlist->isfather != 0)
	{
		visit.tmppxpath = xpathlist;
		int i = 0;
		visit.cur_pos = 0;
		memset(tmpresult, 0, sizeof(tmpresult));
		//printf("nodelength=%d\n",nodelength);
		while (tmpresult1[i] && i < nodelength)
		{
			//printf("process  node =%s\n",tmpresult1[i]->html_tag.tag_name);
			if (flag > 0)
			{
				visit.self_node = tmpresult1[i];
			}
			html_node_visitforxpath(tmpresult1[i], start_visit_xpath, NULL, &visit, flag);
			i++;
		}
		flag++;
		if (visit.cur_pos < 1)
		{

			printf(" cur pos<1 break,flag=%d\n", flag);
			run = false;
			break;
		}
		if (run)
		{
			memset(tmpresult1, 0, sizeof(tmpresult1));
			for (i = 0; i < visit.cur_pos; i++)
			{
				tmpresult1[i] = (html_node_t*) (tmpresult[i].node);
			}
			nodelength = visit.cur_pos;

			xpathlist = xpathlist->pre;
		}

	}
	if (run && visit.cur_pos > 0)
	{
		for (int i = 0; i < visit.cur_pos && i < resultlen; i++)
		{
			// printf("copy addr=%x,type=%d\n",visit.nodelist[i].node,visit.nodelist[i].type);
			result[i].node = visit.nodelist[i].node;
			result[i].type = visit.nodelist[i].type;
		}
		resultlen = visit.cur_pos;
	}
	else
	{
		resultlen = 0;
	}
	return resultlen;
}

/**
 * 打印xpath解析表达式
 * xpathlist：in
 * xpathlength：in
 * 结果：-1：参数错误；0：正确
 */
int printxpath(xpathnode ** xpathlist, int xpathlength)
{

	if (xpathlist == NULL)
	{
		return -1;
	}
	int i = 0;
	xpathnode * xpatha[5000] =
	{ 0 };
	while (xpathlist[i] && i < xpathlength)
	{
		xpathnode * pxpathnode = xpathlist[i];
		memset(xpatha, 0, sizeof(xpatha));
		int j = 0;
		while (pxpathnode)
		{
			xpatha[j] = pxpathnode;
			if (xpatha[j]->isfather == 0)
			{
				printf("/@");
			}
			if (xpatha[j]->isfather == 1)
			{
				printf("/");
			}
			if (xpatha[j]->isfather == 2)
			{
				printf("//");
			}
			unsigned int cc = 0;
			for (; cc < xpatha[j]->namelength; cc++)
			{
				printf("%c", xpatha[j]->name[cc]);
			}
			if (xpatha[j]->index > 0)
			{
				printf("[%d]", xpatha[j]->index);
			}
			if (xpatha[j]->attrlist[0])
			{
				printf("[");
				int mm = 0;
				while (xpatha[j]->attrlist[mm])
				{
					xpathattr * pattr = xpatha[j]->attrlist[mm];
					while (pattr)
					{
						cc = 0;
						if (pattr->namelength > 0)
						{
							printf("@");
						}
						cc = 0;
						for (; cc < pattr->namelength; cc++)
						{
							printf("%c", pattr->name[cc]);
						}
						if (pattr->valuelength > 0)
						{
							printf("='");
							cc = 0;
							for (; cc < pattr->valuelength; cc++)
							{
								printf("%c", pattr->value[cc]);
							}
							printf("'");
						}
						pattr = pattr->next;
						if (pattr)
						{
							printf(" and ");
						}
					}
					if (xpatha[j]->attrlist[mm + 1])
					{
						printf(" or ");
					}
					mm++;
				}
				printf("]");
			}
			j++;
			pxpathnode = pxpathnode->pre;
		}

		if (i + 1 < xpathlength)
		{
			printf("|");
		}
		i++;
	}
	return 0;
}

/**
 * 利用多个xpath表达式dom树节点
 * htmlnode：in
 * xpathlist：in
 * xpathlength：in
 * result：in，out
 * resultlen：in，out
 * 结果：满足条件的节点数
 */
int xpathselect(html_node_t* htmlnode, xpathnode ** xpathlist, int xpathlength, XPATH_RESULT_NODE *result, int & resultlen)
{
	bool run = true;
	int iresult = 0;
	int pos = 0;
	if (htmlnode && xpathlist && xpathlength > 0 && result && resultlen > 0)
	{

		int i = 0;
		while (xpathlist[i] && i < xpathlength && run)
		{
			int templength = 5000;
			XPATH_RESULT_NODE tempresult[5000] =
			{
			{ 0, 0 } };
			xpathselectnode(htmlnode, xpathlist[i], tempresult, templength);
			if (templength > 0)
			{
				for (int j = 0; j < templength && pos < resultlen; j++)
				{
					result[pos].node = tempresult[j].node;
					result[pos].type = tempresult[j].type;
					pos++;
					if (pos >= resultlen)
					{
						run = false;
						break;
					}
				}
			}
			i++;
		}
		iresult = pos;
		resultlen = pos;

	}
	else
	{
		iresult = -1;
	}
	return iresult;
}

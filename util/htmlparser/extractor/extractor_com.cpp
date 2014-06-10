/*
 * easou_extractor_com.cpp
 *
 *  Created on: 2012-1-11
 *      Author: xunwu
 */
#include "html_text_utils.h"
#include "easou_mark_baseinfo.h"
#include "easou_extractor_com.h"
#include "easou_extractor_content.h"
#include  "html_text_utils.h"
#include "easou_html_attr.h"
#include "easou_html_extractor.h"
#include "easou_url.h"
typedef struct LinksOfATree_t
{
	area_tree_t *area_tree;
	anchor_info *links;
	int available;
	int end;
	const char *baseurl;
	char base_domain[MAX_SITE_LEN];
	char base_port[MAX_PORT_LEN];
	char base_path[MAX_PATH_LEN];
} LinksOfATree;

struct longest_text_visit_t
{
	html_vnode_t *last_text_vnode;
	html_vnode_t *last2_text_vnode;
	char *buf;
	int buf_size;
	int text_len;
};

static int start_visit_for_longest_text(html_vnode_t *vnode, void *result)
{
	if (vnode->hpNode->html_tag.tag_type != TAG_PURETEXT)
	{
		return VISIT_NORMAL;
	}
	longest_text_visit_t *visit = (longest_text_visit_t*) result;
	if (vnode->isValid && vnode->textSize > visit->text_len)
	{
		if (visit->last_text_vnode == NULL)
		{
			visit->last_text_vnode = vnode;
		}
		else
		{
			visit->last2_text_vnode = visit->last_text_vnode;
			visit->last_text_vnode = vnode;
		}
		copy_html_text(visit->buf, 0, visit->buf_size - 1, vnode->hpNode->html_tag.text);
		visit->text_len = vnode->textSize;
	}
	return VISIT_NORMAL;
}

static int extract_all_content_on_marked_area(char *cont, const int size, const html_area_t *area)
{
	int avail = 0;
	cont[0] = '\0';

	vnode_list_t *vlist = area->baseinfo->text_info.cont_vnode_list_begin;
	for (; vlist; vlist = vlist->next)
	{
		html_vnode_t *vnode = vlist->vnode;
		if (vnode->isValid && vnode->textSize > 0)
		{
			avail = copy_html_text(cont, avail, size - 1, vnode->hpNode->html_tag.text);
			if (avail >= size - 1)
				break;
		}
		if (vlist == area->baseinfo->text_info.cont_vnode_list_end)
			break;
	}

	return avail;
}

static int remove_tail_left_bracket(char* src, int len)
{
	if (src == NULL || len <= 0)
		return len;
	char* end = src + len -1;
	static const char* bj_bracket[] = { "(" , "[" , 0 };
	static const char* fj_bracket[] = { "（" , "［" , 0 };

	for (int i = 0; bj_bracket[i]; i++)
	{
		if (*end == *bj_bracket[i])
		{
			*end = 0;
			return len - 1;
		}
	}
	if (len >= 2)
	{
		for (int i = 0; fj_bracket[i]; i++)
		{
			if (strncmp(end - 1, fj_bracket[i], 2) == 0)
			{
				*end = 0;
				*(end - 1) = 0;
				return len - 2;
			}
		}
	}
	return len;
}

/**
 * @brief 从分块中提取内容. 要求已完成分块标注.
 **/
static int extract_content_on_marked_area(char *cont, const int size, const html_area_t *area, bool isrealtile = false)
{
	bool hasH1Flag = false;
	html_vnode_t *target = NULL;
	if (isrealtile)
	{
		if (area->begin == area->end && area->valid_subArea_num == 0)
		{
			html_vnode_t *vnode = area->begin;
			while (vnode)
			{
				if (vnode->hpNode->html_tag.tag_type == TAG_H1)
				{
					hasH1Flag = true;
					target = vnode;
					break;
				}
				if (!vnode->firstChild || vnode->firstChild->nextNode != NULL)
				{
					target = vnode;
					break;
				}
				vnode = vnode->firstChild;
			}
		}
	}
	int avail = 0;
	cont[0] = '\0';
	int savedLength = 0;
	if (!isrealtile || is_func_area(area, AREA_FUNC_MYPOS) || target == NULL)
	{
		vnode_list_t *vlist = area->baseinfo->text_info.cont_vnode_list_begin;
		for (; vlist; vlist = vlist->next)
		{
			html_vnode_t *vnode = vlist->vnode;
			if (vnode->isValid && vnode->textSize > 0)
			{
				if (isrealtile && is_func_area(area, AREA_FUNC_MYPOS))
				{
					avail = 0;
				}
				else if (isrealtile && hasH1Flag)
				{
					if (vnode->textSize > savedLength)
					{
						avail = 0;
						savedLength = vnode->textSize;
					}
					else
					{
						continue;
					}
				}
				avail = copy_html_text(cont, avail, size - 1, vnode->hpNode->html_tag.text);
				char *pcopypos = strrchr(cont, '>');
				if (pcopypos != NULL)
				{
					pcopypos++;
					char *pcopydest = cont;
					while (*pcopypos)
					{
						*pcopydest++ = *pcopypos++;
					}
					*pcopydest = 0;
				}
				if (avail >= size - 1)
				{
					break;
				}
			}
			if (vlist == area->baseinfo->text_info.cont_vnode_list_end)
				break;
		}
	}
	else
	{
		longest_text_visit_t longest_text_visit;
		longest_text_visit.buf = cont;
		longest_text_visit.buf_size = size;
		longest_text_visit.text_len = 0;
		longest_text_visit.last2_text_vnode = NULL;
		longest_text_visit.last_text_vnode = NULL;
		html_vnode_visit(target, start_visit_for_longest_text, NULL, &longest_text_visit);
		avail = longest_text_visit.text_len;
		if (longest_text_visit.last2_text_vnode && longest_text_visit.text_len > 2 && (*cont == ':' || strncmp(cont, "：", strlen("：")) == 0))
		{
			avail = copy_html_text(cont, 0, size - 1, longest_text_visit.last2_text_vnode->hpNode->html_tag.text);
			avail = copy_html_text(cont, avail, size - 1, longest_text_visit.last_text_vnode->hpNode->html_tag.text);
		}
		avail = remove_tail_left_bracket(cont, avail);
	}
	return avail;
}

/**
 * @brief 将分块列表的content提取.
 **/
static void get_area_list_content(char *buf, int size, const area_list_t *alist, bool isrealtile = false)
{
	buf[0] = '\0';

	int avail = 0;
	int left_size = size;

	if (alist)
	{
		for (area_list_node_t *ln = alist->head; ln; ln = ln->next)
		{
			html_area_t *area = ln->area;

			if (avail > 0 && avail < size - 1)
			{
				buf[avail] = '\t';

				avail++;
				left_size--;

				buf[avail] = '\0';
			}

			int len = extract_content_on_marked_area(buf + avail, left_size, area, isrealtile);
			if (len > 0)
			{
				break;
			}
			avail += len;
			left_size -= len;

			if (ln == alist->tail)
				break;
		}
	}
}

/**
 * @brief 将所有分块列表的content提取.
 **/
static void get_all_area_list_content(char *buf, int size, const area_list_t *alist)
{
	buf[0] = '\0';

	int avail = 0;
	int left_size = size;

	if (alist)
	{
		for (area_list_node_t *ln = alist->head; ln; ln = ln->next)
		{
			html_area_t *area = ln->area;

			if (avail > 0 && avail < size - 1)
			{
				buf[avail] = '\n';

				avail++;
				left_size--;

				buf[avail] = '\0';
			}
			if (avail > 0 && avail < size - 1)
			{
				buf[avail] = '\n';

				avail++;
				left_size--;

				buf[avail] = '\0';
			}

			int len = extract_all_content_on_marked_area(buf + avail, left_size, area);
			avail += len;
			left_size -= len;

			if (ln == alist->tail)
				break;
		}
	}
}

/**
 * @brief 获取某资源类型分块的内容.
 **/
char *get_area_content(char *buf, int size, area_tree_t *atree, html_area_srctype_t srctype)
{
	assert(IS_MARKED_SRCTYPE(atree->mark_status));
	const area_list_t *alist = get_srctype_mark_result(atree, srctype);
	get_area_list_content(buf, size, alist);
	return buf;
}

/**
 * @brief 获取某功能类型的分块的内容.
 **/
char *get_area_content(char *buf, int size, area_tree_t *atree, html_area_func_t func)
{
	assert(IS_MARKED_FUNC(atree->mark_status));
	const area_list_t *alist = get_func_mark_result(atree, func);
	get_area_list_content(buf, size, alist);
	return buf;
}

/**
 * @brief 获取某语义类型的分块的内容.
 **/
char *get_area_content(char *buf, int size, area_tree_t *atree, html_area_sem_t sem)
{
	assert(IS_MARKED_SEM(atree->mark_status));
	const area_list_t *alist = get_sem_mark_result(atree, sem);
	bool isrealtile = false;
	if (sem == AREA_SEM_REALTITLE)
	{
		isrealtile = true;
	}
	get_area_list_content(buf, size, alist, isrealtile);
	return buf;
}

/**
 * 添加换行信息
 */
int addBreakInfo(char *buffer, int available, int end, const char *break_info)
{
	while (available - 1 >= 0 && isspace(buffer[available - 1]))
	{
		available--;
	}
	if (available == 0)
	{ // when first in buffer, do not render break
		return available;
	}
	while (*break_info != '\0')
	{
		if (available == end)
		{
			break;
		}
		buffer[available++] = *break_info++;
	}
	buffer[available] = '\0';
	return available;
}

int createTree(html_tree_t *&html_tree, vtree_in_t *&vtree_in, html_vtree_t *&vtree, area_tree_t * &atree)
{
	//char *pret = resultbuf;
	html_tree = html_tree_create(MAX_PAGE_SIZE);
	if (!html_tree)
	{

		printf("html_tree_create fail\n");
		return -1;
	}

	//创建V树
	vtree_in = vtree_in_create();
	if (!vtree_in)
	{

		printf("vtree_in_create fail\n");
		return -1;
	}
	vtree = html_vtree_create_with_tree(html_tree);
	if (!vtree)
	{

		printf("html_vtree_create_with_tree fail\n");
		return -1;
	}
	//创建A树
	atree = area_tree_create(NULL);
	if (!atree)
	{

		printf("area_tree_create fail\n");
		return -1;
	}

	return 0;
}

int destroyTree(html_tree_t *&html_tree, vtree_in_t *&vtree_in, html_vtree_t *&vtree, area_tree_t * &atree)
{

	/************下面是资源释放************/
	//销毁A树
	if (atree)
	{
		mark_tree_destory(atree);
		atree = NULL;
	}
	//销毁V树
	if (vtree)
	{
		html_vtree_del(vtree);
		vtree = NULL;
	}
	if (vtree_in)
	{
		vtree_in_destroy(vtree_in);
		vtree_in = NULL;
	}
	//销毁DOM树
	if (html_tree)
	{
		html_tree_del(html_tree);
		html_tree = NULL;
	}
	return 0;
}

int resetTree(html_tree_t *&html_tree, vtree_in_t *&vtree_in, html_vtree_t *&vtree, area_tree_t * &atree)
{

	/************下面是资源复位************/

	{
		//resetcount++;
		if (atree)
		{
			mark_tree_reset(atree);

		}

		if (vtree)
		{
			html_vtree_reset(vtree);

		}
		if (vtree_in)
		{
			vtree_in_reset(vtree_in);
		}

		if (html_tree)
		{
			html_tree_reset_no_destroy((struct html_tree_impl_t*) html_tree);

		}
	}

	return 0;
}

/**
 * @brief 获取某资源类型所有分块的内容.
 **/
char *get_all_area_content(char *buf, int size, area_tree_t *atree, html_area_srctype_t srctype)
{
	assert(IS_MARKED_SRCTYPE(atree->mark_status));
	const area_list_t *alist = get_srctype_mark_result(atree, srctype);
	get_all_area_list_content(buf, size, alist);
	return buf;
}

/**
 * @brief 获取某功能类型的所有分块的内容.
 **/
char *get_all_area_content(char *buf, int size, area_tree_t *atree, html_area_func_t func)
{
	assert(IS_MARKED_FUNC(atree->mark_status));
	const area_list_t *alist = get_func_mark_result(atree, func);
	get_all_area_list_content(buf, size, alist);
	return buf;
}

/**
 * @brief 获取某语义类型的所有分块的内容.
 **/
char *get_all_area_content(char *buf, int size, area_tree_t *atree, html_area_sem_t sem)
{
	assert(IS_MARKED_SEM(atree->mark_status));
	const area_list_t *alist = get_sem_mark_result(atree, sem);
	get_all_area_list_content(buf, size, alist);
	return buf;
}

static int visit_atree_links(html_area_t *area, void *data)
{
	if (!area->isValid)
	{
		return AREA_VISIT_SKIP;
	}

	if (area->valid_subArea_num > 0)
	{
		return AREA_VISIT_NORMAL;
	}
	LinksOfATree *links_data = (LinksOfATree *) data;

	if (links_data->available > links_data->end)
	{
		return AREA_VISIT_FINISH;
	}
	//printf("get anchor num=%d,size=%d\n",links_data->available,links_data->end);
	vnode_list_t *vlist = area->baseinfo->link_info.url_vnode_list_begin;
//	if(vlist&&vlist->vnode){
//		printf("area %d has link\n",area->no);
//	}
	for (; vlist; vlist = vlist->next)
	{
		html_vnode_t *vnode = vlist->vnode;
		//if(vnode){
		//printf("node type=%d,node name=%s\n",vnode->hpNode->html_tag.tag_type,vnode->hpNode->html_tag.tag_name);}
		if (vnode && vnode->hpNode->html_tag.tag_type == TAG_A)
		{
			char *src = get_attribute_value(&(vnode->hpNode->html_tag), ATTR_HREF);
			//printf("url=%s\n",src);
			if (src && strlen(src))
			{
				if (html_combine_url(links_data->links[links_data->available].url, src, links_data->base_domain, links_data->base_path, links_data->base_port) < 0)
				{

					continue;
				}

				if (strlen(links_data->links[links_data->available].url))
				{
					html_node_extract_content(vnode->hpNode, links_data->links[links_data->available].text, sizeof(links_data->links[links_data->available].text));

				}
				if (is_func_area(area, AREA_FUNC_COPYRIGHT))
				{
					SET_LINK_COPYRIGHT(links_data->links[links_data->available].linktype);
				}
				if (is_in_func_area(area, AREA_FUNC_NAV))
				{
					SET_LINK_NAV(links_data->links[links_data->available].linktype);
				}
				if (is_in_func_area(area, AREA_FUNC_FRIEND))
				{
					SET_LINK_FRIEND(links_data->links[links_data->available].linktype);
				}
				if (is_in_func_area(area, AREA_FUNC_RELATE_LINK))
				{
					SET_LINK_RELATE_LINK(links_data->links[links_data->available].linktype);
				}
				if (is_in_func_area(area, AREA_FUNC_MYPOS))
				{
					SET_LINK_MYPOS(links_data->links[links_data->available].linktype);
				}
				if (is_in_func_area(area, AREA_FUNC_ARTICLE_SIGN))
				{
					SET_LINK_ARTICLE_SIGN(links_data->links[links_data->available].linktype);
				}

				links_data->available++;
				//printf("get anchor num=%d\n",links_data->available);
				if (links_data->available > links_data->end)
				{
					return AREA_VISIT_FINISH;
				}
			}
		}
		if (vlist == area->baseinfo->link_info.url_vnode_list_end)
			break;
	}
	return AREA_VISIT_NORMAL;

}
int getAnchorInfos(area_tree_t *atree, anchor_info * anchors, int anchorsize, const char *baseUrl)
{
	if (NULL == atree || NULL == anchors || anchorsize < 1 || NULL == baseUrl)
	{
		return -1;
	}
	LinksOfATree atreelinks;
	atreelinks.area_tree = atree;
	atreelinks.available = 0;
	atreelinks.end = anchorsize - 1;
	atreelinks.baseurl = baseUrl;
	atreelinks.links = anchors;
	if (!easou_parse_url(baseUrl, atreelinks.base_domain, atreelinks.base_port, atreelinks.base_path))
	{
		return -1;
	}
	areatree_visit(atree->root, visit_atree_links, NULL, &atreelinks);
	return atreelinks.available;
}

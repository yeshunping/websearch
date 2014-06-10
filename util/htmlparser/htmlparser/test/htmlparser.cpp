#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "log.h"
#include "easou_html_tree.h"

using namespace EA_COMMON;

#define TIMEDELTA_MS(time1,time0) ((time1.tv_sec-time0.tv_sec)*1000+(time1.tv_usec-time0.tv_usec)/1000)


html_tree_t *m_pDTree; //dom tree

void PrintNode(html_node_t *html_node, int level, FILE *fp)
{
	html_attribute_t *attribute;
	html_node_t *child;

	if (level != 0)
	{
		fprintf(fp, " ");
	}
	for (int i = 0; i < level - 1; i++)
	{
		fprintf(fp, "  ");
	}

	if (html_node->html_tag.tag_type != TAG_PURETEXT && html_node->parent != NULL)
	{
		if (html_node->html_tag.tag_type == TAG_DOCTYPE)
		{
			fprintf(fp, "<!DOCTYPE %s tagCode=\"%d\"", html_node->html_tag.text, html_node->html_tag.tag_code);
		}
		else if (html_node->html_tag.tag_type == TAG_COMMENT)
		{
			fprintf(fp, "<!-- %s tagCode=\"%d\" -->", html_node->html_tag.text, html_node->html_tag.tag_code);
		}
		else
		{
			if (html_node->html_tag.tag_name)
			{
				fprintf(fp, "<%s tagCode=\"%d\"", html_node->html_tag.tag_name, html_node->html_tag.tag_code);
			}
			for (attribute = html_node->html_tag.attribute; attribute != NULL; attribute = attribute->next)
			{
				fprintf(fp, " %s", attribute->name);
				if (attribute->value != NULL)
				{
					fprintf(fp, "=\"%s\"", attribute->value);
				}
			}
			fprintf(fp, ">\n");
		}
	}

	if (html_node->html_tag.tag_type == TAG_SCRIPT || html_node->html_tag.tag_type == TAG_STYLE
			|| html_node->html_tag.tag_type == TAG_PURETEXT)
	{
		fprintf(fp, "%s\n", html_node->html_tag.text);
	}
	else
	{
		for (child = html_node->child; child != NULL; child = child->next)
		{
			PrintNode(child, level + 1, fp);
		}
	}
	if (html_node->html_tag.tag_type != TAG_PURETEXT && html_node->parent != NULL
			&& html_node->html_tag.tag_type != TAG_COMMENT && html_node->html_tag.tag_type != TAG_DOCTYPE
			&& !html_node->html_tag.is_self_closed)
	{
		for (int i = 0; i < level - 1; i++)
		{
			fprintf(fp, "  ");
		}
		fprintf(fp, "</%s>\n", html_node->html_tag.tag_name);
	}
}

void Release()
{
	if (m_pDTree)
	{
		html_tree_del(m_pDTree);
		m_pDTree = NULL;
	}
}

int ParseTree(char *url, char *page, int page_len)
{
	int ret = html_tree_parse(m_pDTree, page, page_len);
	if (ret != 1)
	{
		printf("html_tree_parse fail, url:%s\n", url);
		return -1;
	}
	return 1;
}

int ResetTree()
{
	if (m_pDTree)
	{
		html_tree_reset_no_destroy((struct html_tree_impl_t*) m_pDTree);
	}
	return 1;
}

int CreateTree()
{
	m_pDTree = html_tree_create(MAX_PAGE_SIZE);
	if (m_pDTree == NULL)
	{
		printf("html_tree_create fail\n");
		return -1;
	}
	return 1;
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		printf("%s filePath url\n", argv[0]);
		return -1;
	}
	//初始化日志
	if (!Init_Log("log.conf"))
	{
		printf("log.conf not exist\n");
		return 0;
	}

	char *filePath = argv[1];
	char *url = argv[2];

	char pageBuf[128000];
	FILE *fp = fopen(filePath, "r");
	int pageLen = fread(pageBuf, 1, 128000 - 1, fp);
	fclose(fp);
	fp = NULL;

	if (1 != CreateTree())
	{
		printf("create tree fail\n");
		return -1;
	}

	timeval endtv1, endtv2;
	gettimeofday(&endtv1, NULL);

	ResetTree();
	if (1 != ParseTree(url, pageBuf, pageLen))
	{
		printf("parse tree fail\n");
		return -1;
	}
	else
		printf("parse tree success\n");

	gettimeofday(&endtv2, NULL);
	int millis = TIMEDELTA_MS(endtv2, endtv1);

	printf("millis:%d\n", millis);

	fp = fopen("result.html", "w");
	if (fp != NULL)
	{
		PrintNode(&m_pDTree->root, 0, fp);
		fclose(fp);
		fp = NULL;
	}
	return 0;
}

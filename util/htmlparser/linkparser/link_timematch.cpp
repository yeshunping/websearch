/***************************************************************************
 *
 * Copyright (c) 2012 Easou.com, Inc. All Rights Reserved
 * $Id: easou_link_timematch.cpp,v 1.0 2012/09/01 pageparse Exp $
 *
 **************************************************************************/

#include "log.h"
#include "easou_link_timematch.h"

#include <assert.h>
#include <string.h>

using namespace EA_COMMON;

static const int g_regex_num = 5;
static const char * g_date_pattern[] =
		{ "([0-9]{1,4})[-年月日/.]{1,2}([0-9]{1,2})[-年月日/.]{1,2}([0-9]{1,2})",
				"([jJ]an|[fF]eb|[mM]ar|[aA]pr|[mM]ay|[jJ]un|[jJ]ul|[aA]ug|[sS]ep|[oO]ct|[nN]ov|[dD]ec)[a-zA-Z]{0,6}[^0-9a-zA-Z]{1,2}([0-9]{1,2})[a-zA-Z]{0,2}[^0-9a-zA-Z]{1,2}([0-9]{2,4})",
				"([0-9]{1,2})[^0-9a-zA-Z]{1,2}(jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec)[a-zA-Z]{0,6}[^0-9a-zA-Z]{1,2}([0-9]{2,4})",
				"([0-9]{1,4})[-年/.]{1,2}([0-9]{1,2})[-月/.]{1,2}([0-9]{1,2})[^0-9a-zA-Z]{1,3}([0-9]{1,2})[^0-9a-zA-Z]{1,2}([0-9]{1,2})",
				"(一|二|三|四|五|六|七|八|九|十|十一|十二)月[^0-9a-zA-Z]{1,2}([0-9]{1,2})[a-zA-Z]{0,2}[^0-9a-zA-Z]{1,2}([0-9]{2,4})" };
static const int g_max_vector = 10;

timematch_pack_t * timematch_pack_create()
{
	const char *where = "timematch_pack_create()";
	const char *error = NULL;
	int erroffset = 0;

	timematch_pack_t *ppack;
	ppack = (timematch_pack_t*) calloc(1, sizeof(timematch_pack_t));
	if (!ppack)
		goto failed;

	ppack->date_re = (pcre **) calloc(g_regex_num, sizeof(pcre*));
	if (!ppack->date_re)
		goto failed;

	ppack->date_pe = (pcre_extra **) calloc(g_regex_num, sizeof(pcre_extra*));
	if (!ppack->date_pe)
		goto failed;

	for (int i = 0; i < g_regex_num; i++)
	{
		ppack->date_re[i] = pcre_compile(g_date_pattern[i], PCRE_CASELESS | PCRE_EXTENDED, &error, &erroffset, NULL);
		if (!ppack->date_re[i])
		{
			Error("%s: PCRE compil date failed at offset %d: %s\n", where, erroffset, error);
			goto failed;
		}
		ppack->date_pe[i] = pcre_study(ppack->date_re[i], 0, &error);
	}
	return ppack;

	failed: if (ppack)
	{
		if (ppack->date_re)
		{
			for (int i = 0; i < g_regex_num; i++)
			{
				if (ppack->date_re[i])
					(*pcre_free)(ppack->date_re[i]);
			}
			free(ppack->date_re);
			ppack->date_re = NULL;
		}
		if (ppack->date_pe)
		{
			for (int i = 0; i < g_regex_num; i++)
			{
				if (ppack->date_pe[i])
					(*pcre_free)(ppack->date_pe[i]);
			}
			free(ppack->date_pe);
			ppack->date_pe = NULL;
		}
		free(ppack);
		ppack = NULL;
	}
	return NULL;
}

void timematch_pack_del(timematch_pack_t *ppack)
{
	if (ppack)
	{
		if (ppack->date_re)
		{
			for (int i = 0; i < g_regex_num; i++)
			{
				if (ppack->date_re[i])
					(*pcre_free)(ppack->date_re[i]);
			}
			free(ppack->date_re);
			ppack->date_re = NULL;
		}
		if (ppack->date_pe)
		{
			for (int i = 0; i < g_regex_num; i++)
			{
				if (ppack->date_pe[i])
					(*pcre_free)(ppack->date_pe[i]);
			}
			free(ppack->date_pe);
			ppack->date_pe = NULL;
		}
		free(ppack);
		ppack = NULL;
	}
}

int hastime(timematch_pack_t *ppack, char *text, int bbs)
{
	assert(ppack && text);
	const char *where = "hastime()";
	int rc = 0;
	int ovector[g_max_vector];
	int istime = 0;

	for (int i = 0; i < g_regex_num; i++)
	{
		if (bbs) //bbs只使用第4个模板
			i = 3;
		rc = pcre_exec(ppack->date_re[i], ppack->date_pe[i], text, strlen(text), 0, 0, ovector, g_max_vector);

		if (rc == PCRE_ERROR_NOMATCH)
			continue;
		else if (rc < 0)
		{
			Error("%s: date pattern %d match error[%d].", where, i, rc);
			continue;
		}
		else
		{
			istime = 1;
			break;
		}
	}
	return istime;
}

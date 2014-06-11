/*
 * debuginfo.h
 *
 *  Created on: 2012-4-1
 *      Author: shuangwei
 */

#ifndef DEBUGINFO_H_
#define DEBUGINFO_H_
#include "stdio.h"
#include "stdlib.h"
extern int g_EASOU_DEBUG ;
extern FILE *g_logfile;
extern char * g_debugbuf;
extern int g_debugbuflen;
#define DEBUG_MEMERY 1
#define VHTMLTREE   2
#define HTMLTREE    3
#define MARKTREE   4
#define MARK_SRCTYPE 5
#define MARK_REALTITLE 6
#define MARK_CENTRAL   7
#define MARK_COPYRIGHT 8
#define MARK_MYPOS    9
#define MARK_NAV      10
#define MARK_RELATELINK 11
#define MARK_SUBTITFORRT   12
#define MARK_TIME   13
#define MARK_ARTICLESIGN 14
#define MARK_FRIEND 15
#define MARK_SUBTITLE 16
#define MARK_SRC_LINK 17
#define MARK_SRC_HUB 18
#define MARK_CANDI_SUBTIT_4RT   19
#define DIVIDING_AREA    20
#define PRINTF_AREA_TITLE 21
#define MARK_AREA_TITLE 22
#define PRINT_CSS 23
#define MARK_HUB_PAGE 24
#define MARK_TURNPAGE 25
#define MARK_POS 26
#define CALC_VTREE 27
#define MARK_INTERACTION 28
#define MARK_PIC 29
#define MARK_AC 30 //文章文内容块
#define MARK_HUB_CENTRAL   31
#define ALL_REALTITLE 106
#define ALL_CENTRAL   107
#define ALL_COPYRIGHT 108
#define ALL_MYPOS    109
#define ALL_NAV      110
#define ALL_RELATE_LINK 111
#define ALL_SUBTITFORRT   112
#define ALL_TIME   113
#define ALL_ARTICLE_SIGN 114
#define ALL_FRIEND 115
#define ALL_SUBTITLE 116
#define ALL_SRC_LINK 117
#define ALL_SRC_HUB 118
#define ALL_CANDI_SUBTIT_4RT 119
#define ALL_SRC_HUB_SUBUNIT 120
#define PT_FEATURE_PIC 121

#define easouprintf(info){\
	  if(g_EASOU_DEBUG==DEBUG_MEMERY&&info!=NULL&&g_debugbuf&&g_debugbuflen>2){\
		 int len= snprintf(g_debugbuf,g_debugbuflen,"easou print:%s at %s(%d)-%s\r\n",info,__FILE__,__LINE__,__FUNCTION__);\
		 g_debugbuf=g_debugbuf+len;g_debugbuflen=g_debugbuflen-len;\
	  }\
  }

#define marktreeprintfs(type,info,...){\
	  if(g_EASOU_DEBUG==type&&info!=NULL&&g_debugbuf&&g_debugbuflen>2){\
		  int len= snprintf(g_debugbuf,g_debugbuflen,info,##__VA_ARGS__);\
		  g_debugbuf=g_debugbuf+len;g_debugbuflen=g_debugbuflen-len;\
	  }\
  }

#define marktreeprintftofile(type,info,...){\
	  if(g_EASOU_DEBUG==type&&info!=NULL&&g_logfile!=NULL){\
		  fprintf(g_logfile,info,##__VA_ARGS__);\
	  }\
  }

#define myprintf(info,...){\
	  if(info!=NULL&&g_debugbuf&&g_debugbuflen>2){\
		  int len= snprintf(g_debugbuf,g_debugbuflen,info,##__VA_ARGS__);\
		  g_debugbuf=g_debugbuf+len;g_debugbuflen=g_debugbuflen-len;\
	  }\
  }
#define printline(){\
	if(g_debugbuf&&g_debugbuflen>2){\
	   int len= snprintf(g_debugbuf,g_debugbuflen,"-------------------------------\r\n");\
	   g_debugbuf=g_debugbuf+len;g_debugbuflen=g_debugbuflen-len;\
	}\
  }
#define printlines(info){\
		{\
				int len= snprintf(g_debugbuf,g_debugbuflen,"---------------%s----------------\r\n",info);\
				g_debugbuf=g_debugbuf+len;g_debugbuflen=g_debugbuflen-len;\
		}\
   }

#endif /* DEBUGINFO_H_ */

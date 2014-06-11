/*
 * url.cpp
 *
 *  Created on: 2011-11-21
 *      Author: ddt
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "string_util.h"
#include "url.h"

/*
 * @breif 是否是绝对url
 */
int is_url(const char *url)
{
	if (strncasecmp(url, "http://", 7) ==0 || strncasecmp(url, "https://", 8) ==0)
	{
		return 1;
	}
	return 0;
}

int is_dotip(const char *sitename);
int is_ignore(const char* slip);

const char* fetch_maindomain_l(const char* site, char *domain, int size)
{
	const char* pfirst;
	const char* ptail;
	int len;
	int tailLen = 0;
	if (is_dotip(site))
	{
		strncpy(domain, site, size - 1);
		domain[size - 1] = 0;
		return site;
	}

	ptail = strrchr(site, '.');
	if (ptail == NULL)
	{
		goto end;
	}
	tailLen = strlen(ptail);
	pfirst = ptail - 1;
	while (pfirst >= site - 1)
	{
		if ((pfirst == site - 1) || (*pfirst == '.'))
		{
			if (is_ignore(pfirst + 1) == 0)
			{
				len = size - 1 > ptail - pfirst - 1 + tailLen ? ptail - pfirst - 1 + tailLen : size - 1;
				memcpy(domain, pfirst + 1, len);
				domain[len] = 0;
				return pfirst + 1;
			}
			else
			{
				ptail = pfirst;
			}
		}
		pfirst--;
	}
	end: strncpy(domain, site, size - 1);
	domain[size - 1] = 0;
	return site;
}

const char*  fetch_maindomain(const char* site,char *domain,int size) {
    const char* pfirst;
    const char* ptail;
    int len;
    if(is_dotip(site)) {
        strncpy(domain,site,size-1);
        domain[size-1]=0;
        return site;
    }


    ptail=strrchr(site,'.');
    if(ptail == NULL) {
        goto end;
    }
    pfirst=ptail-1;
    while(pfirst >=site-1) {
        if((pfirst == site - 1 ) || (*pfirst=='.')) {
            if(is_ignore(pfirst+1)==0) {
                len=size-1>ptail-pfirst-1?ptail-pfirst-1:size-1;
                memcpy(domain,pfirst+1,len);
                domain[len]=0;
                return pfirst+1;
            } else {
                ptail=pfirst;
            }
        }
        pfirst--;
    }
end:
    strncpy(domain,site,size-1);
    domain[size-1]=0;
    return site;
}

const char * fetch_maindomain_from_url(const char * url , char * domain , int size )
{
	char site[MAX_URL_LEN];
	site[0] = '\0';
	if(parse_url(url , site , NULL ,NULL)==0){
		return NULL ;
	}
	return fetch_maindomain(site ,domain , size ) ;
}

const char * fetch_domain_l_from_url(const char * url, char * domain, int size)
{
	char site[MAX_URL_LEN];
	site[0] = '\0';
	if(parse_url(url , site , NULL ,NULL)==0){
		return NULL ;
	}
	return fetch_maindomain_l(site ,domain , size ) ;
}

typedef struct _url_protocol_t
{
	const char *pro_name;
	const int len;
}url_protocol_t;

static const url_protocol_t url_pro= {"http://" ,7 } ;

/**
 * @brief 判断href的类型：站内，站外，js（ERR）
 **/
int get_href_type(const char *phref, const char *base_url)
{
	if (phref == NULL)
	{
		return IS_LINK_ERR;
	}
	if (strncasecmp(phref, url_pro.pro_name, url_pro.len) != 0)
	{ // not "http://" means inner link
		if (strncasecmp(phref, "javascript:", 11) != 0)
		{
			return IS_LINK_INNER;
		}
		else
		{ //javascript:
			return IS_LINK_ERR;
		}
	}

	char base_main_buff[MAX_SITE_LEN];
	base_main_buff[0] = '\0';
	char this_main_buff[MAX_SITE_LEN];
	this_main_buff[0] = '\0';
	fetch_maindomain_from_url(base_url, base_main_buff, sizeof(base_main_buff));
	fetch_maindomain_from_url(phref, this_main_buff, sizeof(this_main_buff));
	if (base_main_buff[0] != '\0' && this_main_buff[0] != '\0')
	{
		if (strcasecmp(base_main_buff, this_main_buff) == 0)
		{
			return IS_LINK_INNER;
		}
		else
		{
			return IS_LINK_OUT;
		}
	}
	else
	{
		return IS_LINK_ERR;
	}
}

static char EASOU_PCHAR[256] = {    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,
                                 1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
                            };

//uchar | reserved
//add ' ' '"' '<' '>' 32 34 60 62
static char EASOU_UCH_RES[256] = {    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
                              };

//pchar | "/"
//add ' ' '"' '<' '>' 32 34 60 62
static char EASOU_PARAM[256] = {    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
                            };

//rfc2396
//define following chars:
// '$' '%' '&' '+' ',' '/' ':' ';' '=' '?' '@'
static char EASOU_RESERVED[256] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 0,0,0,0,1,1,1,0,0,0,0,1,1,0,0,1,
                                 0,0,0,0,0,0,0,0,0,0,1,1,0,1,0,1,
                                 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
                               };

// '\r' '\n'
static char EASOU_LFCR[256] = {
    0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};


/**
 * @brief 移除\r、\n
 **/
static int delete_inter(char * str)
{
	if (NULL == str)
	{
		return 0;
	}
	size_t len = strcspn(str, "\r\n");
	char *p1 = str + len;
	char *p2 = str + len;
	while (*p2 != '\0')
	{
		if (EASOU_LFCR[(unsigned char) *p2])
		{
			p2++;
		}
		else
		{
			*p1++ = *p2++;
		}
	}
	if (*p1 != '\0')
	{
		*p1 = '\0';
	}
	return p1 - str;
}

static int space_escape(char * url,int max_url_len, int max_site_len, int max_path_len) {
    char * pin = url ;
    int  pout = 0 ;
    char url_tmp[MAX_URL_LEN] ;
    while(*pin!='\0') {
        if(*pin!=' ') {
            if(pout>=max_url_len -1){
                return 0 ;
            }
            url_tmp[pout++] = *pin++ ;
        } else { //space
            if(pout>=max_url_len-3){
                return 0 ;
            }
            url_tmp[pout++]='%' ;
            url_tmp[pout++]='2' ;
            url_tmp[pout++]='0' ;
            pin++ ;
        }
    }
    if(pout>=max_url_len)
        return 0 ;
    url_tmp[pout]='\0' ;
    //detect site len and path len
    if(strncmp(url_tmp,"http://",7)==0) {
        pin = url_tmp+7 ;
    } else {
        pin = url_tmp ;
    }
    char * path = strchr(pin,'/') ;
    if(path==pin)
        return 0 ;
    if(path!=NULL && strlen(path)>=(u_int)max_path_len)
        return 0 ;
    char * port = strchr(pin,':') ;
    char * psite_end = NULL ;
    if(port==pin)
        return 0 ;
    if(NULL!=port&&NULL!=path&&port<path) {
        psite_end=port-1 ;
    } else if(path!=NULL) {
        psite_end = path -1 ;
    }
    if(psite_end!=NULL && psite_end-pin>=max_site_len) {
        return 0 ;
    } else if (psite_end==NULL && strlen(pin) >= (u_int)max_site_len) {
        return 0 ;
    }
    strncpy(url,url_tmp,pout+1) ;
    return 1 ;
}
static int space_escape_path(char * path,int max_path_len) {
    char * pin = path ;
    int  pout = 0 ;
    char url_tmp[MAX_URL_LEN] ;
    while(*pin!='\0') {
        if(*pin!=' ') {
            if(pout>=max_path_len -1)
                return 0 ;
            url_tmp[pout++] = *pin++ ;
        } else { //space
            if(pout>=max_path_len-3)
                return 0 ;
            url_tmp[pout++]='%' ;
            url_tmp[pout++]='2' ;
            url_tmp[pout++]='0' ;
            pin++ ;
        }
    }
    if(pout>=max_path_len)
        return 0 ;
    url_tmp[pout]='\0' ;

    strncpy(path,url_tmp,pout+1) ;
    return 1 ;
}
// {{{ to_char

//translate the HEX to DEC
//return the DEC
//A-F:65-70   a-f:97-102 ->10-15 '0'-'9':48-57 -> 0-9
static char EASOU_TO_CHAR[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
    0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
int to_char(char hex[2]) {

    int r;
    int h[2];
    if((EASOU_TO_CHAR[(unsigned char)hex[0]]==0 && hex[0]!=48) || (EASOU_TO_CHAR[(unsigned char)hex[1]]==0 && hex[1]!=48))
        return 0;
    h[0] = EASOU_TO_CHAR[(unsigned char)hex[0]];
    h[1] = EASOU_TO_CHAR[(unsigned char)hex[1]];
    r = 16*h[0]+h[1];
    return r;
}

static int single_path_nointer(char *path) {
    char *pin;
    int   pout;
    char  hex[2];
    char  tmp_hex[2];
    char  escape_char;
    int space_flag = 0;
    pin = path;
    pout = 0;
    //check abs_path
    if(*pin == '/') {
        path[pout++] = *pin ++;
    }

    //path
    while(EASOU_PCHAR[(unsigned char)*pin]) {
        tmp_hex[0] = '\0';
        if((*pin == '%') && (*(pin+1) != '\0') && (*(pin+2) !='\0')) {
            strncpy(hex,++pin,2);
            strncpy(tmp_hex,hex,2);
            pin ++;
            escape_char = to_char(hex);
            if(escape_char == 0) {
                pin--;
                path[pout++]='%';
                continue;
            }
            *pin = escape_char;
        }
        if(EASOU_PCHAR[(unsigned char)*pin] &&
                (tmp_hex[0]=='\0' || !EASOU_RESERVED[(unsigned char)*pin])) {
            if(*pin == ' ') {
                space_flag = 1;
            }
            path[pout++] = *pin++;
        } else {
            path[pout++]='%';
            strncpy(&path[pout],tmp_hex,2);
            pout=pout+2;
            pin++;
        }
    }

    while(*pin == '/') {
        path[pout++] = *pin ++;
        while(EASOU_PCHAR[(unsigned char)*pin]) {
            tmp_hex[0] = '\0';
            if((*pin == '%') && (*(pin+1) != '\0') && (*(pin+2) !='\0')) {
                strncpy(hex,++pin,2);
                strncpy(tmp_hex,hex,2);
                pin ++;
                escape_char = to_char(hex);
                if(escape_char == 0) {
                    pin--;
                    path[pout++]='%';
                    continue;
                }
                *pin = escape_char;
            }

            if(EASOU_PCHAR[(unsigned char)*pin] &&
                    (tmp_hex[0]=='\0' || !EASOU_RESERVED[(unsigned char)*pin])) {
                if(*pin == ' ') {
                    space_flag = 1;
                }
                path[pout++] = *pin++;
            } else {
                path[pout++]='%';
                strncpy(&path[pout],tmp_hex,2);
                pout=pout+2;
                pin++;
            }
        }
    }

    // ";" params  == *(";" param)
    while(*pin == ';') {
        path[pout++] = *pin ++;
        while(EASOU_PARAM[(unsigned char)*pin]) {
            tmp_hex[0] = '\0';
            if((*pin == '%') && (*(pin+1) != '\0') && (*(pin+2) !='\0')) {
                strncpy(hex,++pin,2);
                strncpy(tmp_hex,hex,2);
                pin ++;
                escape_char = to_char(hex);
                if(escape_char == 0) {
                    pin--;
                    path[pout++]='%';
                    continue;
                }
                *pin = escape_char;
            }
            if(EASOU_PARAM[(unsigned char)*pin] &&
                    (tmp_hex[0]=='\0' || !EASOU_RESERVED[(unsigned char)*pin])) {
                if(*pin == ' ') {
                    space_flag = 1;
                }
                path[pout++] = *pin++;
            } else {
                path[pout++]='%';
                strncpy(&path[pout],tmp_hex,2);
                // snprintf(&path[pout],4,"%%%s",tmp_hex);
                pout=pout+2;
                pin++;
            }
        }
    }

    //"?" query
    while(*pin == '?') {
        path[pout++] = *pin ++;
        while(EASOU_UCH_RES[(unsigned char)*pin]) {
            tmp_hex[0] = '\0';
            if((*pin == '%') && (*(pin+1) != '\0') && (*(pin+2) !='\0')) {
                strncpy(hex,++pin,2);
                strncpy(tmp_hex,hex,2);
                pin ++;
                escape_char = to_char(hex);
                if(escape_char == 0) {
                    pin--;
                    path[pout++]='%';
                    continue;
                }
                *pin = escape_char;
            }
            if(EASOU_UCH_RES[(unsigned char)*pin] &&
                    (tmp_hex[0]=='\0' || !EASOU_RESERVED[(unsigned char)*pin])) {
                if(*pin == ' ') {
                    space_flag = 1;
                }
                path[pout++] = *pin++;
            } else {
                path[pout++]='%';
                strncpy(&path[pout],tmp_hex,2);
                pout=pout+2;
                pin++;
            }
        }
    }
    //if(*pin != 0)
    if(*pin != 0 && *pin != '#')
        return 0;
    path[pout] = 0;
    if ( pout > (MAX_PATH_LEN-1) ){ //TODO
#ifdef DEBUG_PARSER
    Debug("[single_path_nointer] - pout length return. pout:%d path:%s",pout,path);
#endif
        return 0;
    }
    if(space_flag==1) {
        return space_escape_path(path,MAX_PATH_LEN);
    } else {
        return 1;
    }
}

/*
 * function : convert the escape squence to original one
 */
int single_path(char *path) {
    delete_inter(path);
    return single_path_nointer(path);
}

static int single_path_ex_nointer(char *path, int len) {
    char *pin;
    int   pout;
    char  hex[2];
    char  tmp_hex[2];
    char  escape_char;
    int space_flag = 0;
    //len = strlen(path);
    pin = path;
    pout = 0;
    //check abs_path
    if(*pin == '/') {
        path[pout++] = *pin ++;
    }

    //path
    while(EASOU_PCHAR[(unsigned char)*pin]) {
        tmp_hex[0] = '\0';
        if((*pin == '%') && (*(pin+1) != '\0') && (*(pin+2) !='\0')) {
            strncpy(hex,++pin,2);
            strncpy(tmp_hex,hex,2);
            pin ++;
            escape_char = to_char(hex);
            if(escape_char == 0) {
                pin--;
                path[pout++]='%';
                continue;
            }
            *pin = escape_char;
        }
        if(EASOU_PCHAR[(unsigned char)*pin] &&
                (tmp_hex[0]=='\0' || !EASOU_RESERVED[(unsigned char)*pin])) {
            if(*pin == ' ') {
                space_flag = 1;
            }
            path[pout++] = *pin++;
        } else {
            path[pout++]='%';
            strncpy(&path[pout],tmp_hex,2);
            pout=pout+2;
            pin++;
        }
    }

    while(*pin == '/') {
        path[pout++] = *pin ++;
        while(EASOU_PCHAR[(unsigned char)*pin]) {
            tmp_hex[0] = '\0';
            if((*pin == '%') && (*(pin+1) != '\0') && (*(pin+2) !='\0')) {
                strncpy(hex,++pin,2);
                strncpy(tmp_hex,hex,2);
                pin ++;
                escape_char = to_char(hex);
                if(escape_char == 0) {
                    pin--;
                    path[pout++]='%';
                    continue;
                }
                *pin = escape_char;
            }

            if(EASOU_PCHAR[(unsigned char)*pin] &&
                    (tmp_hex[0]=='\0' || !EASOU_RESERVED[(unsigned char)*pin])) {
                if(*pin == ' ') {
                    space_flag = 1;
                }
                path[pout++] = *pin++;
            } else {
                path[pout++]='%';
                strncpy(&path[pout],tmp_hex,2);
                pout=pout+2;
                pin++;
            }
        }
    }

    // ";" params  == *(";" param)
    while(*pin == ';') {
        path[pout++] = *pin ++;
        while(EASOU_PARAM[(unsigned char)*pin]) {
            tmp_hex[0] = '\0';
            if((*pin == '%') && (*(pin+1) != '\0') && (*(pin+2) !='\0')) {
                strncpy(hex,++pin,2);
                strncpy(tmp_hex,hex,2);
                pin ++;
                escape_char = to_char(hex);
                if(escape_char == 0) {
                    pin--;
                    path[pout++]='%';
                    continue;
                }
                *pin = escape_char;
            }
            if(EASOU_PARAM[(unsigned char)*pin] &&
                    (tmp_hex[0]=='\0' || !EASOU_RESERVED[(unsigned char)*pin])) {
                if(*pin == ' ') {
                    space_flag = 1;
                }
                path[pout++] = *pin++;
            } else {
                path[pout++]='%';
                strncpy(&path[pout],tmp_hex,2);
                // snprintf(&path[pout],4,"%%%s",tmp_hex);
                pout=pout+2;
                pin++;
            }
        }
    }

    //"?" query
    while(*pin == '?') {
        path[pout++] = *pin ++;
        while(EASOU_UCH_RES[(unsigned char)*pin]) {
            tmp_hex[0] = '\0';
            if((*pin == '%') && (*(pin+1) != '\0') && (*(pin+2) !='\0')) {
                strncpy(hex,++pin,2);
                strncpy(tmp_hex,hex,2);
                pin ++;
                escape_char = to_char(hex);
                if(escape_char == 0) {
                    pin--;
                    path[pout++]='%';
                    continue;
                }
                *pin = escape_char;
            }
            if(EASOU_UCH_RES[(unsigned char)*pin] &&
                    (tmp_hex[0]=='\0' || !EASOU_RESERVED[(unsigned char)*pin])) {
                if(*pin == ' ') {
                    space_flag = 1;
                }
                path[pout++] = *pin++;
            } else {
                path[pout++]='%';
                strncpy(&path[pout],tmp_hex,2);
                pout=pout+2;
                pin++;
            }
        }
    }
    //if(*pin != 0)
    if(*pin != 0 && *pin != '#')
        return 0;
    path[pout] = 0;
    if (pout > len )
        return 0;
    if(space_flag == 1) {
        return space_escape_path(path,MAX_PATH_LEN);;
    } else {
        return 1;
    }
    //check abs_path over
}

int single_path_ex(char *path) {
    int len = delete_inter(path); //delete '\r' '\n';
    return single_path_ex_nointer(path, len);
}

void replace_slash(char *path) {
    char *p, *pend;

    pend = strchr(path, '?');
    if (pend == NULL) {
        pend = strchr(path, ';');
    }
    if (pend == NULL) {
        pend = path + strlen(path);
    }

    for (p = path; p != pend; p++) {
        if (*p == '\\')
            *p = '/';
    }
}


void normalize_path(char *path) {
    char *p, *q;
    char tmp[MAX_URL_LEN];
    char *pend;
    assert(*path == '/');
    replace_slash(path);

    strcpy(tmp, path);

    pend = strchr(tmp, '?');
    if (pend == NULL) {
        pend = strchr(tmp, ';');
    }
    if (pend == NULL) {
        pend = tmp + strlen(tmp);
    }

    p = tmp+1;
    q = path;
    while (p != pend) {
        /* normal */
        if (*q != '/') {
            q++;
            *q = *p++;
            continue;
        }

        /* /./ */
        if (strncmp(p, "./", 2)==0) {
            p += 2;
            continue;
        }

        /* /../ */
        if (strncmp(p, "../", 3)==0) {
            p += 3;
            if (q == path) {
                continue;
            }
            /* change q to prev '/' */
            q--;
            while (*q != '/')
                q--;
            continue;
        }

        /* // */
        if (*p=='/') {
            p++;
            continue;
        }

        /* "/." */
        if (strncmp(p, ".", pend-p)==0) {
            p++;
            continue;
        }

        /* "/.." */
        if (strncmp(p, "..", pend-p)==0) {
            p += 2;
            if (q == path) {
                continue;
            }
            /* change q to prev '/' */
            q--;
            while (*q != '/')
                q--;
            continue;
        }

        q++;
        *q = *p++;

    }
    q++;
    strcpy(q, p);
    return ;
}


//parse a url and extract site name, port and others.
int parse_url(const char *input, char *site, char *port, char *path)
{
	char tmp[MAX_URL_LEN];
	char *pin = tmp;
	char *p, *q;
	char *p_query = NULL;

	if (strlen(input) >= MAX_URL_LEN)
	{
		//Warn("parse_url: url is too long");
		return 0;
	}

	strcpy(tmp, input);
	delete_inter(tmp);

	if (strncasecmp(pin, "http://", 7) == 0)
		pin += 7;
	//add
	else if (strncasecmp(pin, "https://", 8) == 0)
		pin += 8;

	//get path
	p = strchr(pin, '/');
	p_query = strchr(pin, '?');
	if (NULL == p)
	{
		p = p_query;
	}
	if ((NULL != p_query) && (p_query <= p))
	{
		if (path != NULL)
		{
			path[0] = '\0';
			if (strlen(p_query) < MAX_PATH_LEN - 1)
			{
				strlcpy(path, "/", 2);
				strcat(path, p_query);
			}
			else
				return 0;
		}
		*p_query = '\0';
	}
	else
	{
		if (p != NULL)
		{
			if (path != NULL)
			{
				path[0] = '\0';
				if (strlen(p) < MAX_PATH_LEN)
					strcpy(path, p);
				else
					return 0;
			}
			*p = '\0';
		}
		else
		{
			if (path != NULL)
			{
				strcpy(path, "/");
			}
		}
	}

	q = strchr(pin, ':');
	//get port

	if (q != NULL)
	{
		if (port != NULL)
		{
			port[0] = '\0';
			if (strlen(q) < MAX_PORT_LEN && atoi(q + 1) > 0)
			{
				strcpy(port, q + 1);
			}
			else
				return 0;
		}
		*q = '\0';
	}
	else
	{
		if (port != NULL)
			port[0] = '\0';
	}
	//check if the default port
	if ((port != NULL) && (strncmp(port, "80", 3)) == 0)
	{
		port[0] = 0;
	}
	//get site
	if (site != NULL)
	{
		site[0] = '\0';
		if (pin[0] != '\0' && strlen(pin) < MAX_SITE_LEN)
		{
			strcpy(site, pin);
		}
		else
		{
			//Warn("parse_url: site name too long or empty url[%s]", pin);
			return 0;
		}
	}
	return 1;
}

char *get_path(const char *url, char *path)
{
	if (parse_url(url, NULL, NULL, path) == 0)
		return NULL;
	return path;
}

char *get_site(const char *url, char *site)
{
	if (parse_url(url, site, NULL, NULL) == 0)
		return NULL;
	return site;
}

int get_port(const char* url, int* pport)
{
	char strport[MAX_PORT_LEN];
	if (parse_url(url, NULL, strport, NULL) == 1)
	{
		if (strlen(strport) == 0)
		{
			*pport = 80;
			return 1;
		}
		else
		{
			*pport = atoi(strport);
			return 1;
		}
	}
	else
		return 0;
}

void get_static_part(const char *url, char *staticurl)
{
	char *p;
	char buffer[MAX_URL_LEN];

	assert(strlen(url) < MAX_URL_LEN);
	strcpy(buffer, url);

	p = strchr(buffer, '?');
	if (p != NULL)
	{
		*p = '\0';
	}
	p = strchr(buffer, ';');
	if (p != NULL)
	{
		*p = '\0';
	}

	strcpy(staticurl, buffer);
}

const int dyn_char_table[256]= {
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

int isdyn(const char* str)
{
	unsigned char* pch = (unsigned char*) str;

	while (dyn_char_table[*pch++])
	{
	}
	return *(--pch);
}

static char URL_DIGIT[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static unsigned char URL_LET_DIGIT[256]= {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

//LETTER DIGIT HYPHEN
static unsigned char URL_LET_DIG_HY[256]= {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
    1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static char URL_UP_LET[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

//uchar = unreserved | escape
/*
   static char UL_URL_UCHAR[256] = {
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,1,0,0,1,1,0,1,1,1,1,0,1,1,1,0,
   1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
   0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
 */

//pchar = uchar | ":" | "@" | "&" | "=" | "+"
//add ' ' '"' '<' '>' 32 34 60 62
static char URL_PCHAR[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,
    1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

//uchar | reserved
//add ' ' '"' '<' '>' 32 34 60 62
static char URL_UCH_RES[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

//pchar | "/"
//add ' ' '"' '<' '>' 32 34 60 62
static char URL_PARAM[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

int check_url(char *url)
{
	char *pin;
	char *pout;
	char port[6];
	int k = 0;
	int dotcount = 0;
	char *url_start;
	int space_flag = 0;
	int len = delete_inter(url); //delete \r\n;
	if (strncasecmp(url, "http://", 7) == 0)
	{
		pin = url + 7;
	}
	else if (strncasecmp(url, "https://", 7) == 0)
	{
		pin = url + 8;
	}
	else
	{
		pin = url;
	}
	pout = pin;
	url_start = pin;
	//check host name
	while (URL_LET_DIG_HY[(unsigned char) *pin] || *pin == '%')
	{
		if (URL_UP_LET[(unsigned char) *pin])
			*pin += 32;
		else
		{
			if (*pin == '%')
			{
				if (pin[1] != '\0' && pin[2] != '\0')
				{
					unsigned char escape_char = to_char(pin + 1);
					if (URL_LET_DIG_HY[escape_char])
					{
						if (URL_UP_LET[escape_char])
						{
							pin[2] = escape_char + 32;
						}
						else
						{
							pin[2] = escape_char;
						}
						pin += 2;
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
		}
		*pout++ = *pin++;
	}
	if (pin == url_start)
		return 0;

	pin--;
	if (!URL_LET_DIGIT[(unsigned char) *pin++])
		return 0;
	while (*pin == '.')
	{
		dotcount++;
		*pout++ = *pin++;
		while (URL_LET_DIG_HY[(unsigned char) *pin] || (*pin == '%'))
		{
			if (URL_UP_LET[(unsigned char) *pin])
				*pin += 32;
			else
			{
				if (*pin == '%')
				{
					if (pin[1] != '\0' && pin[2] != '\0')
					{
						unsigned char escape_char = to_char(pin + 1);
						if (URL_LET_DIG_HY[escape_char] == 1)
						{
							if (URL_UP_LET[escape_char])
							{
								pin[2] = escape_char + 32;
							}
							else
							{
								pin[2] = escape_char;
							}
							pin += 2;
						}
						else
						{
							return 0;
						}
					}
					else
					{
						return 0;
					}
				}
			}
			*pout++ = *pin++;
		}
		pin--;
		if (!URL_LET_DIGIT[(unsigned char) *pin++])
			return 0;
	}
	if (dotcount == 0)
	{
		return 0;
	}
	//check host name over

	//check port
	if (*pin == ':')
	{
		pin++;
		if (*pin != '/')
		{
			while (URL_DIGIT[(unsigned char) *pin])
			{
				port[k++] = *pin++;
				if (k > 5)
					return 0;
			}
			port[k] = 0;
			if (*pin != '/')
				return 0;
			if (strcmp(port, "80") != 0)
			{
				memcpy(pout, pin - k - 1, k + 1);
				pout += (k + 1);
			}
		}
	}
	else if ((*pin != '/') && (*pin != '\0'))
		return 0;
	//check port over

	//check abs_path
	if (*pin == '/')
	{
		*pout++ = *pin++;
		//path
		while (URL_PCHAR[(unsigned char) *pin])
		{
			space_flag |= ((*pout++ = *pin++) == ' ');
		}

		while (*pin == '/')
		{
			*pout++ = *pin++;
			while (URL_PCHAR[(unsigned char) *pin])
			{
				space_flag |= ((*pout++ = *pin++) == ' ');
			}
		}

		// ";" params  == *(";" param)
		while (*pin == ';')
		{
			*pout++ = *pin++;
			while (URL_PARAM[(unsigned char) *pin])
			{
				space_flag |= ((*pout++ = *pin++) == ' ');
			}
		}

		//"?" query
		while (*pin == '?')
		{
			*pout++ = *pin++;
			while (URL_UCH_RES[(unsigned char) *pin])
			{
				space_flag |= ((*pout++ = *pin++) == ' ');
			}
		}
		if (*pin != 0)
			return 0;
		*pout = 0;
		if (pout - url > len)
			return 0;
		if (space_flag == 1)
			return space_escape(url, MAX_URL_LEN, MAX_SITE_LEN, MAX_PATH_LEN);
		else
			return 1;
	}
	else if (*pin != 0)
		return 0;
	if (space_flag == 1)
		return space_escape(url, MAX_URL_LEN, MAX_SITE_LEN, MAX_PATH_LEN);
	else
		return 1;
	//check abs_path over
}

// LETTER DIGIT
static unsigned char LET_DIGIT[256]= {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                             0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
                                        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                        1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
                                        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                        1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

//LETTER DIGIT HYPHEN
static unsigned char LET_DIG_HY[256]= {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
        1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,
        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};


//      <hostname> ::= <name>*["."<name>]
//      <name>  ::= <let>[*[<let-or-digit-or-hyphen>]<let-or-digit>]
int check_hostname(char *host_name)
{
	if (host_name == NULL)
		return 0;
	char *purl = host_name;

	while (LET_DIG_HY[(unsigned char) (*purl++)])
		;
	purl -= 2;
	if (!LET_DIGIT[(unsigned char) (*purl++)])
		return 0;
	while (*purl++ == '.')
	{
		while (LET_DIG_HY[(unsigned char) (*purl++)])
			;
		purl -= 2;
		if (!LET_DIGIT[(unsigned char) (*purl++)])
			return 0;
	}

	purl--;
	if (*purl == '\0')
		return 1;
	else
		return 0;
}


/*
 * check a sitename is in dotip format
 * input : sitename : the sitename
 * return : non-zero if is
 *        : 0 if not
 */
int is_dotip(const char *sitename)
{
	in_addr l_inp;
	int ret = inet_aton(sitename, &l_inp);
	return ret;
}

const int IGNORE_COUNT=47;

typedef struct ilist
{
	char name[36];
	int length;
} ilist_t;

const ilist_t ignore_list[IGNORE_COUNT]= {
    {"ac.",3},
    {"ah.",3},
    {"bj.",3},
    {"co.",3},
    {"com.",4},
    {"cq.",3},
    {"ed.",3},
    {"edu.",4},
    {"fj.",3},
    {"gd.",3},
    {"go.",3},
    {"gov.",4},
    {"gs.",3},
    {"gx.",3},
    {"gz.",3},
    {"ha.",3},
    {"hb.",3},
    {"he.",3},
    {"hi.",3},
    {"hk.",3},
    {"hl.",3},
    {"hn.",3},
    {"jl.",3},
    {"js.",3},
    {"jx.",3},
    {"ln.",3},
    {"mo.",3},
    {"ne.",3},
    {"net.",4},
    {"nm.",3},
    {"nx.",3},
    {"or.",3},
    {"org.",4},
    {"pe.",3},
    {"qh.",3},
    {"sc.",3},
    {"sd.",3},
    {"sh.",3},
    {"sn.",3},
    {"sx.",3},
    {"tj.",3},
    {"tw.",3},
    {"www.",4},
    {"xj.",3},
    {"xz.",3},
    {"yn.",3},
    {"zj.",3}
};

int is_ignore(const char* slip)
{
	int head = 0;
	int tail = IGNORE_COUNT - 1;
	int cur;
	int ret;
	while (head <= tail)
	{
		cur = (head + tail) / 2;
		ret = strncmp(slip, ignore_list[cur].name, ignore_list[cur].length);
		if (ret < 0)
		{
			tail = cur - 1;
		}
		else if (ret > 0)
		{
			head = cur + 1;
		}
		else
		{
			return 1;
		}
	}
	return 0;
}

/*
 * fetch the trunk from the sitename, if site is a dotip, the ip
 * will be return in trunk
 * in-argu : site : the site name
 * out-argu : trunk : the trunk of the sitename
 * in-argu : size : the buffer size of the trunk
 * return : 1 if get trunk successfully
 *          -2 if the site have no trunk
 *          -3 if the site have no dot
 *          -1 unknown error
 */
int fetch_trunk(const char* site, char *trunk, int size)
{
	const char* pfirst;
	const char* ptail;
	int len;

	if (is_dotip(site))
	{
		strncpy(trunk, site, size - 1);
		trunk[size - 1] = 0;
		return 1;
	}

	ptail = strrchr(site, '.');
	if (ptail == NULL)
	{
		return -3;
	}
	pfirst = ptail - 1;
	while (pfirst >= site - 1)
	{
		if ((pfirst < site) || (*pfirst == '.'))
		{
			if (is_ignore(pfirst + 1) == 0)
			{
				len = size - 1 > ptail - pfirst - 1 ? ptail - pfirst - 1 : size - 1;
				memcpy(trunk, pfirst + 1, len);
				trunk[len] = 0;
				return 1;
			}
			else
			{
				ptail = pfirst;
			}
		}
		pfirst--;
	}
	return -2;
}

int isnormalized_url(const char *url)
{
	const char *pin;
	char port[6];
	int k = 0;
	unsigned uport;
	int dotcount = 0;

	//modify
	if (strncasecmp(url, "http://", 7) == 0 || strncasecmp(url, "https://", 8) == 0)
		return 0; // should not start with "http://"
	else
		pin = url;

	//check host name
	while (URL_LET_DIG_HY[(unsigned char) *pin])
	{ // [0-9a-zA-Z\-]
		if (URL_UP_LET[(unsigned char) *pin]) //[A-Z]
			return 0; // site should be lower case.
		pin++;
	}
	pin--;
	if (!URL_LET_DIGIT[(unsigned char) *pin++]) //[0-9a-zA-Z]
		return 0; //not end with "-"
	while (*pin == '.')
	{
		dotcount++;
		pin++;
		while (URL_LET_DIG_HY[(unsigned char) *pin])
		{
			if (URL_UP_LET[(unsigned char) *pin])
				return 0; // site should be lower case.
			pin++;
		}
		pin--;
		if (!URL_LET_DIGIT[(unsigned char) *pin++])
			return 0;
	}
	if (dotcount == 0)
	{
		return 0;
	}
	//check host name over

	//check port
	if (*pin == ':')
	{
		pin++;
		if (*pin != '/')
		{
			while (URL_DIGIT[(unsigned char) *pin])
			{
				port[k++] = *pin++;
				if (k > 5)
					return 0;
			}
			port[k] = 0;
			if (*pin != '/')
				return 0;

			uport = atoi(port);
			if (uport <= 0 || uport > 65535)
				return 0;

			if (uport == 80)
				return 0; // should omit port 80
		}
		else
			return 0;
	}
	else if ((*pin != '/'))
		return 0;
	//check port over

	//check abs_path
	if (*pin == '/')
	{
		pin++;
		//path
		while (URL_PCHAR[(unsigned char) *pin])
		{
			pin++;
		}
		while (*pin == '/')
		{
			pin++;
			while (URL_PCHAR[(unsigned char) *pin])
			{
				pin++;
			}
		}

		// ";" params  == *(";" param)
		while (*pin == ';')
		{
			pin++;
			while (URL_PARAM[(unsigned char) *pin])
			{
				pin++;
			}
		}

		//"?" query
		while (*pin == '?')
		{
			pin++;
			while (URL_UCH_RES[(unsigned char) *pin])
			{
				pin++;
			}
		}
		if (*pin != 0)
			return 0;
		if (pin - url >= MAX_URL_LEN)
			return 0;
		return 1;
	}
	else
		return 0;

	//check abs_path over
}

int normalize_site(char *site)
{
	int dotcount = 0;
	char *pin = site;
	char *pout = site;
	if (!URL_LET_DIG_HY[(unsigned char) *pin]) //[0-9a-zA-Z\-]
		return 0; //must start with "0-9a-zA-Z\-"

	while (URL_LET_DIG_HY[(unsigned char) *pin] || *pin == '%')
	{ // [0-9a-zA-Z\-]
		if (URL_UP_LET[(unsigned char) *pin]) //[A-Z]
			*pin += 32;
		else
		{
			if (*pin == '%')
			{
				if (pin[1] != '\0' && pin[2] != '\0')
				{
					unsigned char escape_char = to_char(pin + 1);
					if (URL_LET_DIG_HY[escape_char])
					{
						if (URL_UP_LET[escape_char])
						{
							pin[2] = escape_char + 32;
						}
						else
						{
							pin[2] = escape_char;
						}
						pin += 2;
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
		}
		*pout++ = *pin++;
	}
	if (pin == site)
		return 0;

	pin--;
	if (!URL_LET_DIGIT[(unsigned char) *pin++]) //[0-9a-zA-Z]
		return 0; //not end with "-"
	while (*pin == '.')
	{
		dotcount++;
		*pout++ = *pin++;
		while (URL_LET_DIG_HY[(unsigned char) *pin] || (*pin == '%'))
		{
			if (URL_UP_LET[(unsigned char) *pin])
				*pin += 32;
			else
			{
				if (*pin == '%')
				{
					if (pin[1] != '\0' && pin[2] != '\0')
					{
						unsigned char escape_char = to_char(pin + 1);
						if (URL_LET_DIG_HY[escape_char] == 1)
						{
							if (URL_UP_LET[escape_char])
							{
								pin[2] = escape_char + 32;
							}
							else
							{
								pin[2] = escape_char;
							}
							pin += 2;
						}
						else
						{
							return 0;
						}
					}
					else
					{
						return 0;
					}
				}
			}
			*pout++ = *pin++;
		}
		pin--;
		if (!URL_LET_DIGIT[(unsigned char) *pin++])
			return 0;
	}
	if (dotcount == 0)
	{
		return 0;
	}
	if (*pin != 0)
		return 0;
	*pout = '\0';
	return 1;
}

int normalize_site_special_for_ps_20101026(char *site)
{
	//加入这个函数是一个很扯淡的原因
	//因为在dict里面的create_url_sign的话依赖了normalize_url_ex2
	//而在normalize_url_ex2里面依赖修改这个函数的行为
	//所以我们需要增加这个接口函数.
	//相对于url_normalize_site部分的变动会使用(MODIFY)标记.
	//要求加入这个功能的接口人是刘成城[PSSearcher]
	int dotcount = 0;
	char *pin = site;
	char *pout = site;
	if (!URL_LET_DIG_HY[(unsigned char) *pin]) //[0-9a-zA-Z\-]
		return 0; //must start with "0-9a-zA-Z\-"

	while (URL_LET_DIG_HY[(unsigned char) *pin] || *pin == '%')
	{ // [0-9a-zA-Z\-]
		if (URL_UP_LET[(unsigned char) *pin]) //[A-Z]
			*pin += 32;
		else
		{
			if (*pin == '%')
			{
				if (pin[1] != '\0' && pin[2] != '\0')
				{
					unsigned char escape_char = to_char(pin + 1);
					if (URL_LET_DIG_HY[escape_char])
					{
						if (URL_UP_LET[escape_char])
						{
							pin[2] = escape_char + 32;
						}
						else
						{
							pin[2] = escape_char;
						}
						pin += 2;
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
		}
		*pout++ = *pin++;
	}
	if (pin == site)
		return 0;

	pin--;

	//MODIFY.现在允许最后出现-
	//if(!URL_LET_DIGIT[(unsigned char)*pin++]) //[0-9a-zA-Z]
	//return 0; //not end with "-"
	if (!URL_LET_DIG_HY[(unsigned char) *pin++])
	{ //[0-9a-zA-Z\-]
		return 0;
	}

	while (*pin == '.')
	{
		dotcount++;
		*pout++ = *pin++;
		while (URL_LET_DIG_HY[(unsigned char) *pin] || (*pin == '%'))
		{
			if (URL_UP_LET[(unsigned char) *pin])
				*pin += 32;
			else
			{
				if (*pin == '%')
				{
					if (pin[1] != '\0' && pin[2] != '\0')
					{
						unsigned char escape_char = to_char(pin + 1);
						if (URL_LET_DIG_HY[escape_char] == 1)
						{
							if (URL_UP_LET[escape_char])
							{
								pin[2] = escape_char + 32;
							}
							else
							{
								pin[2] = escape_char;
							}
							pin += 2;
						}
						else
						{
							return 0;
						}
					}
					else
					{
						return 0;
					}
				}
			}
			*pout++ = *pin++;
		}
		pin--;
		//MODIFY.现在允许最后出现-
		//if(!URL_LET_DIGIT[(unsigned char)*pin++])
		//return 0;
		if (!URL_LET_DIG_HY[(unsigned char) *pin++])
		{ //[0-9a-zA-Z\-]
			return 0;
		}
	}

	if (dotcount == 0)
	{
		return 0;
	}
	if (*pin != 0)
		return 0;
	*pout = '\0';
	return 1;
}


int normalize_port(char *port)
{
	char *pin = port;
	unsigned uport;
	int k = 0;

	if (*port == 0)
		return 1;

	while (URL_DIGIT[(unsigned char) *pin])
	{
		k++;
		pin++;
		if (k > 5)
			return 0;
	}
	if (*pin != 0)
		return 0;
	uport = atoi(port);
	if (uport <= 0 || uport > 65535)
		return 0;

	if (uport != 80)
		sprintf(port, "%d", uport);
	else
		port[0] = '\0';
	return 1;
}

int normalize_url(const char* url, char* buf)
{
	char site[MAX_SITE_LEN];
	char port[MAX_PORT_LEN];
	char path[MAX_PATH_LEN];

	if (parse_url(url, site, port, path) == 0)
		return 0;

	if (normalize_site(site) == 0)
		return 0;
	if (normalize_port(port) == 0)
		return 0;
	//这里调用single_path_nointer而不调用single_path
	//single_path_nointer里没有delete_inter操作,在parse_url
	//里面已经作过一次parse_url这里没必要再作一次
	//single_path为了支持外部程序中直接调用single_path对未经过parse_url
	//处理的path进行处理，加上了delete_inter
	if (single_path_nointer(path) == 0)
		return 0;
	normalize_path(path);

	if (port[0] == '\0')
	{
		snprintf(buf, MAX_URL_LEN, "%s%s", site, path);
	}
	else
	{
		snprintf(buf, MAX_URL_LEN, "%s:%s%s", site, port, path);
	}
	return 1;
}

//int parse_url(char *input,char *site,char *port,char *path) {
//    return parse_url((const char*)input, site, port, path);
//}
//
//char *get_path(char *url, char *path) {
//    return get_path((const char *)url, path);
//}
//
//char *get_site(char *url, char *site) {
//    return get_site((const char *)url, site);
//}
//
//int get_port(char* url,int* pport) {
//    return get_port((const char* )url, pport);
//}
//
//void get_static_part(char *url, char *staticurl) {
//    get_static_part((const char *)url, staticurl);
//}


int parse_url_ex(const char *input, char *site,size_t site_size,
                    char *port, size_t port_size, char *path,
		size_t max_path_size)
{
	char tmp[MAX_URL_LEN];
	char *pin = tmp;
	char *p, *q;
	char *p_query;

	if (strlen(input) >= MAX_URL_LEN)
	{
//        printf("parse_url_ex: url is too long");
		return 0;
	}

	strcpy(tmp, input);
	delete_inter(tmp);

	if (strncasecmp(pin, "http://", 7) == 0)
		pin += 7;
	else if (strncasecmp(pin, "https://", 8) == 0)
			pin += 8;
	/*get path*/
	p = strchr(pin, '/');
	p_query = strchr(pin, '?');
	if (NULL == p)
	{
		p = p_query;
	}
	if ((NULL != p_query) && (p_query <= p))
	{
		if (path != NULL)
		{
			path[0] = '\0';
			if (strlen(p_query) < max_path_size - 1)
			{
				strcpy(path, "/");
				strcat(path, p_query);
			}
			else
				return 0;
		}
		*p_query = '\0';
	}
	else
	{
		if (p != NULL)
		{
			if (path != NULL)
			{
				path[0] = '\0';
				if (strlen(p) < max_path_size)
					strcpy(path, p);
				else
					return 0;
			}
			*p = '\0';
		}
		else
		{
			if (path != NULL)
			{
				strcpy(path, "/");
			}
		}
	}

	q = strchr(pin, ':');
	/*get port*/

	if (q != NULL)
	{
		if (port != NULL)
		{
			port[0] = '\0';
			if (strlen(q) < port_size && atoi(q + 1) > 0)
			{
				strcpy(port, q + 1);
			}
			else
				return 0;
		}
		*q = '\0';
	}
	else
	{
		if (port != NULL)
			port[0] = '\0';

	}
	/*check if the default port*/
	if ((port != NULL) && (strncmp(port, "80", 3)) == 0)
	{
		port[0] = 0;
	}
	/*get site*/
	if (site != NULL)
	{
		site[0] = '\0';
		if (pin[0] != '\0' && strlen(pin) < site_size)
		{
			strcpy(site, pin);
		}
		else
		{
//            printf("parse_url_ex: site name too long or empty url[%s]",pin);
			return 0;
		}

	}
	return 1;

}

char *get_path_ex(const char *url, char *path, size_t path_size)
{
	if (parse_url_ex(url, NULL, 0, NULL, 0, path, path_size) == 0)
		return NULL;
	return path;
}

char *get_site_ex(const char *url, char *site, size_t site_size)
{
	if (parse_url_ex(url, site, site_size, NULL, 0, NULL, 0) == 0)
		return NULL;
	return site;
}

int get_port_ex(const char* url, int* pport)
{
	char strport[MAX_PORT_LEN];
	if (parse_url_ex(url, NULL, 0, strport, MAX_PORT_LEN, NULL, 0) == 1)
	{
		if (strlen(strport) == 0)
		{
			*pport = 80;
			return 1;
		}
		else
		{
			*pport = atoi(strport);
			return 1;
		}
	}
	else
		return 0;
}

int normalize_url_ex(const char* url, char* buf, size_t buf_size)
{
	char site[MAX_SITE_LEN];
	char port[MAX_PORT_LEN];
	char path[MAX_PATH_LEN];

	if (parse_url_ex(url, site, MAX_SITE_LEN, port, MAX_PORT_LEN, path, MAX_PATH_LEN) == 0)
		return 0;

	if (normalize_site(site) == 0)
		return 0;
	if (normalize_port(port) == 0)
		return 0;
	//这里调用single_path_ex_nointer而不调用single_path_ex
	//single_path_ex_nointer里没有delete_inter操作,在parse_url_ex
	//里面已经作过一次parse_url_ex这里没必要再作一次
	//single_path_ex为了支持外部程序中直接调用single_path_ex对未经过parse_url_ex
	//处理的path进行处理，加上了delete_inter
	if (single_path_ex_nointer(path, MAX_PATH_LEN) == 0)
		return 0;
	normalize_path_ex(path);

	if (port[0] == '\0')
	{
		snprintf(buf, buf_size, "%s%s", site, path);
	}
	else
	{
		snprintf(buf, buf_size, "%s:%s%s", site, port, path);
	}
	return 1;
}

void get_static_part_ex(const char *url, char *staticurl, size_t staticurl_size)
{
	char *p;
	char buffer[MAX_URL_LEN];

	assert(strlen(url) < MAX_URL_LEN);
	strcpy(buffer, url);

	p = strchr(buffer, '?');
	if (p != NULL)
	{
		*p = '\0';
	}
	p = strchr(buffer, ';');
	if (p != NULL)
	{
		*p = '\0';
	}
	snprintf(staticurl, staticurl_size, "%s", buffer);
}

int isnormalized_url_ex(const char *url)
{
	const char *pin;
	char port[6];
	int len;
	int k = 0;
	unsigned uport;
	int dotcount = 0;
	len = strlen(url);
	if (strncasecmp(url, "http://", 7) == 0 || strncasecmp(url, "https://", 8) == 0)
		return 0; // should not start with "http://"
	else
		pin = url;

	//check host name
	while (URL_LET_DIG_HY[(unsigned char) *pin])
	{ // [0-9a-zA-Z\-]
		if (URL_UP_LET[(unsigned char) *pin]) //[A-Z]
			return 0; // site should be lower case.
		pin++;
	}
	pin--;
	if (!URL_LET_DIGIT[(unsigned char) *pin++]) //[0-9a-zA-Z]
		return 0; //not end with "-"
	while (*pin == '.')
	{
		dotcount++;
		pin++;
		while (URL_LET_DIG_HY[(unsigned char) *pin])
		{
			if (URL_UP_LET[(unsigned char) *pin])
				return 0; // site should be lower case.
			pin++;
		}
		pin--;
		if (!URL_LET_DIGIT[(unsigned char) *pin++])
			return 0;
	}
	if (dotcount == 0)
	{
		return 0;
	}
	//check host name over

	//check port
	if (*pin == ':')
	{
		pin++;
		if (*pin != '/')
		{
			while (URL_DIGIT[(unsigned char) *pin])
			{
				port[k++] = *pin++;
				if (k > 5)
					return 0;
			}
			port[k] = 0;
			if (*pin != '/')
				return 0;

			uport = atoi(port);
			if (uport <= 0 || uport > 65535)
				return 0;

			if (uport == 80)
				return 0; // should omit port 80
		}
		else
			return 0;
	}
	else if ((*pin != '/'))
		return 0;
	//check port over

	//check abs_path
	if (*pin == '/')
	{
		pin++;
		//path
		while (URL_PCHAR[(unsigned char) *pin])
		{
			pin++;
		}
		while (*pin == '/')
		{
			pin++;
			while (URL_PCHAR[(unsigned char) *pin])
			{
				pin++;
			}
		}

		// ";" params  == *(";" param)
		while (*pin == ';')
		{
			pin++;
			while (URL_PARAM[(unsigned char) *pin])
			{
				pin++;
			}
		}

		//"?" query
		while (*pin == '?')
		{
			pin++;
			while (URL_UCH_RES[(unsigned char) *pin])
			{
				pin++;
			}
		}
		if (*pin != 0)
			return 0;
		if ((pin - url) >= len)
			return 0;
		return 1;
	}
	else
		return 0;

	//check abs_path over
}

void normalize_path_ex(char *path)
{
	char *p, *q;
	char tmp[MAX_URL_LEN];
	char *pend;
	size_t len = 0;

	assert(*path == '/');
	len = strlen(path);
	assert(len < MAX_URL_LEN);

	replace_slash(path);

	strcpy(tmp, path);

	pend = strchr(tmp, '?');
	if (pend == NULL)
	{
		pend = strchr(tmp, ';');
	}
	if (pend == NULL)
	{
		pend = tmp + strlen(tmp);
	}

	p = tmp + 1;
	q = path;
	while (p != pend)
	{
		/* normal */
		if (*q != '/')
		{
			q++;
			*q = *p++;
			continue;
		}

		/* /./ */
		if (strncmp(p, "./", 2) == 0)
		{
			p += 2;
			continue;
		}

		/* /../ */
		if (strncmp(p, "../", 3) == 0)
		{
			p += 3;
			if (q == path)
			{
				continue;
			}
			/* change q to prev '/' */
			q--;
			while (*q != '/')
				q--;
			continue;
		}

		/* // */
		if (*p == '/')
		{
			p++;
			continue;
		}

		/* "/." */
		if (strncmp(p, ".", pend - p) == 0)
		{
			p++;
			continue;
		}

		/* "/.." */
		if (strncmp(p, "..", pend - p) == 0)
		{
			p += 2;
			if (q == path)
			{
				continue;
			}
			/* change q to prev '/' */
			q--;
			while (*q != '/')
				q--;
			continue;
		}

		q++;
		*q = *p++;

	}
	q++;
	strcpy(q, p);
	return;
}

int check_url_ex(char *url)
{
	char *pin;
	char *pout;
	char port[6];
	int k = 0;
	int dotcount = 0;
	char *url_start;
	int len;
	int space_flag = 0;
	len = delete_inter(url); //delete \r\n;
	if (strncasecmp(url, "http://", 7) == 0)
	{
		pin = url + 7;
	}
	else if (strncasecmp(url, "https://", 8) == 0)
	{
		pin = url + 8;
	}
	else
	{
		pin = url;
	}
	pout = pin;
	url_start = pin;
	//check host name
	while (URL_LET_DIG_HY[(unsigned char) *pin] || *pin == '%')
	{
		if (URL_UP_LET[(unsigned char) *pin])
			*pin += 32;
		else
		{
			if (*pin == '%')
			{
				if (pin[1] != '\0' && pin[2] != '\0')
				{
					unsigned char escape_char = to_char(pin + 1);
					if (URL_LET_DIG_HY[escape_char])
					{
						if (URL_UP_LET[escape_char])
						{
							pin[2] = escape_char + 32;
						}
						else
						{
							pin[2] = escape_char;
						}
						pin += 2;
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
		}
		*pout++ = *pin++;
	}
	if (pin == url_start)
		return 0;

	pin--;
	if (!URL_LET_DIGIT[(unsigned char) *pin++])
		return 0;
	while (*pin == '.')
	{
		dotcount++;
		*pout++ = *pin++;
		while (URL_LET_DIG_HY[(unsigned char) *pin] || (*pin == '%'))
		{
			if (URL_UP_LET[(unsigned char) *pin])
				*pin += 32;
			else
			{
				if (*pin == '%')
				{
					if (pin[1] != '\0' && pin[2] != '\0')
					{
						unsigned char escape_char = to_char(pin + 1);
						if (URL_LET_DIG_HY[escape_char] == 1)
						{
							if (URL_UP_LET[escape_char])
							{
								pin[2] = escape_char + 32;
							}
							else
							{
								pin[2] = escape_char;
							}
							pin += 2;
						}
						else
						{
							return 0;
						}
					}
					else
					{
						return 0;
					}
				}
			}
			*pout++ = *pin++;
		}
		pin--;
		if (!URL_LET_DIGIT[(unsigned char) *pin++])
			return 0;
	}
	if (dotcount == 0)
	{
		return 0;
	}
	//check host name over

	//check port
	if (*pin == ':')
	{
		pin++;
		if (*pin != '/')
		{
			while (URL_DIGIT[(unsigned char) *pin])
			{
				port[k++] = *pin++;
				if (k > 5)
					return 0;
			}
			port[k] = 0;
			if (*pin != '/')
				return 0;
			memcpy(pout, pin - k - 1, k + 1);
			if (strcmp(port, "80") != 0)
			{
				pout += (k + 1);
			}
		}
	}
	else if ((*pin != '/') && (*pin != '\0'))
		return 0;
	//check port over

	//check abs_path
	if (*pin == '/')
	{
		*pout++ = *pin++;
		//path
		while (URL_PCHAR[(unsigned char) *pin])
		{
			space_flag |= ((*pout++ = *pin++) == ' ');
		}

		while (*pin == '/')
		{
			*pout++ = *pin++;
			while (URL_PCHAR[(unsigned char) *pin])
			{
				space_flag |= ((*pout++ = *pin++) == ' ');
			}
		}

		// ";" params  == *(";" param)
		while (*pin == ';')
		{
			*pout++ = *pin++;
			while (URL_PARAM[(unsigned char) *pin])
			{
				space_flag |= ((*pout++ = *pin++) == ' ');
			}
		}

		//"?" query
		while (*pin == '?')
		{
			*pout++ = *pin++;
			while (URL_UCH_RES[(unsigned char) *pin])
			{
				space_flag |= ((*pout++ = *pin++) == ' ');
			}
		}
		if (*pin != 0)
			return 0;
		*pout = 0;
		if (pout - url > len)
			return 0;
		if (space_flag == 1)
			return space_escape(url, MAX_URL_LEN, MAX_SITE_LEN, MAX_PATH_LEN);
		else
			return 1;
	}
	else if (*pin != 0)
		return 0;
	*pout = '\0';
	if (space_flag == 1)
		return space_escape(url, MAX_URL_LEN, MAX_SITE_LEN, MAX_PATH_LEN);
	else
		return 1;

	//check abs_path over
}

//##########################################################
//##########################################################
//##########################################################
//the difference to 'parse_url_ex' is
//MAX_URL_LEN->MAX_URL_LEN
int parse_url_ex2(const char *input, char *site,size_t site_size,
                     char *port, size_t port_size, char *path,
		size_t max_path_size)
{
	char tmp[MAX_URL_LEN];
	char *pin = tmp;
	char *p, *q;
	char *p_query;

	if (strlen(input) >= MAX_URL_LEN)
	{
//        printf("parse_url_ex2: url is too long");
		return 0;
	}

	strcpy(tmp, input);
	delete_inter(tmp);

	if (strncasecmp(pin, "http://", 7) == 0)
		pin += 7;
	else if (strncasecmp(pin, "https://", 8) == 0)
		pin += 8;

	/*get path*/
	p = strchr(pin, '/');
	p_query = strchr(pin, '?');
	if (NULL == p)
	{
		p = p_query;
	}
	if ((NULL != p_query) && (p_query <= p))
	{
		if (path != NULL)
		{
			path[0] = '\0';
			if (strlen(p_query) < max_path_size - 1)
			{
				strlcpy(path, "/", 2);
				strcat(path, p_query);
			}
			else
				return 0;
		}
		*p_query = '\0';
	}
	else
	{
		if (p != NULL)
		{
			if (path != NULL)
			{
				path[0] = '\0';
				if (strlen(p) < max_path_size)
					strcpy(path, p);
				else
					return 0;
			}
			*p = '\0';
		}
		else
		{
			if (path != NULL)
			{
				strcpy(path, "/");
			}
		}
	}

	q = strchr(pin, ':');
	/*get port*/

	if (q != NULL)
	{
		if (port != NULL)
		{
			port[0] = '\0';
			if (strlen(q) < port_size && atoi(q + 1) > 0)
			{
				strcpy(port, q + 1);
			}
			else
				return 0;
		}
		*q = '\0';
	}
	else
	{
		if (port != NULL)
			port[0] = '\0';

	}
	/*check if the default port*/
	if ((port != NULL) && (strncmp(port, "80", 3)) == 0)
	{
		port[0] = 0;
	}
	/*get site*/
	if (site != NULL)
	{
		site[0] = '\0';
		if (pin[0] != '\0' && strlen(pin) < site_size)
		{
			strcpy(site, pin);
		}
		else
		{
//            printf("parse_url_ex2: site name too long or empty url[%s]",pin);
			return 0;
		}

	}
	return 1;
}

char *get_path_ex2(const char *url, char *path, size_t path_size)
{
	if (parse_url_ex2(url, NULL, 0, NULL, 0, path, path_size) == 0)
		return NULL;
	return path;
}

char *get_site_ex2(const char *url, char *site, size_t site_size)
{
	if (parse_url_ex2(url, site, site_size, NULL, 0, NULL, 0) == 0)
		return NULL;
	return site;
}

int get_port_ex2(const char* url, int* pport)
{
	char strport[MAX_PORT_LEN];
	if (parse_url_ex2(url, NULL, 0, strport, MAX_PORT_LEN, NULL, 0) == 1)
	{
		if (strlen(strport) == 0)
		{
			*pport = 80;
			return 1;
		}
		else
		{
			*pport = atoi(strport);
			return 1;
		}
	}
	else
		return 0;
}

//escape sapce of whole url...and check the site and path length..
static int space_escape_ex2(char * url, int max_url_len, int max_site_len, int max_path_len)
{
	char * pin = url;
	int pout = 0;
	char url_tmp[MAX_URL_LEN];
	while (*pin != '\0')
	{
		if (*pin != ' ')
		{
			if (pout >= max_url_len - 1)
				return 0;
			url_tmp[pout++] = *pin++;
		}
		else
		{ //space
			if (pout >= max_url_len - 3)
				return 0;
			url_tmp[pout++] = '%';
			url_tmp[pout++] = '2';
			url_tmp[pout++] = '0';
			pin++;
		}
	}
	if (pout >= max_url_len)
		return 0;
	url_tmp[pout] = '\0';
	//detect site len and path len
	if (strncmp(url_tmp, "http://", 7) == 0)
	{
		pin = url_tmp + 7;
	}
	else if (strncmp(url_tmp, "https://", 8) == 0)
	{
		pin = url_tmp + 8;
	}
	else
	{
		pin = url_tmp;
	}
	char * path = strchr(pin, '/');
	if (path == pin)
		return 0;
	//check path length:-)[including the slash /]
	if (path != NULL && strlen(path) >= (u_int) max_path_len)
		return 0;
	char * port = strchr(pin, ':');
	char * psite_end = NULL;
	//ugly but I understand its meaning..:-)
	if (port == pin) //no any site..
		return 0;
	if (NULL != port && NULL != path && port < path)
	{ //with a port
		psite_end = port - 1;
	}
	else if (path != NULL)
	{ //no port just path
		psite_end = path - 1;
	}
	//here psite_end==NULL...just a site...
	if (psite_end != NULL && psite_end - pin >= max_site_len)
	{
		return 0;
	}
	else if (psite_end == NULL && strlen(pin) >= (u_int) max_site_len)
	{
		return 0;
	}
	strncpy(url, url_tmp, pout + 1);
	return 1;
}

static int space_escape_path_ex2(char * path, int max_path_len)
{
	char * pin = path;
	int pout = 0;
	char url_tmp[MAX_URL_LEN];
	while (*pin != '\0')
	{
		if (*pin != ' ')
		{
			if (pout >= max_path_len - 1) //we have to judge here...
				return 0;
			url_tmp[pout++] = *pin++;
		}
		else
		{ //space
			if (pout >= max_path_len - 3)
				return 0;
			url_tmp[pout++] = '%';
			url_tmp[pout++] = '2';
			url_tmp[pout++] = '0';
			pin++;
		}
	}
	if (pout >= max_path_len)
		return 0;
	url_tmp[pout] = '\0';

	strncpy(path, url_tmp, pout + 1);
	return 1;
}

static int single_path_ex2_nointer(char *path, int len)
{
	char *pin;
	int pout;
	char hex[2];
	char tmp_hex[2];
	char escape_char;
	int space_flag = 0;
	//len = strlen(path);
	pin = path;
	pout = 0;
	//check abs_path
	if (*pin == '/')
	{
		path[pout++] = *pin++;
	}

	//path
	while (EASOU_PCHAR[(unsigned char) *pin])
	{
		tmp_hex[0] = '\0';
		if ((*pin == '%') && (*(pin + 1) != '\0') && (*(pin + 2) != '\0'))
		{
			strncpy(hex, ++pin, 2);
			strncpy(tmp_hex, hex, 2);
			pin++;
			escape_char = to_char(hex);
			if (escape_char == 0)
			{
				pin--;
				path[pout++] = '%';
				continue;
			}
			*pin = escape_char;
		}
		if (EASOU_PCHAR[(unsigned char) *pin] && (tmp_hex[0] == '\0' || !EASOU_RESERVED[(unsigned char) *pin]))
		{
			if (*pin == ' ')
			{
				space_flag = 1;
			}
			path[pout++] = *pin++;
		}
		else
		{
			path[pout++] = '%';
			strncpy(&path[pout], tmp_hex, 2);
			pout = pout + 2;
			pin++;
		}
	}

	while (*pin == '/')
	{
		path[pout++] = *pin++;
		while (EASOU_PCHAR[(unsigned char) *pin])
		{
			tmp_hex[0] = '\0';
			if ((*pin == '%') && (*(pin + 1) != '\0') && (*(pin + 2) != '\0'))
			{
				strncpy(hex, ++pin, 2);
				strncpy(tmp_hex, hex, 2);
				pin++;
				escape_char = to_char(hex);
				if (escape_char == 0)
				{
					pin--;
					path[pout++] = '%';
					continue;
				}
				*pin = escape_char;
			}

			if (EASOU_PCHAR[(unsigned char) *pin] && (tmp_hex[0] == '\0' || !EASOU_RESERVED[(unsigned char) *pin]))
			{
				if (*pin == ' ')
				{
					space_flag = 1;
				}
				path[pout++] = *pin++;
			}
			else
			{
				path[pout++] = '%';
				strncpy(&path[pout], tmp_hex, 2);
				pout = pout + 2;
				pin++;
			}
		}
	}

	// ";" params  == *(";" param)
	while (*pin == ';')
	{
		path[pout++] = *pin++;
		while (EASOU_PARAM[(unsigned char) *pin])
		{
			tmp_hex[0] = '\0';
			if ((*pin == '%') && (*(pin + 1) != '\0') && (*(pin + 2) != '\0'))
			{
				strncpy(hex, ++pin, 2);
				strncpy(tmp_hex, hex, 2);
				pin++;
				escape_char = to_char(hex);
				if (escape_char == 0)
				{
					pin--;
					path[pout++] = '%';
					continue;
				}
				*pin = escape_char;
			}
			if (EASOU_PARAM[(unsigned char) *pin] && (tmp_hex[0] == '\0' || !EASOU_RESERVED[(unsigned char) *pin]))
			{
				if (*pin == ' ')
				{
					space_flag = 1;
				}
				path[pout++] = *pin++;
			}
			else
			{
				path[pout++] = '%';
				strncpy(&path[pout], tmp_hex, 2);
				pout = pout + 2;
				pin++;
			}
		}
	}

	//"?" query
	while (*pin == '?')
	{
		path[pout++] = *pin++;
		while (EASOU_UCH_RES[(unsigned char) *pin])
		{
			tmp_hex[0] = '\0';
			if ((*pin == '%') && (*(pin + 1) != '\0') && (*(pin + 2) != '\0'))
			{
				strncpy(hex, ++pin, 2);
				strncpy(tmp_hex, hex, 2);
				pin++;
				escape_char = to_char(hex);
				if (escape_char == 0)
				{
					pin--;
					path[pout++] = '%';
					continue;
				}
				*pin = escape_char;
			}
			if (EASOU_UCH_RES[(unsigned char) *pin] && (tmp_hex[0] == '\0' || !EASOU_RESERVED[(unsigned char) *pin]))
			{
				if (*pin == ' ')
				{
					space_flag = 1;
				}
				path[pout++] = *pin++;
			}
			else
			{
				path[pout++] = '%';
				strncpy(&path[pout], tmp_hex, 2);
				pout = pout + 2;
				pin++;
			}
		}
	}
	//if(*pin != 0)
	if (*pin != 0 && *pin != '#')
		return 0;
	path[pout] = 0;
	if (pout > len)
		return 0;
	if (space_flag == 1)
	{
		return space_escape_path_ex2(path, MAX_PATH_LEN);;
	}
	else
	{
		return 1;
	}
	//check abs_path over
}


//加入这个函数的原因是因为.
//ps想不省去fragment来进行归一化.
static int single_path_ex2_nointer_special_for_ps_20110221(char *path, int len)
{
	char *pin;
	int pout;
	char hex[2];
	char tmp_hex[2];
	char escape_char;
	int space_flag = 0;
	//len = strlen(path);
	pin = path;
	pout = 0;
	//check abs_path
	if (*pin == '/')
	{
		path[pout++] = *pin++;
	}

	//path
	while (EASOU_PCHAR[(unsigned char) *pin])
	{
		tmp_hex[0] = '\0';
		if ((*pin == '%') && (*(pin + 1) != '\0') && (*(pin + 2) != '\0'))
		{
			strncpy(hex, ++pin, 2);
			strncpy(tmp_hex, hex, 2);
			pin++;
			escape_char = to_char(hex);
			if (escape_char == 0)
			{
				pin--;
				path[pout++] = '%';
				continue;
			}
			*pin = escape_char;
		}
		if (EASOU_PCHAR[(unsigned char) *pin] && (tmp_hex[0] == '\0' || !EASOU_RESERVED[(unsigned char) *pin]))
		{
			if (*pin == ' ')
			{
				space_flag = 1;
			}
			path[pout++] = *pin++;
		}
		else
		{
			path[pout++] = '%';
			strncpy(&path[pout], tmp_hex, 2);
			pout = pout + 2;
			pin++;
		}
	}

	while (*pin == '/')
	{
		path[pout++] = *pin++;
		while (EASOU_PCHAR[(unsigned char) *pin])
		{
			tmp_hex[0] = '\0';
			if ((*pin == '%') && (*(pin + 1) != '\0') && (*(pin + 2) != '\0'))
			{
				strncpy(hex, ++pin, 2);
				strncpy(tmp_hex, hex, 2);
				pin++;
				escape_char = to_char(hex);
				if (escape_char == 0)
				{
					pin--;
					path[pout++] = '%';
					continue;
				}
				*pin = escape_char;
			}

			if (EASOU_PCHAR[(unsigned char) *pin] && (tmp_hex[0] == '\0' || !EASOU_RESERVED[(unsigned char) *pin]))
			{
				if (*pin == ' ')
				{
					space_flag = 1;
				}
				path[pout++] = *pin++;
			}
			else
			{
				path[pout++] = '%';
				strncpy(&path[pout], tmp_hex, 2);
				pout = pout + 2;
				pin++;
			}
		}
	}

	// ";" params  == *(";" param)
	while (*pin == ';')
	{
		path[pout++] = *pin++;
		while (EASOU_PARAM[(unsigned char) *pin])
		{
			tmp_hex[0] = '\0';
			if ((*pin == '%') && (*(pin + 1) != '\0') && (*(pin + 2) != '\0'))
			{
				strncpy(hex, ++pin, 2);
				strncpy(tmp_hex, hex, 2);
				pin++;
				escape_char = to_char(hex);
				if (escape_char == 0)
				{
					pin--;
					path[pout++] = '%';
					continue;
				}
				*pin = escape_char;
			}
			if (EASOU_PARAM[(unsigned char) *pin] && (tmp_hex[0] == '\0' || !EASOU_RESERVED[(unsigned char) *pin]))
			{
				if (*pin == ' ')
				{
					space_flag = 1;
				}
				path[pout++] = *pin++;
			}
			else
			{
				path[pout++] = '%';
				strncpy(&path[pout], tmp_hex, 2);
				pout = pout + 2;
				pin++;
			}
		}
	}

	//"?" query
	while (*pin == '?')
	{
		path[pout++] = *pin++;
		while (EASOU_UCH_RES[(unsigned char) *pin])
		{
			tmp_hex[0] = '\0';
			if ((*pin == '%') && (*(pin + 1) != '\0') && (*(pin + 2) != '\0'))
			{
				strncpy(hex, ++pin, 2);
				strncpy(tmp_hex, hex, 2);
				pin++;
				escape_char = to_char(hex);
				if (escape_char == 0)
				{
					pin--;
					path[pout++] = '%';
					continue;
				}
				*pin = escape_char;
			}
			if (EASOU_UCH_RES[(unsigned char) *pin] && (tmp_hex[0] == '\0' || !EASOU_RESERVED[(unsigned char) *pin]))
			{
				if (*pin == ' ')
				{
					space_flag = 1;
				}
				path[pout++] = *pin++;
			}
			else
			{
				path[pout++] = '%';
				strncpy(&path[pout], tmp_hex, 2);
				pout = pout + 2;
				pin++;
			}
		}
	}
	//if(*pin != 0)
	if (*pin != 0 && *pin != '#')
		return 0;
	//补齐最后面的fragment.
	while (*pin && pout < len)
	{
		path[pout] = *pin;
		pin++;
		pout++;
	}
	//长度过大的话.
	if (pout >= len)
	{
		return 0;
	}
	path[pout] = 0;
	if (space_flag == 1)
	{
		return space_escape_path_ex2(path, MAX_PATH_LEN);;
	}
	else
	{
		return 1;
	}
	//check abs_path over
}

int normalize_url_ex2(const char* url, char* buf, size_t buf_size)
{
	char site[MAX_SITE_LEN];
	char port[MAX_PORT_LEN];
	char path[MAX_PATH_LEN];

	if (parse_url_ex2(url, site, MAX_SITE_LEN, port, MAX_PORT_LEN, path, MAX_PATH_LEN) == 0)
		return 0;

	if (normalize_site(site) == 0)
		return 0;
	if (normalize_port(port) == 0)
		return 0;
	//这里调用single_path_ex2_nointer而不调用single_path_ex2
	//single_path_ex2_nointer里没有delete_inter操作,在parse_url_ex2
	//里面已经作过一次parse_url_ex2这里没必要再作一次
	//single_path_ex2为了支持外部程序中直接调用single_path_ex2对未经过parse_url_ex2
	//处理的path进行处理，加上了delete_inter
	if (single_path_ex2_nointer(path, MAX_PATH_LEN) == 0)
		return 0;
	normalize_path_ex2(path);

	if (port[0] == '\0')
	{
		snprintf(buf, buf_size, "%s%s", site, path);
	}
	else
	{
		snprintf(buf, buf_size, "%s:%s%s", site, port, path);
	}
	return 1;
}

int normalize_url_ex2_special_for_ps_20110221(const char* url, char* buf, size_t buf_size)
{
	//加入这个函数的原因
	//可以参看 normalize_url_ex2_special_for_ps_20101026 这个函数.
	char site[MAX_SITE_LEN];
	char port[MAX_PORT_LEN];
	char path[MAX_PATH_LEN];

	if (parse_url_ex2(url, site, MAX_SITE_LEN, port, MAX_PORT_LEN, path, MAX_PATH_LEN) == 0)
		return 0;

	if (normalize_site_special_for_ps_20101026(site) == 0)
		return 0;
	if (normalize_port(port) == 0)
		return 0;
	//这里调用single_path_ex2_nointer而不调用single_path_ex2
	//single_path_ex2_nointer里没有delete_inter操作,在parse_url_ex2
	//里面已经作过一次parse_url_ex2这里没必要再作一次
	//single_path_ex2为了支持外部程序中直接调用single_path_ex2对未经过parse_url_ex2
	//处理的path进行处理，加上了delete_inter
	//这里需要把fragment也作为归一化的内容里面去.
	if (single_path_ex2_nointer_special_for_ps_20110221(path, MAX_PATH_LEN) == 0)
		return 0;
	normalize_path_ex2(path);

	if (port[0] == '\0')
	{
		snprintf(buf, buf_size, "%s%s", site, path);
	}
	else
	{
		snprintf(buf, buf_size, "%s:%s%s", site, port, path);
	}
	return 1;
}

void get_static_part_ex2(const char *url, char *staticurl, size_t staticurl_size)
{
	char *p;
	char buffer[MAX_URL_LEN];

	assert(strlen(url) < MAX_URL_LEN);
	strcpy(buffer, url);

	p = strchr(buffer, '?');
	if (p != NULL)
	{
		*p = '\0';
	}
	p = strchr(buffer, ';');
	if (p != NULL)
	{
		*p = '\0';
	}
	snprintf(staticurl, staticurl_size, "%s", buffer);
}

//actually not modified ...
int isnormalized_url_ex2(const char *url)
{
	const char *pin;
	char port[6];
	int len;
	int k = 0;
	unsigned uport;
	int dotcount = 0;
	len = strlen(url);
	if (strncasecmp(url, "http://", 7) == 0 || strncasecmp(url, "https://", 8) == 0)
		return 0; // should not start with "http://"
	else
		pin = url;

	//check host name
	while (URL_LET_DIG_HY[(unsigned char) *pin])
	{ // [0-9a-zA-Z\-]
		if (URL_UP_LET[(unsigned char) *pin]) //[A-Z]
			return 0; // site should be lower case.
		pin++;
	}
	pin--;
	if (!URL_LET_DIGIT[(unsigned char) *pin++]) //[0-9a-zA-Z]
		return 0; //not end with "-"
	while (*pin == '.')
	{
		dotcount++;
		pin++;
		while (URL_LET_DIG_HY[(unsigned char) *pin])
		{
			if (URL_UP_LET[(unsigned char) *pin])
				return 0; // site should be lower case.
			pin++;
		}
		pin--;
		if (!URL_LET_DIGIT[(unsigned char) *pin++])
			return 0;
	}
	if (dotcount == 0)
	{
		return 0;
	}
	//check host name over

	//check port
	if (*pin == ':')
	{
		pin++;
		if (*pin != '/')
		{
			while (URL_DIGIT[(unsigned char) *pin])
			{
				port[k++] = *pin++;
				if (k > 5)
					return 0;
			}
			port[k] = 0;
			if (*pin != '/')
				return 0;

			uport = atoi(port);
			if (uport <= 0 || uport > 65535)
				return 0;

			if (uport == 80)
				return 0; // should omit port 80
		}
		else
			return 0;
	}
	else if ((*pin != '/'))
		return 0;
	//check port over

	//check abs_path
	if (*pin == '/')
	{
		pin++;
		//path
		while (URL_PCHAR[(unsigned char) *pin])
		{
			pin++;
		}
		while (*pin == '/')
		{
			pin++;
			while (URL_PCHAR[(unsigned char) *pin])
			{
				pin++;
			}
		}

		// ";" params  == *(";" param)
		while (*pin == ';')
		{
			pin++;
			while (URL_PARAM[(unsigned char) *pin])
			{
				pin++;
			}
		}

		//"?" query
		while (*pin == '?')
		{
			pin++;
			while (URL_UCH_RES[(unsigned char) *pin])
			{
				pin++;
			}
		}
		if (*pin != 0)
			return 0;
		if ((pin - url) >= len)
			return 0;
		return 1;
	}
	else
		return 0;

	//check abs_path over
}

void normalize_path_ex2(char *path)
{
	char *p, *q;
	char tmp[MAX_URL_LEN];
	char *pend;
	size_t len = 0;

	assert(*path == '/');
	len = strlen(path);
	assert(len < MAX_URL_LEN);

	replace_slash(path);

	strcpy(tmp, path);

	pend = strchr(tmp, '?');
	if (pend == NULL)
	{
		pend = strchr(tmp, ';');
	}
	if (pend == NULL)
	{
		pend = tmp + strlen(tmp);
	}

	p = tmp + 1;
	q = path;
	while (p != pend)
	{
		/* normal */
		if (*q != '/')
		{
			q++;
			*q = *p++;
			continue;
		}

		/* /./ */
		if (strncmp(p, "./", 2) == 0)
		{
			p += 2;
			continue;
		}

		/* /../ */
		if (strncmp(p, "../", 3) == 0)
		{
			p += 3;
			if (q == path)
			{
				continue;
			}
			/* change q to prev '/' */
			q--;
			while (*q != '/')
				q--;
			continue;
		}

		/* // */
		if (*p == '/')
		{
			p++;
			continue;
		}

		/* "/." */
		if (strncmp(p, ".", pend - p) == 0)
		{
			p++;
			continue;
		}

		/* "/.." */
		if (strncmp(p, "..", pend - p) == 0)
		{
			p += 2;
			if (q == path)
			{
				continue;
			}
			/* change q to prev '/' */
			q--;
			while (*q != '/')
				q--;
			continue;
		}

		q++;
		*q = *p++;

	}
	q++;
	strcpy(q, p);
	return;
}

int single_path_ex2(char *path)
{
	int len = delete_inter(path); //delete '\r' '\n';
	return single_path_ex2_nointer(path, len);
}

int check_url_ex2(char *url)
{
	char *pin;
	char *pout;
	char port[6];
	int k = 0;
	int dotcount = 0;
	char *url_start;
	int len;
	int space_flag = 0;
	len = delete_inter(url); //delete \r\n;
	if (strncasecmp(url, "http://", 7) == 0)
	{
		pin = url + 7;
	}
	else if (strncasecmp(url, "https://", 8) == 0)
	{
		pin = url + 8;
	}
	else
	{
		pin = url;
	}
	pout = pin;
	url_start = pin;
	//check host name
	while (URL_LET_DIG_HY[(unsigned char) *pin] || *pin == '%')
	{
		if (URL_UP_LET[(unsigned char) *pin])
			*pin += 32;
		else
		{
			if (*pin == '%')
			{
				if (pin[1] != '\0' && pin[2] != '\0')
				{
					unsigned char escape_char = to_char(pin + 1);
					if (URL_LET_DIG_HY[escape_char])
					{
						if (URL_UP_LET[escape_char])
						{
							pin[2] = escape_char + 32;
						}
						else
						{
							pin[2] = escape_char;
						}
						pin += 2;
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
		}
		*pout++ = *pin++;
	}
	if (pin == url_start)
		return 0;

	pin--;
	if (!URL_LET_DIGIT[(unsigned char) *pin++])
		return 0;
	while (*pin == '.')
	{
		dotcount++;
		*pout++ = *pin++;
		while (URL_LET_DIG_HY[(unsigned char) *pin] || (*pin == '%'))
		{
			if (URL_UP_LET[(unsigned char) *pin])
				*pin += 32;
			else
			{
				if (*pin == '%')
				{
					if (pin[1] != '\0' && pin[2] != '\0')
					{
						unsigned char escape_char = to_char(pin + 1);
						if (URL_LET_DIG_HY[escape_char] == 1)
						{
							if (URL_UP_LET[escape_char])
							{
								pin[2] = escape_char + 32;
							}
							else
							{
								pin[2] = escape_char;
							}
							pin += 2;
						}
						else
						{
							return 0;
						}
					}
					else
					{
						return 0;
					}
				}
			}
			*pout++ = *pin++;
		}
		pin--;
		if (!URL_LET_DIGIT[(unsigned char) *pin++])
			return 0;
	}
	if (dotcount == 0)
	{
		return 0;
	}
	//check host name over

	//check port
	if (*pin == ':')
	{
		pin++;
		if (*pin != '/')
		{
			while (URL_DIGIT[(unsigned char) *pin])
			{
				port[k++] = *pin++;
				if (k > 5)
					return 0;
			}
			port[k] = 0;
			if (*pin != '/')
				return 0;
			memcpy(pout, pin - k - 1, k + 1);
			if (strcmp(port, "80") != 0)
			{
				pout += (k + 1);
			}
		}
	}
	else if ((*pin != '/') && (*pin != '\0'))
		return 0;
	//check port over

	//check abs_path
	if (*pin == '/')
	{
		*pout++ = *pin++;
		//path
		while (URL_PCHAR[(unsigned char) *pin])
		{
			space_flag |= ((*pout++ = *pin++) == ' ');
		}

		while (*pin == '/')
		{
			*pout++ = *pin++;
			while (URL_PCHAR[(unsigned char) *pin])
			{
				space_flag |= ((*pout++ = *pin++) == ' ');
			}
		}

		// ";" params  == *(";" param)
		while (*pin == ';')
		{
			*pout++ = *pin++;
			while (URL_PARAM[(unsigned char) *pin])
			{
				space_flag |= ((*pout++ = *pin++) == ' ');
			}
		}

		//"?" query
		while (*pin == '?')
		{
			*pout++ = *pin++;
			while (URL_UCH_RES[(unsigned char) *pin])
			{
				space_flag |= ((*pout++ = *pin++) == ' ');
			}
		}
		if (*pin != 0)
			return 0;
		*pout = 0;
		if (pout - url > len)
			return 0;
		if (space_flag == 1)
			return space_escape_ex2(url, MAX_URL_LEN, MAX_SITE_LEN, MAX_PATH_LEN);
		else
			return 1;
	}
	else if (*pin != 0)
		return 0;
	*pout = '\0';
	if (space_flag == 1)
		return space_escape_ex2(url, MAX_URL_LEN, MAX_SITE_LEN, MAX_PATH_LEN);
	else
		return 1;
	//check abs_path over
}


/*获得url深度*/
int get_url_depth(const char * url )
{
	const char c = '/';
	const char * looper = url;
	if (strncasecmp(looper, "http://", 7) == 0)
	{
		looper = looper + 7;
	}
	else if (strncasecmp(looper, "https://", 8) == 0)
	{
		looper = looper + 8;
	}
	int num = 0;
	while (*looper)
	{
		if (*looper == c)
		{
			num++;
		}
		looper++;
	}
	//最后一个'/'不计算在内
	if (num > 0 && *(looper - 1) == c)
	{
		num--;
	}
	assert(num >=0 );
	return num;
}

static int to_char_inner(char hex[2])
{
	int i, r;
	int h[2];
	for (i = 0; i < 2; i++)
	{
		switch (hex[i])
		{
		case 'a':
		case 'A':
			h[i] = 10;
			break;
		case 'b':
		case 'B':
			h[i] = 11;
			break;
		case 'c':
		case 'C':
			h[i] = 12;
			break;
		case 'd':
		case 'D':
			h[i] = 13;
			break;
		case 'e':
		case 'E':
			h[i] = 14;
			break;
		case 'f':
		case 'F':
			h[i] = 15;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			h[i] = hex[i] - 48;
			break;
		default:
			return 0;
		}
	}
	r = 16 * h[0] + h[1];
	return r;
}


/**
 * url归一化
 */
int single_path_inner(char *path)
{
	char *pin;
	int pout;
	char hex[2];
	char tmp_hex[2];
	char flag_0d_0a = 0;

	pin = path;
	pout = 0;

	// first operation on '#'
	char *p = path;
	p = strchr(p, '#');
	while (p)
	{
		if (p == path)
		{
			*p = 0;
			break;
		}
		else if (*(p - 1) != '&')
		{
			*p = 0;
			break;
		}
		p++;
		if (*p == '\0')
			break;
		p = strchr(p, '#');
	}
	// end operation on '#'

	//check abs_path

	if (*pin == '/')
	{
		path[pout++] = *pin++;
	}

	//path
	while (EASOU_PCHAR[(unsigned char) *pin] || (*pin == '#') || *pin == 0x0d || *pin == 0x0a)
	{
		tmp_hex[0] = '\0';
		flag_0d_0a = 0;
		if ((*pin == '%') && (*(pin + 1) != 0) && (*(pin + 2) != 0))
		{
			strncpy(hex, ++pin, 2);
			strncpy(tmp_hex, hex, 2);
			pin++;
			char escape_char = to_char_inner(hex);
			if (escape_char == 0)
			{ // illeage escape
				pin--;
				path[pout++] = '%';
				continue;
			}
			else
			{
				*pin = escape_char;
				if (*pin == 0x0d || *pin == 0x0a)
				{
					flag_0d_0a = 1;
				}
			}
		}
		if ((EASOU_PCHAR[(unsigned char) *pin] || (*pin == '#'))
				&& (tmp_hex[0] == '\0' || !EASOU_RESERVED[(unsigned char) *pin]))
		{
			path[pout++] = *pin++;
		}
		else
		{
			if ((*pin == 0x0d || *pin == 0x0a) && 0 == flag_0d_0a)
			{ // remove 0d 0a
				pin++;
				continue;
			}
			path[pout++] = '%';
			strncpy(&path[pout], tmp_hex, 2);
			pout = pout + 2;
			pin++;
		}
	}

	while (*pin == '/')
	{
		path[pout++] = *pin++;
		while (EASOU_PCHAR[(unsigned char) *pin] || (*pin == '#') || (0x0d == *pin) || (0x0a == *pin))
		{

			tmp_hex[0] = '\0';
			flag_0d_0a = 0;
			if ((*pin == '%') && (*(pin + 1) != 0) && (*(pin + 2) != 0))
			{
				strncpy(hex, ++pin, 2);
				strncpy(tmp_hex, hex, 2);
				pin++;
				char escape_char = to_char_inner(hex);
				if (escape_char == 0) // illeage escape
				{
					pin--;
					path[pout++] = '%';
					continue;
				}
				else
				{
					*pin = escape_char;
					if (*pin == 0x0d || *pin == 0x0a)
					{
						flag_0d_0a = 1;
					}
				}
			}

			if ((EASOU_PCHAR[(unsigned char) *pin] || (*pin == '#'))
					&& (tmp_hex[0] == '\0' || !EASOU_RESERVED[(unsigned char) *pin]))
			{
				path[pout++] = *pin++;
			}
			else
			{
				if (flag_0d_0a == 0 && ((0x0d == *pin) || (0x0a == *pin)))
				{
					*pin++;
					continue;
				}
				path[pout++] = '%';
				strncpy(&path[pout], tmp_hex, 2);
				pout = pout + 2;
				pin++;
			}
		}
	}

	// ";" params  == *(";" param)
	while (*pin == ';')
	{
		path[pout++] = *pin++;
		while (EASOU_PARAM[(unsigned char) *pin] || (*pin == '#') || (0x0d == *pin) || (0x0a == *pin))
		{
			tmp_hex[0] = '\0';
			flag_0d_0a = 0;
			if ((*pin == '%') && (*(pin + 1) != 0) && (*(pin + 2) != 0))
			{
				strncpy(hex, ++pin, 2);
				strncpy(tmp_hex, hex, 2);
				pin++;
				char escape_char = to_char_inner(hex);
				if (escape_char == 0)
				{
					pin--;
					path[pout++] = '%';
					continue;
				}
				else
				{
					*pin = escape_char;
					if (0x0d == *pin || 0x0a == *pin)
					{
						flag_0d_0a = 1;
					}
				}
			}
			if ((EASOU_PARAM[(unsigned char) *pin] || (*pin == '#'))
					&& ((tmp_hex[0] == '\0') | !EASOU_RESERVED[(unsigned char) *pin]))
			{
				path[pout++] = *pin++;
			}
			else
			{
				if (((0x0d == *pin) || (0x0a == *pin)) && 0 == flag_0d_0a)
				{
					pin++;
					continue;
				}
				path[pout++] = '%';
				strncpy(&path[pout], tmp_hex, 2);
				pout = pout + 2;
				pin++;
			}
		}
	}

	//"?" query
	while (*pin == '?')
	{
		path[pout++] = *pin++;
		while (EASOU_UCH_RES[(unsigned char) *pin] || (*pin == '#') || (0x0d == *pin) || (0x0a == *pin))
		{
			tmp_hex[0] = '\0';
			flag_0d_0a = 0;
			if ((*pin == '%') && (*(pin + 1) != 0) && (*(pin + 2) != 0))
			{
				strncpy(hex, ++pin, 2);
				strncpy(tmp_hex, hex, 2);
				pin++;
				char escape_char = to_char_inner(hex);
				if (escape_char == 0)
				{
					pin--;
					path[pout++] = '%';
					continue;
				}
				else
				{
					*pin = escape_char;
					if (0x0d == *pin || 0x0a == *pin)
					{
						flag_0d_0a = 1;
					}
				}
			}
			if ((EASOU_UCH_RES[(unsigned char) *pin] || (*pin == '#'))
					&& (tmp_hex[0] == '\0' || !EASOU_RESERVED[(unsigned char) *pin]))
			{
				path[pout++] = *pin++;
			}
			else
			{
				if ((0 == flag_0d_0a) && ((0x0d == *pin) || (0x0a == *pin)))
				{
					pin++;
					continue;
				}
				path[pout++] = '%';
				strncpy(&path[pout], tmp_hex, 2);
				pout = pout + 2;
				pin++;
			}
		}
	}
	if (*pin != 0 && *pin != '#')
	{
		return 0;
	}
	path[pout] = 0;
	if (pout > 256)
	{
		return 0;
	}
	return 1;
}

/*
 * unparse url
 */
void combine_url_inner(char *url, char *domain, char *port, char *path)
{
	if (*port == '\0')
	{
		if (domain != NULL)
			snprintf(url, MAX_URL_LEN, "%s%s", domain, path);
		else
			snprintf(url, MAX_URL_LEN, "%s", path);
	}
	else
	{
		snprintf(url, MAX_URL_LEN, "%s:%s%s", domain, port, path);
	}
	//todo
//	html_derefrence_text(url, 0, MAX_URL_LEN-1, url);
}

/*
 * is network path
 */
static int is_net_path(const char *path)
{
	if (strncmp(path, "//", 2) == 0)
		return 1;
	return 0;
}
/*
 * is absolute path
 */
static int is_abs_path(const char *path)
{
	if (*path == '/')
		return 1;
	return 0;
}

/*
 * is relative path
 */
static int is_rel_path(const char *url)
{
	const char *p;
	if (is_url(url))
		return 0;
	if (is_net_path(url))
		return 0;
	if (is_abs_path(url))
		return 0;
	p = strchr(url, ':');
	if (p != NULL && p - url <= 10)
		//10 is the length of the longest shemas javascript
		return 0;
	return 1;
}

/*
 * get directory of path
 */
void remove_path_file_name(char *path)
{
	char *p;
	p = strchr(path, '?');
	if (p != NULL)
	{
		*p = '\0';
	}
	p = strrchr(path, '/');
	assert(p != NULL);
	*(p + 1) = '\0';
}

/*页面内的相对URL拼成一个绝对URL*/
int combine_url (char *result_url, const char *base_url, const char *relative_url)
{
	char domain[MAX_SITE_LEN], base_domain[MAX_SITE_LEN];
	char port[MAX_PORT_LEN], base_port[MAX_PORT_LEN];
	char path[MAX_PATH_LEN], base_path[MAX_PATH_LEN];
	char relpath[MAX_PATH_LEN];
	if (!parse_url(base_url, base_domain, base_port, base_path))
	{
		return -1;
	}
	if (is_url(relative_url))
	{
		if (strlen(relative_url) >= MAX_URL_LEN || !parse_url(relative_url, domain, port, path)
				|| !single_path_inner(path))
		{
			return -1;
		}
		normalize_path(path);
		combine_url_inner(result_url, domain, port, path);
	}
	else if (is_net_path(relative_url))
	{
		if (strlen(relative_url) >= MAX_PATH_LEN)
			return -1;
		snprintf(path, sizeof(path), "%s", relative_url);
		if (!single_path_inner(path))
			return -1;
		normalize_path(path);
		char *p = path;
		while (*p == '/')
			p++;
		if (*p != '\0' && p != path)
		{
			int l = strlen(p);
			memmove(path, p, l);
			path[l] = '\0';
		}
		port[0] = '\0';
		combine_url_inner(result_url, NULL, port, path);
	}
	else if (is_abs_path(relative_url))
	{
		if (strlen(relative_url) >= MAX_PATH_LEN)
			return -1;
		snprintf(path, sizeof(path), "%s", relative_url);
		if (!single_path_inner(path))
			return -1;
		normalize_path(path);
		combine_url_inner(result_url, base_domain, base_port, path);
	}
	else if (is_rel_path(relative_url))
	{
		if (strlen(base_path) + strlen(relative_url) >= MAX_PATH_LEN)
		{
			return -1;
		}
		if (*relative_url != '?' && *relative_url != ';' && *relative_url != '#')
		{
			snprintf(relpath, sizeof(relpath), "%s", base_path);
			remove_path_file_name(relpath);
			snprintf(path, MAX_PATH_LEN, "%s%s", relpath, relative_url);
		}
		else
		{
			snprintf(relpath, sizeof(relpath), "%s", base_path);
			char ch = relative_url[0];
			char *p = NULL;
			if ((p = strchr(relpath, ch)) != NULL)
				*p = 0;
			if (relative_url[0] != '#')
				snprintf(path, MAX_PATH_LEN, "%s%s", relpath, relative_url);
			else if (strlen(relative_url) > 1)
				snprintf(path, MAX_PATH_LEN, "%s", relpath);
			else
				// src is a single '#'
				return -1;
		}
		if (!single_path_inner(path))
		{
			return -1;
		}
		normalize_path(path);
		combine_url_inner(result_url, base_domain, base_port, path);
	}
	else
	{
		return -1;
	}
	//check syntax of url, lower upper char in site name
	if (check_url(result_url) == 0)
	{
		return -1;
	}
	return 0;
}

/**
 * @brief 是不是类似首页的url
**/
int is_like_top_url(const char *url)
{
	if (NULL == url)
	{
		return 0;
	}
	char urlBuf[MAX_URL_LEN];
	int inputLen = strlen(url);
	int newLen = inputLen < MAX_URL_LEN ? inputLen : (MAX_URL_LEN - 1);
	memcpy(urlBuf, url, newLen);
	*(urlBuf + newLen) = 0;
	if (check_url(urlBuf) == 0)
		return 0;
	char buf[16];
	buf[0] = '\0';
	if (isdyn(url)) // filter dynamic url
		return 0;
	char path[MAX_PATH_LEN];
	path[0] = '\0';
	if (get_path(url, path) == 0)
		return 0;
	if (strcmp(path, "/") == 0)
		return 1;
	char *p = strchr(path + 1, '/');
	if (p)
	{ // exist 2nd '/'
		if (*(p + 1) == '\0') // like "abc.com/.../"
			return 1;
		if (strchr(p + 1, '/') == NULL)
		{ // no 3rd '/'
			snprintf(buf, sizeof(buf), "%s", p + 1);
			trans2lower(buf, buf);
			if (strstr(buf, "index") || strstr(buf, "main"))
				return 1; // like "abc.com/.../index... or main..."
			else
				return 0; // like "abc.com/.../..."
		}
		return 0;
	}
	if (strlen(path) > 15) // no 2nd '/'
		return 0;
	return 1;
}


static unsigned char SITE_CHAR[256]={
		0,1,1,1,1,1,1,1,1,0,0,1,1,0,1,1,  //0-        '\0', '\r', '\n','\t'
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //16-
                                      0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, //32-    ' ', '/'
                                      1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1, //48-    ':'
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //64-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //80-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //96-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //112-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //128-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //144-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //160-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //176-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //192-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //208-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //224-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };//240-

static unsigned char PORT_CHAR[256]={
		0,1,1,1,1,1,1,1,1,0,0,1,1,0,1,1,  //0-        '\0', '\r', '\n','\t'
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //16-
                                      0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, //32-    ' ', '/'
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //48-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //64-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //80-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //96-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //112-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //128-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //144-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //160-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //176-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //192-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //208-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //224-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };//240-

static unsigned char PATH_CHAR[256]={
		0,1,1,1,1,1,1,1,1,0,0,1,1,0,1,1,  //0-        '\0', '\r', '\n','\t'
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //16-
                                      0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1, //32-    ' ', '&'
                                      1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0, //48-    ';', '?'
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //64-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //80-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //96-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //112-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //128-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //144-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //160-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //176-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //192-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //208-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //224-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };//240-

static unsigned char DIR_CHAR[256]={
		0,1,1,1,1,1,1,1,1,0,0,1,1,0,1,1,  //0-        '\0', '\r', '\n','\t'
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //16-
                                      0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0, //32-    ' ', '&', '/'
                                      1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0, //48-    ';', '?'
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //64-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //80-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //96-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //112-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //128-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //144-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //160-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //176-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //192-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //208-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //224-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };//240-

static unsigned char PARAM_CHAR[256]={
		0,1,1,1,1,1,1,1,1,0,0,1,1,0,1,1,  //0-    '\0', '\r', '\n','\t'
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //16-
                                      0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //32-    ' ',
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //48-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //64-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //80-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //96-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //112-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //128-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //144-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //160-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //176-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //192-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //208-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //224-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };//240-

static unsigned char PARAM_NAME_CHAR[256]={
		0,1,1,1,1,1,1,1,1,0,0,1,1,0,1,1,  //0-    '\0', '\r', '\n','\t'
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //16-
                                      0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1, //32-    ' ', '&'
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0, //48-    '=', '?'
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //64-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //80-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //96-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //112-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //128-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //144-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //160-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //176-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //192-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //208-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //224-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };//240-

static unsigned char PARAM_VALUE_CHAR[256]={
		0,1,1,1,1,1,1,1,1,0,0,1,1,0,1,1,  //0-    '\0', '\r', '\n','\t'
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //16-
                                      0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1, //32-    ' ', '&'
                                      1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0, //48-    ';', '?'
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //64-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //80-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //96-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //112-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //128-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //144-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //160-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //176-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //192-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //208-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //224-
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };//240-


const char *parse_url_inner(url_t *url, const char *url_buffer, int length)
{
	const char *p;
	const char *end;
	int sflag = 0; //flag for 'jsessionid' url
	const char *s_end = NULL; //flag for the end of sessionid
	const char *separate = NULL;
//    assert(length <= UL_MAX_URL_LEN);
	if (length > MAX_URL_LEN)
		length = MAX_URL_LEN; // to prevent invalid url input, cause border overrun.
	end = url_buffer + length;
	p = url_buffer;

	//site
	if (strncasecmp(p, "http://", 7) == 0)
	{
		p = p + 7;
	}
	if (strncasecmp(p, "https://", 8) == 0)
	{
		p = p + 8;
	}
	url->site = p;
	while (p < end && SITE_CHAR[(unsigned char) *p])
	{ //not ':', '/', ' ', '\r', '\n', '\0','\t'
		p++;
	}
	url->site_len = p - url->site;
	if (url->site_len <= 0 || url->site_len >= MAX_SITE_LEN)
	{
//    	Warn("Empty or too long site found in url buffer");
		return NULL;
	}
	//port
	if (p < end && *p == ':')
	{
		p++;
		url->port = p;
		while (p < end && PORT_CHAR[(unsigned char) *p])
		{ //not '/', ' ', '\r', '\n', '\0','\t'
			p++;
		}
		url->port_len = p - url->port;
		if (url->port_len < 0 || url->port_len >= MAX_PORT_LEN)
		{
//        	Warn("too long port found in url buffer");
			return NULL;
		}
	}
	else
	{
		url->port = NULL;
		url->port_len = 0;
	}
	//path
	url->dir_num = 0;
	url->path = p;
	while (p < end && PATH_CHAR[(unsigned char) *p])
	{ //not '?', '&', ';', ' ', '\r', '\n', '\0','\t'
		//too many dir, skip this url
		if (url->dir_num == MAX_DIR_NUM)
		{
//        	Warn("Too deep url found in url buffer");
			return NULL;
		}
		//one dir
		assert(*p == '/');
		p++;
		url->dir[url->dir_num] = p;
		while (p < end && DIR_CHAR[(unsigned char) *p])
		{ //not '?', '&', ';', '/', ' ', '\r', '\n', '\0','\t'
			p++;
		}
		url->dir_len[url->dir_num] = p - url->dir[url->dir_num];
		url->dir_num++;
	}
	if (url->dir_num == 0)
	{
//    	Warn("Parse path error in url buffer");
		return NULL;
	}

	//param        (name=value&name=value)

	url->param_num = 0;
	separate = p;
	while (p < end && PARAM_CHAR[(unsigned char) *p])
	{ //not ' ', '\r', '\n', '\0','\t'
		if (url->param_num == MAX_PARAM_NUM)
		{
//            ul_writelog(LOG_WARNING, "Too many parameter url found in url buffer");
			return NULL;
		}

		assert(*p == '?' || *p == ';' || *p == '&');

		//add by yangmin
		if (*p == '&' && strncasecmp(p, "&amp;", 5) == 0)
		{
			p = p + 4;
		}
		p++;
		url->name[url->param_num] = p;

		while (p < end && PARAM_NAME_CHAR[(unsigned char) *p])
		{ // not '=', '&', ' ', '\r', '\n', '\0','\t'
			p++;
		}
		url->name_len[url->param_num] = p - url->name[url->param_num];

		//add by yangmin
		if (strncmp(url->name[url->param_num], "jsessionid", 10) == 0 && url->param_num == 0 && *separate == ';')
		{
			sflag = 1;
		}

		if (*p == '=')
		{
			p++;
			url->value[url->param_num] = p;
			while (p < end && PARAM_VALUE_CHAR[(unsigned char) *p])
			{ // not '&', ' ', '\r', '\n', '\0','\t'
				p++;
			}
			url->value_len[url->param_num] = p - url->value[url->param_num];
			//match jsession pattern
			if (sflag == 1)
			{
				if ((s_end = strchr(url->value[url->param_num], '?')) != NULL
						&& s_end - url->value[url->param_num] < url->value_len[url->param_num])
				{
					url->value_len[url->param_num] = s_end - url->value[url->param_num];
					p = s_end;
					url->param_num++;
					sflag = 0; //reset flag
					continue;
				}
			}
		}
		else
		{
			url->value[url->param_num] = url->name[url->param_num];
			url->value_len[url->param_num] = url->name_len[url->param_num];
			url->name_len[url->param_num] = 0;
		}
		url->param_num++;
	}
	url->path_len = p - url->path;
	if (url->path_len < 0 || url->path_len >= MAX_PATH_LEN)
	{
//        Warn("too long path found in url buffer");
		return NULL;
	}
	return p;
}

bool is_home_page(const char* url, const int urlLen)
{
	const char *begin = NULL;
	const char *slashpos = NULL;

	assert(url != NULL);
	begin = url;
	if (strncmp(begin, "http://", 7) == 0)
	{
		begin += 7;
	}
	else if (strncmp(begin, "https://", 8) == 0)
	{
		begin += 8;
	}
	// some  not invaild url is not suited
	if ((slashpos = strchr(begin, '/')) == NULL)
	{
		return true;
	}
	else
	{
		if ((*(slashpos + 1)) == 0)
		{
			return true;
		}
		else if (strncmp(slashpos + 1, "index", 5) == 0 || strncmp(slashpos + 1, "main", 4) == 0
				|| strncmp(slashpos + 1, "default", 7) == 0)
		{

			if (strchr(slashpos + 1, '?') == NULL && strchr(slashpos + 1, '/') == NULL)
				return true;
		}
	}
	return false;
}


/**
 * @brief 判断url是否是目录；1：目录；0：非目录
 */
int is_dir_url(const char *url)
{
	if (url == NULL)
		return 0;
	char urlBuf[MAX_URL_LEN];
	int inputLen = strlen(url);
	int newLen = inputLen < MAX_URL_LEN ? inputLen : (MAX_URL_LEN - 1);
	memcpy(urlBuf, url, newLen);
	*(urlBuf + newLen) = 0;
	if (check_url(urlBuf) == 0)
		return 0;
	const char *p = url;
	if (url[strlen(url) - 1] == '/')
		return 1;
	if (strncasecmp(url, "http://", 7) == 0)
		p += 7;
	else if (strncasecmp(url, "https://", 8) == 0)
			p += 8;
	if (strchr(p, '/') != NULL)
		return 0;
	return 1;
}

bool has_not_ascii_char(const unsigned char* url)
{
	const unsigned char* pch = url;
	while (*pch++ != '\0')
	{
		if (*pch > 0x7F)
		{
			return true;
		}
	}
	return false;
}

int urlstr_to_regexstr(const char *url, int url_len, char *buf, int buf_len)
{
	const char *p = url;
	const char *pEnd = url + url_len;

	*buf = 0;
	char *pSave = buf;
	int left = buf_len - 1;

	int savelen = 0;
	int status = 0;
	int step;

	static const char *digit_common = "[0-9]+";
	static const int digit_common_len = 6;

	const char *pFilePos = NULL;
	const char *pParamPos = NULL;
	const char *pSitePos = NULL;
	const char *pSegStartPos = NULL;
	const char *pSegFinishPos = NULL;
	while (*p && p < pEnd)
	{
		step = 1;
		if (status == 0)
		{//http
			if (strncmp(p, "http://", 7) == 0)
			{
				step = 7;
				status = 1;
				pSitePos = p + 7;
			}
			else
			{
				return -1;
			}
		}
		else if (status == 1)
		{//site
			if (*p == '/')
				status = 10;
			else
			{
				int digit = 0;
				int alpha = 0;
				const char *pTmp = p;
				while (*pTmp && *pTmp != '.' && *pTmp != '/')
				{
					if (isdigit(*pTmp))
						digit++;
					else if (isalpha(*pTmp) || *pTmp == '%')
						alpha++;
					else
						break;
					pTmp++;
				}
				if (alpha == 0 && digit >= 5)
				{
					memcpy(pSave, "[0-9]+", 6);
					pSave += 6;
					left -= 6;
					step = 0;
					p += digit;
				}
			}
		}
		else if (status == 10)
		{//new seg, check if go to dir or file
			pSegStartPos = p;
			step = 0;
			const char *pTmp = strchr(p, '/');	
			if (pTmp == NULL)
				status = 3;
			else
			{
				pSegFinishPos = pTmp;
				status = 11;
			}
		}
		else if (status == 3)
		{//file
			pSegStartPos = p;
			step = 0;
			const char *pTmp = strchr(p, '.');
			if (pTmp == NULL)
				status = 4;
			else
			{
				pSegFinishPos = pTmp;
				status = 11;
			}
		}
		else if (status == 4)
		{
			status = 11;
			pSegStartPos = p;
			step = 0;
			const char *pTmp = strchr(p, '?');
			if (pTmp == NULL)
				pSegFinishPos = pEnd;
			else
				pSegFinishPos = pTmp;
		}
		else if (status == 11)
		{//dir
			if (*p == '=')
			{
				int len = pSegFinishPos - p;
				if (2 <= left)
				{
					memcpy(pSave, "=.+", 3);
					pSave += 3;
					left -= 3;
					step = 1;
					p += len;
					status = 12;
					continue;
				}
				else
					return -1;
			}
			int digit = 0;
			int alpha = 0;
			const char *pTmp = p;
			while (pTmp < pSegFinishPos)
			{
				if (isdigit(*pTmp))
					digit++;
				else if (isalpha(*pTmp) || *pTmp == '%')
					alpha++;
				else
					break;
				pTmp++;
			}
			if (digit > 0 && alpha == 0)
			{
				if (digit_common_len <= left)
				{
					memcpy(pSave, digit_common, digit_common_len);
					pSave += digit_common_len;
					left -= digit_common_len;
					step = 0;
					p += digit;
				}
				else
					return -1;
			}
			else if (digit > 0 && alpha > 0)
			{
				if (digit_common_len <= left)
				{
					memcpy(pSave, ".+", 2);
					pSave += 2;
					left -= 2;
					step = 0;
					p += (digit + alpha);
				}
				else
					return -1;
			}
			else if (digit = 0 && alpha > 0)
				step = alpha;
			if (*p == '/')
				status = 10;
			else if (*p == '.')
			{
				/*
				   if (left > 1)
				   {
				 *pSave = '\\';
				 pSave++;
				 left--;
				 }
				 */
				status = 4;
			}
			else if (*p == '?')
			{
				if (left > 1)
				{
					*pSave = '\\';
					pSave++;
					left--;
				}
				status = 12;
			}
			else if (*p == '&')
				status = 12;
			else if (p == pEnd)
				step = 0;
		}
		else if (status == 12)
		{
			pSegStartPos = p;
			step = 0;
			status = 11;
			const char *pTmp = strchr(p, '&');
			if (pTmp)
				pSegFinishPos = pTmp;
			else
				pSegFinishPos = pEnd;
		}
		if (step > 0)
		{
			savelen = step < left ? step : left;
			memcpy(pSave, p, savelen);
			pSave += savelen;
			*pSave = 0;
			p += step;
		}
	}
	if (pSave > buf && *(pSave - 1) == '/')
		*(pSave - 1) = 0;
	else
		*pSave = 0;
	return 0;
}



#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <iconv.h>

#include "chinese.h"
#include "string_util.h"
#include "html_text_utils.h"

static unsigned int ASC_MARK[256]=
{
    0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 0, 0, 0, 0, 0,	1, 1, 0, 0, 1, 0, 1, 0,//33 ! | 34 " | 40 ( | 41 ) | 44 , | 46 .
    0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 1, 1, 1, 0, 1, 1,//58 : | 59 ; | 60 < | 62 > | 63 ?
    0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 1, 0, 1, 0, 0,// 91 [ | 93 ]
    0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 1, 1, 1, 0, 0,//123 { | 124 | | 125 }
0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0
};


#define ENCODE_COUNT         252

static const char *ul_html_encode_name[ENCODE_COUNT]=
{
    "AElig","Aacute",
    "Acirc","Agrave",
    "Alpha","Aring",
    "Atilde","Auml",
    "Beta","Ccedil",
    "Chi","Dagger",
    "Delta","ETH",
    "Eacute","Ecirc",
    "Egrave","Epsilon",
    "Eta",
    "Euml",
    "Gamma",
    "Iacute",
    "Icirc",
    "Igrave",
    "Iota",
    "Iuml",
    "Kappa",
    "Lambda",
    "Mu",
    "Ntilde",
    "Nu",
    "OElig",
    "Oacute",
    "Ocirc",
    "Ograve",
    "Omega",
    "Omicron",
    "Oslash",
    "Otilde",
    "Ouml",
    "Phi",
    "Pi",
    "Prime",
    "Psi",
    "Rho",
    "Scaron",
    "Sigma",
    "THORN",
    "Tau",
    "Theta",
    "Uacute",
    "Ucirc",
    "Ugrave",
    "Upsilon",
    "Uuml",
    "Xi",
    "Yacute",
    "Yuml",
    "Zeta",
    "aacute",
    "acirc",
    "acute",
    "aelig",
    "agrave",
    "alefsym",
    "alpha",
    "amp",
    "and",
    "ang",
    "aring",
    "asymp",
    "atilde",
    "auml",
    "bdquo",
    "beta",
    "brvbar",
    "bull",
    "cap",
    "ccedil",
    "cedil",
    "cent",
    "chi",
    "circ",
    "clubs",
    "cong",
    "copy",
    "crarr",
    "cup",
    "curren",
    "dArr",
    "dagger",
    "darr",
    "deg",
    "delta",
    "diams",
    "divide",
    "eacute",
    "ecirc",
    "egrave",
    "empty",
    "emsp",
    "ensp",
    "epsilon",
    "equiv",
    "eta",
    "eth",
    "euml",
    "euro",
    "exist",
    "fnof",
    "forall",
    "frac12",
    "frac14",
    "frac34",
    "frasl",
    "gamma",
    "ge",
    "gt",
    "hArr",
    "harr",
    "hearts",
    "hellip",
    "iacute",
    "icirc",
    "iexcl",
    "igrave",
    "image",
    "infin",
    "int",
    "iota",
    "iquest",
    "isin",
    "iuml",
    "kappa",
    "lArr",
    "lambda",
    "lang",
    "laquo",
    "larr",
    "lceil",
    "ldquo",
    "le",
    "lfloor",
    "lowast",
    "loz",
    "lrm",
    "lsaquo",
    "lsquo",
    "lt",
    "macr",
    "mdash",
    "micro",
    "middot",
    "minus",
    "mu",
    "nabla",
    "nbsp",
    "ndash",
    "ne",
    "ni",
    "not",
    "notin",
    "nsub",
    "ntilde",
    "nu",
    "oacute",
    "ocirc",
    "oelig",
    "ograve",
    "oline",
    "omega",
    "omicron",
    "oplus",
    "or",
    "ordf",
    "ordm",
    "oslash",
    "otilde",
    "otimes",
    "ouml",
    "para",
    "part",
    "permil",
    "perp",
    "phi",
    "pi",
    "piv",
    "plusmn",
    "pound",
    "prime",
    "prod",
    "prop",
    "psi",
    "quot",
    "rArr",
    "radic",
    "rang",
    "raquo",
    "rarr",
    "rceil",
    "rdquo",
    "real",
    "reg",
    "rfloor",
    "rho",
    "rlm",
    "rsaquo",
    "rsquo",
    "sbquo",
    "scaron",
    "sdot",
    "sect",
    "shy",
    "sigma",
    "sigmaf",
    "sim",
    "spades",
    "sub",
    "sube",
    "sum",
    "sup",
    "sup1",
    "sup2",
    "sup3",
    "supe",
    "szlig",
    "tau",
    "there4",
    "theta",
    "thetasym",
    "thinsp",
    "thorn",
    "tilde",
    "times",
    "trade",
    "uArr",
    "uacute",
    "uarr",
    "ucirc",
    "ugrave",
    "uml",
    "upsih",
    "upsilon",
    "uuml",
    "weierp",
    "xi",
    "yacute",
    "yen",
    "yuml",
    "zeta",
    "zwj",
    "zwnj"
};

static unsigned short ul_html_encode_value[ENCODE_COUNT]={
    198,
    193,
    194,
    192,
    913,
    197,
    195,
    196,
    914,
    199,
    935,
    8225,
    916,
    208,
    201,
    202,
    200,
    917,
    919,
    203,
    915,
    205,
    206,
    204,
    921,
    207,
    922,
    923,
    924,
    209,
    925,
    338,
    211,
    212,
    210,
    937,
    927,
    216,
    213,
    214,
    934,
    928,
    8243,
    936,
    929,
    352,
    931,
    222,
    932,
    920,
    218,
    219,
    217,
    933,
    220,
    926,
    221,
    376,
    918,
    225,
    226,
    180,
    230,
    224,
    8501,
    945,
    38,
    8743,
    8736,
    229,
    8776,
    227,
    228,
    8222,
    946,
    166,
    8226,
    8745,
    231,
    184,
    162,
    967,
    710,
    9827,
    8773,
    169,
    8629,
    8746,
    164,
    8659,
    8224,
    8595,
    176,
    948,
    9830,
    247,
    233,
    234,
    232,
    8709,
    8195,
    8194,
    949,
    8801,
    951,
    240,
    235,
    8364,
    8707,
    402,
    8704,
    189,
    188,
    190,
    8260,
    947,
    8805,
    62,
    8660,
    8596,
    9829,
    8230,
    237,
    238,
    161,
    236,
    8465,
    8734,
    8747,
    953,
    191,
    8712,
    239,
    954,
    8656,
    955,
    9001,
    171,
    8592,
    8968,
    8220,
    8804,
    8970,
    8727,
    9674,
    8206,
    8249,
    8216,
    60,
    175,
    8212,
    181,
    183,
    8722,
    956,
    8711,
    32,
    8211,
    8800,
    8715,
    172,
    8713,
    8836,
    241,
    957,
    243,
    244,
    339,
    242,
    8254,
    969,
    959,
    8853,
    8744,
    170,
    186,
    248,
    245,
    8855,
    246,
    182,
    8706,
    8240,
    8869,
    966,
    960,
    982,
    177,
    163,
    8242,
    8719,
    8733,
    968,
    34,
    8658,
    8730,
    9002,
    187,
    8594,
    8969,
    8221,
    8476,
    174,
    8971,
    961,
    8207,
    8250,
    8217,
    8218,
    353,
    8901,
    167,
    173,
    963,
    962,
    8764,
    9824,
    8834,
    8838,
    8721,
    8835,
    185,
    178,
    179,
    8839,
    223,
    964,
    8756,
    952,
    977,
    8201,
    254,
    732,
    215,
    8482,
    8657,
    250,
    8593,
    251,
    249,
    168,
    978,
    965,
    252,
    8472,
    958,
    253,
    165,
    255,
    950,
    8205,
    8204
};


/*
 * find location of src in reference char table
 * if not found, return -1
 */
static int seek_reference_char_table(char *src, int len)
{

	int locate;
	int head,tail;
	int ret;
	int	encode_len;

	head = 0;
	tail = ENCODE_COUNT-1;

	while(head <= tail ) {
		locate = (head+tail)/2;
		encode_len = strlen (ul_html_encode_name[locate]);
		ret = strncmp(src, ul_html_encode_name[locate], encode_len);
		if(ret == 0) {
			if (encode_len == len)
				return locate;
			else
				return -1;
		} else if( ret > 0) {
			head = locate+1;
		} else {
			tail = locate-1;
		}
	}
	return -1;
}

/*
 * if src is a valid reference string, return unicode value of it
 * if is invalid reference string, return ULONG_MAX
 */
unsigned long dereference_reference_char(char *src, int len)
{
	int locate;
    unsigned short unicode;

	locate = seek_reference_char_table(src, len);
	if (locate == -1) {
		return ULONG_MAX;
	}
	assert(locate >= 0 && locate <= ENCODE_COUNT-1);
	unicode = ul_html_encode_value[locate];
	return unicode;
}

char *html_dereference_ex(char *src, unsigned int *unicode)
{
	char *new_src;
	unsigned long long_value;

	if (*src != '&') {
		return NULL;
	}

	new_src = NULL;
	long_value = ULONG_MAX;

	if (*(src+1) == '#') {
		if (*(src+2) == 'x' || *(src+2) == 'X') {
			long_value = strtoul(src+3, &new_src, 16);
		} else {
			long_value = strtoul(src+2, &new_src, 10);
		}
	} else {
		for (new_src=src+1; isalpha(*new_src); new_src++) {
			;
		}
		long_value = dereference_reference_char(src+1, new_src-src-1);
	}
	if(long_value >= 0x80 && long_value <= 0x9f){
		long_value = 32;
	}
	assert(new_src != NULL);
	if (*new_src == ';') {
		new_src++;
	}
	*unicode = long_value;
	return new_src;
}


static inline bool is_gb18030_4bytes(const char* s){
    return (s != NULL)
        && IN_RANGE(s[0], 0x81, 0xFE)
        && IN_RANGE(s[1], 0x30, 0x39)
        && IN_RANGE(s[2], 0x81, 0xFE)
        && IN_RANGE(s[3], 0x30, 0x39);
}

static inline bool is_gb18030_4bytes_hanzi(const char* s){
    return (s != NULL)
        && (IN_RANGE(s[0], 0x81, 0x82) || IN_RANGE(s[0], 0x95, 0x98)) //2 sections
        && IN_RANGE(s[1], 0x30, 0x39)
        && IN_RANGE(s[2], 0x81, 0xFE)
        && IN_RANGE(s[3], 0x30, 0x39);
}

static int next_gb18030(const char* s){
    if(s == NULL || *s == '\0'){
        return 0;
    }
    if(((*s) & 0x7F) == *s){//ascii
        return 1;
    }else if(IS_GBK((u_char*) s)){//gbk
        return 2;
    }else if(is_gb18030_4bytes(s)){//gb18030 4bytes
        return 4;
    }
    return -1;//invalid
}

/*
   U00000000 - U0000007F: 0xxxxxxx
   U00000080 - U000007FF: 110xxxxx 10xxxxxx
   U00000800 - U0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx
   U00010000 - U001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
   U00200000 - U03FFFFFF: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
   U04000000 - U7FFFFFFF: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
   */
static inline int unicode_to_utf8(unsigned int value, char* buffer){
    if(value <= 0x0000007F){
        buffer[0] = (char)value;
        buffer[1] = '\0';
    }else if(value <= 0x000007FF){
        buffer[0] = (char)(0xC0 | (value >> 6));
        buffer[1] = (char)(0x80 | (value & 0x3F));
        buffer[2] = '\0';
    }else if(value <= 0x0000FFFF){
        buffer[0] = (char)(0xE0 | (value >> 12));
        buffer[1] = (char)(0x80 | ((value >> 6) & 0x3F));
        buffer[2] = (char)(0x80 | (value & 0x3F));
        buffer[3] = '\0';
    }else if(value <= 0x001FFFFF){
        buffer[0] = (char)(0xF0 | (value >> 18));
        buffer[1] = (char)(0x80 | ((value >> 12) & 0x3F));
        buffer[2] = (char)(0x80 | ((value >> 6) & 0x3F));
        buffer[3] = (char)(0x80 | (value & 0x3F));
        buffer[4] = '\0';
    }else if(value <= 0x03FFFFFF){
        buffer[0] = (char)(0xF8 | (value >> 24));
        buffer[1] = (char)(0x80 | ((value >> 18) & 0x3F));
        buffer[2] = (char)(0x80 | ((value >> 12) & 0x3F));
        buffer[3] = (char)(0x80 | ((value >> 6) & 0x3F));
        buffer[4] = (char)(0x80 | (value & 0x3F));
        buffer[5] = '\0';
    }else if(value <= 0x7FFFFFFF){
        buffer[0] = (char)(0xFC | (value >> 30));
        buffer[1] = (char)(0x80 | ((value >> 24) & 0x3F));
        buffer[2] = (char)(0x80 | ((value >> 18) & 0x3F));
        buffer[3] = (char)(0x80 | ((value >> 12) & 0x3F));
        buffer[4] = (char)(0x80 | ((value >> 6) & 0x3F));
        buffer[5] = (char)(0x80 | (value & 0x3F));
        buffer[6] = '\0';
    }else{
        return -1;
    }
    return 1;
}

static inline int utf8_to_gb18030(char* in, size_t* inlen, char* out, size_t* outlen){
	iconv_t cd = iconv_open("gb18030", "utf-8");
	if(cd <= 0){
		return -1;
	}
	size_t tmp = *outlen;
	int ret = iconv(cd, &in, inlen, &out, outlen);
    if (cd > 0){
        iconv_close(cd);
    }
    *outlen = tmp - *outlen;
    return ret;
}

/**
 * @brief 将一个gb18030编码的含有转义字符的字符串中的转义字符转成gb18030编码字符
 * <p>请保证输入字符串是gb18030编码，否则可能出现乱码</p>
 *
 * @param psrc 输入字符串
 * @param pdes 输出buffer
 * @param slen 输入长度
 * @param dlen 输出buffer大小
 *
 * @return 成功返回1，出错返回-1
 */
int html_deref_to_gb18030_str(char* psrc, char* pdes, int slen, int dlen){
    //check param
    if(psrc == NULL || pdes == NULL || dlen <= 0){
        return -1;
    }

    char in[10];
    char out[10];
    size_t inlen = 0;
    size_t outlen = 10;
    in[0] = '\0';
    out[0] = '\0';

    char* next = NULL;
    unsigned int unicode = 0;
    int i = 0;
    int j = 0;
    int len = slen;
    if(len <= 0){
        len = strlen(psrc);
    }

    while(i < len){
        if((next = html_dereference_ex(psrc + i, &unicode)) != NULL && unicode_to_utf8(unicode, in) != -1){//do dereference{
        	inlen = strlen(in);
        	if(unicode < 128){
    			if(j+(int)inlen >= dlen){
    				 break;
    			}
    			strncpy(pdes + j, in, inlen);
    			i = next - psrc;
    			j = j + inlen;
            	in[0] = '\0';
            	inlen = 0;
        	}
        	else {
				if(utf8_to_gb18030(in, &inlen, out, &outlen) == -1){
					i = next - psrc;
					continue;
				}
				if(j+(int)outlen >= dlen){
					 break;
				}
				strncpy(pdes + j, out, outlen);
				i = next - psrc;
				j = j + outlen;
				in[0] = '\0';
				inlen = 0;
				out[0] = '\0';
				outlen = 10;
        	}
        }else{//dereference failed
            int nlen = next_gb18030(psrc + i);
            if(nlen < 0){//illegal
                nlen = 1;
            }else if(nlen == 0){//done
                break;
            }
            if(j + nlen + 1 >= dlen){
                break;
            }
            strncpy(pdes + j, psrc + i, nlen);
            i += nlen;
            j += nlen;
        	in[0] = '\0';
        	inlen = 0;
        }
    }
    pdes[j] = '\0';
    return 1;
}

static bool last_is_gb18030(const char* s, const char* beg){
    if((s - beg >= 2 && IS_GBK(s-2)) || (s - beg >= 4 && is_gb18030_4bytes(s-4))){
        return true;
    }
    return false;
}

static bool next_is_gb18030(const char* s, int curlen){
    if(IS_GBK(s+curlen) || is_gb18030_4bytes(s+curlen)){
        return true;
    }
    return false;
}

//片假名
static int is_pajm(char * word) {
    u_char * p = (u_char*)word;
    if((p[0] == 0xa5 && p[1] >= 0xa1 && p[1] <= 0xf6)
            ||(p[0] == 0xa9 && p[1] == 0x60))
        return 1;
    return 0;
}

//平假名
static int is_pijm(char * word) {
    u_char * p = (u_char*)word;
    if((p[0] == 0xa4 && p[1] >= 0xa1 && p[1] <= 0xf3))
        return 1;
    return 0;
}

static int is_gb_punct(char* s) {
    if (!IS_GB_MARK(s))
        return 0;
    if (is_pajm(s) || is_pijm(s))
        return 0;
    return 1;
}

/*
 * description :
 * 		copy text from src to buffer after merging blank
 * input :
 * 		buffer : the buffer to store text
 * 		available : the begin postion of buffer this time
 * 		end : the last position of buffer
 * 		src : the source text
 * output :
 * 		buffer : the text will append to it
 * return
 * 		the new postion of buffer
 */
int html_tree_copy_html_text_gb18030(char *buffer, int available, int end, char *src) {

    int sflag = 0;
    int start = available;
    char *begin = src;
    char *mark = src;  /* Mark of first punct or non-space ascii */
    int gbkcount = 0;  /* Number of Hanzi before mark */

    assert(buffer != NULL && src != NULL);

    /* Count the number of Hanzi */
    while (gbkcount < 3) {
        if (*mark == '\0')
            break;

        /* Space */
        if (isspace(*mark)) {
            mark++;
            continue;
        }
        if (IS_GB_SPACE(mark) || IS_GB_UNDEFINED(mark)) {
            mark += 2;
            continue;
        }


        /* Hanzi */
        if (IS_GBK(mark) && !is_gb_punct(mark)) {
            mark += 2;
            gbkcount++;
            continue;
        }else if(is_gb18030_4bytes_hanzi(mark)){
            mark += 4;
            gbkcount ++;
            continue;
        }

        /* Stop otherwise */
        break;
    }

    /* Neglect any space before first punct or non-space ascii */
    if (gbkcount > 0 && gbkcount < 3) {
        while (src < mark) {
            if (available == end) {
                break;
            }

            /* Space */
            if (isspace(*src)) {
                if (src == begin && available-1 >= 0 && buffer[available-1] != ' ')
                    buffer[available++] = ' ';
                src++;
                continue;
            }
            if (IS_GB_SPACE(src) || IS_GB_UNDEFINED(src)) {
                if (src == begin && available-1 >= 0 && buffer[available-1] != ' ')
                    buffer[available++] = ' ';
                src += 2;
                continue;
            }

            /* Otherwise copy */
            if (IS_GBK(src)) {
                if (available + 1 == end) {
                    break;
                }
                buffer[available++] = *src++;
                buffer[available++] = *src++;
            }else if(is_gb18030_4bytes(src)){
                if(available + 3 >= end){
                    break;
                }
                buffer[available++] = *src++;
                buffer[available++] = *src++;
                buffer[available++] = *src++;
                buffer[available++] = *src++;
            }else
                buffer[available++] = *src++;
        }
    }

    while (*src != '\0') {
        if (available == end) {
            break;
        }

        /* Windows linebreak */
        if (sflag == 0 && *src == '\r' && *(src + 1) == '\n') {
            if (last_is_gb18030(src, begin) && next_is_gb18030(src, 2)) {
                src += 2;
                continue;
            }
        }

        /* Unix linebreak */
        if (sflag == 0 && *src == '\n') {
            if (last_is_gb18030(src, begin) && next_is_gb18030(src, 1)) {
                src++;
                continue;
            }
        }

        //space
        if (isspace(*src)) {
            if (available-1 >= 0 && buffer[available-1] != ' ') {
                buffer[available++] = ' ';
            }else{
                sflag = 2;
            }

            src++;
            continue;
        }

        //gbk space
        if ((IS_GB_SPACE ((src))) || (IS_GB_UNDEFINED ((src)))) {
            if (available-1 >= 0 && buffer[available-1] != ' ') {
                buffer[available++] = ' ';
            }else{
                sflag = 2;
            }
            src++;
            src++;
            continue;
        }

        //gbk & gb18030
        if ((IS_GBK((src))) || is_gb18030_4bytes(src)) {
            int len = IS_GBK(src) ? 2 : 4;
            //1.前面紧跟一个非合并过的空格, 并且空格前必须是一个GBK字符
            if (available-start-1 >=0 && buffer[available-1]==' ' && sflag!=2
                    && (available-start-3<0 || last_is_gb18030(buffer+available-1, buffer+start)))
            { //2. 空格前一个GBK的前面是空格或标点 或已经被合并过空格的GBK
                int offset = 0;
                if(available - start - 3 >= 0 && IS_GBK(buffer + available - 3)){
                    offset = 2;
                }else{
                    offset = 4;
                }
                if (available-start-offset-2 < 0 || buffer[available-offset-2] == ' '|| sflag == 1||
                        ((available-start-offset-3>=0) && is_gb_punct(buffer+available-offset-3))
                        || (!last_is_gb18030(buffer+available-offset-1, buffer+start)
                            && ASC_MARK[(unsigned char)(buffer[available-offset-2])]))
                {
                    //3. 后面是一个无法识别字符或空格或标点或ASCII字符
                    if ((IS_GB_SPACE((src+len))) || (IS_GB_UNDEFINED((src+len))) ||
                            (is_gb_punct(src+len)) || (!IS_GBK(src+len)))
                    {
                        sflag = 1;
                        available--;
                    }else{
                        sflag = 0;
                    }
                }else{
                    sflag = 0;
                }
            }else{
                sflag = 0;
            }

            if (available + len > end) {//buffer not enough
                break;
            }
            if(len == 2){//GBK
                buffer[available++] = *src++;
                buffer[available++] = *src++;
                continue;
            }else{//GB18030
                buffer[available++] = *src++;
                buffer[available++] = *src++;
                buffer[available++] = *src++;
                buffer[available++] = *src++;
                continue;
            }
        }

        //ascii
        buffer[available++] = *src++;
    }
    buffer[available] = '\0';
    return available;
}

/**
 * @brief 快速的copy_html_text. 若当前文本不含&，则忽略转义.
**/
int copy_html_text(char *buffer, int available, int end, char *src)
{
	if(!src){
		return available;
	}
	if(strchr(src, '&') != NULL){
	    html_deref_to_gb18030_str (src, buffer + available, 0, end - available + 1);
	    available = html_tree_copy_html_text_gb18030 (buffer, available, end, buffer + available);
	    return available;
	}
	else{
		available = html_tree_copy_html_text_gb18030 (buffer, available, end, src);
		return available;
	}
//	html_deref_to_gb18030_str (src, buffer+available, end - available + 1, sizeof(buffer), 0);
//
//    available = html_tree_copy_html_text_gb18030 (buffer, available, end, buffer + available);
//
//    return available;
}

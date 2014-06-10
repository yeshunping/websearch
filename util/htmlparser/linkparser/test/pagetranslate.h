#ifndef _PAGE_TRANSLATE_H_
#define _PAGE_TRANSLATE_H_
#include <string>
#include <fstream>
#include "easou_html_tree.h"
#include "chardet.h"
#include "iconv.h"
#include "errno.h"
using namespace std;
class PageTranslator
{
private:
	chardet_t det;
public:
	PageTranslator()
	{
	}
	int fillBufferWithGb18030(char* fromCharset,const char* page,int pagelen,char* buffer,int bufferlen)
	{
		return charsetConvert(fromCharset,"GB18030",page,pagelen,buffer,bufferlen);
	}
	int guessCharset(const char* buf,int bufLen,char* charset)
	{
		int ret = 0;
		if (chardet_create(&det)) 
		{
			ret = -1;
			goto END;
		}
		if (chardet_handle_data(det, buf,(unsigned int) bufLen) != CHARDET_RESULT_OK) 
		{
			ret = -2;
			goto END;
		}
		if (chardet_data_end(det) != CHARDET_RESULT_OK) 
		{
			ret = -3;
			goto END;
		}
		if (chardet_get_charset(det, charset,CHARDET_MAX_ENCODING_NAME) != CHARDET_RESULT_OK)
		{
			ret = -4;
			goto END;
		}
		END: if(det)
			chardet_destroy(det);
	 	return ret;
	}
	int detectCharset(const char* page,int size,char* charset)
	{
		return guessCharset(page,size,charset);
	}
	
	int translate(const char* page,int size,char* buffer,int bufferlen)
	{
		char charset[50] = {0};
		bool gotCharset = false;
		if(detectCharset(page,size,charset) == 0 && strlen(charset) > 0)
		{
			gotCharset = true;
		}
		int len = 0;
		if(gotCharset)
		{
			len = fillBufferWithGb18030(charset,page,size,buffer,bufferlen);
			if(len< 0)
				gotCharset = false;
			else
				buffer[len] = 0;
		}
		return len < 0 || !gotCharset ? 0 : len;
	}
	int charsetConvert(const char *from_charset,const char *to_charset, 
			const char *src, const int srclen, char* save,int savelen) 
	{  
    	if(save==NULL||srclen == 0)
    	{	  
        	return -1;  
    	}    
    	save[0] = 0;  
    	if (strcasecmp(from_charset, to_charset) == 0) 
		{  
        	if(savelen<=srclen)  
            	strncpy(save, src, savelen);  
        	else  
            	strncpy(save, src, srclen);  
        	return savelen>srclen ? srclen : savelen;  
    	}    
    	iconv_t cd;  
    	int status = 0; //result  
    	char *outbuf = save;//iconv outptr begin  
    	const char* inptr = src;  
    	char* outptr = outbuf;  
    	size_t insize = srclen;  
    	size_t outsize = savelen;   
    	cd = iconv_open(to_charset, from_charset);  
    	if((iconv_t)(-1) == cd)
    	{  
        	return -1;  
    	}  
    	iconv(cd, NULL, NULL, NULL, NULL);  
    	while (insize > 0) 
		{  
        	size_t res = iconv(cd, (char**)&inptr, &insize, &outptr,&outsize);  
        	if (outptr != outbuf) 
			{  
            	outbuf=outptr;  
            	*outbuf=0;  
        	}  
        	if (res == (size_t) (-1)) 
			{  
            	if (errno == EILSEQ) 
				{	  
                	int one = 1;  
                	iconvctl(cd, ICONV_SET_DISCARD_ILSEQ, &one);  
                	status = -3;  
            	} 
				else if (errno == EINVAL) 
				{  
                	if (srclen == 0) 
					{  
                    	status = -4;  
                   		goto done;  
                	} 
					else 
					{  
                    	break;  
                	}  
            	} 
				else if (errno == E2BIG) 
				{  
                	status = -5;  
                	goto done;  
            	} 
				else 
				{  
                	status = -6;  
                	goto done;  
            	}  
        	}  
    	}	  
    	status = outbuf - save;  
	done:  
    	iconv_close(cd);  
    	return status;
  	}
};
#endif

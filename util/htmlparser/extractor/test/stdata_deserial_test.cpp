//翻库结束后测试程序 反序列化读取字段
#include <stdio.h>
#include <stdlib.h>     
#include <sys/types.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>

#include "easou_html_tree.h"
#include "easou_vhtml_tree.h"
#include "easou_html_extractor.h"
#include "easou_html_attr.h"

#include "pagetranslate.h"
#include "easou_debug.h"
#include "easou_string.h"
#include "log.h"

//includes for edb 
#include "RandomRead.h"
#include "ScannerClient.h"
#include "Scanner.h"
#include "readlocal.h"
#include "ClientConfig.h"
#include "PageDBValues.h"
#include "GlobalFun.h"
#include "dump.h"
#include "Queue.h"
#include "Log.h"

#include "CCharset.h"
                
        
#include "easou_extractor_stdata.h"
#include "StructData_types.h"
        
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using boost::shared_ptr;


using namespace std;
using namespace nsPageDB;

#define MAX_PAGE_LEN 1<<20

//CLog g_Log;
CClientConfig gConfig;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int process_remote_bucket(int bucketno)
{
	int pid = getpid();
	vector<UINT8> familyno;
	familyno.push_back(3); //content

	CScanner reador;
	int ret = reador.open(bucketno, familyno, &gConfig, NULL, 0);
	if (CPageDBValues::DB_OPR_SCAN_COMPLETE == ret)
	{
		printf("complete!\n");
		return 0;
	}
	if (ret)
	{
		printf("errorno:%d\n", ret);
		return -1;
	}

	CTblRowReador *rowreador;
	const char* key;
        UINT16 keylen;
	char url[UL_MAX_URL_LEN];
        int urlLen;

	while (true)
	{
		ret = reador.next(rowreador);
		if (CPageDBValues::DB_OPR_SCAN_COMPLETE == ret)
			break;
		if (ret)
		{
			printf("errorno:%d\n", ret);
			return -1;
		}

		rowreador->getKey(key, keylen);
		urlLen = keylen < UL_MAX_URL_LEN ? keylen : (UL_MAX_URL_LEN - 1);
		memcpy(url, key, urlLen);
		*(url + urlLen) = 0;
		//if (strncmp(url, "http://baike.baidu.com/view/", 28) != 0 && strncmp(url, "http://www.dianping.com/shop/", 29) != 0)
		if (strncmp(url, "http://baike.baidu.com/view/", 28) != 0)
			continue;

		char* v;
		int vlen;

		pthread_mutex_lock(&mutex);
		rowreador->get(3, 5, v, vlen);

		printf("url=%s\tvlen=%d\n", url, vlen);
		StructureData s2;
		int des = html_tree_extract_deserial(&s2, (uint8_t*)v, vlen);
		if(des != 0)
		{
			printf("deserial fail\n");
			pthread_mutex_unlock(&mutex);
			continue;
		}

		if(s2.all.size() < 5)
		{
			pthread_mutex_unlock(&mutex);
			continue;
		}
		printf("version=%d\ttype=%d\tlen=%d\n",  s2.version, s2.type, s2.all.size());
		for(vector<StructureKV>::iterator j=s2.all.begin(); j!=s2.all.end(); j++)
			printf("\tkey=%d\tvalue=%s\n", j->key, j->value.c_str());

		pthread_mutex_unlock(&mutex);

	}
	reador.close();
	return 1;
}

void* read_thread(void *p)
{
	for (int i = 50; i < 200; i++)
		process_remote_bucket(i);
	return NULL;
}


/*int randomread(char *key, UINT16 keylen, char *buf, int len)
{
	CRandomRead reador;
	reador.init(&gConfig);
	CTblRowReador *rowreador = NULL;
	if (reador.get(key, keylen, rowreador))
	{
		printf("read error! %s\n", string(key, keylen).c_str());
		return -1;
	}
	else
		printf("read success! %s\n", string(key, keylen).c_str());

	char* v;
	int vlen;
	rowreador->get(0, 1, v, vlen);

	string page(v, vlen);
	len = len <= page.length() ? len - 1 : page.length();
	memcpy(buf, page.c_str(), len);
	*(buf + len) = 0;
	return len;
}*/


int main(int argc, char** argv)
{

	if (!Init_Log("log.conf"))
	{
		printf("log.conf not exist\n");
		exit(-1);
	}
	if (!init_css_server("./config.ini", "./log"))
	//if (!init_css_server("./config.ini", "./log", 1000, 10))
	{
		printf("init css server fail\n");
		exit(-1);
	}
	if (gConfig.init("./config.ini", "pagedb") != 0)
	{
		printf("config init error\n");
		exit(-1);
	}

	//read_thread();

	/*char *key = "http://baike.baidu.com/view/3133766.htm";
	int keylen = strlen(key);
	
	char *page = (char *)malloc(2048000);
	int pagelen = randomread(key, keylen, page, 2048000);

	FILE *fp = fopen("bk.html", "w");
	assert(fp);
	fwrite(page, pagelen, 1, fp);
	fclose(fp);

	free(page);*/
	/*pthread_t r_id;
	int error = pthread_create(&r_id, NULL, read_thread, NULL);
	if (error != 0)
	{
		printf("create read thread fail\n");
		exit(-1);
	}*/

	pthread_t* p_ids = new pthread_t[3];
	for (int i = 0; i < 3; i++)
	{
		int error = pthread_create(&p_ids[i], NULL, read_thread, NULL);
		if (error != 0)
		{
			printf("create read thread fail\n");
			exit(-1);
		}
	}

	//waitting..
	//pthread_join(r_id, NULL);
	for (int i = 0; i < 3; i++)
		pthread_join(p_ids[i], NULL);

	_FAIL: gConfig.destroy();
	if (p_ids)
		delete[] p_ids;

	return 0;
}



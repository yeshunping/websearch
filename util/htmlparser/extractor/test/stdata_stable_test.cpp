//稳定性测试 onpagedb

#include <stdio.h>
#include <stdlib.h>     
#include <sys/types.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>

#include "easou_html_tree.h"
#include "easou_vhtml_tree.h"
#include "easou_ahtml_tree.h"
#include "easou_mark_parser.h"
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
	familyno.push_back(0); //page
	familyno.push_back(3); //content
	familyno.push_back(4); //feature

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

	StructureData *stdata = new StructureData;
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
		//if (strncmp(url, "http://www", 10) != 0 && strncmp(url, "http://novel", 12) != 0)
		//if (strncmp(url, "http://www.xxsy.net/info/", 25) != 0 && strncmp(url, "http://www.yuncheng.com/book/detail/", 36) != 0 && strncmp(url, "http://www.xs8.cn/book/",23 ) != 0)
		if (strncmp(url, "http://soft.d.cn/", 17) != 0)
			continue;
		char* v;
		int vlen;
		//
		//dead link skip
		rowreador->get(4, 2, v, vlen);
		int dead_link = *(UINT8*) v;
		if (dead_link == 1)
			continue;


		rowreador->get(0, 1, v, vlen);
		string page;
		page = string(v, vlen);

		char *pagebuf = const_cast<char *>(page.c_str());
                int pageLen = page.length();

                char pageGBBuf[MAX_PAGE_LEN];
                PageTranslator pt;
                int pageGBLen = pt.translate(pagebuf, pageLen, pageGBBuf, MAX_PAGE_LEN);
                pageGBBuf[pageGBLen] = 0;
                if(pageGBLen == 0)
                        continue;

                pthread_mutex_lock(&mutex);
		//init
                html_tree_t *mytree = html_tree_create(128000); //初始化dom树
		vtree_in_t *vtree_in = vtree_in_create();
		html_vtree_t *vtree = html_vtree_create_with_tree(mytree);
		area_tree_t *atree = area_tree_create(NULL);
		assert(mytree);	
		assert(vtree_in);
		assert(vtree);
		assert(atree);

		//parser
                html_tree_parse(mytree, pageGBBuf, pageGBLen); //解析dom树 1 success,0 fail
		html_vtree_parse_with_tree(vtree, vtree_in, url);
		area_partition(atree, vtree, url);
		mark_area_tree(atree, url);

		printf("\n\n\n%s\n", url);
		//ret = html_tree_extract_stdata(url, urlLen, 0, mytree, stdata);
		ret = html_tree_extract_download_stdata(url, urlLen, (unsigned long long)0, mytree, vtree, atree, stdata);
                if (ret != 0)
                {
                        printf("extract structure data fail\n");
                        exit(-1);
                }
                //printf("stdata.type=%d\n", stdata->type);
                //for(vector<StructureKV>::iterator j=stdata->all.begin(); j!=stdata->all.end(); j++)
		//	printf("key=%d\tvalue=%s\n", j->key, j->value.c_str());
                uint8_t *buf_ptr = (uint8_t*) malloc (32768);
                uint32_t sz = html_tree_extract_serial(stdata, buf_ptr, 32768);
                if(sz == -1)
                {
                        printf("serial fail");
			free(buf_ptr);
			html_tree_del(mytree);
                        pthread_mutex_unlock(&mutex);
                        continue;
                }       

                StructureData s2;
                int des = html_tree_extract_deserial(&s2, buf_ptr, sz);
                if(des != 0)
                {
                        printf("deserial fail");
			free(buf_ptr);
			html_tree_del(mytree);
                        pthread_mutex_unlock(&mutex);
                        continue;
                }

                printf("version=%d\ttype=%d\tlen=%d\n",  s2.version, s2.type, s2.all.size());
                for(vector<StructureKV>::iterator j=s2.all.begin(); j!=s2.all.end(); j++)
                        printf("key=%d\tvalue=%s\n", j->key, j->value.c_str());

		html_vtree_del(vtree);
		vtree_in_destroy(vtree_in);
		html_tree_del(mytree);
		//free(buf_ptr);
                pthread_mutex_unlock(&mutex);

	}
	delete stdata;
	reador.close();
	return 1;
}

void* read_thread(void* p)
{
	//for (int i = 15752; i < 15755; i++)
	for (int i = 0; i < 10; i++)
		process_remote_bucket(i);
	return NULL;
}

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

	pthread_t r_id;
	int error = pthread_create(&r_id, NULL, read_thread, NULL);
	if (error != 0)
	{
		printf("create read thread fail\n");
		exit(-1);
	}

	/*pthread_t* p_ids = new pthread_t[3];
	for (int i = 0; i < 3; i++)
	{
		int error = pthread_create(&p_ids[i], NULL, read_thread, NULL);
		if (error != 0)
		{
			printf("create read thread fail\n");
			exit(-1);
		}
	}*/

	//waitting..
	pthread_join(r_id, NULL);
	//for (int i = 0; i < 3; i++)
	//	pthread_join(p_ids[i], NULL);

	_FAIL: gConfig.destroy();
	//if (p_ids)
	//	delete[] p_ids;
	return 0;
}



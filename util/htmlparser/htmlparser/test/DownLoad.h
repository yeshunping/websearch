// DownLoad.h
#ifndef __DOWN_LOAD_H_
#define __DOWN_LOAD_H_

#include "GlobalInfo.h"

class DownLoad
{
public:
	DownLoad();
	~DownLoad();
	
	int download(char* url, int url_len, char* buf, int buf_len, char*& result);
	
	int set_agent(char* ip, int port, char* usr_name, char* pwd);

private:
	int down(char* url, int url_len, char* buf, int buf_len, char*& result);
	// 从字符串src中分析出网站地址和端口，并得到用户要下载的文件
	int GetHost(char* url, int url_len, char* addr, char* file, int* port);
	// 合成发送数据头
	int GetRequestHead(char* host_addr, char* host_file, int port, char* head_buf);
	// 解析包头
	int ParseHead(char* buf, int buf_len, char*& page, int& page_len, char* jump_url, int& jump_len);
	// 发送数据
	int SendData(int sockfd, char* buf, int send_len);
	// 接收数据
	int RecvData(int sockfd, char* buf, int recv_len);
	// 分析跳转链接
	int ParseJump(char* buf, int buf_len, char* jump_url);
	// 将相对路径转换为绝对路径
	int to_real_addr(char* base, char* tmp_url, char* url, char* tmp_buf);

	int to_base64(char* instr, char* base64string);
	
	int m_use_agent;
	int m_agent_port;
	char m_agent_ip[16];
	char m_agent_usr[32];
	char m_agent_pwd[32];
	char m_proxy[64];
};

#endif

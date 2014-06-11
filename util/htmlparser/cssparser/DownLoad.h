// DownLoad.h
#ifndef __DOWN_LOAD_H_
#define __DOWN_LOAD_H_

#include "GlobalInfo.h"

class DownLoad {
 public:
  DownLoad();
  ~DownLoad();

  int download(char* url, int url_len, char* buf, int buf_len, char*& result);

  int set_agent(char* ip, int port, char* usr_name, char* pwd);

 private:
  int down(char* url, int url_len, char* buf, int buf_len, char*& result);
  int GetHost(char* url, int url_len, char* addr, char* file, int* port);
  int GetRequestHead(char* host_addr, char* host_file, int port,
                     char* head_buf);
  int ParseHead(char* buf, int buf_len, char*& page, int& page_len,
                char* jump_url, int& jump_len);
  int SendData(int sockfd, char* buf, int send_len);
  int RecvData(int sockfd, char* buf, int recv_len);
  int ParseJump(char* buf, int buf_len, char* jump_url);
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

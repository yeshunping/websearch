// DownLoad.cpp
#include "DownLoad.h"

DownLoad::DownLoad() {
#ifdef WIN32
  WSADATA data;
  WSAStartup(MAKEWORD(2,2), &data);
#endif
  m_use_agent = 0;
}

DownLoad::~DownLoad() {
#ifdef WIN32
  WSACleanup();
#endif
}

int DownLoad::download(char* url, int url_len, char* buf, int buf_len,
                       char*& result) {
  if (url_len > 1020)
    return -1;

  char jump_url[1024];
  char tmp_url[1024], new_url[1024];

  int jump_len = url_len, tmp_len;
  memcpy(jump_url, url, url_len + 1);

  for (int i = 0; i < 3; i++) {
    int page_len = down(jump_url, jump_len, buf, buf_len, result);
    int station = ParseHead(buf, page_len, result, page_len, tmp_url, tmp_len);
    if (station == 301 || station == 302) {
      jump_len = to_real_addr(jump_url, tmp_url, new_url, buf);
      strcpy(jump_url, new_url);
      continue;
    }
    if (station != 200)
      return -1;
    jump_len = ParseJump(result, page_len, jump_url);
    if (jump_len <= 0)
      return page_len;
  }
  return -1;
}

int DownLoad::set_agent(char* ip, int port, char* usr_name, char* pwd) {
  m_use_agent = 1;
  m_agent_port = port;

  strcpy(m_agent_ip, ip);
  strcpy(m_agent_usr, usr_name);
  strcpy(m_agent_pwd, pwd);

  char buf[300];
  sprintf(buf, "%s:%s", usr_name, pwd);
  to_base64(buf, m_proxy);

  return 0;
}

int DownLoad::down(char* url, int url_len, char* buf, int buf_len,
                   char*& result) {
  char host_addr[1024];
  char host_file[1024];
  int port;

  if (GetHost(url, url_len, host_addr, host_file, &port))
    return -1;
  int req_len = GetRequestHead(host_addr, host_file, port, buf);
  if (req_len <= 0)
    return -1;

  struct sockaddr_in sockaddr;
  memset(&sockaddr, 0, sizeof(sockaddr));

  sockaddr.sin_family = AF_INET;
  if (m_use_agent) {
    sockaddr.sin_port = htons(m_agent_port);
    sockaddr.sin_addr.s_addr = inet_addr(m_agent_ip);
  } else {
    struct hostent* phost = gethostbyname(host_addr);
    if (phost == NULL) {
      printf("gethostbyname error\n");
      return -1;
    }
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr = *((struct in_addr*) phost->h_addr);
  }

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
    return -1;

  struct timeval time_out;
  time_out.tv_sec = 30;
  time_out.tv_usec = 0;

  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*) &time_out,
             sizeof(time_out));
  setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*) &time_out,
             sizeof(time_out));

  if (connect(sockfd, (struct sockaddr*) (&sockaddr), sizeof(struct sockaddr))
      == -1) {
    printf("Connect Error:%s\n", strerror(errno));
    closesocket(sockfd);
    return -1;
  }

  if (SendData(sockfd, buf, req_len)) {
    printf("Send Req Error:%s\n", strerror(errno));
    closesocket(sockfd);
    return -1;
  }

  int ret_len = RecvData(sockfd, buf, buf_len);
  closesocket(sockfd);
  if (ret_len <= 0)
    return -1;
  return ret_len;
}

int DownLoad::ParseJump(char* buf, int buf_len, char* jump_url) {
  char* begin = buf, *end = buf + buf_len;

  while (begin < end) {
    while (begin < end && *begin != '<')
      begin++;
    if (begin >= end)
      break;
    begin++;
    if (strnicmp(begin, "meta ", 5) == 0) {
      begin += 5;
      char* req = NULL, *pt = NULL, *url = NULL;

      while (begin < end && *(short*) begin != *(short*) "/>") {
        if (strnicmp(begin, "http-equiv=", 11) == 0) {
          begin += 11;
          req = begin;
        } else if (strnicmp(begin, "content=", 8) == 0) {
          begin += 8;
          pt = begin;
        } else if (strnicmp(begin, "url=", 4) == 0) {
          begin += 4;
          url = begin;
        } else {
          begin++;
        }
      }
      if (req && pt && url) {
        if (*req == '\"')
          req++;
        if (*pt == '\"')
          pt++;
        if (*url == '\"')
          url++;
        if (strnicmp(req, "refresh", 7) == 0 && atol(pt) <= 3) {
          char* p1 = jump_url;
          while (url < begin && *url != '\"' && *url != ' ')
            *p1++ = *url++;
          *p1 = 0;
          return p1 - jump_url;
        }
      }
    } else {
      begin++;
    }
  }

  return -1;
}

int DownLoad::GetHost(char* url, int url_len, char* addr, char* file,
                      int* port) {
  char* p1 = url, *p2 = url + url_len, *p3 = addr, *p4 = file;

  while (p1 < p2 && *(short*) p1 != *(short*) "//")
    p1++;
  p1 += 2;
  if (p1 >= p2)
    return -1;

  while (p1 < p2 && *p1 != '/')
    *p3++ = *p1++;
  *p3 = 0;
  p1++;

  while (p1 < p2)
    *p4++ = *p1++;
  *p4 = 0;

  *port = 80;

  p3 = strchr(addr, ':');
  if (p3) {
    *p3++ = 0;
    *port = atoi(p3);
  }

  return 0;
}

int DownLoad::GetRequestHead(char* host_addr, char* host_file, int port,
                             char* head_buf) {
  char* lastmodify = "Thu, 01 Jan 1970 00:00:00";
  char* p = head_buf;

  if (m_use_agent) {
    if (port == 80)
      p += sprintf(p, "GET http://%s/%s HTTP/1.0\r\n", host_addr, host_file);
    else
      p += sprintf(p, "GET http://%s:%d/%s HTTP/1.0\r\n", host_addr, port,
                   host_file);
    p += sprintf(p, "Proxy-authorization:Basic %s\r\n", m_proxy);
  } else {
    p += sprintf(p, "GET /%s HTTP/1.0\r\n", host_file);
  }
//	p += sprintf(p, "User-Agent: Opera/9.51 (Windows NT 5.1; U; zh-cn)\r\n");
  p += sprintf(p, "User-Agent:\r\n");
  p += sprintf(p, "Host: %s\r\n", host_addr);
  p +=
      sprintf(
          p,
          "Accept:text/html, application/xml;q=0.9, application/xhtml+xml, image/x-xbitmap, */*;q=0.1\r\n");
  p += sprintf(p, "If-Modified-Since:%sGMT\r\n\r\n", lastmodify);

  return p - head_buf;
}

int DownLoad::ParseHead(char* buf, int buf_len, char*& page, int& page_len,
                        char* jump_url, int& jump_len) {
  buf[buf_len] = 0;

  char* begin = buf;
  char* end = strstr(buf, "\r\n\r\n");
  if (end == NULL)
    return -1;
  while (begin < end && *begin == ' ')
    begin++;
  if (strnicmp(begin, "http", 4))
    return -2;
  begin += 4;
  if (*begin != '/' && *begin != '\\')
    return -3;
  begin++;
  while (begin < end && *begin != ' ')
    begin++;
  if (begin >= end)
    return -4;
  begin++;
  while (begin < end && *begin == ' ')
    begin++;
  int station = atol(begin);

  while (begin < end) {
    char* location = begin;
    while (begin < end && *begin != 0x0a)
      begin++;
    while (begin < end && (*begin == '\r' || *begin == '\n'))
      begin++;
    if (strnicmp(location, "Location", 8) == 0) {
      location += 8;
      while (location < begin && (*location == ':' || *location == ' '))
        location++;
      jump_len = 0;
      if (begin - location >= 1024)
        return -1;
      while (location < begin)
        jump_url[jump_len++] = *location++;
      jump_url[jump_len--] = 0;
      while (jump_len >= 0
          && (jump_url[jump_len] == '\r' || jump_url[jump_len] == '\n'
              || jump_url[jump_len] == ' '))
        jump_len--;
      jump_url[++jump_len] = 0;
      if (station == 301 || station == 302)
        return station;
    }
  }

  page = end + 4;
  end = buf + buf_len;
  while (page < end && (*page == ' ' || *page == '\r' || *page == '\n'))
    page++;
  page_len = buf_len - (page - buf);

  return station;
}

int DownLoad::SendData(int sockfd, char* buf, int send_len) {
  int ret, finish = 0;
  do {
    ret = send(sockfd, buf + finish, send_len - finish, 0);
    if (ret > 0)
      finish += ret;
  } while (ret > 0 && finish != send_len);
  if (finish != send_len)
    return -1;
  return 0;
}

int DownLoad::RecvData(int sockfd, char* buf, int recv_len) {
  int ret, finish = 0;
  do {
    ret = recv(sockfd, buf + finish, recv_len - finish, 0);
    if (ret > 0)
      finish += ret;
  } while (ret > 0 && finish != recv_len);

  return finish;
}

int DownLoad::to_real_addr(char* base, char* tmp_url, char* url,
                           char* tmp_buf) {
  char* p1 = tmp_url, *p2 = url;
  if (*p1 == '\"' || *p1 == '\'')
    p1++;

  char* baseurl = tmp_buf;
  strcpy(baseurl, base);

  if (strnicmp(p1, "http://", 7)) {
    if (*p1 == '/') {
      char* p = strstr(baseurl, "//");
      if (p)
        p = strchr(p + 2, '/');
      if (p)
        *p = 0;
      sprintf(url, "%s%s", baseurl, p1);
      if (p)
        *p = '/';
    } else if (p1[0] == '.' && p1[1] == '/') {
      char* p = strrchr(baseurl, '/');
      if (p)
        *p = 0;
      sprintf(url, "%s%s", baseurl, p1 + 1);
      if (p)
        *p = '/';
    } else {
      char* p = strstr(baseurl, "//");
      if (p)
        p = strrchr(p + 2, '/');
      if (p)
        *p = 0;
      sprintf(url, "%s/%s", baseurl, p1);
      if (p)
        *p = '/';
    }
  } else {
    strcpy(url, p1);
  }

  p1 = url;
  p2 = url;
  while (*p1) {
    *p2++ = *p1;
    if (memcmp(p1, "&amp;", 5))
      p1++;
    else
      p1 += 5;
  }
  *p2 = 0;

  return p2 - url;
}

int DownLoad::to_base64(char* instr, char* base64string) {
  char char_array_3[3];
  char char_array_4[4];
  char* p1 = instr, *p2 = base64string;
  char* base64table =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  long i = 0, j = 0;

  while (*p1) {
    char_array_3[i++] = *p1++;
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4)
          + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2)
          + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; i < 4; i++)
        *p2++ = base64table[char_array_4[i]];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4)
        + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2)
        + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; j < i + 1; j++)
      *p2++ = base64table[char_array_4[j]];

    while ((i++ < 3))
      *p2++ = '=';
  }
  *p2++ = 0;

  return p2 - base64string;
}

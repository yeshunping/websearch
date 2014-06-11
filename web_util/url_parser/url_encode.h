// Copyright 2010. All Rights Reserved.
// Author: shunpingye@gmail.com (Shunping Ye)

#ifndef WEB_UTIL_URL_URLENCODE_H_
#define WEB_UTIL_URL_URLENCODE_H_

#include <string>

namespace web_util {

int UrlDecode(const char *source, char *dest);
int UrlEncode(const char *source, char *dest, unsigned max);
std::string UrlDecodeString(const std::string & encoded);
std::string UrlEncodeString(const std::string & decoded);
}

#endif  //  WEB_UTIL_URL_URLENCODE_H_


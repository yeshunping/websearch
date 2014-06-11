// Copyright 2010. All Rights Reserved.
// Author: shunpingye@gmail.com (Shunping Ye)

#ifndef WEB_UTIL_URL_PARSER_URL_PARSER_H_
#define WEB_UTIL_URL_PARSER_URL_PARSER_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "base/hash_tables.h"
#include "base/scoped_ptr.h"
#include "thirdparty/googleurl/gurl.h"

namespace web_util {
namespace url {

bool IsValidUrl(const std::string& url);
bool GetSpecFormat(const std::string& url, std::string* out);
bool IsStandard(const std::string& url);

bool SchemeIsFile(const std::string& url);
bool SchemeIsSecure(const std::string& url);
bool HostIsIPAddress(const std::string& url);
bool GetScheme(const std::string& url, std::string* out);
bool GetUsername(const std::string& url, std::string* out);
bool GetPassword(const std::string& url, std::string* out);
bool GetHost(const std::string& url, std::string* out);
std::string GetHost(const std::string& url);
bool GetPort(const std::string& url, std::string* out);
bool GetPath(const std::string& url, std::string* out);
bool GetQuery(const std::string& url, std::string* out);
bool GetRef(const std::string& url, std::string* out);
bool GetRequestString(const std::string& url, std::string* out);

bool HasScheme(const std::string& url);
bool HasUsername(const std::string& url);
bool HasPassword(const std::string& url);
bool HasHost(const std::string& url);
bool HasPort(const std::string& url);
bool HasPath(const std::string& url);
bool HasQuery(const std::string& url);
bool HasRef(const std::string& url);

bool IsHostPage(const std::string& url);
// TODO(yesp) : implement this function
bool NormalizeURLForCrawler(const std::string& url, std::string* out);
bool GetDomain(const std::string& url, std::string* out);
std::string GetDomain(const std::string& url);
bool GetDomainFromHost(const std::string& host, std::string* out);
std::string GetDomainFromHost(const std::string& host);

bool IsSameDomain(const std::string &url1, const std::string &url2);

void ParseKvlist(
    const std::string& line,
    const std::string& key_value_delimiter,
    char key_value_pair_delimiter,
    std::map<std::string, std::string> *kv_pairs,
    std::vector<std::pair<std::string, std::string> >* vec);

class UrlParser {
 public:
  UrlParser();
  explicit UrlParser(const std::string& url);
  ~UrlParser();
  void ResetUrl(const std::string& url);
  void Resolve(const std::string& url, std::string* out) const;
  bool HostIsIPAddress() const;
  bool HasPort() const;

  bool GetHost(std::string* out) const;
  std::string GetHost() const;
  bool GetDomain(std::string* out);
  std::string GetDomain();
  std::string GetScheme() const;
  std::string GetRequestPath() const;
  std::string GetPath() const;

 private:
  scoped_ptr<GURL> gurl_;
  std::string url_;
  std::string host_;
  std::string domain_;
  bool domain_parsed_;

  DISALLOW_COPY_AND_ASSIGN(UrlParser);
};
}
}
#endif  // WEB_UTIL_URL_PARSER_URL_PARSER_H_

// Copyright 2010. All Rights Reserved.
// Author: shunpingye@gmail.com (Shunping Ye)

#include "web_util/url_parser/url_parser.h"

#include <algorithm>
#include <string>
#include <vector>

#include "base/es.h"
#include "base/flags.h"
#include "base/singleton.h"
#include "base/string_util.h"
#include "file/simple_line_reader.h"
#include "web_util/url_parser/registry_controlled_domain.h"

DEFINE_string(home_page_suffix, "conf/web_util/url_parser/home_page_suffix.dat", "");

namespace {
static const string kStarString = "*.";
}

namespace web_util {
namespace url {

bool IsValidUrl(const string& url) {
  GURL gurl(url);
  return gurl.is_valid();
}
bool GetSpecFormat(const string& url, string* out) {
  GURL gurl(url);
  out->assign(gurl.spec());
  return true;
}
bool IsStandard(const string& url) {
  GURL gurl(url);
  return gurl.IsStandard();
}

bool SchemeIsFile(const string& url) {
  GURL gurl(url);
  return gurl.SchemeIsFile();
}
bool SchemeIsSecure(const string& url) {
  GURL gurl(url);
  return gurl.SchemeIsSecure();
}
bool HostIsIPAddress(const string& url) {
  GURL gurl(url);
  return gurl.HostIsIPAddress();
}
bool GetScheme(const string& url, string* out) {
  GURL gurl(url);
  out->assign(gurl.scheme());
  return true;
}
bool GetUsername(const string& url, string* out) {
  GURL gurl(url);
  out->assign(gurl.username());
  return true;
}
bool GetPassword(const string& url, string* out) {
  GURL gurl(url);
  out->assign(gurl.password());
  return true;
}

bool GetHost(const string& url, string* out) {
  GURL gurl(url);
  out->assign(gurl.host());
  return true;
}

string GetHost(const string& url) {
  GURL gurl(url);
  return gurl.host();
}

bool GetPort(const string& url, string* out) {
  GURL gurl(url);
  out->assign(gurl.port());
  return true;
}

bool GetPath(const string& url, string* out) {
  GURL gurl(url);
  out->assign(gurl.path());
  return true;
}

bool GetQuery(const string& url, string* out) {
  GURL gurl(url);
  out->assign(gurl.query());
  return true;
}

bool GetRef(const string& url, string* out) {
  GURL gurl(url);
  out->assign(gurl.ref());
  return true;
}

bool GetRequestString(const string& url, string* out) {
  GURL gurl(url);
  out->assign(gurl.PathForRequest());
  return true;
}

bool HasScheme(const string& url) {
  GURL gurl(url);
  return gurl.has_scheme();
}

bool HasUsername(const string& url) {
  GURL gurl(url);
  return gurl.has_username();
}

bool HasPassword(const string& url) {
  GURL gurl(url);
  return gurl.has_password();
}

bool HasHost(const string& url) {
  GURL gurl(url);
  return gurl.has_host();
}

bool HasPort(const string& url) {
  GURL gurl(url);
  return gurl.has_port();
}

bool HasPath(const string& url) {
  GURL gurl(url);
  return gurl.has_path();
}

bool HasQuery(const string& url) {
  GURL gurl(url);
  return gurl.has_query();
}

bool HasRef(const string& url) {
  GURL gurl(url);
  return gurl.has_ref();
}

bool IsHostPage(const string& url) {
  GURL gurl(url);
  string domain;
  GetDomainFromHost(gurl.host(), &domain);
  if (gurl.host() == domain ||
      gurl.host() == string("www.") + domain) {
    if (gurl.PathForRequest() == "/" ||
          gurl.PathForRequest() == "/index.html" ||
          gurl.PathForRequest() == "/index.jsp" ||
          gurl.PathForRequest() == "/index.asp" ||
          gurl.PathForRequest() == "/index.aspx" ||
          gurl.PathForRequest() == "/index.htm" ||
          gurl.PathForRequest() == "/index.php") {
        return true;
      }
  }
  return false;
}


bool NormalizeURLForCrawler(const string& url, string* out) {
  out->assign(url);
  return true;
}

#include "web_util/url_parser/effective_tld_names.cc"

bool GetDomain(const string& url, string* out) {
  string host;
  GetHost(url, &host);
  return GetDomainFromHost(host, out);
}

string GetDomain(const string& url) {
  string host;
  GetHost(url, &host);
  string domain;
  return GetDomainFromHost(host);
}

bool GetDomainFromHost(const string& host, string* out) {
  out->assign(RegistryControlledDomainService::GetDomainAndRegistry(host));
  return !out->empty();
}

string GetDomainFromHost(const string& host) {
  return RegistryControlledDomainService::GetDomainAndRegistry(host);
}


bool IsSameDomain(const string &url1, const string &url2) {
  return GetDomain(url1) == GetDomain(url2);
}

void ParseKvlist(
    const string& line,
    const string& key_value_delimiter,
    char key_value_pair_delimiter,
    map<string, string> *kv_pairs,
    vector<pair<string, string> >* vec) {
  vector<string> pairs;
  vector<string> kvpair;
  SplitString(line, key_value_pair_delimiter, &pairs);
  VLOG(6) << "pairs num:" << pairs.size();
  for (size_t i = 0; i < pairs.size(); ++i) {
    kvpair.clear();
    string::size_type index = pairs[i].find(key_value_delimiter);
    if (index == string::npos) {
      continue;
    }
    string key = pairs[i].substr(0, index);
    string value = pairs[i].substr(index + key_value_delimiter.length());
    if (kv_pairs) {
      kv_pairs->insert(make_pair(key, value));
    }
    if (vec) {
      vec->push_back(make_pair(key, value));
    }
  }
}

UrlParser::UrlParser()
  : domain_parsed_(false) {
}
UrlParser::UrlParser(const string& url)
  : gurl_(new GURL(url)),
    url_(url),
    domain_parsed_(false) {
  host_ = gurl_->host();
}

UrlParser::~UrlParser() {}

void UrlParser::ResetUrl(const std::string& url) {
  gurl_.reset(new GURL(url));
  url_ = url;
  host_ = gurl_->host();
  domain_ = "";
  domain_parsed_ = false;
}

void UrlParser::Resolve(const string& url, string* out) const {
  out->assign(gurl_->Resolve(url).spec());
}

bool UrlParser::HostIsIPAddress() const {
  return gurl_->HostIsIPAddress();
}

bool UrlParser::HasPort() const {
  return gurl_->has_port();
}

bool UrlParser::GetHost(string* out) const {
  out->assign(host_);
  return true;
}

string UrlParser::GetHost() const {
  return host_;
}

bool UrlParser::GetDomain(string* out) {
  if (!domain_parsed_) {
    domain_parsed_ = true;
    if (!GetDomainFromHost(host_, &domain_))
      return false;
  }
  out->assign(domain_);
  return true;
}

string UrlParser::GetDomain() {
  if (!domain_parsed_) {
    domain_parsed_ = true;
    if (!GetDomainFromHost(host_, &domain_))
      return "";
  }
  return domain_;
}

string UrlParser::GetScheme() const {
  return gurl_->scheme();
}

string UrlParser::GetRequestPath() const {
  return gurl_->PathForRequest();
}

string UrlParser::GetPath() const {
  return gurl_->path();
}
}
}

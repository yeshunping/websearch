// Copyright 2010. All Rights Reserved.
// Author: shunpingye@gmail.com (Shunping Ye)

#include "base/logging.h"
#include "base/es.h"
#include "web_util/url_parser/url_parser.h"
#include "thirdparty/gtest/gtest.h"

namespace web_util {
namespace url {
TEST(GetDomainUnittest, test) {
  string domain;
  EXPECT_TRUE(GetDomain("http://abcde.co.uk/", &domain));
  EXPECT_EQ(domain, "abcde.co.uk");
  domain.clear();
  EXPECT_TRUE(GetDomain("http://ab.cde.co.uk/", &domain));
  EXPECT_EQ(domain, "cde.co.uk");
  domain.clear();
  EXPECT_TRUE(GetDomain("http://www.baidu.com/", &domain));
  EXPECT_EQ(domain, "baidu.com");
  domain.clear();
  EXPECT_TRUE(GetDomain("http://www.sina.com.cn/", &domain));
  EXPECT_EQ(domain, "sina.com.cn");
  EXPECT_FALSE(GetDomain("http://cn/", &domain));
  EXPECT_FALSE(GetDomain("http://com.cn/", &domain));
  domain.clear();
  EXPECT_TRUE(GetDomain("http://xxx.com.cn/", &domain));
  EXPECT_EQ(domain, "xxx.com.cn");
  domain.clear();
  EXPECT_TRUE(GetDomain("http://xxx.baidu.cc/", &domain));
  EXPECT_EQ(domain, "baidu.cc");
  domain.clear();
  EXPECT_TRUE(GetDomainFromHost("xiechuyan.0739.cc", &domain));
  EXPECT_EQ(domain, "0739.cc");
  domain.clear();
  EXPECT_TRUE(GetDomainFromHost("city.kawasaki.jp", &domain));
  EXPECT_EQ(domain, "city.kawasaki.jp");
  domain.clear();
  EXPECT_FALSE(GetDomainFromHost("a.kawasaki.jp", &domain));
  EXPECT_TRUE(GetDomain("http://a.b.c..com.cn/", &domain));
  EXPECT_EQ(domain, ".com.cn");

  EXPECT_FALSE(GetDomain("http://192.168.1.1/", &domain));

  // TODO: why are these valid domains valid ?
  EXPECT_TRUE(GetDomain("http://192.168.1.acd/", &domain));
  EXPECT_EQ(domain, "1.acd");
  EXPECT_TRUE(GetDomain("http://www.a/", &domain));
  EXPECT_EQ(domain, "www.a");

  EXPECT_TRUE(GetDomain("http://www.qld.edu.au/", &domain));
  EXPECT_EQ(domain, "www.qld.edu.au");
}

TEST(GetSchemeUnittest, test) {
  {
    UrlParser parser("http://abcde.co.uk/");
    EXPECT_EQ("http", parser.GetScheme());
  }

  {
    UrlParser parser("http://news.sina.com.cn/c/2013-10-19/112928476796.shtml");
    string url;
    parser.Resolve("i0.sinaimg.cn/dy/c/2013-10-19/1382154034_oCnAid.jpg", &url);
    LOG(INFO) << url;
  }


  {
    UrlParser parser("https://abcde.co.uk/");

    EXPECT_EQ("https", parser.GetScheme());
  }
}
}
}  // namespace util

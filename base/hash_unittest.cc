// Copyright 2012 . All Rights Reserved.
// Author: yeshunping@gmail.com (Shunping Ye)

#include <string>

#include "base/hash.h"
#include "base/es.h"
#include "thirdparty/gtest/gtest.h"

TEST(HashUnittest, Fingerprint) {
  const string& url = "http://app.kid.qq.com/exam/5528/5528_103392.htm";
  uint64 hash_value = Fingerprint(url);
  EXPECT_EQ(hash_value, 17105673616436300159UL);
  string str = FingerprintToString(hash_value);
  EXPECT_EQ(str, "7F09753F868F63ED");
  uint64 hash2 = StringToFingerprint(str);
  EXPECT_EQ(hash_value, hash2);
}

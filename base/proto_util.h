// Copyright 2010. All Rights Reserved.
// Author: shunpingye@gmail.com (Shunping Ye)

#ifndef BASE_PROTO_UTIL_H_
#define BASE_PROTO_UTIL_H_

#include <string>

#include "thirdparty/protobuf/include/google/protobuf/text_format.h"

namespace base {
namespace proto_util {

template<class Message>
bool ParseFromString(const std::string& debug_str, Message* out) {
  return ParseFromASCII(debug_str, out);
}

template<class Message>
bool ParseFromASCII(const std::string& debug_str, Message* out) {
  google::protobuf::TextFormat::Parser parser;
  return parser.ParseFromString(debug_str, out);
}
}
}
#endif  // BASE_PROTO_UTIL_H_

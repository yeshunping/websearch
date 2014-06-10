// Copyright (c) 2013, The util Authors. All rights reserved.
// Author: Ye Shunping <yeshunping@gmail.com>

#ifndef UTIL_PROTOBUF_PROTO_JSON_FORMAT_H_
#define UTIL_PROTOBUF_PROTO_JSON_FORMAT_H_

#include <string>

#include "base/basictypes.h"

namespace Json {
class Value;
}

namespace google {
namespace protobuf {
class Message;
}
}

namespace util {

// This class implements protocol buffer json format.  Printing and parsing
// protocol messages in json format is useful for javascript
class ProtoJsonFormat {
 public:
  static bool PrintToStyledString(const google::protobuf::Message& message,
                                  std::string* output);

  static bool PrintToFastString(const google::protobuf::Message& message,
                                std::string* output);

  static bool WriteToValue(const google::protobuf::Message& message,
                           Json::Value* output);

  static bool ParseFromValue(const Json::Value& input,
                             google::protobuf::Message* output);

  static bool ParseFromString(const std::string& input,
                              google::protobuf::Message* output);

 private:
  DISALLOW_COPY_AND_ASSIGN(ProtoJsonFormat);
};

}  // namespace util

#endif  // UTIL_PROTOBUF_PROTO_JSON_FORMAT_H_

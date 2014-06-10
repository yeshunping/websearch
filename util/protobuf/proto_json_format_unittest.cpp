// Copyright (c) 2013, The util Authors. All rights reserved.
// Author: Ye Shunping <yeshunping@gmail.com>

#include "util/protobuf/proto_json_format.h"

#include <math.h>
#include <stdlib.h>
#include <limits>
#include <string>

#include "base/logging.h"
#include "file/file.h"
#include "util/protobuf/unittest.pb.h"

#include "thirdparty/protobuf/include/google/protobuf/text_format.h"
#include "thirdparty/gtest/gtest.h"
#include "thirdparty/jsoncpp/json.h"

namespace util {

TEST(JsonFormatUnittest, PrintToJson) {
    util::Person p;
    p.set_age(30);
    util::NameInfo* name = p.mutable_name();
    name->set_first_name("Ye");
    name->set_second_name("Shunping");
    p.set_address("beijing");
    p.add_phone_number("15100000000");
    p.add_phone_number("15100000001");
    p.set_address_id(434798436777434024L);
    p.set_people_type(util::HAN_ZU);
    LOG(INFO)<< "Text format for Message:\n" << p.Utf8DebugString();

    std::string styled_str;
    ProtoJsonFormat::PrintToStyledString(p, &styled_str);
    std::string expected_styled_str;
    file::File::ReadFileToString("util/protobuf/json_styled_string.txt", &expected_styled_str);
    EXPECT_EQ(styled_str, expected_styled_str);

    std::string fast_str;
    ProtoJsonFormat::PrintToFastString(p, &fast_str);
    VLOG(3)<< "FastString json format for Message:\n" << fast_str;
    std::string expected_fast_str;
    file::File::ReadFileToString("util/protobuf/json_fast_string.txt", &expected_fast_str);
    EXPECT_EQ(fast_str, expected_fast_str);
}

void TestJsonString(const std::string& json_file) {
    std::string styled_str;
    file::File::ReadFileToString(json_file, &styled_str);
    util::Person pb;
    ProtoJsonFormat::ParseFromString(styled_str, &pb);

    util::Person expected_pb;
    {
        std::string content;
        std::string path = "util/protobuf/debug_string.txt";
        file::File::ReadFileToString(path, &content);
        google::protobuf::TextFormat::ParseFromString(content, &expected_pb);
    }
    EXPECT_EQ(pb.SerializeAsString(), expected_pb.SerializeAsString());
}

TEST(JsonFormatUnittest, ParseFromFastJsonString) {
    std::string path = "util/protobuf/json_fast_string.txt";
    TestJsonString(path);
}

TEST(JsonFormatUnittest, ParseFromStyledJsonString) {
    std::string path = "util/protobuf/json_styled_string.txt";
    TestJsonString(path);
}
}  // namespace util

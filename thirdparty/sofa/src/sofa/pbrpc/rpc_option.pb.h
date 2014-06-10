// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: thirdparty/sofa/src/sofa/pbrpc/rpc_option.proto

#ifndef PROTOBUF_thirdparty_2fsofa_2fsrc_2fsofa_2fpbrpc_2frpc_5foption_2eproto__INCLUDED
#define PROTOBUF_thirdparty_2fsofa_2fsrc_2fsofa_2fpbrpc_2frpc_5foption_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2005000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include "google/protobuf/descriptor.pb.h"
// @@protoc_insertion_point(includes)

namespace sofa {
namespace pbrpc {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_thirdparty_2fsofa_2fsrc_2fsofa_2fpbrpc_2frpc_5foption_2eproto();
void protobuf_AssignDesc_thirdparty_2fsofa_2fsrc_2fsofa_2fpbrpc_2frpc_5foption_2eproto();
void protobuf_ShutdownFile_thirdparty_2fsofa_2fsrc_2fsofa_2fpbrpc_2frpc_5foption_2eproto();


enum CompressType {
  CompressTypeNone = 0,
  CompressTypeGzip = 1,
  CompressTypeZlib = 2,
  CompressTypeSnappy = 3,
  CompressTypeLZ4 = 4
};
bool CompressType_IsValid(int value);
const CompressType CompressType_MIN = CompressTypeNone;
const CompressType CompressType_MAX = CompressTypeLZ4;
const int CompressType_ARRAYSIZE = CompressType_MAX + 1;

const ::google::protobuf::EnumDescriptor* CompressType_descriptor();
inline const ::std::string& CompressType_Name(CompressType value) {
  return ::google::protobuf::internal::NameOfEnum(
    CompressType_descriptor(), value);
}
inline bool CompressType_Parse(
    const ::std::string& name, CompressType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<CompressType>(
    CompressType_descriptor(), name, value);
}
// ===================================================================


// ===================================================================

static const int kServiceTimeoutFieldNumber = 20000;
extern ::google::protobuf::internal::ExtensionIdentifier< ::google::protobuf::ServiceOptions,
    ::google::protobuf::internal::PrimitiveTypeTraits< ::google::protobuf::int64 >, 3, false >
  service_timeout;
static const int kMethodTimeoutFieldNumber = 20000;
extern ::google::protobuf::internal::ExtensionIdentifier< ::google::protobuf::MethodOptions,
    ::google::protobuf::internal::PrimitiveTypeTraits< ::google::protobuf::int64 >, 3, false >
  method_timeout;
static const int kRequestCompressTypeFieldNumber = 20001;
extern ::google::protobuf::internal::ExtensionIdentifier< ::google::protobuf::MethodOptions,
    ::google::protobuf::internal::EnumTypeTraits< ::sofa::pbrpc::CompressType, ::sofa::pbrpc::CompressType_IsValid>, 14, false >
  request_compress_type;
static const int kResponseCompressTypeFieldNumber = 20002;
extern ::google::protobuf::internal::ExtensionIdentifier< ::google::protobuf::MethodOptions,
    ::google::protobuf::internal::EnumTypeTraits< ::sofa::pbrpc::CompressType, ::sofa::pbrpc::CompressType_IsValid>, 14, false >
  response_compress_type;

// ===================================================================


// @@protoc_insertion_point(namespace_scope)

}  // namespace pbrpc
}  // namespace sofa

#ifndef SWIG
namespace google {
namespace protobuf {

template <>
inline const EnumDescriptor* GetEnumDescriptor< ::sofa::pbrpc::CompressType>() {
  return ::sofa::pbrpc::CompressType_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_thirdparty_2fsofa_2fsrc_2fsofa_2fpbrpc_2frpc_5foption_2eproto__INCLUDED

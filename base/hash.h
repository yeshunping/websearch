// Copyright 2009 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This file defines a url hash function.

#ifndef BASE_HASH_H_
#define BASE_HASH_H_

#include <string>

#include "base/basictypes.h"
#include "base/string_piece.h"

namespace base {
uint32 Fingerprint32(const std::string &key);
uint32 Fingerprint32(const char *str, size_t length);
uint32 Fingerprint32WithSeed(const std::string &key,
                             uint32 seed);
uint32 Fingerprint32WithSeed(const char *str,
                             size_t length,
                             uint32 seed);
uint32 Fingerprint32WithSeed(const char *str,
                             uint32 seed);

uint64 Fingerprint(const std::string& str);
uint64 Fingerprint(const StringPiece& str);
std::string FingerprintToString(uint64);
uint64 StringToFingerprint(const std::string& str);
uint64 MurmurHash64A(const void* key, int len, uint32 seed);

//  The Jenkins hash functions are a collection of
//  (non-cryptographic) hash functions for multi-byte
//  keys designed by Bob Jenkins. They can be used also as
//  checksums to detect accidental data corruption or detect
//  identical records in a database.
//  see http://en.wikipedia.org/wiki/Jenkins_hash_function
//  for details
uint32 JenkinsOneAtATimeHash(const std::string& str);
uint32 JenkinsOneAtATimeHash(const char *key, size_t len);
}  //  namespace base
#endif  // BASE_HASH_H_

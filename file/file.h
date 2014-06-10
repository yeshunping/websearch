// Copyright 2013. All Rights Reserved.
// Author: yeshunping@gmail.com (Shunping Ye)

#ifndef FILE_FILE_H_
#define FILE_FILE_H_

#include <stdio.h>

#include <string>
#include <vector>

#include "base/file_util.h"

namespace file {
class File {
 public:
  static bool IsDir(const std::string& dir);

  static bool IsRegFile(const std::string& path);

  static bool Exists(const std::string& filename);
  static std::string GetCurrentDir();

  static bool WriteLinesToFile(const std::vector<std::string>& lines,
                               const std::string& filename);

  static bool AppendLinesToFile(const std::vector<std::string>& lines,
                               const std::string& filename);

  static bool ReadFileToLines(const std::string& filename,
                              std::vector<std::string>* lines);

  static void WriteLinesToFileOrDie(const std::vector<std::string>& lines,
                                    const std::string& filename);

  static bool WriteStringToFile(const std::string& content,
                                const std::string& filename);
  static void WriteStringToFileOrDie(const std::string& content,
                                     const std::string& filename);
  static bool AppendStringToFile(const std::string& content,
                                    const std::string& filename);

  static bool ReadFileToString(const std::string& filename,
                               std::string* out);
  static void ReadFileToStringOrDie(const std::string& filename,
                                    std::string* out);

  static const std::string MakeTempFile(const std::string& prefix);

  static bool CreateDir(const std::string& dir, int mode);

  static bool CreateDirDeeply(const std::string& path, int mode);
  static bool CreateDirIfMissing(const std::string &dir);

  static bool DeleteFile(const std::string& filename);

  static bool CopyFile(const std::string& from,
                       const std::string& to);
  static bool RenameFile(const std::string& from,
                         const std::string& to);
  static bool GetFilesInDir(const std::string& dir,
                            std::vector<std::string>* out);
  static bool GetDirsInDir(const std::string& dir,
                            std::vector<std::string>* out);
  static bool GetFilesInDirDeeply(const std::string& dir,
                                  std::vector<std::string>* out);

  static bool FileSize(const std::string& file, size_t* size);

  static std::string JoinPath(const std::string& base_path,
                              const std::string& path);

  static std::string GetExtension(const std::string& path);

  static bool CreateEmptyFile(const std::string& fname);

};

// A class for enumerating the files in a provided path. The order of the
// results is not guaranteed.
//
// DO NOT USE FROM THE MAIN THREAD of your application unless it is a test
// program where latency does not matter. This class is blocking.
class FileEnumerator {
 public:
  enum FILE_TYPE {
    FILES                 = 1 << 0,
    DIRECTORIES           = 1 << 1,
    INCLUDE_DOT_DOT       = 1 << 2,
    SHOW_SYM_LINKS        = 1 << 4,
  };
  FileEnumerator(const std::string& root_path,
                 bool recursive,
                 FileEnumerator::FILE_TYPE file_type);
  ~FileEnumerator();

  // Returns an empty string if there are no more results.
  std::string Next();
 private:
  file_util::FileEnumerator inner_enumerator_;
  DISALLOW_COPY_AND_ASSIGN(FileEnumerator);
};
}

#endif  // FILE_FILE_H_

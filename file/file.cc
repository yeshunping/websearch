// Copyright 2013. All Rights Reserved.
// Author: yeshunping@gmail.com (Shunping Ye)

#include "file/file.h"

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "base/time.h"
#include "base/logging.h"
#include "base/string_util.h"
#include "base/ns.h"
#include "base/file_path.h"

namespace file {

bool File::IsDir(const string& dir) {
  struct stat buf;

  if (lstat(dir.c_str(), &buf) < 0) {
    LOG(ERROR) << "lstat error for dir:" << dir;
    return false;
  }

  return S_ISDIR(buf.st_mode);
}

bool File::IsRegFile(const string& path) {
  struct stat buf;

  if (lstat(path.c_str(), &buf) < 0) {
    LOG(ERROR) << "lstat error for dir:" << path;
    return false;
  }

  return S_ISREG(buf.st_mode);
}

bool File::Exists(const string& filename) {
  return (access(filename.c_str(), 0) == 0);
}

string File::GetCurrentDir() {
  char* buff = get_current_dir_name();
  string ret(buff);
  free(buff);
  return ret;
}

bool File::WriteLinesToFile(const vector<string>& lines,
                            const string& filename) {
  string content = JoinString(lines, '\n');
  return WriteStringToFile(content, filename);
}

bool File::AppendLinesToFile(const std::vector<std::string>& lines,
                             const std::string& filename) {
  string content = JoinString(lines, '\n');
  return AppendStringToFile(content, filename);
}

void File::WriteLinesToFileOrDie(const vector<string>& lines,
                                 const string& filename) {
  CHECK(WriteLinesToFile(lines, filename));
}


bool File::ReadFileToLines(const string& filename,
                           vector<string>* lines) {
  string content;
  if (!ReadFileToString(filename, &content)) {
    return false;
  }

  SplitString(content, '\n', lines);

  return true;
}

bool File::WriteStringToFile(const string& content,
                             const string& filename) {
  FILE* fp = fopen(filename.c_str(), "wb");
  if (!fp) {
    LOG(ERROR) << "fail to open file: " << filename;
    return false;
  }

  if (fwrite(content.c_str() , 1, content.length(), fp)
      != content.length()) {
    LOG(INFO) << "fwrite fail!";
    return false;
  }
  fclose(fp);
  return true;
}

void File::WriteStringToFileOrDie(const string& content,
                                   const string& filename) {
  CHECK(WriteStringToFile(content, filename));
}

bool File::ReadFileToString(const string& filename,
                            string* out) {
  FILE* fp = fopen(filename.c_str(), "rb");
  if (fp == NULL) {
    LOG(ERROR) << "fail to open file:" << filename;
    return false;
  }
  char buf[1 << 16];
  size_t len = 0;
  while ((len = fread(buf, 1, sizeof(buf), fp)) > 0) {
      out->append(buf, len);
  }
  fclose(fp);
  return true;
}

void File::ReadFileToStringOrDie(const string& filename,
                                  string* out) {
  CHECK(File::ReadFileToString(filename, out));
}

const string File::MakeTempFile(const string& prefix) {
  // TODO(yesp) : rewrite this function using base/time.h
//  char buff[] = "abcdefghijkXXXXXX";
//  int ret = mkstemp(buff);
//  CHECK(ret != -1);
//  return prefix + buff;
  Time::Exploded exploded;
  Time::Now().UTCExplode(&exploded);
  return StringPrintf("%s_%04d%02d%02d%02d%02d%02d%03d",
                      prefix.c_str(),
                      exploded.year,
                      exploded.month,
                      exploded.day_of_month,
                      exploded.hour,
                      exploded.minute,
                      exploded.second,
                      exploded.millisecond);
}

bool File::AppendStringToFile(const string& content,
                              const string& filename) {
  // TODO(yesp) :
  FILE* fp = fopen(filename.c_str(), "ab");
  if (!fp) {
    LOG(ERROR) << "fail to open file: " << filename;
    return false;
  }

  if (fwrite(content.c_str() , 1, content.length(), fp)
      != content.length()) {
    LOG(INFO) << "fwrite fail!";
    return false;
  }
  fclose(fp);

  return true;
}

bool File::DeleteFile(const string& filename) {
  return remove(filename.c_str()) == 0;
}

bool File::CreateDir(const string& dir, int mode) {
  int ret = mkdir(dir.c_str(), mode);
  return ret == 0;
}

bool File::CreateDirDeeply(const string& path, int mode) {
  string buff = *path.rbegin()=='/' ? path : path + '/';
  char* index = &buff[0];
  while (*index != '\0') {
    if (*index == '/') {
      *index = '\0';
    if (buff[0] == '\0') {
      *index++ = '/';
      continue;
    }
      if (!Exists(buff)) {
        if (!CreateDir(buff, mode)) {
          return false;
        }
      }
      *index = '/';
    }
    ++index;
  }
  return true;
}


bool File::CreateDirIfMissing(const string &dir) {
  size_t pos1 = 0, pos2 = 0;
  while (pos2 != string::npos) {
    pos2 = dir.find('/', pos1);
    pos1 = pos2 + 1;
    if (pos2 == 0) {
      continue;
    }

    string parent = dir.substr(0, pos2);
    struct stat st;
    if (stat(parent.c_str(), &st) == -1) {
      int ret = mkdir(parent.c_str(), 0755);
      if (ret == -1) {
        LOG(ERROR)<< "Failed to create :" << dir <<" err=" << strerror(errno);
        return false;
      }
    } else if (!S_ISDIR(st.st_mode)) {
      LOG(ERROR) << "Failed to create " << dir << ", "
                 << parent << " already exists";
      return false;
    }
  }
  return true;
}

bool File::CopyFile(const string& from,
                    const string& to) {
  string buff;
  CHECK(ReadFileToString(from, &buff));
  return WriteStringToFile(buff, to);
  return true;
}

bool File::RenameFile(const std::string& from,
                      const std::string& to) {
  return ::rename(from.c_str(), to.c_str()) == 0;
}

bool File::GetFilesInDir(const string& dir,
                         vector<string>* out) {
  DIR *dp;
  struct dirent *dirp;
  if ((dp  = opendir(dir.c_str())) == NULL) {
      LOG(ERROR) << "fail to open dir: " << dir;
      return false;
  }

  while ((dirp = readdir(dp)) != NULL) {
    string path;
    if (dir[dir.length() - 1] == '/') {
      path = dir + dirp->d_name;
    } else {
      path = dir + "/" + dirp->d_name;
    }
    if (!IsDir(path)) {
      out->push_back(path);
    }
  }
  closedir(dp);
  return true;
}

bool File::GetDirsInDir(const string& dir,
                        vector<string>* out) {
  DIR *dp;
  struct dirent *dirp;
  if ((dp  = opendir(dir.c_str())) == NULL) {
      LOG(ERROR) << "fail to open dir: " << dir;
      return false;
  }

  while ((dirp = readdir(dp)) != NULL) {
    if (IsDir(dir + "/" + dirp->d_name)) {
      out->push_back(string(dirp->d_name));
    }
  }
  closedir(dp);
  return true;
}

bool File::GetFilesInDirDeeply(const string& dir,
                                vector<string>* out) {
  if (!GetFilesInDir(dir, out)) {
    return false;
  }
  vector<string> sub_dirs;
  if (GetDirsInDir(dir, &sub_dirs)) {
    return false;
  }
  for (size_t i = 0; i < sub_dirs.size(); ++i) {
    if (!GetFilesInDirDeeply(sub_dirs[i], out)) {
      return false;
    }
  }
  return true;
}

bool File::FileSize(const string& file, size_t* size) {
  CHECK(Exists(file));
  CHECK(!IsDir(file));
  struct stat st;
  if (stat(file.c_str(), &st) < 0) {
    return false;
  }
  *size = st.st_size;
  return true;
}

string File::JoinPath(const string& base_path, const string& path) {
  // TODO(yesp) : implement another version not using boost
  FilePath dir(base_path);
  return dir.Append(path).value();
}

bool File::CreateEmptyFile(const std::string& fname) {
  FILE* fp = fopen(fname.c_str(), "wb");
  if (!fp) {
    LOG(ERROR) << "fail to open file: " << fname;
    return false;
  }
  fclose(fp);
  return true;
}

std::string File::GetExtension(const string& path) {
  FilePath p_obj(path);
  return p_obj.Extension();
}

FileEnumerator::FileEnumerator(
    const std::string& root_path,
    bool recursive,
    FileEnumerator::FILE_TYPE file_type)
    : inner_enumerator_(FilePath(root_path),
        recursive,
        static_cast<file_util::FileEnumerator::FILE_TYPE>(file_type)) {
}

FileEnumerator::~FileEnumerator() {
}

// Returns an empty string if there are no more results.
string FileEnumerator::Next() {
  return inner_enumerator_.Next().value();
}

}

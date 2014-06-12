#!/usr/bin/env python

# Copyright (c) 2002, Google Inc.
# All rights reserved.

import os

def GetFilesInDir(path):
    files = list()
    for fname in dirList:
        if not os.path.isdir(fname):
            files.append(fname)
    return files

def GetFilesInDirRecursively(path, files):
    dirList = os.listdir(path)
    for fname in dirList:
        abs_path = path + '/' + fname
        #print "abs_path:", abs_path
        if not os.path.isdir(abs_path):
            files.append(abs_path)
        else:
            GetFilesInDirRecursively(abs_path, files)

def UpdateIncludedPath(file):
    fh = open(file)
    lines = fh.readlines()
    fh.close()
    fsave = open(file, 'w')
    key1 = '#include "boost'
    len1 = len(key1)
    key2 = '#include <boost'
    len2 = len(key2)
    for line in lines:
        if (line.find("include") != -1 and line.find("boost") != -1) or \
         (line.find("define ") != -1 and line.find("boost") != -1) and line.find("boost::") == -1:
            #print "original:", line,
            new_line = line.replace("boost/", "thirdparty/boost/boost/")
            new_line = new_line.replace('<', '"')
            new_line = new_line.replace('>', '"')
            #print "new_line:", new_line,
            fsave.write(new_line)
        else:
            fsave.write(line)
    fsave.close()

if __name__ == "__main__":
    path = "./boost"
    files = list()
    GetFilesInDirRecursively(path, files)
    for file in files:
        if file.find('.svn') != -1:
          continue;
        print "process file:", file
        UpdateIncludedPath(file)
        pass

    print "done!"

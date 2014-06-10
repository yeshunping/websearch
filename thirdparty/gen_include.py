#!/usr/bin/env python
#
# Copyright (c) 2002, Google Inc.
# All rights reserved.

import os
import sys

def GetFilesInDir(path):
    files = list()
    for fname in dirList:
        if not os.path.isdir(fname):
            files.append(fname)
    return files

def GetFilesInDirRecursively(path, files):
    dirList = os.listdir(path)
    for fname in dirList:
        abs_path = os.path.join(path, fname)
        #print "abs_path:", abs_path
        if not os.path.isdir(abs_path):
            files.append(abs_path)
        else:
            GetFilesInDirRecursively(abs_path, files)

def GenIncludeFile(src_dir, file, target):
    basename = file[len(src_dir):]
    print basename
    target_file = os.path.join(target, basename)
    print "target file:", target_file
    dir_name = os.path.dirname(target_file)
    if not os.path.exists(dir_name):
        os.makedirs(dir_name)
    fh = open(target_file, 'w')
    fh.write(str('#include "%s"') % (file))
    fh.close()

if __name__ == "__main__":
    src_dir = sys.argv[1]
    target_dir = sys.argv[2]
    if not os.path.exists(target_dir):
        os.makedirs(target_dir)
    files = list()
    GetFilesInDirRecursively(src_dir, files)
    for file in files:
        if file.find('.svn') != -1 or not file.endswith('.h'):
            continue;
        print "process file:", file
        GenIncludeFile(src_dir, file, target_dir)
        pass

    print "done!"

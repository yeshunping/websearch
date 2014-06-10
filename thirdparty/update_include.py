#!/usr/bin/env python
#
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
    key1 = '#include "unicode/'
    len1 = len(key1)
    key2 = '#include "'
    len2 = len(key2)
    for line in lines:
		if line.find('include "util/') != -1:
			#print "original:", line,
			new_line = line.replace("util/", "thirdparty/re2/util/")
			#print "new_line:", new_line,
			fsave.write(new_line)
			continue
		if line.find('include "re2/') != -1:
			#print "original:", line,
			new_line = line.replace('re2/', 'thirdparty/re2/re2/')
			#print "new_line:", new_line,
			fsave.write(new_line)
			continue
		fsave.write(line)
    fsave.close()

if __name__ == "__main__":
    path = "./re2"
    files = list()
    GetFilesInDirRecursively(path, files)
    for file in files:
	if file.find('.svn') != -1:
	  continue;
        print "process file:", file
        UpdateIncludedPath(file)
        pass

    print "done!"

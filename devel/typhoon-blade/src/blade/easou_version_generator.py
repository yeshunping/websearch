#!/bin/python

"""
 Copyright (c) 2014 Easou Inc. All rights reserved.
 Author: Shunping Ye <shunping_ye@staff.easou.com>
This class will generate version info for easou binaries.
"""

import os
import subprocess
import string
import time
import socket

class VersionFileGenerator:
    def __init__(self, build_strategy):
        self.build_strategy = build_strategy

    def _get_svn_version(self):
        lines = os.popen('svn info').read().split('\n')
        key_en = 'Revision:'
        for f in lines:
            if f.startswith(key_en):
              return f[len(key_en) + 1:]
        assert(False); 
        print ''
  
    def _get_svn_url(self):
        lines = os.popen('svn info').read().split('\n')
        key_en = 'URL: '
        for f in lines:
            if f.startswith(key_en):
              return f[len(key_en):]
        assert(False); 
        print 'error'
  
    def _get_gcc_version(self):
        lines = os.popen('g++ --version').read().split('\n')
        return lines[0]
  
    def _get_os_version(self):
        lines = os.popen('uname -rv').read().split('\n')
        return lines[0]
  
    def _gen_easou_format_version_file(self):
        # generate /tmp/version.h ,which contains some helpful info for debuging.
        fh = open('base/version.h.template')
        content = fh.read()
        fh.close()
        svn_version = self._get_svn_version()
        bin_version = '1.3.' + svn_version
        svn_url = self._get_svn_url()
        gcc_version = self._get_gcc_version()
        os_version = self._get_os_version()
        build_time = time.strftime("%Y/%m/%d %H:%M:%S", time.localtime())
        build_host = socket.gethostname()
        build_strategy = self.build_strategy
        content = content.replace('BIN_VERSION_TEMPLATE', bin_version)
        content = content.replace('SVN_VERSION_TEMPLATE', svn_version)
        content = content.replace('SVN_URL_TEMPLATE', svn_url)
        content = content.replace('GCC_VERSION_TEMPLATE', gcc_version)
        content = content.replace('OS_VERSION_TEMPLATE', os_version)
        content = content.replace('BUILD_TIME_TEMPLATE', build_time)
        content = content.replace('BUILD_HOST_TEMPLATE', build_host)
        content = content.replace('BUILD_STRATEGY_TEMPLATE', build_strategy)
        fh = open('base/version.h', 'w')
        fh.write(content)
        fh.close()

if __name__ == "__main__":
    generator = VersionFileGenerator("opt") 
    generator._gen_easou_format_version_file()


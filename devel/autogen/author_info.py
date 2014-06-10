#!/usr/bin/python2.6
#

"""Generate author info.

"""
import os
import re
import sys

username2mail = dict()
username2mail['shunpingye'] = 'yeshunping'
username2mail['shunping'] = 'yeshunping'

username_dict = dict()
username_dict['shunpingye'] = 'Shunping Ye'
username_dict['shunping'] = 'Shunping Ye'

def GetAuthorInfo():
  user = os.environ['USER']
#  name = os.environ['USERNAME']
  return '%s@gmail.com (%s)' % (username2mail[user], username_dict[user])

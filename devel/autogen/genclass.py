#!/usr/bin/python2.6

import sys
import os

#print os.getcwd()
sys.path.append('python/base/')

import gflags
FLAGS = gflags.FLAGS

gflags.DEFINE_string('cname', '', 'class name')
gflags.DEFINE_string('namespace', '', 'namespace for this class')

def GenClassBoilerplate(class_name, namespace):
    return str("namespace %s {\n"
               "class %s {\n"
               " public:\n"
               "  %s();\n"
               "  ~%s();\n"
               "\n"
               " private:\n"
               "\n"
               "  DISALLOW_COPY_AND_ASSIGN(%s);\n"
               "};\n"
               "}  //  namespace %s"
                % (namespace, class_name, class_name,
                    class_name, class_name, namespace))

def main(argv):
  try:
    argv = FLAGS(argv)
  except gflags.FlagsError, e:
    print '%s\\nUsage: %s ARGS\\n%s' % (e, sys.argv[0], FLAGS)
    sys.exit(1)
  bp_str = GenClassBoilerplate(FLAGS.cname, FLAGS.namespace)
  print bp_str

if __name__ == '__main__':
  main(sys.argv)

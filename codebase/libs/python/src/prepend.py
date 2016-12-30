#!/usr/bin/env python

""" Insert a copyright notice in Makefile, *.c, *.h, *.cc and *.hh """

import getopt
import os
import re
import sys

# Change this regular expression if you wish to modify the files that will be prepended
cregexp = re.compile(r""".*\.c$
                    | .*\.h$
                    | .*\.cc$
                    | .*\.hh$""", re.VERBOSE)

mregexp = re.compile("Makefile")

regexp = cregexp

def visit(arg, dirname, names):
    """ This function is called by os.path.walk to prepend a file to matched files """
    global regexp

    for f in names:
        if regexp.match(f):
            filename = os.path.join(dirname, f)
            tmpname = os.tmpnam()
            ret = os.system("cat %s %s > %s" % (arg, filename, tmpname))
            if ret == 0:
                ret = os.system("mv %s %s" % (tmpname, filename))
                if ret != 0:
                    print "%s: mv failed on %s, %s" % (sys.argv[0], tmpname, filename)
            else:
                print "%s: cat failed on %s %s > %s" % (sys.argv[0], arg, filename, tmpname)
            
            
def usage():
    print "usage: %s [-m] prepend_file file_paths" % sys.argv[0]
    print "The file, prepend_file, will be prepended to every file in file_paths that ends in .c, .h, .cc, .hh."
    print "If -m is specified, the file, prepend_file, will be prepended to every Makefile."
    sys.exit(2)
    
def main():
    global regexp
    
    optlist = []
    args = []

    try:
	optlist, args = getopt.getopt(sys.argv[1:], "m")
    except:
	print "%s: error in parsing options, %s" % (sys.argv[0], sys.exc_value)
	usage()
	
    for i in xrange(0,len(optlist)):
        if optlist[i][0] == "-m":
            regexp = mregexp

    if len(args) == 0:
        usage()
        sys.exit(2)
        
    insert = args[0]

    # For each path specified on the command line, do a walk
    for fpath in args[1:]:
        os.path.walk(fpath, visit, insert)

main()

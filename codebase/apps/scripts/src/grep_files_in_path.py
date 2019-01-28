#!/usr/bin/env python2

import os
import sys
import getopt
import re

def print_usage():
    print
    print "Recursively Searches files under the directory "
    print "and prints the lines that contain the specified string."
    print
    print "Usage: ", prog_name, " [options]"
    print "Options:"
    print "   -h | --help     : Print usage and exit"
    print "   --dir           : directory to start from"
    print "   --str           : string to search for"
    print "   --single        : only search in this file name (defaults to all files)\n"

def file_grep(str, filename):
    searchterm = str
    file = filename

    try:
        fileToSearch = open( file, 'r' )
    except IOError:
        print
        print "No such file", file
        return

    # Well, we've got this far - the file must exist!
    data = fileToSearch.read()
    if '\0'in data:
        print
        print "File ", file, " is binary, skipping"
        return
    data = data.split('\n')
        
    patternprog = re.compile( searchterm )

    cnt = 0
    for line in data:
        a_match = patternprog.search( line )
        if ( a_match ):
            cnt = cnt + 1
            if cnt is 1:
                print "\nFILE: %s" % ( file )
            print line

    fileToSearch.close()
    

prog_name = os.path.basename(sys.argv[0])
#
# Get the command line arguments.
#

optlist, args = getopt.getopt(sys.argv[1:], 'dh', \
                              [ 'help', \
                                'dir=',
                                'str=',
                                'single=',])
# set command line defaults
# use "" if defaults are not perfered
search_string = ""
search_file = ""
directory = "/suri/dt1/cvs/projects/dallas"

for opt in optlist:
    if opt[0] ==  "-h" or opt[0] == "--help":
        print_usage()
        sys.exit()
    if opt[0] == "--dir":
        directory = opt[1]
    if opt[0] == "--str":
        search_string = opt[1]
    if opt[0] == "--single":
        search_file = opt[1]

if search_string is "" or directory is "":
    print_usage()
    sys.exit()
        
from os.path import join, getsize
for root, dirs, files in os.walk(directory):
    for name in files:
        if name == search_file or search_file == "":
            file_grep(search_string, join(root,name) )

#    if 'CVS' in dirs:
#        dirs.remove('CVS')  # don't visit CVS directories
#    if 'ciddHome' in dirs:
#        dirs.remove('ciddHome')  # don't visit ciddHome directories
#    if 'sysviewHome' in dirs:
#        dirs.remove('sysviewHome')  # don't visit ciddHome directories
#    if 'ref_grids' in dirs:
#        dirs.remove('ref_grids')  # don't visit ciddHome directories




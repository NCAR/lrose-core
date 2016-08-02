#!/usr/bin/env python

#===========================================================================
#
# Fixing configure for bugs
#
#===========================================================================

import os
import sys
import shutil
from optparse import OptionParser
from datetime import datetime

def main():

    global options
    global subdirList
    global makefileCreateList

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default='False',
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--verbose',
                      dest='verbose', default='False',
                      action="store_true",
                      help='Set verbose debugging on')
    parser.add_option('--dir',
                      dest='dir', default=".",
                      help='Directory containing configure script')

    (options, args) = parser.parse_args()

    if (options.verbose == True):
        options.debug = True
    
    if (options.debug == True):
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  Dir:", options.dir

    # go to the dir

    os.chdir(options.dir)

    # fix the configure

    ret = fix()
            
    sys.exit(ret)

########################################################################
# fix the configure script

def fix():

    # copy the configure script to a backup file

    shutil.copy("configure", "configure.bak")

    # read in lines from bak file

    try:
        bak = open('configure.bak', 'r')
    except IOError as e:
        if (options.verbose == True):
            print >>sys.stderr, "ERROR - ", thisScriptName
            print >>sys.stderr, "  Cannot open file configure.bak for reading"
            print >>sys.stderr, "  dir: ", options.dir
        return(-1)

    lines = bak.readlines()
    bak.close()

    # write lines to configure, commenting out bad lines

    try:
        conf = open('configure', 'w')
    except IOError as e:
        if (options.verbose == True):
            print >>sys.stderr, "ERROR - ", thisScriptName
            print >>sys.stderr, "  Cannot open file configure for writing"
            print >>sys.stderr, "  dir: ", options.dir
        return(-1)

    inBad = False
    for line in lines:

        if (line.find("#") == 0):
            conf.write(line)
            continue
        
        if (line.find("_LT_DECL") >= 0):
            inBad = True
        else:
            cleanLine = line.strip()
            if (len(cleanLine) < 1):
                inBad = False
        if (inBad == True):
            commentLine = "#" + line
            conf.write(commentLine)
        else:
            conf.write(line)

    conf.close()
    return(0)

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

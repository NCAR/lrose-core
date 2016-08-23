#!/usr/bin/env python

#===========================================================================
#
# Check that all libs are installed
#
#===========================================================================

import os
import time
import sys
import shutil
from optparse import OptionParser
from datetime import datetime

def main():

    global options
    global requiredLibs
    global requiredLibsLine
    global missingLibs
    global oldLibs

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    # parse the command line

    usage = "usage: %prog [options]"
    # homeDir = os.environ['HOME']
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--libDir',
                      dest='libDir', default='.',
                      help='Dir with installed libs')
    parser.add_option('--listPath',
                      dest='listPath', default='.',
                      help='Path with requried list')
    parser.add_option('--label',
                      dest='label', default="LROSE",
                      help='Label for messages')
    parser.add_option('--maxAge',
                      dest='maxAge', default=900,
                      help='Max file age in secs')

    (options, args) = parser.parse_args()
    
    if (options.debug == True):
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  libDir:", options.libDir
        print >>sys.stderr, "  listPath:", options.listPath
        print >>sys.stderr, "  label:", options.label
        print >>sys.stderr, "  maxAge:", options.maxAge

    # read in required list

    if (readRequiredList(options.listPath) != 0):
        sys.exit(255)
        
    # check required files exist

    checkForLibs()

    if (len(missingLibs) > 0):
        print >>sys.stderr, "==================>> ERROR <<===================="
        print >>sys.stderr, "=====>> INCOMPLETE " + options.label + " LIBS INSTALLATION <<===="
        print >>sys.stderr, "  n libraries missing: " + str(len(missingLibs))
        for index, lib in enumerate(missingLibs):
            print >>sys.stderr, "    missing lib: " + requiredLibsLine[lib]
    else:
        print >>sys.stderr, "=================>> SUCCESS <<==================="
        print >>sys.stderr, "=========>> ALL " + options.label + " LIBS INSTALLED <<========"

    if (len(oldLibs) > 0):
        print >>sys.stderr, "=================>> WARNING <<==================="
        print >>sys.stderr, "===========>> SOME " + options.label + " LIBS ARE OLD <<========="
        print >>sys.stderr, "  n old libs: " + str(len(oldLibs))
        print >>sys.stderr, "  These libs may not have been built"
        for lib in oldLibs:
            print >>sys.stderr, "    lib out-of-date: " + lib

    print >>sys.stderr, "================================================="

    sys.exit(0)

########################################################################
# read in required list

def readRequiredList(path):

    global requiredLibs
    global requiredLibsLine
    requiredLibs = []
    requiredLibsLine = {}
    
    try:
        fp = open(path, 'r')
    except IOError as e:
        print >>sys.stderr, "ERROR - ", thisScriptName
        print >>sys.stderr, "  Cannot open lib list file:", path
        return -1

    lines = fp.readlines()
    fp.close()
    
    for line in lines:
        line = line.strip()
        if (len(line) < 1):
            continue
        if (line[0] == '#'):
            continue
        toks = line.split(" ")
        if (len(toks) > 0):
            libName = toks[0]
            requiredLibs.append(libName)
            requiredLibsLine[libName] = line

    if (options.debug == True):
        print >>sys.stderr, "Required libs:"
        for name in requiredLibs:
            print >>sys.stderr, "    ", requiredLibsLine[name]

    return 0

########################################################################
# check that the lib list is installed

def checkForLibs():

    global missingLibs
    global oldLibs
    missingLibs = []
    oldLibs = []
    
    for name in requiredLibs:

        path = os.path.join(options.libDir, name)

        if (options.debug == True):
            print >>sys.stderr, "Checking for installed lib: ", path

        if (os.path.isfile(path) == False):
            if (options.debug == True):
                print >>sys.stderr, "   .... missing"
            missingLibs.append(name)
        else:
            if (options.debug == True):
                print >>sys.stderr, "   .... found"
            age = getFileAge(path)
            if (age > float(options.maxAge)):
                oldLibs.append(name)
                if (options.debug == True):
                    print >>sys.stderr, "   file is old, age: ", age


########################################################################
# get file age

def getFileAge(path):

    stats = os.stat(path)
    age = (time.time() - stats.st_mtime)
    return age

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

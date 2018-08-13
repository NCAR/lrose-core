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
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  libDir:", options.libDir, file=sys.stderr)
        print("  listPath:", options.listPath, file=sys.stderr)
        print("  label:", options.label, file=sys.stderr)
        print("  maxAge:", options.maxAge, file=sys.stderr)

    # read in required list

    if (readRequiredList(options.listPath) != 0):
        sys.exit(255)
        
    # check required files exist

    checkForLibs()

    if (len(missingLibs) > 0):
        print("==================>> ERROR <<====================", file=sys.stderr)
        print("=====>> INCOMPLETE " + options.label + " LIBS INSTALLATION <<====", file=sys.stderr)
        print("  n libraries missing: " + str(len(missingLibs)), file=sys.stderr)
        for index, lib in enumerate(missingLibs):
            print("    missing lib: " + requiredLibsLine[lib], file=sys.stderr)
    else:
        print("=================>> SUCCESS <<===================", file=sys.stderr)
        print("=========>> ALL " + options.label + " LIBS INSTALLED <<========", file=sys.stderr)

    if (len(oldLibs) > 0):
        print("=================>> WARNING <<===================", file=sys.stderr)
        print("===========>> SOME " + options.label + " LIBS ARE OLD <<=========", file=sys.stderr)
        print("  n old libs: " + str(len(oldLibs)), file=sys.stderr)
        print("  These libs may not have been built", file=sys.stderr)
        for lib in oldLibs:
            print("    lib out-of-date: " + lib, file=sys.stderr)

    print("=================================================", file=sys.stderr)

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
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot open lib list file:", path, file=sys.stderr)
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
        print("Required libs:", file=sys.stderr)
        for name in requiredLibs:
            print("    ", requiredLibsLine[name], file=sys.stderr)

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
            print("Checking for installed lib: ", path, file=sys.stderr)

        if (os.path.isfile(path) == False):
            if (options.debug == True):
                print("   .... missing", file=sys.stderr)
            missingLibs.append(name)
        else:
            if (options.debug == True):
                print("   .... found", file=sys.stderr)
            age = getFileAge(path)
            if (age > float(options.maxAge)):
                oldLibs.append(name)
                if (options.debug == True):
                    print("   file is old, age: ", age, file=sys.stderr)


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

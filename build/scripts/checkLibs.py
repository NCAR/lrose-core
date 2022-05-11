#!/usr/bin/env python3

#===========================================================================
#
# Check that all libs are installed
# Checks against a list of expected apps
#
#===========================================================================

from __future__ import print_function
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

    global thisScriptDir
    thisScriptDir = os.path.dirname(__file__)
 
    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    prefixDirDefault = os.path.join(homeDir, 'lrose')
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--package',
                      dest='package', default='lrose-core',
                      help='Package name. Options are: ' + \
                      'lrose-core (default), lrose-blaze, lrose-cyclone, lrose-radx, lrose-cidd, samurai')
    parser.add_option('--prefix',
                      dest='prefix', default=prefixDirDefault,
                      help='Install directory, default is ~/lrose')
    parser.add_option('--maxAge',
                      dest='maxAge', default=-1,
                      help='Max file age in secs. Check not done if negative (default).')

    (options, args) = parser.parse_args()

    # compute dirs and paths

    global buildDir, checkListPath

    buildDir = os.path.join(thisScriptDir, "..")
    os.chdir(buildDir)
    buildDir = os.getcwd()

    checklistDir = os.path.join(buildDir, "checklists")
    checkListPath = os.path.join(checklistDir, "libs_check_list." + options.package)

    # print status

    if (options.debug == True):
        print("Running %s: " % thisScriptName, file=sys.stderr)
        print("  package: ", options.package, file=sys.stderr)
        print("  prefix: ", options.prefix, file=sys.stderr)
        print("  maxAge: ", options.maxAge, file=sys.stderr)
        print("  buildDir: ", buildDir, file=sys.stderr)
        print("  checkListPath: ", checkListPath, file=sys.stderr)

    # read in required list

    if (readRequiredList(checkListPath) != 0):
        sys.exit(255)
        
    # check required files exist

    checkForLibs()

    if (len(missingLibs) > 0):
        print("==================>> ERROR <<====================", file=sys.stderr)
        print("=====>> INCOMPLETE " + options.package + " LIBS INSTALLATION <<====", file=sys.stderr)
        print("  n libraries missing: " + str(len(missingLibs)), file=sys.stderr)
        for index, lib in enumerate(missingLibs):
            print("    missing lib: " + requiredLibsLine[lib], file=sys.stderr)
    else:
        print("=================>> SUCCESS <<===================", file=sys.stderr)
        print("=========>> ALL " + options.package + " LIBS INSTALLED <<========", file=sys.stderr)

    if (len(oldLibs) > 0):
        print("=================>> WARNING <<===================", file=sys.stderr)
        print("===========>> SOME " + options.package + " LIBS ARE OLD <<=========", file=sys.stderr)
        print("  n old libs: " + str(len(oldLibs)), file=sys.stderr)
        if (options.debug):
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

    installLibDir = os.path.join(options.prefix, "lib")
    maxAge = float(options.maxAge)
    
    for name in requiredLibs:

        path = os.path.join(installLibDir, name)

        if (options.debug == True):
            print("Checking for installed lib: ", path, file=sys.stderr)

        # check .a file
        found = True
        if (os.path.isfile(path) == False):
            # check .so file
            path = os.path.splitext(path)[0] + ".so"
            if (os.path.isfile(path) == False):
                if (options.debug == True):
                    print("   .... missing", file=sys.stderr)
                missingLibs.append(name)
                found = False

        if (found):
            if (options.debug == True):
                print("   .... found", file=sys.stderr)
            age = getFileAge(path)
            if (maxAge > 0 and age > maxAge):
                oldLibs.append(name)
                if (options.debug == True):
                    print("   file is old, age: ", age, file=sys.stderr)


########################################################################
# get file age

def getFileAge(path):

    stats = os.stat(path)
    age = float(time.time() - stats.st_mtime)
    return age

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

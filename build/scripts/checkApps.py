#!/usr/bin/env python

#===========================================================================
#
# Check that all apps are installed
# Checks against a list of expected apps.
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
    global requiredApps
    global missingApps
    global oldApps

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

    global buildDir, checklistPath

    buildDir = os.path.join(thisScriptDir, "..")
    os.chdir(buildDir)
    buildDir = os.getcwd()

    checkListDir = os.path.join(buildDir, "checklists")
    checkListPath = os.path.join(checkListDir, "apps_check_list." + options.package)

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

    checkForApps()

    if (len(missingApps) > 0):
        print("==================>> ERROR <<====================", file=sys.stderr)
        print("=====>> INCOMPLETE " + options.package + " APPS INSTALLATION <<====", file=sys.stderr)
        print("  n applications missing: " + str(len(missingApps)), file=sys.stderr)
        for app in missingApps:
            print("    missing app: " + app, file=sys.stderr)
        print("=================================================", file=sys.stderr)
    else:
        print("================>> SUCCESS <<==================", file=sys.stderr)
        print("=========>> ALL " + options.package + " APPS INSTALLED <<========", file=sys.stderr)
        print("===============================================", file=sys.stderr)

    if (len(oldApps) > 0):
        print("==================>> WARNING <<====================", file=sys.stderr)
        print("=====>> SOME " + options.package + " APPS ARE OLD <<====", file=sys.stderr)
        print("  n old apps: " + str(len(oldApps)), file=sys.stderr)
        if (options.debug):
            for app in oldApps:
                print("    old app: " + app, file=sys.stderr)
        print("=================================================", file=sys.stderr)

    sys.exit(0)

########################################################################
# read in required list

def readRequiredList(path):

    global requiredApps
    requiredApps = []

    try:
        fp = open(path, 'r')
    except IOError as e:
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot open app list file:", path, file=sys.stderr)
        return -1

    lines = fp.readlines()
    fp.close()
    
    for line in lines:
        line = line.strip()
        if (len(line) < 1):
            continue
        if (line[0] == '#'):
            continue
        requiredApps.append(line)

    if (options.debug == True):
        print("Required apps:", file=sys.stderr)
        for name in requiredApps:
            print("    ", name, file=sys.stderr)

    return 0

########################################################################
# check that the app list is installed

def checkForApps():

    global missingApps
    global oldApps
    missingApps = []
    oldApps = []
    
    installBinDir = os.path.join(options.prefix, "bin")
    maxAge = float(options.maxAge)

    for name in requiredApps:

        path = os.path.join(installBinDir, name)

        if (options.debug == True):
            print("Checking for installed app: ", path, file=sys.stderr)

        if (os.path.isfile(path) == False):
            if (options.debug == True):
                print("   .... missing", file=sys.stderr)
            missingApps.append(path)
        else:
            if (options.debug == True):
                print("   .... found", file=sys.stderr)
            age = getFileAge(path)
            if ((maxAge > 0) and (age > maxAge)):
                oldApps.append(path)
                if (options.debug == True):
                    print("   file is old, age: ", age, file=sys.stderr)
                    print("   maxAge: ", maxAge, file=sys.stderr)


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

#!/usr/bin/env python

#===========================================================================
#
# Check that all apps are installed
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

    # parse the command line

    usage = "usage: %prog [options]"
    # homeDir = os.environ['HOME']
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--appDir',
                      dest='appDir', default='.',
                      help='Dir with installed apps')
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
        print("  appDir:", options.appDir, file=sys.stderr)
        print("  listPath:", options.listPath, file=sys.stderr)
        print("  label:", options.label, file=sys.stderr)
        print("  maxAge:", options.maxAge, file=sys.stderr)

    # read in required list

    if (readRequiredList(options.listPath) != 0):
        sys.exit(255)
        
    # check required files exist

    checkForApps()

    if (len(missingApps) > 0):
        print("==================>> ERROR <<====================", file=sys.stderr)
        print("=====>> INCOMPLETE " + options.label + " APPS INSTALLATION <<====", file=sys.stderr)
        print("  n applications missing: " + str(len(missingApps)), file=sys.stderr)
        for app in missingApps:
            print("    missing app: " + app, file=sys.stderr)
        print("=================================================", file=sys.stderr)
    else:
        print("================>> SUCCESS <<==================", file=sys.stderr)
        print("=========>> ALL " + options.label + " APPS INSTALLED <<========", file=sys.stderr)
        print("===============================================", file=sys.stderr)

    if (len(oldApps) > 0):
        print("==================>> WARNING <<====================", file=sys.stderr)
        print("=====>> SOME " + options.label + " APPS ARE OLD <<====", file=sys.stderr)
        print("  n old apps: " + str(len(oldApps)), file=sys.stderr)
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
    
    for name in requiredApps:

        path = os.path.join(options.appDir, name)

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
            if (age > float(options.maxAge)):
                oldApps.append(path)
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

#!/usr/bin/env python

#===========================================================================
#
# Check that all apps are installed
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
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  appDir:", options.appDir
        print >>sys.stderr, "  listPath:", options.listPath
        print >>sys.stderr, "  label:", options.label
        print >>sys.stderr, "  maxAge:", options.maxAge

    # read in required list

    if (readRequiredList(options.listPath) != 0):
        sys.exit(255)
        
    # check required files exist

    checkForApps()

    if (len(missingApps) > 0):
        print >>sys.stderr, "==================>> ERROR <<===================="
        print >>sys.stderr, "=====>> INCOMPLETE " + options.label + " APPS INSTALLATION <<===="
        print >>sys.stderr, "  n applications missing: " + str(len(missingApps))
        for app in missingApps:
            print >>sys.stderr, "    missing app: " + app
        print >>sys.stderr, "================================================="
    else:
        print >>sys.stderr, "================>> SUCCESS <<=================="
        print >>sys.stderr, "=========>> ALL " + options.label + " APPS INSTALLED <<========"
        print >>sys.stderr, "==============================================="

    if (len(oldApps) > 0):
        print >>sys.stderr, "==================>> WARNING <<===================="
        print >>sys.stderr, "=====>> SOME " + options.label + " APPS ARE OLD <<===="
        print >>sys.stderr, "  n old apps: " + str(len(oldApps))
        for app in oldApps:
            print >>sys.stderr, "    old app: " + app
        print >>sys.stderr, "================================================="

    sys.exit(0)

########################################################################
# read in required list

def readRequiredList(path):

    global requiredApps
    requiredApps = []

    try:
        fp = open(path, 'r')
    except IOError as e:
        print >>sys.stderr, "ERROR - ", thisScriptName
        print >>sys.stderr, "  Cannot open app list file:", path
        return -1

    lines = fp.readlines()
    fp.close()
    
    for line in lines:
        line = line.strip()
        if (line[0] == '#'):
            continue
        requiredApps.append(line)

    if (options.debug == True):
        print >>sys.stderr, "Required apps:"
        for name in requiredApps:
            print >>sys.stderr, "    ", name

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
            print >>sys.stderr, "Checking for installed app: ", path

        if (os.path.isfile(path) == False):
            if (options.debug == True):
                print >>sys.stderr, "   .... missing"
            missingApps.append(path)
        else:
            if (options.debug == True):
                print >>sys.stderr, "   .... found"
            age = getFileAge(path)
            if (age > float(options.maxAge)):
                oldApps.append(path)
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

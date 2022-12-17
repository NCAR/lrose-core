#!/usr/bin/env python3

#===========================================================================
#
# Install a binary release
# Should be run from within the untarred directory
#
#===========================================================================

from __future__ import print_function
import os
import sys
import shutil
import subprocess
from optparse import OptionParser
import time
from datetime import datetime
from datetime import date
from datetime import timedelta
import glob
from sys import platform

def main():

    # globals

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global options
    global package
    global version
    global srcRelease

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    prefixDefault = '/usr/local/lrose'

    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--verbose',
                      dest='verbose', default=False,
                      action="store_true",
                      help='Set verbose debugging on')
    parser.add_option('--prefix',
                      dest='prefix', default=prefixDefault,
                      help='Directory for installation')

    (options, args) = parser.parse_args()

    if (options.verbose):
        options.debug = True
        
    installDir = options.prefix

    # runtime

    now = time.gmtime()
    nowTime = datetime(now.tm_year, now.tm_mon, now.tm_mday,
                       now.tm_hour, now.tm_min, now.tm_sec)
    dateStr = nowTime.strftime("%Y%m%d")
    timeStr = nowTime.strftime("%Y%m%d%H%M%S")
    dateTimeStr = nowTime.strftime("%Y/%m/%d-%H:%M:%S")
    runDir = os.getcwd()

    # read in release info

    readReleaseInfoFile()

    # let users know what we are doing

    print("****************************************************", file=sys.stderr)
    print("  Running " + thisScriptName, file=sys.stderr)
    print("", file=sys.stderr)
    print("  Installing " + package + " binary release", file=sys.stderr)
    print("", file=sys.stderr)
    print("  NCAR, Boulder, CO, USA", file=sys.stderr)
    print("", file=sys.stderr)
    print("  " + dateTimeStr, file=sys.stderr)
    print("", file=sys.stderr)
    print("****************************************************", file=sys.stderr)
    print("  dateStr: ", dateStr, file=sys.stderr)
    print("  timeStr: ", timeStr, file=sys.stderr)
    print("  runDir: ", runDir, file=sys.stderr)
    print("  installDir: ", installDir, file=sys.stderr)
    print("  platform: ", platform, file=sys.stderr)
    print("  package: ", package, file=sys.stderr)
    print("  version: ", version, file=sys.stderr)
    print("  srcRelease: ", srcRelease, file=sys.stderr)
    print("****************************************************", file=sys.stderr)

    # create the install dir

    if (os.path.isdir(installDir) == False):
        os.makedirs(installDir)

    # do the install
    
    if (os.path.isdir("bin")):
        shellCmd("cp -r -p bin " + installDir)

    if (os.path.isdir("lib")):
        shellCmd("cp -r -p lib " + installDir)

    if (os.path.isdir("include")):
        shellCmd("cp -r -p include " + installDir)

    if (os.path.isdir("share")):
        shellCmd("cp -r -p share " + installDir)

    #--------------------------------------------------------------------
    # done
    
    print("  **************************************************")
    print("  *** Done installing binary release ***")
    print(("  *** installed in dir: " + installDir + " ***"))
    print("  **************************************************")

    sys.exit(0)

########################################################################
# read release information file

def readReleaseInfoFile():

    global package
    global version
    global srcRelease

    package = "unknown"
    version = "unknown"
    srcRelease = "unknown"

    # open info file
    
    releaseInfoPath = "ReleaseInfo.txt"
    if (options.verbose):
        print("==>> reading info file: ", releaseInfoPath, file=sys.stderr)
        
    info = open(releaseInfoPath, 'r')

    # read in lines

    lines = info.readlines()
    
    # close

    info.close()

    # decode lines

    if (len(lines) < 1):
        print("ERROR reading info file: ", releaseInfoPath, file=sys.stderr)
        print("  No contents", file=sys.stderr)
        sys.exit(1)

    for line in lines:
        line = line.strip()
        toks = line.split(":")
        if (options.verbose):
            print("  line: ", line, file=sys.stderr)
            print("  toks: ", toks, file=sys.stderr)
        if (len(toks) == 2):
            if (toks[0] == "package"):
                package = toks[1]
            if (toks[0] == "version"):
                version = toks[1]
            if (toks[0] == "release"):
                srcRelease = toks[1]
        
    if (options.verbose):
        print("==>> done reading info file: ", releaseInfoPath, file=sys.stderr)

########################################################################
# Run a command in a shell, wait for it to complete

def shellCmd(cmd):

    if (options.debug):
        print("running cmd:", cmd, " .....", file=sys.stderr)
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print("Child exited with code: ", retcode, file=sys.stderr)
            sys.exit(1)
        else:
            if (options.verbose):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, file=sys.stderr)
        sys.exit(1)

    if (options.debug):
        print(".... done", file=sys.stderr)
    
########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

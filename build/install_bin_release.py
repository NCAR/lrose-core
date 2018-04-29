#!/usr/bin/env python

#===========================================================================
#
# Install a binary release
#
#===========================================================================

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
    global runDir
    global dateStr
    global timeStr
    global debugStr
    global releaseName

    global package
    global version
    global srcRelease

    global ostype

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
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
                      dest='prefix', default='not-set',
                      help='Directory for installation')
    parser.add_option('--devel',
                      dest='devel', default=False,
                      action="store_true",
                      help='Install development libs and includes as well as binaries')

    (options, args) = parser.parse_args()

    if (options.verbose):
        options.debug = True
        
    debugStr = " "
    if (options.verbose):
        debugStr = " --verbose "
    elif (options.debug):
        debugStr = " --debug "

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

    # get the OS type - x86_64, i686, macosx_64
    
    getOsType()

    # let users know what we are doing

    print >>sys.stderr, "*********************************************************************"
    print >>sys.stderr, "  Running " + thisScriptName
    print >>sys.stderr, ""
    print >>sys.stderr, "  Installing " + package + " binary release"
    print >>sys.stderr, "  OS type: " + ostype
    print >>sys.stderr, ""
    print >>sys.stderr, "  NCAR, Boulder, CO, USA"
    print >>sys.stderr, ""
    print >>sys.stderr, "  " + dateTimeStr
    print >>sys.stderr, ""
    print >>sys.stderr, "*********************************************************************"
    print >>sys.stderr, "  dateStr: ", dateStr
    print >>sys.stderr, "  timeStr: ", timeStr
    print >>sys.stderr, "  platform: ", platform
    print >>sys.stderr, "  installDir: ", options.prefix
    print >>sys.stderr, "  package: ", package
    print >>sys.stderr, "  version: ", version
    print >>sys.stderr, "  srcRelease: ", srcRelease
    print >>sys.stderr, "  installDevel: ", options.devel
    print >>sys.stderr, "*********************************************************************"

    # create the install dir

    os.makedirs(options.prefix)

    # do the install
    
    shellCmd("rsync -av bin " + options.prefix)
    if (os.path.isdir("share")):
        shellCmd("rsync -av share " + options.prefix)
    if (options.devel):
        shellCmd("rsync -av lib " + options.prefix)
        shellCmd("rsync -av include " + options.prefix)

    #--------------------------------------------------------------------
    # done
    
    print("  **************************************************")
    print("  *** Done installing binary release ***")
    print("  *** installed in run dir: " + runDir + " ***")
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
        print >>sys.stderr, "==>> reading info file: ", releaseInfoPath
        
    info = open(releaseInfoPath, 'r')

    # read in lines

    lines = info.readlines()
    
    # close

    info.close()

    # decode lines

    if (len(lines) < 1):
        print >>sys.stderr, "ERROR reading info file: ", releaseInfoPath
        print >>sys.stderr, "  No contents"
        sys.exit(1)

    for line in lines:
        line = line.strip()
        toks = line.split(":")
        if (options.verbose):
            print >>sys.stderr, "  line: ", line
            print >>sys.stderr, "  toks: ", toks
        if (len(toks) == 2):
            if (toks[0] == "package"):
                package = toks[1]
            if (toks[0] == "version"):
                version = toks[1]
            if (toks[0] == "release"):
                srcRelease = toks[1]
        
    if (options.verbose):
        print >>sys.stderr, "==>> done reading info file: ", releaseInfoPath

########################################################################
# get the OS type

def getOsType():

    global ostype

    if (platform == "darwin"):
        ostype = "mac_osx"
        return

    ostype = "x86_64"
    tmpFile = os.path.join("/tmp", "ostype." + timeStr + ".txt")

    shellCmd("uname -a > " + tmpFile)
    f = open(tmpFile, 'r')
    lines = f.readlines()
    f.close()

    if (len(lines) < 1):
        print >>sys.stderr, "ERROR getting OS type"
        print >>sys.stderr, "  'uname -a' call did not succeed"
        sys.exit(1)

    for line in lines:
        line = line.strip()
        if (options.verbose):
            print >>sys.stderr, "  line: ", line
        if (line.find("x86_64") > 0):
            ostype = "x86_64"
        elif (line.find("i686") > 0):
            ostype = "i686"
            
########################################################################
# Run a command in a shell, wait for it to complete

def shellCmd(cmd):

    if (options.debug):
        print >>sys.stderr, "running cmd:", cmd, " ....."
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print >>sys.stderr, "Child exited with code: ", retcode
            sys.exit(1)
        else:
            if (options.verbose):
                print >>sys.stderr, "Child returned code: ", retcode
    except OSError, e:
        print >>sys.stderr, "Execution failed:", e
        sys.exit(1)

    if (options.debug):
        print >>sys.stderr, ".... done"
    
########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

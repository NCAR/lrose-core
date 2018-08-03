#!/usr/bin/env python

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
    global runDir
    global installDir
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
        
    debugStr = " "
    if (options.verbose):
        debugStr = " --verbose "
    elif (options.debug):
        debugStr = " --debug "

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

    # get the OS type - x86_64, i686, macosx_64
    
    getOsType()

    # let users know what we are doing

    print("****************************************************\n"
          "  Running %s\n"
          "  Installing %s binary release\n"
          "  OS type: %s\n"
          ""
          "  NCAR, Boulder, CO, USA\n"
          ""
          "  %s\n"
          ""
          "****************************************************\n"
          "  dateStr: %s\n"
          "  runDir: %s\n"
          "  installDir: %s\n"
          "  platform: %s\n"
          "  package: %s\n"
          "  version: %s\n"
          "  srcRelease: %s\n"
          "****************************************************"
          %(thisScriptName, package, ostype, dateTimeStr, dateStr,
            runDir, installDir, platform, package, version, srcRelease),
          file=sys.stderr)

    # create the install dir

    if (os.path.isdir(installDir) == False):
        os.makedirs(installDir)

    # do the install
    
    if (os.path.isdir("bin")):
        shellCmd("rsync -av bin " + installDir)

    if (os.path.isdir("lib")):
        shellCmd("rsync -av lib " + installDir)

    if (os.path.isdir("include")):
        shellCmd("rsync -av include " + installDir)

    if (os.path.isdir("share")):
        shellCmd("rsync -av share " + installDir)

    #--------------------------------------------------------------------
    # done
    
    print("  **************************************************")
    print("  *** Done installing binary release ***")
    print("  *** installed in dir: " + installDir + " ***")
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
 #       print(("==>> reading info file: %s"% releaseInfoPath), file=sys.stderr)
        print("==>> reading info file: " + releaseInfoPath, file=sys.stderr)
        
    info = open(releaseInfoPath, 'r')

    # read in lines

    lines = info.readlines()
    
    # close

    info.close()

    # decode lines

    if (len(lines) < 1):
        print(("ERROR reading info file: %s"% releaseInfoPath), file=sys.stderr)
        print("  No contents", file=sys.stderr)
        sys.exit(1)

    for line in lines:
        line = line.strip()
        toks = line.split(":")
        if (options.verbose):
            print(("  line: %s"% line), file=sys.stderr)
            print(("  toks: %s"% toks), file=sys.stderr)
        if (len(toks) == 2):
            if (toks[0] == "package"):
                package = toks[1]
            if (toks[0] == "version"):
                version = toks[1]
            if (toks[0] == "release"):
                srcRelease = toks[1]
        
    if (options.verbose):
        print("==>> done reading info file: %s"% releaseInfoPath, file=sys.stderr)

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
        print("ERROR getting OS type"
              "  'uname -a' call did not succeed", file=sys.stderr)
        sys.exit(1)

    for line in lines:
        line = line.strip()
        if (options.verbose):
            print("  line: %s"% line, file=sys.stderr)
        if (line.find("x86_64") > 0):
            ostype = "x86_64"
        elif (line.find("i686") > 0):
            ostype = "i686"
            
########################################################################
# Run a command in a shell, wait for it to complete

def shellCmd(cmd):

    if (options.debug):
        print("running cmd: %s ....."% cmd, file=sys.stderr)
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print("Child exited with code: %s"% retcode, file=sys.stderr)
            sys.exit(1)
        else:
            if (options.verbose):
                print("Child returned code: %s"% retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed: ", e, file=sys.stderr)
        sys.exit(1)

    if (options.debug):
        print(".... done", file=sys.stderr)
    
########################################################################
# Run - entry point

if __name__ == "__main__":
   main()
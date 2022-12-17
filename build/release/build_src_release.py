#!/usr/bin/env python3

#===========================================================================
#
# Build and install binaries from a src release
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
    prefixDefault = os.path.join(homeDir, 'lrose')

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
    parser.add_option('--automake',
                      dest='automake', default=False,
                      action="store_true",
                      help='Use automake for the build. Default is cmake')
    parser.add_option('--scripts',
                      dest='installScripts', default=False,
                      action="store_true",
                      help='Install scripts as well as binaries')

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

    print("****************************************************", file=sys.stderr)
    print("  Running " + thisScriptName, file=sys.stderr)
    print("", file=sys.stderr)
    print("  Building and installing " + package + " release", file=sys.stderr)
    print("  OS type: " + ostype, file=sys.stderr)
    print("", file=sys.stderr)
    print("  NCAR, Boulder, CO, USA", file=sys.stderr)
    print("", file=sys.stderr)
    print("  " + dateTimeStr, file=sys.stderr)
    print("", file=sys.stderr)
    print("****************************************************", file=sys.stderr)
    print("  dateStr: ", dateStr, file=sys.stderr)
    print("  timeStr: ", timeStr, file=sys.stderr)
    print("  platform: ", platform, file=sys.stderr)
    print("  runDir: ", runDir, file=sys.stderr)
    print("  installDir: ", installDir, file=sys.stderr)
    print("  package: ", package, file=sys.stderr)
    print("  version: ", version, file=sys.stderr)
    print("  srcRelease: ", srcRelease, file=sys.stderr)
    print("  installScripts: ", options.installScripts, file=sys.stderr)
    print("****************************************************", file=sys.stderr)

    # create the install dir

    if (os.path.isdir(installDir) == False):
        os.makedirs(installDir)

    # build netcdf support

    if (package == "lrose-cidd"):
        buildNetcdf()

    # build and install the package

    if (options.automake):
        buildPackageAutomake()
    else:
        buildPackageCmake()

    #--------------------------------------------------------------------
    # done
    
    print("  **************************************************")
    print("  *** Done building and installing src release ***")
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
        print("ERROR getting OS type", file=sys.stderr)
        print("  'uname -a' call did not succeed", file=sys.stderr)
        sys.exit(1)

    for line in lines:
        line = line.strip()
        if (options.verbose):
            print("  line: ", line, file=sys.stderr)
        if (line.find("x86_64") > 0):
            ostype = "x86_64"
        elif (line.find("i686") > 0):
            ostype = "i686"
            
########################################################################
# build netCDF

def buildNetcdf():

    netcdfDir = os.path.join(runDir, "lrose-netcdf")
    os.chdir(netcdfDir)
    if (package == "lrose-cidd"):
        shellCmd("./build_and_install_netcdf.cidd_linux32 -x " + installDir)
    else:
        if (platform == "darwin"):
            shellCmd("./build_and_install_netcdf.osx -x " + installDir)
        else:
            shellCmd("./build_and_install_netcdf -x " + installDir)

########################################################################
# build package using automake

def buildPackageAutomake():

    os.chdir(runDir)

    args = ""
    args = args + " --prefix " + installDir
    args = args + " --package " + package
    if (options.installScripts):
        args = args + " --scripts "

    shellCmd("./build/scripts/build_lrose_automake.py " + args)

########################################################################
# build package using cmake

def buildPackageCmake():

    os.chdir(runDir)

    args = ""
    args = args + " --prefix " + installDir
    args = args + " --package " + package
    if (options.installScripts):
        args = args + " --scripts "

    shellCmd("./build/scripts/build_lrose_cmake.py " + args)

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

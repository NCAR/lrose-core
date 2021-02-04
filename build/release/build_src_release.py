#!/usr/bin/env python

#===========================================================================
#
# Build and install binaries from a src release
# Should be run from within the untarred directory
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

    print >>sys.stderr, "****************************************************"
    print >>sys.stderr, "  Running " + thisScriptName
    print >>sys.stderr, ""
    print >>sys.stderr, "  Building and installing " + package + " release"
    print >>sys.stderr, "  OS type: " + ostype
    print >>sys.stderr, ""
    print >>sys.stderr, "  NCAR, Boulder, CO, USA"
    print >>sys.stderr, ""
    print >>sys.stderr, "  " + dateTimeStr
    print >>sys.stderr, ""
    print >>sys.stderr, "****************************************************"
    print >>sys.stderr, "  dateStr: ", dateStr
    print >>sys.stderr, "  timeStr: ", timeStr
    print >>sys.stderr, "  platform: ", platform
    print >>sys.stderr, "  runDir: ", runDir
    print >>sys.stderr, "  installDir: ", installDir
    print >>sys.stderr, "  package: ", package
    print >>sys.stderr, "  version: ", version
    print >>sys.stderr, "  srcRelease: ", srcRelease
    print >>sys.stderr, "  installScripts: ", options.installScripts
    print >>sys.stderr, "****************************************************"

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
        print >>sys.stderr, "running cmd:", cmd, " ....."
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print >>sys.stderr, "Child exited with code: ", retcode
            sys.exit(1)
        else:
            if (options.verbose):
                print >>sys.stderr, "Child returned code: ", retcode
    except OSError as e:
        print >>sys.stderr, "Execution failed:", e
        sys.exit(1)

    if (options.debug):
        print >>sys.stderr, ".... done"
    
########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

#!/usr/bin/env python

#===========================================================================
#
# Install a CIDD binary release
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
    installBinDir = os.path.join(installDir, "bin")

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

    print >>sys.stderr, "****************************************************"
    print >>sys.stderr, "  Running " + thisScriptName
    print >>sys.stderr, ""
    print >>sys.stderr, "  Installing " + package + " binary release"
    print >>sys.stderr, ""
    print >>sys.stderr, "  NCAR, Boulder, CO, USA"
    print >>sys.stderr, ""
    print >>sys.stderr, "  " + dateTimeStr
    print >>sys.stderr, ""
    print >>sys.stderr, "****************************************************"
    print >>sys.stderr, "  dateStr: ", dateStr
    print >>sys.stderr, "  timeStr: ", timeStr
    print >>sys.stderr, "  runDir: ", runDir
    print >>sys.stderr, "  installBinDir: ", installBinDir
    print >>sys.stderr, "  platform: ", platform
    print >>sys.stderr, "  package: ", package
    print >>sys.stderr, "  version: ", version
    print >>sys.stderr, "  srcRelease: ", srcRelease
    print >>sys.stderr, "****************************************************"

    # create the install dir

    if (os.path.isdir(installBinDir) == False):
        os.makedirs(installBinDir)

    # do the install

    os.chdir(runDir)
    if (os.path.isdir("bin")):
        os.chdir("bin")
        shellCmd("rsync -av * " + installBinDir)

    os.chdir(runDir)
    if (os.path.isdir("scripts")):
        os.chdir("scripts")
        shellCmd("rsync -av convert_image.csh " + installBinDir)
        shellCmd("rsync -av convert_image_print.csh " + installBinDir)
        shellCmd("rsync -av make_anim.csh " + installBinDir)
        shellCmd("rsync -av make_anim.sh " + installBinDir)
        shellCmd("rsync -av prepare_for_cidd " + installBinDir)
        shellCmd("rsync -av set_font_path " + installBinDir)

    #--------------------------------------------------------------------
    # done
    
    print("  **************************************************")
    print("  *** Done installing CIDD binary release ***")
    print("  *** installed in dir: " + installBinDir + " ***")
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

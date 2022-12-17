#!/usr/bin/env python3

#===========================================================================
#
# Checkout and build CIDD in 32-bit mode.
#
# This script performs the following steps:
#
#   1. clone lrose-core from git
#   2. clone lrose-netcdf from git
#   3. setup autoconf Makefile.am files
#   4. run configure to create makefiles
#   5. perform the build in 32-bit mode, and install
#   6. check the build
#
# You can optionally specify a release date.
#
# Use --help to see the command line options.
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

def main():

    # globals

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global thisScriptDir
    thisScriptDir = os.path.dirname(__file__)
    os.chdir(thisScriptDir)
    thisScriptDir = os.getcwd()
    
    global options

    # parse the command line

    usage = "usage: " + thisScriptName + " [options]"
    homeDir = os.environ['HOME']
    prefixDirDefault = os.path.join(homeDir, 'cidd')
    buildDirDefault = '/tmp/cidd-build'
    logDirDefault = '/tmp/cidd-build/logs'

    parser = OptionParser(usage)

    parser.add_option('--clean',
                      dest='clean', default=False,
                      action="store_true",
                      help='Cleanup tmp build dir')
    parser.add_option('--debug',
                      dest='debug', default=True,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--verbose',
                      dest='verbose', default=False,
                      action="store_true",
                      help='Set verbose debugging on')
    parser.add_option('--releaseDate',
                      dest='releaseDate', default='latest',
                      help='Date from which to compute tag for git clone. Applies if --tag is not used.')
    parser.add_option('--tag',
                      dest='tag', default='master',
                      help='Tag to check out lrose. Overrides --releaseDate')
    parser.add_option('--prefix',
                      dest='prefix', default=prefixDirDefault,
                      help='Install directory, default is ~/lrose')
    parser.add_option('--buildDir',
                      dest='buildDir', default=buildDirDefault,
                      help='Temporary build dir, default is /tmp/lrose_build')
    parser.add_option('--logDir',
                      dest='logDir', default=logDirDefault,
                      help='Logging dir, default is /tmp/lrose_build/logs')
    parser.add_option('--installLroseRuntimeLibs',
                      dest='installLroseRuntimeLibs', default=False,
                      action="store_true",
                      help=\
                      'Install dynamic runtime lrose libraries for cidd binaries, ' + \
                      'in a directory relative to the bin dir. ' + \
                      'System libraries are not included.')

    (options, args) = parser.parse_args()
    
    if (options.verbose):
        options.debug = True

    # runtime

    now = time.gmtime()
    nowTime = datetime(now.tm_year, now.tm_mon, now.tm_mday,
                       now.tm_hour, now.tm_min, now.tm_sec)
    dateStr = nowTime.strftime("%Y%m%d")

    # set the script dir (lsose-core/build)
    
    scriptToCall = os.path.join(thisScriptDir, 'checkout_and_build_cmake.py')

    # set up command

    cmd = scriptToCall

    if (options.debug):
        cmd += " --debug"

    if (options.verbose):
        cmd += " --verbose"

    cmd += " --package lrose-cidd"

    if (options.tag != "master"):
        cmd += " --tag " + options.tag
    elif (options.releaseDate != "latest"):
        cmd += " --releaseDate " + options.releaseDate

    cmd += " --prefix " + options.prefix
    cmd += " --buildDir " + options.buildDir
    cmd += " --logDir " + options.logDir

    cmd += " --static"

    if (options.installLroseRuntimeLibs):
        cmd += " --installLroseRuntimeLibs"

    # cmd += " --noScripts"
    cmd += " --buildNetcdf"

    # debug print

    if (options.debug):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  package: lrose-cidd", file=sys.stderr)
        if (options.tag != "master"):
            print("  tag: ", options.tag, file=sys.stderr)
        else:
            print("  releaseDate: ", options.releaseDate, file=sys.stderr)
        print("  buildDir: ", options.buildDir, file=sys.stderr)
        print("  logDir: ", options.logDir, file=sys.stderr)
        print("  prefixDir: ", options.prefix, file=sys.stderr)
        print("  cmd to run: ", cmd, file=sys.stderr)

    # run the command

    shellCmd(cmd)

    # exit

    sys.exit(0)

########################################################################
# get the OS type from the /etc/os-release file in linux

def getOSType():

    osrelease_file = open("/etc/os-release", "rt")
    lines = osrelease_file.readlines()
    osrelease_file.close()
    osType = "unknown"
    for line in lines:
        if (line.find('PRETTY_NAME') == 0):
            lineParts = line.split('=')
            osParts = lineParts[1].split('"')
            osType = osParts[1]
    return osType

########################################################################
# Run a command in a shell, wait for it to complete

def shellCmd(cmd):

    print("Running cmd:", cmd, file=sys.stderr)
    
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

    print("    done", file=sys.stderr)
    
########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

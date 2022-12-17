#!/usr/bin/env python3

#===========================================================================
#
# Create a binary release for the lrose-core.
#
# This script performs the following steps:
#
#   1. create_src_release for core
#   2. untar the src release in /tmp
#   3. build the release from /tmp
#   4. install it in releases dir
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
    if (len(thisScriptDir) > 0):
        print("chdir to %s" % thisScriptDir, file=sys.stderr)
        os.chdir(thisScriptDir)
    thisScriptDir = os.getcwd()

    homeDir = os.environ['HOME']
    releaseDirDefault = os.path.join(homeDir, 'releases/binary')
    
    global options

    # parse the command line

    usage = "usage: " + thisScriptName + " [options]"
    parser = OptionParser(usage)

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
    parser.add_option('--releaseDir',
                      dest='releaseDir', default=releaseDirDefault,
                      help='Top-level release dir')
    parser.add_option('--static',
                      dest='static', default=False,
                      action="store_true",
                      help='Create static lib objects. Default is shared')

    (options, args) = parser.parse_args()
    
    if (options.verbose):
        options.debug = True

    # runtime

    now = time.gmtime()
    nowTime = datetime(now.tm_year, now.tm_mon, now.tm_mday,
                       now.tm_hour, now.tm_min, now.tm_sec)
    nowDateStr = nowTime.strftime("%Y%m%d")
    versionStr = "lrose-core-" + nowDateStr
    
    if (options.tag != "master"):
        versionStr = options.tag
    elif (options.releaseDate != "latest"):
        versionStr = "lrose-core-" + options.releaseDate
    
    if (options.debug):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  package: lrose-core", file=sys.stderr)
        print("  version: ", versionStr, file=sys.stderr)
        if (options.tag != "master"):
            print("  tag: ", options.tag, file=sys.stderr)
        else:
            print("  releaseDate: ", options.releaseDate, file=sys.stderr)
        if (options.releaseDir != releaseDirDefault):
            print("  releaseDir: ", options.releaseDir, file=sys.stderr)

    ################################################################
    # run create_src_release.py for lrose-core
    
    cmd = os.path.join(thisScriptDir, 'create_src_release.py')
    if (options.debug):
        cmd += " --debug"
    if (options.verbose):
        cmd += " --verbose"
    cmd += " --package lrose-core"
    if (options.tag != "master"):
        cmd += " --tag " + options.tag
    elif (options.releaseDate != "latest"):
        tag = "lrose-core-" + options.releaseDate
        cmd += " --tag " + tag
    cmd += " --releaseDir " + options.releaseDir
    cmd += " --force"
    if (options.static):
        cmd += " --static"
    shellCmd(cmd)

    ################################################################
    # untar the source in /tmp

    os.chdir("/tmp")
    shellCmd("tar xvfz " + options.releaseDir + "/lrose-core/" + versionStr + ".src.tgz > /tmp/lrose-core.untar.log")

    ################################################################
    # create a binary release by building the src release

    os.chdir("/tmp/" + versionStr + ".src")
    cmd = "create_bin_release.py"
    if (options.debug):
        cmd += " --debug"
    if (options.verbose):
        cmd += " --verbose"
    cmd += " --prefix /tmp/core-build"
    cmd += " --releaseDir " + options.releaseDir
    cmd += " --force"
    cmd += " --scripts"
    cmd += " --automake"
    shellCmd(cmd)

    # exit

    sys.exit(0)

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

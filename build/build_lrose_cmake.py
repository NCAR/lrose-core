#!/usr/bin/env python

#===========================================================================

# Building LROSE and required libraries
# =====================================
#
# Build lrose using cmake
#
# This script will be executed from the top level of lrose-core.
# 
# By default the libraries and applications will be installed in:
#
#   ${HOME}/lrose/include
#   ${HOME}/lrose/lib
#   ${HOME}/lrose/bin
#
# You can change the install location by specifying it as
# a single argument to this script.
#
# For example:
#
#   build_lrose_cmake.py --prefix /usr/local/lrose
#
# will install in:
#
#   /usr/local/lrose/include
#   /usr/local/lrose/lib
#   /usr/local/lrose/bin

# Depends on HDF5 and NETCDF
# --------------------------
# Before running this, either install hdf5 and netcdf
# checkout https://github.com/NCAR/lrose-netcdf
# build and install hdf5, netcdf

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
    global coreDir
    global codebaseDir
    global dateStr
    global timeStr
    global debugStr
    global package

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
    parser.add_option('--package',
                      dest='package', default='lrose-core',
                      help='Package name. Options are: ' + \
                      'lrose-core (default), lrose-radx, lrose-cidd')
    parser.add_option('--prefix',
                      dest='prefix', default=prefixDefault,
                      help='Prefix name for install location')
    parser.add_option('--scripts',
                      dest='installScripts', default=False,
                      action="store_true",
                      help='Install scripts as well as binaries')
    parser.add_option('--static',
                      dest='static', default=False,
                      action="store_true",
                      help='use static linking, default is dynamic')
    parser.add_option('--cmake3',
                      dest='use_cmake3', default=False,
                      action="store_true",
                      help='Use cmake3 instead of cmake for samurai')

    (options, args) = parser.parse_args()

    if (options.verbose):
        options.debug = True
        
    debugStr = " "
    if (options.verbose):
        debugStr = " --verbose "
    elif (options.debug):
        debugStr = " --debug "

    staticStr = " "
    if (options.static):
        staticStr = " --static "

    package = options.package

    # runtime

    now = time.gmtime()
    nowTime = datetime(now.tm_year, now.tm_mon, now.tm_mday,
                       now.tm_hour, now.tm_min, now.tm_sec)
    dateStr = nowTime.strftime("%Y%m%d")
    timeStr = nowTime.strftime("%Y%m%d%H%M%S")
    dateTimeStr = nowTime.strftime("%Y/%m/%d-%H:%M:%S")

    # script is in lrose-core/build
    
    global thisScriptDir
    thisScriptDir = os.path.dirname(__file__)

    # compute core dir relative to script dir

    coreDir = os.path.join(thisScriptDir, "..")
    os.chdir(coreDir)
    coreDir = os.getcwd()
    
    # check we are in the correct place

    codebaseDir = os.path.join(coreDir, "codebase")
    if (os.path.isdir(codebaseDir) == False):
        print("ERROR - script: ", thisScriptName, file=sys.stderr)
        print("  Incorrect run directory: ", coreDir, file=sys.stderr)
        print("  Must be run just above codebase dir", file=sys.stderr)
        sys.exit(1)
        
    # install the distribution-specific makefiles

    os.chdir(coreDir)
    cmd = "./build/scripts/installPackageMakefiles.py --package " + options.package
    shellCmd(cmd)

    # create CMakeLists.txt files for cmake

    os.chdir(coreDir)
    shellCmd("./build/cmake/createCMakeLists.py --coreDir . " +
             " --installPrefix " + options.prefix + debugStr + staticStr)

    # run cmake to generate Makefiles from CMakeLists.txt files
    # this is done in a build directory, so it an out-of-source build
    
    cmakeBuildDir = os.path.join(codebaseDir, "build")
    try:
        os.makedirs(cmakeBuildDir)
    except:
        print("Dir exists: " + cmakeBuildDir, file=sys.stderr)
    os.chdir(cmakeBuildDir)
    if (options.use_cmake3):
        shellCmd("cmake3 -DCMAKE_INSTALL_PREFIX=" + options.prefix + " ..")
    else:
        shellCmd("cmake -DCMAKE_INSTALL_PREFIX=" + options.prefix + " ..")

    # build and install libs

    os.chdir(os.path.join(cmakeBuildDir, "libs"))
    cmd = "make -j 8 install"
    shellCmd(cmd)

    # build and install tdrp_gen

    os.chdir(os.path.join(cmakeBuildDir, "apps/tdrp/src/tdrp_gen"))
    cmd = "make install"
    shellCmd(cmd)

    # build and install apps

    os.chdir(os.path.join(cmakeBuildDir, "apps"))
    cmd = "make -j 8 install"
    shellCmd(cmd)

    # optionally install the scripts

    if (options.installScripts):

        # install perl5
        
        perl5Dir = os.path.join(options.prefix, "lib/perl5")
        try:
            os.makedirs(perl5Dir)
        except:
            print("Dir exists: " + perl5Dir, file=sys.stderr)

        perl5LibDir = os.path.join(codebaseDir, "libs/perl5/src")
        if (os.path.isdir(perl5LibDir)):
            os.chdir(os.path.join(codebaseDir, "libs/perl5/src"))
            shellCmd("rsync -av *pm " + perl5Dir)

        # procmap

        procmapScriptsDir = os.path.join(codebaseDir, "apps/procmap/src/scripts")
        if (os.path.isdir(procmapScriptsDir)):
            os.chdir(procmapScriptsDir)
            shellCmd("./install_scripts.lrose " + options.prefix + "bin")

        # general

        generalScriptsDir = os.path.join(codebaseDir, "apps/scripts/src")
        if (os.path.isdir(generalScriptsDir)):
            os.chdir(generalScriptsDir)
            shellCmd("./install_scripts.lrose " + options.prefix + "bin")

    # check the install

    checkInstall()

########################################################################
# check the install

def checkInstall():

    os.chdir(coreDir)

    print(("============= Checking libs for " + package + " ============="))
    shellCmd("./build/scripts/checkLibs.py" + \
             " --prefix " + options.prefix + \
             " --package " + package)
    print("====================================================")
    print(("============= Checking apps for " + package + " ============="))
    shellCmd("./build/scripts/checkApps.py" + \
             " --prefix " + options.prefix + \
             " --package " + package)
    print("====================================================")
    
    print("**************************************************")
    print("*** Done building auto release *******************")
    print(("*** Installed in dir: " + options.prefix + " ***"))
    print("**************************************************")

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

#!/usr/bin/env python

#===========================================================================

# Building LROSE and required libraries
# =====================================
#
# Build lrose using configure based on the automake system.
#
# This script will be executed from the top level of lrose-core.
# 
# If --autoconf is specified, the Makefile.am files will be generated.
# If not, automake must already have been run to generate makefile.am files.
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
#   build_lrose_automake.py --prefix /usr/local/lrose
#
# will install in:
#
#   /usr/local/lrose/include
#   /usr/local/lrose/lib
#   /usr/local/lrose/bin

# Depends on HDF5, NETCDF and UDUNITS
# -----------------------------------
# Before running this,
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
    global prefix

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
    parser.add_option('--autoconf',
                      dest='autoconf', default=False,
                      action="store_true",
                      help='Force to run autoconf to generate Makefile.am files')
    parser.add_option('--scripts',
                      dest='installScripts', default=False,
                      action="store_true",
                      help='Install scripts as well as binaries')
    parser.add_option('--static',
                      dest='static', default=False,
                      action="store_true",
                      help='use static linking, default is dynamic')

    (options, args) = parser.parse_args()

    if (options.verbose):
        options.debug = True
        
    debugStr = " "
    if (options.verbose):
        debugStr = " --verbose "
    elif (options.debug):
        debugStr = " --debug "

    package = options.package
    prefix = options.prefix

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

    coreDir = os.path.join(thisScriptDir, "../..")
    os.chdir(coreDir)
    coreDir = os.getcwd()
    
    # check we are in the correct place

    codebaseDir = os.path.join(coreDir, "codebase")
    if (os.path.isdir(codebaseDir) == False):
        print("ERROR - script: ", thisScriptName, file=sys.stderr)
        print("  Incorrect run directory: ", coreDir, file=sys.stderr)
        print("  Must be run just above codebase dir", file=sys.stderr)
        sys.exit(1)
        
    # set up autoconf if needed

    os.chdir(codebaseDir)
    if ((os.path.isfile("configure") == False) or options.autoconf):
        setupAutoconf()

    # run qmake for QT apps to create moc_ files

    if (options.package.find("lrose-core") >= 0):
        mocDirs = ["apps/radar/src/HawkEye",
                   "apps/radar/src/HawkEdit",
                   "apps/radar/src/Condor"]
    elif (options.package.find("lrose") >= 0):
        mocDirs = ["apps/radar/src/HawkEye"]

    for dir in mocDirs:
        mocPath = os.path.join(codebaseDir, dir)
        createQtMocFiles(mocPath)

    # set the environment

    os.environ["LDFLAGS"] = "-L" + prefix + "/lib " + \
                            "-Wl,--enable-new-dtags," + \
                            "-rpath," + \
                            "'$$ORIGIN/" + package + "_runtime_libs" + \
                            ":$$ORIGIN/../lib" + \
                            ":" + prefix + "/lib'"
    os.environ["FC"] = "gfortran"
    os.environ["F77"] = "gfortran"
    os.environ["F90"] = "gfortran"

    if (platform == "darwin"):
        os.environ["PKG_CONFIG_PATH"] = "/usr/local/opt/qt/lib/pkgconfig"
    else:
        os.environ["CXXFLAGS"] = " -std=c++11 "

    print("=========================================", file=sys.stderr)
    shellCmd("env")
    print("=========================================", file=sys.stderr)

    # do the build and install

    os.chdir(codebaseDir)
    shellCmd("./configure --prefix=" + prefix)

    os.chdir(os.path.join(codebaseDir, "libs"))
    shellCmd("make -k -j 8")
    shellCmd("make -k install-strip")

    os.chdir(os.path.join(codebaseDir, "apps"))
    shellCmd("make -k -j 8")
    shellCmd("make -k install-strip")

    # optionally install the scripts

    if (options.installScripts):

        # general

        generalScriptsDir = os.path.join(codebaseDir, "apps/scripts/src")
        if (os.path.isdir(generalScriptsDir)):
            os.chdir(generalScriptsDir)
            shellCmd("./install_scripts.lrose " + prefix + "bin")

        # install perl5 - deprecated
        #
        #perl5Dir = os.path.join(prefix, "lib/perl5")
        #try:
        #    os.makedirs(perl5Dir)
        #except:
        #    print("Dir exists: " + perl5Dir, file=sys.stderr)
        #
        #perl5LibDir = os.path.join(codebaseDir, "libs/perl5/src")
        #if (os.path.isdir(perl5LibDir)):
        #    os.chdir(os.path.join(codebaseDir, "libs/perl5/src"))
        #    shellCmd("rsync -av *pm " + perl5Dir)

    # check the install

    checkInstall()

########################################################################
# check the install

def checkInstall():

    os.chdir(coreDir)

    print(("============= Checking libs for " + package + " ============="))
    shellCmd("./build/scripts/checkLibs.py" + \
             " --prefix " + prefix + \
             " --package " + package)
    print("====================================================")
    print(("============= Checking apps for " + package + " ============="))
    shellCmd("./build/scripts/checkApps.py" + \
             " --prefix " + prefix + \
             " --package " + package)
    print("====================================================")
    
    print("**************************************************")
    print("*** Done building auto release *******************")
    print(("*** Installed in dir: " + prefix + " ***"))
    print("**************************************************")

########################################################################
# Run qmake for QT apps such as HawkEye to create _moc files

def createQtMocFiles(appDir):
    
    if (os.path.isdir(appDir) == False):
        return
    
    os.chdir(appDir)
    shellCmd("rm -f moc*");
    if (platform == "darwin"):
        shellCmd("/usr/local/opt/qt/bin/qmake -o Makefile.qmake");
    else:
        shellCmd("qmake -o Makefile.qmake");
    shellCmd("make -f Makefile.qmake mocables");

########################################################################
# set up autoconf for configure etc

def setupAutoconf():

    if (platform == "darwin"):
        print("ERROR - setupAutoconf", file=sys.stderr)
        print("  Cannot run autoconf on OSX", file=sys.stderr)
        print("  This must be run in a LINUX host", file=sys.stderr)
        print("  On OSX, use a tar file which contains autoconf files", file=sys.stderr)
        sys.exit(1)

    os.chdir(codebaseDir)

    # create files for configure

    shutil.copy("../build/autoconf/Makefile.top",
                "./Makefile")
    
    if (options.static):
        if (package == "lrose-cidd"):
            shutil.copy("../build/autoconf/configure.base.cidd",
                        "./configure.base")
        else:
            shutil.copy("../build/autoconf/configure.base",
                        "./configure.base")
        shellCmd("../build/autoconf/createConfigure.am.py --dir . " +
                 " --baseName configure.base" +
                 " --pkg " + package + debugStr)
    else:
        if (package == "lrose-cidd"):
            shutil.copy("../build/autoconf/configure.base.shared.cidd",
                        "./configure.base.shared")
        else:
            shutil.copy("../build/autoconf/configure.base.shared",
                        "./configure.base.shared")
        shellCmd("../build/autoconf/createConfigure.am.py --dir . " +
                 " --baseName configure.base.shared --shared" +
                 " --pkg " + package + debugStr)

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

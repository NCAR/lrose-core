#!/usr/bin/env python

#===========================================================================

# Building LROSE and required libraries
# =====================================
#
# This requires automake makefile.am files
#
# This script must be run from the top level of lrose-core'.
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
#   build_lrose.py --prefix /usr/local/lrose
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

import os
import sys
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
    global codebaseDir
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
                      help='Package name. Options are lrose-core (default), radx, cidd, titan, lrose-blaze')
    parser.add_option('--prefix',
                      dest='prefix', default=prefixDefault,
                      help='Prefix name for install location')
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

    package = options.package
    prefix = options.prefix

    # runtime

    now = time.gmtime()
    nowTime = datetime(now.tm_year, now.tm_mon, now.tm_mday,
                       now.tm_hour, now.tm_min, now.tm_sec)
    dateTimeStr = nowTime.strftime("%Y/%m/%d-%H:%M:%S")
    corePath = os.getcwd()
    
    # check we are in the correct place

    codebaseDir = os.path.join(corePath, "codebase")
    if (os.path.isdir(codebaseDir) == False):
        print >>sys.stderr, "ERROR - script: ", thisScriptName
        print >>sys.stderr, "  Incorrect run directory: ", corePath
        print >>sys.stderr, "  Must be run just above codebase dir"
        sys.exit(1)

    # run qmake for QT apps to create moc_ files

    hawkEyeDir = os.path.join(codebaseDir, "apps/radar/src/HawkEye")
    createQtMocFiles(hawkEyeDir)

    # set the environment

    os.environ["LDFLAGS"] = "-L" + prefix + "/lib " + \
                            " -Wl,-rpath,'$$ORIGIN/" + package + "_runtime_libs:" + \
                            prefix + "/lib'"
    os.environ["FC"] = "gfortran"
    os.environ["F77"] = "gfortran"
    os.environ["F90"] = "gfortran"

    if (platform == "darwin"):
        os.environ["PKG_CONFIG_PATH"] = "/usr/local/opt/qt/lib/pkgconfig"
    else:
        os.environ["CXXFLAGS"] = " -std=c++11 "

    cmd = "env"
    print >>sys.stderr, "========================================="
    shellCmd(cmd)
    print >>sys.stderr, "========================================="

    # do the build and install

    os.chdir(codebaseDir)
    cmd = "./configure --with-hdf5=" + prefix + \
          " --with-netcdf=" + prefix + \
          " --prefix=" + prefix
    shellCmd(cmd)

    os.chdir(os.path.join(codebaseDir, "libs"))
    cmd = "make -k -j 8"
    shellCmd(cmd)
    cmd = "make -k install-strip"
    shellCmd(cmd)

    os.chdir(os.path.join(codebaseDir, "apps"))
    cmd = "make -k -j 8"
    shellCmd(cmd)
    cmd = "make -k install-strip"
    shellCmd(cmd)

    # optionally install the scripts

    if (options.installScripts):

        # install perl5
        
        perl5Dir = os.path.join(prefix, "lib/perl5")
        try:
            os.makedirs(perl5Dir)
        except:
            print >>sys.stderr, "Dir exists: " + perl5Dir

        perl5LibDir = os.path.join(codebaseDir, "libs/perl5/src")
        if (os.path.isdir(perl5LibDir)):
            os.chdir(os.path.join(codebaseDir, "libs/perl5/src"))
            shellCmd("rsync -av *pm " + perl5Dir)

        # procmap

        procmapScriptsDir = os.path.join(codebaseDir, "apps/procmap/src/scripts")
        if (os.path.isdir(procmapScriptsDir)):
            os.chdir(procmapScriptsDir)
            shellCmd("./install_scripts.lrose " + prefix + "bin")

        # general

        generalScriptsDir = os.path.join(codebaseDir, "apps/scripts/src")
        if (os.path.isdir(generalScriptsDir)):
            os.chdir(generalScriptsDir)
            shellCmd("./install_scripts.lrose " + prefix + "bin")

    # check the install

    checkInstall(corePath)

########################################################################
# check the install

def checkInstall(corePath):

    os.chdir(corePath)
    print("============= Checking libs for " + package + " =============")
    shellCmd("./codebase/make_bin/check_libs.py " + \
             "--listPath ./build/checklists/libs_check_list." + package + " " + \
             "--libDir " + prefix + "/lib " + \
             "--label " + package + " --maxAge 3600")
    print("====================================================")
    print("============= Checking apps for " + package + " =============")
    shellCmd("./codebase/make_bin/check_apps.py " + \
             "--listPath ./build/checklists/apps_check_list." + package + " " + \
             "--appDir " + prefix + "/bin " + \
             "--label " + package + " --maxAge 3600")
    print("====================================================")
    
    print("**************************************************")
    print("*** Done building package: " + package)
    print("*** Installed in dir: " + prefix + " ***")
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
        shellCmd("qmake-qt5 -o Makefile.qmake");
    shellCmd("make -f Makefile.qmake mocables");

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

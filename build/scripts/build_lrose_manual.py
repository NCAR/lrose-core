#!/usr/bin/env python

#===========================================================================

# Building LROSE and required libraries
# =====================================
#
# This is the manual build, using the LROSE make system.
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
#   build_lrose_manual.py --prefix /usr/local/lrose
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
    dateStr = nowTime.strftime("%Y%m%d")
    timeStr = nowTime.strftime("%Y%m%d%H%M%S")
    dateTimeStr = nowTime.strftime("%Y/%m/%d-%H:%M:%S")
    
    # script is in lrose-core/build
    
    global thisScriptDir
    thisScriptDir = os.path.dirname(__file__)

    # compute core dir relative to script dir

    global coreDir
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

    # set flag to indicate OSX on a mac
    # For OSX, Makefile and makefile are confused because the
    # file system is not properly case-sensitive

    isOsx = False
    if (platform == "darwin"):
        isOsx = True
    
    # set the environment

    os.environ["LROSE_CORE_DIR"] = coreDir
    os.environ["LROSE_INSTALL_DIR"] = prefix

    os.environ["HOST_OS"] = "LINUX_LROSE"
    if (isOsx):
        os.environ["HOST_OS"] = "OSX_LROSE"
    elif (package == "lrose-cidd"):
        os.environ["HOST_OS"] = "LINUX_64_CIDD32"

    print("=========================================", file=sys.stderr)
    shellCmd("env")
    print("=========================================", file=sys.stderr)

    # install makefiles for this package

    print("==== installing package makefiles =======", file=sys.stderr)
    if (isOsx):
        shellCmd("./build/scripts/installPackageMakefiles.py --package " +
                 package + " --osx")
    else:
        shellCmd("./build/scripts/installPackageMakefiles.py --package " +
                 package)

    print("==== finished installing makefiles ======", file=sys.stderr)

    # build and install tdrp lib

    print("==== installing tdrp lib =================", file=sys.stderr)
    os.chdir(os.path.join(codebaseDir, "libs/tdrp/src"))
    shellCmd("make -j 8 install")
    print("==== finished installing tdrp lib ========", file=sys.stderr)
    
    # build and install tdrp_gen

    print("==== installing tdrp_gen =================", file=sys.stderr)
    os.chdir(os.path.join(codebaseDir, "apps/tdrp/src/tdrp_gen"))
    shellCmd("make -j 8 install")
    print("==== finished installing tdrp_gen ========", file=sys.stderr)
    
    # install include files

    print("==== installing all includes =============", file=sys.stderr)
    os.chdir(os.path.join(codebaseDir, "libs"))
    shellCmd("make -j 8 -k install_include")
    print("==== finished installing all includes ====", file=sys.stderr)
    
    # build and install libs

    print("==== installing all libs =================", file=sys.stderr)
    os.chdir(os.path.join(codebaseDir, "libs"))
    shellCmd("make -j 8 -k install")
    print("==== finished installing all libs ========", file=sys.stderr)
    
    # build and install apps

    print("==== installing all apps =================", file=sys.stderr)
    os.chdir(os.path.join(codebaseDir, "apps"))
    shellCmd("make -j 8 -k install")
    print("==== finished installing all apps ========", file=sys.stderr)
    
    # create pkgconfig scripys

    print("==== installing pkgconfig ================", file=sys.stderr)
    os.chdir(coreDir)
    shellCmd("./build/scripts/createPkgConfig.py " + prefix)
    shellCmd("rsync -av " + coreDir + "/build/templates/lrose-config.template " + prefix + "/bin")
    shellCmd("chmod u+x " + prefix + "/bin/lrose-config")
    print("==== finished installing pkgconfig =======", file=sys.stderr)
    
    # optionally install the scripts

    if (options.installScripts):

        # general

        generalScriptsDir = os.path.join(codebaseDir, "apps/scripts/src")
        if (os.path.isdir(generalScriptsDir)):
            os.chdir(generalScriptsDir)
            shellCmd("./install_scripts.lrose " + prefix + "bin")

        # install perl5 - deprecated
        #
        #print("==== installing perl5 modules ===========", file=sys.stderr)
        #perl5TargetDir = os.path.join(prefix, "lib/perl5")
        #try:
        #    os.makedirs(perl5TargetDir)
        #except:
        #    print("Dir exists: " + perl5TargetDir, file=sys.stderr)
        #perl5SourceDir = os.path.join(codebaseDir, "libs/perl5/src")
        #if (os.path.isdir(perl5SourceDir)):
        #    os.chdir(perl5SourceDir)
        #    shellCmd("rsync -av *pm " + perl5TargetDir)
        #print("==== finished installing perl5 modules ==", file=sys.stderr)

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
    print(("*** Done building package: " + package))
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

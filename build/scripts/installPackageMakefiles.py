#!/usr/bin/env python3

#===========================================================================
#
# install the makefiles for a given package
#
#===========================================================================

from __future__ import print_function
import os
import sys
import shutil
from sys import platform
from optparse import OptionParser
from datetime import datetime

def main():

    global options
    global subdirList
    global makefileCreateList

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    # parse the command line

    usage = "usage: %prog [options]"

    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=True,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--osx',
                      dest='osx', default=False,
                      action="store_true",
                      help='Configure for MAC OSX')
    parser.add_option('--package',
                      dest='package', default="lrose-core",
                      help='Package name. Options are: ' + \
                      'lrose-core (default), lrose-radx, lrose-cidd')

    (options, args) = parser.parse_args()

    # set flag to indicate OSX on a mac
    # For OSX, Makefile and makefile are confused because the
    # file system is not properly case-sensitive

    isOsx = False
    if (platform == "darwin"):
        isOsx = True
    if (options.osx):
        isOsx = True
    
    if (options.debug):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  package:", options.package, file=sys.stderr)
        print("  osx: ", isOsx, file=sys.stderr)

    # script is in lrose-core/build/scripts

    global thisScriptDir
    thisScriptDir = os.path.dirname(__file__)

    # compute codebase dir relative to script dir

    codebaseDir = os.path.join(thisScriptDir, "../../codebase")

    # go to the codebase dir
    # get the complete path
    
    os.chdir(codebaseDir)
    codebaseDir = os.getcwd()
    if (options.debug):
        print("  codebaseDir: %s" % codebaseDir, file=sys.stderr)

    # install the makefiles

    if (isOsx):
        doInstallOsx()
    else:
        doInstall()
            
    sys.exit(0)

########################################################################
# install the makefiles

def doInstall():

    # search for given makefile name

    packageMakefileName = "makefile" + "." + options.package
    if (options.debug):
        print("Searching for makefiles: ", packageMakefileName, file=sys.stderr)

    # first remove any existing 'makefile' files

    for root, dirs, files in os.walk(".", topdown=False):
        for dir in dirs:
            dirPath = os.path.join(root, dir)
            # check if makefile exists, remove it if it does
            makefilePath = os.path.join(root, "makefile")
            if (os.path.isfile(makefilePath)):
                if (options.debug):
                    print("Removing " + makefilePath, file=sys.stderr)
                os.remove(makefilePath)

    # find _makefiles dirs in tree

    for root, dirs, files in os.walk(".", topdown=False):
        for dir in dirs:
            if (dir == "_makefiles"):
                dirPath = os.path.join(root, dir)
                if (options.package == "lrose-core"):
                    # for the core, use Makefile instead of makefile
                    # since this will reinstall the defaults
                    makefilePath = os.path.join(root, "Makefile")
                else:
                    makefilePath = os.path.join(root, "makefile")
                # check if package makefile exists
                packageMakefilePath = os.path.join(dirPath, packageMakefileName)
                if (os.path.isfile(packageMakefilePath)):
                    # copy the package makefile to the root/makefile
                    if (options.debug):
                        print("Copying " + packageMakefilePath + \
                        " to " + makefilePath, file=sys.stderr)
                    shutil.copy(packageMakefilePath, makefilePath)

    return

########################################################################
# install the makefiles

def doInstallOsx():

    # search for given makefile name

    packageMakefileName = "makefile" + "." + options.package
    if (options.debug):
        print("Searching for makefiles: ", packageMakefileName, file=sys.stderr)

    # find _makefiles dirs in tree

    for root, dirs, files in os.walk(".", topdown=False):
        for dir in dirs:
            if (dir == "_makefiles"):
                dirPath = os.path.join(root, dir)
                makefilePathLower = os.path.join(root, "makefile")
                makefilePathUpper = os.path.join(root, "Makefile")
                # check if package makefile exists
                packageMakefilePath = os.path.join(dirPath, packageMakefileName)
                if (os.path.isfile(packageMakefilePath)):
                    # remove makefile in the target dir
                    # because OSX is not properly case sensitive
                    if (os.path.isfile(makefilePathLower)):
                        os.remove(makefilePathLower)
                    if (os.path.isfile(makefilePathUpper)):
                        os.remove(makefilePathUpper)
                    # copy the package makefile to the root/makefile
                    if (options.debug):
                        print("Copying " + packageMakefilePath + \
                            " to " + makefilePathLower, file=sys.stderr)
                    shutil.copy(packageMakefilePath, makefilePathLower)

    return

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

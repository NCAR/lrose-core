#!/usr/bin/env python

#===========================================================================
#
# install the makefiles for a given package
#
#===========================================================================

import os
import sys
import shutil
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
    try:
        coreDir = os.environ['LROSE_CORE_DIR']
    except:
        coreDir = os.getcwd()
    defaultCodeDir = os.path.join(coreDir, "codebase")
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default='False',
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--codedir',
                      dest='codedir', default=defaultCodeDir,
                      help='Code dir from which we search for makefiles')
    parser.add_option('--package',
                      dest='package', default="lrose",
                      help='Name of distribution for which we are building')
    parser.add_option('--osx',
                      dest='osx', default='False',
                      action="store_true",
                      help='Configure for MAC OSX')

    (options, args) = parser.parse_args()
    
    if (options.debug == True):
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  codedir:", options.codedir
        print >>sys.stderr, "  package:", options.package
        print >>sys.stderr, "  osx: ", options.osx

    # go to the code dir

    os.chdir(options.codedir)

    # install the makefiles

    if (options.osx == True):
        doInstallOsx()
    else:
        doInstall()
            
    sys.exit(0)

########################################################################
# install the makefiles

def doInstall():

    # search for given makefile name

    packageMakefileName = "makefile" + "." + options.package
    if (options.debug == True):
        print >>sys.stderr, "Searching for makefiles: ", packageMakefileName

    # first remove any existing 'makefile' files

    for root, dirs, files in os.walk(".", topdown=False):
        for dir in dirs:
            dirPath = os.path.join(root, dir)
            # check if makefile exists, remove it if it does
            makefilePath = os.path.join(root, "makefile")
            if (os.path.isfile(makefilePath)):
                if (options.debug):
                    print >>sys.stderr, "Removing " + makefilePath
                os.remove(makefilePath)

    # find _makefiles dirs in tree

    for root, dirs, files in os.walk(".", topdown=False):
        for dir in dirs:
            if (dir == "_makefiles"):
                dirPath = os.path.join(root, dir)
                makefilePath = os.path.join(root, "makefile")
                # check if package makefile exists
                packageMakefilePath = os.path.join(dirPath, packageMakefileName)
                if (os.path.isfile(packageMakefilePath)):
                    # copy the package makefile to the root/makefile
                    if (options.debug):
                        print >>sys.stderr, \
                        "Copying " + packageMakefilePath + \
                        " to " + makefilePath
                    shutil.copy(packageMakefilePath, makefilePath)

    return

########################################################################
# install the makefiles

def doInstallOsx():

    # search for given makefile name

    packageMakefileName = "makefile" + "." + options.package
    if (options.debug == True):
        print >>sys.stderr, "Searching for makefiles: ", packageMakefileName

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
                    if (os.path.isfile(makefilePathLower) == True):
                        os.remove(makefilePathLower)
                    if (os.path.isfile(makefilePathUpper) == True):
                        os.remove(makefilePathUpper)
                    # copy the package makefile to the root/makefile
                    if (options.debug):
                        print >>sys.stderr, \
                            "Copying " + packageMakefilePath + \
                            " to " + makefilePathLower
                    shutil.copy(packageMakefilePath, makefilePathLower)

    return

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

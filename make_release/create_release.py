#!/usr/bin/env python

#===========================================================================
#
# Create an LROSE release
#
#===========================================================================

import os
import sys
import shutil
import subprocess
from optparse import OptionParser
from datetime import datetime
import glob

def main():

    # globals

    global options
    global thisScriptName
    global releaseDir
    global tmpDir
    global coreDir
    global codebaseDir
    global debugStr
    thisScriptName = os.path.basename(__file__)

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    releaseDirDefault = os.path.join(homeDir, 'releases')
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
                      dest='package', default='lrose',
                      help='Package name, default is lrose')
    parser.add_option('--releaseDir',
                      dest='releaseTopDir', default=releaseDirDefault,
                      help='Top-level release dir')
    parser.add_option('--force',
                      dest='force', default=False,
                      action="store_true",
                      help='force, do not request user to check it is OK to proceed')
    parser.add_option('--static',
                      dest='static', default=False,
                      action="store_true",
                      help='produce distribution for static linking, default is dynamic')

    (options, args) = parser.parse_args()

    if (options.verbose == True):
        options.debug = True
        
    debugStr = " "
    if (options.verbose):
        debugStr = " --verbose "
    elif (options.debug):
        debugStr = " --debug "

    releaseDir = os.path.join(options.releaseTopDir, options.package)
    tmpDir = os.path.join(releaseDir, "tmp")
    coreDir = os.path.join(tmpDir, "lrose-core")
    codebaseDir = os.path.join(coreDir, "codebase")

    if (options.debug == True):
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  package: ", options.package
        print >>sys.stderr, "  releaseTopDir: ", options.releaseTopDir
        print >>sys.stderr, "  releaseDir: ", releaseDir
        print >>sys.stderr, "  tmpDir: ", tmpDir
        print >>sys.stderr, "  force: ", options.force
        print >>sys.stderr, "  static: ", options.static

    # save previous releases

    savePrevReleases()

    # create tmp dir

    createTmpDir()

    # get repos from git

    gitCheckout()

    # set up autoconf

    setupAutoconf()

    sys.exit(0)

    # go to the src dir

    srcDir = os.path.join(options.dir, 'src')
    if (options.debug == True):
        print >>sys.stderr, "src dir: ", srcDir
    os.chdir(srcDir)

    # get makefile name in use
    # makefile has preference over Makefile

    makefileName = '__makefile.template'
    if (os.path.exists(makefileName) == False):
        makefileName = 'makefile'
        if (os.path.exists(makefileName) == False):
            makefileName = 'Makefile'
            if (os.path.exists(makefileName) == False):
                print >>sys.stderr, "ERROR - ", thisScriptName
                print >>sys.stderr, "  Cannot find makefile or Makefile"
                print >>sys.stderr, "  dir: ", options.dir
                sys.exit(1)

    # copy makefile in case we rerun this script

    if (makefileName != "__makefile.template"):
        shutil.copy(makefileName, "__makefile.template")

    if (options.debug == True):
        print >>sys.stderr, "-->> using makefile template: ", makefileName

    # get the lib name

    thisLibName = ""
    getLibName()
    if (options.debug == True):
        print >>sys.stderr, "  Lib name: ", thisLibName

    # get list of subdirs and their makefiles

    getSubDirList()

    if (options.debug == True):
        print >>sys.stderr, "======================="
        for subDir in subDirList:
            print >>sys.stderr, "subDir, makefile: %s, %s" % \
                (subDir.subDirName, subDir.makefilePath)
        print >>sys.stderr, "======================="

    # load list of files to be compiled

    compileFileList = []
    for subDir in subDirList:
        addSubDirToCompileList(subDir)

    if (options.debug == True):
        print >>sys.stderr, "======================="
        for compileFile in compileFileList:
            print >>sys.stderr, "compileFile: %s" % (compileFile)
        print >>sys.stderr, "======================="

    # get list of header files

    loadHeaderFileList()
    if (options.debug == True):
        print >>sys.stderr, "======================="
        for headerFile in headerFileList:
            print >>sys.stderr, "headerFile: %s" % (headerFile)
        print >>sys.stderr, "======================="

    # get list of include directories to be referenced

#    for headerFile in headerFileList:
#        setIncludeList(headerFile)

#    for compileFile in compileFileList:
#        setIncludeList(compileFile)

#    if (options.debug == True):
#        for lib in includeList:
#            if (lib.used == True):
#                print >>sys.stderr, "Use lib for include: %s" % (lib.name)

    # write out makefile.am

    writeMakefileAm()

    sys.exit(0)

########################################################################
# move previous releases

def savePrevReleases():

    os.chdir(releaseDir)
    prevDirPath = os.path.join(releaseDir, 'previous_releases')

    # remove if file instead of dir

    if (os.path.isfile(prevDirPath)):
        os.remove(prevDirPath)

    # ensure dir exists
    
    if (os.path.isdir(prevDirPath) == False):
        os.makedirs(prevDirPath)

    # get old releases

    pattern = options.package + "-????????*.tgz"
    oldReleases = glob.glob(pattern)

    for name in oldReleases:
        newName = os.path.join(prevDirPath, name)
        if (options.debug):
            print >>sys.stderr, "saving oldRelease: ", name
            print >>sys.stderr, "to: ", newName
        os.rename(name, newName)

########################################################################
# create the tmp dir

def createTmpDir():

    # check if exists already

    if (os.path.isdir(tmpDir)):

        if (options.force == False):
            print("WARNING: you are about to remove all contents in dir: " + tmpDir)
            print("===============================================")
            contents = os.listdir(tmpDir)
            for filename in contents:
                print("  " + filename)
            print("===============================================")
            answer = raw_input("WARNING: do you wish to proceed (y/n)? ")
            if (answer != "y"):
                print("  aborting ....")
                sys.exit(1)
                
        # remove it

        shutil.rmtree(tmpDir)

    # make it clean

    os.makedirs(tmpDir)

########################################################################
# check out repos from git

def gitCheckout():

    os.chdir(tmpDir)

    runCommand("git clone https://github.com/NCAR/lrose-core")
    runCommand("git clone https://github.com/NCAR/lrose-netcdf")

########################################################################
# set up autoconf for configure etc

def setupAutoconf():

    os.chdir(codebaseDir)

    # install the distribution-specific makefiles

    runCommand("./make_bin/install_distro_makefiles.py --distro " + 
               options.package + " --codedir .")

    # create files for configure

    shutil.copy("../build/Makefile.top", "Makefile")

    if (options.static):
        shutil.copy("../build/configure.base", "./configure.base")
        runCommand("./make_bin/createConfigure.am.py --dir ." +
                   " --baseName configure.base" +
                   " --pkg " + options.package + debugStr)
    else:
        shutil.copy("../build/configure.base.shared", "./configure.base.shared")
        runCommand("./make_bin/createConfigure.am.py --dir ." +
                   " --baseName configure.base.shared --shared" +
                   " --pkg " + options.package + debugStr)

########################################################################
# Run a command in a shell, wait for it to complete

def runCommand(cmd):

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

#!/usr/bin/env python

#===========================================================================
#
# Create an LROSE source release
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

def main():

    # globals

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global options
    global releaseDir
    global tmpDir
    global coreDir
    global codebaseDir
    global versionStr
    global debugStr
    global releaseName
    global tarName
    global tarDir

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    releaseDirDefault = os.path.join(homeDir, 'releases')
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=True,
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

    # runtime

    now = time.gmtime()
    nowTime = datetime(now.tm_year, now.tm_mon, now.tm_mday,
                       now.tm_hour, now.tm_min, now.tm_sec)
    versionStr = nowTime.strftime("%Y%m%d")

    # set globals

    releaseDir = os.path.join(options.releaseTopDir, options.package)
    tmpDir = os.path.join(releaseDir, "tmp")
    coreDir = os.path.join(tmpDir, "lrose-core")
    codebaseDir = os.path.join(coreDir, "codebase")

    # compute release name and dir name
    
    releaseName = options.package + "-" + versionStr + ".src"
    tarName = releaseName + ".tgz"
    tarDir = os.path.join(coreDir, releaseName)

    if (options.debug == True):
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  package: ", options.package
        print >>sys.stderr, "  releaseTopDir: ", options.releaseTopDir
        print >>sys.stderr, "  releaseDir: ", releaseDir
        print >>sys.stderr, "  tmpDir: ", tmpDir
        print >>sys.stderr, "  force: ", options.force
        print >>sys.stderr, "  static: ", options.static
        print >>sys.stderr, "  versionStr: ", versionStr
        print >>sys.stderr, "  releaseName: ", releaseName
        print >>sys.stderr, "  tarName: ", tarName
        
    # save previous releases

    savePrevReleases()

    # create tmp dir

    createTmpDir()

    # get repos from git

    gitCheckout()

    # set up autoconf

    setupAutoconf()

    # create the release information file
    
    createReleaseInfoFile()

    # create the tar file

    createTarFile()

    # create the brew formula for OSX builds

    createBrewFormula()

    # move the tar file up into release dir

    os.chdir(releaseDir)
    os.rename(os.path.join(coreDir, tarName),
              os.path.join(releaseDir, tarName))
              
    # delete the tmp dir

    shutil.rmtree(tmpDir)

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
    shellCmd("git clone https://github.com/NCAR/lrose-core")
    shellCmd("git clone https://github.com/NCAR/lrose-netcdf")

########################################################################
# set up autoconf for configure etc

def setupAutoconf():

    os.chdir(codebaseDir)

    # install the distribution-specific makefiles

    shellCmd("./make_bin/install_distro_makefiles.py --distro " + 
               options.package + " --codedir .")

    # create files for configure

    shutil.copy("../build/Makefile.top", "Makefile")

    if (options.static):
        shutil.copy("../build/configure.base", "./configure.base")
        shellCmd("./make_bin/createConfigure.am.py --dir ." +
                 " --baseName configure.base" +
                 " --pkg " + options.package + debugStr)
    else:
        shutil.copy("../build/configure.base.shared", "./configure.base.shared")
        shellCmd("./make_bin/createConfigure.am.py --dir ." +
                 " --baseName configure.base.shared --shared" +
                 " --pkg " + options.package + debugStr)

########################################################################
# write release information file

def createReleaseInfoFile():

    # go to core dir
    os.chdir(coreDir)

    # open info file

    releaseInfoPath = os.path.join(coreDir, "ReleaseInfo.txt")
    info = open(releaseInfoPath, 'w')

    # write release info

    info.write("package:" + options.package + "\n")
    info.write("version:" + versionStr + "\n")
    info.write("release:" + releaseName + "\n")

    # close

    info.close()

########################################################################
# create the tar file

def createTarFile():

    # go to core dir, make tar dir

    os.chdir(coreDir)
    os.makedirs(tarDir)

    # move lrose contents into tar dir

    for fileName in [ "LICENSE.txt", "README.md", "ReleaseInfo.txt" ]:
        os.rename(fileName, os.path.join(tarDir, fileName))

    for dirName in [ "build", "codebase", "docs", "make_release", "release_notes" ]:
        os.rename(dirName, os.path.join(tarDir, dirName))

    # move netcdf support into tar dir

    netcdfDir = os.path.join(tmpDir, "lrose-netcdf")
    netcdfSubDir = os.path.join(tarDir, "lrose-netcdf")
    os.makedirs(netcdfSubDir)
    
    for name in [ "README.md", "build_and_install_netcdf", "tar_files" ]:
        os.rename(os.path.join(netcdfDir, name),
                  os.path.join(netcdfSubDir, name))

    # create the tar file

    shellCmd("tar cvfz " + tarName + " " + releaseName)
    
########################################################################
# create the brew formula for OSX builds

def createBrewFormula():

    # go to core dir

    os.chdir(coreDir)

    # create the brew formula file

    tarUrl = "https://github.com/NCAR/lrose-core/releases/" + tarName
    formulaName = options.package + ".rb"
    scriptName = "build_" + options.package + "_formula"
    buildDirPath = os.path.join(tarDir, "build")
    scriptPath = os.path.join(buildDirPath, scriptName)
    
    shellCmd(scriptPath + " " + tarUrl + " " +
             tarName + " " + formulaName)

    # move it up into the release dir

    os.rename(os.path.join(coreDir, formulaName),
              os.path.join(releaseDir, formulaName))

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

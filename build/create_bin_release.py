#!/usr/bin/env python

#===========================================================================
#
# Prepare an LROSE binary release
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
    global runDir
    global releaseDir
    global tmpDir
    global coreDir
    global codebaseDir
    global dateStr
    global timeStr
    global debugStr
    global releaseName
    global tarName
    global tarDir

    global package
    global version
    global srcRelease

    global ostype

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
    parser.add_option('--prefix',
                      dest='prefix', default='not-set',
                      help='Prefix name for install location')
    parser.add_option('--releaseDir',
                      dest='releaseTopDir', default=releaseDirDefault,
                      help='Top-level release dir')
    parser.add_option('--force',
                      dest='force', default=False,
                      action="store_true",
                      help='force, do not request user to check if it is OK to proceed')
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

    # runtime

    now = time.gmtime()
    nowTime = datetime(now.tm_year, now.tm_mon, now.tm_mday,
                       now.tm_hour, now.tm_min, now.tm_sec)
    dateStr = nowTime.strftime("%Y%m%d")
    timeStr = nowTime.strftime("%Y%m%d%H%M%S")
    dateTimeStr = nowTime.strftime("%Y/%m/%d-%H:%M:%S")
    runDir = os.getcwd()

    # read in release info

    readReleaseInfoFile()

    # get the OS type - x86_64, i686, macosx_64
    
    getOsType()

    # set tmpDir temporary staging area
    # this is set to a very long name because on macosx
    # we need to reset the library paths and we need to
    # ensure there is space available for the rename

    if (options.prefix == "not-set"):
        tmpDir = os.path.join("/tmp", package + "_prepare_release_bin_directory")
    else:
        tmpDir = options.prefix

    # set directories

    releaseDir = os.path.join(options.releaseTopDir, package)
    coreDir = os.path.join(tmpDir, "lrose-core")
    codebaseDir = os.path.join(coreDir, "codebase")

    # compute release name and dir name
    
    releaseName = package + "-" + dateStr + "." + ostype
    tarName = releaseName + ".tgz"
    tarDir = os.path.join(tmpDir, releaseName)
    
    print >>sys.stderr, "*********************************************************************"
    print >>sys.stderr, "  Running " + thisScriptName
    print >>sys.stderr, ""
    print >>sys.stderr, "  Preparing " + package + " binary release"
    print >>sys.stderr, "  OS type: " + ostype
    print >>sys.stderr, ""
    print >>sys.stderr, "  NCAR, Boulder, CO, USA"
    print >>sys.stderr, ""
    print >>sys.stderr, "  " + dateTimeStr
    print >>sys.stderr, ""
    print >>sys.stderr, "*********************************************************************"
    print >>sys.stderr, "  dateStr: ", dateStr
    print >>sys.stderr, "  timeStr: ", timeStr
    print >>sys.stderr, "  prefix: ", options.prefix
    print >>sys.stderr, "  package: ", package
    print >>sys.stderr, "  version: ", version
    print >>sys.stderr, "  srcRelease: ", srcRelease
    print >>sys.stderr, "  tmpDir: ", tmpDir
    print >>sys.stderr, "  releaseName: ", releaseName
    print >>sys.stderr, "  tarName: ", tarName
    print >>sys.stderr, "  tarDir: ", tarDir
    print >>sys.stderr, "  installScripts: ", options.installScripts
    print >>sys.stderr, "*********************************************************************"

    # create tmp dir for staging area

    createTmpDir()

    # For full LROSE package, copy in CIDD binaries if they are available

    if (package == "lrose"):
        copyCiddBinaries()

    # create the tar dir
        
    os.makedirs(tarDir)

    # build netcdf support

    buildNetcdf()

    # build the package

    buildPackage()

    # detect which dynamic libs are needed
    # copy the dynamic libraries into runtime area:
    #     $prefix/bin/${package}_runtime_libs

    os.chdir(runDir)
    shellCmd("./codebase/make_bin/installOriginLibFiles.py --binDir " + \
             tmpDir + "/bin " + \
             "--relDir " + package + "_runtime_libs --debug")

    # copy the required files and directories into the tar directory
    
    shellCmd("/bin/cp -f LICENSE.txt " + tarDir)
    shellCmd("/bin/cp -rf release_notes " + tarDir)
    shellCmd("/bin/cp -f docs/README_INSTALL_BIN.txt " + tarDir)
    shellCmd("/bin/cp -f ./build/install_bin_release " + tarDir)
    shellCmd("/bin/cp -f ./build/install_devel_release " + tarDir)
    shellCmd("/bin/cp -rf " + tmpDir + "/bin " + tarDir)
    shellCmd("/bin/cp -rf " + tmpDir + "/lib " + tarDir)
    shellCmd("/bin/cp -rf " + tmpDir + "/include " + tarDir)

    if (package == "cidd"):
        shellCmd("/bin/cp -rf ./codebase/apps/cidd/src/CIDD/example_scripts " + tarDir)

    # make the tar file, copy into run dir

    os.chdir(tmpDir)
    shellCmd("tar cvfz " + tarName + " " + releaseName)
    shellCmd("mv " + tarName + "  " + runDir)
    os.chdir(runDir)

    # copy into release dir if it exists

    if (os.path.isdir(releaseDir)):
        shellCmd("cp " + tarName + "  " + releaseDir)

    # check the build
    
    os.chdir(runDir)
    print("============= Checking libs for " + package + " =============")
    shellCmd("./codebase/make_bin/check_libs.py " + \
             "--listPath ./build/libs_check_list." + package + " " + \
             "--libDir " + tmpDir + "/lib " + \
             "--label " + package + " --maxAge 3600")
    print("====================================================")
    print("============= Checking apps for " + package + " =============")
    shellCmd("./codebase/make_bin/check_apps.py " + \
             "--listPath ./build/apps_check_list." + package + " " + \
             "--appDir " + tmpDir + "/bin " + \
             "--label " + package + " --maxAge 3600")
    print("====================================================")
    
    #--------------------------------------------------------------------
    # done
    
    print("  **************************************************")
    print("  *** Done preparing binary release ***")
    print("  *** binary tar file name: " + tarName + " ***")
    print("  *** installed in run dir: " + runDir + " ***")
    print("  **************************************************")

    sys.exit(0)

########################################################################
# read release information file

def readReleaseInfoFile():

    global package
    global version
    global srcRelease

    package = "unknown"
    version = "unknown"
    srcRelease = "unknown"

    # open info file
    
    releaseInfoPath = "ReleaseInfo.txt"
    if (options.verbose):
        print >>sys.stderr, "==>> reading info file: ", releaseInfoPath
        
    info = open(releaseInfoPath, 'r')

    # read in lines

    lines = info.readlines()
    
    # close

    info.close()

    # decode lines

    if (len(lines) < 1):
        print >>sys.stderr, "ERROR reading info file: ", releaseInfoPath
        print >>sys.stderr, "  No contents"
        sys.exit(1)

    for line in lines:
        line = line.strip()
        toks = line.split(":")
        if (options.verbose):
            print >>sys.stderr, "  line: ", line
            print >>sys.stderr, "  toks: ", toks
        if (len(toks) == 2):
            if (toks[0] == "package"):
                package = toks[1]
            if (toks[0] == "version"):
                version = toks[1]
            if (toks[0] == "release"):
                srcRelease = toks[1]
        
    if (options.verbose):
        print >>sys.stderr, "==>> done reading info file: ", releaseInfoPath

########################################################################
# get the OS type

def getOsType():

    global ostype
    ostype = "x86_64"
    tmpFile = os.path.join("/tmp", "ostype." + timeStr + ".txt")

    shellCmd("uname -a > " + tmpFile)
    f = open(tmpFile, 'r')
    lines = f.readlines()
    f.close()

    if (len(lines) < 1):
        print >>sys.stderr, "ERROR getting OS type"
        print >>sys.stderr, "  'uname -a' call did not succeed"
        sys.exit(1)

    for line in lines:
        line = line.strip()
        if (options.verbose):
            print >>sys.stderr, "  line: ", line
        if (line.find("x86_64") > 0):
            ostype = "x86_64"
        elif (line.find("Darwin") > 0):
            ostype = "macosx_64"
        elif (line.find("i686") > 0):
            ostype = "i686"
            
########################################################################
# create the tmp dir

def createTmpDir():

    # check if exists already

    if (os.path.isdir(tmpDir)):

        if (options.force == False):
            print("\n===============================================")
            print("WARNING: you are about to remove all contents in dir:")
            print("    " + tmpDir)
            print("Contents:")
            contents = os.listdir(tmpDir)
            for filename in contents:
                print("  " + filename)
            answer = raw_input("Do you wish to proceed (y/n)? ")
            if (answer != "y"):
                print("  aborting ....")
                sys.exit(1)
                
        # remove it

        shutil.rmtree(tmpDir)

    # make it clean

    os.makedirs(tmpDir)

########################################################################
# copy in CIDD binaries if they exist

def copyCiddBinaries():

    ciddBinDir = "/tmp/cidd_prepare_bin_release_directory/bin"
    if (os.path.isdir(ciddBinDir)):
        if (options.debug):
            print >>sys.stderr, "Copying in CIDD binaries from: "
            print >>sys.stderr, "  " + ciddBinDir
        shellCmd("rsync -av " + ciddBinDir + " " + tmpDir)

########################################################################
# build netCDF

def buildNetcdf():

    netcdfDir = os.path.join(runDir, "lrose-netcdf")
    os.chdir(netcdfDir)
    if (package == "cidd"):
        shellCmd("./build_and_install_netcdf.m32 -x " + tmpDir)
    else:
        shellCmd("./build_and_install_netcdf -x " + tmpDir)

########################################################################
# build package

def buildPackage():

    os.chdir(runDir)
    if (options.installScripts):
        shellCmd("./build/build_lrose -s -x " + tmpDir + " -p " + package)
    else:
        shellCmd("./build/build_lrose -x " + tmpDir + " -p " + package)

########################################################################
# create the tar file

def createTarFile():

    # go to core dir, make tar dir

    os.chdir(coreDir)
    os.makedirs(tarDir)

    # move lrose contents into tar dir

    for fileName in [ "LICENSE.txt", "README.md" ]:
        os.rename(fileName, os.path.join(tarDir, fileName))

    for dirName in [ "build", "codebase", "docs", "release_notes" ]:
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

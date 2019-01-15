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
from sys import platform

def main():

    # globals

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global options
    global runDir
    global releaseDir
    global buildDir
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
                      help='Temporary directory for build')
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

    # set buildDir temporary staging area
    # this is set to a very long name because on macosx
    # we need to reset the library paths and we need to
    # ensure there is space available for the rename

    if (platform == "darwin"):
        if (options.prefix == "not-set"):
            buildDir = "/usr/local/lrose"
        else:
            buildDir = options.prefix
    else:
        if (options.prefix == "not-set"):
            buildDir = os.path.join("/tmp", 
                                    package + "_prepare_release_bin_directory")
        else:
            buildDir = options.prefix

    # set directories

    releaseDir = os.path.join(options.releaseTopDir, package)
    coreDir = os.path.join(buildDir, "lrose-core")
    codebaseDir = os.path.join(coreDir, "codebase")

    # compute release name and dir name
    
    releaseName = package + "-" + dateStr + ".bin." + ostype
    tarName = releaseName + ".tgz"
    tarDir = os.path.join(buildDir, releaseName)
    
    print("*****************************************************************", 
          file=sys.stderr)
    print("  Running " + thisScriptName, file=sys.stderr)
    print("", file=sys.stderr)
    print("  Preparing " + package + " binary release", file=sys.stderr)
    print("  OS type: " + ostype, file=sys.stderr)
    print("", file=sys.stderr)
    print("  NCAR, Boulder, CO, USA", file=sys.stderr)
    print("", file=sys.stderr)
    print("  " + dateTimeStr, file=sys.stderr)
    print("", file=sys.stderr)
    print("*****************************************************************",
          file=sys.stderr)
    print("  dateStr: ", dateStr, file=sys.stderr)
    print("  timeStr: ", timeStr, file=sys.stderr)
    print("  platform: ", platform, file=sys.stderr)
    print("  prefix: ", options.prefix, file=sys.stderr)
    print("  package: ", package, file=sys.stderr)
    print("  version: ", version, file=sys.stderr)
    print("  srcRelease: ", srcRelease, file=sys.stderr)
    print("  buildDir: ", buildDir, file=sys.stderr)
    print("  releaseName: ", releaseName, file=sys.stderr)
    print("  tarName: ", tarName, file=sys.stderr)
    print("  tarDir: ", tarDir, file=sys.stderr)
    print("  installScripts: ", options.installScripts, file=sys.stderr)
    print("*****************************************************************",
          file=sys.stderr)

    # create build dir for staging area

    createBuildDir()

    # For full LROSE package, copy in CIDD binaries if they are available

    if (package == "lrose"):
        copyCiddBinaries()

    # create the tar dir
        
    os.makedirs(tarDir)

    # build netcdf support

    if (platform != "darwin"):
        buildNetcdf()

    # build the package

    buildPackage()

    # detect which dynamic libs are needed
    # copy the dynamic libraries into runtime area:
    #     $prefix/bin/${package}_runtime_libs

    os.chdir(runDir)

    if (platform != "darwin"):
        shellCmd("./codebase/make_bin/installOriginLibFiles.py3 --binDir " + \
                 buildDir + "/bin " + \
                 "--relDir " + package + "_runtime_libs --debug")

    # copy the required files and directories into the tar directory
    
    os.chdir(runDir)
    shellCmd("rsync -av LICENSE.txt " + tarDir)
    shellCmd("rsync -av ReleaseInfo.txt " + tarDir)
    shellCmd("rsync -av release_notes " + tarDir)
    # shellCmd("rsync -av docs/README_INSTALL_BIN.txt " + tarDir)
    shellCmd("rsync -av ./build/install_bin_release.py3 " + tarDir)
    shellCmd("rsync -av " + buildDir + "/bin " + tarDir)
    shellCmd("rsync -av " + buildDir + "/lib " + tarDir)
    shellCmd("rsync -av " + buildDir + "/include " + tarDir)

    if (package == "cidd"):
        scriptDir = "./codebase/apps/cidd/src/CIDD/example_scripts"
        if (os.path.isdir(scriptDir)):
            shellCmd("rsync -av ./codebase/apps/cidd/src/CIDD/example_scripts " + tarDir)

    # make the tar file, copy into run dir

    os.chdir(buildDir)
    shellCmd("tar cvfz " + tarName + " " + releaseName)
    shellCmd("mv " + tarName + "  " + runDir)
    os.chdir(runDir)

    # copy into release dir if it exists

    if (os.path.isdir(releaseDir) == False):
        os.makedirs(releaseDir)

    if (os.path.isdir(releaseDir)):
        shellCmd("rsync -av " + tarName + "  " + releaseDir)

    # check the build
    
    os.chdir(runDir)
    print(("============= Checking libs for " + package + " ============="))
    shellCmd("./codebase/make_bin/check_libs.py3 " + \
             "--listPath ./build/checklists/libs_check_list." + package + " " + \
             "--libDir " + buildDir + "/lib " + \
             "--label " + package + " --maxAge 3600")
    print("====================================================")
    print(("============= Checking apps for " + package + " ============="))
    shellCmd("./codebase/make_bin/check_apps.py3 " + \
             "--listPath ./build/checklists/apps_check_list." + package + " " + \
             "--appDir " + buildDir + "/bin " + \
             "--label " + package + " --maxAge 3600")
    print("====================================================")
    
    #--------------------------------------------------------------------
    # done
    
    print("  **************************************************")
    print("  *** Done preparing binary release ***")
    print(("  *** binary tar file name: " + tarName + " ***"))
    print(("  *** installed in run dir: " + runDir + " ***"))
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
        print("==>> reading info file: ", releaseInfoPath, file=sys.stderr)
        
    info = open(releaseInfoPath, 'r')

    # read in lines

    lines = info.readlines()
    
    # close

    info.close()

    # decode lines

    if (len(lines) < 1):
        print("ERROR reading info file: ", releaseInfoPath, file=sys.stderr)
        print("  No contents", file=sys.stderr)
        sys.exit(1)

    for line in lines:
        line = line.strip()
        toks = line.split(":")
        if (options.verbose):
            print("  line: ", line, file=sys.stderr)
            print("  toks: ", toks, file=sys.stderr)
        if (len(toks) == 2):
            if (toks[0] == "package"):
                package = toks[1]
            if (toks[0] == "version"):
                version = toks[1]
            if (toks[0] == "release"):
                srcRelease = toks[1]
        
    if (options.verbose):
        print("==>> done reading info file: ", releaseInfoPath, file=sys.stderr)

########################################################################
# get the OS type

def getOsType():

    global ostype

    if (platform == "darwin"):
        ostype = "mac_osx"
        return

    ostype = "x86_64"
    tmpFile = os.path.join("/tmp", "ostype." + timeStr + ".txt")

    shellCmd("uname -a > " + tmpFile)
    f = open(tmpFile, 'r')
    lines = f.readlines()
    f.close()

    if (len(lines) < 1):
        print("ERROR getting OS type", file=sys.stderr)
        print("  'uname -a' call did not succeed", file=sys.stderr)
        sys.exit(1)

    for line in lines:
        line = line.strip()
        if (options.verbose):
            print("  line: ", line, file=sys.stderr)
        if (line.find("x86_64") > 0):
            ostype = "x86_64"
        elif (line.find("i686") > 0):
            ostype = "i686"
            
########################################################################
# create the build dir

def createBuildDir():

    # only do this for directories in /tmp

    if (("/tmp" in buildDir) == False):
        return

    # check if exists already

    if (os.path.isdir(buildDir)):

        if (options.force == False):
            print("\n===============================================")
            print("WARNING: you are about to remove all contents in dir:")
            print(("    " + buildDir))
            print("Contents:")
            contents = os.listdir(buildDir)
            for filename in contents:
                print(("  " + filename))
            answer = "n"
            if (sys.version_info > (3, 0)):
                answer = eval(input("Do you wish to proceed (y/n)? "))
            else:
                answer = input("Do you wish to proceed (y/n)? ")
            if (answer != "y"):
                print("  aborting ....")
                sys.exit(1)
                
        # remove it

        shutil.rmtree(buildDir)

    # make it clean

    os.makedirs(buildDir)

########################################################################
# copy in CIDD binaries if they exist

def copyCiddBinaries():

    ciddBinDir = "/tmp/cidd_prepare_bin_release_directory/bin"
    if (os.path.isdir(ciddBinDir)):
        if (options.debug):
            print("Copying in CIDD binaries from: ", file=sys.stderr)
            print("  " + ciddBinDir, file=sys.stderr)
        shellCmd("rsync -av " + ciddBinDir + " " + buildDir)

########################################################################
# build netCDF

def buildNetcdf():

    netcdfDir = os.path.join(runDir, "lrose-netcdf")
    os.chdir(netcdfDir)
    if (package == "cidd"):
        shellCmd("./build_and_install_netcdf.m32 -x " + buildDir)
    else:
        if (platform == "darwin"):
            shellCmd("./build_and_install_netcdf.osx -x " + buildDir)
        else:
            shellCmd("./build_and_install_netcdf -x " + buildDir)

########################################################################
# build package

def buildPackage():

    os.chdir(runDir)

    args = ""
    args = args + " --prefix " + buildDir
    args = args + " --package " + package
    if (options.installScripts):
        args = args + " --scripts "

    shellCmd("./build/build_lrose.py3 " + args)

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

    # for LINUX, move netcdf support into tar dir

    if (platform != "darwin"):

        netcdfDir = os.path.join(buildDir, "lrose-netcdf")
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

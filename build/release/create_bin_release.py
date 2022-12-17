#!/usr/bin/env python3

#===========================================================================
#
# Prepare an LROSE binary release
#
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
import platform

def main():

    # globals

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global thisScriptDir
    global runDir
    thisScriptDir = os.path.dirname(__file__)
    os.chdir(thisScriptDir)
    runDir = os.getcwd()

    global options
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

    global logPath
    global logFp

    global ostype
    global prefixDefault

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    prefixDefault = "/tmp/lrose-scratch"
    releaseDirDefault = os.path.join(homeDir, 'releases')
    logDirDefault = '/tmp/logs/create_bin_release'
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
                      dest='prefix', default=prefixDefault,
                      help='Temporary directory for build')
    parser.add_option('--releaseDir',
                      dest='releaseTopDir', default=releaseDirDefault,
                      help='Top-level release dir')
    parser.add_option('--runDir',
                      dest='runDir', default='.',
                      help='This script should either be run from the top level of the src tree, or you should specify this location')
    parser.add_option('--logDir',
                      dest='logDir', default=logDirDefault,
                      help='Logging dir')
    parser.add_option('--automake',
                      dest='automake', default=False,
                      action="store_true",
                      help='Use automake for the build. Default is cmake')
    parser.add_option('--force',
                      dest='force', default=False,
                      action="store_true",
                      help='force, do not request user to check if it is OK to proceed')
    parser.add_option('--scripts',
                      dest='installScripts', default=False,
                      action="store_true",
                      help='Install scripts as well as binaries')
    parser.add_option('--buildNetcdf',
                      dest='buildNetcdf', default=False,
                      action="store_true",
                      help='Build netcdf and hdf5 from source')

    (options, args) = parser.parse_args()

    if (options.verbose):
        options.debug = True
        
    debugStr = " "
    if (options.verbose):
        debugStr = " --verbose "
    elif (options.debug):
        debugStr = " --debug "

    if (options.runDir != "."):
        os.chdir(options.runDir)
        runDir = os.getcwd()

    # initialize logging

    if (os.path.isdir(options.logDir)):
        shutil.rmtree(options.logDir)
    os.makedirs(options.logDir)

    logPath = os.path.join(options.logDir, "no-logging");
    logFp = open(logPath, "w+")

    # runtime

    now = time.gmtime()
    nowTime = datetime(now.tm_year, now.tm_mon, now.tm_mday,
                       now.tm_hour, now.tm_min, now.tm_sec)
    dateStr = nowTime.strftime("%Y%m%d")
    timeStr = nowTime.strftime("%Y%m%d%H%M%S")
    dateTimeStr = nowTime.strftime("%Y/%m/%d-%H:%M:%S")

    # read in release info

    readReleaseInfoFile()
    if (package == "lrose-cidd"):
        options.buildNetcdf = True

    # get the OS type - x86_64, i686, macosx_64
    
    if (platform.system() == "Darwin"):
        ostype = "mac_osx"
    else:
        ostype = platform.machine()

    # set buildDir temporary staging area
    # this is set to a very long name because on macosx
    # we need to reset the library paths and we need to
    # ensure there is space available for the rename

    if (sys.platform == "darwin"):
        if (options.prefix == prefixDefault):
            buildDir = "/usr/local/lrose"
        else:
            buildDir = options.prefix
    else:
        if (options.prefix == prefixDefault):
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
    print("  platform: ", sys.platform, file=sys.stderr)
    print("  prefix: ", options.prefix, file=sys.stderr)
    print("  package: ", package, file=sys.stderr)
    print("  version: ", version, file=sys.stderr)
    print("  srcRelease: ", srcRelease, file=sys.stderr)
    print("  codebaseDir: ", codebaseDir, file=sys.stderr)
    print("  buildDir: ", buildDir, file=sys.stderr)
    print("  logDir: ", options.logDir, file=sys.stderr)
    print("  releaseName: ", releaseName, file=sys.stderr)
    print("  tarName: ", tarName, file=sys.stderr)
    print("  tarDir: ", tarDir, file=sys.stderr)
    print("  buildNetcdf: ", options.buildNetcdf, file=sys.stderr)
    print("  installScripts: ", options.installScripts, file=sys.stderr)
    print("*****************************************************************",
          file=sys.stderr)

    # create build dir for staging area

    createBuildDir()

    # For full LROSE package, copy in CIDD binaries if they are available

    if (package == "lrose-core"):
        logPath = prepareLogFile("copy-cidd-binaries");
        copyCiddBinaries()

    # create the tar dir
        
    os.makedirs(tarDir)

    # build netcdf support

    if (options.buildNetcdf):
        logPath = prepareLogFile("build-netcdf");
        buildNetcdf()

    # build the package

    logPath = prepareLogFile("build-package");
    if (options.automake):
        buildPackageAutomake()
    else:
        buildPackageCmake()

    # detect which dynamic libs are needed
    # copy the dynamic libraries into runtime area:
    #     $prefix/bin/${package}_runtime_libs

    os.chdir(runDir)

    if (sys.platform != "darwin"):
        shellCmd("./build/scripts/installOriginLibFiles.py --binDir " + \
                 buildDir + "/bin " + \
                 "--relDir " + package + "_runtime_libs --debug")

    # copy the required files and directories into the tar directory
    
    os.chdir(runDir)
    shellCmd("rsync -av LICENSE.txt " + tarDir)
    shellCmd("rsync -av ReleaseInfo.txt " + tarDir)
    shellCmd("rsync -av release_notes " + tarDir)
    shellCmd("rsync -av ./build/release/install_bin_release.py " + tarDir)
    shellCmd("rsync -av ./build/release/install_cidd_bin_release.py " + tarDir)
    shellCmd("rsync -av " + buildDir + "/bin " + tarDir)
    shellCmd("rsync -av " + buildDir + "/lib " + tarDir)
    shellCmd("rsync -av " + buildDir + "/include " + tarDir)

    if (package == "lrose-cidd"):
        scriptDir = "./codebase/apps/cidd/src/CIDD/scripts"
        if (os.path.isdir(scriptDir)):
            shellCmd("rsync -av ./codebase/apps/cidd/src/CIDD/scripts " + tarDir)

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
    
    logPath = prepareLogFile("no-logging");
    os.chdir(runDir)
    print(("============= Checking libs for " + package + " ============="))
    shellCmd("./build/scripts/checkLibs.py" + \
             " --prefix " + options.prefix + \
             " --package " + package + \
             " --maxAge 3600")
    print("====================================================")
    print(("============= Checking apps for " + package + " ============="))
    shellCmd("./build/scripts/checkApps.py" + \
             " --prefix " + options.prefix + \
             " --package " + package + \
             " --maxAge 3600")
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
                answer = input("WARNING: do you wish to proceed (y/n)? ")
            else:
                answer = raw_input("WARNING: do you wish to proceed (y/n)? ")
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
    if (package == "lrose-cidd"):
        shellCmd("./build_and_install_netcdf.cidd_linux32 -x " + buildDir)
    else:
        if (sys.platform == "darwin"):
            shellCmd("./build_and_install_netcdf.osx -x " + buildDir)
        else:
            shellCmd("./build_and_install_netcdf -x " + buildDir)

########################################################################
# build package using automake

def buildPackageAutomake():

    global logPath

    # set the environment

    runtimeLibRelDir = package + "_runtime_libs"

    os.environ["LDFLAGS"] = "-L" + buildDir + "/lib " + \
                            "-Wl,--enable-new-dtags," + \
                            "-rpath," + \
                            "'$$ORIGIN/" + runtimeLibRelDir + \
                            ":$$ORIGIN/../lib" + \
                            ":" + buildDir + "/lib'"

    os.environ["FC"] = "gfortran"
    os.environ["F77"] = "gfortran"
    os.environ["F90"] = "gfortran"

    if (sys.platform == "darwin"):
        os.environ["PKG_CONFIG_PATH"] = "/usr/local/opt/qt/lib/pkgconfig"
    else:
        os.environ["CXXFLAGS"] = " -std=c++11 "

    # print out environment

    logPath = prepareLogFile("print-environment");
    cmd = "env"
    shellCmd(cmd)

    # the build is done relative to the current dir

    baseDir = os.path.join(runDir, "codebase")
    os.chdir(baseDir)

    # run configure

    logPath = prepareLogFile("run-configure");
    if (options.buildNetcdf):
        cmd = "./configure --with-hdf5=" + buildDir + \
              " --with-netcdf=" + buildDir + \
                                " --prefix=" + buildDir
    else:
        cmd = "./configure --prefix=" + buildDir
    shellCmd(cmd)

    # build the libraries

    logPath = prepareLogFile("build-libs");
    os.chdir(os.path.join(baseDir, "libs"))
    cmd = "make -k -j 8"
    shellCmd(cmd)

    # install the libraries

    logPath = prepareLogFile("install-libs");
    cmd = "make -k install-strip"
    shellCmd(cmd)

    # build the apps

    logPath = prepareLogFile("build-apps");
    os.chdir(os.path.join(baseDir, "apps"))
    cmd = "make -k -j 8"
    shellCmd(cmd)

    # install the apps

    logPath = prepareLogFile("install-apps");
    cmd = "make -k install-strip"
    shellCmd(cmd)

    # optionally install the scripts

    if (options.installScripts):
        installScripts()

########################################################################
# build package using cmake

def buildPackageCmake():

    global logPath

    # For Centos 7, use cmake3

    getOSType()
    cmakeCmd = "cmake"
    if (osId == "centos" and osVersion == "7"):
        cmakeCmd = "cmake3"

    # set the environment

    runtimeLibRelDir = package + "_runtime_libs"

    # print out environment

    logPath = prepareLogFile("print-environment");
    cmd = "env"
    shellCmd(cmd)

    # the build is done relative to the current dir

    baseDir = os.path.join(runDir, "codebase")
    cmakeBuildDir = os.path.join(baseDir, "build")
    cmd = "/bin/rm -rf " + cmakeBuildDir
    shellCmd(cmd)
    os.makedirs(cmakeBuildDir)
    os.chdir(cmakeBuildDir)

    # run cmake

    logPath = prepareLogFile("run-cmake")
    cmd = cmakeCmd + " -DCMAKE_INSTALL_PREFIX=" + buildDir + " .."
    shellCmd(cmd)

    # build and install

    logPath = prepareLogFile("build-libs");

    libsBuildDir = os.path.join(runDir, "codebase/build/libs")
    os.chdir(libsBuildDir)
    cmd = "make -j 8 install"
    shellCmd(cmd)

    logPath = prepareLogFile("build-tdrp_gen");

    tdrpGenBuildDir = os.path.join(runDir, "codebase/build/apps/tdrp/src/tdrp_gen")
    os.chdir(tdrpGenBuildDir)
    cmd = "make -j 8 install"
    shellCmd(cmd)

    logPath = prepareLogFile("build-apps");

    appsBuildDir = os.path.join(runDir, "codebase/build/apps")
    os.chdir(appsBuildDir)
    cmd = "make -j 8 install"
    shellCmd(cmd)

    # optionally install the scripts

    if (options.installScripts):
        installScripts()

########################################################################
# install the scripts

def installScripts():

    logPath = prepareLogFile("install-scripts");

    baseDir = os.path.join(runDir, "codebase")
    os.chdir(baseDir)

    # general
    
    installDir = os.path.join(buildDir, "bin")
    scriptsDir = os.path.join(baseDir, "apps/scripts/src")
    if (os.path.isdir(scriptsDir)):
        os.chdir(scriptsDir)
        shellCmd("./install_scripts.lrose " + installDir)

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

    if (sys.platform != "darwin"):

        netcdfDir = os.path.join(buildDir, "lrose-netcdf")
        netcdfSubDir = os.path.join(tarDir, "lrose-netcdf")
        os.makedirs(netcdfSubDir)
    
        for name in [ "README.md", "build_and_install_netcdf", "tar_files" ]:
            os.rename(os.path.join(netcdfDir, name),
                      os.path.join(netcdfSubDir, name))

    # create the tar file

    shellCmd("tar cvfz " + tarName + " " + releaseName)
    
########################################################################
# prepare log file

def prepareLogFile(logFileName):

    global logFp

    logFp.close()
    logPath = os.path.join(options.logDir, logFileName + ".log");
    if (logPath.find('no-logging') >= 0):
        return logPath
    print("========================= " + logFileName + " =========================", file=sys.stderr)
    if (options.verbose):
        print("====>> Creating log file: " + logPath + " <<==", file=sys.stderr)
    logFp = open(logPath, "w+")
    logFp.write("===========================================\n")
    logFp.write("Log file from script: " + thisScriptName + "\n")

    return logPath

########################################################################
# get the OS type from the /etc/os-release file in linux

def getOSType():

    global osId, osVersion
    osId = "unknown"
    osVersion = "unknown"

    if sys.platform == "darwin":
        osId = "darwin"
        return

    if (os.path.exists("/etc/os-release") == False):
        return

    osrelease_file = open("/etc/os-release", "rt")
    lines = osrelease_file.readlines()
    osrelease_file.close()
    for line in lines:
        if (line.find('ID=') == 0):
            osId = line.split('=')[1].replace('"', '').strip()
        elif (line.find('VERSION_ID=') == 0):
            osVersion = line.split('=')[1].replace('"', '').strip()

########################################################################
# Run a command in a shell, wait for it to complete

def shellCmd(cmd):

    print("Running cmd:", cmd, file=sys.stderr)
    
    if (logPath.find('no-logging') >= 0):
        cmdToRun = cmd
    else:
        print("Log file is:", logPath, file=sys.stderr)
        print("    ....", file=sys.stderr)
        cmdToRun = cmd + " 1>> " + logPath + " 2>&1"

    try:
        retcode = subprocess.check_call(cmdToRun, shell=True)
        if retcode != 0:
            print("Child exited with code: ", retcode, file=sys.stderr)
            sys.exit(1)
        else:
            if (options.verbose):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, file=sys.stderr)
        sys.exit(1)

    print("    done", file=sys.stderr)
    
########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

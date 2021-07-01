#!/usr/bin/env python

#===========================================================================
#
# Checkout and build LROSE, using cmake.
#
# This script performs the following steps:
#
#   1. clone lrose-core from git
#   2. run createCMakeLists.py script to generate CMakeLists.txt files
#   3. run cmake to create Makefiles
#   4. perform the build and install
#   5. check the build
#
# You can optionally specify a release date.
#
# Use --help to see the command line options.
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
from sys import platform

def main():

    # globals

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global thisScriptDir
    thisScriptDir = os.path.dirname(__file__)

    global options
    global package

    global releaseName
    global releaseTag
    global releaseDate

    global netcdfDir
    global displaysDir

    global coreDir
    global codebaseDir
    global runtimeLibRelDir

    global prefixDir
    global prefixBinDir
    global prefixLibDir
    global prefixIncludeDir
    global prefixShareDir

    global dateStr
    global logPath
    global logFp

    # parse the command line

    usage = "usage: " + thisScriptName + " [options]"
    homeDir = os.environ['HOME']
    prefixDirDefault = os.path.join(homeDir, 'lrose')
    buildDirDefault = '/tmp/lrose-build'
    logDirDefault = '/tmp/lrose-build/logs'
    parser = OptionParser(usage)
    parser.add_option('--clean',
                      dest='clean', default=False,
                      action="store_true",
                      help='Cleanup tmp build dir')
    parser.add_option('--debug',
                      dest='debug', default=True,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--verbose',
                      dest='verbose', default=False,
                      action="store_true",
                      help='Set verbose debugging on')
    parser.add_option('--package',
                      dest='package', default='lrose-core',
                      help='Package name. Options are: ' + \
                      'lrose-core (default), lrose-radx, lrose-cidd, apar, samurai')
    parser.add_option('--releaseDate',
                      dest='releaseDate', default='latest',
                      help='Date from which to compute tag for git clone. Applies if --tag is not used.')
    parser.add_option('--tag',
                      dest='tag', default='master',
                      help='Tag to check out lrose. Overrides --releaseDate')
    parser.add_option('--prefix',
                      dest='prefix', default=prefixDirDefault,
                      help='Install directory, default: ' + prefixDirDefault)
    parser.add_option('--buildDir',
                      dest='buildDir', default=buildDirDefault,
                      help='Temporary build dir, default: ' + buildDirDefault)
    parser.add_option('--logDir',
                      dest='logDir', default=logDirDefault,
                      help='Logging dir, default: ' + logDirDefault)
    parser.add_option('--static',
                      dest='static', default=False,
                      action="store_true",
                      help='use static linking, default is dynamic')
    parser.add_option('--installAllRuntimeLibs',
                      dest='installAllRuntimeLibs', default=False,
                      action="store_true",
                      help=\
                      'Install dynamic runtime libraries for all binaries, ' + \
                      'in a directory relative to the bin dir. ' + \
                      'System libraries are included.')
    parser.add_option('--installLroseRuntimeLibs',
                      dest='installLroseRuntimeLibs', default=False,
                      action="store_true",
                      help=\
                      'Install dynamic runtime lrose libraries for all binaries, ' + \
                      'in a directory relative to the bin dir. ' + \
                      'System libraries are not included.')
    parser.add_option('--buildNetcdf',
                      dest='buildNetcdf', default=False,
                      action="store_true",
                      help='Build netcdf and hdf5 from source')
    parser.add_option('--fractl',
                      dest='build_fractl', default=False,
                      action="store_true",
                      help='Checkout and build fractl after core build is complete')
    parser.add_option('--vortrac',
                      dest='build_vortrac', default=False,
                      action="store_true",
                      help='Checkout and build vortrac after core build is complete')
    parser.add_option('--samurai',
                      dest='build_samurai', default=False,
                      action="store_true",
                      help='Checkout and build samurai after core build is complete')
    parser.add_option('--cmake3',
                      dest='use_cmake3', default=False,
                      action="store_true",
                      help='Use cmake3 instead of cmake')
    parser.add_option('--noApps',
                      dest='noApps', default=False,
                      action="store_true",
                      help='Do not build the lrose core apps')
    parser.add_option('--withJasper',
                      dest='withJasper', default=False,
                      action="store_true",
                      help='Set if jasper library is installed. This provides support for jpeg compression in grib files.')
    parser.add_option('--verboseMake',
                      dest='verboseMake', default=False,
                      action="store_true",
                      help='Verbose output for make, default is summary')

    (options, args) = parser.parse_args()
    
    if (options.verbose):
        options.debug = True

    # check package name

    if (options.package != "lrose-core" and
        options.package != "lrose-radx" and
        options.package != "lrose-cidd" and
        options.package != "apar" and
        options.package != "samurai") :
        print("ERROR: invalid package name: %s:" % options.package, file=sys.stderr)
        print("  options: lrose-core, lrose-radx, lrose-cidd, samurai",
              file=sys.stderr)
        sys.exit(1)

    # For Centos 7, use cmake3

    getOSType()
    if (osId == "centos" and osVersion == "7"):
        options.use_cmake3 = True

    # cmake version

    global cmakeExec
    cmakeExec = 'cmake'
    if (options.use_cmake3):
        cmakeExec = 'cmake3'
    
    # for CIDD, set to static linkage
    if (options.package == "lrose-cidd"):
        options.static = True
        
    package = options.package
    runtimeLibRelDir = package + "_runtime_libs"

    # runtime

    now = time.gmtime()
    nowTime = datetime(now.tm_year, now.tm_mon, now.tm_mday,
                       now.tm_hour, now.tm_min, now.tm_sec)
    dateStr = nowTime.strftime("%Y%m%d")

    # set release tag

    if (options.tag != "master"):
        releaseTag = options.tag
        releaseName = options.tag
        releaseDate = "not-set"
    elif (options.releaseDate == "latest"):
        releaseDate = datetime(int(dateStr[0:4]),
                               int(dateStr[4:6]),
                               int(dateStr[6:8]))
        releaseTag = "master"
        releaseName = options.package + "-" + dateStr
    else:
        # check we have a good release date
        releaseDate = datetime(int(options.releaseDate[0:4]),
                               int(options.releaseDate[4:6]),
                               int(options.releaseDate[6:8]))
        releaseTag = options.package + "-" + options.releaseDate[0:8]
        releaseName = releaseTag

    # set directories
    
    coreDir = os.path.join(options.buildDir, "lrose-core")
    displaysDir = os.path.join(options.buildDir, "lrose-displays")
    netcdfDir = os.path.join(options.buildDir, "lrose-netcdf")
    codebaseDir = os.path.join(coreDir, "codebase")

    prefixDir = options.prefix
    prefixBinDir = os.path.join(prefixDir, 'bin')
    prefixLibDir = os.path.join(prefixDir, 'lib')
    prefixIncludeDir = os.path.join(prefixDir, 'include')
    prefixShareDir = os.path.join(prefixDir, 'share')

    # debug print

    if (options.debug):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  osId: ", osId, file=sys.stderr)
        print("  osVersion: ", osVersion, file=sys.stderr)
        print("  package: ", package, file=sys.stderr)
        print("  releaseDate: ", releaseDate, file=sys.stderr)
        print("  releaseName: ", releaseName, file=sys.stderr)
        print("  releaseTag: ", releaseTag, file=sys.stderr)
        print("  static: ", options.static, file=sys.stderr)
        print("  buildDir: ", options.buildDir, file=sys.stderr)
        print("  logDir: ", options.logDir, file=sys.stderr)
        print("  coreDir: ", coreDir, file=sys.stderr)
        print("  codebaseDir: ", codebaseDir, file=sys.stderr)
        print("  displaysDir: ", displaysDir, file=sys.stderr)
        print("  netcdfDir: ", netcdfDir, file=sys.stderr)
        print("  prefixDir: ", prefixDir, file=sys.stderr)
        print("  prefixBinDir: ", prefixBinDir, file=sys.stderr)
        print("  prefixLibDir: ", prefixLibDir, file=sys.stderr)
        print("  prefixIncludeDir: ", prefixIncludeDir, file=sys.stderr)
        print("  prefixShareDir: ", prefixShareDir, file=sys.stderr)
        print("  buildNetcdf: ", options.buildNetcdf, file=sys.stderr)
        print("  use_cmake3: ", options.use_cmake3, file=sys.stderr)
        print("  cmakeExec: ", cmakeExec, file=sys.stderr)
        print("  build_fractl: ", options.build_fractl, file=sys.stderr)
        print("  build_vortrac: ", options.build_vortrac, file=sys.stderr)
        print("  build_samurai: ", options.build_samurai, file=sys.stderr)
        print("  noApps: ", options.noApps, file=sys.stderr)

    # create build dir
    
    createBuildDir()

    # initialize logging

    if (os.path.isdir(options.logDir) == False):
        os.makedirs(options.logDir)
    logPath = os.path.join(options.logDir, "initialize");
    logFp = open(logPath, "w+")
    
    # make dirs

    try:
        os.makedirs(prefixDir)
        os.makedirs(prefixBinDir)
        os.makedirs(prefixLibDir)
        os.makedirs(prefixIncludeDir)
        os.makedirs(options.logDir)
    except:
        print("  note - dirs already exist", file=sys.stderr)

    # get repos from git

    logPath = prepareLogFile("git-checkout");
    gitCheckout()

    # install the distribution-specific makefiles

    logPath = prepareLogFile("install-package-makefiles");
    os.chdir(codebaseDir)
    scriptPath = "../build/scripts/installPackageMakefiles.py"
    shellCmd(scriptPath + " --debug --package " + package)

    # trim libs and apps to those required by distribution makefiles

    trimToMakefiles("libs")
    trimToMakefiles("apps")

    # create the CMakeLists files

    logPath = prepareLogFile("create-CMakeLists-files");
    createCMakeLists()

    # create the release information file
    
    createReleaseInfoFile()

    # prune any empty directories

    prune(codebaseDir)

    # build netcdf support
    
    if (options.buildNetcdf):
        logPath = prepareLogFile("build-netcdf");
        buildNetcdf()

    # build the package

    buildPackage()

    # detect which dynamic libs are needed
    # copy the dynamic libraries into a directory relative
    # to the binary install dir:
    #     bin/${package}_runtime_libs

    os.chdir(codebaseDir)
    if (options.installAllRuntimeLibs):
        scriptPath = "../build/scripts/installOriginLibFiles.py"
        cmd = scriptPath + \
              " --binDir " + prefixBinDir + \
              " --relDir " + runtimeLibRelDir
        if (options.verbose):
            cmd = cmd + " --verbose"
        elif (options.debug):
            cmd = cmd + " --debug"
        shellCmd(cmd)
    elif (options.installLroseRuntimeLibs):
        scriptPath = "../build/scripts/installOriginLroseLibs.py"
        cmd = scriptPath + \
              " --binDir " + prefixBinDir + \
              " --libDir " + prefixLibDir + \
              " --relDir " + runtimeLibRelDir
        if (options.verbose):
            cmd = cmd + " --verbose"
        elif (options.debug):
            cmd = cmd + " --debug"
        shellCmd(cmd)

    # perform the install

    logPath = prepareLogFile("do-final-install");
    doFinalInstall();

    # check the install

    logPath = prepareLogFile("no-logging");
    checkInstall()

    # build CSU packages

    if (options.build_fractl):
        logPath = prepareLogFile("build-fractl");
        buildFractl()

    if (options.build_vortrac):
        logPath = prepareLogFile("build-vortrac");
        buildVortrac()

    if (options.build_samurai):
        logPath = prepareLogFile("build-samurai");
        buildSamurai()

    # delete the tmp dir

    if (options.clean):
        shutil.rmtree(options.buildDir)

    logFp.close()
    sys.exit(0)

########################################################################
# create the build dir

def createBuildDir():

    # check if exists already

    if (os.path.isdir(options.buildDir)):

        print("WARNING: you are about to remove all contents in dir: " + 
              options.buildDir)
        print("===============================================")
        contents = os.listdir(options.buildDir)
        for filename in contents:
            print(("  " + filename))
        print("===============================================")
        answer = "n"
        if (sys.version_info > (3, 0)):
            answer = input("WARNING: do you wish to proceed (y/n)? ")
        else:
            answer = raw_input("WARNING: do you wish to proceed (y/n)? ")
        if (answer != "y"):
            print("  aborting ....")
            sys.exit(1)
                
        # remove it

        shutil.rmtree(options.buildDir)

    # make it clean
    
    print(("INFO: you are about to create build dir: " + 
          options.buildDir))
    
    os.makedirs(options.buildDir)

########################################################################
# check out repos from git

def gitCheckout():

    os.chdir(options.buildDir)

    # lrose core

    shellCmd("/bin/rm -rf lrose-core")
    if (options.tag == "master"):
        shellCmd("git clone https://github.com/NCAR/lrose-core")
    else:
        shellCmd("git clone --branch " + releaseTag + \
                 " https://github.com/NCAR/lrose-core")

    # netcdf and hdf5

    if (options.buildNetcdf):
        shellCmd("/bin/rm -rf lrose-netcdf")
        shellCmd("git clone https://github.com/NCAR/lrose-netcdf")

    # color scales and maps in displays repo

    if (options.package != "samurai") :
        shellCmd("/bin/rm -rf lrose-displays")
        shellCmd("git clone https://github.com/NCAR/lrose-displays")

########################################################################
# create CMakeLists files

def createCMakeLists():

    os.chdir(codebaseDir)

    staticStr = " "
    if (options.static):
        staticStr = " --static "

    m32Str = " "
    if (options.package == "lrose-cidd"):
        m32Str = " --m32 "

    withJasperStr = " "
    if (options.withJasper):
        withJasperStr = " --withJasper "
    
    verboseMakeStr = " "
    if (options.verboseMake):
        verboseMakeStr = " --verboseMake "
    
    debugStr = " "
    if (options.verbose):
        debugStr = " --verbose "
    elif (options.debug):
        debugStr = " --debug "

    dependDirsStr = ""
    if (options.buildNetcdf):
        dependDirsStr = " --dependDirs " + prefixDir + " "

    shellCmd("../build/cmake/createCMakeLists.py " +
             debugStr + staticStr + verboseMakeStr +
             withJasperStr + dependDirsStr + m32Str +
             " --prefix " + prefixDir)

########################################################################
# write release information file

def createReleaseInfoFile():

    # go to core dir

    os.chdir(prefixDir)

    # open info file

    releaseInfoPath = os.path.join(coreDir, "ReleaseInfo.txt")
    info = open(releaseInfoPath, 'w')

    # write release info

    info.write("package:" + package + "\n")
    info.write("version:" + dateStr + "\n")
    info.write("release:" + releaseName + "\n")

    # close

    info.close()

########################################################################
# get string value based on search key
# the string may span multiple lines
#
# Example of keys: SRCS, SUB_DIRS, MODULE_NAME, TARGET_FILE
#
# value is returned

def getValueListForKey(path, key):

    valueList = []

    try:
        fp = open(path, 'r')
    except IOError as e:
        if (options.verbose):
            print("ERROR - ", thisScriptName, file=sys.stderr)
            print("  Cannot open file:", path, file=sys.stderr)
        return valueList

    lines = fp.readlines()
    fp.close()

    foundKey = False
    multiLine = ""
    for line in lines:
        if (foundKey == False):
            if (line[0] == '#'):
                continue
        if (line.find(key) >= 0):
            foundKey = True
            multiLine = multiLine + line
            if (line.find("\\") < 0):
                break;
        elif (foundKey):
            if (line[0] == '#'):
                break
            if (len(line) < 2):
                break
            multiLine = multiLine + line;
            if (line.find("\\") < 0):
                break;

    if (foundKey == False):
        return valueList

    multiLine = multiLine.replace(key, " ")
    multiLine = multiLine.replace("=", " ")
    multiLine = multiLine.replace("\t", " ")
    multiLine = multiLine.replace("\\", " ")
    multiLine = multiLine.replace("\r", " ")
    multiLine = multiLine.replace("\n", " ")

    toks = multiLine.split(' ')
    for tok in toks:
        if (len(tok) > 0):
            valueList.append(tok)

    return valueList

########################################################################
# Trim libs and apps to those required by distribution

def trimToMakefiles(subDir):

    if (options.verbose):
        print("Trimming unneeded dirs, subDir: " + subDir, file=logFp)

    # get list of subdirs in makefile

    dirPath = os.path.join(codebaseDir, subDir)
    os.chdir(dirPath)

    # need to allow upper and lower case Makefile (makefile or Makefile)
    subNameList = getValueListForKey("makefile", "SUB_DIRS")
    if not subNameList:
        if (options.verbose):
            print("Trying uppercase Makefile ... ", file=logFp)
        subNameList = getValueListForKey("Makefile", "SUB_DIRS")
    
    for subName in subNameList:
        if (os.path.isdir(subName)):
            if (options.verbose):
                print("  need sub dir: " + subName, file=logFp)
            
    # get list of files in subDir

    entries = os.listdir(dirPath)
    for entry in entries:
        theName = os.path.join(dirPath, entry)
        if (options.verbose):
            print("considering: " + theName, file=logFp)
        if (entry == "perl5") or (entry == "scripts") or (entry == "include"):
            # always keep scripts directories
            continue
        if (entry == "images") or (entry == "resources"):
            # always keep QT resources
            continue
        if (os.path.isdir(theName)):
            if (entry not in subNameList):
                if (options.verbose):
                    print("discarding it", file=logFp)
                shutil.rmtree(theName)
            else:
                if (options.verbose):
                    print("keeping it and recursing", file=logFp)
                # check this child's required subdirectories (recurse)
                trimToMakefiles(os.path.join(subDir, entry))

########################################################################
# build netCDF

def buildNetcdf():

    os.chdir(netcdfDir)
    if (package == "lrose-cidd"):
        shellCmd("./build_and_install_netcdf.cidd_linux32 -x " + prefixDir)
    else:
        if sys.platform == "darwin":
            shellCmd("./build_and_install_netcdf.osx -x " + prefixDir)
        else:
            shellCmd("./build_and_install_netcdf -x " + prefixDir)

########################################################################
# build package

def buildPackage():

    global logPath

    # set the environment

    os.environ["LDFLAGS"] = "-L" + prefixLibDir + " " + \
                            "-Wl,--enable-new-dtags," + \
                            "-rpath," + \
                            "'$$ORIGIN/" + runtimeLibRelDir + \
                            ":$$ORIGIN/../lib" + \
                            ":" + prefixLibDir + \
                            ":" + prefixLibDir + "'"

    if (sys.platform == "darwin"):
        os.environ["PKG_CONFIG_PATH"] = "/usr/local/opt/qt/lib/pkgconfig"

    # print out environment

    logPath = prepareLogFile("print-environment");
    cmd = "env"
    shellCmd(cmd)

    # run cmake in build dir, as a subdir of codebase
    
    logPath = prepareLogFile("run-cmake");
    cmakeBuildDir = os.path.join(codebaseDir, "build")
    os.makedirs(cmakeBuildDir)
    os.chdir(cmakeBuildDir)
    cmd = cmakeExec + " .."
    shellCmd(cmd)
    
    # build the libraries

    logPath = prepareLogFile("build-libs");
    os.chdir(os.path.join(cmakeBuildDir, "libs"))
    cmd = "make -j 8"
    shellCmd(cmd)

    # install the libraries

    logPath = prepareLogFile("install-libs");

    cmd = "make -j 8 install/strip"
    shellCmd(cmd)

    if (options.noApps == False):

        # build and install tdrp_gen

        logPath = prepareLogFile("build-tdrp-gen");
        os.chdir(os.path.join(cmakeBuildDir, "apps/tdrp/src/tdrp_gen"))
        cmd = "make install/strip"
        shellCmd(cmd)
        
        # build the apps

        logPath = prepareLogFile("build-apps");
        os.chdir(os.path.join(cmakeBuildDir, "apps"))
        cmd = "make -j 8"
        shellCmd(cmd)
        
        # install the apps
        
        logPath = prepareLogFile("install-apps");
        cmd = "make -j 8 install/strip"
        shellCmd(cmd)

########################################################################
# perform final install

def doFinalInstall():

    # install docs etc
    
    os.chdir(coreDir)

    shellCmd("rsync -av LICENSE.txt " + prefixDir)
    shellCmd("rsync -av release_notes " + prefixDir)
    shellCmd("rsync -av docs " + prefixDir)

    if (package == "lrose-cidd"):
        shellCmd("rsync -av ./codebase/apps/cidd/src/CIDD/scripts " +
                 prefixDir)

    # install color scales

    if (os.path.isdir(displaysDir)):
        os.chdir(displaysDir)
        shellCmd("rsync -av color_scales " + prefixShareDir)

########################################################################
# check the install

def checkInstall():

    os.chdir(coreDir)
    print(("============= Checking libs for " + package + " ============="))
    shellCmd("./build/scripts/checkLibs.py" + \
             " --prefix " + prefixDir + \
             " --package " + package)
    print("====================================================")

    if (options.noApps == False):
        print(("============= Checking apps for " + package + " ============="))
        shellCmd("./build/scripts/checkApps.py" + \
                 " --prefix " + prefixDir + \
                 " --package " + package)
        print("====================================================")
    
    print("**************************************************")
    print("*** Done building auto release *******************")
    print(("*** Installed in dir: " + prefixDir + " ***"))
    print("**************************************************")

########################################################################
# prune empty dirs

def prune(tree):

    # walk the tree
    if (os.path.isdir(tree)):
        contents = os.listdir(tree)

        if (len(contents) == 0):
            if (options.verbose):
                print("pruning empty dir: " + tree, file=logFp)
            shutil.rmtree(tree)
        else:
            for l in contents:
                # remove CVS directories
                if (l == "CVS") or (l == ".git"): 
                    thepath = os.path.join(tree,l)
                    if (options.verbose):
                        print("pruning dir: " + thepath, file=logFp)
                    shutil.rmtree(thepath)
                else:
                    thepath = os.path.join(tree,l)
                    if (os.path.isdir(thepath)):
                        prune(thepath)
            # check if this tree is now empty
            newcontents = os.listdir(tree)
            if (len(newcontents) == 0):
                if (options.verbose):
                    print("pruning empty dir: " + tree, file=logFp)
                shutil.rmtree(tree)

########################################################################
# build fractl package

def buildFractl():

    global logPath

    print("==>> buildFractl", file=sys.stderr)
    print("====>> prefixDir: ", prefixDir, file=sys.stderr)
    
    # set the environment

    os.environ["LROSE_INSTALL_DIR"] = prefixDir
    
    # check out fractl

    os.chdir(options.buildDir)
    shellCmd("/bin/rm -rf fractl")
    shellCmd("git clone https://github.com/mmbell/fractl")

    # run cmake to create makefiles

    fractlDir = os.path.join(options.buildDir, "fractl");
    cmakeBuildDir = os.path.join(fractlDir, "build")
    os.makedirs(cmakeBuildDir)
    os.chdir(cmakeBuildDir)
    
    cmd = cmakeExec + " .."
    shellCmd(cmd)
    
    # do the build and install

    cmd = "make -k -j 8 install/strip"
    shellCmd(cmd)

    return

########################################################################
# build vortrac package

def buildVortrac():

    global logPath

    print("====>> buildVortrac", file=sys.stderr)
    print("====>> prefixDir: ", prefixDir, file=sys.stderr)

    # set the environment

    os.environ["LROSE_INSTALL_DIR"] = prefixDir

    # check out vortrac

    os.chdir(options.buildDir)
    shellCmd("/bin/rm -rf vortrac")
    shellCmd("git clone https://github.com/mmbell/vortrac")

    # run cmake to create makefiles

    vortracDir = os.path.join(options.buildDir, "vortrac");
    cmakeBuildDir = os.path.join(vortracDir, "build")
    os.makedirs(cmakeBuildDir)
    os.chdir(cmakeBuildDir)
    
    # run cmake to create makefiles - in-source build
    
    cmd = cmakeExec + " .."
    shellCmd(cmd)
    
    # do the build and install
    
    cmd = "make -k -j 8 install/strip"
    shellCmd(cmd)
    
    # install resources
    
    os.chdir(vortracDir)

    if (sys.platform == "darwin"):
        os.makedirs("vortrac.app/Contents/Resources")
        cmd = "rsync -av Resources/*.xml vortrac.app/Contents/Resources"
        shellCmd(cmd)

    cmd = "rsync -av Resources " + prefixDir
    shellCmd(cmd)
    
    return

########################################################################
# build samurai package

def buildSamurai():

    global logPath

    print("==>> buildSamurai", file=sys.stderr)
    print("====>> prefixDir: ", prefixDir, file=sys.stderr)

    # set the environment

    os.environ["LROSE_INSTALL_DIR"] = prefixDir
    
    # check out samurai

    os.chdir(options.buildDir)
    shellCmd("/bin/rm -rf samurai")
    shellCmd("git clone https://github.com/mmbell/samurai")
    
    # run cmake to create makefiles - in-source build
    
    samuraiDir = os.path.join(options.buildDir, "samurai");
    cmakeBuildDir = os.path.join(samuraiDir, "build")
    os.makedirs(cmakeBuildDir)
    os.chdir(cmakeBuildDir)

    cmd = cmakeExec + " .."
    shellCmd(cmd)

    # do the build and install

    cmd = "make -k -j 8 install/strip"
    shellCmd(cmd)

    return

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
    logFp.write(logFileName + "\n")

    return logPath

########################################################################
# Run a command in a shell, wait for it to complete

def shellCmd(cmd):

    print("Running cmd:", cmd, file=sys.stderr)
    
    if (options.verbose):
        cmdToRun = cmd
    elif (logPath.find('no-logging') >= 0):
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

#!/usr/bin/env python

#===========================================================================
#
# Build LROSE release, using autoconf and configure
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
    global package
    global prefix
    global releaseName
    global netcdfDir
    global displaysDir
    global coreDir
    global codebaseDir
    global tmpDir
    global tmpBinDir
    global binDir
    global libDir
    global includeDir
    global shareDir
    global versionStr
    global debugStr

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    prefixDirDefault = os.path.join(homeDir, 'lrose')
    buildDirDefault = '/tmp/lrose_build'
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
                      dest='package', default='lrose',
                      help='Package name. Options are: ' + \
                      'lrose (default), cidd, radx, titan, lrose-blaze')
    parser.add_option('--prefix',
                      dest='prefix', default=prefixDirDefault,
                      help='Install directory')
    parser.add_option('--buildDir',
                      dest='buildDir', default=buildDirDefault,
                      help='Temporary build dir')
    parser.add_option('--static',
                      dest='static', default=False,
                      action="store_true",
                      help='use static linking, default is dynamic')
    parser.add_option('--scripts',
                      dest='installScripts', default=False,
                      action="store_true",
                      help='Install scripts as well as binaries')

    (options, args) = parser.parse_args()
    
    if (options.verbose == True):
        options.debug = True

    # for CIDD, set to static linkage
    if (options.package == "cidd"):
        options.static = True
        
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
    versionStr = nowTime.strftime("%Y%m%d")

    # set directories
    
    tmpDir = os.path.join(options.buildDir, 'tmp')
    coreDir = os.path.join(options.buildDir, "lrose-core")
    displaysDir = os.path.join(options.buildDir, "lrose-displays")
    netcdfDir = os.path.join(options.buildDir, "lrose-netcdf")
    codebaseDir = os.path.join(coreDir, "codebase")
    releaseName = options.package + "-" + versionStr + ".src"
    
    tmpBinDir = os.path.join(tmpDir, 'bin')
    binDir = os.path.join(prefix, 'bin')
    libDir = os.path.join(prefix, 'lib')
    includeDir = os.path.join(prefix, 'include')
    shareDir = os.path.join(prefix, 'share')
    
    if (options.debug == True):
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  package: ", package
        print >>sys.stderr, "  releaseName: ", releaseName
        print >>sys.stderr, "  static: ", options.static
        print >>sys.stderr, "  buildDir: ", options.buildDir
        print >>sys.stderr, "  coreDir: ", coreDir
        print >>sys.stderr, "  codebaseDir: ", codebaseDir
        print >>sys.stderr, "  displaysDir: ", displaysDir
        print >>sys.stderr, "  netcdfDir: ", netcdfDir
        print >>sys.stderr, "  prefixDir: ", prefix
        print >>sys.stderr, "  binDir: ", binDir
        print >>sys.stderr, "  libDir: ", libDir
        print >>sys.stderr, "  includeDir: ", includeDir
        print >>sys.stderr, "  shareDir: ", shareDir

    # create build dir
    
    createBuildDir()

    # make tmp dirs

    try:
        os.makedirs(tmpDir)
        os.makedirs(tmpBinDir)
    except:
        print >>sys.stderr, "  note - dirs already exist"

    # get repos from git

    gitCheckout()

    # install the distribution-specific makefiles

    os.chdir(codebaseDir)
    shellCmd("./make_bin/install_package_makefiles.py --package " + 
             package + " --codedir .")

    # trim libs and apps to those required by distribution makefiles

    if (package != "lrose"):
        trimToMakefiles("libs")
        trimToMakefiles("apps")

    # set up autoconf

    setupAutoconf()

    # create the release information file
    
    createReleaseInfoFile()

    # run qmake for QT apps to create moc_ files

    hawkEyeDir = os.path.join(codebaseDir, "apps/radar/src/HawkEye")
    createQtMocFiles(hawkEyeDir)

    # prune any empty directories

    prune(codebaseDir)

    # build netcdf support
    
    buildNetcdf()

    # build the package

    buildPackage()

    # perform the install

    doInstall();

    # check the install

    checkInstall()

    # delete the tmp dir

    if (options.clean):
        shutil.rmtree(options.buildDir)

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
            print("  " + filename)
        print("===============================================")
        answer = raw_input("WARNING: do you wish to proceed (y/n)? ")
        if (answer != "y"):
            print("  aborting ....")
            sys.exit(1)
                
        # remove it

        shutil.rmtree(options.buildDir)

    # make it clean

    os.makedirs(options.buildDir)

########################################################################
# check out repos from git

def gitCheckout():

    os.chdir(options.buildDir)
    shellCmd("git clone https://github.com/NCAR/lrose-core")
    shellCmd("git clone https://github.com/NCAR/lrose-netcdf")
    shellCmd("git clone https://github.com/NCAR/lrose-displays")

########################################################################
# set up autoconf for configure etc

def setupAutoconf():

    os.chdir(codebaseDir)

    # create files for configure

    shutil.copy("../build/Makefile.top", "Makefile")

    if (options.static):
        if (package == "cidd"):
             shutil.copy("../build/autoconf/configure.base.cidd", "./configure.base")
        else:
             shutil.copy("../build/autoconf/configure.base", "./configure.base")
        shellCmd("./make_bin/createConfigure.am.py --dir ." +
                 " --baseName configure.base" +
                 " --pkg " + package + debugStr)
    else:
        if (package == "cidd"):
            shutil.copy("../build/autoconf/configure.base.shared.cidd",
                        "./configure.base.shared")
        else:
            shutil.copy("../build/autoconf/configure.base.shared",
                        "./configure.base.shared")
        shellCmd("./make_bin/createConfigure.am.py --dir ." +
                 " --baseName configure.base.shared --shared" +
                 " --pkg " + package + debugStr)

########################################################################
# Run qmake for QT apps such as HawkEye to create _moc files

def createQtMocFiles(appDir):
    
    if (os.path.isdir(appDir) == False):
        return

    os.chdir(appDir)
    shellCmd("rm -f moc*");
    shellCmd("qmake-qt5 -o Makefile.qmake");
    shellCmd("make -f Makefile.qmake mocables");

########################################################################
# write release information file

def createReleaseInfoFile():

    # go to core dir

    os.chdir(tmpDir)

    # open info file

    releaseInfoPath = os.path.join(coreDir, "ReleaseInfo.txt")
    info = open(releaseInfoPath, 'w')

    # write release info

    info.write("package:" + package + "\n")
    info.write("version:" + versionStr + "\n")
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
        print >>sys.stderr, "ERROR - ", thisScriptName
        print >>sys.stderr, "  Cannot open file:", path
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
        elif (foundKey == True):
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

    print >>sys.stderr, "Trimming unneeded dirs, subDir: " + subDir

    # get list of subdirs in makefile

    dirPath = os.path.join(codebaseDir, subDir)
    os.chdir(dirPath)

    # need to allow upper and lower case Makefile (makefile or Makefile)
    subNameList = getValueListForKey("makefile", "SUB_DIRS")
    if not subNameList:
        print >>sys.stderr, "Trying uppercase Makefile ... "
        subNameList = getValueListForKey("Makefile", "SUB_DIRS")
    
    for subName in subNameList:
        if (os.path.isdir(subName)):
            print >>sys.stderr, "  need sub dir: " + subName
            
    # get list of files in subDir

    entries = os.listdir(dirPath)
    for entry in entries:
        theName = os.path.join(dirPath, entry)
        print >>sys.stderr, "considering: " + theName
        if (entry == "scripts") or (entry == "include"):
            # always keep scripts directories
            continue
        if (os.path.isdir(theName)):
            if (entry not in subNameList):
                print >>sys.stderr, "discarding it"
                shutil.rmtree(theName)
            else:
                print >>sys.stderr, "keeping it and recurring"
                # check this child's required subdirectories ( recurse )
                # nextLevel = os.path.join(dirPath, entry)
                # print >> sys.stderr, "trim to makefile on subdirectory: "
                trimToMakefiles(os.path.join(subDir, entry))

########################################################################
# build netCDF

def buildNetcdf():

    os.chdir(netcdfDir)
    if (package == "cidd"):
        shellCmd("./build_and_install_netcdf.m32 -x " + tmpDir)
    else:
        if platform == "darwin":
            shellCmd("./build_and_install_netcdf.osx -x " + tmpDir)
        else:
            shellCmd("./build_and_install_netcdf -x " + tmpDir)

########################################################################
# build package

def buildPackage():

    os.chdir(coreDir)

    # perform the build

    args = ""
    args = args + " --prefix " + tmpDir
    args = args + " --package " + package
    if (options.installScripts):
        args = args + " --scripts "
    shellCmd("./build/build_lrose.py " + args)

    # detect which dynamic libs are needed
    # copy the dynamic libraries into runtime area:
    #     $prefix/bin/${package}_runtime_libs

    if (platform != "darwin"):
        os.chdir(coreDir)
        shellCmd("./codebase/make_bin/installOriginLibFiles.py " + \
                 " --binDir " + tmpBinDir +
                 " --relDir " + package + "_runtime_libs --debug")

########################################################################
# perform install

def doInstall():

    # make target dirs

    try:
        os.makedirs(binDir)
        os.makedirs(libDir)
        os.makedirs(includeDir)
        os.makedirs(shareDir)
    except:
        print >>sys.stderr, "  note - dirs already exist"
    
    # install docs etc
    
    os.chdir(coreDir)

    shellCmd("rsync -av LICENSE.txt " + prefix)
    shellCmd("rsync -av release_notes " + prefix)
    shellCmd("rsync -av docs " + prefix)

    if (package == "cidd"):
        shellCmd("rsync -av ./codebase/apps/cidd/src/CIDD/example_scripts " +
                 options.prefix)

    # install color scales

    os.chdir(displaysDir)
    shellCmd("rsync -av color_scales " + shareDir)

    # install binaries and libs

    os.chdir(tmpDir)

    shellCmd("rsync -av bin " + prefix)
    shellCmd("rsync -av lib " + prefix)
    shellCmd("rsync -av include " + prefix)

########################################################################
# check the install

def checkInstall():

    os.chdir(coreDir)
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
    print("*** Done building auto release *******************")
    print("*** Installed in dir: " + prefix + " ***")
    print("**************************************************")

########################################################################
# prune empty dirs

def prune(tree):

    # walk the tree
    if (os.path.isdir(tree)):
        contents = os.listdir(tree)

        if (len(contents) == 0):
            print >> sys.stderr, "pruning empty dir: " + tree
            shutil.rmtree(tree)
        else:
            for l in contents:
                # remove CVS directories
                if (l == "CVS") or (l == ".git"): 
                    thepath = os.path.join(tree,l)
                    print >> sys.stderr, "pruning dir: " + thepath
                    shutil.rmtree(thepath)
                else:
                    thepath = os.path.join(tree,l)
                    if (os.path.isdir(thepath)):
                        prune(thepath)
            # check if this tree is now empty
            newcontents = os.listdir(tree)
            if (len(newcontents) == 0):
                print >> sys.stderr, "pruning empty dir: " + tree
                shutil.rmtree(tree)


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

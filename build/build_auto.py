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

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    buildDirDefault = os.path.join('/tmp/lrose_build')
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
                      help='Package name. Options are lrose (default), cidd, radx, titan, lrose-blaze')
    parser.add_option('--buildDir',
                      dest='buildDir', default=buildDirDefault,
                      help='Temporary build dir')
    parser.add_option('--static',
                      dest='static', default=False,
                      action="store_true",
                      help='produce distribution for static linking, default is dynamic')

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

    # runtime

    now = time.gmtime()
    nowTime = datetime(now.tm_year, now.tm_mon, now.tm_mday,
                       now.tm_hour, now.tm_min, now.tm_sec)
    versionStr = nowTime.strftime("%Y%m%d")

    # set directories

    coreDir = os.path.join(buildDir, "lrose-core")
    codebaseDir = os.path.join(coreDir, "codebase")

    if (options.debug == True):
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  package: ", options.package
        print >>sys.stderr, "  buildDir: ", buildDir
        print >>sys.stderr, "  static: ", options.static
        
    # create build dir
    
    createBuildDir()

    # get repos from git

    gitCheckout()

    # install the distribution-specific makefiles

    os.chdir(codebaseDir)
    shellCmd("./make_bin/install_package_makefiles.py --package " + 
               options.package + " --codedir .")

    # trim libs and apps to those required by distribution makefiles

    if (options.package != "lrose"):
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

    # delete the tmp dir

    # shutil.rmtree(tmpDir)

    sys.exit(0)

########################################################################
# create the build dir

def createBuildDir():

    # check if exists already

    if (os.path.isdir(buildDir)):

        print("WARNING: you are about to remove all contents in dir: " + buildDir)
        print("===============================================")
        contents = os.listdir(buildDir)
        for filename in contents:
            print("  " + filename)
        print("===============================================")
        answer = raw_input("WARNING: do you wish to proceed (y/n)? ")
        if (answer != "y"):
            print("  aborting ....")
            sys.exit(1)
                
        # remove it

        shutil.rmtree(buildDir)

    # make it clean

    os.makedirs(buildDir)

########################################################################
# check out repos from git

def gitCheckout():

    os.chdir(buildDir)
    shellCmd("git clone https://github.com/NCAR/lrose-core")
    shellCmd("git clone https://github.com/NCAR/lrose-netcdf")

########################################################################
# set up autoconf for configure etc

def setupAutoconf():

    os.chdir(codebaseDir)

    # create files for configure

    shutil.copy("../build/Makefile.top", "Makefile")

    if (options.static):
        if (options.package == "cidd"):
             shutil.copy("../build/configure.base.cidd", "./configure.base")
        else:
             shutil.copy("../build/configure.base", "./configure.base")
        shellCmd("./make_bin/createConfigure.am.py --dir ." +
                 " --baseName configure.base" +
                 " --pkg " + options.package + debugStr)
    else:
        if (options.package == "cidd"):
            shutil.copy("../build/configure.base.shared.cidd", "./configure.base.shared")
        else:
            shutil.copy("../build/configure.base.shared", "./configure.base.shared")
        shellCmd("./make_bin/createConfigure.am.py --dir ." +
                 " --baseName configure.base.shared --shared" +
                 " --pkg " + options.package + debugStr)

########################################################################
# Run qmake for QT apps such as HawkEye to create _moc files

def createQtMocFiles(appDir):
    
    os.chdir(appDir)
    shellCmd("rm -f moc*");
    shellCmd("qmake-qt5 -o Makefile.qmake");
    shellCmd("make -f Makefile.qmake mocables");

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
#        print >>sys.stderr, "  dir: ", options.dir
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
# Run - entry point

if __name__ == "__main__":
   main()

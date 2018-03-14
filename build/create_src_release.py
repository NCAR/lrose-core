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
                      help='Package name. Options are lrose (default), radx, cidd, hcr, hsrl, titan, lrose-blaze')
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

    # prune any empty directories
    prune(codebaseDir)

    # create the tar file

    createTarFile()

    # create the brew formula for OSX builds

    createBrewFormula()

    # move the tar file up into release dir

    os.chdir(releaseDir)
    os.rename(os.path.join(coreDir, tarName),
              os.path.join(releaseDir, tarName))
              
    # delete the tmp dir

    # shutil.rmtree(tmpDir)

    sys.exit(0)

########################################################################
# move previous releases

def savePrevReleases():

    if (os.path.isdir(releaseDir) == False):
        return
    
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

    # copy in script to make binary release

    shellCmd("cp build/create_bin_release.py " + tarDir)

    # move lrose contents into tar dir

    for fileName in [ "LICENSE.txt", "README.md", "ReleaseInfo.txt" ]:
        os.rename(fileName, os.path.join(tarDir, fileName))
        
    for dirName in [ "build", "codebase", "docs", "release_notes" ]:
        os.rename(dirName, os.path.join(tarDir, dirName))

    # move netcdf support into tar dir

    netcdfDir = os.path.join(tmpDir, "lrose-netcdf")
    netcdfSubDir = os.path.join(tarDir, "lrose-netcdf")
    os.makedirs(netcdfSubDir)
    
    if (options.package == "cidd"):
        name = "build_and_install_netcdf.m32"
        os.rename(os.path.join(netcdfDir, name),
                  os.path.join(netcdfSubDir, name))
    else:
        name = "build_and_install_netcdf"
        os.rename(os.path.join(netcdfDir, name),
                  os.path.join(netcdfSubDir, name))

    for name in [ "README.md", "tar_files" ]:
        os.rename(os.path.join(netcdfDir, name),
                  os.path.join(netcdfSubDir, name))

    # create the tar file

    shellCmd("tar cvfz " + tarName + " " + releaseName)
    
########################################################################
# create the brew formula for OSX builds

def createBrewFormula():

    # go to core dir

    os.chdir(coreDir)

    tarUrl = "https://github.com/NCAR/lrose-core/releases/download/" + \
             options.package + "-" + versionStr + "/" + tarName
    formulaName = options.package + ".rb"
    scriptName = "build_" + options.package + "_formula"
    buildDirPath = os.path.join(tarDir, "build")
    scriptPath = os.path.join(buildDirPath, scriptName)

    # check if script exists

    if (os.path.isfile(scriptPath) == False):
        print >>sys.stderr, "WARNING - ", thisScriptName
        print >>sys.stderr, "  No script: ", scriptPath
        print >>sys.stderr, "  Will not build brew formula for package"
        return

    # create the brew formula file

    shellCmd(scriptPath + " " + tarUrl + " " +
             tarName + " " + formulaName)

    # move it up into the release dir

    os.rename(os.path.join(coreDir, formulaName),
              os.path.join(releaseDir, formulaName))

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
    try: 
# need to allow upper and lower case Makefile (makefile or Makefile)
    subNameList = getValueListForKey("makefile", "SUB_DIRS")
    if (subNameList.empty()):
        print >>sys.stderr, "Trying uppercase Makefile ... "
        subNameList = getValueListForKey("Makefile", "SUB_DIRS")
    
    for subName in subNameList:
        if (os.path.isdir(subName)):
            print >>sys.stderr, "  need sub dir: " + subName
            
    # get list of files in subDir

    entries = os.listdir(dirPath)
    for entry in entries:
        if (entry == "scripts"):
            # always keep scripts directories
            continue
        if (os.path.isdir(entry)):
            if (entry not in subNameList):
                print >>sys.stderr, "discarding unneeded dir: " + entry
                shutil.rmtree(entry)
            else:
                # check this child's required subdirectories ( recurse )
                nextLevel = os.path.join(dirPath, entry)
                print >> sys.stderr, "trim to makefile on subdirectory: " + nextLevel
                trimToMakefiles(nextLevel)

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

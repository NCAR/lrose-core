#!/usr/bin/env python

#===========================================================================
#
# Create CMakeLists from LROSE Makefiles
#
#===========================================================================

from __future__ import print_function
import os
import sys
import shutil
import subprocess
from optparse import OptionParser
from datetime import datetime

def main():

    global options
    global subdirList
    global libList
    global makefileCreateList

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    # We will be executing some sibling scripts. Get our path so that
    # the sibling scripts from the same path can be executed explicitly.
    global thisScriptDir
    thisScriptDir = os.path.dirname(os.path.abspath(__file__))

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    coreDirDefault = os.path.join(thisScriptDir, "../..")
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--verbose',
                      dest='verbose', default=False,
                      action="store_true",
                      help='Set verbose debugging on')
    parser.add_option('--coreDir',
                      dest='coreDir', default=coreDirDefault,
                      help='Path of lrose-core top level directory, default is: ' +
                      coreDirDefault)
    parser.add_option('--shared',
                      dest='shared', default=False,
                      action="store_true",
                      help='Create shared lib objects')
    parser.add_option('--pkg',
                      dest='pkg', default="lrose-core",
                      help='Name of package being built')
    parser.add_option('--osx',
                      dest='osx', default=False,
                      action="store_true",
                      help='Configure for MAC OSX')

    (options, args) = parser.parse_args()

    if (options.verbose):
        options.debug = True
    
    if (options.debug):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  Core dir: ", options.coreDir, file=sys.stderr)
        print("  shared: ", options.shared, file=sys.stderr)
        print("  pkg: ", options.pkg, file=sys.stderr)
        print("  osx: ", options.osx, file=sys.stderr)

    # go to the top level codebase

    codebaseDir = os.path.join(options.coreDir, "codebase")
    os.chdir(codebaseDir)

    # copy the top level CMakeLists.txt file into place

    print("  info: copying top level CMakeLists.txt file", file=sys.stderr)
    shutil.copy('../build/cmake/CMakeLists.txt.top', './CMakeLists.txt')
    
    # recursively search the libs and apps trees

    libsDir =  os.path.join(codebaseDir, "libs")
    appsDir =  os.path.join(codebaseDir, "apps")

    if (options.debug):
        print("  codebase dir: ", codebaseDir, file=sys.stderr)
        print("  libs dir: ", libsDir, file=sys.stderr)
        print("  apps dir: ", appsDir, file=sys.stderr)

    # get list of libs
    
    getLibList(libsDir)

    # search libs and apps for makefiles

    makefileCreateList = []
    searchDir(libsDir)
    searchDir(appsDir)

    if (options.debug):
        for path in makefileCreateList:
            print("  Need to create makefile: ", path, file=sys.stderr)
            
    sys.exit(0)


    # write out configure.ac
            
    if (writeConfigureAc() != 0):
        sys.exit(1)
        
    # run autoconf

    debugStr = "";
    if (options.verbose):
        debugStr = " --verbose "
    elif (options.debug):
        debugStr = " --debug "
        
    sharedStr = ""
    if (options.shared):
        sharedStr = " --shared "

    cmd = os.path.join(thisScriptDir, "runAutoConf.py") + \
          " --dir " + options.coreDir + sharedStr + debugStr
    # runCommand(cmd)

    sys.exit(0)

########################################################################
# search libs directory to compile libs list

def getLibList(dir):
                    
    global libList
    libList = ""
    libArray = []

    if (options.debug):
        print("  Getting lib list from dir: ", dir, file=sys.stderr)

    # check if this dir has a makefile or Makefile

    makefilePath = getMakefileTemplatePath(dir)
    if (os.path.exists(makefilePath) == False):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  No makefile in lib dir: ", dir, file=sys.stderr)
        exit(1)

    if (options.debug):
        print("  Searching makefile template: ", makefilePath, file=sys.stderr)

    # search for SUB_DIRS key in makefile

    subNameList = getValueListForKey(makefilePath, "SUB_DIRS")

    if (len(subNameList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find SUB_DIRS in ", makefileName, file=sys.stderr)
        print("  coreDir: ", options.coreDir, file=sys.stderr)
        exit(1)

    for subName in subNameList:
        subPath = os.path.join(dir, subName)
        if (os.path.isdir(subPath)):
            if (subName.find("perl5") < 0):
                libArray.append(subName)

    for index, libName in enumerate(libArray):
        libList += libName
        if (index < len(libArray) - 1):
            libList += ","

    if (options.debug):
        print("  libList: ", libList, file=sys.stderr)

    return

########################################################################
# search directory and subdirectories

def searchDir(dir):
                    
    global makefileCreateList

    if (options.debug):
        print("  Searching dir: ", dir, file=sys.stderr)

    # check if this dir has a makefile or Makefile

    makefilePath = getMakefileTemplatePath(dir)
    if (os.path.exists(makefilePath) == False):
        if (options.verbose):
            print("  No makefile or Makefile found", file=sys.stderr)
        return

    # detect which type of directory we are in
        
    if (options.verbose):
        print("  Found makefile: ", makefilePath, file=sys.stderr)

    if ((dir == "libs/perl5") or
        (dir == "apps/scripts")):
        if (options.debug):
            print("  Ignoring dir:", dir, file=sys.stderr)
        return

    debugStr = "";
    if (options.verbose):
        debugStr = " --debug "

    absDir = os.path.join(options.coreDir, dir)
    pathToks = absDir.split("/")
    ntoks = len(pathToks)
    makefileCreatePath = os.path.join(dir, 'makefile')

    if (pathToks[ntoks-3] == "libs" and
        pathToks[ntoks-1] == "src"):

        # src level of lib - create CMakeLists.txt for lib

        libDir = absDir[:-4]
        sharedStr = ""
        if (options.shared):
            sharedStr = " --shared "
        cmd = os.path.join(thisScriptDir, "createCMakeLists.lib.py") + \
              " --dir " + libDir + sharedStr + debugStr
        cmd += " --libList " + libList
        runCommand(cmd)
        makefileCreateList.append(makefileCreatePath)

        return

    elif ((pathToks[ntoks-4] == "apps") and
          (pathToks[ntoks-2] == "src") and
          (pathToks[ntoks-1] == "scripts")):

        # scripts dir - do nothing
        if (options.debug):
            print("  Ignoring dir:", dir, file=sys.stderr)

        return

    elif ((pathToks[ntoks-4] == "apps") and
          (pathToks[ntoks-2] == "src")):

        # app directory - create CMakeLists.txt for app

        # compute default script path - assumes core package

        #createScript = "createCMakeLists.app.lrose-core.py"
        createScript = "createCMakeLists.app.py"
        scriptPath = os.path.join(thisScriptDir, createScript)

        # use package-specific version if available
        #pkgCreateScript = "createCMakeLists.app." + options.pkg + ".py"
        #pkgScriptPath = os.path.join(thisScriptDir, pkgCreateScript)
        #if (os.path.exists(pkgScriptPath)):
        #    createScript = pkgCreateScript
        #    scriptPath = pkgScriptPath

        if (options.debug):
            print("  createScript:", createScript, file=sys.stderr)

        cmd = scriptPath
        cmd += " --dir " + absDir + debugStr
        cmd += " --libList " + libList
        if (options.osx):
            cmd += " --osx "
        runCommand(cmd)
        makefileCreateList.append(makefileCreatePath)

        return

    else:

        # create CMakeLists.txt for recursion
        cmd = os.path.join(thisScriptDir, "createCMakeLists.recurse.py") + \
              " --dir " + absDir + debugStr
        runCommand(cmd)
        makefileCreateList.append(makefileCreatePath)
        # recurse
        loadSubdirList(dir)
        for subdir in subdirList:
            subdirPath = os.path.join(dir, subdir)
            searchDir(subdirPath)

    return

########################################################################
# load list of sub directories

def loadSubdirList(dir):
                    
    global subdirList
    subdirList = []

    makefilePath = getMakefileTemplatePath(dir)

    try:
        fp = open(makefilePath, 'r')
    except IOError as e:
        if (options.verbose):
            print("ERROR - ", thisScriptName, file=sys.stderr)
            print("  Cannot find makefile or Makefile", file=sys.stderr)
            print("  coreDir: ", options.coreDir, file=sys.stderr)
        return

    lines = fp.readlines()
    fp.close()

    subDirsFound = False
    for line in lines:
        if (subDirsFound == False):
            if (line[0] == '#'):
                continue
            if (line.find("SUB_DIRS") >= 0):
                subDirsFound = True
                toks = line.split(' ')
                for tok in toks:
                    thisTok = tok.strip(" \t\n\r")
                    if (thisTok != "SUB_DIRS"):
                        if (len(thisTok) > 1):
                            subdirList.append(thisTok)
        else:
            if (line[0] == '#'):
                return
            if (len(line) < 2):
                return
            toks = line.split(' ')
            for tok in toks:
                thisTok = tok.strip(" \t\n\r")
                if (len(thisTok) > 1):
                    subdirList.append(thisTok)

########################################################################
# Write out configure.ac

def writeConfigureAc():

    # read preamble lines from base configure file

    try:
        base = open(options.baseName, "r")
    except IOError as e:
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot read base configure template", file=sys.stderr)
        print("  base name: ", options.baseName, file=sys.stderr)
        print("  This file should be in: ", options.coreDir, file=sys.stderr)
        return 1

    base = open(options.baseName, "r")
    lines = base.readlines()
    base.close()

    # open configure.ac for writing

    try:
        confac = open("configure.ac", "w")
    except IOError as e:
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot open configure.ac for writing", file=sys.stderr)
        print("  dir: ", options.coreDir, file=sys.stderr)
        return 1

    confac = open("configure.ac", "w")

    confac.write("###############################################\n")
    confac.write("#\n")
    confac.write("# configure template for autoconf\n")
    confac.write("#\n")
    confac.write("# dir: %s\n" % options.coreDir)
    confac.write("#\n")
    confac.write("# baseName: %s\n" % options.baseName)
    confac.write("#\n")
    confac.write("# written by script %s\n" % thisScriptName)
    confac.write("#\n")
    confac.write("# created %s\n" % datetime.now())
    confac.write("#\n")
    confac.write("###############################################\n")
    confac.write("######### COPIED FROM BASE TEMPLATE ###########\n")
    confac.write("\n")

    for line in lines:
        confac.write(line)

    confac.write("\n")
    confac.write("############## DONE WITH BASE #################\n")
    confac.write("###############################################\n")
    confac.write("\n")
    confac.write("\n")
    confac.write("# create makefiles\n")
    confac.write("\n")
    confac.write("AC_CONFIG_FILES([\n")
    confac.write("  makefile\n")
    for path in makefileCreateList:
        confac.write("  %s\n" % path)
    confac.write("])\n")
    confac.write("AC_OUTPUT\n")
    confac.write("\n")

    confac.close

    return 0

########################################################################
# find makefile template

def getMakefileTemplatePath(dir):
                    
    makefilePath = os.path.join(dir, '__makefile.template')
    if (os.path.exists(makefilePath) == False):
        makefilePath = os.path.join(dir, 'makefile')
        if (os.path.exists(makefilePath) == False):
            makefilePath = os.path.join(dir, 'Makefile')
            if (os.path.exists(makefilePath) == False):
                makefilePath = os.path.join(dir, 'not_found')

    return makefilePath

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
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot open file:", path, file=sys.stderr)
        print("  dir: ", options.coreDir, file=sys.stderr)
        return valueList

    lines = fp.readlines()
    fp.close()

    foundKey = False
    multiLine = ""
    for line in lines:
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
# Run a command in a shell, wait for it to complete

def runCommand(cmd):

    if (options.debug):
        print("running cmd:",cmd, file=sys.stderr)
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print("Child exited with code: ", retcode, file=sys.stderr)
            exit(1)
        else:
            if (options.verbose):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, retcode, file=sys.stderr)
        exit(1)

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

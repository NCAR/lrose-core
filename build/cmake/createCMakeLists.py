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

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global count
    count = 0

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
                      dest='debug', default=True,
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
    count = count + 1
    
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

    searchDir(libsDir)
    searchDir(appsDir)

    if (options.debug):
        print("==>> n CMakeLists.txt files created: ", count, file=sys.stderr)

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

    makefilePath = getMakefilePath(dir)
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
        print("  Cannot find SUB_DIRS in ", makefilePath, file=sys.stderr)
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
                    
    global count

    if (options.debug):
        print("  Searching dir: ", dir, file=sys.stderr)

    # check if this dir has a makefile or Makefile

    makefilePath = getMakefilePath(dir)
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
        count = count + 1

    elif ((pathToks[ntoks-4] == "apps") and
          (pathToks[ntoks-2] == "src")):

        # app directory - create CMakeLists.txt for app

        createScript = "createCMakeLists.app.py"
        scriptPath = os.path.join(thisScriptDir, createScript)

        if (options.debug):
            print("  createScript:", createScript, file=sys.stderr)

        cmd = scriptPath
        cmd += " --dir " + absDir + debugStr
        cmd += " --libList " + libList
        if (options.osx):
            cmd += " --osx "
        runCommand(cmd)
        count = count + 1

    elif ((pathToks[ntoks-4] == "apps") and
          (pathToks[ntoks-2] == "src") and
          (pathToks[ntoks-1] == "scripts")):

        # scripts dir - do nothing
        if (options.debug):
            print("  Ignoring dir:", dir, file=sys.stderr)

    else:

        # create CMakeLists.txt for recursion
        createCMakeListsRecurse(absDir)
        count = count + 1
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

    makefilePath = getMakefilePath(dir)

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
# find makefile template

def getMakefilePath(dir):
                    
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
        if ((not foundKey) and (line[0] == '#')):
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

#===========================================================================
#
# Create CMakeLists.txt for directory recursion
#
#===========================================================================

def createCMakeListsRecurse(dir):

    if (options.debug):
        print("  Recurse, dir: ", dir, file=sys.stderr)

    # go to the dir

    # currentDir = os.getcwd()
    # os.chdir(dir)

    # get makefile name in use

    makefilePath = getMakefilePath(dir)

    # load list of subdirs
    
    subdirList = getSubdirList(makefilePath)

    if (options.debug == True):
        print("=======================", file=sys.stderr)
        print("subdirList:", file=sys.stderr)
        for subdir in subdirList:
            print("subdir: %s" % (subdir), file=sys.stderr)
        print("=======================", file=sys.stderr)

    # write out CMakeLists.txt for recursion
            
    writeCMakeListsRecurse(dir, subdirList)

    # os.chdir(currentDir)

########################################################################
# load list of sub directories

def getSubdirList(makefilePath):
                    
    subdirList = []
    
    # search for SUB_DIRS key in makefile

    subNameList = getValueListForKey(makefilePath, "SUB_DIRS")

    if (len(subNameList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find SUB_DIRS in ", makefilePath, file=sys.stderr)
        print("  dir: ", options.dir, file=sys.stderr)
        exit(1)

    for subName in subNameList:
        if (os.path.isdir(subName) == True):
            subdirList.append(subName)

    return subdirList

########################################################################
# Write out CMakeLists.am

def writeCMakeListsRecurse(dir, subdirList):

    fo = open("CMakeLists.txt", "w")

    fo.write("###############################################\n")
    fo.write("#\n")
    fo.write("# CMakeLists for cmake recursion\n")
    fo.write("#\n")
    fo.write("# dir: %s\n" % dir)
    fo.write("#\n")
    fo.write("# written by script %s\n" % thisScriptName)
    fo.write("#\n")
    fo.write("# created %s\n" % datetime.now())
    fo.write("#\n")
    fo.write("###############################################\n")
    fo.write("\n")
    fo.write("project (LROSE-CORE)\n")
    fo.write("\n")

    if (len(subdirList) > 0):
        fo.write("# subdirectories\n")
        fo.write("\n")
        for subdir in subdirList:
            fo.write("add_subdirectory (%s)\n" % subdir)

    fo.write("\n")
    fo.close

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

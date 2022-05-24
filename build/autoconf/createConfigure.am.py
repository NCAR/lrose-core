#!/usr/bin/env python

#===========================================================================
#
# Create configure.am for directory recursion
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
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default='False',
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--verbose',
                      dest='verbose', default='False',
                      action="store_true",
                      help='Set verbose debugging on')
    parser.add_option('--osx',
                      dest='osx', default='False',
                      action="store_true",
                      help='Configure for MAC OSX')
    parser.add_option('--dir',
                      dest='dir', default=".",
                      help='Path of top level directory')
    parser.add_option('--baseName',
                      dest='baseName', default="configure.base",
                      help='Name of configure base file - this will be copied to the configure.ac file, followed by the makefile list. This file should reside in the specifed dir. Default is \'configure.base\'.')
    parser.add_option('--shared',
                      dest='shared', default='False',
                      action="store_true",
                      help='Create shared lib objects')
    parser.add_option('--pkg',
                      dest='pkg', default="lrose-core",
                      help='Name of package being built')

    (options, args) = parser.parse_args()

    if (options.verbose == True):
        options.debug = True
    
    if (options.debug == True):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  Top level dir: ", options.dir, file=sys.stderr)
        print("  Base file name: ", options.baseName, file=sys.stderr)
        print("  shared: ", options.shared, file=sys.stderr)
        print("  pkg: ", options.pkg, file=sys.stderr)
        print("  osx: ", options.osx, file=sys.stderr)

    # go to the dir

    os.chdir(options.dir)

    # create makefile.am at top level

    cmd = os.path.join(thisScriptDir, "createMakefile.am.topdir.py") + \
          " --dir " + options.dir
    if (options.debug == True):
        cmd += " --debug"
    runCommand(cmd)

    # recursively search the libs and apps trees

    libsDir = "libs"
    appsDir = "apps"

    if (options.debug == True):
        print("  libs dir: ", libsDir, file=sys.stderr)
        print("  apps dir: ", appsDir, file=sys.stderr)

    # get list of libs
    
    getLibList(libsDir)

    # search libs and apps for makefiles

    makefileCreateList = []
    searchDir(libsDir)
    searchDir(appsDir)

    if (options.debug == True):
        for path in makefileCreateList:
            print("  Need to create makefile: ", path, file=sys.stderr)
            
    # write out configure.ac
            
    if (writeConfigureAc() != 0):
        sys.exit(1)
        
    # run autoconf

    debugStr = "";
    if (options.verbose == True):
        debugStr = " --verbose "
    elif (options.debug == True):
        debugStr = " --debug "
        
    sharedStr = ""
    if (options.shared == True):
        sharedStr = " --shared "

    cmd = os.path.join(thisScriptDir, "runAutoConf.py") + \
          " --dir " + options.dir + sharedStr + debugStr
    runCommand(cmd)

    sys.exit(0)

########################################################################
# search libs directory to compile libs list

def getLibList(dir):
                    
    global libList
    libList = ""
    libArray = []

    if (options.debug == True):
        print("  Getting lib list from dir: ", dir, file=sys.stderr)

    # check if this dir has a makefile or Makefile

    templatePath = getMakefileTemplatePath(dir)
    if (os.path.exists(templatePath) == False):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  No makefile in lib dir: ", dir, file=sys.stderr)
        exit(1)

    if (options.debug == True):
        print("  Searching makefile template: ", templatePath, file=sys.stderr)

    # search for SUB_DIRS key in makefile

    subNameList = getValueListForKey(templatePath, "SUB_DIRS")

    if (len(subNameList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find SUB_DIRS in ", makefileName, file=sys.stderr)
        print("  dir: ", options.dir, file=sys.stderr)
        exit(1)

    for subName in subNameList:
        subPath = os.path.join(dir, subName)
        if (os.path.isdir(subPath) == True):
            if (subName.find("perl5") < 0):
                libArray.append(subName)

    for index, libName in enumerate(libArray):
        libList += libName
        if (index < len(libArray) - 1):
            libList += ","

    if (options.debug == True):
        print("  libList: ", libList, file=sys.stderr)

    return

########################################################################
# search directory and subdirectories

def searchDir(dir):
                    
    global makefileCreateList

    if (options.debug == True):
        print("  Searching dir: ", dir, file=sys.stderr)

    # check if this dir has a makefile or Makefile
    
    templatePath = getMakefileTemplatePath(dir)
    if (os.path.exists(templatePath) == False):
        if (options.verbose == True):
            print("  No makefile or Makefile found", file=sys.stderr)
        return

    # detect which type of directory we are in
        
    if (options.verbose == True):
        print("  Found makefile: ", templatePath, file=sys.stderr)

    if ((dir == "libs/perl5") or
        (dir == "apps/scripts")):
        if (options.debug):
            print("  Ignoring dir:", dir, file=sys.stderr)
        return

    debugStr = "";
    if (options.verbose == True):
        debugStr = " --debug "

    absDir = os.path.join(options.dir, dir)
    pathToks = absDir.split("/")
    ntoks = len(pathToks)
    makefileCreatePath = os.path.join(dir, 'makefile')

    if (pathToks[ntoks-3] == "libs" and
        pathToks[ntoks-1] == "src"):

        # src level of lib - create makefile.am for lib

        libDir = absDir[:-4]
        sharedStr = ""
        if (options.shared == True):
            sharedStr = " --shared "
        cmd = os.path.join(thisScriptDir, "createMakefile.am.lib.py") + \
            " --dir " + libDir + \
            " --template " + templatePath + \
            sharedStr + debugStr
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

        # app directory - create makefile.am for app

        # compute default script path - assumes core package

        createScript = "createMakefile.am.app.lrose-core.py"
        scriptPath = os.path.join(thisScriptDir, createScript)

        # use package-specific version if available
        pkgCreateScript = "createMakefile.am.app." + options.pkg + ".py"
        pkgScriptPath = os.path.join(thisScriptDir, pkgCreateScript)
        if (os.path.exists(pkgScriptPath)):
            createScript = pkgCreateScript
            scriptPath = pkgScriptPath

        if (options.debug):
            print("  createScript:", createScript, file=sys.stderr)

        cmd = scriptPath
        cmd += " --dir " + absDir + debugStr
        cmd += " --template " + templatePath
        cmd += " --libList " + libList
        if (options.osx == True):
            cmd += " --osx "
        runCommand(cmd)
        makefileCreateList.append(makefileCreatePath)

        return

    else:

        # create makefile.am for recursion
        cmd = os.path.join(thisScriptDir, "createMakefile.am.recurse.py") + \
              " --dir " + absDir + \
              " --template " + templatePath + \
              debugStr
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

    templatePath = getMakefileTemplatePath(dir)

    try:
        fp = open(templatePath, 'r')
    except IOError as e:
        if (options.verbose == True):
            print("ERROR - ", thisScriptName, file=sys.stderr)
            print("  Cannot find makefile or Makefile", file=sys.stderr)
            print("  dir: ", options.dir, file=sys.stderr)
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
        print("  This file should be in: ", options.dir, file=sys.stderr)
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
        print("  dir: ", options.dir, file=sys.stderr)
        return 1

    confac = open("configure.ac", "w")

    confac.write("###############################################\n")
    confac.write("#\n")
    confac.write("# configure template for autoconf\n")
    confac.write("#\n")
    confac.write("# dir: %s\n" % options.dir)
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
                    
    # get Makefile and template

    cwd = os.getcwd()
    dirPath = os.path.join(cwd, dir)
    
    templateMakefilePath = os.path.join(dirPath, '__makefile.template')
    upperMakefilePath = os.path.join(dirPath, 'Makefile')
    lowerMakefilePath = os.path.join(dirPath, 'makefile')

    # use lower case makefile if it exists
    
    if (os.path.exists(lowerMakefilePath)):
        templatePath = lowerMakefilePath
    elif (os.path.exists(upperMakefilePath)):
        templatePath = upperMakefilePath
    elif (os.path.exists(templateMakefilePath)):
        templatePath = templateMakefilePath
    else:
        print("-->> Makefile not found, dir: ", dirPath, file=sys.stderr)
        return 'not-found'
    
    print("  ---->> makefile template path: ", templatePath, file=sys.stderr)

    return templatePath

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
        print("  dir: ", options.dir, file=sys.stderr)
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
# Run a command in a shell, wait for it to complete

def runCommand(cmd):

    if (options.debug == True):
        print("running cmd:",cmd, file=sys.stderr)
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print("Child exited with code: ", retcode, file=sys.stderr)
            exit(1)
        else:
            if (options.verbose == True):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, retcode, file=sys.stderr)
        exit(1)

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

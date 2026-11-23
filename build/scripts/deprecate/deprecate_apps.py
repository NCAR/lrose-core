#!/usr/bin/env python3

#===========================================================================
#
# Deprecate apps that are not used.
# Create script for moving apps to deprecated dir.
#
#===========================================================================

import os
import sys
import shutil
import subprocess
from optparse import OptionParser
from datetime import datetime
from pathlib import Path

def main():

    global options
    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    # We will be executing some sibling scripts. Get our path so that
    # the sibling scripts from the same path can be executed explicitly.

    global thisScriptDir
    thisScriptDir = os.path.dirname(os.path.abspath(__file__))
    
    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    coreDirDefault = os.path.join(thisScriptDir, "../../..")
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--coreDir',
                      dest='coreDir', default=coreDirDefault,
                      help='Path of lrose-core top level directory, default is: ' +
                      coreDirDefault)
    parser.add_option('--appDir',
                      dest='appDir', default="apps/titan/src",
                      help='Path to app dir')
    
    (options, args) = parser.parse_args()
    
    os.chdir(options.coreDir)
    coreDir = os.getcwd()

    codebaseAppDir = os.path.join("codebase", options.appDir);
    deprecatedAppDir = os.path.join("deprecated", options.appDir);

    if (options.debug):
        print("=============================================", file=sys.stderr)
        print("Running %s         :" % thisScriptName, file=sys.stderr)
        print("  thisScriptDir    : ", thisScriptDir, file=sys.stderr)
        print("  coreDir          : ", coreDir, file=sys.stderr)
        print("  appDir           : ", options.appDir, file=sys.stderr)
        print("  codebaseAppDir   : ", codebaseAppDir, file=sys.stderr)
        print("  deprecatedAppDir : ", deprecatedAppDir, file=sys.stderr)
        print("=============================================", file=sys.stderr)
        
    # get list of subdirs from Makefile
    
    makefileSubdirList = getMakefileSubdirList(codebaseAppDir)
    if (options.debug):
        print("=====================================", file=sys.stderr)
        print("  makefileSubdirs: ", makefileSubdirList, file=sys.stderr)
        print("=====================================", file=sys.stderr)
    
    # get list of subdirs from file system
    
    actualSubdirList = getActualSubdirList(codebaseAppDir)
    if (options.debug):
        print("=====================================", file=sys.stderr)
        print("  actualSubdirs: ", actualSubdirList, file=sys.stderr)
        print("=====================================", file=sys.stderr)

    # get list of unused dirs

    unused = []
    for dir in actualSubdirList:
        if dir not in makefileSubdirList:
            unused.append(dir)

    if (options.debug):
        print("=====================================", file=sys.stderr)
        print("  unused: ", unused, file=sys.stderr)
        print("=====================================", file=sys.stderr)

    # write commands
    
    print("mkdir -p ", deprecatedAppDir, file=sys.stdout)
    for dir in unused:
        codebaseDir = os.path.join(codebaseAppDir, dir)
        deprecatedDir = os.path.join(deprecatedAppDir, dir)
        print("git mv ", codebaseDir, " ", deprecatedDir, file=sys.stdout)
        
    sys.exit(0)

########################################################################
# load list of sub directories from Makefile

def getMakefileSubdirList(dir):
                    
    subdirList = []
    
    # get makefile for this dir

    makefilePath = getMakefilePath(dir)

    # search for SUB_DIRS key in makefile

    subNameList = getValuesForKey(makefilePath, "SUB_DIRS")

    if (len(subNameList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find SUB_DIRS in ", makefilePath, file=sys.stderr)
        print("  dir: ", dir, file=sys.stderr)
        sys.exit(1)

    for subName in subNameList:
        subPath = os.path.join(dir, subName)
        if (os.path.isdir(subPath)):
            subdirList.append(subName)

    return subdirList

########################################################################
# load list of sub directories

def getActualSubdirList(dir):
                    
    os.chdir(dir)
    target_dir = Path(".")

    # Returns a list of Path objects
    subdirPaths = [d for d in target_dir.iterdir() if d.is_dir()]

    subdirStrings = []
    for dir in subdirPaths:
        subdirStrings.append(str(dir))

    return subdirStrings
    
########################################################################
# find makefile template

def getMakefilePath(dir):

    # get Makefile and template
    
    templateMakefilePath = os.path.join(dir, '__makefile.template')
    upperMakefilePath = os.path.join(dir, 'Makefile')
    lowerMakefilePath = os.path.join(dir, 'makefile')

    # read the upper-case Makefile and template
    # file contents as strings

    templateContents = ""
    upperContents = ""
    try:
        templateFp = open(templateMakefilePath, 'r')
        templateContents = templateFp.read()
        templateFp.close()
    except IOError as e:
        print("  no __makefileTemplate, dir: ", dir, file=sys.stderr)
    try:
        upperMakefileFp = open(upperMakefilePath, 'r')
        upperContents = upperMakefileFp.read()
        upperMakefileFp.close()
    except IOError as e:
        print("  no Makefile, dir: ", dir, file=sys.stderr)
        
    # update the template from Makefile if needed
    # or vice versa
    
    if (upperContents.find("lrose_make_macros") > 0):
        if (templateContents != upperContents):
            shutil.copy(upperMakefilePath, templateMakefilePath)
            print("  copying Makefile to template, dir: ", dir, file=sys.stderr)
            print("  upperMakefilePath: ", upperMakefilePath, file=sys.stderr)
            print("  templateMakefilePath: ", templateMakefilePath, file=sys.stderr)
    elif (templateContents.find("lrose_make_macros") > 0):
        shutil.copy(templateMakefilePath, upperMakefilePath)
        print("  copying template to Makefile, dir: ", dir, file=sys.stderr)
        print("  templateMakefilePath: ", templateMakefilePath, file=sys.stderr)
        print("  upperMakefilePath: ", upperMakefilePath, file=sys.stderr)

    # use lower case makefile if it exists
    
    makefilePath = lowerMakefilePath
    if (os.path.exists(lowerMakefilePath)):
        makefilePath = lowerMakefilePath
        print("  using lower-case makefile, dir: ", dir, file=sys.stderr)
    else:
        makefilePath = upperMakefilePath

    if (os.path.exists(makefilePath) == False):
        if (options.debug):
            print("-->> Makefile not found, dir: ", dir, file=sys.stderr)
        return 'not-found'

    return makefilePath

########################################################################
# get string value based on search key
# the string may span multiple lines
#
# Example of keys: SRCS, SUB_DIRS, MODULE_NAME, TARGET_FILE
#
# value is returned

def getValuesForKey(makefilePath, key):

    valueList = []

    try:
        fp = open(makefilePath, 'r')
    except IOError as e:
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot open file:", makefilePath, file=sys.stderr)
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
                break
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
            sys.exit(1)
        else:
            if (options.verbose):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, retcode, file=sys.stderr)
        sys.exit(1)

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()
 

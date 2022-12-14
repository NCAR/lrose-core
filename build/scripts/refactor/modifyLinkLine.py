#!/usr/bin/env python

#===========================================================================
#
# modify link line in all Makefiles
#
#===========================================================================

import os
import sys
import subprocess
from optparse import OptionParser
from datetime import datetime

def main():

    global options
    global libList
    global makefileList

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
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--verbose',
                      dest='verbose', default=False,
                      action="store_true",
                      help='Set verbose debugging on')
    parser.add_option('--all',
                      dest='process_all', default=False,
                      action="store_true",
                      help='Process all Makefiles, rather than just those pointed to by the other Makefiles')
    parser.add_option('--dir',
                      dest='dir', default=".",
                      help='Path of top level directory')

    (options, args) = parser.parse_args()

    if (options.verbose):
        options.debug = True
    
    if (options.debug):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  Top level dir: ", options.dir, file=sys.stderr)

    # created ordered lib list for correct dependencies
    
    createOrderedLibList()

    # search recursively for Makefile from dir down

    makefileList = []
    processDir(options.dir)

    if (options.debug):
        for path in makefileList:
            print("  Makefile: ", path, file=sys.stderr)

    # process each Makefile

    for path in makefileList:
        fixMakefile(path)
            
    sys.exit(0)

########################################################################
# process this directory and its subdirectories

def processDir(dir):
                    
    global makefileList

    if (options.debug):
        print("  Searching dir: ", dir, file=sys.stderr)

    # check if this dir has a makefile or Makefile

    makefilePath = getMakefilePath(dir)
    if (os.path.exists(makefilePath) == False):
        if (options.verbose):
            print("  No Makefile or makefile found", file=sys.stderr)
        return
        
    # detect which type of directory we are in
    
    if (options.verbose):
        print("  Found makefile: ", makefilePath, file=sys.stderr)

    pathToks = dir.split("/")
    nToks = len(pathToks)

    if (options.verbose):
        print("  pathToks: ", pathToks, file=sys.stderr)
        print("  nToks: ", nToks, file=sys.stderr)
    
    if ((nToks > 4) and
        (pathToks[-4] == "apps") and
        (pathToks[-2] == "src")):
        # app directory
        # use this makefile
        makefileList.append(makefilePath)
        return
    else:

        if (options.process_all):
            # recurse
            subDirPaths = getAllSubDirPaths(dir)
        else:
            # get list from makefiles
            subDirPaths = getMakefileSubdirPaths(dir)

        print("  subDirPaths: ", subDirPaths, file=sys.stderr)

        # process sub dirs

        for subDirPath in subDirPaths:
            processDir(subDirPath)

    return

########################################################################
# getlist of sub directories listed in makefile

def getMakefileSubdirPaths(dir):
                    
    subDirPaths = []
    makefilePath = getMakefilePath(dir)

    if (options.debug):
        print("Found makefile: ", makefilePath, file=sys.stderr)

    try:
        fp = open(makefilePath, 'r')
    except IOError as e:
        if (options.verbose):
            print("ERROR - ", thisScriptName, file=sys.stderr)
            print("  Cannot find makefile or Makefile", file=sys.stderr)
            print("  dir: ", options.dir, file=sys.stderr)
        return subDirPaths

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
                            subDirPath = os.path.join(dir, thisTok)
                            subDirPaths.append(subDirPath)
        else:
            if (line[0] == '#'):
                break
            if (len(line) < 2):
                break
            toks = line.split(' ')
            for tok in toks:
                thisTok = tok.strip(" \t\n\r")
                if (len(thisTok) > 1):
                    subDirPath = os.path.join(dir, thisTok)
                    subDirPaths.append(subDirPath)

    return subDirPaths

########################################################################
# get all subdirs of a given dir

def getAllSubDirPaths(dir):

    subDirPaths = list(filter(os.path.isdir, [os.path.join(dir,f) for f in os.listdir(dir)]))
    return subDirPaths

########################################################################
# find makefile template

def getMakefilePath(dir):
                    
    makefilePath = os.path.join(dir, 'Makefile')
    if (os.path.exists(makefilePath) == False):
        makefilePath = os.path.join(dir, 'makefile')
        if (os.path.exists(makefilePath) == False):
                makefilePath = os.path.join(dir, 'not_found')
                
    return makefilePath

########################################################################
# fix the makefile link line

def fixMakefile(makefilePath):

    # parse make file for local libs

    (libsList, lines, lineNumsLibs) = getValueListForKey(makefilePath, "LOC_LIBS")

    print("=============================", file=sys.stderr)
    print("  path: ", makefilePath, file=sys.stderr)
    print("  libsList: ", libsList, file=sys.stderr)
    print("  lineNumsLibs: ", lineNumsLibs, file=sys.stderr)
    if (options.verbose):
        print("  makefile contents: ", file=sys.stderr)
        for line in lines:
            print(line.rstrip(), file=sys.stderr)

    # augment the libsList with dependencies that are not already included

    extendedList = libsList
    for libName in libsList:
        dependList = getDependentLibs(libName)
        for dependName in dependList:
            if dependName not in extendedList:
                extendedList.append(dependName)

    # create lrose libs list in the correct order

    goodList = []
    for libName in orderedLibList:
        if (libName in extendedList):
            goodList.append(libName)
            print("3333333333333333333333333333", file=sys.stderr)
            print("  adding libName: ", libName, file=sys.stderr)
            print("3333333333333333333333333333", file=sys.stderr)
    print("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb", file=sys.stderr)
    print("  goodList: ", goodList, file=sys.stderr)
    print("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb", file=sys.stderr)

    # add in libs not in lrose

    for libName in libsList:
        if (libName not in goodList):
            goodList.append(libName)
            print("4444444444444444444444444444", file=sys.stderr)
            print("  adding libName: ", libName, file=sys.stderr)
            print("4444444444444444444444444444", file=sys.stderr)
        
    print("1111111111111111111111111111111111111111", file=sys.stderr)
    print("  libsList: ", libsList, file=sys.stderr)
    print("  extendedList: ", extendedList, file=sys.stderr)
    print("  goodList: ", goodList, file=sys.stderr)
    print("1111111111111111111111111111111111111111", file=sys.stderr)

    # write out amended makefile

    print("88888888888888888888888888", file=sys.stderr)
    print("  len(goodList): ", len(goodList), file=sys.stderr)
    print("  len(libsList): ", len(libsList), file=sys.stderr)

    modified = False
    if (len(goodList) != len(libsList)):
        modified = True

    if (len(goodList) >= len(libsList)):
        for index, libName in enumerate(libsList):
            if (libName != goodList[index]):
                modified = True
                
    if (len(libsList) >= len(goodList)):
        for index, libName in enumerate(goodList):
            if (libName != libsList[index]):
                modified = True
                
    print("  goodList: ", goodList, file=sys.stderr)
    print("  libsList: ", libsList, file=sys.stderr)
    print("  modified: ", modified, file=sys.stderr)

    if (modified):
        overwriteMakefile(makefilePath, goodList, lines, lineNumsLibs)

    return
                    
########################################################################
# create the list of ordered libraries

def createOrderedLibList():

    global orderedLibList
    orderedLibList = []

    orderedLibList.append('-lmm5')
    orderedLibList.append('-lRefract')
    orderedLibList.append('-lFiltAlgVirtVol')
    orderedLibList.append('-lFiltAlg')
    orderedLibList.append('-ldsdata')
    orderedLibList.append('-lradar')
    orderedLibList.append('-lhydro')
    orderedLibList.append('-ltitan')
    orderedLibList.append('-lMdv')
    orderedLibList.append('-lSpdb')
    orderedLibList.append('-lFmq')
    orderedLibList.append('-lrapformats')
    orderedLibList.append('-ladvect')
    orderedLibList.append('-lcidd')
    orderedLibList.append('-ldsserver')
    orderedLibList.append('-ldidss')
    orderedLibList.append('-lcontour')
    orderedLibList.append('-lgrib')
    orderedLibList.append('-lgrib2')
    orderedLibList.append('-leuclid')
    orderedLibList.append('-lrapmath')
    orderedLibList.append('-lrapplot')
    orderedLibList.append('-lqtplot')
    orderedLibList.append('-ltoolsa')
    orderedLibList.append('-ldataport')
    orderedLibList.append('-ltdrp')
    orderedLibList.append('-lRadx')
    orderedLibList.append('-lNcxx')
    orderedLibList.append('-lphysics')
    orderedLibList.append('-lshapelib')
    orderedLibList.append('-lkd')
    orderedLibList.append('-ldevguide')
    orderedLibList.append('-lxview')
    orderedLibList.append('-lolgx')
    orderedLibList.append('-ltrmm_rsl')
    orderedLibList.append('-lforayRal')

    
########################################################################
# get dependent libs for a specified library

def getDependentLibs(libName):

    if (libName == '-lmm5'):
        return ['-lMdv', '-ltoolsa', '-ldataport', '-lphysics']
    
    if (libName == '-lRefract'):
        return ['-ldsdata', '-lMdv', '-lrapmath', '-ltoolsa', '-ldataport', '-ltdrp']
    
    if (libName == '-lFiltAlgVirtVol'):
        return ['-ldsdata', '-lradar', '-lMdv', '-lSpdb', '-lrapformats', '-leuclid', '-lrapmath', '-ltoolsa', '-ltdrp']
    
    if (libName == '-lFiltAlg'):
        return ['-ldsdata', '-lMdv', '-lSpdb', '-lrapformats', '-leuclid', '-lrapmath', '-ltoolsa', '-ltdrp']
    
    if (libName == '-ldsdata'):
        return ['-lMdv', '-lSpdb', '-lFmq', '-lrapformats', '-ldsserver', '-ldidss', '-leuclid', '-lrapmath', '-ltoolsa', '-ldataport']
    
    if (libName == '-lradar'):
        return ['-lMdv', '-lSpdb', '-lFmq', '-lrapformats', '-ldidss', '-lrapmath', '-ltoolsa', '-ldataport', '-ltdrp', '-lRadx', '-lNcxx', '-lphysics']
    
    if (libName == '-lhydro'):
        return ['-lMdv', '-leuclid', '-ltoolsa', '-ldataport', '-lshapelib']
    
    if (libName == '-ltitan'):
        return ['-lMdv', '-lrapformats', '-ldsserver', '-ldidss', '-lrapmath', '-ltoolsa', '-ldataport']
    
    if (libName == '-lMdv'):
        return ['-lrapformats', '-ldsserver', '-ldidss', '-leuclid', '-ltoolsa', '-ldataport', '-lRadx', '-lNcxx']
    
    if (libName == '-lSpdb'):
        return ['-lrapformats', '-ldsserver', '-ldidss', '-leuclid', '-ltoolsa', '-ldataport']
    
    if (libName == '-lFmq'):
        return ['-lrapformats', '-ldsserver', '-ldidss', '-ltoolsa', '-ldataport']
    
    if (libName == '-lrapformats'):
        return ['-ldidss', '-leuclid', '-ltoolsa', '-ldataport', '-lphysics']
    
    if (libName == '-ladvect'):
        return ['-leuclid', '-ltoolsa', '-ldataport']
    
    if (libName == '-lcidd'):
        return ['-ldsserver', '-ltoolsa', '-ldataport']
    
    if (libName == '-ldsserver'):
        return ['-ldidss', '-ltoolsa', '-ldataport']
    
    if (libName == '-ldidss'):
        return ['-ltoolsa', '-ldataport']

    if (libName == '-lcontour'):
        return ['-leuclid']
    
    if (libName == '-lgrib'):
        return ['-leuclid', '-ltoolsa', '-ldataport']
    
    if (libName == '-lgrib2'):
        # return ['-ljasper']
        return []
    
    if (libName == '-leuclid'):
        return ['-lrapmath', '-ltoolsa']
    
    if (libName == '-lrapmath'):
        return ['-ltoolsa', '-ltdrp']
    
    if (libName == '-lrapplot'):
        return ['-ltoolsa']
    
    if (libName == '-lqtplot'):
        return ['-ltoolsa']
    
    if (libName == '-ltoolsa'):
        return ['-ldataport']
    
    if (libName == '-ldataport'):
        return []
    
    if (libName == '-ltdrp'):
        return []
    
    if (libName == '-lRadx'):
        return ['-lNcxx']
    
    if (libName == '-lNcxx'):
        return []

    if (libName == '-lphysics'):
        return []

    if (libName == '-lshapelib'):
        return []

    if (libName == '-lkd'):
        return []

    if (libName == '-ldevguide'):
        return ['-lxview']
    
    if (libName == '-lxview'):
        return ['-lolgx']
    
    if (libName == '-lolgx'):
        return []
    
    if (libName == '-ltrmm_rsl'):
        return []
    
    if (libName == '-lforayRal'):
        return []

    return []

########################################################################
# get string value based on search key
# the string may span multiple lines
#
# Example of keys: SRCS, SUB_DIRS, MODULE_NAME, TARGET_FILE
#
# value is returned

def getValueListForKey(path, key):

    valueList = []
    lineNumsLibs = []

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

    for lineNum, line in enumerate(lines):
        if (foundKey == False):
            if (line[0] == '#'):
                continue
        if (line.find(key) >= 0):
            foundKey = True
            multiLine = multiLine + line
            lineNumsLibs.append(lineNum)
            if (line.find("\\") < 0):
                break;
        elif (foundKey == True):
            if (line[0] == '#'):
                break
            if (len(line) < 2):
                break
            multiLine = multiLine + line;
            lineNumsLibs.append(lineNum)
            if (line.find("\\") < 0):
                break;

    if (foundKey == False):
        return (valueList, lines, lineNumsLibs)

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

    return (valueList, lines, lineNumsLibs)

########################################################################
# Write out amended Makefile

def overwriteMakefile(makefilePath, libsList, lines, lineNumsLibs):

    if (options.debug):
        print("  overwriting makefile: ", makefilePath, file=sys.stderr)
    if (options.verbose):
        print("  makefile contents: ", file=sys.stderr)
        for line in lines:
            print(line.rstrip(), file=sys.stderr)

    out = open(makefilePath, "w")
    libListWritten = False

    for lineNum, line in enumerate(lines):

        if (lineNum in lineNumsLibs):
            # write out libs list
            if (lineNum == lineNumsLibs[0]):
                writeLibsList(out, libsList)
        else:
            # echo other lines unchanged
            out.write(line)
                
    out.write("\n")
    out.close

########################################################################
# Write out the libs list to makefile

def writeLibsList(out, libsList):

    out.write("LOC_LIBS = \\\n\t")
    count = 0
    for libNum, lib in enumerate(libsList):
        if (count != 0):
            out.write(" ")
        out.write(lib)
        count = count + 1
        if (libNum == len(libsList) - 1):
            out.write("\n")
        elif (count == 4):
            out.write(" \\\n\t")
            count = 0

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
        print("Execution failed:", e, file=sys.stderr)
        exit(1)

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

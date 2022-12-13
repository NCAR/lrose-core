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
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  Top level dir: ", options.dir

    # search recursively for Makefile from dir down

    makefileList = []
    processDir(options.dir)

    if (options.debug):
        for path in makefileList:
            print >>sys.stderr, "  Makefile: ", path

    # process each Makefile

    for path in makefileList:
        fixMakefile(path)
            
    sys.exit(0)

########################################################################
# process this directory and its subdirectories

def processDir(dir):
                    
    global makefileList

    if (options.debug):
        print >>sys.stderr, "  Searching dir: ", dir

    # check if this dir has a makefile or Makefile

    makefilePath = getMakefilePath(dir)
    if (os.path.exists(makefilePath) == False):
        if (options.verbose):
            print >>sys.stderr, "  No Makefile or makefile found"
        return
        
    # detect which type of directory we are in
    
    if (options.verbose):
        print >>sys.stderr, "  Found makefile: ", makefilePath

    pathToks = dir.split("/")
    nToks = len(pathToks)

    if (options.verbose):
        print >>sys.stderr, "  pathToks: ", pathToks
        print >>sys.stderr, "  nToks: ", nToks
    
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

        print >>sys.stderr, "  subDirPaths: ", subDirPaths

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
        print >>sys.stderr, "Found makefile: ", makefilePath

    try:
        fp = open(makefilePath, 'r')
    except IOError as e:
        if (options.verbose):
            print >>sys.stderr, "ERROR - ", thisScriptName
            print >>sys.stderr, "  Cannot find makefile or Makefile"
            print >>sys.stderr, "  dir: ", options.dir
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

    subDirPaths = filter(os.path.isdir, [os.path.join(dir,f) for f in os.listdir(dir)])
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

    modified = False

    # parse make file for local libs

    (libsList, lines, lineNumsLibs) = getValueListForKey(makefilePath, "LOC_LIBS")

    print >>sys.stderr, "============================="
    print >>sys.stderr, "  path: ", makefilePath
    print >>sys.stderr, "  libsList: ", libsList
    print >>sys.stderr, "  lines: ", lines
    print >>sys.stderr, "  lineNumsLibs: ", lineNumsLibs

    # check for NETCDF_LIBS and NETCDF4_LIBS

    for index, line in enumerate(lines):
        libs3 = line.find('NETCDF_LIBS')
        if (libs3 >= 0):
            modLine = line[:libs3] + 'NETCDF_LDFLAGS' + line[libs3 + len('NETCDF_LIBS'):]
            modified = True
            print >>sys.stderr, "====>> REPLACING NETCDF_LIBS with NETCDF_LDFLAGS"
            print >>sys.stderr, "  old line: ", line
            print >>sys.stderr, "  mod line: ", modLine
            lines[index] = modLine
        libs4 = line.find('NETCDF4_LIBS')
        if (libs4 >= 0):
            modLine = line[:libs4] + 'NETCDF4_LDFLAGS' + line[libs4 + len('NETCDF4_LIBS'):]
            modified = True
            print >>sys.stderr, "====>> REPLACING NETCDF4_LIBS with NETCDF4_LDFLAGS"
            print >>sys.stderr, "  old line: ", line
            print >>sys.stderr, "  mod line: ", modLine
            lines[index] = modLine

    # detect if various libs are used / to be used

    useNcfMdv = False
    useMdv = False
    useSpdb = False
    useNetcdf4 = False
    useToolsa = False
    useZlib = False
    useBzip2 = False
    usePthread = False
    useMath = False
    useRadx = False

    for index, lib in enumerate(libsList):
        if (lib == '-lNcfMdv'):
            useNcfMdv = True
            useNetcdf4 = True
            useZlib = True
            useBzip2 = True
        if (lib == '-lRadx'):
            useRadx = True
        elif (lib == '-lMdv'):
            useMdv = True
            useNetcdf4 = True
            useZlib = True
            useBzip2 = True
        elif (lib == '-lSpdb'):
            useSpdb = True
            useNetcdf4 = True
            useZlib = True
            useBzip2 = True
        elif (lib == '$(NETCDF4_LIB_LIST)'):
            useNetcdf4 = True
            useZlib = True
            useBzip2 = True
        elif (lib == '-lm'):
            useMath = True
        elif (lib == '-lz'):
            useZlib = True
        elif (lib == '-lbz2'):
            useBzip2 = True
        elif (lib == '-ltoolsa'):
            useToolsa = True
            useLibz = True
            useBzip2 = True
        elif (lib == '-lpthread'):
            usePthread = True

    # remove certain libs or change vals

    for index, lib in enumerate(libsList):
        if (lib == '$(TDRP_LIBS)'):
            libsList[index] = '-ltdrp'
            modified = True
            break

    for index, lib in enumerate(libsList):
        if (lib == '-lNcfMdv'):
            del libsList[index]
            modified = True
            break

    for index, lib in enumerate(libsList):
        if (lib == '-lRadx'):
            del libsList[index]
            modified = True
            break

    for index, lib in enumerate(libsList):
        if (lib == '$(NETCDF4_LIB_LIST)'):
            del libsList[index]
            modified = True
            break

    for index, lib in enumerate(libsList):
        if (lib == '-lm'):
            del libsList[index]
            modified = True
            break

    for index, lib in enumerate(libsList):
        if (lib == '-lz'):
            del libsList[index]
            modified = True
            break

    for index, lib in enumerate(libsList):
        if (lib == '-lbz2'):
            del libsList[index]
            modified = True
            break

    for index, lib in enumerate(libsList):
        if (lib == '-lpthread'):
            del libsList[index]
            modified = True
            break

    # append to list in correct order

    if (useMdv):
        libsList.append('-lMdv')
        libsList.append('-lRadx')
        libsList.append('-lNcxx')
        modified = True
    elif (useRadx):
        libsList.append('-lRadx')
        libsList.append('-lNcxx')
        modified = True

    if (useNetcdf4):
        libsList.append('$(NETCDF4_LIBS)')
        modified = True

    if (useBzip2):
        libsList.append('-lbz2')
        modified = True

    if (useZlib):
        libsList.append('-lz')
        modified = True

    if (usePthread):
        libsList.append('-lpthread')
        modified = True

    if (useMath):
        libsList.append('-lm')
        modified = True

    # write out amended makefile

    if (modified):
        overwriteMakefile(makefilePath, libsList, lines, lineNumsLibs)

    return
                    
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
        print >>sys.stderr, "ERROR - ", thisScriptName
        print >>sys.stderr, "  Cannot open file:", path
        print >>sys.stderr, "  dir: ", options.dir
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
                print >>sys.stderr, "  overwriting makefile: ", makefilePath
                print >>sys.stderr, "  lines: ", lines

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
        print >>sys.stderr, "running cmd:",cmd
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print >>sys.stderr, "Child exited with code: ", retcode
            exit(1)
        else:
            if (options.verbose):
                print >>sys.stderr, "Child returned code: ", retcode
    except OSError, e:
        print >>sys.stderr, "Execution failed:", e
        exit(1)

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

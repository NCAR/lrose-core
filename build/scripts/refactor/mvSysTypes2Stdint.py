#!/usr/bin/env python3

#===========================================================================
#
# Change <sys/types.h> to <cstdint> in recursive search
#
#===========================================================================

import os
import sys
import subprocess
from optparse import OptionParser
from datetime import datetime
import time
import math

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
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--verbose',
                      dest='verbose', default=False,
                      action="store_true",
                      help='Set verbose debugging on')
    parser.add_option('--dir',
                      dest='dir', default=".",
                      help='Path of top level directory')

    (options, args) = parser.parse_args()

    if (options.verbose):
        options.debug = True
    
    if (options.debug):
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  Top level dir: ", options.dir

    # process from this dir down

    processDir(options.dir)

    sys.exit(0)

########################################################################
# process this directory and its subdirectories

def processDir(topDir):

    dir_count = 0
    file_count = 0

    # Traverse directory tree
    for (path, dirs, files) in os.walk(topDir):

        print('Directory: {:s}'.format(path))
        dir_count += 1

        # Repeat for each file in directory
        for file in files:
            fstat = os.stat(os.path.join(path,file))

            # Convert file size to MB, KB or Bytes

            if (fstat.st_size > 1024 * 1024):
                fsize = math.ceil(fstat.st_size / (1024 * 1024))
                unit = "MB"
            elif (fstat.st_size > 1024):
                fsize = math.ceil(fstat.st_size / 1024)
                unit = "KB"
            else:
                fsize = fstat.st_size
                unit = "B"
            
            mtime = time.strftime("%X %x", time.gmtime(fstat.st_mtime))
        
            # Print file attributes
            
            print('\t{:15.15s}{:8d} {:2s} {:18s}'.format(file, int(fsize), unit, mtime))
            file_count += 1

    # Print total files and directory count

    print('\nFound {} files in {} directories.'.format(file_count,dir_count))    


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

    # read in makefile

    print >>sys.stderr, "============================="
    print >>sys.stderr, "  makefile path: ", makefilePath

    lines = readMakefile(makefilePath)
    if (len(lines) < 5):
        print >>sys.stderr, "ERROR - short makefile: ", makefilePath
        for line in lines:
            print >>sys.stderr, line,
        return

    # strip empty lines from end of file

    lastNonEmpty = 0
    for index, line in enumerate(lines):
        if (len(line) > 1):
            lastNonEmpty = index

    saved = lines
    lines = []
    for index, line in enumerate(saved):
        if (index < lastNonEmpty + 2):
            lines.append(saved[index])

    # check for NETCDF4_LIB_LIST

    usingNetcdf4 = False
    for index, line in enumerate(lines):
        if (line.find('NETCDF4_LIB_LIST') >= 0):
            usingNetcdf4 = True
            break
        if (line.find('NETCDF4_LIBS') >= 0):
            usingNetcdf4 = True
            break

    print >>sys.stderr, "  usingNetcdf4: ", usingNetcdf4
    if (usingNetcdf4 == False):
        return

    # parse make file for local libs

    (libsList, lineNumsLibs) = getValueListForKey(lines, "LOC_LIBS")
    (incsList, lineNumsIncs) = getValueListForKey(lines, "LOC_INCLUDES")
    (ldflagsList, lineNumsLdflags) = getValueListForKey(lines, "LOC_LDFLAGS")

    print >>sys.stderr, "====>> Makefile: ", makefilePath
    for line in lines:
        print >>sys.stderr, line,

    print >>sys.stderr, "  libsList: ", libsList
    print >>sys.stderr, "  incsList: ", incsList
    print >>sys.stderr, "  ldflagsList: ", ldflagsList
    print >>sys.stderr, "  lineNumsLibs: ", lineNumsLibs
    print >>sys.stderr, "  lineNumsIncs: ", lineNumsIncs
    print >>sys.stderr, "  lineNumsLdflags: ", lineNumsLdflags

    # check for NETCDF4_LIB_LIST in libs list
    
    haveNetcdf4InLibsList = False
    for entry in libsList:
        if (entry.find('NETCDF4_LIB_LIST') >= 0):
            haveNetcdf4InLibsList = True
            break
    print >>sys.stderr, "  haveNetcdf4InLibsList: ", haveNetcdf4InLibsList

    haveNetcdfLibs = False
    for entry in libsList:
        if (entry.find('NETCDF4_LIBS') >= 0):
            haveNetcdfLibs = True
            break
    print >>sys.stderr, "  haveNetcdfLibs: ", haveNetcdfLibs
    
    haveNetcdf4InIncsList = False
    for entry in incsList:
        if (entry.find('NETCDF4_INCS') >= 0):
            haveNetcdf4InIncsList = True
            break
    print >>sys.stderr, "  haveNetcdf4InIncsList: ", haveNetcdf4InIncsList
    
    haveNetcdf4InLdflagsList = False
    for entry in ldflagsList:
        if (entry.find('NETCDF4_LDFLAGS') >= 0):
            haveNetcdf4InLdflagsList = True
            break
    print >>sys.stderr, "  haveNetcdf4InLdflagsList: ", haveNetcdf4InLdflagsList
    
    # create lines for each of incs, libs and ldflags

    newLibsList = []
    libListEntryFound = False
    for entry in libsList:
        if (entry.find('NETCDF4_LIB_LIST') < 0):
            newLibsList.append(entry)
        else:
            # change from LIB_LIST to LIBS
            newLibsList.append('$(NETCDF4_LIBS)')
            libListEntryFound = True
#    if (libListEntryFound == False):
#        newLibsList.append('$(NETCDF4_LIBS)')

    newIncsList = []
    incsEntryFound = False
    for entry in incsList:
        if (entry.find('NETCDF4_INCS') < 0):
            newIncsList.append(entry)
        else:
            newIncsList.append('$(NETCDF4_INCS)')
            incsEntryFound = True
    if (incsEntryFound == False):
        newIncsList.append('$(NETCDF4_INCS)')

    newLdflagsList = []
    ldflagsEntryFound = False
    for entry in ldflagsList:
        if (entry.find('NETCDF4_LDFLAGS') < 0):
            newLdflagsList.append(entry)
        else:
            newLdflagsList.append('$(NETCDF4_LDFLAGS)')
            ldflagsEntryFound = True
    if (ldflagsEntryFound == False):
        newLdflagsList.append('$(NETCDF4_LDFLAGS)')


    print >>sys.stderr, "newLibsList: ", newLibsList
    print >>sys.stderr, "newIncsList: ", newIncsList
    print >>sys.stderr, "newLdflagsList: ", newLdflagsList

    # write out amended makefile

    lineNumsReplaced = lineNumsLibs + lineNumsIncs + lineNumsLdflags
    lineNumsReplaced.sort()
    print >>sys.stderr, "lineNumsReplaced: ", lineNumsReplaced

    overwriteMakefile(makefilePath, lines, lineNumsReplaced,
                      newIncsList, newLibsList, newLdflagsList)

    return
                    
########################################################################
# read in makefile
# returns the lines in the file

def readMakefile(path):

    lines = []

    try:
        fp = open(path, 'r')
    except IOError as e:
        print >>sys.stderr, "ERROR - ", thisScriptName
        print >>sys.stderr, "  Cannot open file:", path
        print >>sys.stderr, "  dir: ", options.dir
        return lines

    lines = fp.readlines()
    fp.close()

    return lines

########################################################################
# get string value from Makefile lines, based on search key
# the string may span multiple lines
#
# Example of keys: SRCS, SUB_DIRS, MODULE_NAME, TARGET_FILE
#
# value is returned

def getValueListForKey(lines, key):

    valueList = []
    lineNumsUsed = []

    foundKey = False
    multiLine = ""

    for lineNum, line in enumerate(lines):
        if (foundKey == False):
            if (line[0] == '#'):
                continue
        if (line.find(key) >= 0):
            foundKey = True
            multiLine = multiLine + line
            lineNumsUsed.append(lineNum)
            if (line.find("\\") < 0):
                break;
        elif (foundKey == True):
            if (line[0] == '#'):
                break
            if (len(line) < 2):
                break
            multiLine = multiLine + line;
            lineNumsUsed.append(lineNum)
            if (line.find("\\") < 0):
                break;

    if (foundKey == False):
        return (valueList, lineNumsUsed)

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

    return (valueList, lineNumsUsed)

########################################################################
# Write out amended Makefile

def overwriteMakefile(makefilePath, lines, lineNumsReplaced,
                      newIncsList, newLibsList, newLdflagsList):

    if (options.debug):
        print >>sys.stderr, "  overwriting makefile: ", makefilePath
        print >>sys.stderr, "  lines: ", lines

    out = open(makefilePath, "w")

    newWritten = False
    for lineNum, line in enumerate(lines):

        if (lineNum in lineNumsReplaced):
            if (newWritten == False):
                # write out new lists
                writeNewList(out, "LOC_INCLUDES", newIncsList)
                writeNewList(out, "LOC_LIBS", newLibsList)
                writeNewList(out, "LOC_LDFLAGS", newLdflagsList)
            newWritten = True
        else:
            # echo other lines unchanged
            out.write(line)
                
    out.close

########################################################################
# Write out the libs list to makefile

def writeNewList(out, name, newList):

    out.write(name + " = ")
    count = 0

    for entryNum, entry in enumerate(newList):
        if (count != 0):
            out.write(" ")
        out.write(entry)
        count = count + 1
        if (entryNum == len(newList) - 1):
            out.write("\n")
        elif (count == 4):
            out.write(" \\\n\t")
            count = 0

    out.write("\n")


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

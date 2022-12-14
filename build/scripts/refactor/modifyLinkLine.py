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
    print("6666666666666666666666666666666666", file=sys.stderr)
    print("  orderedLibList: ", orderedLibList, file=sys.stderr)
    print("6666666666666666666666666666666666", file=sys.stderr)

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

    modified = False
    
    # parse make file for local libs

    (libsList, lines, lineNumsLibs) = getValueListForKey(makefilePath, "LOC_LIBS")

    print("=============================", file=sys.stderr)
    print("  path: ", makefilePath, file=sys.stderr)
    print("  libsList: ", libsList, file=sys.stderr)
    print("  lines: ", lines, file=sys.stderr)
    print("  lineNumsLibs: ", lineNumsLibs, file=sys.stderr)

    # augment the libsList with dependencies that are not already included

    extendedList = libsList
    for libName in libsList:
        dependList = getDependentLibs(libName)
        for dependName in dependList:
            if dependName not in extendedList:
                extendedList.append(dependName)
                print("2222222222222222222222222222", file=sys.stderr)
                print("  libName: ", libName, file=sys.stderr)
                print("  dependName: ", dependName, file=sys.stderr)
                print("2222222222222222222222222222", file=sys.stderr)
    print("2222222222222222222222222222", file=sys.stderr)
    print("  libsList: ", libsList, file=sys.stderr)
    print("  extendedList: ", extendedList, file=sys.stderr)
    print("  orderedLibList: ", orderedLibList, file=sys.stderr)
    print("2222222222222222222222222222", file=sys.stderr)

    # create lrose libs list in the correct order

    goodList = []
    for libName in orderedLibList:
        entry = "-l" + libName
        if (entry in extendedList):
            goodList.append(entry)
            print("3333333333333333333333333333", file=sys.stderr)
            print("  adding libName: ", libName, file=sys.stderr)
            print("3333333333333333333333333333", file=sys.stderr)
    print("3333333333333333333333333333", file=sys.stderr)
    print("  goodList: ", goodList, file=sys.stderr)
    print("3333333333333333333333333333", file=sys.stderr)

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

    # sys.exit(1)
    
    # check for NETCDF4_LIBS

    #for index, line in enumerate(lines):
    #    libs4 = line.find('NETCDF4_LIBS')
    #    if (libs4 >= 0):
    #        modLine = line[:libs4] + 'NETCDF4_LDFLAGS' + line[libs4 + len('NETCDF4_LIBS'):]
    #        modified = True
    #        print("====>> REPLACING NETCDF4_LIBS with NETCDF4_LDFLAGS", file=sys.stderr)
    #        print("  old line: ", line, file=sys.stderr)
    #        print("  mod line: ", modLine, file=sys.stderr)
    #        lines[index] = modLine

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
# create the list of ordered libraries

def createOrderedLibList():

    global orderedLibList
    orderedLibList = []

    orderedLibList.append('mm5')
    orderedLibList.append('Refract')
    orderedLibList.append('FiltAlgVirtVol')
    orderedLibList.append('FiltAlg')
    orderedLibList.append('dsdata')
    orderedLibList.append('radar')
    orderedLibList.append('hydro')
    orderedLibList.append('titan')
    orderedLibList.append('Mdv')
    orderedLibList.append('Spdb')
    orderedLibList.append('Fmq')
    orderedLibList.append('rapformats')
    orderedLibList.append('advect')
    orderedLibList.append('cidd')
    orderedLibList.append('dsserver')
    orderedLibList.append('didss')
    orderedLibList.append('contour')
    orderedLibList.append('grib')
    orderedLibList.append('grib2')
    orderedLibList.append('euclid')
    orderedLibList.append('rapmath')
    orderedLibList.append('rapplot')
    orderedLibList.append('qtplot')
    orderedLibList.append('toolsa')
    orderedLibList.append('dataport')
    orderedLibList.append('tdrp')
    orderedLibList.append('Radx')
    orderedLibList.append('Ncxx')
    orderedLibList.append('physics')
    orderedLibList.append('shapelib')
    orderedLibList.append('kd')
    orderedLibList.append('devguide')
    orderedLibList.append('xview')
    orderedLibList.append('olgx')
    orderedLibList.append('trmm_rsl')
    orderedLibList.append('forayRal')

    
########################################################################
# get dependent libs for a specified library

def getDependentLibs(libName):

    if (libName == 'mm5'):
        return ['Mdv', 'toolsa', 'dataport', 'physics']
    
    if (libName == 'Refract'):
        return ['dsdata', 'Mdv', 'rapmath', 'toolsa', 'dataport', 'tdrp']
    
    if (libName == 'FiltAlgVirtVol'):
        return ['dsdata', 'radar', 'Mdv', 'Spdb', 'rapformats', 'euclid', 'rapmath', 'toolsa', 'tdrp']
    
    if (libName == 'FiltAlg'):
        return ['dsdata', 'Mdv', 'Spdb', 'rapformats', 'euclid', 'rapmath', 'toolsa', 'tdrp']
    
    if (libName == 'dsdata'):
        return ['Mdv', 'Spdb', 'Fmq', 'rapformats', 'dsserver', 'didss', 'euclid', 'rapmath', 'toolsa', 'dataport']
    
    if (libName == 'radar'):
        return ['Mdv', 'Spdb', 'Fmq', 'rapformats', 'didss', 'rapmath', 'toolsa', 'dataport', 'tdrp', 'Radx', 'Ncxx', 'physics']
    
    if (libName == 'hydro'):
        return ['Mdv', 'euclid', 'toolsa', 'dataport', 'shapelib']
    
    if (libName == 'titan'):
        return ['Mdv', 'rapformats', 'dsserver', 'didss', 'rapmath', 'toolsa', 'dataport']
    
    if (libName == 'Mdv'):
        return ['rapformats', 'dsserver', 'didss', 'euclid', 'toolsa', 'dataport', 'Radx', 'Ncxx']
    
    if (libName == 'Spdb'):
        return ['rapformats', 'dsserver', 'didss', 'euclid', 'toolsa', 'dataport']
    
    if (libName == 'Fmq'):
        return ['rapformats', 'dsserver', 'didss', 'toolsa', 'dataport']
    
    if (libName == 'rapformats'):
        return ['didss', 'euclid', 'toolsa', 'dataport', 'physics']
    
    if (libName == 'advect'):
        return ['euclid', 'toolsa', 'dataport']
    
    if (libName == 'cidd'):
        return ['dsserver', 'toolsa', 'dataport']
    
    if (libName == 'dsserver'):
        return ['didss', 'toolsa', 'dataport']
    
    if (libName == 'didss'):
        return ['toolsa', 'dataport']

    if (libName == 'contour'):
        return ['euclid']
    
    if (libName == 'grib'):
        return ['euclid', 'toolsa', 'dataport']
    
    if (libName == 'grib2'):
        # return ['jasper']
        return []
    
    if (libName == 'euclid'):
        return ['rapmath', 'toolsa']
    
    if (libName == 'rapmath'):
        return ['toolsa', 'tdrp']
    
    if (libName == 'rapplot'):
        return ['toolsa']
    
    if (libName == 'qtplot'):
        return ['toolsa']
    
    if (libName == 'toolsa'):
        return ['dataport']
    
    if (libName == 'dataport'):
        return []
    
    if (libName == 'tdrp'):
        return []
    
    if (libName == 'Radx'):
        return ['Ncxx']
    
    if (libName == 'Ncxx'):
        return []

    if (libName == 'physics'):
        return []

    if (libName == 'shapelib'):
        return []

    if (libName == 'kd'):
        return []

    if (libName == 'devguide'):
        return ['xview']
    
    if (libName == 'xview'):
        return ['olgx']
    
    if (libName == 'olgx'):
        return []
    
    if (libName == 'trmm_rsl'):
        return []
    
    if (libName == 'forayRal'):
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
                print("  lines: ", lines, file=sys.stderr)

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

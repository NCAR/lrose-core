#!/usr/bin/env python

#===========================================================================
#
# Create CMakeLists.txt for a LROSE lib
#
#===========================================================================

from __future__ import print_function
import os
import sys
import shutil
import subprocess
import platform
from optparse import OptionParser
from datetime import datetime

class LibSubDir:
    def __init__(self, subDirName, makefilePath):
        self.subDirName = subDirName
        self.makefilePath = makefilePath

class IncludeLib:
    def __init__(self, name, used):
        self.name = name
        self.used = used

def main():

    global options
    global thisLibName

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

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
                      help='Path of lib directory')
    parser.add_option('--libList',
                      dest='libList', default="",
                      help='List of libs in package')
    parser.add_option('--static',
                      dest='static', default=False,
                      action="store_true",
                      help='Create static lib objects')

    (options, args) = parser.parse_args()

    if (options.verbose):
        options.debug = True
    
    if (options.debug):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  Lib dir: ", options.dir, file=sys.stderr)
        print("  Lib list: ", options.libList, file=sys.stderr)

    # set up list of other libs which may be used for include

    if (len(options.libList) < 1):
        libList = [ 'FiltAlg', 'Fmq', 'Mdv', 'Radx', 'Solo', 'Ncxx',
                    'Refract', 'Spdb', 'SpdbServer', 'advect', 
                    'cidd', 'contour',
                    'dataport', 'devguide', 'didss', 'dsdata', 'dsserver',
                    'euclid', 'forayRal', 'grib', 'grib2', 'hydro',
                    'mdv', 'niwot_basic', 'niwot_util', 
                    'olgx', 'physics', 'radar',
                    'rapformats', 'rapmath', 'rapplot', 'rdi', 'shapelib',
                    'spdb', 'spdbFormats', 'symprod', 'tdrp', 'titan',
                    'toolsa', 'trmm_rsl', 'xview' ]
    else:
        libList = options.libList.split(",")

    if (options.debug):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  App dir:", options.dir, file=sys.stderr)
        print("  Lib list: ", libList, file=sys.stderr)

    # go to the src dir

    libSrcDir = os.path.join(options.dir, 'src')
    if (options.debug):
        print("src dir: ", libSrcDir, file=sys.stderr)
    os.chdir(libSrcDir)

    # get makefile in use
    # makefile has preference over Makefile

    makefilePath = getMakefilePath(libSrcDir)
    if (makefilePath.find('not_found') == 0):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find makefile or Makefile", file=sys.stderr)
        print("  dir: ", options.dir, file=sys.stderr)
        exit(1)

    # get the lib name

    thisLibName = getLibName(libSrcDir, makefilePath)
    if (options.debug):
        print("  Lib name: ", thisLibName, file=sys.stderr)

    # get list of subdirs and their makefiles

    subDirList = getLibSubDirs(libSrcDir)
    if (options.debug):
        print("=======================", file=sys.stderr)
        for subDir in subDirList:
            print("subDir, makefile: %s, %s" % \
                (subDir.subDirName, subDir.makefilePath), file=sys.stderr)
        print("=======================", file=sys.stderr)

    # load list of files to be compiled

    compileFileList = []
    for subDir in subDirList:
        addSubDirToCompileList(subDir, compileFileList)

    if (options.debug):
        print("=======================", file=sys.stderr)
        for compileFile in compileFileList:
            print("compileFile: %s" % (compileFile), file=sys.stderr)
        print("=======================", file=sys.stderr)

    # get list of header files

    # headerFileList = loadHeaderFileList()
    # if (options.debug):
    #     print("=======================", file=sys.stderr)
    #     for headerFile in headerFileList:
    #         print("headerFile: %s" % (headerFile), file=sys.stderr)
    #     print("=======================", file=sys.stderr)

    # write out CMakeLists.txt

    writeCMakeListsLib(libSrcDir, libList, compileFileList)

    sys.exit(0)

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

    # copy makefile in case we rerun this script

    if (makefilePath.find('__makefile.template') < 0):
        templatePath = os.path.join(dir, '__makefile.template')
        shutil.copy(makefilePath, templatePath)

    if (options.debug):
        print("-->> using makefile template: ", makefilePath, file=sys.stderr)

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
        print("  dir: ", options.dir, file=sys.stderr)
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
# parse the LROSE Makefile to get the lib name

def getLibName(dir, makefilePath):

    # search for MODULE_NAME key in makefile

    valList = getValueListForKey(makefilePath, "MODULE_NAME")

    if (len(valList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find MODULE_NAME in ", makefileName, file=sys.stderr)
        print("  dir: ", dir, file=sys.stderr)
        exit(1)

    libName = valList[len(valList)-1]
    return libName

########################################################################
# get list of makefiles for library
# using LROSE Makefile to locate subdirs

def getLibSubDirs(libDir):

    libSubDirs = []

    # search for SUB_DIRS key in makefile

    makefilePath = getMakefilePath(libDir)
    subNameList = getValueListForKey(makefilePath, "SUB_DIRS")

    if (len(subNameList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find SUB_DIRS in ", makefilePath, file=sys.stderr)
        print("  libDir: ", libDir, file=sys.stderr)
        exit(1)

    for subName in subNameList:
        if (os.path.isdir(subName)):
            subMakefilePath = os.path.join(subName, 'makefile')
            if (os.path.isfile(subMakefilePath)):
                subDir = LibSubDir(subName, subMakefilePath)
                libsSubDirs.append(subDir)
            else:
                subMakefilePath = os.path.join(subName, 'Makefile')
                if (os.path.isfile(subMakefilePath)):
                    subDir = LibSubDir(subName, subMakefilePath)
                    libSubDirs.append(subDir)

    return libSubDirs

########################################################################
# append to list of files to be compiled

def addSubDirToCompileList(subDir, compileFileList):
                    
    srcTypeList = [ 'SRCS', 'C_SRCS', 'F_SRCS', 'F_CPP_SRCS', 
                    'F90_SRCS', 'F95_SRCS', 'PGF90_SRCS', 
                    'PGF_SRCS', 'CC_SRCS', 'CPPC_SRCS', 
                    'CPP_SRCS', 'CXX_SRCS' ]
    
    fp = open(subDir.makefilePath, 'r')
    lines = fp.readlines()

    for srcType in srcTypeList:
        handleSrcType(subDir, lines, srcType, compileFileList)
    
########################################################################
# append to compile list for given srcType

def handleSrcType(subDir, lines, srcType, compileFileList):

    # build up multiLine string containing all compile files

    srcTypeFound = False
    multiLine = ""
    for line in lines:
        line = line.strip()
        if (srcTypeFound == False):
            if (len(line) < 2):
                continue
            if (line[0] == '#'):
                continue
            if (line.find(srcType) == 0):
                srcTypeFound = True
                multiLine = multiLine + line;
                if (line.find("\\") < 0):
                    break;
        else:
            if (len(line) < 2):
                break
            if (line[0] == '#'):
                break
            multiLine = multiLine + line;
            if (line.find("\\") < 0):
                break;
            
    if (srcTypeFound == False):
        return

    # remove strings we don't want

    multiLine = multiLine.replace(srcType, " ")
    multiLine = multiLine.replace("=", " ")
    multiLine = multiLine.replace("\t", " ")
    multiLine = multiLine.replace("\\", " ")
    multiLine = multiLine.replace("\r", " ")
    multiLine = multiLine.replace("\n", " ")

    toks = multiLine.split(' ')
    for tok in toks:
        if (tok.find(".") > 0):
            compileFilePath = os.path.join(subDir.subDirName, tok)
            compileFileList.append(compileFilePath)

########################################################################
# load up header file list

# def loadHeaderFileList():

#     headerFileList = []

#     incDirList = os.listdir("include")

#     for incDir in incDirList:
#         incPath = os.path.join("include", incDir)
#         if ((os.path.isdir(incPath)) and (incDir != "CVS")):
#             appendToHeaderFileList(incPath, headerFileList)

#     return headerFileList

########################################################################
# append to header file list

# def appendToHeaderFileList(dir, headerFileList):

#     fileList = os.listdir(dir)
#     for fileName in fileList:
#         last2 = fileName[-2:]
#         last3 = fileName[-3:]
#         if (last2 == ".h") or (last3 == ".hh"):
#             filePath = os.path.join(dir, fileName)
#             headerFileList.append(filePath)

########################################################################
# Write out CMakeLists.txt

def writeCMakeListsLib(libDir, libList, compileFileList):

    cmakePath = os.path.join(libDir, 'CMakeLists.txt')
    fo = open(cmakePath, "w")

    fo.write("###############################################\n")
    fo.write("#\n")
    fo.write("# CMakeLists.txt - auto generated from Makefile\n")
    fo.write("#\n")
    fo.write("# library name: %s\n" % thisLibName)
    fo.write("#\n")
    fo.write("# written by script createCMakeLists.lib.py\n")
    fo.write("#\n")
    fo.write("# created %s\n" % datetime.now())
    fo.write("#\n")
    fo.write("###############################################\n")
    fo.write("\n")

    fo.write("project ( lib%s )\n" % thisLibName)
    fo.write("\n")
    
    fo.write("# include directories\n")
    fo.write("\n")
    fo.write("include_directories ( ./include )\n")
    for lib in libList:
        fo.write("include_directories ( ../../%s/src/include )\n" % lib)
    fo.write("include_directories ( $ENV{LROSE_INSTALL_DIR}/include )\n")
    fo.write("\n")

    fo.write("# source files\n")
    fo.write("\n")
    fo.write("set ( SRCS\n")
    for index, compileFile in enumerate(compileFileList):
        fo.write("      %s\n" % compileFile)
    fo.write("    )\n")
    fo.write("\n")

    fo.write("# build library\n")
    fo.write("\n")
    if (options.static):
        fo.write("add_library (%s STATIC ${SRCS} )\n" % thisLibName)
    else:
        fo.write("add_library (%s SHARED ${SRCS} )\n" % thisLibName)
    fo.write("\n")

    fo.write("# install\n")
    fo.write("\n")
    fo.write("INSTALL(TARGETS %s\n" % thisLibName)
    fo.write("        DESTINATION $ENV{LROSE_INSTALL_DIR}/lib\n")
    fo.write("        )\n")
    fo.write("INSTALL(DIRECTORY include/%s\n" % thisLibName)
    fo.write("        DESTINATION $ENV{LROSE_INSTALL_DIR}/include\n")
    fo.write("        )\n")
    fo.write("\n")

    fo.close
    return

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

#!/usr/bin/env python

#===========================================================================
#
# Create makefile.am for a RAL lib
#
#===========================================================================

import os
import sys
import shutil
import subprocess
from optparse import OptionParser
from datetime import datetime

class SubDir:
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
    global makefileName
    global subDirList
    global compileFileList
    global headerFileList
    global includeList
    global libList
    global includesInSubDir

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

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
    parser.add_option('--dir',
                      dest='dir', default=".",
                      help='Path of lib directory')
    parser.add_option('--libList',
                      dest='libList', default="",
                      help='List of libs in package')
    parser.add_option('--shared',
                      dest='shared', default='False',
                      action="store_true",
                      help='Create shared lib objects')

    (options, args) = parser.parse_args()

    if (options.verbose == True):
        options.debug = True
    
    if (options.debug == True):
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  Lib dir: ", options.dir
        print >>sys.stderr, "  Lib list: ", options.libList

    # set up list of other libs which may be used for include

    if (len(options.libList) < 1):
        libList = [ 'FiltAlg', 'Fmq', 'Mdv', 'Radx',
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

#    includeList = []
#    for libName in libNames:
#        lib = IncludeLib(libName, True)
#        includeList.append(lib)
     
    if (options.debug == True):
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  App dir:", options.dir
        print >>sys.stderr, "  Lib list: ", libList

    # go to the src dir

    srcDir = os.path.join(options.dir, 'src')
    if (options.debug == True):
        print >>sys.stderr, "src dir: ", srcDir
    os.chdir(srcDir)

    # get makefile name in use
    # makefile has preference over Makefile

    makefileName = '__makefile.template'
    if (os.path.exists(makefileName) == False):
        makefileName = 'makefile'
        if (os.path.exists(makefileName) == False):
            makefileName = 'Makefile'
            if (os.path.exists(makefileName) == False):
                print >>sys.stderr, "ERROR - ", thisScriptName
                print >>sys.stderr, "  Cannot find makefile or Makefile"
                print >>sys.stderr, "  dir: ", options.dir
                exit(1)

    # copy makefile in case we rerun this script

    if (makefileName != "__makefile.template"):
        shutil.copy(makefileName, "__makefile.template")

    if (options.debug == True):
        print >>sys.stderr, "-->> using makefile template: ", makefileName

    # get the lib name

    thisLibName = ""
    getLibName()
    if (options.debug == True):
        print >>sys.stderr, "  Lib name: ", thisLibName

    # get list of subdirs and their makefiles

    getSubDirList()

    if (options.debug == True):
        print >>sys.stderr, "======================="
        for subDir in subDirList:
            print >>sys.stderr, "subDir, makefile: %s, %s" % \
                (subDir.subDirName, subDir.makefilePath)
        print >>sys.stderr, "======================="

    # load list of files to be compiled

    compileFileList = []
    for subDir in subDirList:
        addSubDirToCompileList(subDir)

    if (options.debug == True):
        print >>sys.stderr, "======================="
        for compileFile in compileFileList:
            print >>sys.stderr, "compileFile: %s" % (compileFile)
        print >>sys.stderr, "======================="

    # get list of header files

    loadHeaderFileList()
    if (options.debug == True):
        print >>sys.stderr, "======================="
        for headerFile in headerFileList:
            print >>sys.stderr, "headerFile: %s" % (headerFile)
        print >>sys.stderr, "======================="

    # get list of include directories to be referenced

#    for headerFile in headerFileList:
#        setIncludeList(headerFile)

#    for compileFile in compileFileList:
#        setIncludeList(compileFile)

#    if (options.debug == True):
#        for lib in includeList:
#            if (lib.used == True):
#                print >>sys.stderr, "Use lib for include: %s" % (lib.name)

    # write out makefile.am

    writeMakefileAm()

    sys.exit(0)

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
        print >>sys.stderr, "  dir: ", options.dir
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
# parse the RAL Makefile to get the lib name

def getLibName():

    global thisLibName

    # search for MODULE_NAME key in makefile

    valList = getValueListForKey(makefileName, "MODULE_NAME")

    if (len(valList) < 1):
        print >>sys.stderr, "ERROR - ", thisScriptName
        print >>sys.stderr, "  Cannot find MODULE_NAME in ", makefileName
        print >>sys.stderr, "  dir: ", options.dir
        exit(1)

    thisLibName = valList[len(valList)-1]

########################################################################
# get list of makefiles - using RAL Makefile to locate subdirs

def getSubDirList():

    global subDirList
    subDirList = []

    # search for SUB_DIRS key in makefile

    subNameList = getValueListForKey(makefileName, "SUB_DIRS")

    if (len(subNameList) < 1):
        print >>sys.stderr, "ERROR - ", thisScriptName
        print >>sys.stderr, "  Cannot find SUB_DIRS in ", makefileName
        print >>sys.stderr, "  dir: ", options.dir
        exit(1)

    for subName in subNameList:
        if (os.path.isdir(subName) == True):
            makefilePath = os.path.join(subName, 'makefile')
            if (os.path.isfile(makefilePath) == True):
                subDir = SubDir(subName, makefilePath)
                subDirList.append(subDir)
            else:
                makefilePath = os.path.join(subName, 'Makefile')
                if (os.path.isfile(makefilePath) == True):
                    subDir = SubDir(subName, makefilePath)
                    subDirList.append(subDir)

########################################################################
# append to list of files to be compiled

def addSubDirToCompileList(subDir):
                    
    global compileFileList
    
    srcTypeList = [ 'SRCS', 'C_SRCS', 'F_SRCS', 'F_CPP_SRCS', 
                    'F90_SRCS', 'F95_SRCS', 'PGF90_SRCS', 
                    'PGF_SRCS', 'CC_SRCS', 'CPPC_SRCS', 
                    'CPP_SRCS', 'CXX_SRCS' ]
    
    fp = open(subDir.makefilePath, 'r')
    lines = fp.readlines()

    for srcType in srcTypeList:
        handleSrcType(subDir, lines, srcType)
    
########################################################################
# append to compile list for given srcType

def handleSrcType(subDir, lines, srcType):

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

def loadHeaderFileList():

    global headerFileList
    headerFileList = []

    global includesInSubDir
    includesInSubDir = False

    subdirList = os.listdir("include")

    for subdir in subdirList:
        incDir = os.path.join("include", subdir)
        if ((os.path.isdir(incDir) == True) and (subdir != "CVS")):
            appendToHeaderFileList(incDir)
            includesInSubDir = True

    if (includesInSubDir == False):
        appendToHeaderFileList("include")
            
########################################################################
# append to header file list

def appendToHeaderFileList(dir):

    global headerFileList

    fileList = os.listdir(dir)
    for fileName in fileList:
        last2 = fileName[-2:]
        last3 = fileName[-3:]
        if (last2 == ".h") or (last3 == ".hh"):
            filePath = os.path.join(dir, fileName)
            headerFileList.append(filePath)

########################################################################
# set the list of libs to be used for include

def setIncludeList(sourceFile):
                    
    global includeList
    
    if (options.verbose == True):
        print >>sys.stderr, "-->> looking for includes in: ", sourceFile

    fp = open(sourceFile, 'r')
    lines = fp.readlines()
    
    for line in lines:
        if ((line[0] != '#') or
            (line.find("include") < 0) or
            (line.find("/") < 0) or
            (line.find("<") < 0) or
            (line.find(">") < 0)):
            continue

        if (options.verbose == True):
            print >>sys.stderr, "  -->> ", line.strip()
        
        for lib in includeList:
            if (lib.name == thisLibName):
                # skip this lib
                continue
            searchStr = "<%s/" % lib.name
            if (line.find(searchStr) > 0):
                if (options.verbose == True):
                    print >>sys.stderr, "  -->> found lib", lib.name
                lib.used = True
            
########################################################################
# Write out makefile.am

def writeMakefileAm():

    fo = open("makefile.am", "w")

    fo.write("###############################################\n")
    fo.write("#\n")
    fo.write("# makefile template for automake\n")
    fo.write("#\n")
    fo.write("# library name: %s\n" % thisLibName)
    fo.write("#\n")
    fo.write("# written by script createMakefile.am.lib\n")
    fo.write("#\n")
    fo.write("# created %s\n" % datetime.now())
    fo.write("#\n")
    fo.write("###############################################\n")
    fo.write("\n")
    fo.write("# compile flags - include header subdirectory\n")
    fo.write("\n")
    fo.write("AM_CFLAGS = -I./include\n")
    fo.write("# NOTE: X11R6 is for Mac OSX location of XQuartz\n")
    fo.write("AM_CFLAGS += -I/usr/X11R6/include\n")
    if (options.shared == True):
        fo.write("ACLOCAL_AMFLAGS = -I m4\n")
    #    fo.write("AM_CFLAGS += -I$(prefix)/include\n")

    for lib in libList:
        # if (lib.used):
        #fo.write("AM_CFLAGS += -I../../%s/src/include\n" % lib.name)
        fo.write("AM_CFLAGS += -I../../%s/src/include\n" % lib)
    fo.write("\n")
    fo.write("AM_CXXFLAGS = $(AM_CFLAGS)\n")
    fo.write("\n")
    fo.write("# target library file\n")
    fo.write("\n")
    if (options.shared == True):
        fo.write("lib_LTLIBRARIES = lib%s.la\n" % thisLibName)
    else:
        fo.write("lib_LIBRARIES = lib%s.a\n" % thisLibName)
    fo.write("\n")
    fo.write("# headers to be installed\n")
    fo.write("\n")
    if (includesInSubDir == True):
        fo.write("includedir = $(prefix)/include/%s\n" % thisLibName)
    else:
        fo.write("includedir = $(prefix)/include\n")
    fo.write("\n")

    fo.write("include_HEADERS = \\\n")
    for index, headerFile in enumerate(headerFileList):
        fo.write("\t%s" % headerFile)
        if (index == len(headerFileList) - 1):
            fo.write("\n")
        else:
            fo.write(" \\\n")
    fo.write("\n")

    if (options.shared == True):
        fo.write("lib%s_la_SOURCES = \\\n" % thisLibName)
    else:
        fo.write("lib%s_a_SOURCES = \\\n" % thisLibName)
    for index, compileFile in enumerate(compileFileList):
        fo.write("\t%s" % compileFile)
        if (index == len(compileFileList) - 1):
            fo.write("\n")
        else:
            fo.write(" \\\n")
    fo.write("\n")

    fo.close

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

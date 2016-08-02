#!/usr/bin/env python

#===========================================================================
#
# Create makefile.am for a RAL app
#
#===========================================================================

import os
import sys
import shutil
import subprocess
from optparse import OptionParser
from datetime import datetime

class IncludeLib:
    def __init__(self, name, used):
        self.name = name
        self.used = used

def main():

    global options
    global thisAppName
    global makefileName
    global compileFileList
    global headerFileList
    global includeList
    global availLibList
    global usedLibList
    global loadLibList
    global needQt4

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
                      help='Path of app directory')
    parser.add_option('--libList',
                      dest='libList', default="",
                      help='List of libs in package')

    (options, args) = parser.parse_args()

    if (options.verbose == True):
        options.debug = True
    
    # get the app name

    pathParts = options.dir.split('/')

    if (options.debug == True):
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  App dir:", options.dir
        print >>sys.stderr, "  Lib list: ", options.libList

    # set up list of other libs which may be used for include

    if (len(options.libList) < 1):
        availLibList = [ 'FiltAlg', 'Fmq', 'Mdv', 'Radx',
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
        availLibList = options.libList.split(",")

    if (options.debug == True):
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  App dir:", options.dir
        print >>sys.stderr, "  Lib list: ", availLibList

    # go to the app dir

    os.chdir(options.dir)

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

    # parse the RAL Makefile to get the app name

    getAppName()
    if (options.debug == True):
        print >>sys.stderr, "thisAppName: %s" % thisAppName

    # load list of files to be compiled

    compileFileList = []
    setCompileList()

    if (options.debug == True):
        print >>sys.stderr, "======================="
        for compileFile in compileFileList:
            print >>sys.stderr, "compileFile: %s" % (compileFile)
        print >>sys.stderr, "======================="

    # get list of header files

    headerFileList = []
    setHeaderFileList()
    if (options.debug == True):
        print >>sys.stderr, "======================="
        for headerFile in headerFileList:
            print >>sys.stderr, "headerFile: %s" % (headerFile)
        print >>sys.stderr, "======================="
            
    # get list of libs to be loaded

    setLoadLibList()
    if (options.debug == True):
        print >>sys.stderr, "======================="
        for loadLib in loadLibList:
            print >>sys.stderr, "loadLib: %s" % loadLib
        print >>sys.stderr, "======================="

    # get list of RAL libs used

    setUsedLibList()
    if (options.debug == True):
        print >>sys.stderr, "======================="
        for loadLib in usedLibList:
            print >>sys.stderr, "usedLib: %s" % loadLib
        print >>sys.stderr, "======================="

    # check if we need Qt4 support

    checkForQt4()

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
# parse the RAL Makefile to get the app name

def getAppName():

    global thisAppName

    # search for TARGET_FILE key in makefile

    valList = getValueListForKey(makefileName, "TARGET_FILE")

    if (len(valList) < 1):
        print >>sys.stderr, "ERROR - ", thisScriptName
        print >>sys.stderr, "  Cannot find TARGET_FILE in ", makefileName
        print >>sys.stderr, "  dir: ", options.dir
        exit(1)

    thisAppName = valList[len(valList)-1]

########################################################################
# set list of files to be compiled

def setCompileList():
                    
    global compileFileList
    
    srcTypeList = [ 'SRCS', 'C_SRCS', 'F_SRCS', 'F_CPP_SRCS', 
                    'F90_SRCS', 'F95_SRCS', 'PGF90_SRCS', 
                    'PGF_SRCS', 'CC_SRCS', 'CPPC_SRCS', 
                    'CPP_SRCS', 'CXX_SRCS',
                    'NORM_SRCS', 'MOC_SRCS', "MOC_OUTPUT" ]

    try:
        fp = open(makefileName, 'r')
    except IOError as e:
        print >>sys.stderr, "ERROR - ", thisScriptName
        print >>sys.stderr, "  Cannot open: ", makefileName
        exit(1)

    lines = fp.readlines()
    fp.close()

    for srcType in srcTypeList:
        handleSrcType(lines, srcType)
    
########################################################################
# check for dependence on QT4

def checkForQt4():
                    
    global needQt4
    needQt4 = False

    try:
        fp = open(makefileName, 'r')
    except IOError as e:
        print >>sys.stderr, "ERROR - ", thisScriptName
        print >>sys.stderr, "  Cannot open: ", makefileName
        exit(1)

    lines = fp.readlines()
    fp.close()

    for line in lines:
        if (line.find("QT4") >= 0):
            needQt4 = True
            return
        if (line.find("-lQtCore") >= 0):
            needQt4 = True
            return
    
########################################################################
# append to compile list for given srcType

def handleSrcType(lines, srcType):

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
            compileFileList.append(tok)
        elif (tok == "$(PARAMS_CC)"):
            compileFileList.append("Params.cc")
        elif (tok == "$(TDRP_C)"):
            compileFileList.append(thisAppName + "_tdrp.c")
        elif (tok == "$(_TDRP_C)"):
            compileFileList.append("_tdrp.c")

########################################################################
# set header file list

def setHeaderFileList():

    global headerFileList

    fileList = os.listdir('.')

    for fileName in fileList:
        last2 = fileName[-2:]
        last3 = fileName[-3:]
        if (last2 == ".h") or (last3 == ".hh"):
            headerFileList.append(fileName)

########################################################################
# set the list of libs to be used for include

def setIncludeList(sourceFile):
                    
    global includeList
    
    if (options.verbose == True):
        print >>sys.stderr, "-->> looking for includes in:", sourceFile

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
            if (lib.name == thisAppName):
                # skip this lib
                continue
            searchStr = "<%s/" % lib.name
            if (line.find(searchStr) > 0):
                if (options.verbose == True):
                    print >>sys.stderr, "  -->> found lib", lib.name
                lib.used = True
            
########################################################################
# set list of libraries to be loaded by the linker

def setLoadLibList():
                    
    global loadLibList
    loadLibList = []

    # search for LOC_LIBS key in makefile

    locLibsList = getValueListForKey(makefileName, "LOC_LIBS")
    if (len(locLibsList) < 1):
        return

    for locLib in locLibsList:
        if (locLib == "$(TDRP_LIBS)"):
            appendToLibList("-ltdrp")
        else:
            appendToLibList(locLib)

########################################################################
# set list of RAL libs actually used

def setUsedLibList():
                    
    global usedLibList
    usedLibList = []

    for loadLib in loadLibList:
        for availLib in availLibList:
            checkStr = "-l" + availLib
            if (loadLib == checkStr):
                usedLibList.append(availLib)
                break

########################################################################
# append to lib list from given line

def appendToLibList(line):

    toks = line.split(' ')
    for tok in toks:
        thisTok = tok.strip(" \t\n\r")
        if (thisTok.find("-l") == 0):
            loadLibList.append(thisTok)
        elif ((thisTok.find("NETCDF4_LIBS") >= 0) or
              (thisTok.find("NETCDF_LIBS") >= 0)):
            loadLibList.append("-lnetcdf_c++")
            loadLibList.append("-lnetcdf")
            loadLibList.append("-lhdf5_cpp")
            loadLibList.append("-lhdf5_hl")
            loadLibList.append("-lhdf5")
#            loadLibList.append("-ludunits2")
            loadLibList.append("-lz")
            loadLibList.append("-lbz2")
        elif (thisTok.find("NETCDF_C_AND_C++_LIBS") >= 0):
            loadLibList.append("-lnetcdf_c++")
            loadLibList.append("-lnetcdf")
        elif (thisTok.find("NETCDF_C_AND_F_LIBS") >= 0):
            loadLibList.append("-lnetcdff")
            loadLibList.append("-lnetcdf")
        elif (thisTok.find("NETCDF_C_LIB") >= 0):
            loadLibList.append("-lnetcdf")
        elif (thisTok.find("NETCDF_FF_LIB") >= 0):
            loadLibList.append("-lnetcdff")

########################################################################
# Write out makefile.am

def writeMakefileAm():

    fo = open("makefile.am", "w")

    fo.write("###############################################\n")
    fo.write("#\n")
    fo.write("# makefile template for automake\n")
    fo.write("#\n")
    fo.write("# app name: %s\n" % thisAppName)
    fo.write("#\n")
    fo.write("# written by script %s\n" % thisScriptName)
    fo.write("#\n")
    fo.write("# created %s\n" % datetime.now())
    fo.write("#\n")
    fo.write("###############################################\n")
    fo.write("\n")

    fo.write("# compile flags - includes\n")
    fo.write("\n")
    fo.write("AM_CFLAGS = -I.\n")
    fo.write("# NOTE: X11R6 is for Mac OSX location of XQuartz\n")
    fo.write("AM_CFLAGS += -I/usr/X11R6/include\n")
    for lib in usedLibList:
        fo.write("AM_CFLAGS += -I../../../../libs/%s/src/include\n" % lib)
    if (needQt4 == True):
        fo.write("AM_CFLAGS += $(QT4_CFLAGS)\n")
    fo.write("\n")
    fo.write("AM_CXXFLAGS = $(AM_CFLAGS)\n")
    fo.write("\n")

    fo.write("# load flags\n")
    fo.write("\n")
    fo.write("AM_LDFLAGS = -L.\n")
    fo.write("# NOTE: X11R6 is for Mac OSX location of XQuartz\n")
    fo.write("AM_LDFLAGS += -L/usr/X11R6/lib\n")
    for lib in usedLibList:
        fo.write("AM_LDFLAGS += -L../../../../libs/%s/src\n" % lib)
    if (needQt4 == True):
        fo.write("AM_LDFLAGS += $(QT4_LIBS)\n")
    fo.write("\n")

    if (len(loadLibList) > 0):
        fo.write("# load libs\n")
        fo.write("\n")
        for index, loadLib in enumerate(loadLibList):
            if (index == 0):
                fo.write("LDADD = %s\n" % loadLib)
            else:
                fo.write("LDADD += %s\n" % loadLib)
        fo.write("\n")

    fo.write("# set app name\n")
    fo.write("\n")
    fo.write("bin_PROGRAMS = %s\n" % thisAppName)
    fo.write("\n")

    fo.write("# source files\n")
    fo.write("\n")

    fo.write("%s_SOURCES = \\\n" % thisAppName)
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

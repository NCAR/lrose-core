#!/usr/bin/env python

#===========================================================================
#
# Create makefile.am for a LROSE package
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
    global linkOrder
    global compiledLibList
    global orderedLibList
    global makefileLibList
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
            print >>sys.stderr, "  compileFile: %s" % (compileFile)
        print >>sys.stderr, "======================="

    # set list of header files

    headerFileList = []
    setHeaderFileList()
    if (options.debug == True):
        print >>sys.stderr, "======================="
        for headerFile in headerFileList:
            print >>sys.stderr, "  headerFile: %s" % (headerFile)
        print >>sys.stderr, "======================="
            
    # get list showing order in which compiled libs need to be linked

    linkOrder = getLibLinkOrder()

    # set up list of compiled libs

    if (len(options.libList) < 1):
        compiledLibList = linkOrder
    else:
        compiledLibList = options.libList.split(",")

    # create ordered lib list

    orderedLibList = []
    for entry in linkOrder:
        if (entry in compiledLibList):
            orderedLibList.append(entry)
    # orderedLibList.reverse()
    if (options.debug == True):
        print >>sys.stderr, "======== ordered lib list ==================="
        for lib in orderedLibList:
            print >>sys.stderr, "  ordered lib: %s" % lib
        print >>sys.stderr, "============================================="

    # get list of libs listed in makefile

    makefileLibList = getMakefileLibList()
    if (options.debug == True):
        print >>sys.stderr, "========= makefile lib list =============="
        for lib in makefileLibList:
            print >>sys.stderr, "  makefile lib: %s" % lib
        print >>sys.stderr, "=========================================="

    # set list of libs to be loaded
    # this will be the ordered lib list, plus any libs from the makefile
    # that are not included in the ordered libs

    loadLibList = getLoadLibList()
    if (options.debug == True):
        print >>sys.stderr, "======= load lib list ================"
        for lib in loadLibList:
            print >>sys.stderr, "  load lib: -l%s" % lib
        print >>sys.stderr, "======================================"

    # check if we need Qt4 support

    needQt4 = checkForQt4()

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
            return True
        if (line.find("-lQtCore") >= 0):
            return True

    return False
    
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
# get link order for libraries

def getLibLinkOrder():

    # set up list showing order in which compiled libs need to be linked
    
    linkOrder = [ 'FiltAlg',
                  'dsdata',
                  'radar',
                  'hydro',
                  'titan',
                  'Fmq',
                  'Spdb',
                  'Mdv',
                  'Refract',
                  'advect',
                  'physics',
                  'rapplot',
                  'Radx',
                  'rapformats',
                  'dsserver',
                  'didss',
                  'grib',
                  'grib2',
                  'contour',
                  'euclid',
                  'rapmath',
                  'toolsa',
                  'dataport',
                  'tdrp',
                  'shapelib',
                  'cidd',
                  'devguide',
                  'xview',
                  'olgx',
                  'trmm_rsl',
                  'forayRal']
    
    ################ deprecated ####################
    #             'mdv',
    #             'rdi',
    #             'spdb',
    #             'spdbFormats',
    #             'symprod',
    #             'SpdbServer',
    ################################################

    return linkOrder
    
########################################################################
# get list of libraries in makefile

def getMakefileLibList():

    makeLibList = []

    # search for LOC_LIBS key in makefile

    locLibs = getValueListForKey(makefileName, "LOC_LIBS")
    if (len(locLibs) > 0):
        for line in locLibs:
            makeLibList.extend(decodeLibLine(line))

    return makeLibList
            
########################################################################
# decode a line from LOC_LIBS

def decodeLibLine(line):

    libs = []

    toks = line.split(' ')
    for tok in toks:
        thisTok = tok.strip(" \t\n\r")
        if (thisTok.find("-l") == 0):
            libs.append(thisTok[2:]) # strip off '-l'
        elif ((thisTok.find("NETCDF4_LIBS") >= 0) or
              (thisTok.find("NETCDF_LIBS") >= 0)):
            libs.append("netcdf_c++")
            libs.append("netcdf")
            libs.append("hdf5_cpp")
            libs.append("hdf5_hl")
            libs.append("hdf5")
            libs.append("udunits2")
            libs.append("z")
            libs.append("bz2")
        elif (thisTok.find("NETCDF_C_AND_C++_LIBS") >= 0):
            libs.append("netcdf_c++")
            libs.append("netcdf")
        elif (thisTok.find("NETCDF_C_AND_F_LIBS") >= 0):
            libs.append("netcdff")
            libs.append("netcdf")
        elif (thisTok.find("NETCDF_C_LIB") >= 0):
            libs.append("netcdf")
        elif (thisTok.find("NETCDF_FF_LIB") >= 0):
            libs.append("netcdff")
        elif (thisTok.find("TDRP_LIBS") >= 0):
            libs.append("tdrp")

    return libs

########################################################################
# get list of libraries to be loaded

def getLoadLibList():

    # set list of libs to be loaded
    # this will be the ordered lib list, plus any libs from the makefile
    # that are not included in the ordered libs

    loadLibList = orderedLibList
    for lib in makefileLibList:
        if (lib not in orderedLibList):
            loadLibList.append(lib)

    # extend the lib list with required standard libs
    
    extendLibs = [ 'netcdf_c++',
                   'netcdf',
                   'hdf5_cpp',
                   'hdf5_hl',
                   'hdf5',
                   'udunits2',
                   'expat',
                   'jasper',
                   'fl',
                   'X11',
                   'Xext',
                   'pthread',
                   'png',
                   'z',
                   'bz2',
                   'm',
                   'gfortran' ]
    
    for lib in extendLibs:
        if (lib not in loadLibList):
            loadLibList.append(lib)

    if ("radar" in loadLibList and "fftw3" not in loadLibList):
        loadLibList.append("fftw3")

    return loadLibList

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
    for lib in compiledLibList:
        fo.write("AM_CFLAGS += -I../../../../libs/%s/src/include\n" % lib)
    if (needQt4 == True):
        fo.write("AM_CFLAGS += $(QT4_CFLAGS)\n")
    fo.write("\n")
    fo.write("AM_CXXFLAGS = $(AM_CFLAGS)\n")
    fo.write("\n")

    fo.write("# load flags\n")
    fo.write("\n")
    fo.write("AM_LDFLAGS = -L.\n")
    for lib in compiledLibList:
        fo.write("AM_LDFLAGS += -L../../../../libs/%s/src\n" % lib)
    if (needQt4 == True):
        fo.write("AM_LDFLAGS += $(QT4_LIBS)\n")
    fo.write("\n")

    if (len(loadLibList) > 0):
        fo.write("# load libs\n")
        fo.write("\n")
        for index, loadLib in enumerate(loadLibList):
            if (index == 0):
                fo.write("LDADD = -l%s\n" % loadLib)
            else:
                fo.write("LDADD += -l%s\n" % loadLib)
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

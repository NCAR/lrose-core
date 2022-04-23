#!/usr/bin/env python

#===========================================================================
#
# Create makefile.am for a LROSE core package
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

class IncludeLib:
    def __init__(self, name, used):
        self.name = name
        self.used = used

def main():

    global options
    global thisAppName
    global compileFileList
    global headerFileList
    global linkOrder
    global compiledLibList
    global orderedLibList
    global makefileLibList
    global loadLibList
    global needQt
    global needX11
    global isDebianBased

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    osType = getOsType()
    isDebianBased = False
    if (osType == "debian"):
        isDebianBased = True
    if (osType == "ubuntu"):
        isDebianBased = True

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
    parser.add_option('--osx',
                      dest='osx', default=False,
                      action="store_true",
                      help='Configure for MAC OSX')
    parser.add_option('--dir',
                      dest='dir', default=".",
                      help='Path of app directory')
    parser.add_option('--template',
                      dest='template', default="unknown",
                      help='Path of makefile template')
    parser.add_option('--libList',
                      dest='libList', default="",
                      help='List of libs in package')

    (options, args) = parser.parse_args()

    if (options.verbose):
        options.debug = True
    
    # get the app name

    pathParts = options.dir.split('/')

    if (options.debug):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  App dir:", options.dir, file=sys.stderr)
        print("  Makefile template: ", options.template, file=sys.stderr)
        print("  Lib list: ", options.libList, file=sys.stderr)
        print("  osx: ", options.osx, file=sys.stderr)

    # go to the app dir

    os.chdir(options.dir)

    # parse the LROSE Makefile to get the app name

    getAppName()
    if (options.debug):
        print("thisAppName: %s" % thisAppName, file=sys.stderr)

    # load list of files to be compiled

    compileFileList = []
    setCompileList()

    if (options.debug):
        print("=======================", file=sys.stderr)
        for compileFile in compileFileList:
            print("  compileFile: %s" % (compileFile), file=sys.stderr)
        print("=======================", file=sys.stderr)

    # set list of header files

    headerFileList = []
    setHeaderFileList()
    if (options.debug):
        print("=======================", file=sys.stderr)
        for headerFile in headerFileList:
            print("  headerFile: %s" % (headerFile), file=sys.stderr)
        print("=======================", file=sys.stderr)
            
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
    if (options.debug):
        print("======== ordered lib list ===================",
              file=sys.stderr)
        for lib in orderedLibList:
            print("  ordered lib: %s" % lib, file=sys.stderr)
        print("=============================================",
              file=sys.stderr)

    # get list of libs listed in makefile

    makefileLibList = getMakefileLibList()
    if (options.debug):
        print("========= makefile lib list ==============",
              file=sys.stderr)
        for lib in makefileLibList:
            print("  makefile lib: %s" % lib, file=sys.stderr)
        print("==========================================",
              file=sys.stderr)

    # check if we need Qt support

    needQt = checkForQt()
    needX11 = checkForX11()
    
    # set list of libs to be loaded
    # this will be the ordered lib list, plus any libs from the makefile
    # that are not included in the ordered libs

    loadLibList = getLoadLibList()
    if (options.debug):
        print("======= load lib list ================",
              file=sys.stderr)
        for lib in loadLibList:
            print("  load lib: -l%s" % lib, file=sys.stderr)
        print("======================================",
              file=sys.stderr)

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
# parse the LROSE Makefile to get the app name

def getAppName():

    global thisAppName

    # search for TARGET_FILE key in makefile

    valList = getValueListForKey(options.template, "TARGET_FILE")

    if (len(valList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find TARGET_FILE in ", options.template, file=sys.stderr)
        print("  dir: ", options.dir, file=sys.stderr)
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
        fp = open(options.template, 'r')
    except IOError as e:
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot open: ", options.template, file=sys.stderr)
        exit(1)

    lines = fp.readlines()
    fp.close()

    for srcType in srcTypeList:
        handleSrcType(lines, srcType)
    
########################################################################
# check for dependence on QT

def checkForQt():
                    
    try:
        fp = open(options.template, 'r')
    except IOError as e:
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot open: ", options.template, file=sys.stderr)
        exit(1)

    lines = fp.readlines()
    fp.close()

    for line in lines:
        if (line.find("QT") >= 0):
            return True

    return False
    
########################################################################
# check for dependence on X11

def checkForX11():
                    
    try:
        fp = open(options.template, 'r')
    except IOError as e:
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot open: ", options.template, file=sys.stderr)
        exit(1)

    lines = fp.readlines()
    fp.close()

    for line in lines:
        if (line.find("X11") >= 0):
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
# get link order for libraries

def getLibLinkOrder():

    # set up list showing order in which compiled libs need to be linked
    
    linkOrder = [ 'mm5',
                  'Refract',
                  'FiltAlg',
                  'dsdata',
                  'radar',
                  'hydro',
                  'titan',
                  'Fmq',
                  'Spdb',
                  'Mdv',
                  'advect',
                  'rapplot',
                  'Radx',
                  'Ncxx',
                  'rapformats',
                  'dsserver',
                  'didss',
                  'grib',
                  'grib2',
                  'contour',
                  'euclid',
                  'rapmath',
                  'kd',
                  'physics',
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
    
    return linkOrder
    
########################################################################
# get list of libraries in makefile

def getMakefileLibList():

    makeLibList = []

    # search for LOC_LIBS key in makefile

    locLibs = getValueListForKey(options.template, "LOC_LIBS")
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
            libs.append("Ncxx")
            libs.append("netcdf")
            libs.append("hdf5_hl")
            libs.append("hdf5")
            libs.append("z")
            libs.append("bz2")
        elif (thisTok.find("NETCDF_C_AND_C++_LIBS") >= 0):
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
        #elif (thisTok.find("QT_LIBS") >= 0):
        #    libs.append("Qt5Core")
        #    libs.append("Qt5Gui")
        #    libs.append("Qt5Widgets")
        #    libs.append("Qt5Network")

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

    if (options.osx):
        extendLibs = [ 'Ncxx',
                       'netcdf',
                       'hdf5_hl',
                       'hdf5',
                       # 'expat',
                       # 'jasper',
                       # 'fl',
                       'X11',
                       'Xext',
                       'pthread',
                       'jpeg',
                       'png',
                       'z',
                       'bz2',
                       'm' ]
    else:
        extendLibs = [ 'Ncxx',
                       'netcdf',
                       'hdf5_hl',
                       'hdf5',
                       # 'expat',
                       # 'jasper',
                       # 'fl',
                       'X11',
                       'Xext',
                       'pthread',
                       'jpeg',
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
    fo.write("# dir: %s\n" % options.dir)
    fo.write("# libList: %s\n" % options.libList)
    fo.write("# osx: %s\n" % options.osx)
    fo.write("###############################################\n")
    fo.write("\n")

    fo.write("# compile flags\n")
    fo.write("\n")
    fo.write("AM_CFLAGS = -fPIC\n")
    fo.write("AM_CFLAGS += -I.\n")
    for lib in compiledLibList:
        fo.write("AM_CFLAGS += -I../../../../libs/%s/src/include\n" % lib)
    fo.write("# add includes already installed in prefix\n")
    fo.write("AM_CFLAGS += -I${prefix}/include\n")
    fo.write("\n")

    if (needQt):
        fo.write("PKG_CONFIG_PATH = /usr/lib/pkgconfig\n")
        fo.write("PKG_CONFIG_PATH += /usr/local/opt/qt/lib/pkgconfig\n")

    if (isDebianBased):
        fo.write("# NOTE: add in Debian location of HDF5\n")
        fo.write("AM_CFLAGS += -I/usr/include/hdf5/serial\n")
    if (options.osx):
        fo.write("# for OSX\n")
        fo.write("AM_CFLAGS += -I/opt/X11/include\n")
        fo.write("AM_CFLAGS += -I/usr/local/opt/flex/include\n")
    if (needQt):
        fo.write("# for QT\n")
        fo.write("AM_CFLAGS += -std=c++11\n")
        fo.write("AM_CFLAGS += $(shell pkg-config --cflags Qt5Core)\n")
        fo.write("AM_CFLAGS += $(shell pkg-config --cflags Qt5Gui)\n")
        fo.write("AM_CFLAGS += $(shell pkg-config --cflags Qt5Widgets)\n")
        fo.write("AM_CFLAGS += $(shell pkg-config --cflags Qt5Network)\n")
        fo.write("AM_CFLAGS += $(shell pkg-config --cflags Qt5Qml)\n")
    fo.write("\n")

    fo.write("AM_CXXFLAGS = $(AM_CFLAGS)\n")
    fo.write("\n")

    fo.write("# load flags\n")
    fo.write("\n")
    fo.write("AM_LDFLAGS = -L.\n")
    fo.write("AM_LDFLAGS += -L${prefix}/lib\n")
    if (isDebianBased):
        fo.write("# NOTE: add in Debian location of HDF5\n")
        fo.write("AM_LDFLAGS += -L/usr/lib/x86_64-linux-gnu/hdf5/serial\n")
    for lib in compiledLibList:
        fo.write("AM_LDFLAGS += -L../../../../libs/%s/src\n" % lib)
    if (options.osx):
        fo.write("# for OSX\n")
        fo.write("AM_LDFLAGS += -L/opt/X11/lib\n")
        fo.write("AM_LDFLAGS += -L/usr/local/opt/flex/lib\n")
    if (needQt):
        fo.write("# for QT\n")
        fo.write("AM_LDFLAGS += -L$(shell pkg-config --variable=libdir Qt5Gui)\n")
        fo.write("AM_LDFLAGS += $(shell pkg-config --libs Qt5Core)\n")
        fo.write("AM_LDFLAGS += $(shell pkg-config --libs Qt5Gui)\n")
        fo.write("AM_LDFLAGS += $(shell pkg-config --libs Qt5Widgets)\n")
        fo.write("AM_LDFLAGS += $(shell pkg-config --libs Qt5Network)\n")
        fo.write("AM_LDFLAGS += $(shell pkg-config --libs Qt5Qml)\n")
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

    #if (needQt):
    #    fo.write("# qt libs\n")
    #    fo.write("\n")
    #    fo.write("LDADD += $(shell pkg-config --libs Qt5Core)\n")
    #    fo.write("LDADD += $(shell pkg-config --libs Qt5Gui)\n")
    #    fo.write("LDADD += $(shell pkg-config --libs Qt5Widgets)\n")
    #    fo.write("LDADD += $(shell pkg-config --libs Qt5Network)\n")
    #    fo.write("\n")

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
# get the LINUX type from the /etc/os-release file
# or 'darwin' if OSX

def getOsType():                                                                                  

    # check for Mac OSX

    if sys.platform == "darwin":
        return "osx"
    elif sys.platform == "linux":
        osrelease_file = open("/etc/os-release", "rt")
        lines = osrelease_file.readlines()
        osrelease_file.close()
        osType = "unknown"
        for line in lines:
            if (line.find('PRETTY_NAME') == 0):
                lineParts = line.split('=')
                osParts = lineParts[1].split('"')
                osType = osParts[1].lower()
                if (osType.find("debian") >= 0):
                    return "debian"
                if (osType.find("ubuntu") >= 0):
                    return "ubuntu"
                if (osType.find("centos") >= 0):
                    return "centos"
                if (osType.find("rhel") >= 0):
                    return "rhel"
                if (osType.find("opensuse") >= 0):
                    return "opensuse"
                

    return "unknown"
            
########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

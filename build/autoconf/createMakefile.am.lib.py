#!/usr/bin/env python

#===========================================================================
#
# Create makefile.am for a LROSE lib
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
    global subDirList
    global compileFileList
    global headerFileList
    global includeList
    global libList
    global includesInSubDir
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
    parser.add_option('--template',
                      dest='template', default="unknown",
                      help='Path of makefile template')
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
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  Lib dir: ", options.dir, file=sys.stderr)
        print("  Makefile template: ", options.template, file=sys.stderr)
        print("  Lib list: ", options.libList, file=sys.stderr)

    # set up list of other libs which may be used for include

    if (len(options.libList) < 1):
        libList = [ 'FiltAlg', 'Fmq', 'Mdv', 'Radx', 'Ncxx',
                    'Refract', 'Spdb', 'SpdbServer', 'advect', 
                    'cidd', 'contour',
                    'dataport', 'devguide', 'didss', 'dsdata', 'dsserver',
                    'euclid', 'forayRal', 'grib', 'grib2', 'hydro',
                    'mdv', 'niwot_basic', 'niwot_util', 
                    'olgx', 'physics', 'radar',
                    'rapformats', 'rapmath', 'rapplot', 'qtplot', 'rdi', 'shapelib',
                    'spdb', 'spdbFormats', 'symprod', 'tdrp', 'titan',
                    'toolsa', 'trmm_rsl', 'xview' ]
    else:
        libList = options.libList.split(",")

#    includeList = []
#    for libName in libNames:
#        lib = IncludeLib(libName, True)
#        includeList.append(lib)
     
    if (options.debug == True):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  App dir:", options.dir, file=sys.stderr)
        print("  Lib list: ", libList, file=sys.stderr)

    # go to the src dir

    srcDir = os.path.join(options.dir, 'src')
    if (options.debug == True):
        print("src dir: ", srcDir, file=sys.stderr)
    os.chdir(srcDir)

    # get the lib name

    thisLibName = ""
    getLibName()
    if (options.debug == True):
        print("  Lib name: ", thisLibName, file=sys.stderr)

    # get list of subdirs and their makefiles

    getSubDirList()

    if (options.debug == True):
        print("=======================", file=sys.stderr)
        for subDir in subDirList:
            print("subDir, makefile: %s, %s" % \
                (subDir.subDirName, subDir.makefilePath), file=sys.stderr)
        print("=======================", file=sys.stderr)

    # load list of files to be compiled

    compileFileList = []
    for subDir in subDirList:
        addSubDirToCompileList(subDir)

    if (options.debug == True):
        print("=======================", file=sys.stderr)
        for compileFile in compileFileList:
            print("compileFile: %s" % (compileFile), file=sys.stderr)
        print("=======================", file=sys.stderr)

    # get list of header files

    loadHeaderFileList()
    if (options.debug == True):
        print("=======================", file=sys.stderr)
        for headerFile in headerFileList:
            print("headerFile: %s" % (headerFile), file=sys.stderr)
        print("=======================", file=sys.stderr)

    # get list of include directories to be referenced

#    for headerFile in headerFileList:
#        setIncludeList(headerFile)

#    for compileFile in compileFileList:
#        setIncludeList(compileFile)

#    if (options.debug == True):
#        for lib in includeList:
#            if (lib.used == True):
#                print("Use lib for include: %s" % (lib.name)

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
# parse the LROSE Makefile to get the lib name

def getLibName():

    global thisLibName

    # search for MODULE_NAME key in makefile

    valList = getValueListForKey(options.template, "MODULE_NAME")

    if (len(valList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find MODULE_NAME in ", options.template, file=sys.stderr)
        print("  dir: ", options.dir, file=sys.stderr)
        exit(1)

    thisLibName = valList[len(valList)-1]

########################################################################
# get list of makefiles - using LROSE Makefile to locate subdirs

def getSubDirList():

    global subDirList
    subDirList = []

    # search for SUB_DIRS key in makefile

    subNameList = getValueListForKey(options.template, "SUB_DIRS")

    if (len(subNameList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find SUB_DIRS in ", options.template, file=sys.stderr)
        print("  dir: ", options.dir, file=sys.stderr)
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
        print("-->> looking for includes in: ", sourceFile, file=sys.stderr)

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
            print("  -->> ", line.strip(), file=sys.stderr)
        
        for lib in includeList:
            if (lib.name == thisLibName):
                # skip this lib
                continue
            searchStr = "<%s/" % lib.name
            if (line.find(searchStr) > 0):
                if (options.verbose == True):
                    print("  -->> found lib", lib.name, file=sys.stderr)
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
    fo.write("# compile flags\n")
    fo.write("\n")
    fo.write("AM_CFLAGS = -fPIC\n")
    fo.write("AM_CFLAGS += -I./include\n")
    for lib in libList:
        fo.write("AM_CFLAGS += -I../../%s/src/include\n" % lib)
    fo.write("# add includes already installed in prefix\n")
    fo.write("AM_CFLAGS += -I${prefix}/include\n")
    if (isDebianBased):
        fo.write("# add in Debian location of HDF5\n")
        fo.write("AM_CFLAGS += -I/usr/include/hdf5/serial\n")
    fo.write("# add in Mac OSX location of XQuartz\n")
    fo.write("AM_CFLAGS += -I/usr/X11R6/include -I/opt/X11/include\n")
    if (options.shared == True):
        fo.write("ACLOCAL_AMFLAGS = -I m4\n")
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
        fo.write("includedir = ${prefix}/include/%s\n" % thisLibName)
    else:
        fo.write("includedir = ${prefix}/include\n")
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

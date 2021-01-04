#!/usr/bin/env python

#===========================================================================
#
# Create CMakeLists from LROSE Makefiles
#
#===========================================================================

from __future__ import print_function
import os
import sys
import shutil
import subprocess
from optparse import OptionParser
from datetime import datetime

class LibSubDir:
    def __init__(self, subDirName, makefilePath):
        self.subDirName = subDirName
        self.makefilePath = makefilePath

class LibInclude:
    def __init__(self, name, used):
        self.name = name
        self.used = used

def main():

    global options
    global coreDir
    global codebaseDir

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global count
    count = 0

    # We will be executing some sibling scripts. Get our path so that
    # the sibling scripts from the same path can be executed explicitly.
    global thisScriptDir
    thisScriptDir = os.path.dirname(os.path.abspath(__file__))

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    coreDirDefault = os.path.join(thisScriptDir, "../..")
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=True,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--verbose',
                      dest='verbose', default=False,
                      action="store_true",
                      help='Set verbose debugging on')
    parser.add_option('--silent',
                      dest='silent', default=False,
                      action="store_true",
                      help='Set debugging off')
    parser.add_option('--coreDir',
                      dest='coreDir', default=coreDirDefault,
                      help='Path of lrose-core top level directory, default is: ' +
                      coreDirDefault)
    parser.add_option('--static',
                      dest='static', default=False,
                      action="store_true",
                      help='Create static lib objects. Default is shared')
    parser.add_option('--pkg',
                      dest='pkg', default="lrose-core",
                      help='Name of package being built')
    parser.add_option('--osx',
                      dest='osx', default=False,
                      action="store_true",
                      help='Configure for MAC OSX')
    parser.add_option('--verboseMake',
                      dest='verboseMake', default=False,
                      action="store_true",
                      help='Verbose output for make, default is summary')
    parser.add_option('--withJasper',
                      dest='withJasper', default=False,
                      action="store_true",
                      help='Set if jasper library is installed. This provides support for jpeg compression in grib files.')

    (options, args) = parser.parse_args()

    if (options.verbose):
        options.debug = True
    if (options.silent):
        options.debug = False
        options.verbose = False
    
    os.chdir(options.coreDir)
    coreDir = os.getcwd()
    codebaseDir = os.path.join(coreDir, "codebase")
    libsDir =  os.path.join(codebaseDir, "libs")
    appsDir =  os.path.join(codebaseDir, "apps")

    if (options.debug):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  coreDir: ", coreDir, file=sys.stderr)
        print("  codebaseDir: ", codebaseDir, file=sys.stderr)
        print("  libs dir: ", libsDir, file=sys.stderr)
        print("  apps dir: ", appsDir, file=sys.stderr)
        print("  static: ", options.static, file=sys.stderr)
        print("  pkg: ", options.pkg, file=sys.stderr)
        print("  osx: ", options.osx, file=sys.stderr)
        print("  verboseMake: ", options.verboseMake, file=sys.stderr)
        print("  withJasper: ", options.withJasper, file=sys.stderr)

    # go to the top level codebase

    os.chdir(codebaseDir)

    # write the top level CMakeLists.txt file

    writeCMakeListsTop(codebaseDir)
    count = count + 1
    
    # get list of libs
    
    (libArray, libList) = getLibList(libsDir)

    # recursively search libs and apps for makefiles

    searchDirRecurse(libsDir, libArray, libList)
    searchDirRecurse(appsDir, libArray, libList)

    if (options.debug):
        print("==>> n CMakeLists.txt files created: ", count, file=sys.stderr)

    sys.exit(0)

########################################################################
# search libs directory to compile libs list

def getLibList(dir):
                    
    libArray = []
    libList = ""

    if (options.debug):
        print("  Getting lib list from dir: ", dir, file=sys.stderr)

    # check if this dir has a makefile or Makefile

    makefilePath = getMakefilePath(dir)
    if (os.path.exists(makefilePath) == False):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  No makefile in lib dir: ", dir, file=sys.stderr)
        exit(1)

    if (options.debug):
        print("  Searching makefile template: ", makefilePath, file=sys.stderr)

    # search for SUB_DIRS key in makefile

    subNameList = getValueListForKey(makefilePath, "SUB_DIRS")

    if (len(subNameList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find SUB_DIRS in ", makefilePath, file=sys.stderr)
        print("  coreDir: ", options.coreDir, file=sys.stderr)
        exit(1)

    for subName in subNameList:
        subPath = os.path.join(dir, subName)
        if (os.path.isdir(subPath)):
            if (subName.find("perl5") < 0):
                libArray.append(subName)

    for index, libName in enumerate(libArray):
        libList += libName
        if (index < len(libArray) - 1):
            libList += ","

    if (options.debug):
        print("  libList: ", libList, file=sys.stderr)

    return libArray, libList

########################################################################
# search directory and subdirectories

def searchDirRecurse(dir, libArray, libList):
                    
    global count

    if (options.debug):
        print("  Searching dir: ", dir, file=sys.stderr)

    # check if this dir has a makefile or Makefile

    makefilePath = getMakefilePath(dir)
    if (os.path.exists(makefilePath) == False):
        if (options.verbose):
            print("  No makefile or Makefile found", file=sys.stderr)
        return

    # detect which type of directory we are in
        
    if (options.verbose):
        print("  Found makefile: ", makefilePath, file=sys.stderr)

    if ((dir == "libs/perl5") or
        (dir == "apps/scripts")):
        if (options.debug):
            print("  Ignoring dir:", dir, file=sys.stderr)
        return

    debugStr = "";
    if (options.verbose):
        debugStr = " --debug "

    absDir = os.path.join(options.coreDir, dir)
    pathToks = absDir.split("/")
    ntoks = len(pathToks)
    makefileCreatePath = os.path.join(dir, 'makefile')

    if (pathToks[ntoks-3] == "libs" and
        pathToks[ntoks-1] == "src"):

        # src level of lib - create CMakeLists.txt for lib

        libDir = absDir[:-4]
        createCMakeListsLib(libDir, libList)
        # staticStr = ""
        # if (options.static):
        #     staticStr = " --static "
        # cmd = os.path.join(thisScriptDir, "createCMakeLists.lib.py") + \
        #       " --dir " + libDir + staticStr + debugStr
        # cmd += " --libList " + libList
        # runCommand(cmd)
        count = count + 1

    elif ((pathToks[ntoks-4] == "apps") and
          (pathToks[ntoks-2] == "src")):

        # app directory - create CMakeLists.txt for app

        createScript = "createCMakeLists.app.py"
        scriptPath = os.path.join(thisScriptDir, createScript)

        if (options.debug):
            print("  createScript:", createScript, file=sys.stderr)

        cmd = scriptPath
        cmd += " --dir " + absDir + debugStr
        cmd += " --libList " + libList
        if (options.osx):
            cmd += " --osx "
        runCommand(cmd)
        count = count + 1

    elif ((pathToks[ntoks-4] == "apps") and
          (pathToks[ntoks-2] == "src") and
          (pathToks[ntoks-1] == "scripts")):

        # scripts dir - do nothing
        if (options.debug):
            print("  Ignoring dir:", dir, file=sys.stderr)

    else:

        # create CMakeLists.txt for recursion
        createCMakeListsRecurse(absDir)
        count = count + 1
        # recurse
        subdirList = getSubdirList(dir)
        for subdir in subdirList:
            subdirPath = os.path.join(dir, subdir)
            searchDirRecurse(subdirPath, libArray, libList)

    return

########################################################################
# load list of sub directories

def getSubdirList(dir):
                    
    subdirList = []
    
    # get makefile for this dir

    makefilePath = getMakefilePath(dir)

    # search for SUB_DIRS key in makefile

    subNameList = getValueListForKey(makefilePath, "SUB_DIRS")

    if (len(subNameList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find SUB_DIRS in ", makefilePath, file=sys.stderr)
        print("  dir: ", dir, file=sys.stderr)
        exit(1)

    for subName in subNameList:
        subPath = os.path.join(dir, subName)
        if (os.path.isdir(subPath)):
            subdirList.append(subName)

    return subdirList

########################################################################
# find makefile template

def getMakefilePath(dir):
                    
    makefilePath = os.path.join(dir, '__makefile.template')

    if (os.path.exists(makefilePath) == False):
        makefilePath = os.path.join(dir, 'makefile')

    if (os.path.exists(makefilePath) == False):
        makefilePath = os.path.join(dir, 'Makefile')

    if (os.path.exists(makefilePath) == False):
        if (options.debug):
            print("-->> makefile not found, dir: ", dir, file=sys.stderr)
        return 'not-found'

    if (options.debug):
        print("-->> using makefile template: ", makefilePath, file=sys.stderr)

    # if template does not exist,
    # copy makefile to template for later use if needed

    if (makefilePath.find('__makefile.template') < 0):
        templatePath = os.path.join(dir, '__makefile.template')
        shutil.copy(makefilePath, templatePath)

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
        print("  dir: ", options.coreDir, file=sys.stderr)
        return valueList

    lines = fp.readlines()
    fp.close()

    foundKey = False
    multiLine = ""
    for line in lines:
        if ((not foundKey) and (line[0] == '#')):
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

#===========================================================================
#
# Create CMakeLists.txt for directory recursion
#
#===========================================================================

def createCMakeListsRecurse(dir):

    if (options.debug):
        print("  creating CMakeLists.txt, dir: ", dir, file=sys.stderr)

    # go to the dir

    #currentDir = os.getcwd()
    #os.chdir(dir)

    # get makefile name in use

    makefilePath = getMakefilePath(dir)

    # load list of subdirs
    
    subdirList = getSubdirList(dir)

    if (options.debug == True):
        print("=======================", file=sys.stderr)
        print("subdirList:", file=sys.stderr)
        for subdir in subdirList:
            print("subdir: %s" % (subdir), file=sys.stderr)
        print("=======================", file=sys.stderr)

    # write out CMakeLists.txt for recursion
            
    writeCMakeListsRecurse(dir, subdirList)

    # go back to original dir
    
    #os.chdir(currentDir)

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
# Write out top level CMakeLists.txt

def writeCMakeListsTop(dir):

    cmakePath = os.path.join(dir, "CMakeLists.txt")
    if (options.debug):
        print("  writing top level: ", cmakePath, file=sys.stderr)
    fo = open(cmakePath, 'w')

    fo.write('###############################################\n')
    fo.write('#\n')
    fo.write('# Top-level CMakeLists file for lrose-core\n')
    fo.write('#\n')
    fo.write('# dir: %s\n' % dir)
    fo.write('#\n')
    fo.write('# written by script %s\n' % thisScriptName)
    fo.write('#\n')
    fo.write('# created %s\n' % datetime.now())
    fo.write('#\n')
    fo.write('###############################################\n')
    fo.write('\n')
    fo.write('cmake_minimum_required( VERSION 2.8 )\n')
    fo.write('\n')
    fo.write('project (lrose-core)\n')
    fo.write('\n')

    fo.write('set( CMAKE_C_COMPILER_NAMES clang gcc icc cc )\n')
    fo.write('set( CMAKE_CXX_COMPILER_NAMES clang++ g++ icpc c++ cxx )\n')
    fo.write('\n')

    if (options.verboseMake):
        fo.write('set( CMAKE_VERBOSE_MAKEFILE ON )\n')
    else:
        fo.write('set( CMAKE_VERBOSE_MAKEFILE OFF )\n')
    fo.write('\n')

    fo.write('SET( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/"\n')
    fo.write('     CACHE INTERNAL "Location of our custom CMake modules." )\n')
    fo.write('\n')

    fo.write('SET( CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/cmake" )\n')
    fo.write('\n')

    fo.write('set( FETCHCONTENT_QUIET false CACHE BOOL "" FORCE )\n')
    fo.write('\n')

    fo.write('set( PACKAGE "LROSE-CORE" CACHE STRING "" )\n')
    fo.write('\n')

    fo.write('find_package ( Qt5 COMPONENTS Widgets Network Qml REQUIRED PATHS /usr NO_DEFAULT_PATH )\n')
    fo.write('\n')

    fo.write('# If user did not provide CMAKE_INSTALL_PREFIX, use ~/lrose\n')
    fo.write('if( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )\n')
    fo.write('  set( CMAKE_INSTALL_PREFIX "$ENV{HOME}/lrose" CACHE PATH "..." FORCE )\n')
    fo.write('endif(  )\n')
    fo.write('message( "CMAKE_INSTALL_PREFIX is ${CMAKE_INSTALL_PREFIX}" )\n')
    fo.write('\n')

    fo.write('enable_testing(  )\n')
    fo.write('\n')

    fo.write('set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 " )\n')
    if (not options.withJasper):
        fo.write('set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_JASPER_LIB " )\n')
        # fo.write('add_definitions( -DNO_JASPER_LIB  )\n')

    fo.write('\n')

    fo.write('add_subdirectory( libs )\n')
    fo.write('add_subdirectory( apps )\n')
    fo.write('\n')
    fo.close

########################################################################
# Write out CMakeLists.am

def writeCMakeListsRecurse(dir, subdirList):

    fo = open("CMakeLists.txt", "w")

    fo.write("###############################################\n")
    fo.write("#\n")
    fo.write("# CMakeLists for cmake recursion\n")
    fo.write("#\n")
    fo.write("# dir: %s\n" % dir)
    fo.write("#\n")
    fo.write("# written by script %s\n" % thisScriptName)
    fo.write("#\n")
    fo.write("# created %s\n" % datetime.now())
    fo.write("#\n")
    fo.write("###############################################\n")
    fo.write("\n")
    fo.write("project (LROSE-CORE)\n")
    fo.write("\n")

    if (len(subdirList) > 0):
        fo.write("# subdirectories\n")
        fo.write("\n")
        for subdir in subdirList:
            fo.write("add_subdirectory (%s)\n" % subdir)

    fo.write("\n")
    fo.close

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
# append to list of files to be compiled

def addLibSubDirToCompileList(subDir, compileFileList):
                    
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

#===========================================================================
#
# Create CMakeLists.txt for library directory
#
#===========================================================================

def createCMakeListsLib(libDir, libList):

    if (options.debug):
        print("  creating CMakeLists.txt for lib, dir: ", libDir, file=sys.stderr)

    # compute the src dir

    libSrcDir = os.path.join(libDir, 'src')
    if (options.debug):
        print("lib src dir: ", libSrcDir, file=sys.stderr)
    # os.chdir(libSrcDir)

    # get makefile in use
    # makefile has preference over Makefile

    makefilePath = getMakefilePath(libSrcDir)
    if (makefilePath.find('not_found') == 0):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find makefile or Makefile", file=sys.stderr)
        print("  dir: ", options.dir, file=sys.stderr)
        return

    # get the lib name

    libName = getLibName(libSrcDir, makefilePath)
    if (options.debug):
        print("  Lib name: ", libName, file=sys.stderr)

    # get list of subdirs and their makefiles

    subDirList = getLibSubDirs(libSrcDir)
    if (options.debug):
        print("=======================", file=sys.stderr)
        for subDir in subDirList:
            print("subDir, makefile: %s, %s" % \
                (subDir.subDirName, subDir.makefilePath), file=sys.stderr)
        print("=======================", file=sys.stderr)

    # load list of files to be compiled

    libCompileFileList = []
    for subDir in subDirList:
        addLibSubDirToCompileList(subDir, libCompileFileList)

    if (options.debug):
        print("======== lib compfile list ===============", file=sys.stderr)
        for compileFile in libCompileFileList:
            print("compileFile: %s" % (compileFile), file=sys.stderr)
        print("==========================================", file=sys.stderr)

    # write out CMakeLists.txt

    writeCMakeListsLib(libName, libSrcDir, libList, libCompileFileList)

########################################################################
# Write out CMakeLists.txt

def writeCMakeListsLib(libName, libDir, libList, compileFileList):

    cmakePath = os.path.join(libDir, 'CMakeLists.txt')
    fo = open(cmakePath, "w")

    fo.write("###############################################\n")
    fo.write("#\n")
    fo.write("# CMakeLists.txt - auto generated from Makefile\n")
    fo.write("#\n")
    fo.write("# library name: %s\n" % libName)
    fo.write("#\n")
    fo.write("# written by script createCMakeLists.lib.py\n")
    fo.write("#\n")
    fo.write("# created %s\n" % datetime.now())
    fo.write("#\n")
    fo.write("###############################################\n")
    fo.write("\n")

    fo.write("project ( lib%s )\n" % libName)
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
        fo.write("add_library (%s STATIC ${SRCS} )\n" % libName)
    else:
        fo.write("add_library (%s SHARED ${SRCS} )\n" % libName)
    fo.write("\n")

    fo.write("# install\n")
    fo.write("\n")
    fo.write("INSTALL(TARGETS %s\n" % libName)
    fo.write("        DESTINATION $ENV{LROSE_INSTALL_DIR}/lib\n")
    fo.write("        )\n")
    fo.write("INSTALL(DIRECTORY include/%s\n" % libName)
    fo.write("        DESTINATION $ENV{LROSE_INSTALL_DIR}/include\n")
    fo.write("        )\n")
    fo.write("\n")

    fo.close
    return

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
        print("Execution failed:", e, retcode, file=sys.stderr)
        exit(1)

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

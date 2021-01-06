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

def main():

    global options
    global coreDir
    global codebaseDir

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global nLibs, nApps, nRecurse, nOther, nTotal
    nLibs = 0
    nApps = 0
    nRecurse = 0
    nOther = 0
    nTotal = 0

    # We will be executing some sibling scripts. Get our path so that
    # the sibling scripts from the same path can be executed explicitly.
    global thisScriptDir
    thisScriptDir = os.path.dirname(os.path.abspath(__file__))

    # determine the OS type

    osType = getOsType()
    isDebianBased = False
    if (osType == "debian"):
        isDebianBased = True
    if (osType == "ubuntu"):
        isDebianBased = True

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
    parser.add_option('--installDir',
                      dest='installDir', default='',
                      help='Path of lrose install dir, default is ~/lrose')
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
    parser.add_option('--renewTemplates',
                      dest='renewTemplates', default=False,
                      action="store_true",
                      help='Copy Makefile to __makefile.template for all directories used')
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
        print("=============================================", file=sys.stderr)
        print("Running %s      :" % thisScriptName, file=sys.stderr)
        print("  coreDir       : ", coreDir, file=sys.stderr)
        print("  codebaseDir   : ", codebaseDir, file=sys.stderr)
        print("  libs dir      : ", libsDir, file=sys.stderr)
        print("  apps dir      : ", appsDir, file=sys.stderr)
        if (len(options.installDir) == 0):
            print("  install dir   :  ~/lrose", file=sys.stderr)
        else:
            print("  install dir   : ", options.installDir, file=sys.stderr)
        print("  static        : ", options.static, file=sys.stderr)
        print("  pkg           : ", options.pkg, file=sys.stderr)
        print("  osx           : ", options.osx, file=sys.stderr)
        print("  verboseMake   : ", options.verboseMake, file=sys.stderr)
        print("  withJasper    : ", options.withJasper, file=sys.stderr)
        print("=============================================", file=sys.stderr)

    # go to the top level codebase directory

    os.chdir(codebaseDir)

    # write the top level CMakeLists.txt file

    if (options.debug):
        print("=============================================", file=sys.stderr)
        print("Writing to top-level codebase dir: ",
              codebaseDir, file=sys.stderr)
    writeCMakeListsTop(codebaseDir)
    nOther = nOther + 1
    
    # get list of libs
    
    if (options.debug):
        print("=============================================", file=sys.stderr)
        print("Getting lib list from dir: ", libsDir, file=sys.stderr)

    libList = getLibList(libsDir)

    # recursively search libs and apps for makefiles

    if (options.debug):
        print("#############################################", file=sys.stderr)
        print("Searching libs, dir: ", libsDir, file=sys.stderr)

    searchDirRecurse(libsDir, libList)

    if (options.debug):
        print("#############################################", file=sys.stderr)
        print("Searching apps, dir: ", libsDir, file=sys.stderr)

    searchDirRecurse(appsDir, libList)

    nTotal = nRecurse + nLibs + nApps + nOther
    if (options.debug):
        print("=============================================", file=sys.stderr)
        print("==>> CMakeLists.txt files created", file=sys.stderr)
        print("==>>   nRecurse: ", nRecurse, file=sys.stderr)
        print("==>>   nLibs   : ", nLibs, file=sys.stderr)
        print("==>>   nApps   : ", nApps, file=sys.stderr)
        print("==>>   nOther  : ", nOther, file=sys.stderr)
        print("==>>   nTotal  : ", nTotal, file=sys.stderr)

    sys.exit(0)

########################################################################
# search libs directory to compile libs list

def getLibList(dir):
                    
    libList = []

    # check if this dir has a makefile or Makefile

    makefilePath = getMakefilePath(dir)
    if (os.path.exists(makefilePath) == False):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  No makefile in lib dir: ", dir, file=sys.stderr)
        sys.exit(1)

    if (options.debug):
        print("Using makefile: ", makefilePath, file=sys.stderr)

    # search for SUB_DIRS key in makefile

    subNameList = getValuesForKey(makefilePath, "SUB_DIRS")

    if (len(subNameList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find SUB_DIRS in ", makefilePath, file=sys.stderr)
        print("  coreDir: ", options.coreDir, file=sys.stderr)
        sys.exit(1)

    for subName in subNameList:
        subPath = os.path.join(dir, subName)
        if (os.path.isdir(subPath)):
            if (subName.find("perl5") < 0):
                libList.append(subName)

    if (options.debug):
        print("libList: ", libList, file=sys.stderr)

    return libList

########################################################################
# search directory and subdirectories

def searchDirRecurse(dir, libList):
                    
    global nLibs, nApps, nRecurse

    if (options.debug):
        print("*********************************************", file=sys.stderr)
        print("Recursively searching, dir: ", dir, file=sys.stderr)

    # check if this dir has a makefile or Makefile

    makefilePath = getMakefilePath(dir)
    if (os.path.exists(makefilePath) == False):
        if (options.verbose):
            print("  No makefile or Makefile found", file=sys.stderr)
        return

    # detect which type of directory we are in
        
    if ((dir == "libs/perl5") or
        (dir == "apps/scripts")):
        if (options.debug):
            print("=====>> Ignoring dir:", dir, file=sys.stderr)
        return

    thisDir = os.path.join(options.coreDir, dir)
    pathToks = thisDir.split("/")
    ntoks = len(pathToks)

    if (pathToks[ntoks-3] == "libs" and
        pathToks[ntoks-1] == "src"):

        # src level of lib - create CMakeLists.txt for lib

        libDir = thisDir[:-4]
        createCMakeListsLib(libDir, libList)
        nLibs = nLibs + 1

    elif ((pathToks[ntoks-4] == "apps") and
          (pathToks[ntoks-2] == "src")):

        # app directory - create CMakeLists.txt for app

        createCMakeListsApp(thisDir, libList)
        nApps = nApps + 1

    elif ((pathToks[ntoks-4] == "apps") and
          (pathToks[ntoks-2] == "src") and
          (pathToks[ntoks-1] == "scripts")):

        # scripts dir - do nothing
        if (options.debug):
            print("  Ignoring dir:", dir, file=sys.stderr)

    else:

        # create CMakeLists.txt for recursion
        createCMakeListsRecurse(thisDir)
        nRecurse = nRecurse + 1
        # recurse
        subdirList = getSubdirList(dir)
        for subdir in subdirList:
            subdirPath = os.path.join(dir, subdir)
            searchDirRecurse(subdirPath, libList)

    return

########################################################################
# load list of sub directories

def getSubdirList(dir):
                    
    subdirList = []
    
    # get makefile for this dir

    makefilePath = getMakefilePath(dir)

    # search for SUB_DIRS key in makefile

    subNameList = getValuesForKey(makefilePath, "SUB_DIRS")

    if (len(subNameList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find SUB_DIRS in ", makefilePath, file=sys.stderr)
        print("  dir: ", dir, file=sys.stderr)
        sys.exit(1)

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

    # if template does not exist,
    # copy makefile to template for later use if needed

    if (makefilePath.find('__makefile.template') < 0):
        templatePath = os.path.join(dir, '__makefile.template')
        shutil.copy(makefilePath, templatePath)

    if (options.renewTemplates):
        templatePath = os.path.join(dir, '__makefile.template')
        MakePath = os.path.join(dir, 'Makefile')
        shutil.copy(MakePath, templatePath)

    return makefilePath

########################################################################
# get string value based on search key
# the string may span multiple lines
#
# Example of keys: SRCS, SUB_DIRS, MODULE_NAME, TARGET_FILE
#
# value is returned

def getValuesForKey(makefilePath, key):

    valueList = []

    try:
        fp = open(makefilePath, 'r')
    except IOError as e:
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot open file:", makefilePath, file=sys.stderr)
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

    # get makefile name in use

    makefilePath = getMakefilePath(dir)

    # load list of subdirs
    
    subdirList = getSubdirList(dir)

    # write out CMakeLists.txt for recursion
            
    writeCMakeListsRecurse(dir, subdirList)

########################################################################
# get list of makefiles for library
# using LROSE Makefile to locate subdirs

def getLibSubDirs(libDir):

    subDirs = []

    # search for SUB_DIRS key in makefile

    makefilePath = getMakefilePath(libDir)
    subDirNames = getValuesForKey(makefilePath, "SUB_DIRS")

    for subDirName in subDirNames:
        subDirPath = os.path.join(libDir, subDirName)
        if (os.path.isdir(subDirPath)):
            subDirs.append(subDirName)

    return subDirs

########################################################################
# Write out top level CMakeLists.txt

def writeCMakeListsTop(dir):

    cmakePath = os.path.join(dir, "CMakeLists.txt")
    if (options.debug):
        print("--->> Writing top level CMakeLists.txt, dir: ",
              dir, file=sys.stderr)
        print("     ", cmakePath, file=sys.stderr)

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

    if (len(options.installDir) == 0):
        fo.write('# If user did not provide CMAKE_INSTALL_PREFIX, use ~/lrose\n')
        fo.write('if( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )\n')
        fo.write('  set( CMAKE_INSTALL_PREFIX "$ENV{HOME}/lrose" CACHE PATH "..." FORCE )\n')
        fo.write('endif(  )\n')
    else:
        fo.write('set( CMAKE_INSTALL_PREFIX %s CACHE PATH "..." FORCE )\n' % options.installDir)
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

    cmakePath = os.path.join(dir, 'CMakeLists.txt')

    if (options.debug):
        print("--->> Writing recursive CMakeLists.txt, dir: ",
              dir, file=sys.stderr)
        print("     ", cmakePath, file=sys.stderr)

    fo = open(cmakePath, "w")

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

#===========================================================================
#
# Create CMakeLists.txt for library directory
#
#===========================================================================

def createCMakeListsLib(libDir, libList):

    # compute the src dir

    libSrcDir = os.path.join(libDir, 'src')

    # get makefile in use
    # makefile has preference over Makefile

    makefilePath = getMakefilePath(libSrcDir)
    if (makefilePath.find('not_found') == 0):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find makefile or Makefile", file=sys.stderr)
        print("  dir: ", options.dir, file=sys.stderr)
        return

    if (options.debug):
        print("Using makefile: ", makefilePath, file=sys.stderr)

    # get the lib name

    libName = getLibName(libSrcDir, makefilePath)
    if (options.debug):
        print("=============================================", file=sys.stderr)
        print("Lib name: ", libName, file=sys.stderr)

    # get list of subdirs and their makefiles

    libSubDirNames = getLibSubDirs(libSrcDir)
    if (options.verbose):
        for subDirName in libSubDirNames:
            print("==>> subDir: %s" % subDirName, file=sys.stderr)

    # load list of files to be compiled

    libCompileFileList = []
    for subDirName in libSubDirNames:
        subDirPath = os.path.join(libSrcDir, subDirName)
        srcNames = getLibSrcNames(subDirPath)
        for srcName in srcNames:
            relPath = os.path.join('.', subDirName)
            relPath = os.path.join(relPath, srcName)
            libCompileFileList.append(relPath)

    if (options.verbose):
        print("-------- lib compile list ---------------", file=sys.stderr)
        for compileFile in libCompileFileList:
            print("  compileFile: %s" % (compileFile), file=sys.stderr)
        print("-----------------------------------------", file=sys.stderr)

    # write out CMakeLists.txt

    writeCMakeListsLib(libName, libSrcDir, libList, libCompileFileList)

########################################################################
# parse the LROSE Makefile to get the lib name

def getLibName(dir, makefilePath):

    # search for MODULE_NAME key in makefile

    valList = getValuesForKey(makefilePath, "MODULE_NAME")

    if (len(valList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find MODULE_NAME in ", makefileName, file=sys.stderr)
        print("  dir: ", dir, file=sys.stderr)
        sys.exit(1)

    libName = valList[len(valList)-1]
    return libName

########################################################################
# append to list of files to be compiled

def getLibSrcNames(subDir):
                    
    srcNames = []

    makefilePath = getMakefilePath(subDir)
    if (makefilePath.find('not-found') == 0):
        return srcNames

    fp = open(makefilePath, 'r')
    makefileLines = fp.readlines()
    
    srcTypeList = [ 'SRCS', 'C_SRCS', 'F_SRCS', 'F_CPP_SRCS', 
                    'F90_SRCS', 'F95_SRCS', 'PGF90_SRCS', 
                    'PGF_SRCS', 'CC_SRCS', 'CPPC_SRCS', 
                    'CPP_SRCS', 'CXX_SRCS' ]
    
    for srcType in srcTypeList:
        srcNames.extend(getLibSrcNamesByType(makefileLines, srcType))

    return srcNames

########################################################################
# get lib srcs for a given type

def getLibSrcNamesByType(makefileLines, srcType):

    srcNames = []

    # build up multiLine string containing all compile files

    srcTypeFound = False
    multiLine = ""
    for line in makefileLines:
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
        return srcNames

    # remove strings we don't want

    multiLine = multiLine.replace(srcType, " ")
    multiLine = multiLine.replace("=", " ")
    multiLine = multiLine.replace("\t", " ")
    multiLine = multiLine.replace("\\", " ")
    multiLine = multiLine.replace("\r", " ")
    multiLine = multiLine.replace("\n", " ")

    names = multiLine.split(' ')
    for name in names:
        if (name.find(".") > 0):
            srcNames.append(name)

    return srcNames

########################################################################
# Write out CMakeLists.txt

def writeCMakeListsLib(libName, libSrcDir, libList, compileFileList):

    cmakePath = os.path.join(libSrcDir, 'CMakeLists.txt')

    if (options.debug):
        print("--->> Writing CMakeLists.txt for lib: ",
              libName, file=sys.stderr)
        print("     ", cmakePath, file=sys.stderr)

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
    fo.write("include_directories ( ${CMAKE_INSTALL_PREFIX}/include )\n")
    fo.write("\n")

    fo.write("# source files\n")
    fo.write("\n")
    fo.write("set ( SRCS\n")
    for compileFile in compileFileList:
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
    fo.write("        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib\n")
    fo.write("        )\n")
    fo.write("INSTALL(DIRECTORY include/%s\n" % libName)
    fo.write("        DESTINATION ${CMAKE_INSTALL_PREFIX}/include\n")
    fo.write("        )\n")
    fo.write("\n")

    fo.close
    return

#===========================================================================
#
# Create CMakeLists.txt for application
#
#===========================================================================

def createCMakeListsApp(appDir, libList):

    # get makefile in use
    # makefile has preference over Makefile

    makefilePath = getMakefilePath(appDir)
    if (makefilePath.find('not_found') == 0):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find makefile or Makefile", file=sys.stderr)
        print("  app dir: ", appDir, file=sys.stderr)
        return

    if (options.debug):
        print("Using makefile: ", makefilePath, file=sys.stderr)

    # parse the LROSE Makefile to get the app name

    appName = getAppName(makefilePath)
    if (appName.find('not-found') == 0):
        return

    # load list of files to be compiled

    appCompileFileList = getAppCompileList(appName, makefilePath)
    if (options.debug):
        print("==>> appName: %s" % appName, file=sys.stderr)
        for compileFile in appCompileFileList:
            print("  app compile file: %s" % (compileFile), file=sys.stderr)
        print("  =======================", file=sys.stderr)

    # get list of libs to be linked with
        
    makefileLibList = getLinkLibList(makefilePath)

    if (options.static):
        # for static libs, use libs from makefile
        linkLibList = makefileLibList
    else:
        # for shared libs, we need to link with all lib
        # order the list
        linkOrder = getLroseLinkOrder()
        linkLibList = []
        for lib in linkOrder:
            if (lib in libList):
                linkLibList.append(lib)
        for lib in makefileLibList:
            if (lib not in linkLibList):
                linkLibList.append(lib)

    extendedLibs = getExtendedLibs(linkLibList)
    linkLibList.extend(extendedLibs)
                
    # check if we need Qt support

    needQt = checkForQt(makefilePath)
    needX11 = checkForX11(makefilePath)
    
    # write out CMakeLists.txt

    writeCMakeListsApp(appName, appDir, appCompileFileList,
                       libList, linkLibList, needQt, needX11)

########################################################################
# parse the LROSE Makefile to get the app name

def getAppName(makefilePath):

    # search for TARGET_FILE key in makefile

    valList = getValuesForKey(makefilePath, "TARGET_FILE")

    if (len(valList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find TARGET_FILE in ", makefilePath, file=sys.stderr)
        return "not-found"

    appName = valList[len(valList)-1]
    return appName

########################################################################
# get list of files to be compiled for an app

def getAppCompileList(appName, makefilePath):

    appCompileList = []

    try:
        fp = open(makefilePath, 'r')
    except IOError as e:
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot open: ", makefilePath, file=sys.stderr)
        sys.exit(1)

    makefileLines = fp.readlines()
    fp.close()

    srcTypeList = [ 'SRCS', 'C_SRCS', 'F_SRCS', 'F_CPP_SRCS', 
                    'F90_SRCS', 'F95_SRCS', 'PGF90_SRCS', 
                    'PGF_SRCS', 'CC_SRCS', 'CPPC_SRCS', 
                    'CPP_SRCS', 'CXX_SRCS', 'NORM_SRCS', 'MOC_SRCS' ]

    for srcType in srcTypeList:
        appendSrcTypeApp(appName, makefileLines, srcType, appCompileList)

    return appCompileList
    
########################################################################
# append to compile list for app for given srcType

def appendSrcTypeApp(appName, makefileLines, srcType, appCompileList):

    # build up multiLine string containing all compile files

    srcTypeFound = False
    multiLine = ""
    for line in makefileLines:
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

    names = multiLine.split(' ')
    for name in names:
        if (name.find(".") > 0):
            appCompileList.append(name)
        elif (name == "$(PARAMS_CC)"):
            appCompileList.append("Params.cc")
        elif (name == "$(TDRP_C)"):
            appCompileList.append(appName + "_tdrp.c")
        elif (name == "$(_TDRP_C)"):
            appCompileList.append("_tdrp.c")

########################################################################
# get list of libraries to be linked in makefile for an app

def getLinkLibList(makefilePath):

    linkLibList = []

    # search for LOC_LIBS key in makefile

    locLibs = getValuesForKey(makefilePath, "LOC_LIBS")
    for line in locLibs:
        linkLibList.extend(decodeLibLine(line))

    return linkLibList
            
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
            libs.append("hdf5_cpp")
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
# get link order for lrose libraries

def getLroseLinkOrder():

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
# get extended list of libraries to be loaded for shared libs

def getExtendedLibs(linkLibList):

    # extend the lib list with required standard libs

    if (options.osx):
        extendLibs = [ 'Ncxx',
                       'netcdf',
                       'hdf5_cpp',
                       'hdf5_hl',
                       'hdf5',
                       'fftw3',
                       'X11',
                       'Xext',
                       'pthread',
                       'png',
                       'z',
                       'bz2',
                       'm' ]
    else:
        extendLibs = [ 'Ncxx',
                       'netcdf',
                       'hdf5_cpp',
                       'hdf5_hl',
                       'hdf5',
                       'fftw3',
                       'X11',
                       'Xext',
                       'pthread',
                       'png',
                       'z',
                       'bz2',
                       'm',
                       'gfortran' ]

    if (options.withJasper):
        extendLibs.append('jasper')
    
    #if ("radar" in linkLibList and "fftw3" not in linkLibList):
    #    extendLibs.append("fftw3")

    return extendLibs

########################################################################
# check for dependence on QT

def checkForQt(makefilePath):
                    
    try:
        fp = open(makefilePath, 'r')
    except IOError as e:
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot open: ", makefileName, file=sys.stderr)
        sys.exit(1)

    makefileLines = fp.readlines()
    fp.close()

    for line in makefileLines:
        if (line.find("QT") >= 0):
            return True

    return False
    
########################################################################
# check for dependence on X11

def checkForX11(makefilePath):
                    
    try:
        fp = open(makefilePath, 'r')
    except IOError as e:
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot open: ", makefileName, file=sys.stderr)
        sys.exit(1)

    makefileLines = fp.readlines()
    fp.close()

    for line in makefileLines:
        if (line.find("X11") >= 0):
            return True

    return False
    
########################################################################
# write out CMakeLists.txt for app

def writeCMakeListsApp(appName, appDir, appCompileFileList,
                       libList, linkLibList, needQt, needX11):

    cmakePath = os.path.join(appDir, 'CMakeLists.txt')

    if (options.debug):
        print("--->> Writing CMakeLists.txt for app: ",
              appName, file=sys.stderr)
        print("     ", cmakePath, file=sys.stderr)

    fo = open(cmakePath, "w")

    fo.write("###############################################\n")
    fo.write("#\n")
    fo.write("# CMakeLists.txt file for cmake\n")
    fo.write("#\n")
    fo.write("# app name: %s\n" % appName)
    fo.write("#\n")
    fo.write("# written by script %s\n" % thisScriptName)
    fo.write("#\n")
    fo.write("# created %s\n" % datetime.now())
    fo.write("#\n")
    fo.write("# dir: %s\n" % appDir)
    fo.write("# osx: %s\n" % options.osx)
    fo.write("###############################################\n")
    fo.write("\n")

    fo.write("project ( %s )\n" % appName)
    fo.write("\n")
    
    fo.write("# source files\n")
    fo.write("\n")
    fo.write("set ( SRCS\n")
    for compileFile in appCompileFileList:
        fo.write("      %s\n" % compileFile)
    fo.write("    )\n")
    fo.write("\n")

    fo.write("# includes\n")
    fo.write("\n")
    for lib in libList:
        fo.write("include_directories ( ../../../../libs/%s/src/include )\n" % lib)
    fo.write("include_directories( ${CMAKE_INSTALL_PREFIX}/include )\n")
    fo.write("\n")

    fo.write("# link directories\n")
    fo.write("\n")
    fo.write("link_directories( ${CMAKE_INSTALL_PREFIX}/lib )\n")
    fo.write("\n")

    fo.write("# link libs\n")
    fo.write("\n")
    for lib in linkLibList:
        fo.write("link_libraries ( %s )\n" % lib)
    if (needQt):
        fo.write("link_libraries ( ${Qt5Core_LIBRARIES} )\n")
        fo.write("link_libraries ( ${Qt5Gui_LIBRARIES} )\n")
        fo.write("link_libraries ( ${Qt5Widgets_LIBRARIES} )\n")
        fo.write("link_libraries ( ${Qt5Network_LIBRARIES} )\n")
        fo.write("link_libraries ( ${Qt5Qml_LIBRARIES} )\n")
    fo.write("\n")

    if (needQt):
        fo.write("# QT5\n")
        fo.write("\n")
        fo.write("set ( CMAKE_INCLUDE_CURRENT_DIR ON )\n")
        fo.write("set ( CMAKE_AUTOMOC ON )\n")
        fo.write("set ( CMAKE_AUTORCC ON )\n")
        fo.write("set ( CMAKE_AUTOUIC ON )\n")
        fo.write("find_package (Qt5 COMPONENTS Widgets Network Qml REQUIRED PATHS /usr NO_DEFAULT_PATH )\n")
        fo.write("\n")

        fo.write("include( FindPkgConfig )\n")
        fo.write("pkg_search_module( Qt5Core REQUIRED )\n")
        fo.write("pkg_search_module( Qt5Gui REQUIRED )\n")
        fo.write("pkg_search_module( Qt5Widgets REQUIRED )\n")
        fo.write("pkg_search_module( Qt5Network REQUIRED )\n")
        fo.write("pkg_search_module( Qt5Qml REQUIRED )\n")
        fo.write("\n")

        fo.write("include_directories( ${Qt5Core_INCLUDE_DIRS} )\n")
        fo.write("include_directories( ${Qt5Gui_INCLUDE_DIRS} )\n")
        fo.write("include_directories( ${Qt5Widgets_INCLUDE_DIRS} )\n")
        fo.write("include_directories( ${Qt5Network_INCLUDE_DIRS} )\n")
        fo.write("include_directories( ${Qt5Qml_INCLUDE_DIRS} )\n")
        fo.write("\n")

    fo.write("# application\n")
    fo.write("\n")
    fo.write("add_executable ( %s ${SRCS} )\n" % appName)
    fo.write("\n")

    fo.write("# install\n")
    fo.write("\n")
    fo.write("INSTALL(TARGETS ${PROJECT_NAME}\n")
    fo.write("        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin\n")
    fo.write("        )\n")
    fo.write("\n")

    fo.close
    return
    

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
# Run a command in a shell, wait for it to complete

def runCommand(cmd):

    if (options.debug):
        print("running cmd:",cmd, file=sys.stderr)
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print("Child exited with code: ", retcode, file=sys.stderr)
            sys.exit(1)
        else:
            if (options.verbose):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, retcode, file=sys.stderr)
        sys.exit(1)

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()
 

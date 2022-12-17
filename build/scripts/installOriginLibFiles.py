#!/usr/bin/env python3

# ========================================================================== #
#
# Find the unique set of dynamic lib files for all of the binaries
# in a given directory.
# Copy those dynamic libraries to a select location, normally:
#   $ORIGIN/../rel_origin/lib
#
# ========================================================================== #

from __future__ import print_function
import os
import sys
import shutil
import subprocess

import string
from os.path import join, getsize
import subprocess
from optparse import OptionParser
from sys import platform

def main():

    global options
    global debug
    global ignoreList

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    # parse the command line

    usage = "usage: %prog [options]: prints catalog to stdout"
    #ignoreDefault = 'libc.so,libpthread.so,libdl.so,libX11.so'
    if (platform == "darwin"):
        # OSX
        ignoreDefault = 'libapple,libc++,libcache,libclosured' + \
                        ',libcommon,libcompiler,libcopy,libcore' + \
                        ',libcrypto,libdispatch,libdyld,libkeymgr' + \
                        ',libobjc,libremovefile,libsystem,libunwind' + \
                        ',libpthread,libdl,libX,libxpc'
    else:
        # LINUX
        ignoreDefault = 'libc.so,libpthread.so,libdl.so,libm.so'

    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--binDir',
                      dest='binDir', default='.',
                      help='Path to installed binaries')
    parser.add_option('--relDir',
                      dest='relDir', default='../rel_origin/lib',
                      help='Path of installed libs relative to bins')
    parser.add_option('--ignore',
                      dest='ignore',
                      default=ignoreDefault,
                      help='Comma-delimited list of libs to ignore. ' +
                      'Any lib containing these strings will not be ' +
                      'included in the install. Default is: ' +
                      ignoreDefault)
    parser.add_option('--verbose',
                      dest='verbose', default=False,
                      action="store_true",
                      help='Set verbose debugging on')

    (options, args) = parser.parse_args()

    ignoreList = options.ignore.split(',')

    if (options.verbose):
        options.debug = True

    if (options.debug == True):
        print("  Running " + thisScriptName, file=sys.stderr)
        print("    platform: ", platform, file=sys.stderr)
        print("  Options:", file=sys.stderr)
        print("    debug: ", options.debug, file=sys.stderr)
        print("    verbose: ", options.verbose, file=sys.stderr)
        print("    binDir: ", options.binDir, file=sys.stderr)
        print("    relDir: ", options.relDir, file=sys.stderr)
        print("    ignore: ", options.ignore, file=sys.stderr)
        print("    ignoreList: ", ignoreList, file=sys.stderr)

    # create list of binaries in install dir
    
    binPathList = []
    fileList = os.listdir(options.binDir)
    for fileName in fileList:
        binPath = os.path.join(options.binDir, fileName)
        if (fileIsBinary(binPath)):
            binPathList.append(binPath)

    if (options.debug):
        print("------------------------------------------------", file=sys.stderr)
        print("====>> binPathList: ", file=sys.stderr)
        print("====>> nBins: ", len(binPathList), file=sys.stderr)
        for binPath in binPathList:
            print("  ", binPath, file=sys.stderr)
        print("------------------------------------------------", file=sys.stderr)
        
    # compile dictionary of dynamic libraries
    # that are dependent on the binaries
    # also keep a lookup of libs for each bin file

    binPathLookup = {}
    libsInBins = {}
    for binPath in binPathList:
        libsInFile = {}
        findLibsForFile(binPath, libsInFile)
        binPathLookup[binPath] = libsInFile
        libsInBins.update(libsInFile)

    if (options.verbose):
        print("------------------------------------------------", file=sys.stderr)
        print("====>> dictionary of libs in bins: ", file=sys.stderr)
        print("====>> nLibs: ", len(libsInBins), file=sys.stderr)
        for libName in list(libsInBins.keys()):
            libPath = libsInBins[libName]
            print("  ", libName, ",", libPath, file=sys.stderr)
        print("------------------------------------------------", file=sys.stderr)

    # now for each lib add any sub libs needed
    # also keep a lookup of libs for each lib file
    
    libPathLookup = {}
    libsInLibs = {}
    for libName in list(libsInBins.keys()):
        libPath = libsInBins[libName]
        libsInFile = {}
        findLibsForFile(libPath, libsInFile)
        libPathLookup[libPath] = libsInFile
        libsInLibs.update(libsInFile)

    # now do the same for the libs in libs, until the count does not change

    startCount = -1
    endCount = 0
    while (startCount != endCount):
        startCount = len(libPathLookup)
        libsInLibs2 = {}
        for libName in list(libsInLibs.keys()):
            libPath = libsInLibs[libName]
            libsInFile = {}
            findLibsForFile(libPath, libsInFile)
            libPathLookup[libPath] = libsInFile
            libsInLibs2.update(libsInFile)
        libsInLibs.update(libsInLibs2)
        endCount = len(libPathLookup)

    if (options.verbose):
        print("------------------------------------------------", file=sys.stderr)
        print("====>> dictionary of libs in libs: ", file=sys.stderr)
        print("====>> nLibs: ", len(libsInLibs), file=sys.stderr)
        for libName in list(libsInLibs.keys()):
            libPath = libsInLibs[libName]
            print("  ", libName, ",", libPath, file=sys.stderr)
        print("------------------------------------------------", file=sys.stderr)

    # merge lib dicts

    libsInAll = dict(list(libsInBins.items()) + list(libsInLibs.items()))

    if (options.debug):
        print("------------------------------------------------", file=sys.stderr)
        print("====>> dictionary of all libs: ", file=sys.stderr)
        print("====>> nLibs: ", len(libsInAll), file=sys.stderr)
        for libName in list(libsInAll.keys()):
            libPath = libsInAll[libName]
            print("  ", libName, ",", libPath, file=sys.stderr)
        print("------------------------------------------------", file=sys.stderr)

    # copy each library file into runtime area

    for libName in list(libsInAll.keys()):
        libPath = libsInAll[libName]
        copyLibToRelDir(libName, libPath)

    # for LINUX we are done

    if (platform != "darwin"):
        sys.exit(0)

    # for OSX, modify the binaries so that their
    # embedded dynamic library paths point to the relative location

    for binPath in binPathList:
        libsInBin = binPathLookup[binPath]
        for libName in list(libsInBin.keys()):
            libPath = libsInBin[libName]
            modifyBinSubPathOsx(binPath, libName, libPath)

    # for OSX, modify the copied libraries so that their
    # id is correct for relative location and that their
    # embedded dynamic library paths point to the relative location
    
    for libName in list(libsInAll.keys()):
        libPath = libsInAll[libName]
        modifyLibIdOsx(libName)
        libsInLib = libPathLookup[libPath]
        for subLibName in list(libsInLib.keys()):
            subLibPath = libsInLib[subLibName]
            modifyLibSubPathOsx(libName, subLibName, subLibPath)

    # done

    sys.exit(0)

########################################################################
# Find the dynamic libs for a specified binary file
# add to dictionary

def findLibsForFile(filePath, validLibs):
    
    if (options.verbose):
        print("===>>> finding libs for file: ", filePath, file=sys.stderr)

    # get list of dynamic libs
    
    if (platform == "darwin"):
        # OSX
        cmd = 'otool -L '
    else:
        # LINUX
        cmd = 'ldd '
        
    pipe = subprocess.Popen(cmd + filePath, shell=True,
                            stdout=subprocess.PIPE).stdout
    lines = pipe.readlines()

    # loop through lines, adding to temporart lib dictionary

    allLibs = {}

    for line in lines:

        lineStr = bytes.decode(line)
        lineStr.strip()
        lineToks = lineStr.split()
        libName = ''
        libPath = ''

        if (platform == "darwin"):

            # OSX

            if (len(lineToks) < 2):
                continue
            if (lineStr.find('compatibility') < 0):
                continue;
            if (lineStr.find('dylib') < 0):
                continue;
            libPath = lineToks[0]
            libToks = libPath.split('/')
            libName = libToks[-1]
            allLibs[libName] = libPath

        else:

            # LINUX

            for ii, tok in enumerate(lineToks):
                if ((tok == '=>') and (ii < len(lineToks) - 1)):
                    libName = lineToks[ii-1]
                    libPath = lineToks[ii+1]
                    if (libPath.find(libName) < 0):
                        continue
                    allLibs[libName] = libPath
                    break

    # loop through libs

    for libName in list(allLibs.keys()):

        libPath = allLibs[libName]

        # should we ignore this lib?

        ignoreThisLib = False
        for ignoreStr in ignoreList:
            if (len(ignoreStr) > 0 and libName.find(ignoreStr) >= 0):
                # ignore this library - belongs to system
                ignoreThisLib = True
                if (options.verbose):
                    print("  ===>>> ignoring lib: ", libName, file=sys.stderr)
                break

        if (ignoreThisLib == False):
            # add to main dictionary
            validLibs[libName] = libPath
            if (options.verbose):
                print("  ===>>> adding lib: ", libName, file=sys.stderr)

########################################################################
# Test if a file is an execultable or library
# Returns True for binary files, False otherwise

def fileIsBinary(filePath):

    # is this a file?

    if (os.path.isfile(filePath) == False):
        return False

    # check the file is binary

    if (options.verbose):
        print("==>> Checking file is binary: ", filePath, file=sys.stderr)

    pipe = subprocess.Popen('file ' + filePath, shell=True,
                            stdout=subprocess.PIPE).stdout
    lines = pipe.readlines()
    isExecFile = False

    if (platform == "darwin"):
        # MAC OSX
        for line in lines:
            lineStr = bytes.decode(line)
            if (lineStr.find('Mach-O') >= 0):
                isExecFile = True
                break

    else:
        # LINUX
        for line in lines:
            lineStr = bytes.decode(line)
            if (lineStr.find('ELF') >= 0):
                isExecFile = True
                break

    if (isExecFile):
        if (options.verbose):
            print("      isExecutable: ", filePath, file=sys.stderr)

    return isExecFile

########################################################################
# Copy the lib to the directory relative to the binary install path

def copyLibToRelDir(libName, libPath):

    destDir = os.path.join(options.binDir, options.relDir)
    libCopyPath = os.path.join(destDir, libName)
    
    if (libPath.find("loader_path") >= 0):
        if (options.debug):
            print("===>>> lib contains loader_path, ignoring: ", libPath, file=sys.stderr)
        return
            
    if (options.debug):
        print("===>>> Copying lib: ", libPath, file=sys.stderr)
        print("                to: ", destDir, file=sys.stderr)

    # ensure directory exists
    
    if not os.path.exists(destDir):
        os.makedirs(destDir)

    # copy in lib file, converting links into files
    
    cmd = "rsync -avL " + libPath + " " + destDir
    runCommand(cmd)

########################################################################
# Modify a library dependency paths in an executable file

def modifyBinSubPathOsx(binPath, libName, libPath):

    if (options.debug):
        print("===>>> modifyBinSubPathOsx, binPath, libName, libPath: ", \
            binPath, ",", libName, ",", libPath, file=sys.stderr)

    libRelDir = os.path.join("@loader_path", options.relDir)
    libRelPath = os.path.join(libRelDir, libName)
    
    # make writable

    cmd = 'chmod +w ' + binPath
    runCommand(cmd)

    # change runtime paths

    cmd = 'install_name_tool -change "' + libPath + '" "' + libRelPath + '" ' + binPath

    runCommand(cmd)

########################################################################
# Modify a library id

def modifyLibIdOsx(libName):

    if (options.debug):
        print("===>>> modifyLibIdOsx, libName: ", libName, file=sys.stderr)

    libRelDir = os.path.join("@loader_path", options.relDir)
    libRelPath = os.path.join(libRelDir, libName)

    installedDir = os.path.join(options.binDir, options.relDir)
    installedPath = os.path.join(installedDir, libName)
    
    # make writable

    cmd = 'chmod +w ' + installedPath
    runCommand(cmd)

    # change runtime paths

    cmd = 'install_name_tool -id "' + libRelPath + '" ' + installedPath

    runCommand(cmd)

########################################################################
# Modify a library dependency paths in a dynamic library

def modifyLibSubPathOsx(parentLibName, subLibName, subLibPath):

    if (options.debug):
        print("===>>> modifyLibSubPathOsx, parentLibName, subLibName, subLibPath: ", \
            parentLibName, ",", subLibName, ",", subLibPath, file=sys.stderr)

    subLibRelPath = os.path.join("@loader_path", subLibName)

    installedDir = os.path.join(options.binDir, options.relDir)
    installedPath = os.path.join(installedDir, parentLibName)
    
    # make writable

    cmd = 'chmod +w ' + installedPath
    runCommand(cmd)

    # change runtime paths

    cmd = 'install_name_tool -change "' + \
        subLibPath + '" "' + subLibRelPath + '" ' + installedPath
    
    runCommand(cmd)

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
            if (options.debug):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, file=sys.stderr)
        exit(1)

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

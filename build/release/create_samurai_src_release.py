#!/usr/bin/env python3

#===========================================================================
#
# Create a source release for samurai
#
# Assumes that a source release has already been created for lrose-core
#
#===========================================================================

from __future__ import print_function
import os
import sys
import shutil
import subprocess
from optparse import OptionParser
import time
from datetime import datetime
from datetime import date
from datetime import timedelta
import glob

def main():

    # globals

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global thisScriptDir
    thisScriptDir = os.path.dirname(__file__)
    if (len(thisScriptDir) == 0):
        thisScriptDir = "."
    os.chdir(thisScriptDir)
    thisScriptDir = os.getcwd()

    global buildDir
    buildDir = os.path.join(thisScriptDir, '..')
    os.chdir(buildDir)
    buildDir = os.getcwd()

    global homeDir
    homeDir = os.environ['HOME']

    global releaseInfoName
    releaseInfoName = "ReleaseInfo.txt"

    global package
    package = "lrose-samurai"

    global corePackage
    corePackage = "lrose-core"

    global releaseDir
    global tmpDir
    global coreDir
    global versionStr
    global coreVersionStr
    global releaseName
    global tarName
    global logPath
    global logFp

    global options

    # parse the command line

    usage = "usage: %prog [options]"
    releaseDirDefault = os.path.join(homeDir, 'releases')
    logDirDefault = '/tmp/create_samurai_src_release/logs'
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=True,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--verbose',
                      dest='verbose', default=False,
                      action="store_true",
                      help='Set verbose debugging on')
    parser.add_option('--releaseDir',
                      dest='releaseTopDir', default=releaseDirDefault,
                      help='Top-level release dir')
    parser.add_option('--tag',
                      dest='tag', default="master",
                      help='Tag for checking out from git')
    parser.add_option('--logDir',
                      dest='logDir', default=logDirDefault,
                      help='Logging dir')
    parser.add_option('--force',
                      dest='force', default=False,
                      action="store_true",
                      help='force, do not request user to check it is OK to proceed')

    (options, args) = parser.parse_args()
    
    if (options.verbose):
        options.debug = True

    # runtime

    now = time.gmtime()
    nowTime = datetime(now.tm_year, now.tm_mon, now.tm_mday,
                       now.tm_hour, now.tm_min, now.tm_sec)
    versionStr = nowTime.strftime("%Y%m%d")

    # set directories

    releaseDir = os.path.join(options.releaseTopDir, package)
    tmpDir = os.path.join(releaseDir, "tmp")
    coreDir = os.path.join(options.releaseTopDir, "lrose-core")

    # compute release name and dir name
    
    releaseName = package + "-" + versionStr + ".src"
    tarName = releaseName + ".tgz"

    # read the core release info

    readCoreReleaseInfoFile()

    # initialize logging

    if (os.path.isdir(options.logDir) == False):
        os.makedirs(options.logDir)
    logPath = os.path.join(options.logDir, "master");
    logFp = open(logPath, "w+")

    if (options.debug):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  package: ", package, file=sys.stderr)
        print("  releaseTopDir: ", options.releaseTopDir, file=sys.stderr)
        print("  releaseDir: ", releaseDir, file=sys.stderr)
        print("  logDir: ", options.logDir, file=sys.stderr)
        print("  tmpDir: ", tmpDir, file=sys.stderr)
        print("  force: ", options.force, file=sys.stderr)
        print("  versionStr: ", versionStr, file=sys.stderr)
        print("  releaseName: ", releaseName, file=sys.stderr)
        print("  tarName: ", tarName, file=sys.stderr)
        print("  corePackage: ", corePackage, file=sys.stderr)
        print("  coreVersionStr: ", coreVersionStr, file=sys.stderr)
        
    # save previous releases
        
    savePrevReleases()

    # create tmp dir

    createTmpDir()

    # get repos from git

    logPath = prepareLogFile("git-checkout");
    gitCheckout()

    # create the release information file
    
    createReleaseInfoFile()

    # create the tar file

    logPath = prepareLogFile("create-tar-file");
    createTarFile()

    # create the brew formula for OSX builds

    #logPath = prepareLogFile("create-brew-formula");
    logPath = prepareLogFile("no-logging");
    createBrewFormula()

    # delete the tmp dir

    shutil.rmtree(tmpDir)

    logFp.close()
    sys.exit(0)

########################################################################
# move previous releases

def savePrevReleases():

    if (os.path.isdir(releaseDir) == False):
        return
    
    os.chdir(releaseDir)
    prevDirPath = os.path.join(releaseDir, 'previous_releases')

    # remove if file instead of dir

    if (os.path.isfile(prevDirPath)):
        os.remove(prevDirPath)

    # ensure dir exists
    
    if (os.path.isdir(prevDirPath) == False):
        os.makedirs(prevDirPath)

    # get old releases

    pattern = package + "-????????*.tgz"
    oldReleases = glob.glob(pattern)

    for name in oldReleases:
        newName = os.path.join(prevDirPath, name)
        if (options.debug):
            print("saving oldRelease: ", name, file=logFp)
            print("to: ", newName, file=logFp)
        os.rename(name, newName)

########################################################################
# create the tmp dir

def createTmpDir():

    # check if exists already

    if (os.path.isdir(tmpDir)):

        if (options.force == False):
            print(("WARNING: you are about to remove all contents in dir: " + tmpDir))
            print("===============================================")
            contents = os.listdir(tmpDir)
            for filename in contents:
                print(("  " + filename))
            print("===============================================")
            answer = "n"
            if (sys.version_info > (3, 0)):
                answer = input("WARNING: do you wish to proceed (y/n)? ")
            else:
                answer = raw_input("WARNING: do you wish to proceed (y/n)? ")
            if (answer != "y"):
                print("  aborting ....")
                sys.exit(1)
                
        # remove it

        shutil.rmtree(tmpDir)

    # make it clean

    if (os.path.isdir(tmpDir) == False):
        os.makedirs(tmpDir)

########################################################################
# check out repos from git

def gitCheckout():

    os.chdir(tmpDir)
    shellCmd("git clone --branch " + options.tag +
             " https://github.com/mmbell/samurai " + releaseName)

########################################################################
# write release information file

def createReleaseInfoFile():

    global releaseInfoName

    # go to core dir
    os.chdir(releaseDir)

    # open info file

    releaseInfoPath = os.path.join(releaseDir, releaseInfoName)
    info = open(releaseInfoPath, 'w')

    # write release info

    info.write("package:" + package + "\n")
    info.write("version:" + versionStr + "\n")
    info.write("release:" + releaseName + "\n")

    # close

    info.close()

    # copy it up into the release dir

    shellCmd("rsync -av " + releaseInfoName + " " + releaseDir)

########################################################################
# read latest release information file for the core

def readCoreReleaseInfoFile():

    global coreVersionStr
    coreVersionStr = "unknown"

    _corePackage = "unknown"
    _coreSrcRelease = "unknown"

    # open info file
    
    coreInfoPath = os.path.join(coreDir, releaseInfoName)
    if (options.debug):
        print("==>> reading core info file: ", coreInfoPath, file=sys.stderr)
        
    info = open(coreInfoPath, 'r')
        
    # read in lines

    lines = info.readlines()
    
    # close

    info.close()

    # decode lines

    if (len(lines) < 1):
        print("ERROR reading info file: ", coreInfoPath, file=sys.stderr)
        print("  No contents", file=sys.stderr)
        sys.exit(1)

    for line in lines:
        line = line.strip()
        toks = line.split(":")
        if (options.verbose):
            print("  line: ", line, file=sys.stderr)
            print("  toks: ", toks, file=sys.stderr)
        if (len(toks) == 2):
            if (toks[0] == "package"):
                _corePackage = toks[1]
            if (toks[0] == "version"):
                coreVersionStr = toks[1]
            if (toks[0] == "release"):
                _coreSrcRelease = toks[1]
        
    if (options.verbose):
        print("==>> done reading info file: ", coreInfoPath, file=sys.stderr)
        print("======>>     coreVersionStr: ", coreVersionStr, file=sys.stderr)
        print("======>>       _corePackage: ", _corePackage, file=sys.stderr)
        print("======>>    _coreSrcRelease: ", _coreSrcRelease, file=sys.stderr)

########################################################################
# create the tar file

def createTarFile():

    # go to tmp dir
    
    os.chdir(tmpDir)

    # create the tar file
    
    shellCmd("tar cvfzh " + tarName + " " + releaseName)

    # move the tar file into the release dir

    os.rename(tarName, os.path.join(releaseDir, tarName))
    
########################################################################
# template for brew formula

formulaBody = """

require 'formula'

class LroseSamurai < Formula

  homepage 'https://github.com/mmbell/samurai'

  url '{0}'
  version '{1}'
  sha256 '{2}'

  depends_on 'hdf5' => 'enable-cxx'
  depends_on 'netcdf' => 'enable-cxx-compat'
  depends_on 'libx11'
  depends_on 'libxext'
  depends_on 'qt5'
  depends_on 'fftw'
  depends_on 'libomp'
  depends_on 'libzip'
  depends_on 'cmake'
  depends_on 'eigen'
  depends_on 'rsync'
  depends_on 'lrose-core'

  def install

    # Build/install samurai
    ENV['LROSE_INSTALL_DIR'] = prefix
    system "cmake", "-DCMAKE_INSTALL_PREFIX=#{{prefix}}", "."
    system "make install"

  end

  def test
    system "#{{bin}}/samurai", "-h"
  end

end
"""

########################################################################
# create the brew formula for OSX builds

def buildSamuraiFormula(tar_url, tar_name, formula_name):

    os.chdir(releaseDir)

    """ build a Homebrew forumula file for lrose-core """	
    dash = tar_name.find('-')
    period = tar_name.find('.', dash)
    version = tar_name[dash+1:period]
    result = subprocess.check_output(("sha256sum", tar_name))
    checksum = result.split()[0].decode('ascii')
    formula = formulaBody.format(tar_url, version, checksum)
    outf = open(formula_name, 'w')
    outf.write(formula)
    outf.close()

########################################################################
# create the brew formula for OSX builds

def createBrewFormula():

    tarUrl = "https://github.com/NCAR/lrose-core/releases/download/" + \
             corePackage + "-" + coreVersionStr + "/" + tarName
    formulaName = package + ".rb"

    buildSamuraiFormula(tarUrl, tarName, formulaName)
    
########################################################################
# prepare log file

def prepareLogFile(logFileName):

    global logFp

    logFp.close()
    logPath = os.path.join(options.logDir, logFileName + ".log");
    if (logPath.find('no-logging') >= 0):
        logFp = sys.stderr
        return logPath
    print("========================= " + logFileName + " =========================", file=sys.stderr)
    if (options.verbose):
        print("====>> Creating log file: " + logPath + " <<==", file=sys.stderr)
    logFp = open(logPath, "w+")
    logFp.write("===========================================\n")
    logFp.write("Log file from script: " + thisScriptName + "\n")

    return logPath

########################################################################
# Run a command in a shell, wait for it to complete

def shellCmd(cmd):

    print("Running cmd:", cmd, file=sys.stderr)
    
    if (logPath.find('no-logging') >= 0):
        cmdToRun = cmd
    else:
        print("Log file is:", logPath, file=sys.stderr)
        print("    ....", file=sys.stderr)
        cmdToRun = cmd + " 1>> " + logPath + " 2>&1"

    try:
        retcode = subprocess.check_call(cmdToRun, shell=True)
        if retcode != 0:
            print("Child exited with code: ", retcode, file=sys.stderr)
            sys.exit(1)
        else:
            if (options.verbose):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, file=sys.stderr)
        sys.exit(1)

    print("    done", file=sys.stderr)
    
########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

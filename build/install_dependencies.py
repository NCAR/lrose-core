#!/usr/bin/env python

#===========================================================================
#
# Install dependencies for LROSE
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
from sys import platform

def main():

    # globals

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global options
    global osType, osVersion

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    prefixDefault = '/usr/local/lrose'

    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--cidd32',
                      dest='cidd', default=False,
                      action="store_true",
                      help='Install 32-bit dependencies for CIDD apps')
    parser.add_option('--osFile',
                      dest='osFile', default="/etc/os-release",
                      help='OS file path')

    (options, args) = parser.parse_args()
    
    # runtime

    now = time.gmtime()
    nowTime = datetime(now.tm_year, now.tm_mon, now.tm_mday,
                       now.tm_hour, now.tm_min, now.tm_sec)
    dateStr = nowTime.strftime("%Y%m%d")
    timeStr = nowTime.strftime("%Y%m%d%H%M%S")
    dateTimeStr = nowTime.strftime("%Y/%m/%d-%H:%M:%S")

    # get the OS type and version
    
    getOsType()

    # let users know what we are doing

    print("****************************************************", file=sys.stderr)
    print("Running", thisScriptName, file=sys.stderr)
    print(" ", dateTimeStr, file=sys.stderr)
    print("  OS file: ", options.osFile, file=sys.stderr)
    print("  Installing dependencies for LROSE", file=sys.stderr)
    print("  OS type: ", osType, file=sys.stderr)
    print("  OS version: ", osVersion, file=sys.stderr)
    print("****************************************************", file=sys.stderr)

    # do the install
    
    #if (os.path.isdir("bin")):
    #    shellCmd("rsync -av bin " + installDir)

    # done
    
    print("****************************************************", file=sys.stderr)
    print("Done", file=sys.stderr)
    print("****************************************************", file=sys.stderr)

    sys.exit(0)

########################################################################
# get the OS type from the /etc/os-release file in linux

def getOsType():

    global osType, osVersion

    # Mac OSX?

    # if (platform == "darwin"):
    #     osType = "OSX"
    #     osVersion = 10
    #     return

    # not linux?

    # if (platform != "linux"):
    #     osType = "unknown"
    #     osVersion = 0
    #     return

    # Centos 6?

    # if (os.path.isfile("/etc/redhat-release")):
    #     rhrelease_file = open("/etc/redhat-release", "rt")
    #     lines = rhrelease_file.readlines()
    #     rhrelease_file.close()
    #     for line in lines:
    #       if (line.find("CentOS release 6") == 0):
    #         osType = "centos"
    #         osVersion = 6
    #         return
          
    _file = open(options.osFile, "rt")
    lines = _file.readlines()
    _file.close()

    osType = "unknown"
    for line in lines:

        line.strip()
        
        if (line.find("NAME") == 0):
          nameline = line.lower()
          if (nameline.find("centos") >= 0):
            osType = "centos"
          elif (nameline.find("fedora") >= 0):
            osType = "fedora"
          elif (nameline.find("debian") >= 0):
            osType = "debian"
          elif (nameline.find("ubuntu") >= 0):
            osType = "ubuntu"
          elif (nameline.find("suse") >= 0):
            osType = "suse"
            
        if (line.find("VERSION_ID") == 0):
          lineParts = line.split('=')
          print("  lineParts: ", lineParts, file=sys.stderr)
          if (lineParts[1].find('"') >= 0):
            subParts = lineParts[1].split('"')
            print("  subParts: ", subParts, file=sys.stderr)
            osVersion = float(subParts[1])
          else:
            osVersion = float(lineParts[1])

          return

########################################################################
# Run a command in a shell, wait for it to complete

def shellCmd(cmd):

    if (options.debug):
        print("running cmd:", cmd, " .....", file=sys.stderr)
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print("Child exited with code: ", retcode, file=sys.stderr)
            sys.exit(1)
        else:
            if (options.verbose):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, file=sys.stderr)
        sys.exit(1)

    if (options.debug):
        print(".... done", file=sys.stderr)
    
########################################################################
# Run - entry point

if __name__ == "__main__":
    main()

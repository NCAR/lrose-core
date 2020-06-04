#!/usr/bin/env python

#===========================================================================
#
# Install dependencies for LROSE
#
#===========================================================================

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

    print >>sys.stderr, "****************************************************"
    print >>sys.stderr, "  Running " + thisScriptName
    print >>sys.stderr, ""
    print >>sys.stderr, "  Installing dependencies for LROSE"
    print >>sys.stderr, "  OS type: " + osType
    print >>sys.stderr, "  OS version: " + osVersion
    print >>sys.stderr, ""
    print >>sys.stderr, "  NCAR, Boulder, CO, USA"
    print >>sys.stderr, ""
    print >>sys.stderr, "  " + dateTimeStr
    print >>sys.stderr, ""
    print >>sys.stderr, "****************************************************"

    # do the install
    
    #if (os.path.isdir("bin")):
    #    shellCmd("rsync -av bin " + installDir)


    #--------------------------------------------------------------------
    # done
    
    print("  **************************************************")
    print("  *** Done installing dependencies ***")
    print("  *** OS type: " + osType
    print("  *** OS version: " + osVersion
    print("  **************************************************")

    sys.exit(0)

########################################################################
# get the OS type

########################################################################
# get the OS type from the /etc/os-release file in linux

def getOSType():

    global osType, osVersion

    # Mac OSX?

    if (platform == "darwin"):
        osType = "OSX"
        osVersion = 10
        return

    # not linux?

    if (platform != "linux"):
        osType = "unknown"
        osVersion = 0
        return

    # Centos 6?

    if (os.path.isfile("/etc/redhat-release")):
        rhrelease_file = open("/etc/redhat-release", "rt")
        lines = rhrelease_file.readlines()
        rhrelease_file.close()
        for line in lines:
          if (line.find("CentOS release 6") == 0):
          osType = "CentOS"
          osVersion = 6
          return
          
    osrelease_file = open("/etc/os-release", "rt")
    lines = osrelease_file.readlines()
    osrelease_file.close()
    osType = "unknown"
    for line in lines:
        if (line.find('PRETTY_NAME') == 0):
          lineParts = line.split('=')
          osParts = lineParts[1].split('"')
          osType = osParts[1]
          return

def getOsType():



    ostype = "x86_64"
    tmpFile = os.path.join("/tmp", "ostype." + timeStr + ".txt")

    shellCmd("uname -a > " + tmpFile)
    f = open(tmpFile, 'r')
    lines = f.readlines()
    f.close()

    if (len(lines) < 1):
        print >>sys.stderr, "ERROR getting OS type"
        print >>sys.stderr, "  'uname -a' call did not succeed"
        sys.exit(1)

    for line in lines:
        line = line.strip()
        if (options.verbose):
            print >>sys.stderr, "  line: ", line
        if (line.find("x86_64") > 0):
            ostype = "x86_64"
        elif (line.find("i686") > 0):
            ostype = "i686"
            
########################################################################
# Run a command in a shell, wait for it to complete

def shellCmd(cmd):

    if (options.debug):
        print >>sys.stderr, "running cmd:", cmd, " ....."
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print >>sys.stderr, "Child exited with code: ", retcode
            sys.exit(1)
        else:
            if (options.verbose):
                print >>sys.stderr, "Child returned code: ", retcode
    except OSError, e:
        print >>sys.stderr, "Execution failed:", e
        sys.exit(1)

    if (options.debug):
        print >>sys.stderr, ".... done"
    
########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

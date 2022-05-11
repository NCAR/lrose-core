#!/usr/bin/env python3

#===========================================================================
#
# Print a json file
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
import json

def main():

    # globals

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global thisScriptDir
    thisScriptDir = os.path.dirname(__file__)
    os.chdir(thisScriptDir)
    thisScriptDir = os.getcwd()
    
    global options

    # parse the command line

    usage = "usage: " + thisScriptName + " [options]"
    homeDir = os.environ['HOME']

    parser = OptionParser(usage)

    parser.add_option('--debug',
                      dest='debug', default=True,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--path',
                      dest='path', default='test.json',
                      help='path to json file')


    (options, args) = parser.parse_args()
    
    # runtime

    now = time.gmtime()
    nowTime = datetime(now.tm_year, now.tm_mon, now.tm_mday,
                       now.tm_hour, now.tm_min, now.tm_sec)
    dateStr = nowTime.strftime("%Y%m%d")

    # debug print

    if (options.debug):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  path: ", options.path, file=sys.stderr)

    # read in the file

    try:
        fp = open(options.path, 'r')
    except IOError as e:
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot open file:", makefilePath, file=sys.stderr)
        print("  dir: ", options.coreDir, file=sys.stderr)
        return valueList

    lines = fp.readlines()
    fp.close()

    srcCount = 0
    binCount = 0
    tgzCount = 0
    rpmCount = 0
    debCount = 0
    rbCount = 0
    dmgCount = 0
    otherCount = 0

    name = ""
    for line in lines:
        line = line.rstrip()
        if (line.find('name') >= 0):
            name = line
        elif (line.find('download_count') >= 0):
            countParts = line.split(':')
            countStr = countParts[1].strip(',')
            countNum = int(countStr)
            print("====================", file=sys.stderr)
            print("name: ", name, file=sys.stderr)
            print("countNum: ", countNum, file=sys.stderr)
            if (name.find('.src.') >= 0):
                print("====>> src <<====", file=sys.stderr)
                srcCount = srcCount + countNum
            elif (name.find('.rpm') >= 0):
                print("====>> rpm <<====", file=sys.stderr)
                rpmCount = rpmCount + countNum
            elif (name.find('.deb') >= 0):
                print("====>> deb <<====", file=sys.stderr)
                debCount = debCount + countNum
            elif (name.find('.dmg') >= 0):
                print("====>> dmg <<====", file=sys.stderr)
                dmgCount = dmgCount + countNum
            elif (name.find('.bin.') >= 0):
                print("====>> bin <<====", file=sys.stderr)
                binCount = binCount + countNum
            elif (name.find('.x86_64.') >= 0):
                print("====>> bin <<====", file=sys.stderr)
                binCount = binCount + countNum
            elif (name.find('.tgz') >= 0):
                print("====>> tgz <<====", file=sys.stderr)
                tgzCount = tgzCount + countNum
            elif (name.find('.rb') >= 0):
                print("====>> rb <<====", file=sys.stderr)
                rbCount = rbCount + countNum
            else:
                otherCount = otherCount + countNum
                print("====>> other <<====", file=sys.stderr)

    totalBinCount = binCount + rpmCount + debCount + dmgCount
    totalSrcCount = srcCount + tgzCount

    print("totalSrcCount: ", totalSrcCount, file=sys.stderr)
    print("totalBinCount: ", totalBinCount, file=sys.stderr)
    print("srcCount: ", srcCount, file=sys.stderr)
    print("binCount: ", binCount, file=sys.stderr)
    print("tgzCount: ", tgzCount, file=sys.stderr)
    print("rpmCount: ", rpmCount, file=sys.stderr)
    print("debCount: ", debCount, file=sys.stderr)
    print("dmgCount: ", dmgCount, file=sys.stderr)
    print("rbCount: ", rbCount, file=sys.stderr)
    print("otherCount: ", otherCount, file=sys.stderr)

    # exit

    sys.exit(0)

########################################################################
# Run a command in a shell, wait for it to complete

def shellCmd(cmd):

    print("Running cmd:", cmd, file=sys.stderr)
    
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

    print("    done", file=sys.stderr)
    
########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

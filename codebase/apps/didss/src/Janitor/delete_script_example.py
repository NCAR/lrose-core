#!/usr/bin/env python

#===========================================================================
#
# Example of delete script for Janitor
#
#===========================================================================

import os
import sys
import time
import datetime as DT
from datetime import timedelta
import pytz
import string
import subprocess
from optparse import OptionParser
from stat import *

def main():

    global options

    today = DT.datetime.now()

    # parse the command line

    parseArgs()

    # debug print

    utc = pytz.utc
    modTime = DT.datetime(1970, 1, 1, 0, 0, 0)
    dataTime = DT.datetime(1970, 1, 1, 0, 0, 0)
    modTime = modTime.fromtimestamp(float(options.modTime), utc)
    dataTime = dataTime.fromtimestamp(float(options.dataTime), utc)

    print >>sys.stderr, "Script: %prog"
    print >>sys.stderr, "  called at time: " + str(today)
    print >>sys.stderr, "  Options in use:"
    print >>sys.stderr, "    debug? ", options.debug
    print >>sys.stderr, "    verbose? ", options.verbose
    print >>sys.stderr, "    dataTime? ", dataTime
    print >>sys.stderr, "    modTime? ", modTime
    print >>sys.stderr, "    fileDir: ", options.fileDir
    print >>sys.stderr, "    filePath: ", options.filePath
    print >>sys.stderr, "    fileName: ", options.fileName
    print >>sys.stderr, ""
    print >>sys.stderr, "Done"

    sys.exit(0)

########################################################################
# Parse the command line

def parseArgs():
    
    global options

    defaultTargetDir = os.getenv("DATA_DIR") + "/raw/doe"
    defaultTmpDir = os.getenv("DATA_DIR") + "/tmp/raw/doe"
    defaultSubStrings = "RAW"

    # parse the command line

    usage = "usage: %prog [options]"
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug',
                      default='False',
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--verbose',
                      dest='verbose',
                      default='False',
                      action="store_true",
                      help='Set verbose debugging on')
    parser.add_option('--data_time',
                      dest='dataTime',
                      default=-1,
                      help='Set data time')
    parser.add_option('--mod_time',
                      dest='modTime',
                      default=-1,
                      help='Set file mod time')
    parser.add_option('--dir',
                      dest='fileDir',
                      default='unknown_dir_path',
                      help='Path of directory in which file was stored')
    parser.add_option('--path',
                      dest='filePath',
                      default='unknown_file_path',
                      help='Path of file that was deleted')
    parser.add_option('--name',
                      dest='fileName',
                      default='unknown_file_name',
                      help='Name of file that was deleted')
    parser.add_option('--ext',
                      dest='fileExt',
                      default='unknown_file_ext',
                      help='Extension of file that was deleted')

    (options, args) = parser.parse_args()

    if (options.verbose == True):
        options.debug = True
        
########################################################################
# Run a command in a shell, wait for it to complete

def runCommand(cmd):

    if (options.debug == True):
        print >>sys.stderr, "running cmd:",cmd
    
    try:
        retcode = subprocess.call(cmd, shell=True)
        if retcode < 0:
            print >>sys.stderr, "Child was terminated by signal: ", -retcode
        else:
            if (options.debug == True):
                print >>sys.stderr, "Child returned code: ", retcode
    except OSError, e:
        print >>sys.stderr, "Execution failed:", e

########################################################################
# kick off main method

if __name__ == "__main__":

   main()

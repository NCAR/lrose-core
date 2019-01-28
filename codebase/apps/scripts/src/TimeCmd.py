#!/usr/bin/env python2

#===========================================================================
#
# COmpute the elapsed time for a command
#
#===========================================================================

import os
import sys
import subprocess
from optparse import OptionParser
import math
import datetime

def main():

#   globals

    global options

# parse the command line

    usage = "usage: %prog [options]"
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--command',
                      dest='command',
                      default='du -sh *',
                      help='Command to time')
    
    (options, args) = parser.parse_args()
    
    if (options.debug == True):
        print >>sys.stderr, "Running TimeCmd"
        print >>sys.stderr, "  command: ", options.command
        print >>sys.stderr, "This script determines the elapsed time for the command"

    # get start time

    startTime = datetime.datetime.now()

    # run the command

    runCommand(options.command)

    # get end time

    endTime = datetime.datetime.now()

    # get the time difference

    elapsedTime = endTime - startTime

    # print results
    
    print >>sys.stdout, "==============================="
    print >>sys.stdout, "Start   time: ", startTime
    print >>sys.stdout, "End     time: ", endTime
    print >>sys.stdout, "Elapsed time: ", elapsedTime
    print >>sys.stdout, "==============================="

    # done

    sys.exit(0)
    
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
# Run - entry point

if __name__ == "__main__":
   main()


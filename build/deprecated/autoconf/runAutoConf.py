#!/usr/bin/env python3

#===========================================================================
#
# Run autoconf etc on a dir
#
#===========================================================================

from __future__ import print_function
from sys import platform
import os
import sys
import subprocess
import shutil
from optparse import OptionParser
from datetime import datetime

def main():

    global options
    global subdirList
    global makefileCreateList

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    # We will be executing some sibling scripts. Get our path so that
    # the sibling scripts from the same path can be executed explicitly.
    global thisScriptDir
    thisScriptDir = os.path.abspath(os.path.dirname(__file__))

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
                      help='Directory containing configure script')
    parser.add_option('--shared',
                      dest='shared', default='False',
                      action="store_true",
                      help='Create shared lib objects')

    (options, args) = parser.parse_args()

    if (options.verbose == True):
        options.debug = True
    
    if (options.debug == True):
        print("Running:", thisScriptName, file=sys.stderr)
        print("    Dir:", options.dir, file=sys.stderr)

    # go to the dir

    os.chdir(options.dir)

    # run autoconffix the configure

    runAutoConf()
            
    sys.exit(0)

########################################################################
# run autoconf commands

def runAutoConf():

    if (options.shared == True):
        cmd = "libtoolize"
        runCommand(cmd)

    cmd = "aclocal"
    runCommand(cmd)

    cmd = "autoheader"
    runCommand(cmd)

    cmd = "automake --add-missing"
    runCommand(cmd)

    cmd = "autoconf"
    runCommand(cmd)

    #cmd = "autoreconf"
    #runCommand(cmd)

    cmd = os.path.join(thisScriptDir, "fixConfigure.py") + \
          " --dir %s" % options.dir
    runCommand(cmd)

########################################################################
# Run a command in a shell, wait for it to complete

def runCommand(cmd):

    if (options.debug == True):
        print("running cmd:", cmd, " .....", file=sys.stderr)
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print("Child exited with code: ", retcode, file=sys.stderr)
            sys.exit(1)
        else:
            if (options.verbose == True):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, file=sys.stderr)
        sys.exit(1)

    if (options.debug == True):
        print(".... done", file=sys.stderr)
    
########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

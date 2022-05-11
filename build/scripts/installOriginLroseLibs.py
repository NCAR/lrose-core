#!/usr/bin/env python3

# ========================================================================== #
#
# Copy the libraries compiles in LROSE build.
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

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    # parse the command line

    usage = "usage: %prog [options]: prints catalog to stdout"

    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--binDir',
                      dest='binDir', default='.',
                      help='Path to binaries')
    parser.add_option('--libDir',
                      dest='libDir', default='.',
                      help='Path to lrose libraries')
    parser.add_option('--relDir',
                      dest='relDir', default='runtime_libs',
                      help='Path of runtime libs relative to binaries')
    parser.add_option('--verbose',
                      dest='verbose', default=False,
                      action="store_true",
                      help='Set verbose debugging on')

    (options, args) = parser.parse_args()

    if (options.verbose):
        options.debug = True

    if (options.debug == True):
        print("  Running " + thisScriptName, file=sys.stderr)
        print("    platform: ", platform, file=sys.stderr)
        print("  Options:", file=sys.stderr)
        print("    debug: ", options.debug, file=sys.stderr)
        print("    verbose: ", options.verbose, file=sys.stderr)
        print("    binDir: ", options.binDir, file=sys.stderr)
        print("    libDir: ", options.libDir, file=sys.stderr)
        print("    relDir: ", options.relDir, file=sys.stderr)

    # compute path for installed libs

    runtimeLibDir = os.path.join(options.binDir, options.relDir)

    # create the dir

    cmd = "mkdir -p " + runtimeLibDir
    runCommand(cmd)

    # rsync the compiled lrose libs into the runtime dir

    cmd = "rsync -av " + options.libDir + "/* " + runtimeLibDir
    runCommand(cmd)

    # done

    sys.exit(0)

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

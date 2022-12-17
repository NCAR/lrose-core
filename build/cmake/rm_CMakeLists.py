#!/usr/bin/env python3

#===========================================================================
#
# Remove CMakeLists.txt files and other artifacts of createCMakeLists.py
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
    global subdirList
    global libList
    global makefileCreateList

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    # We will be executing some sibling scripts. Get our path so that
    # the sibling scripts from the same path can be executed explicitly.
    global thisScriptDir
    thisScriptDir = os.path.dirname(os.path.abspath(__file__))

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    coreDirDefault = os.path.join(thisScriptDir, "../..")
    parser = OptionParser(usage)
    parser.add_option('--topDir',
                      dest='topDir', default=coreDirDefault,
                      help='Path of top level directory - we remove files from here down')
    (options, args) = parser.parse_args()

    print("Running %s:" % thisScriptName, file=sys.stdout)
    print("  top dir: ", options.topDir, file=sys.stdout)
    print("", file=sys.stdout)

    if (sys.version_info > (3, 0)):
        response = input('  Proceed ............. (yes/no)? ')
    else:
        response = raw_input('  Proceed ............. (yes/no)? ')

    if (len(response) < 3):
        sys.exit(0)

    response = response.lower()
    if (response.find('yes') == 0):
        os.chdir(options.topDir)
        cmd = "find . \'(\' -name CMakeLists.txt \')\' -user $USER -type f -print -exec /bin/rm \'{}\' \;"
        runCommand(cmd)
        cmd = "find . \'(\' -name CMakeCache.txt \')\' -user $USER -type f -print -exec /bin/rm \'{}\' \;"
        runCommand(cmd)
        cmd = "find . \'(\' -name CMakeFiles \')\' -user $USER -type d -print -exec /bin/rm -rf \'{}\' \;"
        runCommand(cmd)

    sys.exit(0)

########################################################################
# Run a command in a shell, wait for it to complete

def runCommand(cmd):

    print("running cmd:",cmd, file=sys.stderr)
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print("Child exited with code: ", retcode, file=sys.stderr)
            exit(1)
        else:
            print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, retcode, file=sys.stderr)
        exit(1)

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

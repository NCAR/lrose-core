#!/usr/bin/env python3

# ========================================================================== #
#
# Replace global strings in files.
#
# ========================================================================== #

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
    global replaceDict

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    # parse the command line

    usage = "usage: %prog [options]: replaces strings in file"

    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--path',
                      dest='path', default='./junk',
                      help='Path for file to be modified')

    (options, args) = parser.parse_args()

    if (options.debug):
        print("  Running " + thisScriptName, file=sys.stderr)
        print("  Options:", file=sys.stderr)
        print("    debug: ", options.debug, file=sys.stderr)
        print("    path: ", options.path, file=sys.stderr)

    # create the replacement dictionary

    createReplaceDict()

    # Opening file in read only 
    # mode using the open() function 

    with open(options.path, 'r') as file: 
  
        # Reading the content of the file 
        # using the read() function and storing 
        # them in a new variable 
        data = file.read() 
  
        # Searching and replacing the text 
        # using the replace() function
        for x, y in replaceDict.items():
            data = data.replace(x, y) 
        
    # Opening our text file in write only 
    # mode to write the replaced content 
    with open(options.path, 'w') as file: 
            
        # Writing the replaced data in our 
        # text file 
        file.write(data) 
  
    # done

    sys.exit(0)

########################################################################
# set up replace dictionary

def createReplaceDict():

    global replaceDict
    replaceDict = {}

    # replaceDict["gd.debug1_flag"] = "_params.debug1_flag"
    # replaceDict["gd.debug2_flag"] = "_params.debug2_flag"
    # replaceDict["gd.debug"] = "_params.debug"

    replaceDict["gd.idle_reset_seconds"] = "_params.idle_reset_seconds"

########################################################################
# Run - entry point

if __name__ == "__main__":
    main()

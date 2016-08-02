#!/usr/bin/env python

#===========================================================================
#
# Create makefile.am for top dir
#
#===========================================================================

import os
import sys
import subprocess
from optparse import OptionParser
from datetime import datetime

def main():

    global options
    global subdirList
    global makefileName

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

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
                      help='Path of directory')

    (options, args) = parser.parse_args()

    if (options.verbose == True):
        options.debug = True
    
    if (options.debug == True):
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  Dir: ", options.dir

    # go to the dir

    os.chdir(options.dir)

    # write out makefile.am
            
    writeMakefileAm()

    sys.exit(0)

########################################################################
# Write out makefile.am

def writeMakefileAm():

    fo = open("makefile.am", "w")

    fo.write("###############################################\n")
    fo.write("#\n")
    fo.write("# makefile template for automake\n")
    fo.write("#\n")
    fo.write("# dir: %s\n" % options.dir)
    fo.write("#\n")
    fo.write("# written by script %s\n" % thisScriptName)
    fo.write("#\n")
    fo.write("# created %s\n" % datetime.now())
    fo.write("#\n")
    fo.write("###############################################\n")
    fo.write("\n")

    fo.write("# subdirectories\n")
    fo.write("\n")
    fo.write("SUBDIRS = \\\n")
    fo.write("\tlibs \\\n")
    fo.write("\tapps\n")
    fo.write("\n")

    fo.close

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

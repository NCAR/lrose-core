#!/usr/bin/env python

#===========================================================================
#
# Create makefile.am for directory recursion
#
#===========================================================================

from __future__ import print_function
import os
import sys
import subprocess
import shutil
from optparse import OptionParser
from datetime import datetime

def main():

    global options
    global subdirList

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
    parser.add_option('--template',
                      dest='template', default="unknown",
                      help='Path of makefile template')

    (options, args) = parser.parse_args()

    if (options.verbose == True):
        options.debug = True
    
    if (options.debug == True):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  Dir: ", options.dir, file=sys.stderr)
        print("  Makefile template: ", options.template, file=sys.stderr)

    # go to the dir

    os.chdir(options.dir)

    # load list of subdirs

    subdirList = []
    getSubdirList()

    if (options.debug == True):
        print("=======================", file=sys.stderr)
        print("subdirList:", file=sys.stderr)
        for subdir in subdirList:
            print("subdir: %s" % (subdir), file=sys.stderr)
        print("=======================", file=sys.stderr)

    # write out makefile.am
            
    writeMakefileAm()

    sys.exit(0)

########################################################################
# get string value based on search key
# the string may span multiple lines
#
# Example of keys: SRCS, SUB_DIRS, MODULE_NAME, TARGET_FILE
#
# value is returned

def getValueListForKey(path, key):

    valueList = []

    try:
        fp = open(path, 'r')
    except IOError as e:
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot open file:", path, file=sys.stderr)
        print("  dir: ", options.dir, file=sys.stderr)
        return valueList

    lines = fp.readlines()
    fp.close()

    foundKey = False
    multiLine = ""
    for line in lines:
        if (line.find(key) >= 0):
            foundKey = True
            multiLine = multiLine + line
            if (line.find("\\") < 0):
                break;
        elif (foundKey == True):
            if (line[0] == '#'):
                break
            if (len(line) < 2):
                break
            multiLine = multiLine + line;
            if (line.find("\\") < 0):
                break;

    if (foundKey == False):
        return valueList

    multiLine = multiLine.replace(key, " ")
    multiLine = multiLine.replace("=", " ")
    multiLine = multiLine.replace("\t", " ")
    multiLine = multiLine.replace("\\", " ")
    multiLine = multiLine.replace("\r", " ")
    multiLine = multiLine.replace("\n", " ")

    toks = multiLine.split(' ')
    for tok in toks:
        if (len(tok) > 0):
            valueList.append(tok)

    return valueList

########################################################################
# load list of sub directories

def getSubdirList():
                    
    global subdirList
    subdirList = []
    
    # search for SUB_DIRS key in makefile

    subNameList = getValueListForKey(options.template, "SUB_DIRS")

    if (len(subNameList) < 1):
        print("ERROR - ", thisScriptName, file=sys.stderr)
        print("  Cannot find SUB_DIRS in ", options.template, file=sys.stderr)
        print("  dir: ", options.dir, file=sys.stderr)
        exit(1)

    for subName in subNameList:
        if (os.path.isdir(subName) == True):
            subdirList.append(subName)

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

    if (len(subdirList) > 0):
        fo.write("# subdirectories\n")
        fo.write("\n")
        fo.write("SUBDIRS = \\\n")
        for index, subdir in enumerate(subdirList):
            fo.write("\t%s" % subdir)
            if (index == len(subdirList) - 1):
                fo.write("\n")
            else:
                fo.write(" \\\n")
        fo.write("\n")

    fo.close

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()

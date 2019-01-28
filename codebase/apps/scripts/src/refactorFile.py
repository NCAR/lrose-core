#!/usr/bin/env python2

#===========================================================================
#
# Refactor a file using strings specified in a list file
#
# Each string pair specifies:
#  (a) the source string
#  (b) the target string
# and uses <> delimiters.
#
# For example, using '|' as the delimiter
#
#    _calib.wavelengthCm|_calib.wavelength_cm
#    _calib.beamWidthDegH|_calib.beamWidth_deg_h
#    _calib.beamWidthDegV|_calib.beamWidth_deg_v
#    _calib.antGainDbH|_calib.gain_ant_db_h
#    _calib.antGainDbV|_calib.gain_ant_db_v
#    _calib.pulseWidthUs|_calib.pulse_width_us
#
#===========================================================================

import os
import sys
import subprocess
from optparse import OptionParser

class RefactorPair(object):

    def __init__(self, sourceStr, targetStr):
        self.sourceStr = sourceStr
        self.targetStr = targetStr

    def printArgs(self):
        print "source, target: ", self.sourceStr, ", ", self.targetStr

def main():

    global options
    global debug
    global refactorPairs

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    dataDir = os.path.join(homeDir, 'projDir/data')
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default='False',
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--sourceFile',
                      dest='sourceFile', default='/tmp/source',
                      help='File to be converted')
    parser.add_option('--targetFile',
                      dest='targetFile', default='/tmp/target',
                      help='File to be written')
    parser.add_option('--pairsFile',
                      dest='pairsFile', default='/tmp/pairs',
                      help='Path to file containing string pairs for conversion')
    parser.add_option('--delimiter',
                      dest='delimiter', default='|',
                      help='String to delimit pairs')

    (options, args) = parser.parse_args()

    if (options.debug == True):
        print >>sys.stderr, "Running refactorFile:"
        print >>sys.stderr, "  sourceFile: ", options.sourceFile
        print >>sys.stderr, "  targetFile: ", options.targetFile
        print >>sys.stderr, "  pairsFile: ", options.pairsFile

    # read in string pairs list

    readRefactorPairsList()
    for pair in refactorPairs:
        if (options.debug == True):
            print >>sys.stderr, "Applying refactor pair:"
            print >>sys.stderr, "  sourceStr: '" + pair.sourceStr + "'"
            print >>sys.stderr, "  targetStr: '" + pair.targetStr + "'"

    # perform the refactoring

    refactor(options.sourceFile, options.targetFile)

    sys.exit(0)

########################################################################
# Read in the refactor pairs list

def readRefactorPairsList():

    global refactorPairs
    refactorPairs = []
 
    fp = open(options.pairsFile, 'r')
    text = fp.read()
    lines = text.splitlines()
    for line in lines:
        if (options.debug == True):
            print >>sys.stderr, "Found pairs line: ", line
        pairList = line.split(options.delimiter)
        if (len(pairList) == 2):
            source = pairList[0]
            target = pairList[1]
            newPair = RefactorPair(source, target)
            refactorPairs.append(newPair)

########################################################################
# perform the refactoring

def refactor(sourceFile, targetFile):

    fp = open(sourceFile, 'r')
    text = fp.read()
    lines = text.splitlines()

    for line in lines:
        for pair in refactorPairs:
            line = line.replace(pair.sourceStr, pair.targetStr)
        print line

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

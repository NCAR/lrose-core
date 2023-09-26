#!/usr/bin/env python3

#=====================================================================
#
# Compute sweep times given PRF, number of samples and number of dwells
#
#=====================================================================

import os
import sys
import time
import datetime
from datetime import timedelta

import string
import subprocess
from optparse import OptionParser

from sys import stdin
from subprocess import call

def main():
    
    global options

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    # parse the command line

    parseArgs()

    prt1 = 1.0 / float(options.prf)
    prt2 = prt1
    prtMean = prt1

    if (options.prf2 != 0):
        prt2 = 1.0 / float(options.prf2)
        prtMean = (prt1 + prt2) / 2.0

    wavelengthM = float(options.wavelengthCm) / 100.0
    vmax = wavelengthM / (4.0 * prt1)
    rmax = (3.0e8 * prt1) / 2000.0
    M = 0.0
    
    if (options.prf2 != 0):
        M = prt1 / (prt2 - prt1)
        vmax = (wavelengthM * M) / (4.0 * prt1)

    print("============================");
    print("prf: ", options.prf);
    print("prf2: ", options.prf2);
    print("prt1: ", prt1);
    print("prt2: ", prt2);
    print("M: ", M);
    print("wavelengthM: ", wavelengthM);
    print("vmax: ", vmax);
    print("rmax: ", rmax);

    for nDwells in (360.0, 720.0):
        for nSamples in (16, 24, 32, 64):
            sweepSecs = computeSweepSecs(prtMean, nSamples, nDwells)
            print("nDwells: ", nDwells, ", nSamples: ", nSamples, ", sweepSecs: ", "%.1f" % sweepSecs)

    print("============================");

    sys.exit(0)

########################################################################
# Compute sweep time

def computeSweepSecs(prtMean, nSamples, nDwells):

    nTotal = nSamples * nDwells
    sweepSecs = prtMean * nSamples * nDwells

    return sweepSecs
        
########################################################################
# Parse the command line

def parseArgs():
    
    global options

    # parse the command line

    usage = "usage: %prog [options]"
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--verbose',
                      dest='verbose', default=False,
                      action="store_true",
                      help='Set verbose debugging on')
    parser.add_option('--prf',
                      dest='prf',
                      default=1200,
                      help='PRF')
    parser.add_option('--prf2',
                      dest='prf2',
                      default=0,
                      help='PRF2')
    parser.add_option('--wavelengthCm',
                      dest='wavelengthCm',
                      default=10.64,
                      help='wavelengthCm')

    (options, args) = parser.parse_args()

    if (options.verbose):
        options.debug = True
        
    if (options.debug):
        print("Options:", file=sys.stderr)
        print("  debug? ", options.debug, file=sys.stderr)
        print("  prf? ", options.prf, file=sys.stderr)
        if (options.prf2 != 0):
            print("  prf2? ", options.prf2, file=sys.stderr)

########################################################################
# Run a command in a shell, wait for it to complete

def runCommand(cmd):

    if (options.debug):
        print("running cmd: ", cmd, file=sys.stderr)
    
    try:
        retcode = subprocess.call(cmd, shell=True)
        if retcode < 0:
            print("Child was terminated by signal: ", -retcode, file=sys.stderr)
        else:
            if (options.debug):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print >>sys.stderr, "Execution failed:", e

########################################################################
# kick off main method

if __name__ == "__main__":

   main()

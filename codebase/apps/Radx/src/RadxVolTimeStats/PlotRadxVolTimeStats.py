#!/usr/bin/env python3


#===========================================================================
#
# Plot results from RadxVolTimeStats
#
#===========================================================================

import os
import sys

import numpy as np
import scipy.stats as stats
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
from optparse import OptionParser
import subprocess

def main():

#   globals

    global options
    global scanName, meanAge, colHeaders, colData

# parse the command line

    usage = "usage: %prog [options]"
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--file',
                      dest='file',
                      default='CAS2_timing.txt',
                      help='File path for timing data')
    parser.add_option('--title',
                      dest='title',
                      default='RADAR SCAN TIMING',
                      help='Title for plot')
    parser.add_option('--width',
                      dest='figWidthMm',
                      default=200,
                      help='Width of figure in mm')
    parser.add_option('--height',
                      dest='figHeightMm',
                      default=150,
                      help='Height of figure in mm')
    (options, args) = parser.parse_args()
    
    if (options.debug):
        print("Running %prog", file=sys.stderr)
        print("  data file: ", options.file, file=sys.stderr)

    # read in headers

    iret = readColumnHeaders(options.file)
    if (iret != 0):
        sys.exit(1)

    # read in data

    iret = readInputData(options.file)
    if (iret != 0):
        sys.exit(1)
        
    # render the plot
    
    doPlot()

    sys.exit(0)
    
########################################################################
# Read columm headers for the data

def readColumnHeaders(filePath):

    global scanName, meanAge, colHeaders
    colHeaders = []

    fp = open(filePath, 'r')
    lines = fp.readlines()
    fp.close()
    gotHeaders = False

    for line in lines:

        # check line starts with #

        commentIndex = line.find("#")
        if (commentIndex < 0):
            continue
            
        # skip "#########....."
        if (line.find("##############") >= 0):
            continue

        # scan name
        if (line.find("scanName") > 0):
            parts = line.strip().split()
            scanName = parts[-1]
            if (options.debug):
                print("  scanName: ", scanName, file=sys.stderr)
            continue

        # scan name
        if (line.find("meanAge") > 0):
            parts = line.strip().split()
            meanAge = parts[-1]
            if (options.debug):
                print("  meanAge: ", meanAge, file=sys.stderr)
            continue

        # col headers?
        if (line.find("binNum") > 0):
            colHeaders = line.lstrip("# ").rstrip("\n").split()
            gotHeaders = True
            if (options.debug):
                print("colHeaders: ", colHeaders, file=sys.stderr)

    if (gotHeaders == False):
        print("ERROR - readColumnHeaders", file=sys.stderr)
        print("  File: ", filePath, file=sys.stderr)
        print("  Cannot find column headers", file=sys.stderr)
        return -1
    
    return 0

########################################################################
# Read in the data

def readInputData(filePath):

    global colData
    colData = {}
    for index, var in enumerate(colHeaders, start=0):
        colData[var] = []

    # open file

    fp = open(filePath, 'r')
    lines = fp.readlines()
    fp.close()

    # read in a line at a time, set colData
    for line in lines:
        
        commentIndex = line.find("#")
        if (commentIndex >= 0):
            continue
            
        # data
        
        data = line.strip().split()
        if (len(data) != len(colHeaders)):
            if (options.debug):
                print("skipping line: ", line, file=sys.stderr)
            continue;

        for index, var in enumerate(colHeaders, start=0):
            colData[var].append(float(data[index]))


    if (options.debug):
        print("colData: ", colData, file=sys.stderr)

    return 0

########################################################################
# Plot

def doPlot():

    binPos = np.array(colData["binPos"]).astype(np.double)
    binFreq = np.array(colData["binFreq"]).astype(np.double)
    cumFreq = np.array(colData["cumFreq"]).astype(np.double)
    revFreq = np.array(colData["revFreq"]).astype(np.double)

    widthIn = float(options.figWidthMm) / 25.4
    htIn = float(options.figHeightMm) / 25.4
    
    fig1 = plt.figure(1, (widthIn, htIn))
    title = (options.title + ' for Scan: ' + scanName)
    fig1.suptitle(title, fontsize=16)
    ax1 = fig1.add_subplot(1,1,1,xmargin=0.0)
    #ax2 = fig1.add_subplot(2,1,2,xmargin=0.0)

    ax1.set_xlim([0.0, 1.0])
    ax1.set_ylim([0.0, 1.0])

    # plot the frequency

    #ax1.plot(binPos, binFreq, \
    #         linewidth=1, label = 'Freq', color = 'blue')

    ax1.plot(binPos, cumFreq, \
             linewidth=1, label = 'CumFreq', color = 'red')

    ax1.plot(binPos, revFreq, \
             linewidth=1, label = 'RevFreq', color = 'blue')

    ax1.set_xlabel('Freq')
    ax1.set_ylabel('Age fraction')
    ax1.set_title('Mean age: ' + meanAge, fontsize=14)
    ax1.grid(True)

    legend1 = ax1.legend(loc='upper left', ncol=2)
    for label in legend1.get_texts():
        label.set_fontsize('x-small')

   # show

    plt.show()

########################################################################
# Annotate a value

def annotVal(ax1, ax2, val, pdf, cdf, label, plen,
             toffx, linecol, textcol,
             horizAlign, vertAlign):

    pval = pdf(val)
    ax1.plot([val, val], [pval - plen, pval + plen], color=linecol, linewidth=2)
    ax1.annotate(label + '=' + '{:.3f}'.format(val),
                 xy=(val, pval + toffx),
                 xytext=(val + toffx, pval),
                 color=textcol,
                 horizontalalignment=horizAlign,
                 verticalalignment=vertAlign)

    cval = cdf(val)
    clen = 0.03
    ax2.plot([val, val], [cval - clen, cval + clen], color=linecol, linewidth=2)
    ax2.annotate(label + '=' + '{:.3f}'.format(val),
                 xy=(val, cval + toffx),
                 xytext=(val + toffx, cval),
                 color=textcol,
                 horizontalalignment=horizAlign,
                 verticalalignment=vertAlign)

########################################################################
# Run a command in a shell, wait for it to complete

def runCommand(cmd):

    if (options.debug):
        print("running cmd:",cmd, file=sys.stderr)
    
    try:
        retcode = subprocess.call(cmd, shell=True)
        if retcode < 0:
            print("Child was terminated by signal: ", -retcode, file=sys.stderr)
        else:
            if (options.debug):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, file=sys.stderr)

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()


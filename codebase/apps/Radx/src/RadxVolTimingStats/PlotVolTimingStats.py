#!/usr/bin/env python3


#===========================================================================
#
# Plot results from RadxVolTimingStats
#
#===========================================================================

import os
import sys

import numpy as np
#import scipy.stats as stats
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
from optparse import OptionParser
import subprocess

def main():

#   globals

    global appName
    appName = os.path.basename(__file__)

    global options
    global scanName, colHeaders, colData
    global heights, elevs, meanAgeFwd, meanAgeRev, volDuration

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
                      default='Volume Age Stats',
                      help='Title for plot')
    parser.add_option('--width',
                      dest='figWidthMm',
                      default=200,
                      help='Width of figure in mm')
    parser.add_option('--height',
                      dest='figHeightMm',
                      default=125,
                      help='Height of figure in mm')
    (options, args) = parser.parse_args()
    
    if (options.debug):
        print("Running: ", appName, file=sys.stderr)
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

    global scanName, heights, elevs, meanAgeFwd, meanAgeRev, volDuration, colHeaders
    colHeaders = []
    heights = []
    meanAgeFwd = []
    meanAgeRev = []

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
            continue

        # volume duration
        if (line.find("duration") > 0):
            parts = line.strip().split()
            volDuration = parts[-1]
            continue

        # elevations
        if (line.find("elevs") > 0):
            parts = line.strip().split()
            elevs = parts[-1]
            continue

        # analysis heights
        if (line.find("heights") > 0):
            parts = line.strip().split()
            heightList = parts[-1]
            heights = heightList.split(',')
            continue

        # mean age - by height

        if (len(heights) > 0):
            for ht in heights:

                fwdLabel = "meanAgeFwd[" + ht + "]"
                if (line.find(fwdLabel) > 0):
                    parts = line.strip().split()
                    ageFwd = parts[-1]
                    meanAgeFwd.append(ageFwd)
                    continue

                revLabel = "meanAgeRev[" + ht + "]"
                if (line.find(revLabel) > 0):
                    parts = line.strip().split()
                    ageRev = parts[-1]
                    meanAgeRev.append(ageRev)
                    continue
                    
        # col headers?
        if (line.find("binNum") > 0):
            colHeaders = line.lstrip("# ").rstrip("\n").split()
            gotHeaders = True

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

    widthIn = float(options.figWidthMm) / 25.4
    htIn = float(options.figHeightMm) / 25.4
    
    fig1 = plt.figure(1, (widthIn, htIn))
    title = (options.title + ' for scan: ' + scanName + ', vol duration: ' + volDuration + ' sec')
    fig1.suptitle(title, fontsize=11)
    ax1 = fig1.add_subplot(1,1,1,xmargin=0.0)
    # ax2 = ax1.twinx() # instantiate a second axes that shares the same x-axis

    ax1.set_xlim([0.0, 1.0])
    ax1.set_ylim([0.0, 1.0])
    # ax2.set_ylim([0.0, 0.2])

    # plot the frequency, for each ht limit

    linestyles = ['solid', 'dashed', 'dashdot', 'dotted']

    count = 0
    dash = 10
    space = 0
    for ht in heights:

        nameFwd = "cumFreqFwd[" + ht + "]"
        nameRev = "cumFreqRev[" + ht + "]"

        if (len(ht) == 1):
            ht = ' ' + ht

        cumFreqFwd = np.array(colData[nameFwd]).astype(np.double)
        cumFreqRev = np.array(colData[nameRev]).astype(np.double)

        meanAgeFwdFloat = float(meanAgeFwd[count])
        meanAgeRevFloat = float(meanAgeRev[count])

        meanAgeFwdSecs = int(meanAgeFwdFloat * float(volDuration) + 0.5)
        meanAgeRevSecs = int(meanAgeRevFloat * float(volDuration) + 0.5)

        labelFwd = "Fwd " + ht + "km meanAge: " + meanAgeFwd[count] + " = " + str(meanAgeFwdSecs) + " s"
        labelRev = "Rev " + ht + "km meanAge: " + meanAgeRev[count] + " = " + str(meanAgeRevSecs) + " s"

        if (count < 4):
            ax1.plot(binPos, cumFreqFwd, \
                     linewidth=1, linestyle = linestyles[count], label = labelFwd, color = 'red')
            ax1.plot(binPos, cumFreqRev, \
                     linewidth=1, linestyle = linestyles[count], label = labelRev, color = 'blue')
        else:
            ax1.plot(binPos, cumFreqFwd, \
                     linewidth=1, dashes = [dash, space], label = labelFwd, color = 'red')
            ax1.plot(binPos, cumFreqRev, \
                     linewidth=1, dashes = [dash, space], label = labelRev, color = 'blue')

        count = count + 1
        dash = dash - 2
        space = space + 2

    ax1.set_ylabel('Fraction of volume')
    ax1.set_xlabel('Normalized age at end of volume - cumulative')
    ax1.set_title('Elevs: ' + elevs, fontsize=8)
    ax1.grid(True)
    
    legend1 = ax1.legend(loc='upper left', ncol=1, framealpha=0.5, fancybox=True)
    for label in legend1.get_texts():
        label.set_fontsize('x-small')

    homeDir = os.environ['HOME']
    downloadsDir = os.path.join(homeDir, 'Downloads')
    savePath = os.path.join(downloadsDir, "vol_timing_stats." + scanName + ".png")
    fig1.savefig(savePath)

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


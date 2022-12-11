#!/usr/bin/env python3


#===========================================================================
#
# Plot range-height results from RadxVolTimingStats
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
import math

def main():

#   globals

    global appName
    appName = os.path.basename(__file__)

    global options
    global scanName, elevList, elevs, colHeaders, colData

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
    parser.add_option('--file',
                      dest='file',
                      default='CAN-VOL_height_table.txt',
                      help='File path for timing data')
    parser.add_option('--title',
                      dest='title',
                      default='Range-Height Plot',
                      help='Title for plot')
    parser.add_option('--width',
                      dest='figWidthMm',
                      default=200,
                      help='Width of figure in mm')
    parser.add_option('--height',
                      dest='figHeightMm',
                      default=125,
                      help='Height of figure in mm')
    parser.add_option('--maxHtKm',
                      dest='maxHtKm',
                      default=20,
                      help='Max height to be plotted in km')
    (options, args) = parser.parse_args()
    
    if (options.debug):
        print("Running ", appName, file=sys.stderr)
        print("  data file: ", options.file, file=sys.stderr)

    # read in headers

    iret = readColumnHeaders(options.file)
    if (iret != 0):
        sys.exit(1)

    if (options.debug):
        print("nGates: ", nGates, file=sys.stderr)
        print("maxRangeKm: ", maxRangeKm, file=sys.stderr)
        print("beamWidth: ", beamWidth, file=sys.stderr)
        print("elevs: ", elevs, file=sys.stderr)
        print("colHeaders: ", colHeaders, file=sys.stderr)

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

    global scanName, nGates, elevList, elevs, beamWidth, maxRangeKm, colHeaders
    colHeaders = []
    elevs = []

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

        # max range
        if (line.find("nGates") > 0):
            parts = line.strip().split()
            nGates = parts[-1]
            continue

        # max range
        if (line.find("maxRangeKm") > 0):
            parts = line.strip().split()
            maxRangeKm = parts[-1]
            continue

        # beam width
        if (line.find("beamWidth") > 0):
            parts = line.strip().split()
            beamWidth = parts[-1]
            continue

        # elevations
        if (line.find("elevs") > 0):
            parts = line.strip().split()
            elevList = parts[-1]
            elevs = elevList.split(',')
            continue

        # col headers?
        if (line.find("gateNum") > 0):
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


    if (options.verbose):
        print("colData: ", colData, file=sys.stderr)

    return 0

########################################################################
# Plot

def doPlot():

    rangeKm = np.array(colData["rangeKm"]).astype(np.double)
    
    widthIn = float(options.figWidthMm) / 25.4
    htIn = float(options.figHeightMm) / 25.4
    
    fig1 = plt.figure(1, (widthIn, htIn))
    title = (options.title + ' for scan: ' + scanName + '  beam width = ' + beamWidth + ' deg')
    fig1.suptitle(title, fontsize=11)
    ax1 = fig1.add_subplot(1,1,1,xmargin=0.0)

    ax1.set_xlim([0.0, float(maxRangeKm)])
    ax1.set_ylim([0.0, float(options.maxHtKm)])

    # plot the heights for each elevation angle

    colors = [ 'red', 'blue', 'orange', 'green' ]
    count = 0
    elevsUsed = []

    for elev in elevs:

        if elev in elevsUsed:
            continue

        elevsUsed.append(elev)

        gndRange = rangeKm * math.cos(math.radians(float(elev)))

        htBotLabel = "htKmBot[" + elev + "]"
        htMidLabel = "htKmMid[" + elev + "]"
        htTopLabel = "htKmTop[" + elev + "]"

        htKmBot = np.array(colData[htBotLabel]).astype(np.double)
        htKmMid = np.array(colData[htMidLabel]).astype(np.double)
        htKmTop = np.array(colData[htTopLabel]).astype(np.double)

        col = colors[count % 4]

        x = []
        y = []
        
        for igate in range(0, int(nGates) - 1, 1):
            x.append(gndRange[igate])
            y.append(htKmBot[igate])

        for igate in range(int(nGates) - 1, 0, -1):
            x.append(gndRange[igate])
            y.append(htKmTop[igate])

        ax1.fill(x, y, color = col, alpha = 0.2)

        ax1.plot(gndRange, htKmBot, linewidth=1, color = col)
        ax1.plot(gndRange, htKmMid, linewidth=1, dashes = [4, 4], color = col)
        ax1.plot(gndRange, htKmTop, linewidth=1, color = col)

        count = count + 1

    ax1.set_ylabel('Height above radar (km)')
    ax1.set_xlabel('Ground range (km)')
    ax1.set_title('Elevs: ' + elevList, fontsize=8)
    ax1.grid(True)
    
    #ax1.grid(which='minor', alpha=0.1)
    #ax1.grid(which='major', alpha=0.2)
    #major_ticks = np.arange(0, 1, 0.2)
    #minor_ticks = np.arange(0.1, 0.9, 0.2)
    #ax1.set_xticks(major_ticks)
    #ax1.set_xticks(minor_ticks, minor=True)
    #ax1.set_yticks(major_ticks)
    #ax1.set_yticks(minor_ticks, minor=True)

    #legend1 = ax1.legend(loc='upper left', ncol=3, framealpha=0.5, fancybox=True)
    #for label in legend1.get_texts():
    #    label.set_fontsize('x-small')

    homeDir = os.environ['HOME']
    downloadsDir = os.path.join(homeDir, 'Downloads')
    savePath = os.path.join(downloadsDir, "range_height." + scanName + ".png")
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


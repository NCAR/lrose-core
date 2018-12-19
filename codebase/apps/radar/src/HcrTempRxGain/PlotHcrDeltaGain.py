#!/usr/bin/env python

#===========================================================================
#
# Produce plots for HCR delta gain results
#
#===========================================================================

import os
import sys
import subprocess
from optparse import OptionParser
import numpy as np
from numpy import convolve
from numpy import linalg, array, ones
import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib import dates
import math
import datetime
import contextlib

def main():

    # globals

    global options
    global debug
    global startTime
    global endTime
    global timeLimitsSet
    timeLimitsSet = False
    global figNum
    figNum = 0

    # parse the command line

    usage = "usage: " + __file__ + " [options]"
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
                      dest='gainFilePath',
                      default='/tmp/hcr/deltaGains.txt',
                      help='File path for delta gains')
    parser.add_option('--widthMain',
                      dest='mainWidthMm',
                      default=400,
                      help='Width of main figure in mm')
    parser.add_option('--heightMain',
                      dest='mainHeightMm',
                      default=300,
                      help='Height of main figure in mm')
    parser.add_option('--start',
                      dest='startTime',
                      default='1970 01 01 00 00 00',
                      help='Start time for XY plot')
    parser.add_option('--end',
                      dest='endTime',
                      default='1970 01 01 00 00 00',
                      help='End time for XY plot')
    
    (options, args) = parser.parse_args()
    
    if (options.verbose == True):
        options.debug = True

    # time limits

    year, month, day, hour, minute, sec = options.startTime.split()
    startTime = datetime.datetime(int(year), int(month), int(day),
                                  int(hour), int(minute), int(sec))

    year, month, day, hour, minute, sec = options.endTime.split()
    endTime = datetime.datetime(int(year), int(month), int(day),
                                int(hour), int(minute), int(sec))

    Jan1970 = datetime.datetime(1970, 1, 1, 0, 0, 0)
    if ((startTime != Jan1970) and (endTime != Jan1970)):
        timeLimitsSet = True

    if (options.debug == True):
        print >>sys.stderr, "Running ", __file__
        print >>sys.stderr, "  gainFilePath: ", options.gainFilePath
        if (timeLimitsSet):
            print >>sys.stderr, "  startTime: ", startTime
            print >>sys.stderr, "  endTime: ", endTime


    # read in column headers for bias results

    iret, gainHdrs, gainData = readColumnHeaders(options.gainFilePath)
    if (iret != 0):
        sys.exit(-1)

    # read in data for comp results

    gainData, gainTimes = readInputData(options.gainFilePath, gainHdrs, gainData)

    # load up the data arrays

    loadDataArrays(gainData, gainTimes)

    # render the plots
    
    doPlots()

    # show them

    plt.show()

    sys.exit(0)
    
########################################################################
# Read columm headers for the data
# this is in the first line

def readColumnHeaders(filePath):

    colHeaders = []
    colData = {}

    fp = open(filePath, 'r')
    line = fp.readline()
    fp.close()
    
    commentIndex = line.find("#")
    if (commentIndex == 0):
        # header
        colHeaders = line.lstrip("# ").rstrip("\n").split()
        if (options.debug == True):
            print >>sys.stderr, "colHeaders: ", colHeaders
    else:
        print >>sys.stderr, "ERROR - readColumnHeaders"
        print >>sys.stderr, "  First line does not start with #"
        return -1, colHeaders, colData
    
    for index, var in enumerate(colHeaders, start=0):
        colData[var] = []
        
    return 0, colHeaders, colData

########################################################################
# Read in the data

def readInputData(filePath, colHeaders, colData):

    # open file

    fp = open(filePath, 'r')
    lines = fp.readlines()

    obsTimes = []
    colData = {}
    for index, var in enumerate(colHeaders, start=0):
        colData[var] = []

    # read in a line at a time, set colData
    for line in lines:
        
        commentIndex = line.find("#")
        if (commentIndex >= 0):
            continue
            
        # data
        
        data = line.strip().split()
        if (len(data) != len(colHeaders)):
            if (options.debug == True):
                print >>sys.stderr, "skipping line: ", line
            continue;

        values = {}
        for index, var in enumerate(colHeaders, start=0):
            # print >>sys.stderr, "index, data[index]: ", index, ", ", data[index]
            if (var == 'count' or var == 'year' or var == 'month' or var == 'day' or \
                var == 'hour' or var == 'min' or var == 'sec' or \
                var == 'unix_time'):
                values[var] = int(data[index])
            else:
                values[var] = float(data[index])

        # load observation times array

        year = values['year']
        month = values['month']
        day = values['day']
        hour = values['hour']
        minute = values['min']
        sec = values['sec']

        thisTime = datetime.datetime(year, month, day,
                                     hour, minute, sec)

        if (thisTime >= startTime and thisTime <= endTime):
            for index, var in enumerate(colHeaders, start=0):
                colData[var].append(values[var])
            obsTimes.append(thisTime)

    fp.close()

    return colData, obsTimes

########################################################################
# Set up arrays for plotting

def loadDataArrays(gainData, gainTimes):

    # set up arrays

    global gtimes

    gtimes = np.array(gainTimes).astype(datetime.datetime)

    global lnaTempObs,podTempObs,lnaTempSmoothed,podTempSmoothed
    global lnaDeltaGain,rxDeltaGain,deltaGain

    lnaTempObs = np.array(gainData["lnaTempObs"]).astype(np.double)
    podTempObs = np.array(gainData["podTempObs"]).astype(np.double)
    lnaTempSmoothed = np.array(gainData["lnaTempSmoothed"]).astype(np.double)
    podTempSmoothed = np.array(gainData["podTempSmoothed"]).astype(np.double)
    lnaDeltaGain = np.array(gainData["lnaDeltaGain"]).astype(np.double)
    rxDeltaGain = np.array(gainData["rxDeltaGain"]).astype(np.double)
    deltaGain = np.array(gainData["deltaGain"]).astype(np.double)


########################################################################
# Plot temperatures

def doPlots():

    # set up plots

    widthIn = float(options.mainWidthMm) / 25.4
    htIn = float(options.mainHeightMm) / 25.4

    global figNum
    fig = plt.figure(figNum, (widthIn, htIn))
    figNum = figNum + 1
    
    ax1 = fig.add_subplot(3,1,1,xmargin=0.0)
    ax2 = fig.add_subplot(3,1,2,xmargin=0.0)
    ax3 = fig.add_subplot(3,1,3,xmargin=0.0)
    
    ax1.plot(gtimes, lnaTempObs, \
             label='lnaTempObs(C)', color='blue', linewidth=1)
    ax1.plot(gtimes, lnaTempSmoothed, \
             label='lnaTempSmoothed(C)', color='red', linewidth=1)
    
    ax2.plot(gtimes, podTempObs, \
             label='podTempObs(C)', color='blue', linewidth=1)
    ax2.plot(gtimes, podTempSmoothed, \
             label='podTempSmoothed(C)', color='red', linewidth=1)
    
    ax3.plot(gtimes, lnaDeltaGain, \
             label='lnaDeltaGain(dB)', color='blue', linewidth=1)
    ax3.plot(gtimes, rxDeltaGain, \
             label='rxDeltaGain(dB)', color='green', linewidth=1)
    ax3.plot(gtimes, deltaGain, \
             label='deltaGain(dB)', color='red', linewidth=1)
    
    configTimeAxis(ax1, -9999, -9999, "LNA Temp", 'upper center')
    configTimeAxis(ax2, -9999, -9999, "POD Temp", 'upper center')
    configTimeAxis(ax3, -1.5, 1.5, "Delta Gains", 'upper center')

    fig.autofmt_xdate()
    fig.tight_layout()
    fig.subplots_adjust(bottom=0.08, left=0.06, right=0.97, top=0.96)

    fig.suptitle("TEMP vs DELTA GAIN - file " + os.path.basename(options.gainFilePath))

    return

########################################################################
# Configure axes, legends etc

def configTimeAxis(ax, miny, maxy, ylabel, legendLoc):
    
    legend = ax.legend(loc=legendLoc, ncol=8)
    for label in legend.get_texts():
        label.set_fontsize('x-small')
        ax.set_xlabel("Time")
    ax.set_ylabel(ylabel)
    ax.grid(True)
    if (miny > -9990 and maxy > -9990):
        ax.set_ylim([miny, maxy])
    hfmt = dates.DateFormatter('%H:%M:%S')
    ax.xaxis.set_major_locator(dates.AutoDateLocator())
    ax.xaxis.set_major_formatter(hfmt)
    for tick in ax.xaxis.get_major_ticks():
        tick.label.set_fontsize(8) 

    if (timeLimitsSet):
        ax.set_xlim(startTime, endTime)

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


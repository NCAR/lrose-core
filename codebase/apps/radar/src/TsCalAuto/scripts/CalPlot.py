#!/usr/bin/env python

#===========================================================================
#
# Plot calibration results
#
#===========================================================================

from __future__ import print_function

import os
import sys
import subprocess
from optparse import OptionParser
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import dates
import math
import datetime

def main():

#   globals

    global options
    global debug

    global varIndex
    varIndex = 8    # starting variable
    
    global colHeaders
    colHeaders = []

    global colIndex
    colIndex = {}

    global colData
    colData = {}

    global obsTimes
    obsTimes = []

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global fig1
    global ax1
    global ax2

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
                      dest='inputFilePath',
                      default='/home/spol/git/spol-configuration/projDir/calibration/results/cal.spoldrx.1.5us.sim/TsCalAuto_20220803_020856.txt',
                      help='Input file path - calibration results')
    parser.add_option('--radarName',
                      dest='radarName',
                      default='SPOL',
                      help='Name of radar')
    parser.add_option('--title',
                      dest='title',
                      default='S-Pol Calibration',
                      help='Title for plot')
    parser.add_option('--width',
                      dest='figWidthMm',
                      default=240,
                      help='Width of figure in mm')
    parser.add_option('--height',
                      dest='figHeightMm',
                      default=300,
                      help='Height of figure in mm')
    
    (options, args) = parser.parse_args()
    
    if (options.verbose):
        options.debug = True

    if (options.debug):
        print("  Running " + thisScriptName, file=sys.stderr)
        print("  inputFile: " + options.inputFilePath, file=sys.stderr)

    # read in column headers

    readColumnHeaders()

    # read in cal results

    readCalResults()

    # plot results
    
    doPlot()

    sys.exit(0)
    
########################################################################
# Read columm headers for the data
# this is in the fist line

def readColumnHeaders():

    global colHeaders
    global colIndex
    global colData

    fp = open(options.inputFilePath, 'r')
    line = fp.readline()
    fp.close()
    
    commentIndex = line.find("#")
    if (commentIndex == 0):
        # header
        headerLine = line.lstrip("# ").rstrip("\n")
    else:
        # no header, use defaults
        headerLine = "siggen hc vc hx vx hcmhx vcmvx wgh wgv hcNs vcNs hxNs vxNs"

    colHeaders = headerLine.split()
    if (options.verbose):
        print('colHeaders: ', colHeaders, file=sys.stderr)
    
    for index, var in enumerate(colHeaders, start=0):
        colIndex[var] = index
        colData[var] = []
        
    if (options.verbose):
        print('colIndex: ', colIndex, file=sys.stderr)

    return 0

########################################################################
# Read in the data

def readCalResults():

    global nData
    global colData
    global obsTimes

    # open file

    fp = open(options.inputFilePath, 'r')
    lines = fp.readlines()
    fp.close()

    # read in a line at a time, set colData

    nData = 0
    for line in lines:
        
        commentIndex = line.find("#")
        if (commentIndex >= 0):
            continue
            
        # data
        
        data = line.strip().split()
        for index, var in enumerate(colHeaders, start=0):
            if (float(data[index]) < -990):
                colData[var].append(math.nan)
            else:
                colData[var].append(float(data[index]))
        nData = nData + 1

    if (options.debug):

        print('Data:', file=sys.stderr)

        # print headers

        print('{:>8s}'.format("index"), end=' ', file=sys.stderr)
        for ifield, field in enumerate(colHeaders, start=0):
            fieldName = colHeaders[ifield]
            if (ifield == len(colHeaders) - 1):
                print('{:>9s}'.format(fieldName), end='', file=sys.stderr)
            else:
                print('{:>9s}'.format(fieldName), end=' ', file=sys.stderr)
        print('', file=sys.stderr)

        # print data

        for index in range(0, nData):

            print('{:8}'.format(index), end=' ', file=sys.stderr)

            for ifield, field in enumerate(colHeaders, start=0):
                fieldName = colHeaders[ifield]
                fieldVal = colData[fieldName][index]
                if (ifield == len(colHeaders) - 1):
                    print('{:9.3f}'.format(fieldVal), end='', file=sys.stderr)
                else:
                    print('{:9.3f}'.format(fieldVal), end=' ', file=sys.stderr)

            print('', file=sys.stderr)

########################################################################
# Plot

def doPlot():

    global fig1
    global ax1
    global ax2
    global ax3

    widthIn = float(options.figWidthMm) / 25.4
    htIn = float(options.figHeightMm) / 25.4

    fig1 = plt.figure(1, (widthIn, htIn))

    ax1 = fig1.add_subplot(3,1,1,xmargin=0.0)
    ax2 = fig1.add_subplot(3,1,2,xmargin=0.0)
    ax3 = fig1.add_subplot(3,1,3,xmargin=0.0)

    plotCalResults()
    
    filePath = options.inputFilePath
    fileName = os.path.basename(filePath)
    subdirName = os.path.basename(os.path.dirname(filePath))
    titleStr = "Calibration file: " + subdirName + " " + fileName

    fig1.suptitle(titleStr)
    plt.tight_layout()
    plt.subplots_adjust(top=0.96)
    plt.show()

########################################################################
# Reload and redraw - after getting new file

def reloadAndDraw():

    # read in column header

    if (readColumnHeaders() != 0):
        sys.exit(-1)

    # read in file

    readInputData()

    # plot XY
    
    plotCalResults()
    plt.draw()
    
########################################################################
# Plot data on axes

def plotCalResults():

    ax1.clear()
    ax2.clear()
    ax3.clear()

    filePath = options.inputFilePath
    fileName = os.path.basename(filePath)
    subdirName = os.path.basename(os.path.dirname(filePath))

    titleStr = "File: " + subdirName + " " + fileName
    hfmt = dates.DateFormatter('%y/%m/%d')

#   set up np arrays

    pwrSiggen = np.array(colData["siggen"]).astype(np.double)
    pwrHc = np.array(colData["hc"]).astype(np.double)
    pwrVc = np.array(colData["vc"]).astype(np.double)
    pwrHx = np.array(colData["hx"]).astype(np.double)
    pwrVx = np.array(colData["vx"]).astype(np.double)
    pwrNsHc = np.array(colData["hcNs"]).astype(np.double)
    pwrNsVc = np.array(colData["vcNs"]).astype(np.double)

    noiseHc = np.mean(pwrHc[-4,])
    noiseVc = np.mean(pwrVc[-4,])

    noiseLinHc = math.pow(10.0, noiseHc / 10.0)
    noiseLinVc = math.pow(10.0, noiseVc / 10.0)

    snrHc = []
    snrVc = []
    for ii in range(0, len(pwrHc)):
        pwrH = pwrNsHc[ii]
        snrH = math.log10((math.pow(10.0, pwrH / 10.0) / noiseLinHc)) * 10.0
        snrHc.append(snrH)
        pwrV = pwrNsVc[ii]
        snrV = math.log10((math.pow(10.0, pwrV / 10.0) / noiseLinVc)) * 10.0
        snrVc.append(snrV)

    snrHc = np.array(snrHc).astype(np.double)
    snrVc = np.array(snrVc).astype(np.double)

    # received power vs siggen

    ax1.plot(pwrSiggen, pwrHc, linestyle = "", marker = "x",
             label = 'pwrHc', color = 'darkgreen')
    ax1.plot(pwrSiggen, pwrVc, linestyle = "", marker = "+",
             label = 'pwrVc', color = 'red')

    ax1.plot(pwrSiggen, pwrNsHc, \
             linewidth=1, label = 'pwrNsHc', color = 'magenta')
    ax1.plot(pwrSiggen, pwrNsVc, \
             linewidth=1, label = 'pwrNsVc', color = 'blue')

    legend1 = ax1.legend(loc='upper left')
    for label in legend1.get_texts():
        label.set_fontsize('x-small')
    ax1.set_xlabel("Injected power from siggen (dBm)")
    ax1.set_ylabel("Received power pwrHc, pwrVc (dBm)")
    ax1.grid(True)

    # received SNR vs siggen

    ax2.plot(pwrSiggen, snrHc, \
             linewidth=1, label = 'snrHc', color = 'magenta')
    ax2.plot(pwrSiggen, snrVc, \
             linewidth=1, label = 'snrVc', color = 'blue')

    legend2 = ax2.legend(loc='upper left')
    for label in legend2.get_texts():
        label.set_fontsize('x-small')
    ax2.set_xlabel("Injected power from siggen (dBm)")
    ax2.set_ylabel("SNR snrHc, snrVc (dB)")
    ax2.grid(True)

    # ZDR

    zdr = pwrNsHc - pwrNsVc
    snrCond = (snrHc > 20.0) & (snrHc < 60.0)
    zdrGood = zdr[snrCond]
    zdrMean = np.mean(zdrGood)
    pwrDiff = pwrHc - pwrVc

    ax3.plot(snrHc, zdr, \
             linewidth=1, label = 'ZDR', color = 'red')

    ax3.plot(snrHc, pwrDiff, \
             linewidth=1, label = 'PwrDiff', color = 'blue')

    legend3 = ax3.legend(loc='upper left')
    for label in legend3.get_texts():
        label.set_fontsize('x-small')
    ax3.set_xlabel("SnrHC (dB)")
    ax3.set_ylabel("ZDR (dB)")
    ax3.grid(True)
    ax3.set_ylim([zdrMean - 2.0, zdrMean + 2.0])

    return

########################################################################
# Run a command in a shell, wait for it to complete

def runCommand(cmd):

    if (options.debug):
        print("running cmd:",cmd, file=sys.stderr)
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print("Child exited with code: ", retcode, file=sys.stderr)
            exit(1)
        else:
            if (options.debug):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, file=sys.stderr)
        exit(1)

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()


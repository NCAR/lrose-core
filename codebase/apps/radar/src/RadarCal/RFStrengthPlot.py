#!/usr/bin/env python

#===========================================================================
#
# Plot RF field strength for a radar
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
    parser.add_option('--peakPowerDbm',
                      dest='peakPowerDbm',
                      default=87.5,
                      help='Peak power in dBm')
    parser.add_option('--peakPowerKw',
                      dest='peakPowerKw',
                      default=-9999.0,
                      help='Peak power in kW')
    parser.add_option('--gain',
                      dest='g0',
                      default=45.0,
                      help='Gain of antenna main lobe (dB)')
    parser.add_option('--efficiency',
                      dest='efficiency',
                      default=1.0,
                      help='Antenna efficiency (fraction)')
    parser.add_option('--diameter',
                      dest='diameter',
                      default=9.0,
                      help='Antenna diameter (m)')
    parser.add_option('--prf',
                      dest='prf',
                      default=1000.0,
                      help='Pulse repetition frequency (s-1)')
    parser.add_option('--pulseLength',
                      dest='tau',
                      default=1.0e-6,
                      help='Pulse length (seconds)')
    parser.add_option('--freq',
                      dest='freq',
                      default=2.809e9,
                      help='Transmit frequency (Hz)')
    
    (options, args) = parser.parse_args()
    
    if (options.verbose):
        options.debug = True

    peakPowerW = math.pow(10.0, float(options.peakPowerDbm) / 10.0) / 1.0e3
    if (float(options.peakPowerKw) > 0) :
        peakPowerW = float(options.peakPowerKw) * 1000.0
        
    g0Db = float(options.g0)
    g0 = math.pow(10.0, g0Db / 10.0)
    efficiency = float(options.efficiency)
    diameter = float(options.diameter)
    prf = float(options.prf)
    tau = float(options.tau)
    freq = float(options.freq)
    wavelength = 2.99792458e8 / freq;

    r1 = (diameter * diameter) / (4.0 * wavelength)
    r2 = 0.6 * (diameter * diameter) / wavelength

    Snf = (16.0 * tau * prf * peakPowerW * efficiency) / (math.pi * diameter * diameter)

    Sffr1 = (g0 * tau * prf * peakPowerW) / (4.0 * math.pi * r1 * r1)
    Sffr2 = (g0 * tau * prf * peakPowerW) / (4.0 * math.pi * r2 * r2)

    print("Running " + thisScriptName, file=sys.stderr)
    print("  radar name: " + options.radarName, file=sys.stderr)
    print(f"  peak power W: {peakPowerW:.3g}", file=sys.stderr)
    print(f"  antenna gain (dB) {g0Db:.2f}", file=sys.stderr)
    print(f"  antenna gain (linear) {g0:.2f}", file=sys.stderr)
    print(f"  antenna efficiency: {efficiency:.3f}", file=sys.stderr)
    print(f"  antenna diameter (m) {diameter:.2f}", file=sys.stderr)
    print(f"  PRF (s-1) {prf:.2f}", file=sys.stderr)
    print(f"  pulse length (s) {tau:.3g}", file=sys.stderr)
    print(f"  transmit freq (Hz) {freq:.3g}", file=sys.stderr)
    print(f"  wavelength (m) {wavelength:.3g}", file=sys.stderr)
    print(f"  r1 (m) {r1:.3g}", file=sys.stderr)
    print(f"  r2 (m) {r2:.3g}", file=sys.stderr)
    print(f"  Snf (W/m2) {Snf:.2g}", file=sys.stderr)
    print(f"  Sffr1 (W/m2) {Sffr1:.2g}", file=sys.stderr)
    print(f"  Sffr2 (W/m2) {Sffr2:.2g}", file=sys.stderr)

    # read in column headers

    # readColumnHeaders()

    # read in cal results

    # readCalResults()

    # plot results
    
    # doPlot()

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


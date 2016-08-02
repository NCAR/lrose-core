#!/usr/bin/env python

#===========================================================================
#
# Produce plots for CP ratio analysis
#
#===========================================================================

import os
import sys
import subprocess
from optparse import OptionParser
import numpy as np
from numpy import convolve
import matplotlib.pyplot as plt
from matplotlib import dates
import math
import datetime

def main():

#   globals

    global options
    global debug
    global meanFilterLen

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

    global fig1
    global ax1
    global ax2
    global ax3
    global ax4

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
                      default='/scr/hail2/rsfdata/pecan/cal/zdr/cp_alt/spol_pecan_clutter_0.5deg_20150601_000146.txt',
                      help='Input file path')
    parser.add_option('--title',
                      dest='title',
                      default='CP Ratio Analysis',
                      help='Title for plot')
    parser.add_option('--width',
                      dest='figWidthMm',
                      default=600,
                      help='Width of figure in mm')
    parser.add_option('--height',
                      dest='figHeightMm',
                      default=400,
                      help='Height of figure in mm')
    parser.add_option('--meanLen',
                      dest='meanLen',
                      default=1,
                      help='Len of moving mean filter')
    
    (options, args) = parser.parse_args()
    
    if (options.verbose == True):
        options.debug = True

    if (options.debug == True):
        print >>sys.stderr, "Running PlotCpRatio:"
        print >>sys.stderr, "  inputFilePath: ", options.inputFilePath

    meanFilterLen = int(options.meanLen)

    # read in column headers

    if (readColumnHeaders() != 0):
        sys.exit(-1)

    # read in file

    readInputData()

    # plot XY
    
    plotXy()

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
        colHeaders = line.lstrip("# ").rstrip("\n").split()
        if (options.debug == True):
            print >>sys.stderr, "colHeaders: ", colHeaders
    else:
        print >>sys.stderr, "ERROR - readColumnHeaders"
        print >>sys.stderr, "  First line does not start with #"
        return -1
    
    for index, var in enumerate(colHeaders, start=0):
        colIndex[var] = index
        colData[var] = []
        
    if (options.debug == True):
        print >>sys.stderr, "colIndex: ", colIndex

    return 0

########################################################################
# Read in the data

def readInputData():

    global colData
    global obsTimes

    # open file

    fp = open(options.inputFilePath, 'r')
    lines = fp.readlines()

    # read in a line at a time, set colData
    for line in lines:
        
        commentIndex = line.find("#")
        if (commentIndex >= 0):
            continue
            
        # data
        
        data = line.strip().split()

        for index, var in enumerate(colHeaders, start=0):
            if (var == 'obsNum' or var == 'year' or var == 'month' or var == 'day' or \
                var == 'hour' or var == 'min' or var == 'sec' or \
                var == 'nPairsClut' or var == 'nPairsWx'):
                colData[var].append(int(data[index]))
            else:
                colData[var].append(float(data[index]))

    fp.close()

    # load observation times array

    year = colData['year']
    month = colData['month']
    day = colData['day']
    hour = colData['hour']
    minute = colData['min']
    sec = colData['sec']

    for ii, var in enumerate(year, start=0):
        thisTime = datetime.datetime(year[ii], month[ii], day[ii],
                                     hour[ii], minute[ii], sec[ii])
        obsTimes.append(thisTime)

########################################################################
# Moving average filter

def movingAverage(values, window):

    weights = np.repeat(1.0, window)/window
    sma = np.convolve(values, weights, 'valid')
    return sma

########################################################################
# Key-press event

def press(event):

    global varIndex

    if (options.debug == True):
        print >>sys.stderr, "press: ", event.key
        
    if (event.key == 'left'):
        if (varIndex > 0):
            varIndex = varIndex - 1
            reloadAndDraw()
            
    if (event.key == 'right'):
        if (varIndex < len(colHeaders) - 1):
            varIndex = varIndex + 1
            reloadAndDraw()
            
    if (options.debug == True):
        print >>sys.stderr, "  Var index : ", varIndex

########################################################################
# Plot - original instance

def plotXy():

    global fig1
    global ax1
    global ax2
    global ax3
    global ax4

    widthIn = float(options.figWidthMm) / 25.4
    htIn = float(options.figHeightMm) / 25.4

    fig1 = plt.figure(1, (widthIn, htIn))
    fig1.canvas.mpl_connect('key_press_event', press)

#    ax1 = fig1.add_subplot(1,3,1,xmargin=0.0)
#    ax2 = fig1.add_subplot(1,3,2,xmargin=0.0)
#    ax3 = fig1.add_subplot(1,3,3,xmargin=0.0)

    ax1 = fig1.add_subplot(4,1,1,xmargin=0.0)
    ax2 = fig1.add_subplot(4,1,2,xmargin=0.0)
    ax3 = fig1.add_subplot(4,1,3,xmargin=0.0)
    ax4 = fig1.add_subplot(4,1,4,xmargin=0.0)

    doPlot()
    fig1.suptitle("Cross-polar power analysis - file: " + options.inputFilePath)
    fig1.autofmt_xdate()
    plt.tight_layout()
    plt.subplots_adjust(top=0.96)
    plt.show()

########################################################################
# Reload and redraw - after getting new file

def reloadAndDraw():

    # read in column headers

    if (readColumnHeaders() != 0):
        sys.exit(-1)

    # read in file

    readInputData()

    # plot XY
    
    doPlot()
    plt.draw()
    
########################################################################
# Plot data on axes

def doPlot():

    ax1.clear()
    ax2.clear()
    ax3.clear()
    ax4.clear()

    fileName = options.inputFilePath
    titleStr = "File: " + fileName
    hfmt = dates.DateFormatter('%y/%m/%d')

#   set up np arrays

    times = np.array(obsTimes).astype(datetime.datetime)

    ratioClut = np.array(colData["cpRatioClut"]).astype(np.double)
    ratioClut = movingAverage(ratioClut, meanFilterLen)

    ratioWx = np.array(colData["cpRatioWx"]).astype(np.double)
#    ratioWx = movingAverage(ratioWx, meanFilterLen)

    txPwrRatioHV = np.array(colData["TxPwrRatioHV"]).astype(np.double)
    txPwrRatioHV = movingAverage(txPwrRatioHV, meanFilterLen)

    tempSite = np.array(colData["TempSite"]).astype(np.double)

    tempRear = np.array(colData["TempRear"]).astype(np.double)

    tempFront = np.array(colData["TempFront"]).astype(np.double)

    tempKlystron = np.array(colData["TempKlystron"]).astype(np.double)

    TxPwrH = np.array(movingAverage(colData["TxPwrH"], meanFilterLen)).astype(np.double)
    TxPwrV = np.array(movingAverage(colData["TxPwrV"], meanFilterLen)).astype(np.double)

    # ax1 - CP ratio
    
    validClut = np.isfinite(ratioClut)
    validWx = np.isfinite(ratioWx)

    #  ax1.set_title(titleStr, fontsize=12)
    ax1.plot(times[validWx], ratioWx[validWx], \
             linewidth=1, label = 'cpRatioWx', color = 'seagreen')
    ax1.plot(times[validClut], ratioClut[validClut], \
             linewidth=1, label = 'cpRatioClut', color = 'black')
    
    legend1 = ax1.legend(loc='upper right')
    for label in legend1.get_texts():
        label.set_fontsize('x-small')
    ax1.set_xlabel("Date")
    ax1.set_ylabel("CpRatioClut, CpRatioWx")
    ax1.grid(True)
#    ax1.set_ylim([0.05, 0.3])

    hfmt = dates.DateFormatter('%y/%m/%d')
    ax1.xaxis.set_major_locator(dates.DayLocator())
    ax1.xaxis.set_major_formatter(hfmt)

    # ax2 - transmit power ratio

#    validXmitRatio = np.isfinite(txPwrRatioHV)
#    validXmitRatio = movingAverage(validXmitRatio, meanFilterLen)
    validXmitRatio = np.isfinite(movingAverage(txPwrRatioHV, meanFilterLen))

    ax2.plot(times[validXmitRatio], txPwrRatioHV[validXmitRatio], \
             linewidth=1, label = 'Xmit Pwr Ratio H/V (dB)', color = 'red')
    
    legend2 = ax2.legend(loc='upper right')
    for label in legend2.get_texts():
        label.set_fontsize('x-small')
    ax2.set_xlabel("Date")
    ax2.set_ylabel("Xmit Pwr Ratio H/V (dB)")
    ax2.grid(True)
    ax2.xaxis.set_major_locator(dates.DayLocator())
    ax2.xaxis.set_major_formatter(hfmt)
    ax2.set_ylim([-0.25, 0.45])

    # ax3 - XMIT power

    validTxPwrH = np.isfinite(TxPwrH)
    validTxPwrV = np.isfinite(TxPwrV)
    ax3.plot(times[validTxPwrH], TxPwrH[validTxPwrH], \
             linewidth=1, label = 'TxPwrH', color = 'blue')
    ax3.plot(times[validTxPwrV], TxPwrV[validTxPwrV], \
             linewidth=1, label = 'TxPwrV', color = 'cyan')
    
    legend3 = ax3.legend(loc='upper right')
    for label in legend3.get_texts():
        label.set_fontsize('x-small')
    ax3.set_xlabel("Date")
    ax3.set_ylabel("Tx Power (dBm)")
    ax3.grid(True)
    ax3.set_ylim([84.5, 89.5])

    ax3.xaxis.set_major_locator(dates.DayLocator())
    ax3.xaxis.set_major_formatter(hfmt)

    # ax4 - Site and klystron Temperature

    validTempSite = np.isfinite(tempSite)
    validTempRear = np.isfinite(tempRear)
    validTempFront = np.isfinite(tempFront)
    validTempKlystron = np.isfinite(tempKlystron)
    ax4.plot(times[validTempSite], tempSite[validTempSite], \
             linewidth=1, label = 'Site Temp', color = 'green')
    ax4.plot(times[validTempRear], tempRear[validTempRear], \
             linewidth=1, label = 'Rear Temp', color = 'chartreuse')
    ax4.plot(times[validTempFront], tempFront[validTempFront], \
             linewidth=1, label = 'Front Temp', color = 'blue')
    ax4.plot(times[validTempKlystron], tempKlystron[validTempKlystron], \
             linewidth=1, label = 'Klystron Temp', color = 'orange')
    
    legend2 = ax4.legend(loc='upper right')
    for label in legend2.get_texts():
        label.set_fontsize('x-small')
    ax4.set_xlabel("Date")
    ax4.set_ylabel("Temperatures (C)")
    ax4.grid(True)

    ax4.xaxis.set_major_locator(dates.DayLocator())
    ax4.xaxis.set_major_formatter(hfmt)
    ax4.set_ylim([10, 70])

    return

    #    nameParts = fileName.split("_")
    #    timeStr = "Time " + nameParts[4] + nameParts[5]
    #    azStr = nameParts[2]
    #    elStr = nameParts[3]
    
    #    rhohv50 = []
    #    for val in rhohv:
    #        rhohv50.append(float(val) * 50)
    #    zdrSdev10 = []
    #    for val in zdrSdev:
    #        zdrSdev10.append(float(val) * 10)

    
    #    ax1.set_title(titleStr, fontsize=12)
    #    ax1.plot(colData["obsNum"], colData["cpRatioClut"], \
        #                     linewidth=2, label = 'cpRatioClut')
    #    ax1.plot(colData["obsNum"], colData["cpRatioWx"], \
        #             linewidth=2, label = 'cpRatioWx')
    
    #    legend1 = ax1.legend(loc='upper right')
    #    for label in legend1.get_texts():
    #        label.set_fontsize('small')
    #    ax1.set_xlabel("obsNum")
    #    ax1.set_ylabel("CpRatioClut, CpRatioWx")
    
    #    validKdp2 = []
    #    for val in validKdp:
    #        validKdp2.append(float(val) * 20)
    #    validUnfold2 = []
    #    for val in validUnfold:
    #        validUnfold2.append(float(val) * -20)
    
    #    ax2.set_title(azStr, fontsize=12)
    #    ax2.plot(obsNum, validKdp2, 'k:', label = 'validKdp')
    #    ax2.plot(obsNum, validUnfold2, 'b:', label = 'validUnfold')
    #ax2.plot(obsNum, phidp, label = 'measured')
    #ax2.plot(obsNum, phidpMean, label = 'meanMeasured')
    #ax2.plot(obsNum, phidpMeanValid, label = 'meanValid')
    #ax2.plot(obsNum, dbz, label = 'DBZ')
    #ax2.plot(obsNum, phidpMeanUnfold, label = 'meanUnfolded')
    #ax2.plot(obsNum, phidpFilt, label = 'Filt')
    #ax2.plot(obsNum, phidpUnfold, label = 'Unfold')
    #ax2.plot(obsNum, phidpCond, label = 'Cond')
    #ax2.plot(obsNum, phidpCondFilt, label = 'CondFilt', color = 'black')
    #ax2.plot(obsNum, phidpSdev, label = 'Sdev', color = 'pink')
    #ax2.plot(obsNum, phidpJitter, label = 'Jitter', color = 'orange')
    #legend2 = ax2.legend(loc='upper right')
    #for label in legend2.get_texts():
    #    label.set_fontsize('small')
    #ax2.set_xlabel("obsNum")
    #ax2.set_ylabel("PHIDP")

    #validKdp3 = []
    #for val in validKdp:
    #    validKdp3.append(float(val) * 5)
    #validUnfold3 = []
    #for val in validUnfold:
    #    validUnfold3.append(float(val) * -2)

    #ax3.set_title(elStr, fontsize=12)
    #ax3.plot(obsNum, validKdp3, 'k:', label = 'validKdp')
    #ax3.plot(obsNum, validUnfold3, 'b:', label = 'validUnfold')
    #ax3.plot(obsNum, psob, label = 'psob', color = 'orange')
    #ax3.plot(obsNum, kdp, label = 'KDP', color = 'red')
    #legend2 = ax3.legend(loc='upper left')
    #for label in legend2.get_texts():
    #    label.set_fontsize('small')
    #ax3.set_xlabel("obsNum")
    #ax3.set_ylabel("KDP,PSOB")

    #    ax1.plot(obsNum, snr, label = 'SNR')
    #    ax1.plot(obsNum, dbz, label = 'DBZ')
    #    ax1.plot(obsNum, rhohv50, label = 'RHOHV*50')
    #    ax1.plot(obsNum, zdrSdev10, label = 'ZdrSdev*10')
    #    ax1.plot(obsNum, dbzAtten, label = 'dbzAtten')
    #ax1.plot(obsNum, dbzCorrected, label = 'dbzCorrected')
    #ax1.plot(obsNum, zdrCorrected, label = 'zdrCorrected')

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


#!/usr/bin/env python

#===========================================================================
#
# Plot Ray details for KDP analysis
#
#===========================================================================

import os
import sys
import subprocess
from optparse import OptionParser
import numpy as np
import matplotlib.pyplot as plt
import math

def main():

    global options
    global debug

    global dirPath
    global fileIndex
    global fileList
    
    global colHeaders
    global colIndex

    global gateNum
    global validKdp
    global validUnfold
    global snr
    global dbz
    global zdr
    global zdrSdev
    global rhohv
    global phidp
    global phidpMean
    global phidpMeanValid
    global phidpJitter
    global phidpSdev
    global phidpMeanUnfold
    global phidpUnfold
    global phidpFilt
    global phidpCond
    global phidpCondFilt
    global psob
    global kdp
    global dbzAtten
    global zdrAtten
    global dbzCorrected
    global zdrCorrected

    global fig1
    global ax1
    global ax2
    global ax3

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
                      default='/tmp/kdp_ray_files/kdp_ray_20130531-231614.500el002.8_az188.0.txt',
                      help='Input file path')
    parser.add_option('--title',
                      dest='title',
                      default='KDP-analysis',
                      help='Title for plot')
    parser.add_option('--width',
                      dest='figWidthMm',
                      default=300,
                      help='Width of figure in mm')
    parser.add_option('--height',
                      dest='figHeightMm',
                      default=150,
                      help='Height of figure in mm')
    parser.add_option('--maxgates',
                      dest='maxGates',
                      default=10000,
                      help='Max number of gates to plot')
    
    (options, args) = parser.parse_args()
    
    if (options.verbose == True):
        options.debug = True

    if (options.debug == True):
        print >>sys.stderr, "Running PlotTitanTableType1:"
        print >>sys.stderr, "  inputFilePath: ", options.inputFilePath
        print >>sys.stderr, "  maxGates: ", options.maxGates

    # read in file list

    readFileList()
    
    # read in column headers

    if (readColumnHeaders() != 0):
        sys.exit(-1)

    # read in file

    readInputData()

    # plot XY
    
    plotXy()

    sys.exit(0)
    
########################################################################
# Read in list of available files in the directory

def readFileList():

    global dirPath
    global fileIndex
    global fileList
    
    dirPath = os.path.dirname(options.inputFilePath)
    fileList = os.listdir(dirPath)
    fileList.sort()

    fileIndex = 0
    for index, file in enumerate(fileList):
        if (options.inputFilePath.find(file) > 0):
            fileIndex = index
            break
    
    if (options.debug == True):
        print >>sys.stderr, "Dir path: ", dirPath
        print >>sys.stderr, "Input file path: ", options.inputFilePath
        print >>sys.stderr, "File index : ", fileIndex
        print >>sys.stderr, "n files : ", len(fileList)
        print >>sys.stderr, "Computed File path: ", getFilePath()

    if (options.verbose == True):
        print >>sys.stderr, "Files:    "
        for index, file in enumerate(fileList):
            print >>sys.stderr, "   ", index, ": ", file

########################################################################
# Get the path to the current data file

def getFilePath():

    filePath = os.path.join(dirPath, fileList[fileIndex])
    return filePath
                            
########################################################################
# Read columm headers for the data
# this is in the fist line

def readColumnHeaders():

    global colHeaders
    colHeaders = []

    global colIndex
    colIndex = {}

    fp = open(getFilePath(), 'r')
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
    
    count = 0
    for var in colHeaders:
        colIndex[var] = count
        count = count + 1
        
    if (options.debug == True):
        print >>sys.stderr, "colIndex: ", colIndex

    return 0

########################################################################
# Read in the data

def readInputData():

    global gateNum
    global validKdp
    global validUnfold
    global snr
    global dbz
    global zdr
    global zdrSdev
    global rhohv
    global phidp
    global phidpMean
    global phidpMeanValid
    global phidpJitter
    global phidpSdev
    global phidpMeanUnfold
    global phidpUnfold
    global phidpFilt
    global phidpCond
    global phidpCondFilt
    global psob
    global kdp
    global dbzAtten
    global zdrAtten
    global dbzCorrected
    global zdrCorrected

    fp = open(getFilePath(), 'r')
    lines = fp.readlines()

    gateNum = []
    validKdp = []
    validUnfold = []
    snr = []
    dbz = []
    zdr = []
    zdrSdev = []
    rhohv = []
    phidp = []
    phidpMean = []
    phidpMeanValid = []
    phidpJitter = []
    phidpSdev = []
    phidpMeanUnfold = []
    phidpUnfold = []
    phidpFilt = []
    phidpCond = []
    phidpCondFilt = []
    psob = []
    kdp = []
    dbzAtten = []
    zdrAtten = []
    dbzCorrected = []
    zdrCorrected = []

    rowNum = 0
    for line in lines:
        
        commentIndex = line.find("#")
        if (commentIndex >= 0):
            continue
            
        # data
        
        data = line.strip().split()

        gateNum.append(data[colIndex["gateNum"]])
        validKdp.append(data[colIndex["validKdp"]])
        validUnfold.append(data[colIndex["validUnfold"]])
        snr.append(data[colIndex["snr"]])
        dbz.append(data[colIndex["dbz"]])
        zdr.append(data[colIndex["zdr"]])
        zdrSdev.append(data[colIndex["zdrSdev"]])
        rhohv.append(data[colIndex["rhohv"]])
        phidp.append(data[colIndex["phidp"]])
        phidpMean.append(data[colIndex["phidpMean"]])
        phidpMeanValid.append(data[colIndex["phidpMeanValid"]])
        phidpJitter.append(data[colIndex["phidpJitter"]])
        phidpSdev.append(data[colIndex["phidpSdev"]])
        phidpMeanUnfold.append(data[colIndex["phidpMeanUnfold"]])
        phidpUnfold.append(data[colIndex["phidpUnfold"]])
        phidpFilt.append(data[colIndex["phidpFilt"]])
        phidpCond.append(data[colIndex["phidpCond"]])
        phidpCondFilt.append(data[colIndex["phidpCondFilt"]])
        psob.append(data[colIndex["psob"]])
        kdp.append(data[colIndex["kdp"]])
        dbzAtten.append(data[colIndex["dbzAtten"]])
        zdrAtten.append(data[colIndex["zdrAtten"]])
        dbzCorrected.append(data[colIndex["dbzCorrected"]])
        zdrCorrected.append(data[colIndex["zdrCorrected"]])
        
        rowNum = rowNum + 1
        if (rowNum > int(options.maxGates)):
            fp.close()
            return
                
    fp.close()

########################################################################
# Key-press event

def press(event):

    global fileIndex

    if (options.debug == True):
        print >>sys.stderr, "press: ", event.key
        
    if (event.key == 'left'):
        if (fileIndex > 0):
            fileIndex = fileIndex - 1
            reloadAndDraw()
            
    if (event.key == 'right'):
        if (fileIndex < len(fileList) - 1):
            fileIndex = fileIndex + 1
            reloadAndDraw()
            
    if (options.debug == True):
        print >>sys.stderr, "  File index : ", fileIndex
        print >>sys.stderr, "  File path  : ", getFilePath()

########################################################################
# Plot - original instance

def plotXy():

    global fig1
    global ax1
    global ax2
    global ax3

    widthIn = float(options.figWidthMm) / 25.4
    htIn = float(options.figHeightMm) / 25.4

    fig1 = plt.figure(1, (widthIn, htIn))
    fig1.canvas.mpl_connect('key_press_event', press)

    ax1 = fig1.add_subplot(1,3,1,xmargin=0.0)
    ax2 = fig1.add_subplot(1,3,2,xmargin=0.0)
    ax3 = fig1.add_subplot(1,3,3,xmargin=0.0)

    doPlot()
    fig1.suptitle("KDP analysis")
    plt.tight_layout()
    plt.subplots_adjust(top=0.9)
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

    fileName = fileList[fileIndex]
    nameParts = fileName.split("_")
    timeStr = "Time " + nameParts[1]
    azStr = nameParts[2]
    elStr = nameParts[3]
    
    validKdp1 = []
    for val in validKdp:
        validKdp1.append(float(val) * 40)
    validUnfold1 = []
    for val in validUnfold:
        validUnfold1.append(float(val) * -40)
    rhohv50 = []
    for val in rhohv:
        rhohv50.append(float(val) * 50)
    zdrSdev10 = []
    for val in zdrSdev:
        zdrSdev10.append(float(val) * 10)

    ax1.set_title(timeStr, fontsize=12)
    ax1.plot(gateNum, validKdp1, 'k:', label = 'validKdp')
    ax1.plot(gateNum, validUnfold1, 'b:', label = 'validUnfold')
    ax1.plot(gateNum, snr, label = 'SNR')
    ax1.plot(gateNum, dbz, label = 'DBZ')
    ax1.plot(gateNum, rhohv50, label = 'RHOHV*50')
    ax1.plot(gateNum, zdrSdev10, label = 'ZdrSdev*10')
    ax1.plot(gateNum, dbzAtten, label = 'dbzAtten')
    #ax1.plot(gateNum, dbzCorrected, label = 'dbzCorrected')
    #ax1.plot(gateNum, zdrCorrected, label = 'zdrCorrected')
    legend1 = ax1.legend(loc='upper right')
    for label in legend1.get_texts():
        label.set_fontsize('small')
    ax1.set_xlabel("gateNum")
    ax1.set_ylabel("SNR, DBZ")

    validKdp2 = []
    for val in validKdp:
        validKdp2.append(float(val) * 20)
    validUnfold2 = []
    for val in validUnfold:
        validUnfold2.append(float(val) * -20)

    ax2.set_title(azStr, fontsize=12)
    ax2.plot(gateNum, validKdp2, 'k:', label = 'validKdp')
    ax2.plot(gateNum, validUnfold2, 'b:', label = 'validUnfold')
    #ax2.plot(gateNum, phidp, label = 'measured')
    #ax2.plot(gateNum, phidpMean, label = 'meanMeasured')
    #ax2.plot(gateNum, phidpMeanValid, label = 'meanValid')
    #ax2.plot(gateNum, dbz, label = 'DBZ')
    ax2.plot(gateNum, phidpMeanUnfold, label = 'meanUnfolded')
    ax2.plot(gateNum, phidpFilt, label = 'Filt')
    #ax2.plot(gateNum, phidpUnfold, label = 'Unfold')
    #ax2.plot(gateNum, phidpCond, label = 'Cond')
    ax2.plot(gateNum, phidpCondFilt, label = 'CondFilt', color = 'black')
    ax2.plot(gateNum, phidpSdev, label = 'Sdev', color = 'pink')
    ax2.plot(gateNum, phidpJitter, label = 'Jitter', color = 'orange')
    legend2 = ax2.legend(loc='upper right')
    for label in legend2.get_texts():
        label.set_fontsize('small')
    ax2.set_xlabel("gateNum")
    ax2.set_ylabel("PHIDP")

    validKdp3 = []
    for val in validKdp:
        validKdp3.append(float(val) * 5)
    validUnfold3 = []
    for val in validUnfold:
        validUnfold3.append(float(val) * -2)

    ax3.set_title(elStr, fontsize=12)
    ax3.plot(gateNum, validKdp3, 'k:', label = 'validKdp')
    ax3.plot(gateNum, validUnfold3, 'b:', label = 'validUnfold')
    ax3.plot(gateNum, psob, label = 'psob', color = 'orange')
    ax3.plot(gateNum, kdp, label = 'KDP', color = 'red')
    legend2 = ax3.legend(loc='upper left')
    for label in legend2.get_texts():
        label.set_fontsize('small')
    ax3.set_xlabel("gateNum")
    ax3.set_ylabel("KDP,PSOB")

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


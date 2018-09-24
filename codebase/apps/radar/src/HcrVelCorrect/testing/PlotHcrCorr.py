#!/usr/bin/env python3

#===========================================================================
#
# Produce plots for HCR vel corrections
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
import pathlib

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
                      dest='filePath',
                      default='/tmp/HcrVelCorrect.txt',
                      help='File path for correction results')
    parser.add_option('--widthMain',
                      dest='mainWidthMm',
                      default=400,
                      help='Width of main figure in mm')
    parser.add_option('--heightMain',
                      dest='mainHeightMm',
                      default=300,
                      help='Height of main figure in mm')
    parser.add_option('--filtLen',
                      dest='filtLen',
                      default=1,
                      help='Len of moving mean filter')
    parser.add_option('--start',
                      dest='startTime',
                      default='2018 01 24 00 25 00',
                      help='Start time for XY plot')
    parser.add_option('--end',
                      dest='endTime',
                      default='2018 01 24 00 40 00',
                      help='End time for XY plot')
    parser.add_option('--figDir',
                      dest='figureDir',
                      default='/figs/',
                      help='Directory for output figures')
    
    
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
        print("Running %prog", file=sys.stderr)
        print("  compFilePath: ", options.compFilePath, file=sys.stderr)
        if (timeLimitsSet):
            print("  startTime: ", startTime, file=sys.stderr)
            print("  endTime: ", endTime, file=sys.stderr)


    # read in column headers for correction results
    
    iret, corrHdrs, corrData = readColumnHeaders(options.filePath)
    if (iret != 0):
        sys.exit(-1)

    # read in data for corr results

    corrData, corrTimes = readInputData(options.filePath, corrHdrs, corrData)

    # load up the data arrays

    loadDataArrays(corrData, corrTimes)
    
    # make output figure name string
    
    if options.figureDir == '/figs/':
        options.figureDir=os.path.split(options.filePath)[0] + '/figs/'
        
    outFile=options.figureDir + os.path.splitext(os.path.split(options.filePath)[1])[0]
    
    # create figure directory if necessary
    
    pathlib.Path(options.figureDir).mkdir(parents=False, exist_ok=True)
    
    # close all existing figures
    
    mpl.pyplot.close("all")

    # render the plots
    
    doPlotRawAndFilt()
    doPlotFiltAndGeoref()
    #doPlotPitchRoll(outFile)
    #doPlotDiffs(outFile)
    #doPlotEstPitchDiff(outFile)
    #doPlotRadarAngles()
    #doPlot2DHist()

    # If you want to show the plots, uncomment the following line
    # Showing the plots will stop the script so it does not work when run as script
    
    plt.show()
   
    sys.exit()
   
    
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
            print("colHeaders: ", colHeaders, file=sys.stderr)
    else:
        print("ERROR - readColumnHeaders", file=sys.stderr)
        print("  First line does not start with #", file=sys.stderr)
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

    # read in a line at a time, set colData
    for line in lines:
        
        commentIndex = line.find("#")
        if (commentIndex >= 0):
            continue
            
        # data
        
        data = line.strip().split()
        if (len(data) != len(colHeaders)):
            if (options.debug == True):
                print("skipping line: ", line, file=sys.stderr)
            continue;

        for index, var in enumerate(colHeaders, start=0):
            if (var == 'count' or \
                var == 'year' or var == 'month' or var == 'day' or \
                var == 'hour' or var == 'min' or var == 'sec' or \
                var == 'unix_time'):
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

    obsTimes = []
    for ii, var in enumerate(year, start=0):
        thisTime = datetime.datetime(year[ii], month[ii], day[ii],
                                     hour[ii], minute[ii], sec[ii])
        obsTimes.append(thisTime)

    return colData, obsTimes

########################################################################
# Moving average filter

def movingAverage(values, window):

    if (window < 2):
        return values

    weights = np.repeat(1.0, window)/window
    sma = np.convolve(values, weights, 'same')
    return sma

########################################################################
# Set up arrays for plotting

def loadDataArrays(corrData, corrTimes):

    # set up arrays

    global ctimes

    ctimes = np.array(corrTimes).astype(datetime.datetime)

    global VelSurf
    global DbzSurf
    global RangeToSurf
    global VelNoiseFilt
    global VelNoiseFiltMean
    global VelNoiseFiltMedian
    global VelWaveFiltMean
    global VelWaveFiltMedian
    global VelWaveFiltPoly
    global VelCorr
    global Altitude
    global VertVel
    global Roll
    global Pitch
    global Rotation
    global Tilt
    global Elevation
    global DriveAngle1
    global DriveAngle2

    VelSurf = np.array(corrData["VelSurf"]).astype(np.double)
    DbzSurf = np.array(corrData["DbzSurf"]).astype(np.double)
    RangeToSurf = np.array(corrData["RangeToSurf"]).astype(np.double)
    VelNoiseFilt = np.array(corrData["VelNoiseFilt"]).astype(np.double)
    VelNoiseFiltMean = np.array(corrData["VelNoiseFiltMean"]).astype(np.double)
    VelNoiseFiltMedian = np.array(corrData["VelNoiseFiltMedian"]).astype(np.double)
    VelWaveFiltMean = np.array(corrData["VelWaveFiltMean"]).astype(np.double)
    VelWaveFiltMedian = np.array(corrData["VelWaveFiltMedian"]).astype(np.double)
    VelWaveFiltPoly = np.array(corrData["VelWaveFiltPoly"]).astype(np.double)
    VelCorr = np.array(corrData["VelCorr"]).astype(np.double)
    Altitude = np.array(corrData["Altitude"]).astype(np.double)
    VertVel = np.array(corrData["VertVel"]).astype(np.double)
    Roll = np.array(corrData["Roll"]).astype(np.double)
    Pitch = np.array(corrData["Pitch"]).astype(np.double)
    Rotation = np.array(corrData["Rotation"]).astype(np.double)
    Tilt = np.array(corrData["Tilt"]).astype(np.double)
    Elevation = np.array(corrData["Elevation"]).astype(np.double)
    DriveAngle1 = np.array(corrData["DriveAngle1"]).astype(np.double)
    DriveAngle2 = np.array(corrData["DriveAngle2"]).astype(np.double)

########################################################################
# Plot raw data plus filtered

def doPlotRawAndFilt():

    # set up plots

    widthIn = float(options.mainWidthMm) / 25.4
    htIn = float(options.mainHeightMm) / 25.4

    global figNum
    fig = plt.figure(figNum, (widthIn, htIn))
    figNum = figNum + 1
    
    ax1 = fig.add_subplot(2,1,1,xmargin=0.0)
    ax2 = fig.add_subplot(2,1,2,xmargin=0.0)
    
    ax1.plot(ctimes, VelSurf, \
             label='VelSurf', color='gray', linewidth=1)
    
    ax1.plot(ctimes, VelNoiseFiltMean, \
             label='VelNoiseFiltMean', color='blue', linewidth=2)
    
    ax1.plot(ctimes, VelNoiseFiltMedian, \
             label='VelNoiseFiltMedian', color='green', linewidth=2)
    
    #ax2.plot(ctimes, VelNoiseFiltMedian, \
    #         label='VelNoiseFiltMedian', color='red', linewidth=1)
    
    ax2.plot(ctimes, VelNoiseFilt, \
             label='VelNoiseFilt', color='gray', linewidth=1)
    
    ax2.plot(ctimes, VelWaveFiltMedian, \
             label='VelWaveFiltMedian', color='blue', linewidth=1)
    
    ax2.plot(ctimes, VelWaveFiltMean, \
             label='VelWaveFiltMean', color='green', linewidth=1)
    
    ax2.plot(ctimes, VelWaveFiltPoly, \
             label='VelWaveFiltPoly', color='red', linewidth=2)
    
    ax2.plot(ctimes, VertVel, \
             label='VertVel', color='cyan', linewidth=1)

    configTimeAxis(ax1, -2, 2, "Surf velocity raw (m/s)", 'upper center')
    configTimeAxis(ax2, -2, 2, "Surf velocity filtered (m/s)", 'upper center')

    fig.autofmt_xdate()
    fig.tight_layout()
    fig.subplots_adjust(bottom=0.08, left=0.06, right=0.97, top=0.96)

    fig.suptitle("RAW VELOCITY WITH FILTERING - file " + os.path.basename(options.filePath))

    return

########################################################################
# Plot filtered data plus georef data

def doPlotFiltAndGeoref():

    # set up plots

    widthIn = float(options.mainWidthMm) / 25.4
    htIn = float(options.mainHeightMm) / 25.4

    global figNum
    fig = plt.figure(figNum, (widthIn, htIn))
    figNum = figNum + 1
    
    ax1 = fig.add_subplot(4,1,1,xmargin=0.0)
    ax2 = fig.add_subplot(4,1,2,xmargin=0.0)
    ax3 = fig.add_subplot(4,1,3,xmargin=0.0)
    ax4 = fig.add_subplot(4,1,4,xmargin=0.0)
    
    ax1.plot(ctimes, VelNoiseFilt, \
             label='VelNoiseFilt', color='gray', linewidth=1)
    
    ax1.plot(ctimes, VelWaveFiltMedian, \
             label='VelWaveFiltMedian', color='blue', linewidth=1)
    
    ax1.plot(ctimes, VelWaveFiltMean, \
             label='VelWaveFiltMean', color='green', linewidth=1)
    
    ax1.plot(ctimes, VelWaveFiltPoly, \
             label='VelWaveFiltPoly', color='red', linewidth=2)
    
    ax1.plot(ctimes, VertVel, \
             label='VertVel', color='cyan', linewidth=2)
    
    ax2.plot(ctimes, Pitch, label='Pitch', color='red', linewidth=1)
    ax2.plot(ctimes, Elevation + 90.0, label='Elev+90', color='blue', linewidth=1)
    ax2.plot(ctimes, Tilt * -1, label='Tilt*-1', color='green', linewidth=1)

    ax3.plot(ctimes, Roll, label='Roll', color='red', linewidth=1)
    ax3.plot(ctimes, Rotation * -1.0 + 180.0, label='-Rotation+180', color='blue', linewidth=1)
    
    ax4.plot(ctimes, Altitude, label='Altitude', color='blue', linewidth=1)

    configTimeAxis(ax1, -2, 2, "Surface velocity (m/s)", 'upper center')
    configTimeAxis(ax2, -3, 3, "Pitch and Radar Elevation (deg)", 'upper center')
    configTimeAxis(ax3, -9999, -9999, "Roll and Radar Rotation (deg)", 'upper center')
    configTimeAxis(ax4, -9999, -9999, "Altitude (km)", 'upper center')

    fig.autofmt_xdate()
    fig.tight_layout()
    fig.subplots_adjust(bottom=0.08, left=0.06, right=0.97, top=0.96)

    fig.suptitle("FILTERING with GEOREF - file " + os.path.basename(options.filePath))

    return

########################################################################
# Plot INS and diffs

def doPlotPitchRoll(outFile):

    # set up plots

    widthIn = float(options.mainWidthMm) / 25.4
    htIn = float(options.mainHeightMm) / 25.4
    
    global figNum
    fig = plt.figure(figNum, (widthIn, htIn))
    figNum = figNum + 1
    
    ax1 = fig.add_subplot(4,1,1,xmargin=0.0)
    ax2 = fig.add_subplot(4,1,2,xmargin=0.0, sharex=ax1)
    ax3 = fig.add_subplot(4,1,3,xmargin=0.0, sharex=ax1)
    ax4 = fig.add_subplot(4,1,4,xmargin=0.0, sharex=ax1)
    
    ax1.plot(ctimes, pitch3Sm, label='PitchIns3', color='orange', linewidth=1)
    ax1.plot(ctimes, pitch2Sm, label='PitchIns2', color='blue', linewidth=1)
    ax1.plot(ctimes, pitchSm, label='PitchIns1', color='green', linewidth=1)
    ax1.plot(ctimes, pitchCmSm, label='PitchCmigits', color='red', linewidth=1)
    
    ax2.plot(ctimes, roll3Sm, label='RollIns3', color='orange', linewidth=1)
    ax2.plot(ctimes, roll2Sm, label='RollIns2', color='green', linewidth=1)
    ax2.plot(ctimes, rollSm, label='RollIns1', color='blue', linewidth=1)
    ax2.plot(ctimes, rollCmSm, label='RollCmigits', color='red', linewidth=1)
    
    ax3.plot(ctimes, pitchDiffCm2Sm, \
             label='pitchDiffCm2Sm', color='magenta', linewidth=1)
    ax3.plot(ctimes, estPitchDiffSm, \
             label='estPitchDiff', color='blue', linewidth=1)
    ax3.plot(ctimes, pitchDiffSm, \
             label='pitchDiffCmigits', color='green', linewidth=1)
    ax3.plot(ctimes, pitchDiff2Sm, \
             label='pitchDiffIns2', color='red', linewidth=1)
    ax3.plot(ctimes, pitchDiff3Sm, \
             label='pitchDiffIns3', color='black', linewidth=1)


    ax4.plot(ctimes, rollDiffSm, \
             label='rollDiffCmigits', color='blue', linewidth=1)
    ax4.plot(ctimes, rollDiff2Sm, \
             label='rollDiffIns2', color='red', linewidth=1)
    ax4.plot(ctimes, rollDiff3Sm, \
             label='rollDiffIns3', color='black', linewidth=1)

    configTimeAxis(ax1, -3, 10, "Pitch", 'upper right')
    configTimeAxis(ax2, -10, 10, "Roll", 'upper right')
    configTimeAxis(ax3, -0.5, 1, "PitchDiffs", 'upper right')
    configTimeAxis(ax4, -1, 1, "RollDiffs", 'upper right')

    fig.autofmt_xdate()
    fig.tight_layout()
    fig.subplots_adjust(bottom=0.08, left=0.06, right=0.97, top=0.96)

    # title name

    fig.suptitle("ROLL and PITCH - file " + os.path.basename(options.filePath))
    
    plt.savefig(outFile + '.rollPitch.png')

    return

########################################################################
# Plot the diffs

def doPlotDiffs(outFile):

    # set up plots

    widthIn = float(options.mainWidthMm) / 25.4
    htIn = float(options.mainHeightMm) / 25.4
    
    global figNum
    fig = plt.figure(figNum, (widthIn, htIn))
    figNum = figNum + 1
    
    ax1 = fig.add_subplot(4,1,1,xmargin=0.0)
    ax2 = fig.add_subplot(4,1,2,xmargin=0.0, sharex=ax1)
    ax3 = fig.add_subplot(4,1,3,xmargin=0.0, sharex=ax1)
    ax4 = fig.add_subplot(4,1,4,xmargin=0.0, sharex=ax1)
    
    ax1.plot(ctimes, driftDiffSm, \
             label='driftDiff', color='black', linewidth=1)
    ax1.plot(ctimes, hdgDiffSm, \
             label='hdgDiff', color='blue', linewidth=1)
    ax1.plot(ctimes, vVelDiffSm, \
             label='vVelDiff', color='green', linewidth=1)
    ax1.plot(ctimes, altDiff, \
             label='altDiff', color='red', linewidth=2)

    ax2.plot(ctimes, estPitchDiffSm, \
             label='estPitchDiff', color='black', linewidth=1)
    ax2.plot(ctimes, altGps / 1000.0, \
             label='Alt(km)', color='gray', linewidth=2)
    #ax2.plot(ctimes, gvMass10000Kg, \
    #         label='Mass (10T)', color='brown', linewidth=1)
    ax2.plot(ctimes, accelNormSm, \
             label='accelNorm', color='orange', linewidth=1)
    ax2.plot(ctimes, aoa2, \
             label='aoa/2', color='red', linewidth=1)
    ax2.plot(ctimes, pitchDiffSm, \
             label='pitchDiff', color='green', linewidth=1)
    ax2.plot(ctimes, rollDiffSm, \
             label='rollDiff', color='blue', linewidth=1)
    ax2.plot(ctimes, temp30OffsetBy10, label='TempOffsetBy10', color='magenta', linewidth=1)
    

    ax3.plot(ctimes, surfaceVelSm, \
             label='surfaceVel', color='blue', linewidth=1)

    ax4.plot(ctimes, elevErrSm, label='ElevErr', color='red', linewidth=1)
    ax4.plot(ctimes, tiltErrSm, label='TiltErr', color='blue', linewidth=1)


    configTimeAxis(ax1, -3, 3.0, "diffs", 'upper right')
    # configTimeAxis(ax2, -0.5, 3, "diffs", 'upper right')
    configTimeAxis(ax2, -3, 8, "diffs", 'upper right')
    configTimeAxis(ax3, -9999, -9999, "SurfaceVel", 'upper right')
    configTimeAxis(ax4, -0.25, 0.25, "Err", 'upper right')
    
    fig.autofmt_xdate()
    fig.tight_layout()
    fig.subplots_adjust(bottom=0.08, left=0.06, right=0.97, top=0.96)

    fig.suptitle("DIFFS GV minus CMIGITS - file " + os.path.basename(options.filePath))
    
    plt.savefig(outFile + '.diffs.png')

    return

########################################################################
# Produce the 2-D histograms

def doPlot2DHist():

    # set up plots

    widthIn = float(options.mainWidthMm) / 25.4
    htIn = float(options.mainHeightMm) / 25.4
    
    global figNum
    fig = plt.figure(figNum, (widthIn, htIn))
    figNum = figNum + 1
    
    ax1 = fig.add_subplot(2,2,1,xmargin=0.0)
    ax2 = fig.add_subplot(2,2,2,xmargin=0.0)
    ax3 = fig.add_subplot(2,2,3,xmargin=0.0)
    ax4 = fig.add_subplot(2,2,4,xmargin=0.0)

    # pitch offset vs G force

    validAccelNorm = accelNormSm[np.isfinite(accelNormSm)]
    validPitchDiff = pitchDiffSm[np.isfinite(pitchDiffSm)]
    validRollDiff = rollDiffSm[np.isfinite(rollDiffSm)]
    validAoa = aoaSm[np.isfinite(aoaSm)]
    validTemp30Offset = temp30OffsetSm[np.isfinite(temp30OffsetSm)]

    (counts1, xedges1, yedges1, Image1) = ax1.hist2d(validAccelNorm, validPitchDiff, 
                                                     bins=100, norm=mpl.colors.LogNorm())
    cbar1 = fig.colorbar(mappable=Image1, ax=ax1)
    cbar1.set_label("count")
    ax1.set_xlabel('G-acceleration normal')
    ax1.set_ylabel('Pitch offset')

    # roll offset vs G force

    (counts2, xedges2, yedges2, Image2) = ax2.hist2d(validAccelNorm, validRollDiff,
                                                     bins=100, norm=mpl.colors.LogNorm())
    cbar2 = fig.colorbar(mappable=Image2, ax=ax2)
    cbar2.set_label("count")
    ax2.set_xlabel('G-acceleration normal')
    ax2.set_ylabel('Roll offset')

    # pitch offset vs temp diff

    (counts3, xedges3, yedges3, Image3) = ax3.hist2d(validTemp30Offset, validPitchDiff, 
                                                     bins=100, norm=mpl.colors.LogNorm())
    cbar3 = fig.colorbar(mappable=Image3, ax=ax3)
    cbar3.set_label("count")
    ax3.set_xlabel('30 - cmigits temp')
    ax3.set_ylabel('Pitch offset')

    # roll offset vs Angle of attack

    (counts4, xedges4, yedges4, Image4) = ax4.hist2d(validAoa, validRollDiff, 
                                                     bins=100, norm=mpl.colors.LogNorm())
    cbar4 = fig.colorbar(mappable=Image4, ax=ax4)
    cbar4.set_label("count")
    ax4.set_xlabel('AOA')
    ax4.set_ylabel('Roll offset')

    fig.tight_layout()
    fig.subplots_adjust(bottom=0.08, left=0.06, right=0.97, top=0.96)

    fig.suptitle("2D histograms - file " + os.path.basename(options.filePath))

    return

########################################################################
# Plot angles

def doPlotRadarAngles():

    # set up plots

    widthIn = float(options.mainWidthMm) / 25.4
    htIn = float(options.mainHeightMm) / 25.4
    
    global figNum
    fig = plt.figure(figNum, (widthIn, htIn))
    figNum = figNum + 1
    
    ax1 = fig.add_subplot(4,1,1,xmargin=0.0)
    ax2 = fig.add_subplot(4,1,2,xmargin=0.0)
    ax3 = fig.add_subplot(4,1,3,xmargin=0.0)
    ax4 = fig.add_subplot(4,1,4,xmargin=0.0)
    
    ax1.plot(ctimes, azimuth, label='Azimuth', color='red', linewidth=1)

    ax2.plot(ctimes, elevErrSm, label='ElevErr', color='blue', linewidth=1)

    ax3.plot(ctimes, rotation, label='Rotation', color='green', linewidth=1)

    ax4.plot(ctimes, tilt, label='Tilt', color='black', linewidth=1)

    configTimeAxis(ax1, -9999, -9999, "Azimuth", 'upper right')
    configTimeAxis(ax2, -0.25, 0.25, "Elevation error", 'upper right')
    configTimeAxis(ax3, -9999, -9999, "Rotation", 'upper right')
    configTimeAxis(ax4, -9999, -9999, "Tilt", 'upper right')

    fig.autofmt_xdate()
    fig.tight_layout()
    fig.subplots_adjust(bottom=0.08, left=0.06, right=0.97, top=0.96)

    # title name

    fig.suptitle("RADAR ANGLES - file " + os.path.basename(options.filePath))

    return

########################################################################
# Plot the estimated pitch difference, using surface vel to estimate it

def doPlotEstPitchDiff(outFile):

    # set up plots

    widthIn = float(options.mainWidthMm) / 25.4
    htIn = float(options.mainHeightMm) / 25.4
    
    global figNum
    fig = plt.figure(figNum, (widthIn, htIn))
    figNum = figNum + 1
    
    ax1 = fig.add_subplot(2,1,1,xmargin=0.0)
    ax2 = fig.add_subplot(2,1,2,xmargin=0.0, sharex=ax1)
    
    ax1.plot(ctimes, estPitchDiffSm, \
             label='estPitchDiff', color='red', linewidth=1)
    ax1.plot(ctimes, pitchDiffSm, \
             label='pitchDiff', color='green', linewidth=1)
    ax1.plot(ctimes, rollDiffSm, \
             label='rollDiff', color='blue', linewidth=1)
    
    ax2.plot(ctimes, surfaceVelSm, \
             label='surfaceVel', color='blue', linewidth=1)

    configTimeAxis(ax1, -1, 1, "diffs", 'upper right')
    configTimeAxis(ax2, -1.5, 1, "SurfaceVel", 'upper right')
    
    fig.autofmt_xdate()
    fig.tight_layout()
    fig.subplots_adjust(bottom=0.08, left=0.06, right=0.97, top=0.96)

    fig.suptitle("ESTIMATED PITCH DIFF: GV-CMIGITS - file " + os.path.basename(options.filePath))
    
    plt.savefig(outFile + '.estPitchDiff.png')
    
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
        print("running cmd:",cmd, file=sys.stderr)
    
    try:
        retcode = subprocess.call(cmd, shell=True)
        if retcode < 0:
            print("Child was terminated by signal: ", -retcode, file=sys.stderr)
        else:
            if (options.debug == True):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, file=sys.stderr)

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()


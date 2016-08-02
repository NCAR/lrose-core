#!/usr/bin/env python

#===========================================================================
#
# Produce plots for GEOREF differences
#
#===========================================================================

import os
import sys
import subprocess
from optparse import OptionParser
import numpy as np
from numpy import convolve
from numpy import linalg, array, ones
import matplotlib.pyplot as plt
from matplotlib import dates
import math
import datetime
import contextlib

def main():

#   globals

    global options
    global debug
    global startTime
    global endTime

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
    parser.add_option('--comp_file',
                      dest='compFilePath',
                      default='./AcGeorefCompare.txt',
                      help='File path for comparison results')
    parser.add_option('--title',
                      dest='title',
                      default='GV minus HCR CMIGITS',
                      help='Title for plot')
    parser.add_option('--widthMain',
                      dest='mainWidthMm',
                      default=400,
                      help='Width of main figure in mm')
    parser.add_option('--heightMain',
                      dest='mainHeightMm',
                      default=300,
                      help='Height of main figure in mm')
    parser.add_option('--lenMean',
                      dest='lenMean',
                      default=21,
                      help='Len of moving mean filter')
    parser.add_option('--start',
                      dest='startTime',
                      default='1970 01 01 00 00 00',
                      help='Start time for XY plot')
    parser.add_option('--end',
                      dest='endTime',
                      default='1970 01 01 01 00 00',
                      help='End time for XY plot')
    
    (options, args) = parser.parse_args()
    
    if (options.verbose == True):
        options.debug = True

    year, month, day, hour, minute, sec = options.startTime.split()
    startTime = datetime.datetime(int(year), int(month), int(day),
                                  int(hour), int(minute), int(sec))

    year, month, day, hour, minute, sec = options.endTime.split()
    endTime = datetime.datetime(int(year), int(month), int(day),
                                int(hour), int(minute), int(sec))

    if (options.debug == True):
        print >>sys.stderr, "Running %prog"
        print >>sys.stderr, "  compFilePath: ", options.compFilePath
        print >>sys.stderr, "  startTime: ", startTime
        print >>sys.stderr, "  endTime: ", endTime

    # read in column headers for bias results

    iret, compHdrs, compData = readColumnHeaders(options.compFilePath)
    if (iret != 0):
        sys.exit(-1)

    # read in data for comp results

    compData, compTimes = readInputData(options.compFilePath, compHdrs, compData)

    # load up the data arrays

    loadDataArrays(compData, compTimes)

    # render the plots
    
    doPlotAircraft()
    doPlotDiffs()
    doPlotXy()

    # show them

    plt.tight_layout()
    plt.subplots_adjust(top=0.9)
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

        for index, var in enumerate(colHeaders, start=0):
            # print >>sys.stderr, "index, data[index]: ", index, ", ", data[index]
            if (var == 'count' or var == 'year' or var == 'month' or var == 'day' or \
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

def loadDataArrays(compData, compTimes):

    lenMeanFilter = int(options.lenMean)
    
    # set up arrays

    global ctimes

    ctimes = np.array(compTimes).astype(datetime.datetime)
    
    global tempAir, tempCmigits, tempTailcone
    global pressure, rh, density, altPres, altGps
    global vertVel, gvMassKg, gvMass10000Kg
    global aoa, aoaSm, aoa2, ias, tas

    tempAir = np.array(compData["tempAir"]).astype(np.double)
    tempCmigits = np.array(compData["tempCmigits"]).astype(np.double)
    tempTailcone = np.array(compData["tempTailcone"]).astype(np.double)
    pressure = np.array(compData["pressure"]).astype(np.double)
    rh = np.array(compData["rh"]).astype(np.double)
    density = np.array(compData["density"]).astype(np.double)
    altPres = np.array(compData["altPresM"]).astype(np.double)
    altGps = np.array(compData["altGpsM"]).astype(np.double)
    vertVel = np.array(compData["vertVel"]).astype(np.double)
    gvMassKg = np.array(compData["weightKg"]).astype(np.double)
    gvMass10000Kg = np.array(compData["weightKg"]).astype(np.double) / 10000.0
    aoa = np.array(compData["aoa"]).astype(np.double)
    aoaSm = movingAverage(aoa, lenMeanFilter)
    aoa2 = aoaSm / 2.0
    ias = np.array(compData["ias"]).astype(np.double)
    tas = np.array(compData["tas"]).astype(np.double)

    global accelNorm, accelNormSm, accelLat, accelLatSm, accelLon, accelLonSm

    accelNorm = np.array(compData["accelNorm"]).astype(np.double)
    accelNormSm = movingAverage(accelNorm, lenMeanFilter) + 1.0
    accelLat = np.array(compData["accelLat"]).astype(np.double)
    accelLatSm = movingAverage(accelLat, lenMeanFilter)
    accelLon = np.array(compData["accelLon"]).astype(np.double)
    accelLonSm = movingAverage(accelLon, lenMeanFilter)

    global altDiff, pitchDiff, rollDiff, trackDiff, hdgDiff, vVelDiff
    global pitchDiffSm, rollDiffSm, trackDiffSm, hdgDiffSm, vVelDiffSm
    global driftDiff, driftDiffSm

    altDiff = np.array(compData["altDiff"]).astype(np.double)
    altDiffSm = movingAverage(altDiff, lenMeanFilter)

    pitchDiff = np.array(compData["pitchDiff"]).astype(np.double)
    pitchDiffSm = movingAverage(pitchDiff, lenMeanFilter)

    rollDiff = np.array(compData["rollDiff"]).astype(np.double)
    rollDiffSm = movingAverage(rollDiff, lenMeanFilter)

    trackDiff = np.array(compData["trackDiff"]).astype(np.double)
    trackDiffSm = movingAverage(trackDiff, lenMeanFilter)

    hdgDiff = np.array(compData["hdgDiff"]).astype(np.double)
    hdgDiffSm = movingAverage(hdgDiff, lenMeanFilter)

    driftDiff = np.array(compData["driftDiff"]).astype(np.double)
    driftDiffSm = movingAverage(driftDiff, lenMeanFilter)

    vVelDiff = np.array(compData["vertVelDiff"]).astype(np.double)
    vVelDiffSm = movingAverage(vVelDiff, lenMeanFilter)
    
########################################################################
# Plot aircraft variables

def doPlotAircraft():

    # title name

    fileName = options.compFilePath
    titleStr = "Aircraft state, file: " + fileName
    hfmt = dates.DateFormatter('%y/%m/%d')
    
    lenMeanFilter = int(options.lenMean)
    
    # set up plots

    widthIn = float(options.mainWidthMm) / 25.4
    htIn = float(options.mainHeightMm) / 25.4
    
    fig = plt.figure(1, (widthIn, htIn))
    
    ax1 = fig.add_subplot(5,1,1,xmargin=0.0)
    ax2 = fig.add_subplot(5,1,2,xmargin=0.0)
    ax3 = fig.add_subplot(5,1,3,xmargin=0.0)
    ax4 = fig.add_subplot(5,1,4,xmargin=0.0)
    ax5 = fig.add_subplot(5,1,5,xmargin=0.0)
    
    ax1.plot(ctimes, altPres, \
             label='Pressure altitude (m)', color='blue', linewidth=1)
    
    #ax2.plot(ctimes, density, \
    #         label='Air density (kg/m3)', color='red', linewidth=1)

    ax2.plot(ctimes, tempAir, \
             label='Air temp (C)', color='red', linewidth=1)
    ax2.plot(ctimes, tempCmigits, \
             label='Cmigits temp (C)', color='blue', linewidth=1)
    ax2.plot(ctimes, tempTailcone, \
             label='Tailcone temp (C)', color='green', linewidth=1)
    
    #ax3.plot(ctimes, tas, \
    #         label='True Airspeed', color='red', linewidth=1)
    ax3.plot(ctimes, ias, \
             label='Indicated Airspeed', color='green', linewidth=1)

    ax4.plot(ctimes, gvMass10000Kg, \
             label='GV mass (10000 Kg)', color='green', linewidth=2)
    ax4.plot(ctimes, aoa, \
             label='Angle of attack', color='red', linewidth=1)

    #ax4.plot(ctimes, vertVel, \
    #         label='vertVel', color='blue', linewidth=1)

    ax5.plot(ctimes, accelLat, \
             label='accelLat', color='blue', linewidth=1)
    ax5.plot(ctimes, accelLon, \
             label='accelLon', color='green', linewidth=1)
    ax5.plot(ctimes, accelNormSm, \
             label='accelNorm', color='orange', linewidth=1)


    configDateAxis(ax1, -9999, -9999, "Altitude", 'lower right')
    configDateAxis(ax2, -50, 50, "Temp (C)", 'lower center')
    #configDateAxis(ax2, -9999, -9999, "Density (kg/m3)", 'upper right')
    #configDateAxis(ax3, 100, 300, "Airspeed (m/s)", 'upper center')
    configDateAxis(ax3, 100, 200, "Airspeed (m/s)", 'upper center')
    configDateAxis(ax4, 1, 5, "Angle of Attack", 'upper right')
    configDateAxis(ax5, -9999, -9999, "G force", 'upper right')
    
    fig.autofmt_xdate()
    fig.tight_layout()
    fig.subplots_adjust(bottom=0.08, left=0.06, right=0.97, top=0.96)

    fig.suptitle("FLIGHT SUMMARY - file " + fileName)

    return

########################################################################
# Plot the diffs

def doPlotDiffs():

    fileName = options.compFilePath
    titleStr = "File: " + fileName
    hfmt = dates.DateFormatter('%y/%m/%d')
    
    lenMeanFilter = int(options.lenMean)
    
    # set up plots

    widthIn = float(options.mainWidthMm) / 25.4
    htIn = float(options.mainHeightMm) / 25.4
    
    fig = plt.figure(2, (widthIn, htIn))
    
    ax1 = fig.add_subplot(2,1,2,xmargin=0.0)
    ax2 = fig.add_subplot(2,1,1,xmargin=0.0)
    
    ax1.plot(ctimes, altGps / 1000.0, \
             label='Alt(km)', color='gray', linewidth=2)
    #ax1.plot(ctimes, gvMass10000Kg, \
    #         label='Mass (10T)', color='cyan', linewidth=1)

    ax1.plot(ctimes, accelNormSm, \
             label='accelNorm', color='orange', linewidth=1)
    ax1.plot(ctimes, aoa2, \
              label='aoa/2', color='red', linewidth=1)
    ax1.plot(ctimes, pitchDiffSm, \
              label='pitchDiff', color='green', linewidth=1)
    ax1.plot(ctimes, rollDiffSm, \
              label='rollDiff', color='blue', linewidth=1)


    ax2.plot(ctimes, driftDiffSm, \
             label='driftDiff', color='black', linewidth=1)
    ax2.plot(ctimes, hdgDiffSm, \
             label='hdgDiff', color='blue', linewidth=1)
    ax2.plot(ctimes, vVelDiffSm, \
             label='vVelDiff', color='green', linewidth=1)
    ax2.plot(ctimes, altDiff, \
             label='altDiff', color='red', linewidth=2)

    configDateAxis(ax1, -0.5, 3.0, "diffs", 'upper right')
    configDateAxis(ax2, -3, 3, "diffs", 'upper right')
    #configDateAxis(ax1, -9999, -9999, "diffs", 'upper right')
    #configDateAxis(ax2, -9999, -9999, "diffs", 'upper right')
    
    fig.autofmt_xdate()
    fig.tight_layout()
    fig.subplots_adjust(bottom=0.08, left=0.06, right=0.97, top=0.96)

    fig.suptitle("DIFFS GV minus CMIGITS - file " + fileName)

    return

########################################################################
# Produce the XY plots

def doPlotXy():

    fileName = options.compFilePath
    titleStr = "File: " + fileName
    hfmt = dates.DateFormatter('%y/%m/%d')
    
    lenMeanFilter = int(options.lenMean)
    
    # set up plots

    widthIn = float(options.mainWidthMm) / 25.4
    htIn = float(options.mainHeightMm) / 25.4
    
    fig = plt.figure(3, (widthIn, htIn))
    
    ax1 = fig.add_subplot(2,2,1,xmargin=0.0)
    ax2 = fig.add_subplot(2,2,2,xmargin=0.0)
    ax3 = fig.add_subplot(2,2,3,xmargin=0.0)
    ax4 = fig.add_subplot(2,2,4,xmargin=0.0)
    
    ax1.plot(gvMass10000Kg, rollDiff, \
             'x', label='mass vs rollDiff', color='red')
    ax1.set_title('rollDiff vs Mass')
    ax1.set_xlabel('mass')

    #ax3.plot(aoa, pitchDiff, \
    #         'x', label='pitchDiff vs AOA', color='red')
    #ax3.set_title('pitchDiff vs aoa')
    #ax3.set_xlabel('AOA')
    #ax3.set_ylabel('pitchDiff')

    ax3.plot(gvMass10000Kg, pitchDiff, \
             'x', label='pitchDiff vs mass', color='red')
    ax3.set_title('pitchDiff vs massaoa')
    ax3.set_xlabel('Mass')
    ax3.set_ylabel('pitchDiff')

    ax2.plot(aoa, rollDiff, \
             'x', label='aoa vs rollDiff', color='green')
    ax2.set_title('rollDiff vs aoa')
    ax2.set_xlabel('AOA')

    ax4.plot(accelNorm, pitchDiff, \
             'x', label='pitchDiff vs accelNorm', color='green')
    ax4.set_title('pitchDiff vs accelNorm')
    ax4.set_xlabel('accelNorm')
    ax4.set_ylabel('pitchDiff')

    #ax1.plot(ctimes, aoa2, \
    #          label='aoa/2', color='green', linewidth=1)
    #ax1.plot(ctimes, pitchDiff, \
    #          label='pitchDiff', color='red', linewidth=1)
    #ax1.plot(ctimes, rollDiff, \
    #          label='rollDiff', color='blue', linewidth=1)

    #ax2.plot(ctimes, hdgDiff, \
    #         label='hdgDiff', color='blue', linewidth=1)
    #ax2.plot(ctimes, vVelDiff, \
    #         label='vVelDiff', color='green', linewidth=1)
    #ax2.plot(ctimes, altDiff, \
    #         label='altDiff', color='red', linewidth=2)

    #configDateAxis(ax1, -0.5, 2.0, "diffs", 'upper right')
    #configDateAxis(ax2, -1, 3, "diffs", 'upper right')
    #configDateAxis(ax1, -9999, -9999, "diffs", 'upper right')
    #configDateAxis(ax2, -9999, -9999, "diffs", 'upper right')
    
    fig.autofmt_xdate()
    fig.tight_layout()
    fig.subplots_adjust(bottom=0.08, left=0.06, right=0.97, top=0.96)

    fig.suptitle("DIFFS GV minus CMIGITS - file " + fileName)

    return

    #ax3 = fig1.add_subplot(3,1,3,xmargin=0.0)

    if (haveTemps):
        fig2 = plt.figure(2, (widthIn/2, htIn/2))
        ax2a = fig2.add_subplot(1,1,1,xmargin=1.0, ymargin=1.0)

    oneDay = datetime.timedelta(1.0)
    ax1.set_xlim([ctimes[0] - oneDay, ctimes[-1] + oneDay])
    ax1.set_title("Residual ZDR comp in ice and Bragg, compared with VERT and CP results (dB)")
    ax2.set_xlim([ctimes[0] - oneDay, ctimes[-1] + oneDay])
    ax2.set_title("Daily mean ZDR comp in ice and Bragg (dB)")
    #ax3.set_xlim([ctimes[0] - oneDay, ctimes[-1] + oneDay])
    #ax3.set_title("Site temperature (C)")

    ax1.plot(validBraggCtimes, validBraggVals, \
              "o", label = 'ZDR Comp In Bragg', color='blue')
    ax1.plot(validBraggCtimes, validBraggVals, \
              label = 'ZDR Comp In Bragg', linewidth=1, color='blue')
    
    #compIceM = np.array(compData["ZdrmInIcePerc25.00"]).astype(np.double)
    #compIceM = movingAverage(compIceM, lenMeanFilter)
    #validIceM = np.isfinite(compIceM)
    
    # compBragg = np.array(compData["ZdrInBraggMean"]).astype(np.double)
    # compBragg = movingAverage(compBragg, lenMeanFilter)

    # site temp, vert pointing and sun scan results

    ZdrmVert = np.array(cpData["ZdrmVert"]).astype(np.double)
    validZdrmVert = np.isfinite(ZdrmVert)
    
    SunscanZdrm = np.array(cpData["SunscanZdrm"]).astype(np.double)
    validSunscanZdrm = np.isfinite(SunscanZdrm)

    cptimes = np.array(cpTimes).astype(datetime.datetime)
    tempSite = np.array(cpData["TempSite"]).astype(np.double)
    validTempSite = np.isfinite(tempSite)

    tempIceVals = []
    compIceVals = []

    for ii, compVal in enumerate(validIceVals, start=0):
        btime = validIceCtimes[ii]
        if (btime >= startTime and btime <= endTime):
            tempTime, tempVal = getClosestTemp(btime, cptimes, tempSite)
            if (np.isfinite(tempVal)):
                tempIceVals.append(tempVal)
                compIceVals.append(compVal)
                if (options.verbose):
                    print >>sys.stderr, "==>> compTime, compVal, tempTime, tempVal:", \
                        btime, compVal, tempTime, tempVal

    # linear regression for comp vs temp

    A = array([tempIceVals, ones(len(tempIceVals))])

    if (len(tempIceVals) > 1):
        # obtain the fit, ww[0] is slope, ww[1] is intercept
        ww = linalg.lstsq(A.T, compIceVals)[0]
        minTemp = min(tempIceVals)
        maxTemp = max(tempIceVals)
        haveTemps = True
    else:
        ww = (1.0, 0.0)
        minTemp = 0.0
        maxTemp = 40.0
        haveTemps = False
        print >>sys.stderr, "NOTE - no valid temp vs ZDR data for period"
        print >>sys.stderr, "  startTime: ", startTime
        print >>sys.stderr, "  endTime  : ", endTime
        
    regrX = []
    regrY = []
    regrX.append(minTemp)
    regrX.append(maxTemp)
    regrY.append(ww[0] * minTemp + ww[1])
    regrY.append(ww[0] * maxTemp + ww[1])
    
    # set up plots

    widthIn = float(options.mainWidthMm) / 25.4
    htIn = float(options.mainHeightMm) / 25.4

    fig1 = plt.figure(1, (widthIn, htIn))

    ax1 = fig1.add_subplot(2,1,1,xmargin=0.0)
    ax2 = fig1.add_subplot(2,1,2,xmargin=0.0)
    #ax3 = fig1.add_subplot(3,1,3,xmargin=0.0)

    if (haveTemps):
        fig2 = plt.figure(2, (widthIn/2, htIn/2))
        ax2a = fig2.add_subplot(1,1,1,xmargin=1.0, ymargin=1.0)

    oneDay = datetime.timedelta(1.0)
    ax1.set_xlim([ctimes[0] - oneDay, ctimes[-1] + oneDay])
    ax1.set_title("Residual ZDR comp in ice and Bragg, compared with VERT and CP results (dB)")
    ax2.set_xlim([ctimes[0] - oneDay, ctimes[-1] + oneDay])
    ax2.set_title("Daily mean ZDR comp in ice and Bragg (dB)")
    #ax3.set_xlim([ctimes[0] - oneDay, ctimes[-1] + oneDay])
    #ax3.set_title("Site temperature (C)")

    ax1.plot(validBraggCtimes, validBraggVals, \
              "o", label = 'ZDR Comp In Bragg', color='blue')
    ax1.plot(validBraggCtimes, validBraggVals, \
              label = 'ZDR Comp In Bragg', linewidth=1, color='blue')
    
    ax1.plot(validIceCtimes, validIceVals, \
              "o", label = 'ZDR Comp In Ice', color='red')
    ax1.plot(validIceCtimes, validIceVals, \
              label = 'ZDR Comp In Ice', linewidth=1, color='red')

    ax1.plot(validBraggMCtimes, validBraggMVals, \
              "o", label = 'ZDRM Comp In Bragg', color='blue')
    ax1.plot(validBraggMCtimes, validBraggMVals, \
              label = 'ZDRM Comp In Bragg', linewidth=1, color='blue')
    
    ax1.plot(validIceMCtimes, validIceMVals, \
              "o", label = 'ZDRM Comp In Ice', color='red')
    ax1.plot(validIceMCtimes, validIceMVals, \
              label = 'ZDRM Comp In Ice', linewidth=1, color='red')
    
    #ax1.plot(ctimes[validSunscanZdrm], SunscanZdrm[validSunscanZdrm], \
    #          linewidth=2, label = 'Zdrm Sun/CP (dB)', color = 'green')
    
    #ax1.plot(ctimes[validZdrmVert], ZdrmVert[validZdrmVert], \
    #          "^", markersize=10, linewidth=1, label = 'Zdrm Vert (dB)', color = 'orange')

    ax2.plot(dailyTimeBragg, dailyValBragg, \
              label = 'Daily Comp Bragg', linewidth=1, color='blue')
    ax2.plot(dailyTimeBragg, dailyValBragg, \
              "^", label = 'Daily Comp Bragg', color='blue', markersize=10)

    ax2.plot(dailyTimeIce, dailyValIce, \
              label = 'Daily Comp Ice', linewidth=1, color='red')
    ax2.plot(dailyTimeIce, dailyValIce, \
              "^", label = 'Daily Comp Ice', color='red', markersize=10)

    #ax3.plot(cptimes[validTempSite], tempSite[validTempSite], \
    #          linewidth=1, label = 'Site Temp', color = 'blue')
    
    #configDateAxis(ax1, -9999, -9999, "ZDR Comp (dB)", 'upper right')
    configDateAxis(ax1, -0.3, 0.7, "ZDR Comp (dB)", 'upper right')
    configDateAxis(ax2, -0.5, 0.5, "ZDR Comp (dB)", 'upper right')
    #configDateAxis(ax3, -9999, -9999, "Temp (C)", 'upper right')

    if (haveTemps):
        label3 = "ZDR Comp In Ice = " + ("%.5f" % ww[0]) + " * temp + " + ("%.3f" % ww[1])
        ax2a.plot(tempIceVals, compIceVals, 
                 "x", label = label3, color = 'blue')
        ax2a.plot(regrX, regrY, linewidth=3, color = 'blue')
    
        legend3 = ax2a.legend(loc="upper left", ncol=2)
        for label3 in legend3.get_texts():
            label3.set_fontsize(12)
            ax2a.set_xlabel("Site temperature (C)")
            ax2a.set_ylabel("ZDR Comp (dB)")
            ax2a.grid(True)
            ax2a.set_ylim([-0.5, 0.5])
            ax2a.set_xlim([minTemp - 1, maxTemp + 1])
            title3 = "ZDR Comp In Ice Vs Temp: " + str(startTime) + " - " + str(endTime)
            ax2a.set_title(title3)


########################################################################
# initialize legends etc

def configDateAxis(ax, miny, maxy, ylabel, legendLoc):
    
    legend = ax.legend(loc=legendLoc, ncol=5)
    for label in legend.get_texts():
        label.set_fontsize('x-small')
        ax.set_xlabel("Time")
    ax.set_ylabel(ylabel)
    ax.grid(True)
    if (miny > -9990 and maxy > -9990):
        ax.set_ylim([miny, maxy])
    hfmt = dates.DateFormatter('%H:%M:%S')
    ax.xaxis.set_major_locator(dates.HourLocator())
    ax.xaxis.set_major_formatter(hfmt)
    for tick in ax.xaxis.get_major_ticks():
        tick.label.set_fontsize(8) 

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


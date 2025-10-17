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
    parser.add_option('--width',
                      dest='figWidthMm',
                      default=200,
                      help='Width of figure in mm')
    parser.add_option('--height',
                      dest='figHeightMm',
                      default=100,
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
    parser.add_option('--sector',
                      dest='sector',
                      default=0.0,
                      help='Sector width (deg), 0 for stationary, 360 for surveillance')
    
    (options, args) = parser.parse_args()
    
    if (options.verbose):
        options.debug = True

    global peakPowerW, g0, efficiency, diameter, prf, tau, freq, wavelength

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
    wavelength = 2.99792458e8 / freq

    global beamWidthDeg
    beamWidthDeg = math.degrees(1.27 * wavelength / diameter)

    global sectorWidthDeg, stationary, ppi, sector
    sectorWidthDeg = float(options.sector)
    stationary = True
    ppi = False
    sector = False

    if (sectorWidthDeg > 359.0):
        stationary = False
        ppi = True
        sectorWidthDeg = 360.0
    elif (sectorWidthDeg > 0.0):
        stationary = False
        sector = True

    global r1, r2, Snf, Sffr1, Sffr2
    
    r1 = (diameter * diameter) / (4.0 * wavelength)
    r2 = 0.6 * (diameter * diameter) / wavelength

    Snf = (16.0 * tau * prf * peakPowerW * efficiency) / (math.pi * diameter * diameter)

    Sffr1 = (g0 * tau * prf * peakPowerW) / (4.0 * math.pi * r1 * r1)
    Sffr2 = (g0 * tau * prf * peakPowerW) / (4.0 * math.pi * r2 * r2)

    print("Running " + thisScriptName, file=sys.stdout)
    print("  radar name: " + options.radarName, file=sys.stdout)
    print(f"  peak power W: {peakPowerW:.3g}", file=sys.stdout)
    print(f"  antenna gain (dB) {g0Db:.2f}", file=sys.stdout)
    print(f"  antenna gain (linear) {g0:.2f}", file=sys.stdout)
    print(f"  antenna efficiency: {efficiency:.3f}", file=sys.stdout)
    print(f"  antenna diameter (m) {diameter:.2f}", file=sys.stdout)
    print(f"  beamWidth (deg) {beamWidthDeg:.2f}", file=sys.stdout)
    print(f"  PRF (s-1) {prf:.2f}", file=sys.stdout)
    print(f"  pulse length (s) {tau:.3g}", file=sys.stdout)
    print(f"  transmit freq (Hz) {freq:.3g}", file=sys.stdout)
    print(f"  wavelength (m) {wavelength:.3g}", file=sys.stdout)
    print(f"  r1 (m) {r1:.3g}", file=sys.stdout)
    print(f"  r2 (m) {r2:.3g}", file=sys.stdout)
    print(f"  Snf (W/m2) {Snf:.2g}", file=sys.stdout)
    # print(f"  Sffr1 (W/m2) {Sffr1:.2g}", file=sys.stdout)
    print(f"  Sffr2 (W/m2) {Sffr2:.2g}", file=sys.stdout)

    # plot results
    
    doPlot()

    sys.exit(0)
    
########################################################################
# Plot

def doPlot():
    
    widthIn = float(options.figWidthMm) / 25.4
    htIn = float(options.figHeightMm) / 25.4

    fig1 = plt.figure(1, (widthIn, htIn))
    ax1 = fig1.add_subplot(1,1,1,xmargin=0.0)

    ax1.clear()

    # set up array of distances from antenna in meters
    
    maxRange = int(r2 * 2.0) + 1.0
    if (maxRange < 50):
        maxRange = 50.0
    dists = np.arange(maxRange, dtype=float)

    # compute power density at each distance from the antenna
    
    pwrDens = np.arange(maxRange, dtype=float)
    for ii in range(0, len(dists)):
        dist = dists[ii]
        if (dist <= r1):
            # near field
            pwrDens[ii] = Snf
        elif (dist >= r2):
            # far field
            pwrDens[ii] = (g0 * tau * prf * peakPowerW) / (4.0 * math.pi * dist * dist)
        else:
            # transition - interpolate
            pwrDens[ii] = Snf + ((dist - r1) / (r2 - r1)) * (Sffr2 - Snf)

    if (not stationary):
        # correct for rotation exposure duty cycle
        for ii in range(1, len(pwrDens)):
            dist = dists[ii]
            arc360 = 2.0 * math.pi * dist
            arcLen = arc360 * (sectorWidthDeg / 360.0)
            duty = 1.0
            dutyNear = diameter / arcLen
            if (dutyNear > 1.0):
                dutyNear = 1.0
            dutyFar = beamWidthDeg / sectorWidthDeg
            if (dist <= r1):
                # duty is diameter/arcLen
                duty = dutyNear
                # print("  near ii, dist, duty: ", ii, ", ", dists[ii], ", ", duty)
            elif (dist >= r2):
                # duty is beamwidth/sectorWidth
                duty = dutyFar
                # print("  far ii, dist, duty: ", ii, ", ", dists[ii], ", ", duty)
            else:
                # transition - interpolate
                duty = dutyNear + ((dist - r1) / (r2 - r1)) * (dutyFar - dutyNear)
                # print("  trans ii, dist, duty: ", ii, ", ", dists[ii], ", ", duty)
            # adjust power density by duty cycle
            pwrDens[ii] = pwrDens[ii] * duty

    # for ii in range(0, len(dists)):
    #     print("  ii, dist, pwrDens: ", ii, ", ", dists[ii], ", ", pwrDens[ii])

    # scale the y axis
    
    maxy = max(pwrDens) * 1.1
    maxy = max(maxy, 60)
    ax1.set_ylim(0, maxy)
    
    # plot power density

    ax1.plot(dists, pwrDens, linestyle = "-", label = 'pwrDens', color = 'blue')

    # plot safety thresholds

    ax1.axhline(y=10, color='green', linestyle='--', linewidth=2)
    ax1.text(x=1, y=12, s='Public threshold = 10  ', color='green',
             ha='right', transform=ax1.get_yaxis_transform())

    ax1.axhline(y=50, color='red', linestyle='--', linewidth=2)
    ax1.text(x=1, y=52, s='Occupational threshold = 50  ', color='red',
             ha='right', transform=ax1.get_yaxis_transform())
    
    ax1.set_xlabel("Range from antenna (m)")
    ax1.set_ylabel("Time-mean RF power density (W/m2)")
    ax1.grid(True)

    titleStr = options.radarName + " RF density plot"
    if (stationary):
        titleStr = titleStr + " - stationary antenna, " + "PRF: {:.0f}".format(prf)
    elif (ppi):
        titleStr = titleStr + " - rotating 360 deg ppi, " + "PRF: {:.0f}".format(prf)
    elif (sector):
        titleStr = titleStr + " - rotating " + "{:.0f}".format(sectorWidthDeg) + " deg sector, " + "PRF: {:.0f}".format(prf)
    fig1.suptitle(titleStr)

    plt.tight_layout()
    plt.subplots_adjust(top=0.9)

    # save the plot to a file

    os.makedirs(os.path.expanduser("~/Downloads"), exist_ok=True)
    savePath = os.environ["HOME"] + "/Downloads/RF_field_strength."
    savePath = savePath + options.radarName
    if (stationary):
        savePath = savePath + ".stationary"
    elif (ppi):
        savePath = savePath + ".ppi_360"
    elif (sector):
        savePath = savePath + ".sector_" + "{:.0f}".format(sectorWidthDeg)
    savePath = savePath + ".prf_{:.0f}".format(prf)
    savePath = savePath + ".png"
    plt.savefig(savePath)
    print("==>> Saved fig: ", savePath)

    plt.show()

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
# Run - entry point

if __name__ == "__main__":
   main()


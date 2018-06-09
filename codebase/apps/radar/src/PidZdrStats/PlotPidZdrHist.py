#!/usr/bin/env python

#===========================================================================
#
# Produce histogram for ZDR for various PID types
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

def main():

#   globals

    global options

# parse the command line

    usage = "usage: %prog [options]"
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--file',
                      dest='file',
                      default='/tmp/PidZdrStats/20150626/zdr_stats.20150626_000423.dry_snow.txt',
                      help='File path for zdr in ice data')
    parser.add_option('--title',
                      dest='title',
                      default='ZDR DISTRIBUTION',
                      help='Title for plot')
    parser.add_option('--width',
                      dest='figWidthMm',
                      default=300,
                      help='Width of figure in mm')
    parser.add_option('--height',
                      dest='figHeightMm',
                      default=300,
                      help='Height of figure in mm')
    parser.add_option('--minElev',
                      dest='minElev',
                      default='0.0',
                      help='Min elevation for ZDR data')
    parser.add_option('--maxElev',
                      dest='maxElev',
                      default='90.0',
                      help='Max elevation for ZDR data')
    (options, args) = parser.parse_args()
    
    if (options.debug == True):
        print >>sys.stderr, "Running %prog"
        print >>sys.stderr, "  file: ", options.file

    # read in headers

    (iret, colHeaders) = readColumnHeaders(options.file)
    if (iret != 0):
        sys.exit(1)

    # read in data

    (iret, colData) = readInputData(options.file, colHeaders)
    if (iret != 0):
        sys.exit(1)

    # render the plot
    
    doPlot(options.file, colHeaders, colData)

    sys.exit(0)
    
########################################################################
# Read columm headers for the data
# this is in the first line

def readColumnHeaders(filePath):

    colHeaders = []

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
        print >>sys.stderr, "  File: ", filePath
        print >>sys.stderr, "  First line does not start with #"
        return -1, colHeaders
    
    return 0, colHeaders

########################################################################
# Read in the data

def readInputData(filePath, colHeaders):

    # open file

    fp = open(filePath, 'r')
    lines = fp.readlines()

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

        for index, var in enumerate(colHeaders, start=0):
            colData[var].append(float(data[index]))

    fp.close()

    return 0, colData

########################################################################
# Plot

def doPlot(filePath, colHeaders, colData):

    zdr = np.array(colData["zdr"]).astype(np.double)
    elev = np.array(colData["elev"]).astype(np.double)

    minElev = float(options.minElev)
    maxElev = float(options.maxElev)

    print >>sys.stderr, "  ==>> zdr: ", zdr
    print >>sys.stderr, "  ==>> minElev: ", minElev
    print >>sys.stderr, "  ==>> maxElev: ", maxElev

    zdrValid = zdr[(elev >= minElev) & (elev <= maxElev)]

    print >>sys.stderr, "  ==>> size of zdr: ", len(zdr)
    print >>sys.stderr, "  ==>> size of elev: ", len(elev)
    print >>sys.stderr, "  ==>> size of zdrValid: ", len(zdrValid)

    mean0 = np.mean(zdrValid)
    sdev0 = np.std(zdrValid)

    zdrLimited = zdrValid[(zdrValid >= mean0 - sdev0 * 3) & (zdrValid <= mean0 + sdev0 * 3)]

    mean = np.mean(zdrLimited)
    sdev = np.std(zdrLimited)

    zdrSorted = np.sort(zdrLimited)
    minVal = np.min(zdrSorted)
    maxVal = np.max(zdrSorted)
    skew = stats.skew(zdrSorted)
    kurtosis = stats.kurtosis(zdrSorted)

    percPoints = np.arange(0,100,1.0)
    percs = np.percentile(zdrSorted, percPoints)

    print >>sys.stderr, "  ==>> min: ", minVal
    print >>sys.stderr, "  ==>> max: ", maxVal
    print >>sys.stderr, "  ==>> mean: ", mean
    print >>sys.stderr, "  ==>> sdev: ", sdev
    print >>sys.stderr, "  ==>> skew: ", skew
    print >>sys.stderr, "  ==>> kurtosis: ", kurtosis

    widthIn = float(options.figWidthMm) / 25.4
    htIn = float(options.figHeightMm) / 25.4
    
    fig1 = plt.figure(1, (widthIn, htIn))
    title = (options.title + '  for Elev Limits [ ' +
             options.minElev + ' : ' + options.maxElev + ' ]')
    #fig1.suptitle(title, fontsize=16)
    fig1.suptitle(filePath, fontsize=12)
    ax1 = fig1.add_subplot(2,1,1,xmargin=0.0)
    ax2 = fig1.add_subplot(2,1,2,xmargin=0.0)

    # the histogram of ZDR

    n1, bins1, patches1 = ax1.hist(zdrSorted, 60, normed=True,
                                   histtype='stepfilled',
                                   facecolor='slateblue',
                                   alpha=0.35)

    ax1.set_xlabel('ZDR')
    ax1.set_ylabel('Frequency')
    ax1.set_title('PDF - Probability Density Function', fontsize=14)
    ax1.grid(True)

    pdf = stats.norm(mean, sdev).pdf
    yy1 = pdf(bins1)
    ll1 = ax1.plot(bins1, yy1, 'b', linewidth=2)

    xmin, xmax = ax1.get_xlim()
    xplot = np.linspace(xmin, xmax, 60)

    # ax1.set_xlim([mean -sdev * 3, mean + sdev * 3])

    # aLog, locLog, scaleLog = stats.lognorm.fit(zdrSorted)
    # print >>sys.stderr, "  ==>> aLog: ", aLog
    # print >>sys.stderr, "  ==>> locLog: ", locLog
    # print >>sys.stderr, "  ==>> scaleLog: ", scaleLog

    # pdLog = stats.lognorm.pdf(xplot, aLog, locLog, scaleLog)
    # llLog = ax1.plot(xplot, pdLog, 'k', linewidth=2)

    # aSkew, locSkew, scaleSkew = stats.skewnorm.fit(zdrSorted)
    # print >>sys.stderr, "  ==>> aSkew: ", aSkew
    # print >>sys.stderr, "  ==>> locSkew: ", locSkew
    # print >>sys.stderr, "  ==>> scaleSkew: ", scaleSkew

    # pdSkew = stats.skewnorm.pdf(xplot, aSkew, locSkew, scaleSkew)
    # llSkew = ax1.plot(xplot, pdSkew, 'r', linewidth=2)

    # CDF of ZDR

    n2, bins2, patches2 = ax2.hist(zdrSorted, 60, normed=True,
                                   cumulative=True,
                                   histtype='stepfilled',
                                   facecolor='slateblue',
                                   alpha=0.35)

    ax2.set_xlabel('ZDR')
    ax2.set_ylabel('Cumulative frequency')
    ax2.set_title('CDF - Cumulative Distribution Function', fontsize=14)
    ax2.grid(True)

    cdf = stats.norm(mean, sdev).cdf
    yy2 = cdf(bins1)
    ll2 = ax2.plot(bins1, yy2, 'b', linewidth=2,
                   label = ('NormalFit mean=' + '{:.3f}'.format(mean) +
                            ' sdev=' + '{:.3f}'.format(sdev) +
                            ' skew=' + '{:.3f}'.format(skew)))
    legend2 = ax2.legend(loc='upper left', ncol=4)
    for label in legend2.get_texts():
        label.set_fontsize('medium')

    # ax2.set_xlim([mean -sdev * 3, mean + sdev * 3])

    # draw line to show mean, annotate

    pmean = pdf(mean)
    plen = pmean * 0.05
    toffx = mean * 0.1

    # draw line to show mean, annotate

    annotVal(ax1, ax2,  mean, pdf, cdf, 'mean', plen, toffx,
             'black', 'black', 'left', 'center')

    # annotate percentiles

    # perc5 = percs[6]
    # annotVal(ax1, ax2,  perc5, pdf, cdf, 'p%5', plen, toffx,
    #          'black', 'black', 'left', 'center')

    # perc15 = percs[16]
    # annotVal(ax1, ax2,  perc15, pdf, cdf, 'p%15', plen, toffx,
    #          'black', 'black', 'left', 'center')

    # perc25 = percs[26]
    # annotVal(ax1, ax2,  perc25, pdf, cdf, 'p%25', plen, toffx,
    #          'black', 'black', 'left', 'center')

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


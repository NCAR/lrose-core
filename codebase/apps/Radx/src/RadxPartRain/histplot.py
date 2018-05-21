#!/usr/bin/env python

#===========================================================================
#
# Produce histogram and CDF plots for ZDR bias in ice
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
    global zdr

# parse the command line

    usage = "usage: %prog [options]"
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--zdr_file',
                      dest='zdrFile',
                      default='./zdr.txt',
                      help='File path for bias results')
    parser.add_option('--title',
                      dest='title',
                      default='ZDR BIAS IN ICE',
                      help='Title for plot')
    parser.add_option('--width',
                      dest='figWidthMm',
                      default=300,
                      help='Width of figure in mm')
    parser.add_option('--height',
                      dest='figHeightMm',
                      default=400,
                      help='Height of figure in mm')
    (options, args) = parser.parse_args()
    
    if (options.debug == True):
        print >>sys.stderr, "Running %prog"
        print >>sys.stderr, "  zdrFile: ", options.zdrFile

    # read in data

    zdr = np.genfromtxt(options.zdrFile)

    # render the plot
    
    doPlot(zdr)

    sys.exit(0)
    
########################################################################
# Plot

def doPlot(zdr):

    zdrSorted = np.sort(zdr)
    mean = np.mean(zdrSorted)
    sdev = np.std(zdrSorted)
    skew = stats.skew(zdrSorted)
    kurtosis = stats.kurtosis(zdrSorted)

    percPoints = np.arange(0,100,1.0)
    percs = np.percentile(zdrSorted, percPoints)

    print >>sys.stderr, "  ==>> mean: ", mean
    print >>sys.stderr, "  ==>> sdev: ", sdev
    print >>sys.stderr, "  ==>> skew: ", skew
    print >>sys.stderr, "  ==>> kurtosis: ", kurtosis
    print >>sys.stderr, "  ==>> percs: ", percs

    widthIn = float(options.figWidthMm) / 25.4
    htIn = float(options.figHeightMm) / 25.4
    
    fig1 = plt.figure(1, (widthIn, htIn))
    ax1 = fig1.add_subplot(2,1,1,xmargin=0.0)
    ax2 = fig1.add_subplot(2,1,2,xmargin=0.0)

    # the histogram of ZDR

    n, bins, patches = ax1.hist(zdrSorted, 50, normed=True, facecolor='green', alpha=0.75)

    ax1.set_xlabel('ZDR')
    ax1.set_ylabel('Probability')
    ax1.set_title('PDF Histogram of ZDR in ice')
    ax1.grid(True)

    yy = mlab.normpdf(bins, mean, sdev)
    ll = ax1.plot(bins, yy, 'r--', linewidth=2)

    # CDF of ZDR

    n, bins, patches = ax2.hist(zdrSorted, 50, normed=True, cumulative=True,
                                facecolor='green', alpha=0.75)

    ax2.set_xlabel('ZDR')
    ax2.set_ylabel('Probability')
    ax2.set_title('CDF Histogram of ZDR in ice')
    ax2.grid(True)

    # show

    plt.show()

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


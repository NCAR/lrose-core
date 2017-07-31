#!/usr/bin/env python

#===========================================================================
#
# Produce histogram and CDF plots for ZDR bias in ice
#
#===========================================================================

import os
import sys

import numpy as np
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
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

    widthIn = float(options.figWidthMm) / 25.4
    htIn = float(options.figHeightMm) / 25.4
    
    fig1 = plt.figure(1, (widthIn, htIn))
    ax1 = fig1.add_subplot(2,1,1,xmargin=0.0)
    ax2 = fig1.add_subplot(2,1,2,xmargin=0.0)

    # the histogram of ZDR

    n, bins, patches = ax1.hist(zdr, 50, normed=True, facecolor='green', alpha=0.75)

    ax1.set_xlabel('ZDR')
    ax1.set_ylabel('Probability')
    ax1.set_title('PDF Histogram of ZDR in ice')
    ax1.grid(True)

    # CDF of ZDR

    n, bins, patches = ax2.hist(zdr, 50, normed=True, cumulative=True,
                                facecolor='green', alpha=0.75)

    ax2.set_xlabel('ZDR')
    ax2.set_ylabel('Probability')
    ax2.set_title('CDF Histogram of ZDR in ice')
    ax2.grid(True)

    # show

    plt.show()

    
    # add a 'best fit' line
    #y = mlab.normpdf( bins, mu, sigma)
    #l = plt.plot(bins, y, 'r--', linewidth=1)

    
    
    
    # #ax1c = fig1.add_subplot(3,1,3,xmargin=0.0)
    
    # if (haveTemps):
    #     fig2 = plt.figure(2, (widthIn/2, htIn/2))
    #     ax2a = fig2.add_subplot(1,1,1,xmargin=1.0, ymargin=1.0)
        
    #     oneDay = datetime.timedelta(1.0)
    #     ax1a.set_xlim([btimes[0] - oneDay, btimes[-1] + oneDay])
    #     ax1a.set_title("DYNAMO - ZDR bias in ice, compared with VERT results (dB)")
    #     ax1b.set_xlim([btimes[0] - oneDay, btimes[-1] + oneDay])
    #     ax1b.set_title("Daily mean ZDR bias in ice (dB)")

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


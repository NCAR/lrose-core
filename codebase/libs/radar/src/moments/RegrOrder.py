#!/usr/bin/env python

#===========================================================================
#
# Compute regression filter order
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

# parse the command line

    usage = "usage: %prog [options]"
    parser = OptionParser(usage)
    parser.add_option('--ss',
                      dest='ss',
                      default=1.0,
                      help='s-factor')
    parser.add_option('--rate',
                      dest='rate',
                      default=10.0,
                      help='scan rate (deg/s)')
    parser.add_option('--nsamples',
                      dest='nsamples',
                      default=64,
                      help='number of samples')
    parser.add_option('--prt',
                      dest='prt',
                      default=0.001,
                      help='PRT (secs)')
    parser.add_option('--wavelength',
                      dest='wavelength',
                      default=0.1068,
                      help='wavelength (m)')
    parser.add_option('--cnr',
                      dest='cnr',
                      default=50.0,
                      help='clutter-to-noise ratio (dB)')
    
    (options, args) = parser.parse_args()

    ss = float(options.ss)
    rate = float(options.rate)
    nsamples = float(options.nsamples)
    prt = float(options.prt)
    wavelength = float(options.wavelength)
    cnr = float(options.cnr)
    
    print("Running %prog", file=sys.stderr)
    print("  ss: ", ss, file=sys.stderr)
    print("  rate: ", rate, file=sys.stderr)
    print("  nsamples: ", nsamples, file=sys.stderr)
    print("  prt: ", prt, file=sys.stderr)
    print("  wavelength: ", wavelength, file=sys.stderr)
    print("  cnr: ", cnr, file=sys.stderr)

    wc = ss * (0.03 + 0.017* rate)
    nyquist = wavelength / (4.0 * prt)
    wcn = wc / nyquist
    on = -1.9791 * wcn * wcn + 0.6456 * wcn
    order = math.ceil(on * math.pow(cnr, 2.0 / 3.0) * nsamples)

    print("  ==>> wc: ", wc, file=sys.stderr)
    print("  ==>> nyquist: ", nyquist, file=sys.stderr)
    print("  ==>> wcn: ", wcn, file=sys.stderr)
    print("  ==>> on: ", on, file=sys.stderr)
    print("  ==>> order: ", order, file=sys.stderr)

    sys.exit(0)

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()


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

    zdrArray = np.arange(0.1, 5., 0.1)
    polySArray = []
    polyCArray = []
    powerArray = []

    for ii, zdr in enumerate(zdrArray, start=0):
        zdrLin = math.pow(10.0, zdr / 10.0)
        power = 3.32 * math.pow(zdrLin, -2.05)
        polyS = 3.19 + -2.16 * zdr + 0.795 * math.pow(zdr, 2.0) + -0.119 * math.pow(zdr, 3.0)
        polyC = 6.7 + -4.42 * zdr + 2.16 * math.pow(zdr, 2.0) + -0.404 * math.pow(zdr, 3.0)
        polySArray.append(polyS)
        polyCArray.append(polyC)
        powerArray.append(power)

    fig = plt.figure(1, (5.0, 5.0))
    ax1 = fig.add_subplot(1,1,1,xmargin=0.0)
    ax1.plot(zdrArray, polySArray, \
             label='Poly-Sband', color='blue', linewidth=1)
    ax1.plot(zdrArray, polyCArray, \
             label='Poly-Cband', color='green', linewidth=1)
    ax1.plot(zdrArray, powerArray, \
             label='Power', color='red', linewidth=1)

    legend = ax1.legend(loc="upper right", ncol=3)
    for label in legend.get_texts():
        label.set_fontsize('x-small')
    ax1.set_xlabel("Zdr")
    ax1.set_ylabel("KDP 50dBZ")
    ax1.grid(True)

    plt.show()

    sys.exit(0)

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()


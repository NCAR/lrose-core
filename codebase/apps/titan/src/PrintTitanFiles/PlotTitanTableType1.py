#!/usr/bin/env python

#===========================================================================
#
# Plot TITAN output Table Type 1 - for Silke intercomparison project
#
#===========================================================================

import os
import sys
import subprocess
from optparse import OptionParser
#import numpy as np
import matplotlib.pyplot as plt
import math

def main():

    global options
    global debug

    global fieldNameX
    global dataListX

    global fieldNameY
    global dataListY

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
                      default='/tmp/fs110042_meghatropiques_edited.txt',
                      help='Input file path')
    parser.add_option('--title',
                      dest='title',
                      default='project',
                      help='Title for plot')
    parser.add_option('--fieldNameX',
                      dest='fieldNameX',
                      default='none',
                      help='X Field to be plotted')
    parser.add_option('--fieldLabelX',
                      dest='fieldLabelX',
                      default='',
                      help='If set, used in plot instead of field name')
    parser.add_option('--unitsX',
                      dest='unitsX',
                      default='',
                      help='Units of X field to be plotted')
    parser.add_option('--columnX',
                      dest='columnX',
                      default=0,
                      help='X Column to be plotted. Note: 0-based')
    parser.add_option('--fieldNameY',
                      dest='fieldNameY',
                      default='none',
                      help='Y Field to be plotted')
    parser.add_option('--fieldLabelY',
                      dest='fieldLabelY',
                      default='',
                      help='If set, used in plot instead of field name')
    parser.add_option('--unitsY',
                      dest='unitsY',
                      default='',
                      help='Units of Y field to be plotted')
    parser.add_option('--columnY',
                      dest='columnY',
                      default=0,
                      help='Y Column to be plotted. Note: 0-based')
    parser.add_option('--delim',
                      dest='delim',
                      default=';',
                      help='Table file delimiter')
    parser.add_option('--xy',
                      dest='plotXy', default=False,
                      action="store_true",
                      help='Set plot type to XY')
    parser.add_option('--hist',
                      dest='plotHist', default=False,
                      action="store_true",
                      help='Set plot type to histogram')
    parser.add_option('--nbins',
                      dest='nbins',
                      default=30,
                      help='Number of histogram bins')
    parser.add_option('--loghist',
                      dest='loghist', default=False,
                      action="store_true",
                      help='Use logs for histogram frequency')
    parser.add_option('--counthist',
                      dest='counthist', default=False,
                      action="store_true",
                      help='Use counts instead of frequency for histograms')
    
    (options, args) = parser.parse_args()
    
    if (options.verbose == True):
        options.debug = True

    if (options.debug == True):
        print >>sys.stderr, "Running PlotTitanTableType1:"
        print >>sys.stderr, "  inputFilePath: ", options.inputFilePath
        print >>sys.stderr, "  plotHist: ", options.plotHist

    # read in file

    readInputFile()

    # plot XY?
    
    if (options.plotXy == True):
        plotXy()

    # plot histogram?
    
    if (options.plotHist == True):
        plotHist()

    sys.exit(0)
    
########################################################################
# Read in the input file

def readInputFile():

    global fieldNameX
    global dataListX

    global fieldNameY
    global dataListY

    fieldNameX = options.fieldNameX
    fieldColX = int(options.columnX)
    dataListX = []

    fieldNameY = options.fieldNameY
    fieldColY = int(options.columnY)
    dataListY = []

    fp = open(options.inputFilePath, 'r')
    lines = fp.readlines()
    fieldSetX = False
    fieldSetY = False

    for line in lines:
        
        commentIndex = line.find("%")

        if (commentIndex < 0):
            
            # data

            data = line.strip().split(options.delim)
            valX = data[fieldColX]
            dataListX.append(float(valX))
            if (options.verbose == True):
                print >>sys.stderr, "data line: ", data
                print >>sys.stderr, "data valX: ", valX

            if (options.plotXy == True):
                valY = data[fieldColY]
                dataListY.append(float(valY))
                if (options.verbose == True):
                    print >>sys.stderr, "data line: ", data
                    print >>sys.stderr, "data valY: ", valY

        else:
            
            # header
            
            headers = line.lstrip(" %").replace(" ", "").strip().split(options.delim)

            if (options.verbose == True):
                print >>sys.stderr, "headers : ", headers

            (fieldColX, fieldNameX) = \
                setField("X", headers, options.columnX, options.fieldNameX)

            if (options.plotXy == True):
                (fieldColY, fieldNameY) = \
                    setField("Y", headers, options.columnY, options.fieldNameY)

########################################################################
# Set data column

def setField(axis, headers, optionsColumn, optionsFieldName):

    fieldSet = False
    fieldCol = int(optionsColumn)
    fieldName = optionsFieldName

    if (optionsFieldName == "none"):

        fieldCol = int(optionsColumn)
        fieldName = headers[fieldCol]
        fieldSet = True
        
    else:
        
        column = 0
        for header in headers:
            if (header == optionsFieldName):
                fieldCol = column
                fieldName = optionsFieldName
                fieldSet = True
                break
            column = column + 1

        if (fieldSet == False):
            fieldCol = int(optionsCcolumn)
            fieldName = headers[fieldCol]
                
    if (options.debug == True):
        print >>sys.stderr, "  Searching for field for axis: " + axis
        if (optionsFieldName == "none"):
            print >>sys.stderr, "  field name not specified"
        else:
            if (fieldSet == False):
                print >>sys.stderr, "  cannot find field: ", optionsFieldName
        print >>sys.stderr, "  using column: ", fieldCol
        print >>sys.stderr, "  using field: ", fieldName
  
    return (fieldCol, fieldName)

########################################################################
# Plot histogram

def plotHist():

    if (options.counthist == True):
        plt.hist(dataListX, bins = int(options.nbins),
                 normed=False, log=options.loghist)
    else:
        plt.hist(dataListX, bins = int(options.nbins),
                 normed=True, log=options.loghist)
    plt.title(options.title)
    if (len(options.fieldLabelX) > 0):
        plt.xlabel(options.fieldLabelX + " " + options.unitsX)
    else:
        plt.xlabel(fieldNameX + " " + options.unitsX)
    if (options.loghist == True):
        if (options.counthist == True):
            plt.ylabel("Count - log scale")
        else:
            plt.ylabel("Frequency - log scale")
    else:
        if (options.counthist == True):
            plt.ylabel("Count")
        else:
            plt.ylabel("Frequency")

    plt.show()
    
    return

########################################################################
# Plot histogram

def plotXy():

    plt.plot(dataListX, dataListY)

    plt.title(options.title)

    if (len(options.fieldLabelX) > 0):
        plt.xlabel(options.fieldLabelX + " " + options.unitsX)
    else:
        plt.xlabel(fieldNameX + " " + options.unitsX)

    if (len(options.fieldLabelY) > 0):
        plt.ylabel(options.fieldLabelY + " " + options.unitsY)
    else:
        plt.ylabel(fieldNameY + " " + options.unitsY)

    plt.show()
    
    return

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


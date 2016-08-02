#!/usr/bin/env python

##############################################################
# UAERainCsv2SfcCsv.py                                       #
##############################################################
# Python script for converting a UAE Rainfall CSV file       #
# to a CSV format that can be ingested by UAEMesonet2Spdb    #
##############################################################

import getopt
import os
import string
import sys
import subprocess
from datetime import datetime
from optparse import OptionParser
import csv

###############################################################
# Main program                                                #
###############################################################

def main():

  # Specify the globals
  
  global options
  # Retrieve the program name from the command line.

  prog_name = os.path.basename(sys.argv[0])

  # Initialize the command line arguments.
  
  usage = "usage: %prog [options]"
  parser = OptionParser(usage)
  parser.add_option('-d', '--debug',
                    dest='debug',
                    default='False',
                    action="store_true",
                    help='Set debugging on.')
  parser.add_option('--input_file',
                    dest='input_file',
                    default='./data/Rainfall_AWS_AIRPORTS_2003_2014.csv',
                    help='Input CSV file.')
  parser.add_option('--output_file',
                    dest='output_file',
                    default='./data/Rainfall_AWS_AIRPORTS_2003_2014-ingest.csv',
                    help='File name for writing new CSV file.')
  
  (options, args) = parser.parse_args()

  if (options.debug == True):
    print >>sys.stderr, 'Running %s:' % prog_name
    print >>sys.stderr, 'debug = ', options.debug
    print >>sys.stderr, 'input_file = ', options.input_file
    print >>sys.stderr, 'output file = ', options.output_file

  of = open(options.output_file,"w")

  stations = []
  with open(options.input_file, 'rU') as CsvFile:
      reader = csv.reader(CsvFile, dialect=csv.excel)
      header = True
      for row in reader:
          if header:
              of.write(":Station,Year,Month,Day,Hour (GMT),Minute,Precip [mm/24hr]\n")
              for i in range(3,len(row)):
                  stations.append(row[i])
              header = False
          else:    
              for y in range(3,len(row)-1):
                  if row[y] == "":
                      row[y] = "-9990";
                  of.write(stations[y - 3] + "," + row[0] + "," + row[1] + "," + row[2] + ",0,0," + row[y] + "\n")

  CsvFile.close()
  of.close()

  sys.exit(0)
  
########################################################################
# Run - entry point

if __name__ == "__main__":
  main()
       

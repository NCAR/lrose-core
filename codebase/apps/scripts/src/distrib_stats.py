#!/usr/bin/env python2

##############################################################
# distrib_stats.py                                           #
##############################################################
# Python script for generating statistics based on the       #
# contents of the distrib log files.                         #
##############################################################

import ftplib
import getopt
import os
import socket
import stat
import string
import sys
import time
import shutil
from datetime import datetime


###############################################################
# Local classes                                               #
###############################################################

class Record:
    def __init__(self):
        self.total_bytes = 0
        self.num_successes = 0
        self.num_errors = 0

    def add(self, num_bytes, success_flag):
      self.total_bytes += num_bytes
      if success_flag == "success":
        self.num_successes += 1
      elif success_flag == "error":
        self.num_errors += 1
      else:
        print "*** unknown success/error flag: " + success_flag
        print "    treating record as an error"
        self.num_errors += 1
      return self

    def get_total_bytes(self):
      return self.total_bytes

    def get_total_errors(self):
      return self.num_errors
    
    def get_total_msgs(self):
      return self.num_successes + self.num_errors

    def println(self):
      print "bytes = %d, success = %d, errors = %d" % \
            (self.total_bytes, self.num_successes, self.num_errors)

    def print_stats(self, key):
      num_records = self.num_successes + self.num_errors
      percent_error = float(self.num_errors) / float(num_records)
      bytes_per_record = float(self.total_bytes) / float(num_records)
      print "%-30s %20d %11d %20.5f %10d (%f%%)" % \
            (key, self.total_bytes, num_records, bytes_per_record, self.num_errors, percent_error * 100.0)


###############################################################
# Local subroutines                                           #
###############################################################

###############################################################
# print_usage(): Print the usage for this script
#

def print_usage():
  print "This script generates statistics from a list of distrib"
  print "log files.  Run on a single file to get daily statistics"
  print "or on a list of files to get statistics for any time period."
  print "Usage: ", prog_name, " < distrib file list >"

  return


###############################################################
# print_stats(): Print the statistics for the given dictionary.
#

def print_stats(dictionary):
  # Sort the keys for better output
  
  keys = dictionary.keys()
  keys.sort()

  # Print the statistics for each record.  Along the way,
  # accumulate total statistics

  total_bytes = 0
  total_msgs = 0
  total_errors = 0
  
  print "                                        total bytes    num msgs            bytes/msg                 errors"
  print "                                        -----------    --------            ---------                 ------"

  for key in keys:
    dictionary[key].print_stats(key)
    total_bytes += dictionary[key].get_total_bytes()
    total_msgs += dictionary[key].get_total_msgs()
    total_errors += dictionary[key].get_total_errors()

  # Now print the total statistics

  percent_error = (float(total_errors) / float(total_msgs)) * 100.0
  bytes_per_msg = float(total_bytes) / float(total_msgs)
  
  print
  print "Total                          %20d %11d %20.5f %10d (%f%%)" % \
        (total_bytes, total_msgs, bytes_per_msg, total_errors, percent_error)
  
  return


###############################################################
# Main program                                                #
###############################################################

if __name__ == "__main__":

  # Retrieve the program name from the command line.

  prog_name = os.path.basename(sys.argv[0])

  # Initialize the statistics records

  dir_records = {}
  host_records = {}

  # The command line consists of a list of files to process.
  # Process each file in turn.
  
  for file in sys.argv[1:]:
    print "*** Processing " + file

    # Open the file

    input_file = open(file, 'r')

    # Process each of the lines in the file

    for line in input_file:
      # Split the line into tokens.  We expect the following
      # tokens:
      #    0 - distrib time
      #    1 - success/error
      #    2 - over (overwrite???)
      #    3 - destination host
      #    4 - num bytes
      #    5 - data time
      #    6 - destination directory

      tokens = str.split(line, ",")
      if len(tokens) != 7:
        print "Invalid line found: " + line
        print "Skipping"
        next

      # Remove the whitespace around each of the tokens and
      # give them mnemonic names

      distrib_time = str.strip(tokens[0])
      success_flag = str.strip(tokens[1])
      over_flag = str.strip(tokens[2])
      dest_host = str.strip(tokens[3])
      num_bytes = str.strip(tokens[4])
      data_time = str.strip(tokens[5])
      dest_dir = str.strip(tokens[6])

      # Update the dest dir record

      if dest_dir in dir_records:
        dir_record = dir_records[dest_dir]
      else:
        dir_record = Record()

      dir_record.add(int(num_bytes), success_flag)
      dir_records[dest_dir] = dir_record
      
      # Update the dest host record

      if dest_host in host_records:
        host_record = host_records[dest_host]
      else:
        host_record = Record()

      host_record.add(int(num_bytes), success_flag)
      host_records[dest_host] = host_record
      
      
    # Close the file

    input_file.close()
    
  # All of the files have been processed, so now we can calculate the
  # statistics.  Start with the destination dir statistics.

  print
  print
  print "Destination Directory Statistics"
  print "================================"
  print_stats(dir_records)
  
  # Now the destination host statistics

  print
  print
  print "Destination Host Statistics"
  print "==========================="
  print_stats(host_records)
  

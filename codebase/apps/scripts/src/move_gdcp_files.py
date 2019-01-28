#!/usr/bin/env python2

#
# move_gdcp_files.py -- this script moves and renames files GDCP files from LARC. The goal
# is to put the files into a 'RAP' directory structure: <base_dir>/<yyyymmdd>/<hhmmss>.<ext>
#
#

# import modules

# system modules

import signal
import getopt
from time import asctime
from time import gmtime
from time import sleep
from time import mktime
from time import timezone
from time import time
from time import strftime
from string import split
from sys import argv
from sys import exit
from sys import stdout
from os import path
from os import system
from os import makedirs
from os import rename
from os import listdir

# local modules from libs/python
import pmu

#
# Initialize the command line arguments.
#
opt_debug = False
opt_input_dir = '/scratch'
opt_output_dir = '/scratch'


################################################################################
# Local subroutines                                                            #
################################################################################

################################################################################
# handle_options(): handles the command line options
#

def handle_options():

  global opt_debug
  global opt_input_dir
  global opt_output_dir

  #
  # Get the command line arguments
  #
  try:
    optlist, args = getopt.getopt(argv[1:], 'dh', \
                                  [ 'debug', 'help', \
                                    'input_dir=', 'output_dir='])
  except getopt.GetoptError, e:
    # print help information and exit
    print e
    usage()
    exit(1)
 
  for opt in optlist:
    if opt[0] == '-h' or opt[0] == '--help':
      usage()
      exit(0)
      
    if opt[0] == '-d' or opt[0] == '--debug':
      opt_debug = True

    if opt[0] == '--output_dir':
      opt_output_dir = opt[1]
    if opt[0] == '--input_dir':
      opt_input_dir = opt[1]

  #
  # check the argument list
  #
  debug_print('Command line options:')
  debug_print('\tdebug = on')
  debug_print('\tinput_dir = '+ opt_output_dir)
  debug_print('\toutput_dir = '+ opt_output_dir)

################################################################################
# print_usage(): Print the usage for this script
#

def usage():

  print 'Usage: ', prog_name, ' [options]'
  print 'Options:'
  print '   -h | --help             : Print usage and exit'
  print '   --output_dir <dir>      : data directory for output'
  print '                             (default = ', opt_output_dir, ')'
  print '   --input_dir <dir>      : data directory for output'
  print '                             (default = ', opt_input_dir, ')'


################################################################################
# handler(signum, frame): handler called by signal.alarm used to detect
#                         when ftp listing is hung
#

def handler(signum, frame):
  print 'Timeout!'


################################################################################
# debug_print(msg): prints debugging message
#

def debug_print(msg):

  if not opt_debug:
    return

  msg_hdr = '\tDEBUG -- '
  print msg_hdr + msg


################################################################################
#  monthday(year, day_of_year):
#

def monthday(year, day_of_year):
  month = 1
  days_in_month = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
  
  #
  # Leap year adjustment
  #
  if (year % 4) == 0:
    if (year % 100) == 0:
      if (year % 400) == 0:
        days_in_month[1] = 29
      else:
        days_in_month[1] = 28
    else:
      days_in_month[1] = 29
					
  while day_of_year > days_in_month[month - 1]:
   day_of_year = day_of_year - days_in_month[month - 1]
   month = month + 1

  return (month, day_of_year)


################################################################################
# get_file_time(file_name): creates the time from time stamp encoded in the
#                  file name
#
# returns unix-like time 
#
#   Input files have the following format
#       goes-w.icing.rucgrid.2005146.1800.cdf.gz
#
def get_file_time(file_name):

  split_filemame = split(file_name, '.')
  date_str = split_filemame[3]
  time_str = split_filemame[4]
  (month, day)  = monthday(int(date_str[:4]), int(date_str[4:]))
  time_tuple = (int(date_str[:4]), month, day, int(time_str[:2]), \
                int(time_str[2:]), 0, 0, 0, 0)
  gmt_time = mktime(time_tuple) - timezone

  return gmt_time


################################################################################
# get_output_filename(file_time, ext):
#
#

def get_output_filename(file_time, file_ext):


  gmtpl = gmtime(file_time)
  date_dir = strftime('%Y%m%d', gmtpl)
  file_dir = path.join(opt_output_dir, date_dir)
  if not path.isdir(file_dir):
    try:
      makedirs(file_dir)
    except OSError, e:
      print 'unable to make ' + file_dir + ': ', e
      print 'exiting ...'
      exit(1)

  file_name =  strftime('%H%M%S', gmtpl) + file_ext

  return path.join(file_dir, file_name)


################################################################################
# Main program                                                                 #
################################################################################

if __name__ == '__main__':

  prog_name = path.basename(argv[0])   

  if len(argv) < 2:
    usage()
    exit(0)

  #
  # handle the command line options
  #
  handle_options()


  #
  # get a list of files in opt_input_dir
  #
  try:
    file_list = listdir(opt_input_dir)
  except OSError, e:
      print 'unable to get directory listing from  ' + opt_input_dir + ': ', e
      print 'exiting ...'
      exit(1)

  for filename in file_list:

    fetch_path = path.join(opt_input_dir, filename)
    
    print  fetch_path

    file_time = get_file_time(filename)

    #
    # now move the retrieved file to it final location 
    #
    (junk, ext) = path.splitext(fetch_path)
    final_path = get_output_filename(file_time, ext)
    print  final_path
    rename(fetch_path, final_path)
      


  exit(0)

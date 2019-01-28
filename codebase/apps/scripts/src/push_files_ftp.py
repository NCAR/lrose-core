#!/usr/bin/env python2

##############################################################
# push_rap_files_ftp.py                                      #
##############################################################
# Python script for pushing files from a RAP-style directory #
# to another machine using FTP.                              #
##############################################################

import ftplib
import getopt
import os
import stat
import string
import sys
import time

import pmu


###############################################################
# Local subroutines                                           #
###############################################################

###############################################################
# print_usage(): Print the usage for this script
#

def print_usage():
  print "Usage: ", prog_name, " [options]"
  print "Options:"
  print "   -h | --help             : Print usage and exit"
  print "   --dest_dir <dir>        : Destination directory"
  print "                             (default = ", opt_dest_dir, ")"
  print "   --dest_host <host>      : Destination host name"
  print "                             (default = ", opt_dest_host, ")"
  print "   --dest_pwd <password>   : Destination login password"
  print "                             (default = ", opt_dest_pwd, ")"
  print "   --dest_user <login>     : Destination login to use"
  print "                             (default = ", opt_dest_user, ")"
  print "   --input_dir <dir>       : RAP style data directory to watch"
  print "                             (default = ", opt_input_dir, ")"
  print "   --instance <instance>   : PMU instance name (default = ", \
	opt_instance, ")"
  print "   --max_valid_age <secs>  : Maximum valid age, in seconds,"
  print "                             for input files.  Files older "
  print "                             than this will not be sent."
  print "                             Set to -1 to ignore the file time."
  print "                             (default = ", opt_max_valid_age, ")"
  print "   -p | --passive          : Use passive mode in FTP"
  print "   --register_interval <secs>"
  print "                           : Number of seconds between registering"
  print "                             with the process mapper."
  print "                             (default = ", opt_register_interval, ")"
  print "   --sleep_secs <secs>     : Number of seconds to sleep between"
  print "                             checks for new data. (default = ", \
        opt_sleep_secs, ")"      
  print "   --substring <string>    : File substring to check for"
  print "                             (default = ", opt_substring, ")"
  print "   --die                   : flag causes script to die after one iteration."
  print "   -d | --debug            : Print debug messages"
  print "   --use_temp_file         : Flag causes script to write the file contents"
  print "                             to a temporary file on the FTP site before moving"
  print "                             it to the permanent file name"
  return


###############################################################
# push_file(): Push the given file to the appropriate destination.
#              If opt_push_ldata is set, the _latest_data_info
#              file is pushed after the data file.
#

def push_file(input_filename):
  if opt_debug:
    print "Pushing file ", input_filename

  pmu.force_register("Pushing file " + input_filename)

  #
  # Open the ftp connection
  #

  ftp = ftplib.FTP(opt_dest_host, opt_dest_user, opt_dest_pwd)

  if opt_passive:
    ftp.set_pasv(1)

  #
  # Move to the appropriate output directory
  #

  ftp.cwd(opt_dest_dir)

  #
  # Push the data file to the destination machine.
  # Prepend an '_' to the filename while sending then
  # rename it to the appropriate names so it appears
  # autonomously.
  #

  pmu.force_register('Sending file ' + input_filename)

  if opt_use_temp_file:
    output_filename = "_" + input_filename
  else:
    output_filename = input_filename
    
  datafile = open(opt_input_dir + '/' + input_filename, 'r')
  try:
    ftp.storbinary('STOR ' + output_filename, datafile, 8192)
  except (socket.error):
    print "Socket error occurred while trying to transfer file: ", datafile
    print "Skipping file!"
    return

  datafile.close()

  if opt_use_temp_file:
    ftp.rename(output_filename, input_filename)

  #
  # Close the FTP connection
  #

  ftp.quit()


###############################################################
# Main program                                                #
###############################################################

if __name__ == "__main__":

  #
  # Retrieve the program name from the command line.
  #

  prog_name = os.path.basename(sys.argv[0])

  #
  # Initialize the command line arguments.
  #

  opt_debug = 0
  opt_dest_dir = 'incoming/ncwf'
  opt_dest_host = 'ftp.rap.ucar.edu'
  opt_dest_pwd = 'ncwf@ftb1.kc.noaa.gov'
  opt_dest_user = 'anonymous'
  opt_input_dir = '.'
  opt_instance = 'test'
  opt_max_valid_age = -1
  opt_passive = 0
  opt_register_interval = 60
  opt_sleep_secs = 2 
  opt_substring = ''
  opt_die = 0
  opt_use_temp_file = 0
  
  #
  # Get the command line arguments.
  #

  optlist, args = getopt.getopt(sys.argv[1:], 'dhp', \
        [ 'debug', 'help', \
          'dest_dir=', 'dest_host=', 'dest_pwd=', 'dest_user=', \
          'input_dir=', 'instance=', \
          'max_valid_age=', 'passive', \
          'register_interval=', 'sleep_secs=', 'substring=', 'die', \
          'use_temp_file' ])

  for opt in optlist:
    if opt[0] == "-h" or opt[0] == "--help":
      print_usage()
      sys.exit()

    if opt[0] == "-d" or opt[0] == "--debug":
      opt_debug = 1

    if opt[0] == "--dest_dir":
       opt_dest_dir = opt[1]

    if opt[0] == "--dest_host":
      opt_dest_host = opt[1]

    if opt[0] == "--dest_pwd":
      opt_dest_pwd = opt[1]

    if opt[0] == "--dest_user":
      opt_dest_user = opt[1]

    if opt[0] == "--input_dir":
      opt_input_dir = opt[1]

    if opt[0] == "--instance":
      opt_instance = opt[1]

    if opt[0] == "--max_valid_age":
      opt_max_valid_age = opt[1]

    if opt[0] == "-p" or opt[0] == "--passive":
      opt_passive = 1

    if opt[0] == "--register_interval":
      try:
        opt_register_interval = int(opt[1])
      except (ValueError):
        print "Error: register_interval argument must be an integer"
        print "Using 60 seconds instead"
        opt_register_interval = 60

    if opt[0] == "--sleep_secs":
      try:
        opt_sleep_secs = opt[1]
      except (ValueError):
        print "Error: sleep_secs arguments must be an integer"
        print "Using 2 seconds"
        opt_sleep_secs = 2

    if opt[0] == "--substring":
      opt_substring = opt[1]

    if opt[0] == "--die":
      opt_die = 1

    if opt[0] == "--use_temp_file":
      opt_use_temp_file = 1
      
  if (opt_debug):
    print "opt_debug = ", opt_debug
    print "opt_dest_dir = ", opt_dest_dir
    print "opt_dest_host = ", opt_dest_host
    print "opt_dest_pwd = ", opt_dest_pwd
    print "opt_dest_user = ", opt_dest_user
    print "opt_input_dir = ", opt_input_dir
    print "opt_instance = ", opt_instance
    print "opt_max_valid_age = ", opt_max_valid_age
    print "opt_passive = ", opt_passive
    print "opt_register_interval = ", opt_register_interval
    print "opt_sleep_secs = ", opt_sleep_secs
    print "opt_substring = ", opt_substring
    print "opt_die = ", opt_die
    print "opt_use_temp_file = ", opt_use_temp_file
    
  #
  # Initialize process mapper registration
  #

  pmu.auto_init(prog_name, opt_instance, opt_register_interval)

  #
  # Wait for new files.  When found, push them to the destination.
  #

  last_dir_mod_time = os.stat(opt_input_dir)[stat.ST_MTIME]
  need_sleep = 0

  while 1:

    pmu.auto_register("Waiting for data")
    if opt_debug:
      print "Waiting for data"

    if need_sleep:
      time.sleep(opt_sleep_secs)

    need_sleep = 1

    #
    # Check for new files
    #

    dir_mod_time = os.stat(opt_input_dir)[stat.ST_MTIME]
    if dir_mod_time <= last_dir_mod_time:
      continue

    #
    # Push each new file to the destination location.
    #

    for filename in os.listdir(opt_input_dir):


      #
      # Check for the correct file substring
      #

      try:
        substring_index = string.index(filename, opt_substring)
      except ValueError:
        continue

      input_path = opt_input_dir + '/' + filename

      try:
        file_mod_time = os.stat(input_path)[stat.ST_MTIME]
      except os.error:
        continue

      if file_mod_time <= last_dir_mod_time:
        continue

      if opt_debug:
        print ""

      push_file(filename)

      if opt_debug:
        print ""

    # Reset the value of the last mod time for the next loop

    last_dir_mod_time = dir_mod_time

    if opt_die:
      break

  pmu.auto_unregister()






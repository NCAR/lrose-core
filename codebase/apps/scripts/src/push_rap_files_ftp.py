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
import string
import sys

import ldata
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
  print "   --flat_output           : Flag indicating the output files"
  print "                             should be put in a flat directory"
  print "                             and named YYYYMMDD_HHMMSS.ext"
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
  print "   --push_ldata            : Push the _latest_data_info file from"
  print "                               the input directory after pushing"
  print "                               the data file."
  print "   --sleep_secs <secs>     : Number of seconds to sleep between"
  print "                             checks for new data. (default = ", \
        opt_sleep_secs, ")"      
  print "   -d | --debug            : Print debug messages"

  return


###############################################################
# push_file(): Push the given file to the appropriate destination.
#              If opt_push_ldata is set, the _latest_data_info
#              file is pushed after the data file.
#

def push_file(input_ldata):
  input_path = input_ldata.data_path(opt_input_dir)

  if opt_debug:
    print "Pushing file ", input_path

  #
  # Make sure the file exists -- it could have been scrubbed.
  #

  try:
    os.stat(input_path)
  except os.error:
    if opt_debug:
      print "*** File doesn't exist, skipping"
    return

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
  # Get the subdirectory and filename for the output file.
  #

  subdir = input_ldata.data_subdir()
  filename = input_ldata.data_filename()

  if opt_debug:
    print "subdir = ", subdir
    print "filename = ", filename

  #
  # Put the output file in the appropriate directory structure
  #

  if opt_flat_output:

    output_filename = subdir + '_' + filename

  else:

    output_filename = filename

    #
    # Create the subdirectory and move to it.  It's okay
    # if there's an exception because that means that the
    # subdir already exists.
    #

    try:
      ftp.mkd(subdir)
    except ftplib.error_perm:
      pass

    ftp.cwd(subdir)

  #
  # Push the data file(s) to the destination machine.
  # Prepend an '_' to the filenames while sending then
  # rename them to the appropriate names so they appear
  # autonomously.
  #

  pmu.force_register('Sending file ' + input_path)

  datafile = open(input_path, 'r')
  ftp.storbinary('STOR _' + output_filename, datafile, 8192)
  datafile.close()
  ftp.rename('_' + output_filename, output_filename)

  if opt_push_ldata:

    if not opt_flat_output:
      ftp.cwd('..')
    
    ldata_path = opt_input_dir + '/_latest_data_info'

    pmu.force_register('Sending file ' + ldata_path)

    ldatafile = open(ldata_path, 'r')
    ftp.storlines('STOR __latest_data_info', ldatafile)
    ldatafile.close()
    ftp.rename('__latest_data_info', '_latest_data_info')

  #
  # Close the FTP connection
  #

  ftp.close()


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
  opt_flat_output = 0
  opt_input_dir = '.'
  opt_instance = 'test'
  opt_max_valid_age = -1
  opt_passive = 0
  opt_push_ldata = 0
  opt_sleep_secs = 2

  #
  # Get the command line arguments.
  #

  optlist, args = getopt.getopt(sys.argv[1:], 'dhp', \
        [ 'debug', 'help', \
          'dest_dir=', 'dest_host=', 'dest_pwd=', 'dest_user=', \
          'flat_output', 'input_dir=', 'instance=', \
          'max_valid_age=', 'passive', 'push_ldata', \
          'sleep_secs=' ])

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

    if opt[0] == "--flat_output":
      opt_flat_output = 1

    if opt[0] == "--input_dir":
      opt_input_dir = opt[1]

    if opt[0] == "--instance":
      opt_instance = opt[1]

    if opt[0] == "--max_valid_age":
      opt_max_valid_age = opt[1]

    if opt[0] == "-p" or opt[0] == "--passive":
      opt_passive = 1

    if opt[0] == "--push_ldata":
      opt_push_ldata = 1

    if opt[0] == "--sleep_secs":
      opt_sleep_secs = opt[1]

  if (opt_debug):
    print "opt_debug = ", opt_debug
    print "opt_dest_dir = ", opt_dest_dir
    print "opt_dest_host = ", opt_dest_host
    print "opt_dest_pwd = ", opt_dest_pwd
    print "opt_dest_user = ", opt_dest_user
    print "opt_flat_output = ", opt_flat_output
    print "opt_input_dir = ", opt_input_dir
    print "opt_instance = ", opt_instance
    print "opt_max_valid_age = ", opt_max_valid_age
    print "opt_passive = ", opt_passive
    print "opt_push_ldata = ", opt_push_ldata
    print "opt_sleep_secs = ", opt_sleep_secs


  #
  # Initialize process mapper registration
  #

  pmu.auto_init(prog_name, opt_instance, 60)

  #
  # Initialize the ldata object
  #

  input_ldata = ldata.Ldata(prog_name, opt_debug)

  #
  # Wait for new files.  When found, push them to the destination.
  #

  while 1:

    #
    # Wait for the next new file
    #

    input_ldata.info_read_blocking(opt_input_dir, opt_max_valid_age, \
                                   opt_sleep_secs * 1000, pmu.auto_register)

    #
    # Push the file to the destination location.
    #

    if opt_debug:
      print ""

    push_file(input_ldata)

    if opt_debug:
      print ""

  pmu.auto_unregister()


#!/usr/bin/env python2

##############################################################
# push_spdb_files_rcp.py                                     #
##############################################################
# Python script for pushing SPDB files from a directory to   #
# another machine using rcp.  This script can also be used   #
# to copy the files on the local host using cp.              #
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
  print "   --copy <cmd>            : Copy command to use"
  print "                             (default = ", opt_copy, ")"
  print "   --dest_dir <dir>        : Destination directory"
  print "                             (default = ", opt_dest_dir, ")"
  print "   --dest_host <host>      : Destination host name."
  print "                             Use localhost to copy the file"
  print "                             on the local host using cp."
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
  #
  # Get the filenames for the output files.
  #

  data_filename = input_ldata.data_subdir() + '.data'
  indx_filename = input_ldata.data_subdir() + '.indx'

  if opt_debug:
    print "data_filename = ", data_filename
    print "indx_filename = ", indx_filename

  #
  # Construct the input file paths
  #

  input_data_path = opt_input_dir + '/' + data_filename
  input_indx_path = opt_input_dir + '/' + indx_filename

  #
  # Push the files to the destination machine
  #

  do_push(input_data_path, opt_dest_dir, data_filename)
  do_push(input_indx_path, opt_dest_dir, indx_filename)

  #
  # If requested, push the ldata file
  #

  if opt_push_ldata:
    input_ldata_path = opt_input_dir + '/_latest_data_info'

    do_push(input_ldata_path, opt_dest_dir, '_latest_data_info')


###############################################################
# do_push(): Push the given file to the destination.
#

def do_push(input_path, output_dir, output_filename):

  #
  # Make sure the output directory exists
  #

  if opt_dest_host == 'localhost':
    command = 'mkdir -p ' + output_dir
  else:
    command = shell_cmd + ' ' + opt_dest_host + '"mkdir -p ' + output_dir + '"'

  if opt_debug:
    print "Executing command: " + command

  os.system(command)

  #
  # Copy the file to a temporary file in the output location
  #

  output_path = output_dir + '/' + output_filename
  temp_output_path = output_dir + '/_' + output_filename

  if opt_dest_host == 'localhost':
    command = 'cp ' + input_path + ' ' + temp_output_path
  else:
    command = opt_copy + ' ' + input_path + ' ' + opt_dest_host + ':' + \
              temp_output_path

  if opt_debug:
    print "Executing command: " + command

  os.system(command)

  #
  # Move the temporary file to its final location
  #

  if opt_dest_host == 'localhost':
    command = 'mv ' + temp_output_path + ' ' + output_path
  else:
    command = shell_cmd + ' ' + opt_dest_host + '"mv ' + temp_output_path + \
              ' ' + output_path + '"'

  if opt_debug:
    print "Executing command: " + command

  os.system(command)


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
  opt_copy = 'scp'
  opt_dest_dir = 'incoming/ncwf'
  opt_dest_host = 'ftp.rap.ucar.edu'
  opt_dest_pwd = 'ncwf@ftb1.kc.noaa.gov'
  opt_dest_user = 'anonymous'
  opt_input_dir = '.'
  opt_instance = 'test'
  opt_max_valid_age = -1
  opt_push_ldata = 0
  opt_sleep_secs = 2

  #
  # Get the command line arguments.
  #

  optlist, args = getopt.getopt(sys.argv[1:], 'dhp', \
        [ 'debug', 'help', \
          'dest_dir=', 'dest_host=', 'dest_pwd=', 'dest_user=', \
          'input_dir=', 'instance=', \
          'max_valid_age=', 'push_ldata', \
          'sleep_secs=' ])

  print optlist
  print args

  for opt in optlist:
    if opt[0] == "-h" or opt[0] == "--help":
      print_usage()
      sys.exit()

    if opt[0] == "-d" or opt[0] == "--debug":
      opt_debug = 1

    if opt[0] == "--copy":
      opt_copy = opt[1]
      if opt_copy == 'rcp':
        shell_cmd = 'rsh'
      else:
        shell_cmd = 'csh'

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

    if opt[0] == "--push_ldata":
      opt_push_ldata = 1

    if opt[0] == "--sleep_secs":
      opt_sleep_secs = opt[1]

  if (opt_debug):
    print "opt_debug = ", opt_debug
    print "opt_copy = ", opt_copy
    print "shell_cmd = ", shell_cmd
    print "opt_dest_dir = ", opt_dest_dir
    print "opt_dest_host = ", opt_dest_host
    print "opt_dest_pwd = ", opt_dest_pwd
    print "opt_dest_user = ", opt_dest_user
    print "opt_input_dir = ", opt_input_dir
    print "opt_instance = ", opt_instance
    print "opt_max_valid_age = ", opt_max_valid_age
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


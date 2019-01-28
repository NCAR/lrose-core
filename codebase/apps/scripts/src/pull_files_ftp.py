#!/usr/bin/env python2

##############################################################
# pull_files_ftp.py                                          #
##############################################################
# Python script for pulling files from another machine using #
# FTP.                                                       #
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
  print "   --source_host <host>    : Source host name"
  print "                             (default = ", opt_source_host, ")"
  print "   --source_pwd <password> : Source login password"
  print "                             (default = ", opt_source_pwd, ")"
  print "   --source_user <login>   : Source login to use"
  print "                             (default = ", opt_source_user, ")"
  print "   --source_dir <dir>      : Source directory"
  print "                             (default = ", opt_source_dir, ")"
  print "   --source_ext <ext>      : File extension to check for"
  print "                             (default = ", opt_source_ext, ")"
  print "   --source_substr <str>   : Only files in the source directory that"
  print "                             contain this substring will be processed."
  print "                             (default = ", opt_source_substr, ")"
  print "   --source_exclude_str <str> : Files in the source directory that"
  print "                             contain this substring will not be processed."
  print "                             (default = ", opt_source_exclude_str, ")"
  print "   --source_exclude_prefix <str> : Files in the source directory that"
  print "                             start with this substring will not be processed."
  print "                             (default = ", opt_source_exclude_prefix, ")"
  print "   --wait_for_quiescence <secs> : Wait for the file to be quiescent"
  print "                             before processing it.  The file size is"
  print "                             checked every <secs> seconds until it"
  print "                             doesn't change."
  print "   --delete                : Delete the file on the FTP site after"
  print "                             pulling it down."
  print "   --output_dir <dir>      : Local directory for storing data"
  print "                             (default = ", opt_output_dir, ")"
  print "   --write_Ldata           : Option to write latest_data_info fiile"
  print "   --Ldata_ext <ext>       : File extension written to latest_data_info_file"
  print "                             (default = ", opt_Ldata_ext, ")"
  print "   --Ldata_dtype <type>    : File data type used for LdataWriter"
  print "                             (default = ", opt_Ldata_dtype, ")"
  print "   --temp_dir <dir>        : Local directory for storing data while it downloads."
  print "                             (default = --output_dir)"
  print "   --instance <instance>   : PMU instance name (default = ", \
	opt_instance, ")"
  print "   -p | --passive          : Use passive mode in FTP"
  print "   --register_interval <secs>"
  print "                           : Number of seconds between registering"
  print "                             with the process mapper."
  print "                             (default = ", opt_register_interval, ")"
  print "   --sleep_secs <secs>     : Number of seconds to sleep between"
  print "                             checks for new data. (default = ", \
        opt_sleep_secs, ")"      
  print "   --die                   : flag causes script to die after one iteration."
  print "   --noDot                 : flag causes script to check for files that have no dot."
  print "                             if not set, script checks for '*.%s'" % opt_source_ext
  print "   -d | --debug            : Print debug messages"

  return


###############################################################
# delete_file(): Delete the given file from the FTP site
#

def delete_file(filename):

  if opt_debug:
    print "Deleting file ", filename, " from FTP site"
    
  try:
    ftp.delete(filename)
    if opt_debug:
      print "    File successfully deleted"
  except:
    print "Error deleting file ", filename, " from FTP site"
    print "Continuing without cleaning up"
    
  return


###############################################################
# pull_file(): Pull the given file to the appropriate destination.
#

def pull_file(filename):
  pmu.auto_register("Checking for new files")

  #
  # Construct the local filenames.  The temporary filename is used
  # for pulling the file down.  When the retrieval is complete, the
  # file is renamed to its original name so that it appears atomically.
  #

  temp_filename = opt_temp_dir + '/.' + filename
  local_filename = opt_output_dir + '/' + filename

  if opt_debug:
    print "local_filename = " + local_filename
    print "temp_filename = " + temp_filename

  #
  # Check to see if we already have this file.  If we do, then skip
  # it.
  #

  local_gz_filename = local_filename + ".gz"

  #
  # this is the case of the file that is gzipped on the ftp site
  # but unzipped locally
  #
  if(os.path.splitext(local_filename)[1] == ".gz"):
    if os.path.exists( os.path.splitext(local_filename)[0] ):
      if opt_debug:
        print
        print "File already exists locally: " + local_filename
        print "Skipping retrieval...."
      return

  if os.path.exists(local_filename) or os.path.exists(local_gz_filename):
    if opt_debug:
      print
      print "File already exists locally: " + local_filename
      print "Skipping retrieval...."
    return

  if opt_debug:
    print "Getting file <" + filename + ">"

  #
  # Wait for the file to be quiescent
  #

  if opt_wait_for_quiescence:
    prev_file_size = 0
    file_size = ftp.size(filename)
    while file_size != prev_file_size:
      if opt_debug:
        print "Waiting for file quiescence..."
      time.sleep(opt_quiescence_secs)
      prev_file_size = file_size
      file_size = ftp.size(filename)
      
  #
  # Pull the data file to the destination directory.
  #

  temp_file = open(temp_filename, 'wb')

  pmu.force_register("Retrieving file " + filename)

  try:
    ftp.retrbinary('RETR ' + filename, temp_file.write)
  except (socket.error):
    print "Socket error occurred while trying to transfer file: ", filename
    print "Skipping file!"
    return

  temp_file.close()

  if(os.path.splitext(temp_filename)[1] == ".gz"):
    filename = os.path.splitext(filename)[0]
    os.system("gunzip " + temp_filename)
    temp_filename = os.path.splitext(temp_filename)[0]
    local_filename = os.path.splitext(local_filename)[0]

  try:
    shutil.copyfile(temp_filename, local_filename)
  except(OSError):
    print "Error Copying temp file to local file\n"

  if(opt_write_Ldata):
    file_stats = os.stat(local_filename)
    data_file_time = datetime.fromtimestamp(file_stats[8])
    Ldata_command = "LdataWriter -dir " + opt_output_dir + " -ext " + opt_Ldata_ext + " -dtype " + opt_Ldata_dtype + " -ltime " + data_file_time.strftime("%Y%m%d%H%M%S") + " -rpath ./" + filename
    print
    print "LdataWriter command line:"
    print Ldata_command
    print
    os.system(Ldata_command)
  
  try:
    os.remove(temp_filename)
  except(OSError):
    print "Error removing temp file\n"
    
  return


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
  opt_source_host = 'ftp.rap.ucar.edu'
  opt_source_pwd = 'test@ucar.edu'
  opt_source_user = 'anonymous'
  opt_source_dir = 'incoming/irap'
  opt_source_ext = 'ext'
  opt_source_substr = ''
  opt_source_exclude_str = ''
  opt_source_exclude_prefix = ''
  opt_wait_for_quiescence = 0
  opt_quiescence_secs = -1
  opt_delete = 0
  opt_output_dir = '.'
  opt_temp_dir = ''
  opt_instance = 'test'
  opt_max_valid_age = -1
  opt_passive = 0
  opt_register_interval = 60
  opt_sleep_secs = 2
  opt_die = 0
  opt_noDot = 0
  opt_write_Ldata = 0
  opt_Ldata_ext = 'nc'
  opt_Ldata_dtype = 'netCDF'

  #
  # Get the command line arguments.
  #

  optlist, args = getopt.getopt(sys.argv[1:], 'dhp', \
        [ 'debug', 'help', \
          'source_host=', 'source_pwd=', 'source_user=', \
          'source_dir=', 'source_ext=', \
          'source_substr=', 'source_exclude_str=', \
          'source_exclude_prefix=', \
          'wait_for_quiescence=', 'delete', \
          'output_dir=', 'temp_dir=', 'instance=', \
          'max_valid_age=', 'passive', \
          'register_interval=', 'sleep_secs=', 'die', 'noDot', 'write_Ldata',
          'Ldata_ext=', 'Ldata_dtype='])

  for opt in optlist:
    if opt[0] == "-h" or opt[0] == "--help":
      print_usage()
      sys.exit()

    if opt[0] == "-d" or opt[0] == "--debug":
      opt_debug = 1

    if opt[0] == "--source_host":
      opt_source_host = opt[1]

    if opt[0] == "--source_pwd":
      opt_source_pwd = opt[1]

    if opt[0] == "--source_user":
      opt_source_user = opt[1]

    if opt[0] == "--source_dir":
       opt_source_dir = opt[1]

    if opt[0] == "--source_ext":
      opt_source_ext = opt[1]

    if opt[0] == "--source_substr":
      opt_source_substr = opt[1]

    if opt[0] == "--source_exclude_str":
      opt_source_exclude_str = opt[1]

    if opt[0] == "--source_exclude_prefix":
      opt_source_exclude_prefix = opt[1]

    if opt[0] == "--wait_for_quiescence":
      opt_wait_for_quiescence = 1
      try:
        opt_quiescence_secs = int(opt[1])
      except (ValueError):
        print "Error: wait_for_quiescence arguments must be an integer"
        print "Using 5 seconds"
        opt_quiescence_secs = 5

    if opt[0] == "--delete":
      opt_delete = 1

    if opt[0] == "--output_dir":
      opt_output_dir = opt[1]

    if opt[0] == "--temp_dir":
      opt_temp_dir = opt[1]

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
        opt_sleep_secs = int(opt[1])
      except (ValueError):
        print "Error: sleep_secs arguments must be an integer"
        print "Using 2 seconds"
        opt_sleep_secs = 2

    if opt[0] == "--die":
      opt_die = 1

    if opt[0] == "--noDot":
      opt_noDot = 1

    if opt[0] == "--write_Ldata":
      opt_write_Ldata = 1

    if opt[0] == "--Ldata_ext":
      opt_Ldata_ext = opt[1]

    if opt[0] == "--Ldata_dtype":
      opt_Ldata_dtype = opt[1]

  if opt_debug:
    print "opt_debug = ", opt_debug
    print "opt_source_host = ", opt_source_host
    print "opt_source_pwd = ", opt_source_pwd
    print "opt_source_user = ", opt_source_user
    print "opt_source_dir = ", opt_source_dir
    print "opt_source_ext = ", opt_source_ext
    print "opt_source_substr = ", opt_source_substr
    print "opt_source_exclude_str = ", opt_source_exclude_str
    print "opt_source_exclude_prefix = ", opt_source_exclude_prefix
    print "opt_delete = ", opt_delete
    print "opt_output_dir = ", opt_output_dir
    print "opt_temp_dir = ", opt_temp_dir
    print "opt_instance = ", opt_instance
    print "opt_max_valid_age = ", opt_max_valid_age
    print "opt_passive = ", opt_passive
    print "opt_register_interval = ", opt_register_interval
    print "opt_sleep_secs = ", opt_sleep_secs
    print "opt_die = ", opt_die
    print "opt_noDot = ", opt_noDot
    print "opt_write_Ldata = ", opt_write_Ldata
  #
  # Initialize process mapper registration
  #

  pmu.auto_init(prog_name, opt_instance, opt_register_interval)

  #
  # Make sure that the output directory exists
  #

  try:
    os.makedirs(opt_output_dir)
  except (os.error):
    pass

  # if the temp dir is not set then it defaults to the
  # output directory which would give the old behavior
  # of this script.
  if opt_temp_dir == '':
    opt_temp_dir = opt_output_dir
  else:
    try:
      os.mkdir(opt_temp_dir)
    except (os.error):
      pass

  #
  # Wait for new files.  When found, push them to the destination.
  #

  need_sleep = 0

  dot_string = '.'
  if opt_noDot:
    dot_string = ''
    
  request_string = '*' + dot_string + opt_source_ext

  if opt_debug:
    print "Request_string: %s" % request_string
    
  while 1:

    pmu.auto_register("Waiting for data")

    if need_sleep:
      sleep_secs_left = opt_sleep_secs
      while sleep_secs_left > 0:
        pmu.auto_register("Waiting for data")
        time.sleep(2)
        sleep_secs_left = sleep_secs_left - 2

    need_sleep = 1

    #
    # Open the ftp connection
    #

    ftp = ftplib.FTP(opt_source_host, opt_source_user, opt_source_pwd)

    try:
      if opt_passive:
        ftp.set_pasv(1)
      else:
        ftp.set_pasv(0)
    except (socket.error):
      print "Socket error occurred while trying to set passive mode"
      continue

    #
    # Move to the appropriate input directory
    #

    try:
      if opt_debug:
        print
        print "Checking directory ", opt_source_dir
      ftp.cwd(opt_source_dir)
    except ftplib.all_errors,resp:
      print "ERROR: ", str(resp)
      continue

    #
    # Get the directory listing
    #

    try:
      listing = ftp.nlst(request_string)
    except ftplib.all_errors, resp:
      error_code = string.split( str(resp) )[0]
      print "ERROR: ", str(resp)      

      if error_code == "550":

        # No files are found so check for a year directory
        year =  time.gmtime( time.time() )[0]
        new_source_dir = opt_source_dir + "/" + str(year)
        try:
          if opt_debug:
            print "Checking in directory ", new_source_dir  
          ftp.cwd( str(year) )
          listing = ftp.nlst(request_string)
        except ftplib.all_errors,resp:
          error_code = string.split( str(resp) )[0]
          print "ERROR: ", str(resp)

          # May be first of year but files are still being
          # written to last years directory so look back one day
          if string.split( str(resp) )[1] == ( str(year) + ":" ):

            year =  time.gmtime( time.time() - 86400 )[0]
            new_source_dir = opt_source_dir + "/" + str(year)
            try:
              if opt_debug:
                print "Cheking in directory ", new_source_dir  
              ftp.cwd( str(year) )
              listing = ftp.nlst(request_string)
            except ftplib.all_errors,resp:
              error_code = string.split( str(resp) )[0]
              print "ERROR: ", str(resp)

              # File or directory not found. Try looking for the
              # year/julian_day directory from one day ago.
              try:
                year =  time.gmtime( time.time() - 86400 )[0]
                julian_date = time.gmtime( time.time() - 86400 )[7]

                if len( str( julian_date ) ) == 1:
                    julian_date = "00" + str(julian_date)
                    
                if len( str( julian_date ) ) == 2:
                    julian_date = "0" + str(julian_date)

                new_source_dir = opt_source_dir + "/" + str(year) + "/" + str(julian_date) 

                if opt_debug:
                  print "Cheking in directory ", new_source_dir  

                ftp.cwd( str(julian_date) )
                listing = ftp.nlst(request_string)

              except ftplib.all_errors,resp:
                error_code = string.split( str(resp) )[0]
                print "ERROR: ", str(resp)
                # give up on this search
                continue

          # Directory was found but no files with search string in it
          # Try looking for the julian day.
          else:

            try:
              year =  time.gmtime( time.time() )[0]
              julian_date = time.gmtime( time.time() )[7]

              if len( str( julian_date ) ) == 1:
                  julian_date = "00" + str(julian_date)
                    
              if len( str( julian_date ) ) == 2:
                  julian_date = "0" + str(julian_date)

              new_source_dir = opt_source_dir + "/" + str(year) + "/" + str(julian_date) 

              if opt_debug:
                print "Cheking in directory ", new_source_dir  

              ftp.cwd( str(julian_date) )
              listing = ftp.nlst(request_string)

            except ftplib.all_errors,resp:
              error_code = string.split( str(resp) )[0]
              print "ERROR: ", str(resp)

              # Look back one day. Could have past 0Z but files still being
              # written to yesterdays directory.
              try:
                year =  time.gmtime( time.time() - 86400 )[0]
                julian_date = time.gmtime( time.time() - 86400 )[7]

                if len( str( julian_date ) ) == 1:
                    julian_date = "00" + str(julian_date)
                    
                if len( str( julian_date ) ) == 2:
                    julian_date = "0" + str(julian_date)
                
                new_source_dir = opt_source_dir + "/" + str(year) + "/" + str(julian_date) 

                if opt_debug:
                  print "Cheking in directory ", new_source_dir  

                ftp.cwd( str(julian_date) )
                listing = ftp.nlst(request_string)

              except ftplib.all_errors,resp:
                error_code = string.split( str(resp) )[0]
                print "ERROR: ", str(resp)
                # give up on this search
                continue

      else:
        print "ERROR: ", str(resp)
        continue

    if opt_debug:
      print "Found ", len( listing ), " file(s)."
      print listing

    #
    # Retrieve all files that aren't specified to be excluded
    #
    if opt_debug:
      print "Retrieving requested files that are not already in local directory"

    for file in listing:
      do_pull_file = 1
      
      if opt_source_exclude_str != '' and \
         string.find(file, opt_source_exclude_str) != -1:
        do_pull_file = 0

      if opt_source_exclude_prefix != '' and \
         string.find(file, opt_source_exclude_prefix) == 0:
        do_pull_file = 0
        print "Excluding prefix file: ", file

      if opt_source_substr != '' and \
         string.find(file, opt_source_substr) == -1:
        do_pull_file = 0

      if do_pull_file:
        pull_file(file)
        if opt_delete:
          delete_file(file)
      else:
        print "*** File excluded: ", file

    #
    # Close the FTP connection
    #

    ftp.quit()

  pmu.auto_unregister()

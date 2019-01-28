#!/usr/bin/env python2

##############################################################
# pull_ASAP_ftp_files.py                                    # 
##############################################################
# Python script for pulling files from an FTP site           #
##############################################################


import os
from os import path

import sys
import ftptools
import time
import signal
import getopt
import string

import datetime
import pmu
from time import sleep


flist = []

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
  print "   --ftp_dir <dir>         : FTP data directory"
  print "                             (default = ", opt_ftp_dir, ")"
  print "   --ftp_host <host>       : FTP server host name"
  print "                             (default = ", opt_ftp_server, ")"
  print "   --ftp_pwd <password>    : FTP server login password"
  print "                             (default = ", opt_ftp_passwd, ")"
  print "   --ftp_user <login>      : FTP server login name "
  print "                             (default = ", opt_ftp_user, ")"
  print "   --output_dir <dir>      : data directory for output"
  print "                             (default = ", opt_output_dir, ")"
  print "   --instance <instance>   : PMU instance name (default = ", \
    opt_instance, ")"
  print "   --sleep <secs>          : Number of seconds to sleep between"
  print "                             tries. "
  print "                             (default = ", opt_sleep_secs, ")"
  print "   --tries <number>        : Maximum number of attemps to"
  print "                             grab a directory listing. "
  print "                             Script will terminate if doesn't "
  print "                             get a listing for sleep seconds "
  print "                             * number of tries"
  print "                             (default = ", opt_max_num_tries, ")"
  print "   --unzip                 : Uncompress FTP files using gunzip"
  if opt_unzip:
    print "                             (default = uncompress files", ")"
  else:
    print "                             (default = leave files compressed", ")"
  print "   --local                 : set if system clock is set to"
  print "                             local time instead of GMT. "
  if opt_gmt_time:
    print "                             (default = GMT ", ")"
  else:
    print "                             (default = Local time", ")"
  print "   -d | --debug            : Print debug messages"

  return

#########################################################################
# handler(signum, frame): handler called by signal.alarm used to detect
#                         when ftp listing is hung
#

def handler(signum, frame):
  print 'Timeout!'


#########################################################################
# getTimeStamp(): creates the time tuple from the filename
#
# outputs unix time 
#
#   Input files follow the follwoing
#       20040816_1330UTC_CINowcast.nc.gz

def getTimeStamp(file_name):

   time_tuple = (string.atoi(f[0:4]),
                           string.atoi(f[4:6]),
                           string.atoi(f[6:8]),
                           string.atoi(f[9:11]),
                           string.atoi(f[11:13]), 0, 0, 0, 0)

   return time.mktime(time_tuple),



#########################################################################
# getCurrentJulianday(): gets ldataname filename and output filename
#

def getCurrentJulianday():
    

    curr_time = time.time()

    # is machine already on gmt time?
    if opt_gmt_time:
      curr_time_tuple = time.localtime(curr_time)
    else:
      curr_time_tuple = time.gmtime(curr_time)

    year = curr_time_tuple[0] 

    subdir = '%02d%03d' %  (year%2000, curr_time_tuple[7])

    #print subdir

    return subdir

 
#########################################################################
# get_output_path(dir, file_time): gets output filename based on the
#   dir -- directory prefix and
#   a file_time object the goes naming convention:
#   Input files follow the follwoing 
#       20040816_1330UTC_CINowcast.nc.gz
#
#   output path follows the RAP file naming convention:
#         day - directory_path/YYYYMMDD/HHMM.reg

#

def get_output_path(dir, flname):

    subdir = flname[0:8]

    hour   = flname[9:11]
    minute = flname[11:13]

    flname = hour + minute + "00.nc.gz"

    pathname = path.join(dir, subdir, flname)

    #
    # make subdirectory
    #
    command = "mkdir -p " + path.join(dir, subdir)

    print "mkdir command = "
    print command
    print " "

    os.system(command)

    if opt_debug:
      print pathname

    #print pathname
    return subdir, pathname, flname

#########################################################################
# get_listingn(): gets ftp listing
#

def get_listing():
   
    #
    # get new time stamp for next file 
    #
    got_list = 0
    tries = 0
    while not got_list:
      tries = tries + 1
      got_list = 1
      try:
        signal.alarm(100)
        #ftp_info.get_filtered_listing("g8.*")
        ftp_info.get_listing()
        signal.alarm(0)
      except:
        print "ftp date error: %s, %s" % (sys.exc_type, sys.exc_value)
        got_list = 0
        if tries > 25:
          print "Exiting after 25 tries"
          sys.exit()

    return 1


###############################################################
# Main program                                                #
###############################################################

if __name__ == '__main__':

  prog_name = os.path.basename(sys.argv[0])   

  #
  # Initialize the command line arguments.
  #

  opt_debug = 1
  opt_ftp_user = "anonymous"
  opt_ftp_passwd = "ocnd@rap.ucar.edu"

  opt_ftp_server = "ftp.ssec.wisc.edu"
  opt_ftp_dir = "asap/products/convective_signatures"
  opt_output_dir = os.environ['OCND_DATA_DIR'] + "/raw/ASAP/convSig"

  opt_unzip = 1
  opt_gmt_time = 1
  opt_log = ""
  opt_instance = 'test'
  opt_sleep_secs = 300
  opt_max_num_tries = 23
 
  new_timestamp = 0
  last_timestamp = 0

  #
  # Get the command line arguments.
  #

  optlist, args = getopt.getopt(sys.argv[1:], 'dh', \
        [ 'debug', 'help', \
        'ftp_dir=', 'ftp_host=', 'ftp_pwd=', 'ftp_user=', \
        'output_dir=', 'instance=', 'satellite=', \
        'local=', \
        'tries=', \
        'sleep=', ])

  print (optlist)
  for opt in optlist:
    if opt[0] == "-h" or opt[0] == "--help":
      print_usage()
      sys.exit()
    if opt[0] == "-d" or opt[0] == "--debug":
      opt_debug = 1

    if opt[0] == "--ftp_dir":
      opt_ftp_dir = opt[1]
    if opt[0] == "--ftp_host":
      opt_ftp_server = opt[1]
    if opt[0] == "--ftp_pwd":
      opt_ftp_passwd = opt[1]
    if opt[0] == "--ftp_user":
      opt_ftp_user = opt[1]
    if opt[0] == "--output_dir":
      opt_output_dir = opt[1]
    if opt[0] == "--unzip":
      opt_unzip = 1
    if opt[0] == "--local":
      opt_gmt_time = 0
    if opt[0] == "--instance":
      opt_instance = opt[1]
    if opt[0] == "--sleep":
      opt_sleep_secs = opt[1]
    if opt[0] == "--tries":
      opt_max_num_tries = opt[1]

  #
  # ouput the argument list
  #

  if opt_debug:
    print opt_ftp_server
    print opt_ftp_user
    print opt_ftp_passwd
    print opt_ftp_dir
    print opt_instance
    print opt_output_dir
    print "debug set to ", + opt_debug

  #
  # Initalize signal handler
  #
  signal.signal(signal.SIGALRM, handler)

  #
  # Initialize process mapper registration
  #

  pmu.auto_init(prog_name, opt_instance, 60)

  print opt_ftp_server
  print opt_ftp_dir
  print opt_ftp_user
  print opt_ftp_passwd
  print opt_log

  old_subDir = "junk"

  last_timestamp = -1

  while 1:

    sub_directory = getCurrentJulianday()

    if sub_directory != old_subDir:
      ftp_dir = path.join (opt_ftp_dir, sub_directory)
      print ftp_dir


      ftp_info = ftptools.FTP_info(opt_ftp_server, ftp_dir, opt_ftp_user, opt_ftp_passwd, opt_log)

    pmu.auto_register("getting ftp file listing.")
    got_list = 0 
    tries = 0
    while not got_list:
      tries = tries + 1
      got_list = 1
    try:

      signal.alarm(100)
      ftp_info.get_listing()
      signal.alarm(0)
    except:
      print "ftp date error: %s, %s" % (sys.exc_type, sys.exc_value)
      got_list = 0
      if tries > opt_max_num_tries:
        print "Exiting after ",  opt_max_num_tries, " tries"
        sys.exit()

    if ftp_info.listing != 0:

      for f in ftp_info.listing :

        new_timestamp = getTimeStamp (f)

        if new_timestamp  > last_timestamp:

          print "fetching file ", f

          sub_dir, outputPath, filename = get_output_path(opt_output_dir, f)

          print "Making sure the ftp file is of constant size."
          pmu.auto_register("Making sure the ftp file is of constant size.")
          ftp_info.size(f)

          ftp_info.fetch(f, outputPath)

          if opt_unzip:
            command = 'gunzip -f %s' % outputPath

          YYYYMMDDHHSS = f[0:4] + f[4:6] + f[6:8] + f[9:11] + f[11:13] +"00"

          print command
          os.system (command)

          command = 'LdataWriter -dtype raw -dir %s -ext %s -info1 %s -info2 %s -ltime %s' % \
                          (opt_output_dir, "nc", YYYYMMDDHHSS[8:], YYYYMMDDHHSS[8:] + '.nc', YYYYMMDDHHSS)

          print command
          os.system (command)

          last_timestamp = new_timestamp


    pmu.auto_register("waiting for data, sleeping.")
    sleep (opt_sleep_secs)



#!/usr/bin/env python2

##############################################################
# pull_files_ftp.py                                          #
##############################################################
# Python script for pulling files from another machine using 
# FTP or WGET.  Here is the usage: 
#                           
# Using FTP: 
# 
# First use FTP list all remote files on the remote machine.
# Then, download loop through these remote files to download
# them, one at a time. 
#   set following command parameters:
#     --source_host (remote FTP host)
#     --source_pwd  (FTP password)
#     --source_user (FTP user)
#     --source_dir  (remote data directory)
#     --output_dir  (local data directory)
#     -p | --passive (true if passive mode)
#   Do NOT set following command parameters:
#     --source_url_base (used for Wget only)
# 
#   To generate latest data info (triggering XML) files, set:
#     --generate_ldata_info (latest data info trigger file)
#         
#                    
# Using Wget (only work for .nc data files): 
# 
# Wget will use connect to given URL and check if each remote
# file is newer than its local copy. If true (including no
# local copy) Wget will download it. Otherwise, Wget will
# skip that file. To run using Wget, provide URL via:
#   --source_url_base <url>
#   --output_dir <dir>
# To generate latest data info trigger XML file, set:
#   --generate_ldata_info
# "source_xxx" parameters will be ignored
# 
# 
# Common for both FTP and Wget:
# 
#   --run_date: rerun data for a given date, format: yyyy/MM/dd
#   --register_interval <secs>: Proc Mapper heatbeat registration 
#   --sleep_secs <secs>: sleep time of this process
#   --instance <instance>: Proc Mapper registration instance name
##############################################################

import ftplib
import getopt
import os
import socket
import stat
import string
import sys
import time
import pmu
import filecmp
from datetime import date
from time import gmtime, strftime

###############################################################
# Local subroutines                                           #
###############################################################

###############################################################
# print_usage(): Print the usage for this script
###############################################################

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
  print "   --output_dir <dir>      : Local directory for storing data"
  print "                             (default = ", opt_output_dir, ")"
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
  print "   --generate_ldata_info   : flag causes script to generate latest data info files."
  print "   --data_type <type>      : Data type to use in the ldata info file."
  print "                             Used only if --generate_ldata_info is included."
  print "                             (default = ", opt_data_type, ")"
  print "   --source_url_base <url> : URL for wget to pull remote file that is newer than local copy."
  print "   --run_date              : date for a single run in the format of yyyy/MM/dd"
  print "   --use_output_dir_only   : Use given output dir without adding date subdir."
  print "   --use_source_url_base_only   : Use given source URL without adding date subdir."

  return

###############################################################
# pull_file(): download the given file (filename) via FTP
# 
###############################################################

def pull_file_ftp(file_ext):
  #
  # Create the output directory
  #
  create_dir(opt_output_dir)

  pmu.force_register("Listing file from output dir:" + opt_output_dir)

  #
  # construct a list of all gz files (used by wget to reject
  #
  files = os.listdir(opt_output_dir)
  print "list files"
  file_time_dict = {}
  rejectNameList = ""
  for f in files:
    if f.split(".")[len(f.split("."))-1] == "gz":
      zfile = f.split(".")[0]
      if opt_debug:
        print "gzipped file: ", zfile
      if zfile.isdigit:
        if rejectNameList == "":
          rejectNameList = zfile
        else:
          rejectNameList = rejectNameList + "," + zfile
    else:
      if opt_debug:
        print "A non gz file: ", f
      if f.startswith("_"):
        if opt_debug:
          print "A file starts with _: ", f
      else:
        file_path = opt_output_dir + "/" + f
        stats = os.stat(file_path)
	file_time_dict[f] = time.localtime(stats[8])

  #
  # Pull the data file to the destination directory.
  #

  pmu.force_register("Retrieving file from remote dir: " + opt_source_dir)

  try:
    formattedSourceDir = "/" + opt_source_dir
    if opt_source_dir.startswith("/"):
      formattedSourceDir = opt_source_dir
    if not formattedSourceDir.endswith("/"):
      formattedSourceDir = formattedSourceDir + "/"
    numCutDir = formattedSourceDir.count("/") - 1
    command = "wget -N -r -l1 --ftp-user=" + opt_source_user + " --ftp-password=" + opt_source_pwd + " --cut-dirs " + str(numCutDir) + " -nH -P" + opt_output_dir + "/ ftp://" + opt_source_host + formattedSourceDir
    if rejectNameList != "":
      command = command + " -R " + rejectNameList
    print command
    os.system(command) 
  except (socket.error):
    print "Error occurred while trying to transfer file: ", filename 
    print "Skipping file!"
    return

  #
  # set trigger file only when data files are updated.
  #
  if opt_generate_ldata_info :  
    set_trigger_newerFTP(opt_output_dir, file_time_dict, file_ext)

  return

###############################################################
# set_trigger
# file_name - the trigger file name
# extension - for -ext of LdataWriter e.g. nc or grb (optional)
# date_str - for -ltime of LdataWriter. Date/Time in XML file.
#            (optional)
###############################################################    
def set_trigger(filename, extension, date_str):

  pmu.force_register("Updating trigger file " + filename)

  print "create _latest data for file", filename
  command = "LdataWriter -dir " + opt_output_dir \
          + " -rpath " + filename + " -dtype " + opt_data_type
  if extension != "":
    command = command + " -ext " + extension
  if date_str != "":
    command = command + " -ltime " + date_str
  os.system(command)
  print command

  pmu.force_register("Updated trigger filename " + filename)

  return

###############################################################
# create given dir (dir_name)                                           
###############################################################
def create_dir(dir_name):
  #
  # Make sure that the output directory exists
  #

  if not os.path.exists(dir_name):
    try:
      os.makedirs(dir_name)
    except (os.error):
      print "failed to create output dir: ", dir_name
      pass
  else:
    if (opt_debug):
      print "output dir already exists: ", dir_name

###############################################################
# download_url
# 
# Download files using Wget. Set download date to today if
# opt_date_str (for re-run a specific date) is not set. 
# time_threshold is used to keep track latest updated file
# date/time so that we only create latest info XML trigger
# file for newly updated data file. Generate latest info
# XML trigger file if opt_generate_ldata_info is true.
# 
# Note: only work for .nc data files.
###############################################################  
def download_url (time_threshold, file_ext):
  date_str = opt_date_str
  if opt_date_str == "" :
    today = date.today()
    date_str = today.strftime("%Y/%m/%d")
  url = opt_source_base_url + date_str + "/"
  if opt_use_source_base_url_only:
    url = opt_source_base_url
  date_str_flat = date_str.replace("/","")

  output_dir = opt_output_dir + "/"+ date_str_flat + "/"   
  if opt_use_output_dir_only:
    output_dir = opt_output_dir + "/" 

  create_dir(output_dir)

  if time_threshold==0 :
    time_threshold = get_latest_file_time(output_dir, file_ext)

  pull_file_wget(url, output_dir, file_ext)

  if opt_generate_ldata_info :
    set_trigger_newer(output_dir, time_threshold, date_str_flat, file_ext)
  
  time_threshold = get_latest_file_time(output_dir, file_ext)
  return time_threshold

###############################################################
# get_latest_file_time
# return latest file update time in the given dir (folder)
# Note: only apply to .nc files
###############################################################
def get_latest_file_time(folder, file_ext):

  pmu.force_register("get latest time " + folder)

  files = os.listdir(folder)
  print "list files"
  current_time = 0
  for f in files:
    if f.split(".")[len(f.split("."))-1] == file_ext:
      file_path = folder + f
      stats = os.stat(file_path)
      lastmod_date = time.localtime(stats[8])
      if current_time < lastmod_date:
        print "older than today", f
        current_time = lastmod_date
      else:
        print "newer than today", f
    else:
      print "non NC file: ", f

  print "Newest file time=", current_time
  pmu.force_register("got latest time " + folder)
  return current_time

###############################################################
# set_trigger
# create latest info XML trigger file
# Only applicable to .nc files.
###############################################################
def set_trigger_newer(folder, threshold_time, date_str_flat, file_ext):

  pmu.force_register("Updating trigger folder " + folder)

  try:
    files = os.listdir(folder)
  except OSError:
    return None

  for f in files:
    if f.split(".")[len(f.split("."))-1] == file_ext:
      file_path = folder + f
      info_file_path = date_str_flat+"/" + f
      if opt_use_output_dir_only:
        info_file_path = f
      stats = os.stat(file_path)
      lastmod_date = time.localtime(stats[8])
      if lastmod_date > threshold_time:
        print "Found a new file", f
        command = "LdataWriter -dir " + opt_output_dir \
                + " -rpath " + info_file_path + " -dtype nc -ext " + file_ext + " -ltime " + date_str_flat + "000000"
        os.system(command)
    else:
      print "set_trigger ignores non NC file: ", f

  pmu.force_register("Updated trigger folder " + folder)

  return

###############################################################
# set_trigger
# create latest info XML trigger file
# Only applicable to .nc files.
###############################################################
def set_trigger_newerFTP(folder, file_time_dict, file_ext):

  pmu.force_register("Updating trigger folder " + folder)

  try:
    files = os.listdir(folder)
  except OSError:
    return None

  for f in files:
    if f.split(".")[len(f.split("."))-1] != "gz" and f.isdigit():
      file_path = folder + "/" + f
      info_file_path = f
      stats = os.stat(file_path)
      lastmod_date = time.localtime(stats[8])
      if f not in file_time_dict or lastmod_date > file_time_dict[f]:
	datetime_str_flat = time.strftime("%Y%m%d%H%M%S", lastmod_date)
        print "Found an updated file ", f, ", lastmod_date=", datetime_str_flat
	if f in file_time_dict:
          print "And file: ", f, ", org_lastmod_date=", time.strftime("%Y%m%d%H%M%S", file_time_dict[f])
        command = "LdataWriter -dir " + opt_output_dir \
                + " -rpath " + info_file_path + " -dtype nc -ext " + file_ext + " -ltime " + datetime_str_flat
        os.system(command)
    else:
      print "set_trigger_newerFTP ignores non NC file: ", f

  pmu.force_register("Updated trigger folder " + folder)

  return

###############################################################
# pull_file(): Download .nc data file from given url to 
# output_base_dir. Will only download remote files that are
# either newer than the local counterpart or there is no
# local counterpart at all.
###############################################################

def pull_file_wget(url, output_base_dir, file_ext):
  if opt_debug:
    print "Getting URL <" + url + ">"
    print "Putting dir <" + output_base_dir + ">"

  #
  # Pull the data file to the destination directory.
  #

  pmu.force_register("Retrieving URL " + url)

  try:
    command = "wget --timestamping -A \'00." + file_ext + "\' -e robots=off -r -l1 -np --cut-dirs 6 -nH -P"+ output_base_dir + " " + url
    print command
    os.system(command)
  except (socket.error):
    print "Error occurred while trying to transfer URL: ", url
    print "Skipping file!"
    return

  pmu.force_register("Retrieved URL " + url)

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

  opt_debug = 1 
  opt_source_host = 'ftp.rap.ucar.edu'
  opt_source_pwd = 'test@ucar.edu'
  opt_source_user = 'anonymous'
  opt_source_dir = 'incoming/irap'
  opt_source_ext = ''
  opt_output_dir = '.'
  opt_instance = 'test'
  opt_max_valid_age = -1
  opt_passive = 0
  opt_register_interval = 240 
  opt_sleep_secs = 2 
  opt_die = 0
  opt_noDot = 1 
  opt_generate_ldata_info = 0
  opt_data_type = 'grib'
  opt_source_base_url = ""
  opt_use_source_base_url_only = 0
  opt_use_output_dir_only = 0
  opt_date_str = ""

  #
  # Get the command line arguments.
  #

  optlist, args = getopt.getopt(sys.argv[1:], 'dhp', \
        [ 'debug', 'help', \
          'source_host=', 'source_pwd=', 'source_user=', \
          'source_dir=', 'source_ext=', \
          'output_dir=', 'instance=', \
          'max_valid_age=', 'passive', \
          'register_interval=', 'sleep_secs=', 'die', 'noDot',\
          'generate_ldata_info', 'use_source_url_base_only', 'data_type=', \
          'use_output_dir_only', 'source_url_base=', 'run_date='])

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

    if opt[0] == "--output_dir":
      opt_output_dir = opt[1]

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

    if opt[0] == "--generate_ldata_info":
      opt_generate_ldata_info = 1

    if opt[0] == "--data_type":
       opt_data_type = opt[1]

    if opt[0] == "--source_url_base":
       opt_source_base_url = opt[1]

    if opt[0] == "--use_source_url_base_only":
       opt_use_source_base_url_only = 1 

    if opt[0] == "--use_output_dir_only":
       opt_use_output_dir_only = 1 

    if opt[0] == "--run_date":
      opt_date_str = opt[1]

  if (opt_debug):
    print "opt_debug = ", opt_debug
    print "opt_source_host = ", opt_source_host
    print "opt_source_pwd = ", opt_source_pwd
    print "opt_source_user = ", opt_source_user
    print "opt_source_dir = ", opt_source_dir
    print "opt_source_ext = ", opt_source_ext
    print "opt_output_dir = ", opt_output_dir
    print "opt_instance = ", opt_instance
    print "opt_max_valid_age = ", opt_max_valid_age
    print "opt_passive = ", opt_passive
    print "opt_register_interval = ", opt_register_interval
    print "opt_sleep_secs = ", opt_sleep_secs
    print "opt_die = ", opt_die
    print "opt_noDot = ", opt_noDot
    print "opt_generate_ldata_info = ", opt_generate_ldata_info
    print "opt_data_type = ", opt_data_type
    print "opt_source_base_url = ", opt_source_base_url
    print "opt_use_source_base_url_only = ", opt_use_source_base_url_only
    print "opt_date_str = ", opt_date_str
    print "opt_use_output_dir_only = ", opt_use_output_dir_only

  #
  # Initialize process mapper registration
  #

  pmu.auto_init(prog_name, opt_instance, opt_register_interval)

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
    
  file_time_threshold = 0
  while 1:

    pmu.auto_register("To sleep")

    if need_sleep:
      time.sleep(opt_sleep_secs)

    need_sleep = 1

    pmu.auto_register("downloading data")
    print "+++++++++++++ starting at ", strftime("%a, %d %b %Y %H:%M:%S +0000", gmtime())

    #
    # if url specified, download .nc data files from the 
    # given url using wget.
    # otherwise, download data file (not necesarily .nc type)
    # using ftp
    #
    if opt_source_base_url == "":
      pull_file_ftp(opt_source_ext)
    else:
      file_time_threshold = download_url(file_time_threshold, opt_source_ext)

    #
    # if specified date, must be single run, so exit
    #
    if opt_date_str != "":
      break

  pmu.auto_unregister()

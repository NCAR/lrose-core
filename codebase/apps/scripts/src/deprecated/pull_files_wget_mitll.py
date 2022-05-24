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
import shutil
from datetime import date
from HTMLParser import HTMLParser
from urllib2 import urlopen

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
  print "   --source_url_base <url> : URL for wget to pull remote file that is newer than local copy."
  print "   --run_date              : date for a single run in the format of yyyy/MM/dd"

  return

###############################################################
# HTML parser class
###############################################################
class Spider(HTMLParser):
      def __init__(self, url):
            HTMLParser.__init__(self)
            self.links = []
            req = urlopen(url)
            self.feed(req.read())

      def handle_starttag(self, tag, attrs):
            if tag == 'a' and attrs:
                  if attrs[0][1].find(".nc") > 0:
                      #print "Found link => %s" % attrs[0][1]
                      self.links.append(attrs[0][1])

      def get_links(self) :     # return the list of extracted links
            return self.links


###############################################################
# pull_file(): download the given file (filename) with given URL 
# 
###############################################################

def pull_file(url, filename, output_dir, date_str_flat):
  if opt_debug:
    print "Getting file <" + filename + ">"

  pmu.force_register("Getting file " + filename)

  time_str_flat = parse_time_from_filename(filename) 
  lfilename = time_str_flat +".nc"
  #
  # Construct the local filenames.  The temporary filename is used
  # for pulling the file down.  When the retrieval is complete, the
  # file is renamed to its original name so that it appears atomically.
  #

  local_filename = output_dir + '/' + filename

  if opt_debug:
    print "local_filename = " + local_filename

  org_lastmod_date = 0
  if os.path.exists(local_filename):
    stats = os.stat(local_filename)
    org_lastmod_date = time.localtime(stats[8])

  #
  # Pull the data file to the destination directory.
  #

  pmu.force_register("Retrieving file " + filename)

  try:
    command = "wget -N -P" + output_dir + "/ " + url + "/" + filename
    print command
    os.system(command) 
  except (socket.error):
    print "Error occurred while trying to transfer file: ", filename 
    print "Skipping file!"
    return

  #
  # set trigger file only when data files are updated.
  #

  if os.path.exists(local_filename):
    stats = os.stat(local_filename)
    lastmod_date = time.localtime(stats[8])
    if org_lastmod_date < lastmod_date:
      if opt_generate_ldata_info :
        rpath = date_str_flat + "/" + filename
        date_str = time.strftime("%Y%m%d%H%M%S", time.localtime(stats[8]))
        set_trigger(rpath,"nc",date_str)

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
          + " -rpath " + filename + " -dtype nc " 
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
      os.mkdir(dir_name)
    except (os.error):
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
def download_url ():
  date_str = opt_date_str
  if opt_date_str == "" :
    today = date.today()
    date_str = today.strftime("%Y/%m/%d")
  url = opt_source_base_url + date_str + "/"
  date_str_flat = date_str.replace("/","")
  output_dir = opt_output_dir + "/"+ date_str_flat + "/"   

  create_dir(output_dir)

  fileList = pull_file_list(url, opt_output_dir)

  minute15 = 15*60
  for aFile in fileList:
    file_time_str = parse_time_from_filename(aFile)
    print "aFile = " + aFile
    file_time_int = convert_file_time(file_time_str)
    if file_time_int % minute15 == 0:
      print "pulling file: ", aFile
      pull_file(url, aFile, output_dir, date_str_flat)
    else:
      if opt_debug:
        print "skip non-15minute file<" + aFile + ">"
    #break

  return

###############################################################
# parse_time_from_filename
# filename=edu.mit.ll.wx.ciws.VIL.Netcdf4.1km.20100122T231500Z.nc
# returns 231500
###############################################################
def parse_time_from_filename(filenameStr):
  dateTimeStr = filenameStr.split(".")[len(filenameStr.split("."))-2]
  timeStr = dateTimeStr[dateTimeStr.find("T")+1:dateTimeStr.find("Z")]
  return timeStr

###############################################################
# convert_file_time(time str): convert to integer in seconds
# returns time in seconds 
#   Input string have the following format 232714 --> HHMMSS
###############################################################
def convert_file_time(time_str):
  hour = int(time_str[0:2])
  minute = int(time_str[2:4])
  second = 0
  total = hour*3600 + minute*60 + second
  return total

###############################################################
# pull_file_list(): 
# use index.xml to get list of files to be downloaded 
# from the remote site URL
###############################################################

def pull_file_list(url, output_base_dir):
  if opt_debug:
    print "Getting URL <" + url + ">"
    print "Putting dir <" + output_base_dir + ">"

  #
  # Pull the data file to the destination directory.
  #
  indexFile = output_base_dir + "/index.xml"
  pmu.force_register("Retrieving index from URL " + url)

  try:
    command = "wget -O " + indexFile + " " + url
    print command
    os.system(command)
  except (socket.error):
    print "Error occurred while trying to transfer URL: ", url
    print "Skipping file!"
    return

  pmu.force_register("Retrieved URL " + url)

  indexFileUrl = "file://" + indexFile
  sp = Spider(indexFileUrl)
  fileList = sp.get_links()
  os.unlink(indexFile)
  sp.close()

  pmu.force_register("Retrieved remote file list " )
  return fileList

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
  opt_source_base_url = ""
  opt_date_str = ""
  opt_do_rename_output = 0

  #
  # Get the command line arguments.
  #

  optlist, args = getopt.getopt(sys.argv[1:], 'dhp', \
        [ 'debug', 'help', \
          'source_host=', 'source_pwd=', 'source_user=', \
          'source_dir=', 'source_ext=', \
          'output_dir=', 'instance=', \
          'max_valid_age=', 'passive', \
          'register_interval=', 'sleep_secs=', 'die', 'noDot', \
          'generate_ldata_info', \
          'source_url_base=', 'run_date=', 'rename_output'])

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

    if opt[0] == "--source_url_base":
       opt_source_base_url = opt[1]

    if opt[0] == "--run_date":
      opt_date_str = opt[1]

    if opt[0] == "--rename_output":
      opt_do_rename_output = 1

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
    print "opt_source_base_url = ", opt_source_base_url
    print "opt_date_str = ", opt_date_str
    print "opt_do_rename_output = ", opt_do_rename_output

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
    
  while 1:

    pmu.auto_register("To sleep")

    if need_sleep:
      time.sleep(opt_sleep_secs)

    need_sleep = 1

    pmu.auto_register("downloading data")

    #
    # if url specified, download .nc data files from the 
    # given url using wget.
    # otherwise, download data file (not necesarily .nc type)
    # using ftp
    #
    download_url()

    #
    # if specified date, must be single run, so exit
    #
    if opt_date_str != "":
      break

  pmu.auto_unregister()

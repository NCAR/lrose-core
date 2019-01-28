#!/usr/bin/env python2

#
# larc_gdcp_retrive.py -- this script uses ftp to retrive Goes Derived Cloud Product
#                         file LARC NASA
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
from urllib import urlretrieve
from urllib2 import Request
from urllib2 import urlopen
import re

# local modules from libs/python
import pmu

#
# Initialize the command line arguments.
#
opt_debug = False
opt_web_server = 'www-angler.larc.nasa.gov'
opt_url_path = 'prod/conus/visst-ruc-icing-cdf/goes12'
opt_output_dir = '/scratch/cunning'
opt_unzip = False
opt_instance = 'test'
opt_sleep_secs = 300
opt_run_once = False 


separator = '/'
#file_match_re = '^<IMG.*<(A|a) (HREF|href)="(.*gz)">'
file_match_re = '^<li><a href="(.*gz)">'


################################################################################
# Local subroutines                                                            #
################################################################################

################################################################################
# handle_options(): handles the command line options
#

def handle_options():

  global opt_debug
  global opt_output_dir
  global opt_web_server
  global opt_url_path
  global opt_unzip
  global opt_instance
  global opt_sleep_secs
  global opt_run_once
  global opt_log

  #
  # Get the command line arguments
  #
  try:
    optlist, args = getopt.getopt(argv[1:], 'dh', \
                                  [ 'debug', 'help', \
                                    'web_server=', 'url_path=', 'output_dir=', 'instance=', \
                                    'unzip', 'run_once', 'sleep=', ])
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

    if opt[0] == '--web_server':
      opt_web_server = opt[1]
    if opt[0] == '--url_path':
      opt_url_path = opt[1]
    if opt[0] == '--output_dir':
      opt_output_dir = opt[1]
    if opt[0] == '--unzip':
      opt_unzip = True
    if opt[0] == '--run_once':
      opt_run_once = True
    if opt[0] == '--instance':
      opt_instance = opt[1]
    if opt[0] == '--sleep':
      opt_sleep_secs = opt[1]

  #
  # check the argument list
  #
  debug_print('Command line options:')
  debug_print('\tdebug = on')
  debug_print('\tweb_server = '+ opt_web_server)
  debug_print('\turl_path = '+ opt_url_path)
  debug_print('\toutput_dir = '+ opt_output_dir)
  debug_print('\tunzip = '+ str(opt_unzip))
  debug_print('\trun_once = '+ str(opt_run_once))
  debug_print('\tinstance = '+ opt_instance)
  debug_print('\tsleep_secs = '+ str(opt_sleep_secs))

################################################################################
# print_usage(): Print the usage for this script
#

def usage():

  print 'Usage: ', prog_name, ' [options]'
  print 'Options:'
  print '   -h | --help             : Print usage and exit'
  print '   --url_path_dir <dir>    : Path to file listing'
  print '                             (default = ', opt_url_path, ')'
  print '   --web_server <host>     : Web server host name'
  print '                             (default = ', opt_web_server, ')'
  print '   --output_dir <dir>      : data directory for output'
  print '                             (default = ', opt_output_dir, ')'
  print '   --instance <instance>   : PMU instance name (default = ', \
    opt_instance, ')'
  print '   --sleep <secs>          : Number of seconds to sleep between'
  print '                             tries. '
  print '                             (default = ', opt_sleep_secs, ')'
  print '   --unzip                 : Uncompress FTP files using gunzip'
  if opt_unzip:
    print '                             (default = uncompress files', ')'
  else:
    print '                             (default = leave files compressed', ')'
  print '   -d | --debug            : Print debug messages'
  print '   --run_once              : script runs once then exits'


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
# read_url(url_text):

def fetch_page(url_text):

  debug_print('URL: ' + url_text)

  the_page = []
  
  req = Request(url_text)

  try:
    response = urlopen(req)
  except IOError, e:
    if hasattr(e, 'reason'):
      print 'Failed to reach server.'
      print 'Reason: ', e.reason
    elif hasattr(e, 'code'):
      print 'The sever couldn\'t fulfill request.'
      print 'Error code: ', e.code
  else:
    the_page = response.read()

  return the_page


################################################################################
# parse_index_page(the_page): 
#
# The function takes html page and parses out a list of files contained in page.
# The file names are imbedded in A tags thate look like:
# <A HREF="goes-e.icing.rucgrid.2006298.0115.cdf.gz">goes-e.icing.rucgrid..&gt;</A>
#
def parse_index_page(the_page):

  regex = re.compile(file_match_re)
  
  file_list = []
  for line in split(the_page, '\n'):

    result = regex.match(line)

    if result:
      file_list.append(result.group(1))

  return file_list


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
  # Initialize process mapper registration
  #
  if not opt_run_once:
    pmu.auto_init(prog_name, opt_instance, 60)

  #
  # set last_time to two hours before now to prevent unecessary searching
  # 
  last_file_time = time() - 7200
  debug_print('last_file_time = ' + asctime(gmtime(last_file_time)))
  
  while 1:

    url_info = 'http://' + opt_web_server + separator + opt_url_path
    
    if not opt_run_once:
      pmu.auto_register('getting file listing from ' + url_info)

    index_page = fetch_page(url_info)

    print 'made it here.'

    print str(index_page)
    
    file_list = parse_index_page(index_page)

    print str(file_list)
    
    #
    # reverse list -- now in descending order
    #
    filename = ''
    file_list.reverse()
    for entry in file_list:
      filename = entry
      print filename
      file_time = get_file_time(filename)
      if file_time > last_file_time:
        last_file_time = file_time
        debug_print('last_file_time = ' + asctime(gmtime(last_file_time)))
        break
      filename = ''

    #
    # retrieve the file
    #
    if len(filename) > 0:
      debug_print('fetching: ' + filename)

      if not opt_run_once:
        pmu.auto_register('fetching: ' + filename)
        
      
      file_contents = fetch_page(url_info + separator + filename)
      
      fetch_path = path.join(opt_output_dir, filename)

      # Could replace the file_contents line above and the next to try/except segments with this line
      #response = urlretrieve(url_info + separator + filename, fetch_path)

      try:
        output_file = open(fetch_path, 'w')
      except IOError, e:
        print 'Failed to open output file ' + output_path
        if hasattr(e, 'reason'):
          print 'Reason: ', e.reason
        elif hasattr(e, 'code'):
          print 'Error code: ', e.code
        if opt_run_once:
          break
        else:
          continue
        
    
      try:
        output_file.write(file_contents)
        output_file.close()
      except IOError, e:
        print 'Failed to write to output file ' + fetch_path
        if hasattr(e, 'reason'):
          print 'Reason: ', e.reason
        elif hasattr(e, 'code'):
          print 'Error code: ', e.code
        if opt_run_once:
          break
        else:
          continue
      
      
      if not path.isfile(fetch_path):
        print 'file was not retrieved successfully.'
        if opt_run_once:
          break
        else:
          continue
        
      if opt_unzip:
        debug_print('unzipping ' + fetch_path)
        if not opt_run_once:
          pmu.auto_register('unzipping ' + fetch_path)

        cmd = '/bin/gunzip ' + fetch_path
        print 'cmd = ' + cmd
        system(cmd)

        #
        # fetch_path has changed -- not .gz extension
        #
        (fetch_path, ext) = path.splitext(fetch_path)


      #
      # now move the retrieved file to it final location 
      #
      (junk, ext) = path.splitext(fetch_path)
      final_path = get_output_filename(file_time, ext)
      rename(fetch_path, final_path)
      gm_tuple = gmtime(file_time)
      yyyymmdd = strftime('%Y%m%d', gm_tuple)
      hhmmss = strftime('%H%M%S', gm_tuple)
      cmd = 'LdataWriter -dir ' + opt_output_dir + ' -ext cdf -ltime ' + yyyymmdd + hhmmss + ' -info2 ' + yyyymmdd + '/' + hhmmss + '.cdf'
      print cmd
      system(cmd)
      
    #
    # maybe we are done ...
    #
    if opt_run_once:
      break

    debug_print('going to sleep for ' + str(opt_sleep_secs) + ' seconds.')
    stdout.flush()

    #
    # break up sleep interval in chunks to allow
    # contact with procamp
    #
    for i in xrange(0, opt_sleep_secs/15):
      pmu.auto_register('waiting for data, sleeping.')
      sleep(15)
      stdout.flush()


  exit(0)

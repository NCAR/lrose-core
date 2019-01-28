#!/usr/bin/env python2

#
# grib_scraps - This script is a companion to grib_notify that copies over
#               model files that complete after the status file appears
#
#


# import modules
import getopt

from os import rename
from os import path
from os import system
from os import walk

from string import join
from string import find
from string import replace

import re

from sys import argv
from sys import exit

#
# Initialize the command line arguments.
#
opt_min_percentage = 100.0
opt_tmp_subdir = 'tmp'
opt_top_dir = '/d1/ldm/data/grib'
opt_debug = False
opt_notify = False

###############################################################
# handle_options(): handles the command line options
#
def handle_options():

  global opt_min_percentage
  global opt_tmp_subdir
  global opt_top_dir
  global opt_debug
  global opt_notify

  #
  # Get the command line arguments
  #
  try:
    optlist, args = getopt.getopt(argv[1:], 'h', \
                                  ['help', 'debug', 'min_percentage=', 'tmp_subdir=', 'top_dir=', 'notify'])
  except getopt.GetoptError, e:
    # print help information and exit
    print e
    usage()
    exit(1)
  
  for opt in optlist:
    if opt[0] == "-h" or opt[0] == "--help":
      usage()
      exit(1)
      
    if opt[0] == "--debug":
      opt_debug = True
        
    if opt[0] == "--notify":
      opt_notify = True
        
    if opt[0] == "--min_percentage":
      opt_min_percentage = float(opt[1])
        
    if opt[0] == "--top_dir":
      opt_top_dir = opt[1]

    if opt[0] == "--tmp_subdir":
      opt_tmp_subdir = opt[1]
        
  #
  # check the argument list
  #
  if opt_debug:
    print 'Command line options:'
    print '\tdebug = on'
    print '\tmin_percentage = ' + str(opt_min_percentage)
    print '\ttop_dir = ' + opt_top_dir
    print '\ttmp_subdir = ' + opt_tmp_subdir
    print '\tnotify = ' + str(opt_notify)




##################################################################
# usage(): prints the usage statement for this script
#
#
def usage():

  print 'Usage: ' +  prog_name + ' [options]'
  print 'Options:'
  print '   -h | --help                 : Print usage and exit'
  print '   --debug                     : Turns on debugging.'
  print '   --min_percentage <n>        : Minimum percentage complete.'
  print '                                 (default = ', opt_min_percentage, ')' 
  print '   --top_dir <str>             : Top directory to search under.'
  print '                                 (default = ', opt_top_dir, ')' 
  print '   --tmp_subdir <str>          : Temporary file subdirectory.'
  print '                                 (default = ', opt_tmp_subdir, ')' 
  print '   --notify                    : Send notifiction message through LDM.'
  print '                                 (default = ', opt_notify, ')' 
  

##################################################################
# ldm_notify(status_file): inserts file compeltion notification
#    message into LDM queue
#
def ldm_notify(status_file):

  print 'ldm_notify not implemented yet.'

#  cmd = 'pqinsert -p \"GRIB AVAL ${ddhh}00 GRIB $productId $fhr\" -f OTHER' + temp_file;


##################################################################
# find_status_size(status_file): extracts the expected file size
#    from status file
#

def find_status_size(status_file):

  if path.isfile(status_file):
    try:
      hdl = open(status_file, 'r')
    except IOError, e:
      print 'unable to open ' +  status_file + ': ', e
      print 'exiting ...'
      exit(1)

    the_line = hdl.readline()
    hdl.close()

    tmatch = re.search(r'\((.*) bytes\)', the_line)

    if tmatch:
      size =tmatch.group(1)
      return size


  return 0

##################################################################
# Main program                                                   #
##################################################################

if __name__ == "__main__":

  #
  # Retrieve the program name from the command line.
  #
  prog_name = path.basename(argv[0])
  
  #
  # handle the command line options
  #
  handle_options()

  tmp_dir = path.join(opt_top_dir, opt_tmp_subdir)

  for root, dirs, files in walk(tmp_dir):

    if opt_debug:
      print 'in dir: ' + root

    for name in files:
      tmp_grib_path = path.join(root, name)

      if find(tmp_grib_path, 'grb') != -1 and find(tmp_grib_path, '.gz') == -1:
        status_file = replace(tmp_grib_path, 'grb', 'status')

        status_size = find_status_size(status_file)

        if status_size <= 0:
          continue
        
        grib_size = path.getsize(tmp_grib_path)

        percent_complete = (float(grib_size)/float(status_size))*100.0
        if opt_debug:
          print 'grib file: ' + path.basename(tmp_grib_path) + '   file size: ' + str(grib_size) + '   status_size = ' + str(status_size)
          print 'percent complete: ' + str(percent_complete)

        if percent_complete >= opt_min_percentage:

          index = find(tmp_grib_path, opt_tmp_subdir) + len(opt_tmp_subdir)
          final_grib_path = opt_top_dir + tmp_grib_path[index:]

          if opt_debug: 
            print 'moving file to ' + final_grib_path

          try:
            rename(tmp_grib_path, final_grib_path)
          except OSError, e:
            print 'unable to move  ' +  tmp_grib_path + ': ', e
            print 'exiting ...'
            exit(1)
            
          if opt_notify:
            ldm_notify(status_file)

        else:
          if opt_debug:
            print 'NOT moving file.'
              
        print '\n'

    



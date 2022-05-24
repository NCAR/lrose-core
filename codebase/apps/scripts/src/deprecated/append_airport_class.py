#!/usr/bin/env python2

#
# This script reads the airport location files, airports.tbl, and assigns a
# priority levels to airports found in the airport classification file.
#
# From NPAIS website, airports are classified as primary (PR), commercial(CM),
# reliever (RL) or general aviation (GA). For the JADE station class, priority
# ranges from 0 to 9, with 0 being highest priority
#
# Priority matching method:
#       PR: [0,2]
#	CM: [3,4]
#	RL: 5
#	GA: [6,9]    
#
# Since no other criteria is available to refine the priorities within a
# class, use the follwing values:
#       PR: 1
#	CM: 3
#	RL: 5
#	GA: 7    
#

# import modules
import getopt
import string
from string import split
from string import join
from string import ljust
from sys import argv
from sys import exit
from os import path
import os

opt_debug = False
opt_airport_file = 'airports.tbl'
opt_priority_file = 'airport_class.tbl'
opt_output_file = 'new_airports.tbl'

################################################################################
# Local subroutines                                                            #
################################################################################

################################################################################
# handle_options(): handles the command line options
#

def handle_options():

  global opt_debug
  global opt_airport_file
  global opt_priority_file
  global opt_output_file

  #
  # Get the command line arguments
  #
  try:
    optlist, args = getopt.getopt(argv[1:], 'dh', \
                                  [ 'debug', 'help', \
                                    'airport_file=', 'priority_file=', 'out_file='])
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

    if opt[0] == '--airport_file':
      opt_airport_file = opt[1]

    if opt[0] == '--priority_file':
      opt_priority_file = opt[1]

    if opt[0] == '--out_file':
      opt_output_file = opt[1]

  #
  # check the argument list
  #
  debug_print('Command line options:')
  debug_print('\tdebug = on')
  debug_print('\tairport_file = '+ opt_airport_file)
  debug_print('\tpriority_file = '+ opt_priority_file)
  debug_print('\tout_file = '+ opt_output_file)


################################################################################
# print_usage(): Print the usage for this script
#

def usage():

  print 'Usage: ', prog_name, ' [options]'
  print 'Options:'
  print '   -h | --help             : Print usage and exit'
  print '   --airport_file <file>   : Airport location file name'
  print '                            (default = ', opt_airport_file, ')'
  print '   --priority_file <file>  : Airport priority file name'
  print '                            (default = ', opt_airport_file, ')'
  print '   --out_file <file>       : Output file name'
  print '                            (default = ', opt_output_file, ')'


################################################################################
# debug_print(msg): prints debugging message
#

def debug_print(msg):

  if not opt_debug:
    return

  msg_hdr = '\tDEBUG -- '
  print msg_hdr + msg


################################################################################
# Main program                                                                 #
################################################################################

if __name__ == '__main__':

  prog_name = path.basename(argv[0])   

  #
  # handle the command line options
  #
  handle_options()

  #
  # open the airport location, airport priority and output files
  try:
    airport_file  = open(opt_airport_file, 'r')
  except (IOError):
    print 'Cannot open input file ' + opt_airport_file + ' ... exiting ...'
    exit(1)

  try:
    priority_file  = open(opt_priority_file, 'r')
  except (IOError):
    print 'Cannot open input file ' + opt_airport_file + ' ... exiting ...'
    exit(1)

  try:
    out_file  = open(opt_output_file, 'w')
  except (IOError):
    print 'Cannot open input file ' + opt_output_file + ' ... exiting ...'
    exit(1)


  #
  # put contents of priority file in a dictionary
  #
  priority_dict = {}
  while 1:
    ln = priority_file.readline()
    if (ln):
      pr_entry = split(ln)
      key = 'K' + pr_entry[0]
      priority_dict[key] = pr_entry[1]
    else:
      break

  print 'priority_dict length is:' + str(len(priority_dict))
    
  #
  # read the airport file
  #
  j = 0
  i = -1
  missed_airports = []
  airports = []
  while 1:
    i = i + 1
    line = airport_file.readline()
    if (line):
      airports.append(split(line))

      #
      # US airport IDs start with K, don't try with airport outside of US
      #
      if airports[i][0][0] != 'K':
#        out_file.write(line)
        continue

      if priority_dict.has_key(airports[i][0]):

        if priority_dict[airports[i][0]] == 'PR':
          new_line = line[:-2] + '1\n'
        elif priority_dict[airports[i][0]] == 'CM':
          new_line = line[:-2] + '3\n'
        elif priority_dict[airports[i][0]] == 'RL':
          new_line = line[:-2] + '5\n'
        elif priority_dict[airports[i][0]] == 'GA':
          new_line = line[:-2] + '7\n'
          
        del priority_dict[airports[i][0]]
        j = j + 1
      else:
        new_line = line[:-2]  + '3\n'
        missed_airports.append(airports[i][0])

      #
      # write line to 'new' airport file
      #
      out_file.write(new_line)
    else:
      break

  print 'airports length is:' + str(len(airports))
  print 'matched ' + str(j) + ' airports'
  print 'missed_airports length is:' + str(len(missed_airports))

  for ap in missed_airports:
    print ap

  #
  # close the files
  #
  airport_file.close()
  priority_file.close()
  out_file.close()

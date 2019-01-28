#!/usr/bin/env python2

#
# This is a script that converts 'map' files for CIDD to
# 'map' files for JADE.
#
# The JADE map files can be read by the following classes
#       jade.model.segment.DiscontLonLatPointsDataLayer
#

# import modules
import getopt
import string
from string import find
from string import split
from sys import argv
from sys import exit
from os import path
import os

opt_debug = False
opt_input_file = 'in.map'
opt_output_file = 'out.map'

################################################################################
# Local subroutines                                                            #
################################################################################

################################################################################
# handle_options(): handles the command line options
#

def handle_options():

  global opt_debug
  global opt_input_file
  global opt_output_file

  #
  # Get the command line arguments
  #
  try:
    optlist, args = getopt.getopt(argv[1:], 'dh', \
                                  [ 'debug', 'help', \
                                    'infile=', 'outfile='])
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

    if opt[0] == '--infile':
      opt_input_file = opt[1]

    if opt[0] == '--outfile':
      opt_output_file = opt[1]

  #
  # check the argument list
  #
  debug_print('Command line options:')
  debug_print('\tdebug = on')
  debug_print('\tin_file = '+ opt_input_file)
  debug_print('\tout_file = '+ opt_output_file)


################################################################################
# print_usage(): Print the usage for this script
#

def usage():

  print 'Usage: ', prog_name, ' [options]'
  print 'Options:'
  print '   -h | --help             : Print usage and exit'
  print '   --infile <file>        : Input file name'
  print '                            (default = ', opt_input_file, ')'
  print '   --outfile <file>       : Output file name'
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
# process_line(line): processes the line
#

def process_line(line):

  #
  # find segment terminator string
  #
  if find(line, '-1000 -1000') == 0:
    return 'LIFT\n'

  #
  # try to find lines that contain lat/lon pairs
  # once found swap them
  #
  pos_list = split(line)

  # not a position pair
  if len(pos_list) != 2:
    return ''

  # not a position pair
  if pos_list[0][0].isalpha():
    return ''
  
  # must be a pair
  new_line = pos_list[1] + ' ' + pos_list[0] + '\n'
  return new_line

  return ''

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
  # open the input and output files
  try:
    infile  = open(opt_input_file, 'r')
  except (IOError):
    print 'Cannot open input file ' + opt_input_file + ' ... exiting ...'
    exit(1)

  try:
    outfile  = open(opt_output_file, 'w')
  except (IOError):
    print 'Cannot open input file ' + opt_output_file + ' ... exiting ...'
    exit(1)

  #
  # first line in segment file is 'LIFT'
  #
  outfile.write('LIFT\n')
  
  while 1:
    ln = infile.readline()
    if (ln):
      if (ln[0] != "#"):
        new_line = process_line(ln)
        outfile.write(new_line)
    else:
      break

  infile.close()
  outfile.close()

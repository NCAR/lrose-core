#!/usr/bin/env python
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2011 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2011/5/5 20:33:14 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

# import modules

from os import environ
from sys import exit
from sys import stdout
from string import join
from time import time
from time import sleep
from time import asctime
from time import gmtime

debug_mode = False
start_time = 0

border = '========================================================================'
edge = '===='
border_len = len(border)
edge_len = len(edge)



###############################################################
# begin_declaration(title, run_time):
#
def begin_declaration(title, run_time):

  global start_time
  
  start_time = time()

  title_str = 'Begin ' + title
  start_time_str = 'start time is: ' +  asctime(gmtime(run_time))

  title_len = len(title_str)
  start_len = len(start_time_str)

  #
  # if start or title lengths are odd add a space so line lengths will work out
  #  
  if title_len%2:
    title_str = title_str + ' '

  if start_len%2:
    start_time_str = start_time_str + ' '

  print border

  gap = ' '
  gap_len = (border_len-title_len)/2 - edge_len - 1
  for i in range(gap_len):
   gap = gap + ' '
  print edge + gap + title_str + gap + edge

  gap = ' '
  gap_len = (border_len-start_len)/2 - edge_len - 1
  for i in range(gap_len):
    gap = gap + ' '
  print edge + gap + start_time_str + gap + edge

  print border + '\n'



###############################################################
# end_declaration(title):
#
def end_declaration(title):

  end_time = time()
  title_str = 'End ' + title
  end_time_str = 'end time is: ' + asctime(gmtime(end_time))
  elapsed_time_str = 'elapsed time is ' + str(int(end_time-start_time)) + ' seconds'

  title_len = len(title_str)
  end_len = len(end_time_str)
  elapsed_len = len(elapsed_time_str)
  
  #
  # if start or title lengths are odd add a space so line lengths will work out
  #  
  if title_len%2:
    title_str = title_str + ' '

  if end_len%2:
    end_time_str = end_time_str + ' '

  if elapsed_len%2:
    elapsed_time_str = elapsed_time_str + ' '

  print '\n' + border
  
  gap = ' '
  gap_len = (border_len-title_len)/2 - edge_len - 1
  for i in range(gap_len):
    gap = gap + ' '
  print edge + gap + title_str + gap + edge

  gap = ' '
  gap_len = (border_len-end_len)/2 - edge_len - 1
  for i in range(gap_len):
    gap = gap + ' '
  print edge + gap + end_time_str + gap + edge

  gap = ' '
  gap_len = (border_len-elapsed_len)/2 - edge_len - 1
  for i in range(gap_len):
    gap = gap + ' '
  print edge + gap + elapsed_time_str + gap + edge

  print border + '\n\n'


###############################################################
# debug(): prints current time and flushes stdout
#

def forced_time_print(msg = ''):
  start_time = time()

  print msg + '\ttime is: ' +  asctime(gmtime(time())) + '\n'

  stdout.flush()
  
###############################################################
# debug(msg): prints debugging message
#

def debug(msg):

  if not debug_mode:
    return

  msg_hdr = '\tDEBUG -- '
  print msg_hdr + msg
  stdout.flush()


###############################################################
# warning(msg): prints warning message
#

def warning(msg):

  msg_hdr = '\tWARNING -- '
  print msg_hdr + msg
  stdout.flush()


###############################################################
# error(msg): prints error message
#

def error(msg):

  msg_hdr = '\tERROR -- '
  print msg_hdr + msg
  stdout.flush()


##################################################################
# MAIN for testing                                               #
##################################################################


if __name__ == "__main__":

  debug_mode = True
  run_time = time()
  print 'test begin_declaration(title, run_time):'
  begin_declaration('TEST', run_time)
  print 'test end_declaration(title, run_time):'
  sleep(1)
  end_declaration('TEST', run_time)
  msg = 'TEST'
  print '\ntest debug(msg):'
  debug(msg)
  print '\ntest warning(msg):'
  warning(msg)
  print '\ntest error(msg):'
  error(msg)
  

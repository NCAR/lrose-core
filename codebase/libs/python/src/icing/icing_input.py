#!/usr/bin/env python
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:29 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

import os
from os import environ
from os import stat
from os import path
from os import listdir
from stat import *

from string import find
from string import replace

from sys import exit

from time import gmtime
from time import sleep
from time import asctime
from time import mktime
from time import timezone
from time import time
from time import strftime

from ldata import Ldata

import re

import icing_message

#
# used by check_for_file
#
max_valid_age = 30
ldata_debug = 0


##################################################################
# get_archive_time(run_time): creates a unix time from from
#                             time string in the form YYYYMMDDHH
#
#

def get_archive_time(run_time):
  'creates a unix time from from time string in the form YYYYMMDDHH'
  
  #
  # separate out the time components
  #
  year = int(run_time[:4])
  month = int(run_time[4:6])
  day = int(run_time[6:8])
  hour = int(run_time[8:10])
  minutes = int(run_time[10:12])
  
  gmt_tpl = (year, month, day, hour, minutes, 0, 0, 0, 0)
  archive_time = mktime(gmt_tpl) - timezone
  
  icing_message.debug('archive time is: ' + asctime(gmtime(archive_time)))

  return archive_time


##################################################################
# fill_template(template, run_time, **fcast_args):
#
# optional keyword arguments are:
#   gen_hour -- generation hour
#   forecast -- forecast interval
#   tile_num -- tile number (for NSSL 2D radar)
#
# the optional keywords support model file name templates
#

def fill_template(template, run_time, **fcast_args):

  tm_tuple = gmtime(run_time)

  if len(fcast_args) > 0:
    try:
      tile_int = '%d' % int(fcast_args['tile_num'])
      template = template.replace('%T', tile_int)
    except KeyError:
      fcast_str = '%02d' % int(fcast_args['forecast'])
      template = template.replace('%F', fcast_str)
      gent_str = '%02d' % int(fcast_args['gen_hour'])
      template = template.replace('%G', gent_str)
  
  template = strftime(template, tm_tuple)

  return template

##################################################################
# (the_dir, the_file, run_time): 
#		
# 

def build_app_time_str(run_time, offset=0): 

  gm_tuple = gmtime(int(run_time/3600)*3600 - offset)
  time_str =  '%04d %02d %02d %02d %02d 00' % \
              (gm_tuple[0], gm_tuple[1], gm_tuple[2], gm_tuple[3], gm_tuple[4])

  return time_str

##################################################################
# build_app_time_str_no_truncation(run_time, offset_secs)
#		
# 

def build_app_time_str_no_truncation(run_time, offset=0): 

  gm_tuple = gmtime(int(run_time) - offset)
  time_str =  '%04d %02d %02d %02d %02d 00' % \
              (gm_tuple[0], gm_tuple[1], gm_tuple[2], gm_tuple[3], gm_tuple[4])

  return time_str


##################################################################
# build_input_path(the_dir, the_file, run_time): 
#		
# 

def build_input_path(the_dir, the_file, run_time): 

  try:
    dir_template = environ[the_dir]
  except KeyError:
    print 'key error'
    icing_message.error('Unknown variable: ' + the_dir)
    exit()
    
  try:
    file_template = environ[the_file]
  except KeyError:
    print 'key error'
    icing_message.error('Unknown variable: ' + the_file)
    exit()

  
  the_path = path.join(fill_template(dir_template, run_time), \
                       fill_template(file_template, run_time))

  icing_message.debug('the_path = ' + the_path)

  return the_path


##################################################################
# add_date_subdir(the_dir,  run_time): 
#		
# 

def add_date_subdir(the_dir, run_time):

  tm_tuple = gmtime(run_time)
  date_subdir = '%04d%02d%02d' % (tm_tuple[0], tm_tuple[1], tm_tuple[2])
  return path.join(the_dir, date_subdir)

  
##################################################################
# find_file_env_var(data_dir, template, idx = 0, print_list = 0): 
#                         locates a raw file given a directory
#                         and a template to test against. idx
#                         can be 0 or -1, where 0 is the first
#                         entry in the list of file matches. -1
#                         is the last entry in the list of matches
#   
#		

def find_file_env_var(data_dir, template, run_time, idx=0, print_list=0, print_debug=1): 


  try:
    dir_template = environ[data_dir]
  except KeyError:
    icing_message.error('Unknown variable: ' + data_dir)
    exit()
    
  try:
    file_template = environ[template]
  except KeyError:
    icing_message.error('Unknown variable: ' + template)
    exit()

  the_dir = fill_template(dir_template, run_time)
  the_file = fill_template(file_template, run_time)

  file_path = find_file(the_dir, the_file, idx, print_list, print_debug)

  return file_path

##################################################################
# find_file_env_var(data_dir, template, idx = 0, print_list = 0): 
#                         locates a raw file given a directory
#                         and a template to test against. idx
#                         can be 0 or -1, where 0 is the first
#                         entry in the list of file matches. -1
#                         is the last entry in the list of matches
#   
#		

def find_file_template(data_dir, template, run_time, idx=0, print_list=0, print_debug=1): 

  the_dir = fill_template(data_dir, run_time)
  the_file = fill_template(template, run_time)

  file_path = find_file(the_dir, the_file, idx, print_list, print_debug)

  return file_path

##################################################################
# find_file(data_dir, template, idx = 0, print_list = 0): 
#                         locates a raw file given a directory
#                         and a template to test against. idx
#                         can be 0 or -1, where 0 is the first
#                         entry in the list of file matches. -1
#                         is the last entry in the list of matches
#   
#		

def find_file(data_dir, file_substr, idx=0, print_list=0, print_debug=1): 

  if(print_debug) :
    icing_message.debug('looking for file in ' + data_dir + \
                        ' matching: ' + file_substr)

  if not path.isdir(data_dir):
    msg = '\tinput data directory %s does not exist.' % data_dir
    icing_message.error(msg)
    return ''

  #
  # get a list of files in data directory
  #
  filelist = listdir(data_dir) 
  filelist.sort()


  test_files = []
  for filename in filelist:
    regex = re.compile(file_substr)
    if re.match(file_substr,filename):
#    if not filename.find(file_substr):
      test_files.append(filename)

  if len(test_files) and print_list:
      icing_message.debug(str(test_files))
  #
  # the newest file will be last in list
  #
  if len(test_files):
    file_path = path.join(data_dir, test_files[idx])
    if get_file(file_path):
      return file_path

  return ''


##################################################################
# check_for_file(the_dir): 
#   
#		
#

def check_for_file(the_dir):

  ldata = Ldata('icing_input', ldata_debug)
  
  ret = ldata.info_read(the_dir, max_valid_age)
  if ret != 0:
    msg = '\tCould not find file in ' + the_dir
    icing_message.error(msg)
    return ''

  file_path = ldata.data_path(the_dir)

  icing_message.debug('file_path = ' + file_path)

  return file_path


##################################################################
# get_ruc_model_file(test_dir, test_file, run_time, fcast=-1, cycle=1): 
#        
# 

def get_ruc_model_file(test_dir, test_file, run_time, fcast=-1, cycle=1, valid=False): 


  try:
    the_dir = environ[test_dir]
  except KeyError:
    print 'Unknown variable: ', test_dir
    return ''
  try:
    the_file = environ[test_file]
  except KeyError, e:
    print 'Unknown variable: ', test_file
    return ''

  if valid:
    gm_tuple = gmtime(run_time - fcast*3600)
    run_shift = gm_tuple[3]%cycle
    gen_time = run_time - (fcast+run_shift)*3600
  else:
    gen_time = run_time

  gm_tuple = gmtime(gen_time)
    
  model_dir = fill_template(the_dir, gen_time)
  filename = fill_template(the_file, gen_time, gen_hour=gm_tuple[3], forecast=fcast)

  if get_file(path.join(model_dir, filename), 5):
    return path.join(model_dir, filename)
 
  return ''

##################################################################
# get_wrf_rr_model_file(test_dir, test_file, run_time, fcast=-1, cycle=1): 
#        
# test_dir - env var pointing to directory where model files can be found
# test_file - env var describing filename
# run_time - the run time of cip/fip_control
# fcast - the forecast hour to look for.


def get_wrf_rr_model_file(test_dir, test_file, run_time, fcast=-1): 


  try:
    the_dir = environ[test_dir]
  except KeyError:
    print 'Unknown variable: ', test_dir
    return ''
  try:
    the_file = environ[test_file]
  except KeyError, e:
    print 'Unknown variable: ', test_file
    return ''

  gen_time = run_time-fcast*3600
  gm_tuple = gmtime(gen_time)

  model_dir = fill_template(the_dir, gen_time)
  filename = fill_template(the_file, gen_time, gen_hour=gm_tuple[3], forecast=fcast)

  print "looking for",path.join(model_dir, filename)

  if get_file(path.join(model_dir, filename), 5):
    return path.join(model_dir, filename)
 
  return ''



##################################################################
# check_for_nssl2d_radar_file(test_dir, test_file, file_time, tile_nbr)
#
#

def check_for_nssl2d_radar_file(test_dir, test_file, file_time, tile_nbr):

  try:
    the_dir = environ[test_dir]
  except KeyError:
    print 'Unknown variable: ', test_dir
    return ''
  try:
    the_file = environ[test_file]
  except KeyError, e:
    print 'Unknown variable: ', test_file
    return ''

  nssl_radar_file = '';
  nssl2d_radar_dir       = fill_template(the_dir,  file_time, tile_num=tile_nbr)
  nssl2d_radar_file_name = fill_template(the_file, file_time)
  nssl2d_radar_file_test = path.join(nssl2d_radar_dir, nssl2d_radar_file_name)

  if os.path.isfile(nssl2d_radar_file_test):
    return nssl2d_radar_file_test
  else:
    nssl2d_radar_file_name = fill_template(the_file, file_time) + ".gz"
    nssl2d_radar_file_test = path.join(nssl2d_radar_dir, nssl2d_radar_file_name)
    if os.path.isfile(nssl2d_radar_file_test):
      return nssl2d_radar_file_test
    
  return ''


##################################################################
# get_file(file, timeout): 
#   search for file. timeout seconds are allowed for completion
#   of a copy, so that the file is complete
#		
#

def get_file(file, timeout=10):
    
  sleep_interval = 2
  wait_interval = timeout
  while not path.exists(file):
    sleep(sleep_interval)
    wait_interval = wait_interval - sleep_interval
    if wait_interval <= 0:
      return 0

  last_size = 0
  while wait_interval > 0:
    try:
      size = stat(file)[ST_SIZE]
    except OSError, e:
      msg =  'stat failed --', e 
      icing_message.error(msg)
      continue

    if size == last_size:
      return 1
    
    last_size = size;
    sleep(sleep_interval)
    wait_interval = wait_interval - sleep_interval
    
  return 0


#################################################################
# check_environ_dir(env_var, run_time):
#
#

def check_environ_dir(env_var, run_time=-1,tileNum=-1, yesterdayOK=False):

  try:
    env_dir = environ[env_var]
  except KeyError, e:
    print e
    icing_message.error('environment variable $' + env_var + ' does not exist ... exiting ...')
    return False
  
  if run_time > 0:
    if tileNum > 0:
      test_dir = fill_template(env_dir,run_time,tile_num=tileNum)
    else:
      test_dir = strftime(env_dir, gmtime(run_time))
  else:
    test_dir = env_dir

    
  if not path.isdir(test_dir):
    if not yesterdayOK:
      icing_message.error('Directory (' + test_dir + ') from $' + env_var + ' does not exist ... exiting ...')
      return False
    else:
      test_dir = strftime(env_dir, gmtime(run_time-24*60*60))
      if not path.isdir(test_dir):
        icing_message.error('Directory (' + test_dir + ') from $' + env_var + ' does not exist ... exiting ...')
        return False
        
  return True


#######################################################################
# get_valid_path(run_time, base_dir, fn_tmplate, fcast='', add_date=False)
#   
# gets the pathname based on valid/forecast time  
#

def get_valid_path(run_time, base_dir, fn_tmplate, fcast='', add_date=False):

  gm_tuple = gmtime(run_time)

  #
  # The 6, 9 and 12 hour forecasts are generated on 3 hour 
  # intervals
  #
  if fcast > 3:
    run_shift = gm_tuple[3]%3
    gm_tuple = gmtime((run_time - run_shift*3600) + (fcast * 3600))
  else:
    gm_tuple = gmtime(run_time + (fcast * 3600))
    
  fcast_dir = 'f%02d' % fcast

  date_dir = '%04d%02d%02d' % \
	     (gm_tuple[0], gm_tuple[1], gm_tuple[2])
  
  if fcast > 0:
    fcast_dir = 'f%02d' % fcast
    pathname = path.join(base_dir, fcast_dir)
  else:
    pathname = base_dir
    
  if add_date:
    pathname = path.join(pathname, date_dir, strftime(fn_tmplate, gm_tuple))
  else:
    pathname = path.join(pathname, strftime(fn_tmplate, gm_tuple))
   
  icing_message.debug('looking for ' + pathname)

  return pathname


#######################################################################
# get_gen_path(run_time, base_dir, fn_tmplate, fcast='', add_date=False)
#   
# gets the pathname based on generation time  
#

def get_gen_path(run_time, base_dir, fn_tmplate, fcast=0, add_date=False):

  gm_tuple = gmtime(run_time)

  date_dir = '%04d%02d%02d' % \
	     (gm_tuple[0], gm_tuple[1], gm_tuple[2])
  
  if fcast > 0:
    fcast_dir = 'f%02d' % fcast
    pathname = path.join(base_dir, fcast_dir)
  else:
    pathname = base_dir
    
  if add_date:
    pathname = path.join(pathname, date_dir, strftime(fn_tmplate, gm_tuple))
  else:
    pathname = path.join(pathname, strftime(fn_tmplate, gm_tuple))
   
  icing_message.debug('looking for ' + pathname)

  return pathname


##################################################################
# TESTING                                                        #
##################################################################

##################################################################
# run_test(test_dir, test_name, run_time):
#		
# 

def run_test(test_dir, test_name, run_time):
  
  print 'TESTING: ', test_dir, ' and ', test_name

  the_path = build_input_path(test_dir, test_name, run_time)

  print 'test_dir = ', test_dir
  print 'test__file = ', test_name
  print 'the_path = ', the_path 
  print '\n\n'


##################################################################
# run_ruc_test(test_dir, test_name, run_time):
#		
# 

def run_ruc_test(test_dir, test_name, run_time):
  
  print 'TESTING: ', test_dir, ' and ', test_name
  try:
    a_dir = environ[test_dir]
  except KeyError:
    print 'Unknown variable: ', test_dir
    return
  try:
    a_file = environ[test_name]
  except KeyError, e:
    print 'Unknown variable: ', test_name
    return
  b_file = get_ruc_file(a_dir, a_file, run_time, 6)
  print 'a_dir = ', a_dir
  print 'a_file = ', a_file
  print 'b_file = ', b_file
  print '\n\n'


##################################################################
# MAIN for testing                                               #
##################################################################


if __name__ == "__main__":

  icing_message.debug = True
  
  run_time = get_archive_time('2004062212')

  run_test('TESTBED_PIREP_DIR', 'TESTBED_PIREP_NAME', run_time)
  run_test('TESTBED_METAR_DIR', 'TESTBED_METAR_NAME', run_time)
  run_test('CIP_SAT_E_VIS_DIR', 'CIP_SAT_E_VIS_NAME', run_time)
  run_test('CIP_SAT_E_IR_DIR', 'CIP_SAT_E_IR_NAME', run_time)
  run_test('CIP_SAT_E_SWIR_DIR', 'CIP_SAT_E_SWIR_NAME', run_time)
  run_test('CIP_SAT_E_WV_DIR', 'CIP_SAT_E_WV_NAME', run_time)
  run_test('CIP_SAT_W_VIS_DIR', 'CIP_SAT_W_VIS_NAME', run_time)
  run_test('CIP_SAT_W_IR_DIR', 'CIP_SAT_W_IR_NAME', run_time)
  run_test('CIP_SAT_W_SWIR_DIR', 'CIP_SAT_W_SWIR_NAME', run_time)
  run_test('CIP_SAT_W_WV_DIR', 'CIP_SAT_W_WV_NAME', run_time)
  run_test('CIP_RADAR_DIR', 'CIP_RADAR_NAME', run_time)
  run_test('CIP_METAR_DIR', 'CIP_METAR_NAME', run_time)
  run_test('CIP_PIREP_DIR', 'CIP_PIREP_NAME', run_time)
  run_test('CIP_LIGHTNING_DIR', 'CIP_LIGHTNING_NAME', run_time)
  run_ruc_test('CIP_RUC_HYBRID_DIR', 'CIP_RUC_HYBRID_NAME', run_time)
  run_ruc_test('CIP_RUC_PRESS_DIR', 'CIP_RUC_PRESS_NAME', run_time)


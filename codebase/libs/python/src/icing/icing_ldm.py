#!/usr/bin/env python
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:29 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

#
# icing_ldm -- this module handles inserting icing output
#              in GRIB format into LDM queue.
#

# import modules

import getopt

from os import chmod
from os import environ
from os import mkdir
from os import path
from os import system
from os import rename
from os import remove

from shutil import copy2

from string import split

from sys import argv
from sys import exit

from time import time
from time import gmtime
from time import sleep
from time import strftime

import icing_input
import icing_message

opt_run_time = ''
opt_ldm = False
opt_archive_mode = False
opt_in_dir = '/d2/iida/grib/fip'
opt_out_dir = '/d2/iida/grib/fip_records'
opt_algo_name = 'FIP'
opt_no_delete = False

# 1, 2, 3, 6, 9, 12 hour forecasts
fcast_idx_list = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18]
forecast_interval_code = ['A', 'B', 'C', 'D', 'E', 'F','G','H','I', 'J','K','L', 'M','N','O','P','Q','R','S'] 
file_ext = 'grb'

timestamp_template = '%Y%m%d%H'

field_names = ['ICE', 'SLD', 'ICE_SEV']

file_name_template = '%s_%s_%s_%s_%s.%s'

flight_level_codes = [  '93', # 10
                        '95', # 20
                        '90', # 30
                        '84', # 40
                        '85', # 50
                        '81', # 60
                        '78', # 70
                        '75', # 80
                        '73', # 90
                        '70', # 100
                        '67', # 110
                        '65', # 120
                        '62', # 130
                        '60', # 140
                        '57', # 150
                        '55', # 160
                        '53', # 170
                        '50', # 180
                        '48', # 190
                        '46', # 200
                        '45', # 210
                        '43', # 220
                        '41', # 230
                        '39', # 240
                        '38', # 250
                        '36', # 260
                        '34', # 270
                        '33', # 280
                        '32', # 290
                        '30'] # 300


###################################################################
# stage(gribfile):
#   
#
def stage(algo_name, run_time, fcast, gribfile, grib_record_dir,prodIDSuffix=""):

    print '\n  Inserting GRIB file on LDM:'

    global opt_algo_name
    opt_algo_name = algo_name

    stage_records(run_time, fcast, gribfile, grib_record_dir,prodIDSuffix)

    stage_file(run_time, gribfile, grib_record_dir, fcast,prodIDSuffix)


###################################################################
# stage_file(run_time, gribfile):
#   
#
def stage_file(run_time, gribfile, grib_record_dir, fcast=0,prodIDSuffix=""):

  #
  # generate filename for LDM
  #
  # the format is <YYYY><MM><DD><HH>_FIP.grb
  #
  tm_tuple = gmtime(run_time)

  year = tm_tuple[0]
  month = tm_tuple[1]
  day = tm_tuple[2]
  hour = tm_tuple[3]

  if fcast == 0:
     file_name = '%04d%02d%02d%02d_%s.grb' % \
		 (year, month, day, hour, opt_algo_name)
  else:
     file_name = '%04d%02d%02d%02dF%02d_%s.grb' % \
		 (year, month, day, hour, fcast, opt_algo_name)
     
  ldmfile = [ path.join(grib_record_dir, file_name) ]
  copy2(gribfile, ldmfile[0])

  #
  # reuse split_grib_file module functions
  #
  insert_files(ldmfile,prodIDSuffix)
  remove_files(ldmfile)
  

###################################################################
# stage_records(run_time, gribfile):
#   
#
def stage_records(run_time, fcast, gribfile, grib_record_dir,prodIDSuffix=""):
    
  grib_record_files = split_file(run_time, fcast, gribfile, grib_record_dir)
  insert_files(grib_record_files,prodIDSuffix)
  sleep(5)
  remove_files(grib_record_files)
  

##################################################################
# split_file():
#
#
def split_file(run_time, fcast, input_filename, out_dir):


  fcast_idx =  fcast_idx_list.index(fcast)

  tm_tuple = gmtime(run_time)

  #
  # open the input file
  # 
  input_file = file(input_filename)
  input_buffer = input_file.read()

  #
  # traverse file to find the start and stop indices of each record
  # in file
  #
  record_start = []
  record_stop = []
  for i in range(0, len(input_buffer)):
    if input_buffer[i:i+4] == 'GRIB':
      record_start.append(i)
      
    if input_buffer[i:i+4] == '7777':
      record_stop.append(i+4)

  grib_filenames = []
  i = 0
  for field_name in field_names:

    for level_code in flight_level_codes:

      #
      # generate the grib filename for record
      #
      grib_file = file_name_template % (opt_algo_name, \
                                        field_name, \
                                        timestamp_template, \
                                        forecast_interval_code[fcast_idx], \
                                        level_code, file_ext)
      grib_path = path.join(out_dir, strftime(grib_file , tm_tuple))
      grib_filenames.append(grib_path)

      #
      # create grib file for record
      #
      output_file = file(grib_path, 'w')
      output_file.write(input_buffer[record_start[i]:record_stop[i]])
      output_file.flush()
      output_file.close()

      i += 1
      
  input_file.close()

  return grib_filenames

##################################################################
# insert_files():
#
#
def insert_files(file_list,prodIDSuffix=""):

  for a_file in file_list:
    
    ldm_header = path.splitext(path.basename(a_file))[0]
    insert_cmd = 'pqinsert -l /home/ldm/logs/icing_ldm_notify.log -p "' + ldm_header + \
                 prodIDSuffix + '" -f EXP ' + a_file

    icing_message.debug('inserting ' + a_file + ' on LDM via: ' + insert_cmd)
 
    ret = system(insert_cmd)
    if ret:
       icing_message.warning('pqinsert failed on' + a_file)

    
##################################################################
# remove_files():
#
#
def remove_files(file_list):

  for a_file in file_list:
    
    icing_message.debug('removing ' + a_file)
    
    try:
      remove(a_file)
    except OSError, e:
      print e
      icing_message.warning('unable to remove ' + a_file)

    

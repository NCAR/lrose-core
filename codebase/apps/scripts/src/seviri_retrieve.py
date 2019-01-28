#!/usr/bin/env python2
#
# seviri_retrive.py -- this script uses ftp to retrive SEVIRI satellite
#   data from NOAA/NESDIS.
#
#   NESDIS Contact:
#      Bob Kuligowski, Ph.D.
#      Meteorologist
#      NOAA/NESDIS/Center for Satellite Applications and Research (STAR)
#
#      Mailing Address:
#      E/RA2   RM 712 WWBG
#      5200 Auth Rd.
#      Camp Springs, MD  20746-4304
#
#      Phone:  (301) 763-8251x192
#      Fax:    (301) 763-8580
#

# import modules

# system modules
import getopt
from time import asctime
from time import gmtime
from time import sleep
from time import mktime
from time import timezone
from time import time
from time import strftime
from string import split
from string import join
from sys import argv
from sys import exit
from sys import stdout
from os import environ
from os import path
from os import system
from os import remove
from os import mkdir
from email.MIMEText import MIMEText
import smtplib


# local modules from libs/python
import ftptools

#
# Initialize the command line arguments.
#
opt_debug = False
opt_ftp_user = 'anonymous'
opt_ftp_passwd = 'cunning@ucar.edu'
opt_ftp_server = 'ftp.orbit.nesdis.noaa.gov'
opt_ftp_dir = 'pub/smcd/emb/bobk/SEVIRI/NETCDF'
opt_output_dir = '/home/cunning/data/netcdf/seviri'
opt_log = '/tmp/ftp.log'
opt_stage = False
opt_reformat = False
opt_convert = False
opt_start_time = ''
opt_end_time = ''
opt_remove_tar_file = False
opt_reformat_dir = '/home/cunning/data/mdv/seviri'
opt_send_email = False
opt_compress = False

names = ['cunning@ucar.edu']
mail_subject = 'seviri_retrieve results'
min_tarfile_size = 150000000

################################################################################
# Local subroutines                                                            #
################################################################################

################################################################################
# handle_options(): handles the command line options
#

def handle_options():

  global opt_debug
  global opt_ftp_user
  global opt_ftp_passwd
  global opt_ftp_server
  global opt_ftp_dir
  global opt_output_dir
  global opt_sleep_secs
  global opt_max_num_tries
  global opt_log
  global opt_stage
  global opt_convert
  global opt_reformat
  global opt_start_time
  global opt_end_time
  global opt_remove_tar_file
  global opt_reformat_dir
  global opt_send_email
  global opt_compress
  
  #
  # Get the command line arguments
  #
  try:
    optlist, args = getopt.getopt(argv[1:], 'dh', \
                                  [ 'debug', 'help', \
                                    'ftp_dir=', 'ftp_host=', 'ftp_pwd=', 'ftp_user=', \
                                    'output_dir=', 'start=', 'end=',\
                                    'stage', 'remove_tarfile', \
                                    'send_email', 'compress', 'convert', 'reformat', 'reformat_dir='])
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

    if opt[0] == '--ftp_dir':
      opt_ftp_dir = opt[1]
    if opt[0] == '--ftp_host':
      opt_ftp_server = opt[1]
    if opt[0] == '--ftp_pwd':
      opt_ftp_passwd = opt[1]
    if opt[0] == '--ftp_user':
      opt_ftp_user = opt[1]
    if opt[0] == '--output_dir':
      opt_output_dir = opt[1]
    if opt[0] == '--stage':
      opt_stage = True
    if opt[0] == '--reformat':
      opt_reformat = True;
      opt_stage = True
    if opt[0] == '--reformat_dir':
      opt_reformat_dir = opt[1]
    if opt[0] == '--convert':
      opt_convert = True;
    if opt[0] == '--compress':
      opt_compress = True;
    if opt[0] == '--send_email':
      opt_send_email = True;
    if opt[0] == '--start':
      opt_start_time = opt[1]
    if opt[0] == '--end':
      opt_end_time = opt[1]
    if opt[0] == '--remove_tarfile':
      opt_remove_tar_file = True
    

  #
  # check the argument list
  #
  debug_print('Command line options:')
  debug_print('\tdebug = on')
  debug_print('\tftp_dir = '+ opt_ftp_dir)
  debug_print('\tftp_server = '+ opt_ftp_server)
  debug_print('\tftp_passwd = '+ opt_ftp_passwd)
  debug_print('\tftp_user = '+ opt_ftp_user)
  debug_print('\toutput_dir = '+ opt_output_dir)
  debug_print('\tstage = '+ str(opt_stage))
  debug_print('\tconvert = '+ str(opt_convert))
  debug_print('\tcompress = '+ str(opt_compress))
  debug_print('\tsend_email = '+ str(opt_send_email))
  debug_print('\tstart = '+ opt_start_time)
  debug_print('\tend = '+ opt_end_time)
  debug_print('\tremove_tarfile = '+ str(opt_remove_tar_file))


################################################################################
# print_usage(): Print the usage for this script
#

def usage():

  print 'Usage: ', prog_name, ' [options]'
  print 'Options:'
  print '   -h | --help             : Print usage and exit'
  print '   --ftp_dir <dir>         : FTP data directory'
  print '                             (default = ', opt_ftp_dir, ')'
  print '   --ftp_host <host>       : FTP server host name'
  print '                             (default = ', opt_ftp_server, ')'
  print '   --ftp_pwd <password>    : FTP server login password'
  print '                             (default = ', opt_ftp_passwd, ')'
  print '   --ftp_user <login>      : FTP server login name '
  print '                             (default = ', opt_ftp_user, ')'
  print '   --output_dir <dir>      : data directory for output'
  print '                             (default = ', opt_output_dir, ')'
  print '   -d | --debug            : Print debug messages'
  print '   --stage                 : unpacks and stages individual files from tar file'
  print '   --convert               : runs SeviriNc2Mdv on staged files'
  print '   --compress              : compresses staged files after conversion to MDV'
  print '   --send_email            : send email with list of processed files'
  print '   --start                 : start time for post analysis retrieval. format is '
  print '                              "YYYY MM DD HH MM SS"'
  print '   --end                   : end time for post analysis retrieval. format is '
  print '                              "YYYY MM DD HH MM SS". if option is left out one file matching'
  print '                              --start will be retrived.'


################################################################################
# debug_print(msg): prints debugging message
#

def debug_print(msg):

  if not opt_debug:
    return

  msg_hdr = '\tDEBUG -- '
  print msg_hdr + msg


###############################################################
# send_email(msg): handles messages --  
#        either print messages to stdout or send 
#        email. opt_send_mail is used to select
#        method
#

def send_email(msg):

  #
  # create mail message
  #
  try:
    sender = environ['USERNAME'] + '@ucar.edu'
  except :
    print 'no sender'
    if no_exit:
      return
    else:
      exit('I\'m done')
      
  mime = MIMEText(msg)
  mime['Subject'] = mail_subject
  mime['From'] = sender
  mime['To'] = join(names, ', ')

  #
  # create SMTP server and send message
  #
  try:
    s = smtplib.SMTP('localhost')
  except Exception, e:
    print 'Unable to create SMTP object: ', e
    exit('I\'m done')
    
  try:
    s.sendmail(sender, names, mime.as_string())
  except Exception, e:
    print 'Unable to send mail: ', e
    exit('I\'m done')

  s.quit()

###############################################################
# send_update(): 
#
def send_update(msg_lst):

  email_msg = 'Processed the following SEVIRI files: \n'
  for msg in msg_lst:
    if len(msg) != 0:
      email_msg = email_msg + '\n    ' + msg
  send_email(email_msg)

  
################################################################################
# convert_file_time(file_name): creates the time from time stamp encoded in the
#                  file name
#
# returns unix-like time 
#
#   Input files have the following format
#       200510052327.nc.tar --> YYYYMMDDHHMM.nc.tar
#
def convert_file_time(file_name):

  year = int(file_name[0:4])
  month = int(file_name[4:6])
  day = int(file_name[6:8])
  hour = int(file_name[8:10])
  minute = int(file_name[10:12])
  second = 0
  
  time_tuple = (year, month, day, hour, \
                minute, second, 0, 0, 0)
  gmt_time = mktime(time_tuple) - timezone

  return gmt_time

################################################################################
# convert_arg_time(time str): creates the time from time given in the argument
#                  list
#
# returns unix-like time 
#
#
def convert_arg_time(time_str):

  split_arg = split(time_str, ' ')
  year = int(split_arg[0])
  month = int(split_arg[1])
  day = int(split_arg[2])
  hour = int(split_arg[3])
  minute = int(split_arg[4])
  second = int(split_arg[5])
  
  time_tuple = (year, month, day, hour, \
                minute, second, 0, 0, 0)
  gmt_time = mktime(time_tuple) - timezone

  return gmt_time


################################################################################
# stage_files(base_path, file_name):
#
# 1) create date and time subdirectories
# 2) untar file into final location
# 3) remove tar file
#

def stage_files(base_path, file_name):
  
  debug_print('staging ' + path.join(base_path, file_name))
  #
  # make the date subdirectory if it doesn't exist
  #
  date_dir = path.join(base_path, file_name[0:8])
  if not path.isdir(date_dir):
    try:
      mkdir(date_dir)
    except OSError, e:
      msg =  'mkdir ' + date_dir + ' failed --' +  str(e) 
      print msg
      exit

  #
  # make the time subdirectory exists, if it doesn't exist
  #
  time_subdir = file_name[8:12] + '00'
  final_dir = path.join(date_dir, time_subdir)
  if not path.isdir(final_dir):
    try:
      mkdir(final_dir)
    except OSError, e:
      msg =  'mkdir ' + final_dir + ' failed --' +  str(e) 
      print msg
      exit
  #
  # unpack tar file  
  #
  tar_file = path.join(base_path, file_name)
  tar_command = 'tar -xf ' + tar_file + ' -C ' + final_dir
  ret = system(tar_command)
  if ret:
    print 'system( ' + tar_command + ' ) failed'
    exit

  if opt_remove_tar_file:
    try:
      remove(tar_file)
    except OSError, e:
      print 'unable to remove ' + tar_file + ' -- ' + str(e)
      
  return final_dir


################################################################################
# reformat_files(stage_dir): this function runs SeviriNc2Mdv 
#
# 1) run SeviriNc2Mdv on files in stage_dir
# 2) run gzip on files in stage_dir
#

def reformat_files(stage_dir):


  debug_print('reformatting files in  ' + stage_dir)

  convert_option = ''
  if opt_convert:
    convert_option = '-convert '

  reformat_command = 'SeviriNc2Mdv ' + convert_option + '-in_dir_list ' + \
                     stage_dir + ' -out_url ' + opt_reformat_dir + \
                     ' -verbose'

  ret = system(reformat_command)
  if ret:
    print 'system( ' + reformat_command + ' ) failed'
    exit

  if opt_compress:
    debug_print('compressing files in  ' + stage_dir)
    gzip_command = 'gzip ' + path.join(stage_dir, '*.nc')
    ret = system(gzip_command)
    if ret:
      print 'system( ' + gzip_command + ' ) failed'
    

  

################################################################################
#  real_time_retrieve():
#
#  manages real-time file retrieval
#

def real_time_retrieve():

  print 'This function has not been implemented yet.'
  return



################################################################################
#  post_analysis_retrieve():
#
#  manages post analysis file retrieval
#

def post_analysis_retrieve():

  max_num_files = 10

  debug_print('Inside post_analysis_retrieve()')

  ftp_info = ftptools.FTP_info(opt_ftp_server, opt_ftp_dir, opt_ftp_user, \
                               opt_ftp_passwd, opt_log)
  
  listing = ftp_info.get_filtered_listing('*.tar')

  if len(listing) == 0:
    debug_print( 'no files in list')
    return
  
  #
  # search through list for file times within start and end times
  #
  start_time = convert_arg_time(opt_start_time)
  if len(opt_end_time) == 0:
    end_time = start_time
  else:
    end_time = convert_arg_time(opt_end_time)

  num_files = 1
  file_list = []
  for name in listing:
    file_time = convert_file_time(name)
    if file_time >= start_time and file_time <= end_time:
      begin_time = time()
      debug_print('retieving ' + name + ' at ' + asctime(gmtime(begin_time)))
      file_list.append(name)

      #
      # retrieve file
      #
      fetch_path =  path.join(opt_output_dir, name)
      ftp_info.fetch(name, fetch_path)
      if not path.isfile(fetch_path):
        print 'file was not retrieved successfully.'
        continue
  
      #
      # make sure file is complete
      #
      if path.getsize(fetch_path) < min_tarfile_size:
        debug_print(fetch_path + ' size too small continue')
        continue

      #
      # stage file
      #
      if opt_stage:
        stage_dir = stage_files(opt_output_dir, name)

      if opt_reformat:
        reformat_files(stage_dir)
                   
      elapsed_sec = time() - begin_time 
      debug_print('elapsed time (minutes): ' + str(elapsed_sec/60))


      if num_files >= max_num_files:
        if opt_send_email:
          send_update(file_list)
        num_files = 0
        file_list = []
      num_files = num_files + 1


  if opt_send_email and len(file_list) > 0:
    send_update(file_list)
      
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
  # either run in real-time or retrieve files for post analysis
  #
  if len(opt_start_time) == 0:
    real_time_retrieve()
  else:
    post_analysis_retrieve()

  exit(0)

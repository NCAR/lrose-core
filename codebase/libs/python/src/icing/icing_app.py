#!/usr/bin/env python
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:28 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

#
# icing_app
#

from os import system
from datetime import datetime

import icing_message
import icing_input



##################################################################
# run_app(): 
#   
#		
#


def run_app(app_name, instance, check_outdir=True, start_time='',
            stop_time='', in_file='', opt_in_file='', out_dir='', diag_dir='', rt=''):

  startTime = datetime.now()
  icing_message.debug('\tStarting ' + app_name + ' : ' + instance + " at time " + startTime.strftime("%Y-%m-%d %H:%M"))

  # build command line
  if len(start_time) > 0 and len(stop_time) > 0 and  len(out_dir) > 0:
    icing_message.debug('start_time = ' + start_time)
    icing_message.debug('stop_time = ' + stop_time)
    icing_message.debug('out_dir = ' + out_dir)
    command = 'start_' + app_name + '.%s \'%s\' \'%s\' %s' % \
              (instance, start_time, stop_time, instance)
  elif len(in_file) > 0 and len(out_dir) > 0 and len(diag_dir) > 0 and len(opt_in_file) == 0:
    icing_message.debug('in_file = ' + in_file)
    icing_message.debug('out_dir = ' + out_dir)
    icing_message.debug('diag_dir = ' + diag_dir)
    command = 'start_' + app_name + '.%s %s %s %s %s' % \
              (instance, in_file, out_dir, diag_dir, instance) 
  elif len(in_file) > 0 and len(out_dir) > 0 and len(opt_in_file) == 0 and len(diag_dir) == 0:
    icing_message.debug('in_file = ' + in_file)
    icing_message.debug('out_dir = ' + out_dir)
    command = 'start_' + app_name + '.%s %s %s %s' % \
              (instance, in_file, out_dir, instance) 
  elif len(start_time) > 0 and  len(out_dir) > 0:
    icing_message.debug('start_time = ' + start_time)
    icing_message.debug('out_dir = ' + out_dir)
    command = 'start_' + app_name + '.%s \'%s\' %s' % \
              (instance, start_time, instance) 
  elif len(in_file) > 0 and len(opt_in_file) > 0 and len(out_dir) > 0 and len(rt) == 0:
    icing_message.debug('in_file = ' + in_file)
    icing_message.debug('opt_in_file = ' + opt_in_file)
    icing_message.debug('out_dir = ' + out_dir)
    command = 'start_' + app_name + '.%s %s %s %s %s' % \
              (instance, in_file, opt_in_file, out_dir, instance) 
  elif len(in_file) > 0 and len(opt_in_file) > 0 and len(out_dir) > 0 and len(rt) > 0:
    icing_message.debug('run_time = ' + rt)
    icing_message.debug('in_file = ' + in_file)
    icing_message.debug('opt_in_file = ' + opt_in_file)
    icing_message.debug('out_dir = ' + out_dir)
    command = 'start_' + app_name + '.%s %s %s %s %s %s' % \
              (instance, rt, in_file, opt_in_file, out_dir, instance) 
  elif len(in_file) == 0 and len(opt_in_file) == 0 and len(out_dir) > 0 and len(rt) > 0:
    icing_message.debug('run_time = ' + rt)
    icing_message.debug('out_dir = ' + out_dir)
    command = 'start_' + app_name + '.%s %s %s \'%s\'' % (instance, out_dir, instance, rt)
  elif len(in_file) > 0:
		icing_message.debug('in_file = ' + in_file)
		command = 'start_' + app_name + '.%s %s %s' % (instance, in_file, instance)
  elif len(in_file) == 0 and len(opt_in_file) == 0 and len(out_dir) > 0 and \
       len(start_time) == 0 and len(stop_time) == 0:
    icing_message.debug('out_dir = ' + out_dir)
    command = 'start_' + app_name + '.%s %s' % (instance, instance) 
  else:
    icing_message.error('command not set ... exiting.')
    return ''
    
  ret = system(command)
  if ret:
    icing_message.error(app_name + ' failed for instance: ' + instance)
    
  endTime = datetime.now()
  icing_message.debug('\t'+app_name + ' finished run at ' + endTime.strftime("%Y-%m-%d %H:%M"))

  if check_outdir:
    return icing_input.check_for_file(out_dir)

  return ''
    
##################################################################
# MAIN for testing                                               #
##################################################################


if __name__ == "__main__":

  from time import time
  
  run_time = int(time()/3600)*3600
  start_str =  icing_input.build_app_time_str(run_time, 3600)
  end_str = icing_input.build_app_time_str(run_time)
  the_dir = '/dev/null'
  the_file = 'junk'
  ruc_file = 'junk.grb'
  
  print 'test for run_app'
  run_app_test('app', 'cip', start_time=start_str, stop_time=end_str, out_dir=the_dir)
  print 'test for run_app2, run_app4'
  run_app_test('app', 'cip24', in_file=the_file, out_dir=the_dir)
  print 'test for run_app6'
  run_app_test('app', 'cip6', check_outdir=False, in_file=the_file, out_dir=the_dir)
  print 'test for run_app3'
  run_app_test('app', 'cip3', start_time=start_str, out_dir=the_dir)
  print 'test for run_app5'
  run_app_test('app', 'cip5', in_file=the_file, opt_in_file=ruc_file, out_dir=the_dir)

# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:29 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# module 'pmu' -- Process registration

#
# Load the needed modules
#

import os
import time

debug = 0

#########################################################################
# Exported functions
#########################################################################

#########################################################################
# auto_init(): Initialize automatic procmap registration.
#

def auto_init(prog_name, instance, reg_interval):

  # Define the necessary global variables

  global PMU_prog_name, PMU_instance, PMU_reg_interval
  global PMU_last_reg, PMU_start_time, PMU_pid

  # Save the information in global variables

  PMU_prog_name = prog_name
  PMU_instance = instance
  PMU_reg_interval = reg_interval
  PMU_last_reg = -1
  PMU_start_time = time.time()
  PMU_pid = os.getpid()

  if debug:
    print "PMU_auto_init:"
    print "prog_name = ", PMU_prog_name
    print "instance = ", PMU_instance
    print "reg_interval = ", PMU_reg_interval
    print "pid = ", PMU_pid

  force_register("Inside PMU_auto_init()")


#########################################################################
# auto_register():  Automatically registers if PMU_reg_interval
#                       seconds have expired since the previous registration.
#
# This routine may be called frequently, registration will only occur at
# the specified reg_interval.
#

def auto_register(status_string):

  # Define the proper global variables

  global PMU_last_reg

  # Register if enough time has elapsed

  current_time = time.time()

  if (current_time - PMU_last_reg) > PMU_reg_interval:
    command = \
      "procmap_register -name %s -instance %s -status_str \"%s\" -pid %d -reg_int %d -start %d" % \
        (PMU_prog_name, PMU_instance, status_string, PMU_pid, \
         PMU_reg_interval, PMU_start_time)

    if debug:
      print "Command: ", command

    os.system(command)

    PMU_last_reg = current_time


#########################################################################
# force_register():  Forced registration.
#
# This routine should only be called from places in the code which do
# not run frequently.  Call auto_register() for most places in the
# code.
#

def force_register(status_string):

  # Define the proper global variables

  global PMU_last_reg

  # Register

  current_time = time.time()

  command = \
    "procmap_register -name %s -instance %s -status_str \"%s\" -pid %d -reg_int %d -start %d" % \
      (PMU_prog_name, PMU_instance, status_string, PMU_pid, \
       PMU_reg_interval, PMU_start_time)

  if debug:
    print "Command: ", command

  os.system(command)

  PMU_last_reg = current_time


#########################################################################
# auto_unregister():  Automatically unregisters - remembers process
#                     name and instance
#

def auto_unregister():
  command = ("procmap_unregister -name %s -instance %s -pid %d" % \
              (PMU_prog_name, PMU_instance, PMU_pid))

  if debug:
    print "Command: ", command

  os.system(command)

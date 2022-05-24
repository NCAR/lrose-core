#!/usr/bin/env python2

# This program copies realtime GDCP files from the ftp site, which
# has been cross mounted to satrs.
# It will check the input directory and only copy files that don't already
# exist in the output directory.

import os, sys, string, glob,operator
import time
#from conv import *

def usage():
   print '\n\n'
   print 'Usage: %s' % sys.argv[0]
   print '\n\n'
   sys.exit(1)

# jd_to_mmdd converts a Julian date (YYYYJdd) to a regular date (YYYYMMDD)
#  Using the time module gives errors when this is attempted, so it must
#  be forced.
def jd_to_mmdd(jdate):
    
    year = int(jdate[:4])
    jd = int(jdate[4:])
    if ((year%4) == 0):
        if (jd <= 31):
            month = 1
            day = jd
        elif (jd <= 60):
            month = 2
            day = jd - 31
        elif (jd <= 91):
            month = 3
            day = jd - 60
        elif (jd <= 121):
            month = 4
            day = jd - 91
        elif (jd <= 152):
            month = 5
            day = jd - 121
        elif (jd <= 182):
            month = 6
            day = jd - 152
        elif (jd <= 213):
            month = 7
            day = jd - 182
        elif (jd <= 244):
            month = 8
            day = jd - 213
        elif (jd <= 274):
            month = 9
            day = jd - 244
        elif (jd <= 305):
            month = 10
            day = jd - 274
        elif (jd <= 335):
            month = 11
            day = jd - 305
        elif (jd <= 366):
            month = 12
            day = jd - 335
        else:
            print "Not a valid Julian day"
            sys.exit(2)

    elif ((year%4) != 0):
        if (jd <= 31):
            month = 1
            day = jd
        elif (jd <= 59):
            month = 2
            day = jd - 31
        elif (jd <= 90):
            month = 3
            day = jd - 59
        elif (jd <= 120):
            month = 4
            day = jd - 90
        elif (jd <= 151):
            month = 5
            day = jd - 120
        elif (jd <= 181):
            month = 6
            day = jd - 151
        elif (jd <= 212):
            month = 7
            day = jd - 181
        elif (jd <= 243):
            month = 8
            day = jd - 212
        elif (jd <= 273):
            month = 9
            day = jd - 243
        elif (jd <= 304):
            month = 10
            day = jd - 273
        elif (jd <= 334):
            month = 11
            day = jd - 304
        elif (jd <= 365):
            month = 12
            day = jd - 334
        else:
            print "Not a valid Julian Day"
            sys.exit(2)

    newdate = "%d%02d%02d" % (year, month, day)
    return newdate

arg_len = len(sys.argv)
if arg_len != 1:
   usage()

indir = '/var/autofs/mnt/ftp/root/incoming/irap/cwolff/larc'
outdir = '/d1/cwolff/gdcp/netcdf'

Satellite = ['g11', 'g13']

# Get the current date and the previous day and put them in a list
J = []

t = time.localtime(time.time() - (24*3600))
j = '%d%03d' % (t[0], t[7])
J.append(j)

t = time.localtime(time.time())
j = '%d%03d' % (t[0], t[7])
J.append(j)

# Go through both days and both satellites
for sat in Satellite:
   if sat == 'g11':
      filepre = 'G11'
   elif sat == 'g12':
      filepre = 'G12'
   elif sat == 'g13':
      filepre = 'G13'
      
   for jd in J:
      ymd = jd_to_mmdd(jd)

      # Create the output directory if it doesn't exist
      satdir = 'larc_gdcp_rr_%s' % (sat)
      
      datedir = os.path.join(outdir, satdir, ymd)
      if not os.path.exists(datedir):
         os.mkdir(datedir)

      # List the directories
      inlist = os.listdir(indir)
      outlist = os.listdir(datedir)

      # Go through the files in the input directory
      for infile in inlist:

         # Only get data for the current date and satellite
         if infile[:3] != filepre:
            continue
         if infile[11:18] != jd:
            continue

         hhmm = infile[19:23]
         
         # Check the files against those in the date directory by using the times
         found = 0
         for outfile in outlist:
            if hhmm == outfile[:4]:
               # The file exists locally so it doesn't need to be copied
               found = 1

         # If the file wasn't found then copy it over, unzip it, and rename it
         if found == 0:
            if os.stat(os.path.join(indir, infile))[6] < 15000000:
               # Give it a minute to come in
               time.sleep(60)
               if os.stat(os.path.join(indir, infile))[6] < 10000000:
                  # If it's still small after a minute then ignore it and move on
                  continue
               
            command = 'cp %s/%s %s' % (indir, infile, datedir)
            os.system(command)

            command = 'gunzip %s/%s' % (datedir, infile)
            os.system(command)

            command = ' mv %s/%s %s/%s00.cdf' % (datedir, infile[:-3], datedir, hhmm)
            os.system(command)
      

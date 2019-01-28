#!/usr/bin/env python2

# This program copies GFIP files from NCEP.  It is meant to be run once
# per day and will get files from the previous day.

import os, sys, string, glob,operator
import time
#from conv import *

def usage():
   print '\n\n'
   print 'Usage: %s [date]' % sys.argv[0]
   print ' If no date is given then it will attempt to get files from the previous day'
   print '\n\n'
   sys.exit(1)

# Main
arg_len = len(sys.argv)
if arg_len > 2:
   usage()

if arg_len == 1:
   # Get the previous day
   utime = time.time() - 86400
   t = time.localtime(utime)
   ymd = '%d%02d%02d' % (t[0], t[1], t[2])
else:
   ymd = sys.argv[1]
   
baseftppath = 'ftp://ftp.emc.ncep.noaa.gov/unaff/ymao/gfip/gfs'
outdir = '/d3/cwolff/gfip/grib'
password = 'cwolff@ucar.edu'

ilist = ['00', '06', '12', '18']
flist = ['03', '06']

# Create the output directory if it doesn't exist
gfipdir = os.path.join(outdir, ymd)
if not os.path.exists(gfipdir):
   os.mkdir(gfipdir)

os.chdir(gfipdir)

# Go through all initialization and forecast times
for i in ilist:
      
   for f in flist:

      # Create the file name to get
      infile = 'gfs.t%sz.fip.grbf%s' % (i,f)

      # Get the file
      command = 'wget --password=%s %s/gfs.%s/%s' % (password, baseftppath, ymd, infile)
      ret = os.system(command)
      
      if ret == 0:
         # Rename the file
         command = 'mv %s gfip.i%s.f%s.grb' % (infile, i, f)
         os.system(command)


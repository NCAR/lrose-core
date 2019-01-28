#!/usr/bin/env python2

# This program converts a NCAR Graphics image into gif.

import os, string, sys, getopt, math, time

def usage():
    print "usage: %s [-d directory] [ -f infile1 [infile2 infile3 ...]]" % sys.argv[0]
    print "-f will convert infile where infile is the NCAR Graphics meta file to a gif"
    print "-d will convert all ncgm files in a directory to gif"
    sys.exit(2)

arg_len = len(sys.argv)
if arg_len < 3:
   usage()

option = sys.argv[1]

if (option == '-f'):

    for i in xrange(2, len(sys.argv)):

        infile = sys.argv[i]
        fname = infile[:-5]

        command = "/usr/local/ncarg/bin/ctrans -quiet -d sun -res 680x680 %s > %s.sun" % (infile, fname)
        os.system(command)
        
        command = "/usr/bin/convert %s.sun %s.gif" % (fname, fname)
        os.system(command)

        command = "/bin/rm %s.sun" % (fname)
        os.system(command)

elif (option == '-d'):

    indir = sys.argv[2]
    file_list = os.listdir(indir)
    for infile in file_list:
        if (infile[-5:] != '.ncgm'):
            continue

        fname = infile[:-5]

        command = "/usr/local/ncarg/bin/ctrans -quiet -d sun -res 680x680 %s/%s > %s/%s.sun" % (indir, infile, indir, fname)
        os.system(command)
        
        command = "/usr/bin/convert %s/%s.sun %s/%s.gif" % (indir, fname, indir, fname)
        os.system(command)

        command = "/bin/rm %s/%s.sun" % (indir, fname)
        os.system(command)

        

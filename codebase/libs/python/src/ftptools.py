#!/usr/bin/env python
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:29 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

# Given an ftp site, a directory and an output file, visit the site
# and store the directory in the output file.
# NCEP 140.90.6.103
# /ncepe/nggrib    
# ruc2a.990122

from ftplib import FTP
from fcntl import *
import os, sys
import string
import time
import getopt
from time import sleep

def rem_nl(line):
    return line[:-1]

class FTP_info:
    def __init__(self, ftp_address, remote_dir, user, passwd, logg):
        self.address = ftp_address
        self.remote_dir = remote_dir
        self.user = user
        self.passwd = passwd
        self.listing = []
        self.listing_time = 0
        self.listing_delta = 60
        self.logg = logg
        
    # Write full listing of remote directory to output_file
    def print_full_listing(self):
        ftp = FTP(self.address)
        if (self.user == "" or self.passwd == ""):
            ftp.login()
        else:
            ftp.login(self.user, self.passwd)
        ftp.set_pasv(1)
        if self.remote_dir != "":
            ftp.cwd(self.remote_dir)
        ftp.dir()
        ftp.close()


    # Write a short listing of remote directory to output_file
    def print_listing(self):
        ftp = FTP(self.address)
        if (self.user == "" or self.passwd == ""):
            ftp.login()
        else:
            ftp.login(self.user, self.passwd)
        ftp.set_pasv(1)
        if self.remote_dir != "":
            ftp.cwd(self.remote_dir)
        listing = ftp.nlst()

        for i in xrange(len(listing)):
            print listing[i]

	sys.stdout.flush() 
        ftp.close()


    # Get short listing of remote directory
    def get_listing(self):
        ftp = FTP(self.address)
        if (self.user == "" or self.passwd == ""):
            ftp.login()
        else:
            ftp.login(self.user, self.passwd)
        ftp.set_pasv(1)
        if self.remote_dir != "":
            ftp.cwd(self.remote_dir)
        self.listing = ftp.nlst()
        self.listing.sort()
        ftp.close()
        self.listing_time = time.time()
        return self.listing

    # Get filtered short listing of remote directory
    def get_filtered_listing(self, filter):
        ftp = FTP(self.address)
        if (self.user == "" or self.passwd == ""):
            ftp.login()
        else:
            ftp.login(self.user, self.passwd)
        ftp.set_pasv(1)
        if self.remote_dir != "":
            ftp.cwd(self.remote_dir)
        self.listing = ftp.nlst(filter)
        self.listing.sort()
        ftp.close()
        self.listing_time = time.time()
        return self.listing

    # Fetch single remote file fname and store in output_file
    def fetch(self, fname, output_file):
        ftp = FTP(self.address)
        if (self.user == "" or self.passwd == ""):
            ftp.login()
        else:
            ftp.login(self.user, self.passwd)
        ftp.set_pasv(1)
        if self.remote_dir != "":
            ftp.cwd(self.remote_dir)
        out_fp = open(output_file, 'wb')
        constant = -1
        while ftp.size(fname) != constant :
          constant = ftp.size(fname)
          sleep (5)
        ftp.retrbinary("RETR %s" % fname, out_fp.write)
        out_fp.close()
        ftp.close()

    # Fetch single remote file fname and store in output_file
    def fetch_no_size_ck(self, fname, output_file):
        ftp = FTP(self.address)
        if (self.user == "" or self.passwd == ""):
            ftp.login()
        else:
            ftp.login(self.user, self.passwd)
        ftp.set_pasv(1)
        if self.remote_dir != "":
            ftp.cwd(self.remote_dir)
        out_fp = open(output_file, 'wb')
        ftp.retrbinary("RETR %s" % fname, out_fp.write)
        out_fp.close()
        ftp.close()

    # Fetch single remote file fname and store in output_file
    def size (self, fname):
        ftp = FTP(self.address)
        if (self.user == "" or self.passwd == ""):
            ftp.login()
        else:
            ftp.login(self.user, self.passwd)
        ftp.set_pasv(1)
        if self.remote_dir != "":
            ftp.cwd(self.remote_dir)
        constant = -1
        while ftp.size(fname) != constant :
          constant = ftp.size(fname)
          sleep (10)
        ftp.close()

    # Fetch remote files using a file list and store in directory
    def fetch_files(self, flist, dir, out_flist=[]):
        ftp = FTP(self.address)
        if (self.user == "" or self.passwd == ""):
            ftp.login()
        else:
            ftp.login(self.user, self.passwd)
        ftp.set_pasv(1)
        if self.remote_dir != "":
            ftp.cwd(self.remote_dir)
        if out_flist == []:
            for f in flist:
                out_file = "%s/%s" % (dir, f)
                out_fp = open(out_file, 'wb')
                ftp.retrbinary("RETR %s" % f, out_fp.write)
                out_fp.close()
        else:
            ct = 0
            for f in flist:
                out_file = "%s/%s" % (dir, out_flist[ct])
                ct = ct + 1
                out_fp = open(out_file, 'wb')
                ftp.retrbinary("RETR %s" % f, out_fp.write)
                out_fp.close()
        ftp.close()

    #
    # Get the latest set of files given lists of particular types and
    # hours.  For example, get the latest analysis and 3h forecast files
    # which have been generated on a model run that occurs on the hours 3, 6 or 9
    #
    #
    # We start with a file containing the files that have already been received for
    # the day (could be empty)
    #
    # The algorithm proceeds as follows:
    #
    # Get the latest remote directory listing and store in a list.
    #
    # Iterate through the list of types:
    #
    # Find the latest file of a given type and hour of interest.
    #
    # Check whether this file is in the files that have already been received.
    #
    # If so, continue.
    #
    # If not, fetch it.  If successful, add this file to the list of files
    # that have already been received.  In order to guarantee the update,
    # lock the file, modify it atomically and then unlock it.
    #
    def get_update(self, find_latest, files_recd, output_dir):
        curr_time = time.time()
        listing = []
        if (curr_time - self.listing_time > self.listing_delta):
            # Get the latest remote directory listing.
            listing = self.get_listing()
        else:
            listing = self.listing

        sys.stdout.flush()

        # Open the list of the files that have been received
        files = []
        if (os.path.exists(files_recd)):
            in_fp = open(files_recd, "r")
            flock(in_fp.fileno(), LOCK_SH)
            files = in_fp.readlines()
            flock(in_fp.fileno(), LOCK_UN)
            in_fp.close()

        file_list = map(rem_nl, files)

        # Find the latest file of the given type and hour of interest after start_hour
        (fname, fhour) = find_latest.find(listing)

        if (fname == ""):
            return "", ""

        # Check whether this file is in the files that have already been received.
        if (fname in file_list):
            return "", ""

        # Fetch the file
        if (output_dir[-1] == '/'):
            output_file = "%s%s" % (output_dir, fname)
        else:
            output_file = "%s/%s" % (output_dir, fname)

        self.logg.write_time("Info: trying to fetch %s" % fname)
        self.fetch(fname, output_file)
        self.logg.write_time("Info: fetched %s" % fname)

        # Add file to files_recd
        out_fp = open(files_recd, "a")
        ct = 0

        # Lock and block
        flock(out_fp.fileno(), LOCK_EX)
        string = "%s\n" % fname
        out_fp.write(string)
        flock(out_fp.fileno(), LOCK_UN)
        out_fp.close()

        return (fname, fhour)


def usage(command):
    print "%s [-u user] [-p password] ftp_address remote_dir" % command
    sys.exit(2)
    
if __name__ == '__main__':
    if len(sys.argv) < 3:
        usage(sys.argv[0])

    try:
	optlist, args = getopt.getopt(sys.argv[1:], "p:u:")
    except:
	print "%s: error in parsing options, %s" % (sys.argv[0], sys.exc_value)
	usage(sys.argv[0])
	
    user = ""
    passwd = ""
    for i in xrange(0, len(optlist)):
        # print optlist[i][0]
	if optlist[i][0] == "-h":
            usage(sys.argv[0])
	elif optlist[i][0] == "-u":
	    user = optlist[i][1]
	elif optlist[i][0] == "-p":
	    passwd = optlist[i][1]
            
    ftp_address = args[0]
    remote_dir = args[1]
    flist = args[2:]
    ftp_info = FTP_info(ftp_address, remote_dir, user, passwd)

    out_flist = map(lambda x: "%s.r" % x, flist)
    try:
        #ftp_info.print_full_listing()
        ftp_info.fetch_files(flist, ".", out_flist)
    except:
        print "ftp error: %s, %s" % (sys.exc_type, sys.exc_value)


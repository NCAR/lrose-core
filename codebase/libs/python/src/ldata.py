# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:29 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# module 'ldata' -- Reading and writing _latest_data_info files.

#
# Load the needed modules
#

import os
import stat
import string
import sys
import time

import ral_datetime


#########################################################################
# Ldata class definition
#########################################################################

class Ldata:
    def __init__(self, prog_name, debug):
        self.ltime = ral_datetime.DateTime()
        self.debug = debug
        self.not_exist_print = 0
        self.too_old_print = 0
        self.not_modified_print = 0
        self.prev_mod_time = 0
        self.n_fcasts_alloc = 0
        self.fcast_lead_times = 0
        self.prog_name = prog_name
        self.source_str = ""
        self.file_name = "_latest_data_info"
        self.file_path = ""
        self.temp_path = ""
        self.info = LdataInfo()


    #####################################################################
    # set_file_name()
    #
    # Sets the file name to be used in the routines.
    #
    # The default name is '_latest_data_info'.
    #

    def set_file_name(self, file_name):
        self.file_name = file_name


    #####################################################################
    # info_print()
    #
    # Prints info to output stream
    #
    # returns 0 on success, -1 on failure
    #

    def info_print(self, stream):
        stream.write("%d %d %d %d %d %d %d\n" % \
                     (self.ltime.unix_time, self.ltime.year, \
                      self.ltime.month, self.ltime.day, \
                      self.ltime.hour, self.ltime.minute, \
                      self.ltime.second))
        stream.write(self.info.file_ext + "\n")
        stream.write(self.info.user_info_1 + "\n")
        stream.write(self.info.user_info_2 + "\n")
        stream.write("0\n")


    #####################################################################
    # info_read()
    #
    # Read the struct data from the current file info, including forecast
    # lead times if they are present.
    #
    # Inputs:
    #
    #   handle: see LDATA_init_handle()
    #
    #   source_str:
    #
    #     this is the data directory.
    # 
    #   max_valid_age:
    #
    #     This is the max age (in secs) for which the 
    #     latest data info is considered valid. If the info is
    #     older than this, we need to wait for new info.
    #
    #     If max_valid_age is set negative, the age test is not done.
    #
    # Side effects:
    #
    #    (1) If new data found, sets handle->prev_mod_time to
    #        file modify time.
    #
    #        NOTE: For this to work, the handle must be static between calls
    #        since the prev_mod_time in the handle is used to determine when
    #        the time of the file has changed.
    #
    #    (2) Fills out the file path in the handle.
    #
    # Returns:
    #
    #    0 on success, -1 on failure.
    #

    def info_read(self, source_str, max_valid_age):
        # Set the file path

        self.file_path = source_str + "/" + self.file_name

        # Check if the info file exists

        try:
            file_stat = os.stat(self.file_path)
        except:
            return -1

        # Compute age, check for max valid age

        if max_valid_age >= 0:
            now = time.time()
            file_age = now - file_stat[stat.ST_MTIME]
            if file_age > max_valid_age:
                if self.debug and self.too_old_print:
                    print "LDATA_info_read: info_file ", self.file_path, \
                          " too old"
                    self.too_old_print = 0
                return -1

        # Check for modified file time

        if file_stat[stat.ST_MTIME] == self.prev_mod_time:
            if self.debug and self.not_modified_print:
                print "LDATA_info_read: info file ", self.file_path, \
                      " not modified"
                print "Last mod time: ", time.ctime(self.prev_mod_time)
                self.not_modified_print = 0
            return -1

        # Read in file

        if self.read_info_file(self.file_path):
            if self.debug:
                print "LDATA_info_read: error reading info file ", \
                      self.file_path

                # Remove bad ldata file

                os.unlink(self.file_path)

            return -1

        if self.debug:
            print "LDATA_info_read: success reading ", self.file_path

        # Set prev_mod_time to save it for next iteration

        self.prev_mod_time = file_stat[stat.ST_MTIME]

        # Reset the print flags

        self.not_exist_print = 1
        self.too_old_print = 1
        self.not_modified_print = 1

        return 0


    #####################################################################
    # info_read_blocking()
    #
    # Read latest data info, blocking until info is available.
    #
    # See Ldata.info_read() for the non-blocking behavior upon
    # which this function is based.
    #
    # Inputs:
    #
    #   source_str: see Ldata.info_read()
    #
    #   max_valid_age (secs): see Ldata.info_read()
    #
    #   sleep_msecs (millisecs):
    #     While in the blocked state, the program sleeps for sleep_msecs
    #     millisecs at a time before checking again.
    #
    #   heartbeat_func(): heartbeat function
    #
    #     Each cycle, the function heartbeat_func() is called to allow
    #     any heartbeat actions to be carried out. If heartbeat_func is
    #     set to 0, it is not called.
    #
    #     The string arg passed to the heartbeat
    #     function is "In LDATA_info_read_blocking".
    #
    # Side effects:
    #
    #   See Ldata.info_read()
    #

    def info_read_blocking(self, source_str, max_valid_age, \
                           sleep_msecs, heartbeat_func):
        while self.info_read(source_str, max_valid_age):
            if heartbeat_func != 0:
                heartbeat_func('In LDATA_info_read_blocking')
            time.sleep(sleep_msecs / 1000)


    #####################################################################
    # info_write()
    #
    # Writes latest info to file.
    #
    # Writes to a tmp file first, then moves the tmp file to
    # the final file name when done.
    #
    # Inputs:
    #
    #   source_str:
    #
    #     this is the data directory.
    # 
    #   file_ext: file extension if applicable, otherwise set to ''
    #
    #   user_info: set user information if applicable, otherwise ''
    #
    # Side effect:
    #   Fills out the file path in the object.
    #
    # Returns:
    #   On success, returns 0, on failure returns -1.
    #

    def info_write(self, source_str, latest_time, file_ext, \
                   user_info_1, user_info_2):
        self.file_path = source_str + "/_latest_data_info"
        tmp_path = source_str + "/_latest_data_info.tmp"

        # Fill out the instance information

        self.info.latest_time = latest_time
        self.ltime.set_utime(latest_time)
        self.info.file_ext = file_ext
        self.info.user_info_1 = user_info_1
        self.info.user_info_2 = user_info_2

        # Write the info

        stream = open(tmp_path, "w")
        self.info_print(stream)
        stream.close()

        # Rename the output file

        os.rename(tmp_path, self.file_path)


    #####################################################################
    # data_path()
    #
    # Returns path of latest file using std RAP naming convention,
    # relative to top_dir.
    #

    def data_path(self, top_dir):
        self.file_path = '%s/%.4d%.2d%.2d/%.2d%.2d%.2d.%s' % \
                      (top_dir, \
                       self.ltime.year, self.ltime.month, self.ltime.day, \
                       self.ltime.hour, self.ltime.minute, self.ltime.second, \
                       self.info.file_ext)

        return self.file_path


    #####################################################################
    # data_subdir()
    #
    # Returns subdirectory of latest file using std RAP naming convention.
    #

    def data_subdir(self):
        data_subdir = '%.4d%.2d%.2d' % \
                       (self.ltime.year, self.ltime.month, self.ltime.day)

        return data_subdir


    #####################################################################
    # data_filename()
    #
    # Returns filename of latest file using std RAP naming convention.
    #

    def data_filename(self):
        filename = '%.2d%.2d%.2d.%s' % \
                   (self.ltime.hour, self.ltime.minute, self.ltime.second, \
                    self.info.file_ext)

        return filename


    #####################################################################
    # data_filename_ext()
    #
    # Returns filename of latest file using std RAP naming convention and
    # the given extension.
    #

    def data_filename_ext(self, file_ext):
        filename = '%.2d%.2d%.2d.%s' % \
                   (self.ltime.hour, self.ltime.minute, self.ltime.second, \
                    file_ext)

        return filename


    #####################################################################
    # data_gentime()
    #
    # Returns generation time in a tuple.
    #

    def data_gentime(self):
       gentime  = (self.ltime.year, self.ltime.month, self.ltime.day, \
		   self.ltime.hour, self.ltime.minute, self.ltime.second)

       return gentime


    #####################################################################
    # read_info_file()
    #
    # Open, read and close info file.
    #
    # returns 0 on success, -1 on failure

    def read_info_file(self, file_path):
        # Open the file

        try:
            info_file = open(file_path, 'r')
        except IOError:
            print "ERROR - ", self.prog_name, ":LDATA_info_read"
            print "Could not open latest data info file"
            return -1

        # Perform read

        if self.do_read(info_file):
           info_file.close()
           return -1

        # Debug print

        if self.debug:
            self.info_print(sys.stderr)

        # Close the file

        info_file.close()

        return 0


    #####################################################################
    # "Private" Ldata methods
    #####################################################################

    #####################################################################
    # do_read()
    #
    # reads info from open file descriptor
    #
    # returns 0 on success, -1 on failure
    #

    def do_read(self, input_file):
        # Latest time

        ltime_string = input_file.readline()
        if ltime_string == '':
            return -1
        ltime_string = ltime_string[:string.index(ltime_string, ' ')]
        #print "ltime_string = ", ltime_string
        ltime = string.atoi(ltime_string)
        self.info.latest_time = ltime
        self.ltime.set_utime(ltime)

        # file_ext, user_info

        file_ext = input_file.readline()
        if file_ext == '':
            return -1
        self.info.file_ext = string.rstrip(file_ext)

        user_info_1 = input_file.readline()
        if user_info_1 == '':
            return -1
        self.info.user_info_1 = string.rstrip(user_info_1)

        user_info_2 = input_file.readline()
        if user_info_2 == '':
            return -1
        self.info.user_info_2 = string.rstrip(user_info_2)

        # number of forecasts

        n_fcasts_string = input_file.readline()
        if n_fcasts_string == '':
            return -1
        n_fcasts_string = string.rstrip(n_fcasts_string)
        #print "n_fcasts_string = ", n_fcasts_string
        self.info.n_fcasts = string.atoi(n_fcasts_string)

        return 0


#########################################################################
# LdataInfo class definition
#########################################################################

class LdataInfo:
    def __init__(self):
        self.latest_time = 0
        self.n_fcasts = 0
        self.file_ext = ""
        self.user_info_1 = "none"
        self.user_info_2 = "none"

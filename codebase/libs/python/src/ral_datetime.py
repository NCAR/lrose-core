# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:28 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# module 'ral_datetime' -- Class for manipulating date/time structures.

#
# Load the needed modules
#

import time


#########################################################################
# DateTime class definition
#########################################################################

class DateTime:
    def __init__(self):
        self.unix_time = 0
        self.update_from_utime()


    def set_utime(self, unix_time):
        self.unix_time = unix_time
        self.update_from_utime()


    def update_from_utime(self):
        time_tuple = time.gmtime(self.unix_time)
        self.year = time_tuple[0]
        self.month = time_tuple[1]
        self.day = time_tuple[2]
        self.hour = time_tuple[3]
        self.minute = time_tuple[4]
        self.second = time_tuple[5]
        self.jday = time_tuple[7]

	def monthday(self, year, day_of_year):
		month = 1
		days_in_month = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]

		#
		# Leap year adjustment
		#

		if (year % 4) == 0:
			if (year % 100) == 0:
				if (year % 400) == 0:
					days_in_month[1] = 29
				else:
					days_in_month[1] = 28
			else:
				days_in_month[1] = 29
					
		while day_of_year > days_in_month[month - 1]:
			day_of_year = day_of_year - days_in_month[month - 1]
			month = month + 1

		return (month, day_of_year)

#!/usr/bin/env python
'''
A simple script to show how ConfigMaster works.
'''
from ConfigMaster import ConfigMaster



def main():
    p = ConfigMaster()
    p.setDefaultParams(defaultParams)
    p.init(__doc__)
    
    print "Using these parameters"
    p.printParams()

    if p.opt["debug"]:
        print "\nDEBUG: Using forecast hour: {}".format(p.opt["forecastHour"])



defaultParams = """
import os
import datetime
#####################################
## GENERAL CONFIGURATION
#####################################
 
## debug ##
# Flag to output debugging information
debug = False

# Forecast hour
if datetime.datetime.now().hour % 2 == 0:
  forecastHour = 4
else:
  forecastHour = 3

# Email Address
emailAddy = "prestop@ucar.edu"

dataDir = os.path.join(os.environ["HOME"],"data")

logFile = os.path.join(dataDir,"logs",datetime.datetime.now().strftime("%Y%m%d") + ".log")
 
"""


if __name__ == "__main__":
    main()

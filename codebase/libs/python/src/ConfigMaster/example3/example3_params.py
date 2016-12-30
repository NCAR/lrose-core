#!/usr/bin/env python

import os
import datetime
#####################################
## GENERAL CONFIGURATION
#####################################
 
## debug ##
# Flag to output debugging information
debug = True

# Forecast hour
#forecastHour = 5

# Email Address
emailAddy = "prestop@ucar.edu"

dataDir = os.path.join(os.environ["HOME"],"data")

logFile = os.path.join(dataDir,"logs",datetime.datetime.now().strftime("%Y%m%d") + ".log")
 


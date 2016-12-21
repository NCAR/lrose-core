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
#####################################
## GENERAL CONFIGURATION
#####################################
 
## debug ##
# Flag to output debugging information
debug = False

# Forecast hour
forecastHour = 3

# Email Address
emailAddy = "prestop@ucar.edu"
 
"""


if __name__ == "__main__":
    main()

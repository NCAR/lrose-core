#!/usr/bin/env python
 
from ConfigMaster import ConfigMaster
 
 
class Params(ConfigMaster):
  defaultParams = """
#!/usr/bin/env python
 
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

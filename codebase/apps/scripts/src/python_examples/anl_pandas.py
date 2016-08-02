#! /usr/bin/env python
from datetime import datetime

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# read in the data from ANL met tower, Jan 2015
# available at: http://www.atmos.anl.gov/ANLMET/numeric/2015/jan15met.data
# format is descriped at http://www.atmos.anl.gov/ANLMET/numeric/Readme.data

# read in the data from ANL met tower, Jan 2015
# available at: http://www.atmos.anl.gov/ANLMET/numeric/2015/jan15met.data
# format is descriped at http://www.atmos.anl.gov/ANLMET/numeric/Readme.data
rec_array = np.recfromtxt('anl_jan15met.data.txt')
temp_10m = rec_array.f12[:]   # 13th column is 10 meter temp
temp_10m = np.ma.masked_equal(temp_10m, 99999.0)  # mask out invalid data

humidity_10m = rec_array.f14[:]  # 15th column is relative humidity
# mask out invalid data (below 1% or above 100%)
humidity_10m = np.ma.masked_outside(humidity_10m, 1.0, 100.)

times = [[]] * len(rec_array)
for i in range(len(rec_array)):
    year = rec_array.f2[i] + 2000
    month = rec_array.f1[i]
    day = rec_array.f0[i]
    hour, minute = divmod(rec_array.f3[i], 100)
    if hour == 24:  # subtract one minute, plot is slighly off..
        hour = 23
        minute = 59
    times[i] = datetime(year, month, day, hour, minute)

fig = plt.figure()

ax1 = fig.add_subplot(111)
temp_line, = ax1.plot(times, temp_10m, 'b-')
ax1.set_ylim(-30, 30)
ax1.set_ylabel('Temperature (deg C)')
ax1.set_xlabel('Date')

ax2 = ax1.twinx()
humidity_line, = ax2.plot(times, humidity_10m, 'r-')
ax2.set_ylim(0, 100)
ax2.set_ylabel('Relative Humidity (%)')

plt.legend((temp_line, humidity_line), ('Temp', 'Humidity'), loc='lower right')
fig.autofmt_xdate()  # autoformat the x axis as dates

#####  PANDAS VERSION  #####
# Using the format information, set the column data names
colnames = ['day', 'month', 'year', 'hhmm', 'pasquill_stab_class',
            'wind_60m_dir', 'wind_60m_spd', 'wind_60m_dir_std',
            'temp_60m', 'wind_10m_dir', 'wind_10m_spd', 'wind_10m_std',
            'temp_10m', 'dewpoint', 'rh', 'temp_diff_per_100m',
            'precip', 'solar_rad', 'net_rad', 'pressure', 'vapor_pressure',
            'soil_temp_10cm', 'soil_temp_100cm','soil_temp_10ft']

# Read table data with Pandas (saves to a "dataframe")
df = pd.read_table('anl_jan15met.data.txt', names=colnames, delim_whitespace=True)
# Remove the rows where hhmm = 2400, this is a daily average
df = df[df.hhmm != 2400]

# Create datetime arrays and replace the datafram index
times =pd.to_datetime(df.year.values*100000000 + df.month.values*1000000 +
                      df.day.values*10000 + df.hhmm.values,
                      format='%y%m%d%H%M')
df.index = times

# Replace the missing data values
df.replace('99999.0', np.nan, inplace=True)
df.loc[df['rh'] < 1., 'rh'] = np.nan
df.loc[df['rh'] > 100., 'rh'] = np.nan

fig, ax1 = plt.subplots(1, 1)
ax2 = ax1.twinx()

df.plot(x=df.index, y=['temp_10m'], ylim=(-30,30), ax=ax1)
df.plot(x=df.index, y=['rh'], ylim=(0,100), color='r', ax=ax2)

ax1.legend(loc='lower right')
ax1.set_ylabel('Temperature (deg C)')
ax1.set_xlabel('Date')
ax2.set_ylabel('Relative Humidity (%)')
#####  PANDAS VERSION  #####

#######  LINEAR REGRESSION  ########

# Example from http://stackoverflow.com/questions/19379295/linear-regression-with-pandas-dataframe
# Also see this entry for more detailed info: http://connor-johnson.com/2014/02/18/linear-regression-with-python/

from scipy.stats import linregress
def fit_line1(x, y):
    """Return slope, intercept of best fit line."""
    # Remove entries where either x or y is NaN.
    clean_data = pd.concat([x, y], 1).dropna(0) # row-wise
    (_, x), (_, y) = clean_data.iteritems()
#    x.dropna(0)
#    y.dropna(0)
    slope, intercept, r, p, stderr = linregress(x, y)
    return slope, intercept # could also return stderr

import statsmodels.api as sm
def fit_line2(x, y):
    """Return slope, intercept of best fit line."""
    X = sm.add_constant(x)
    model = sm.OLS(y, X, missing='drop') # ignores entires where x or y is NaN
    fit = model.fit()
    return fit.params[1], fit.params[0] # could also return stderr in each via fit.bse

m1, b1 = fit_line1(df.temp_10m, df.temp_60m)
m2, b2 = fit_line2(df.temp_10m, df.temp_10m)

fig, axf = plt.subplots(1, 1)
df.plot.scatter('temp_10m', 'temp_60m', marker='.', ax=axf)
axf.set_xlabel=('10m Temperature')
axf.set_ylabel=('60m Temperature')
axf.plot(df.temp_10m.values, (m1 * df.temp_10m.values + b1), 'r', lw=3)
axf.plot(df.temp_10m.values, m2 * df.temp_10m.values + b2, 'g:', lw=3)

plt.show()   # uncomment to show the plot interactively
#plt.savefig('temp_and_humidity.png')
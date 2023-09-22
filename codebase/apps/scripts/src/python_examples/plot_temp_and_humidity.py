#! /usr/bin/env python
from datetime import datetime

import numpy as np
import matplotlib.pyplot as plt

# read in the data from ANL met tower, Jan 2015
# available at: http://www.atmos.anl.gov/ANLMET/numeric/2015/jan15met.data
# format is descriped at http://www.atmos.anl.gov/ANLMET/numeric/Readme.data
rec_array = np.recfromtxt('jan15met.data.txt')
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

plt.show()   # uncomment to show the plot interactively
# plt.savefig('temp_and_humidity.png')

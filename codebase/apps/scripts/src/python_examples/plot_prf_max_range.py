#! /usr/bin/env python

import numpy as np
import matplotlib.pyplot as plt

# data example taken from scipy.optimize.curve fit docstring example

def maxRangeKm(prf):
    return (3.0e8 / (prf * 2.0)) / 1000.0

xdata = np.linspace(200, 1500, 13 * 4)
ydata = maxRangeKm(xdata)

fig = plt.figure(1, (10.0, 4.5))
ax = fig.add_subplot(111)
ax.plot(xdata, ydata, 'b-')

ax.legend()

ax.set_xlabel("PRF(Hz)")
ax.set_ylabel("Max Range (km)")

ax.set_title('Max Range from PRF')
ax.grid(color='gray', linestyle='-', linewidth=1)



plt.show()  # uncomment to show the plot interactively
# plt.savefig('fitting_example.png')

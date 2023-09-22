#! /usr/bin/env python

import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit


# data example taken from scipy.optimize.curve fit docstring example
def func(x, a, b, c):
    return a * np.exp(-b * x) + c

xdata = np.linspace(0, 4, 50)
y = func(xdata, 2.5, 1.3, 0.5)
ydata = y + 0.2 * np.random.normal(size=len(xdata))

popt, pcov = curve_fit(func, xdata, ydata)
yfit = func(xdata, *popt)

fig = plt.figure()
ax = fig.add_subplot(111)
ax.plot(xdata, ydata, 'k.', label='Data')
ax.plot(xdata, y, 'b-', label='True')
ax.plot(xdata, yfit, 'r-', label='Fit')

ax.legend()

plt.show()  # uncomment to show the plot interactively
# plt.savefig('fitting_example.png')

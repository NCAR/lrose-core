#!/usr/bin/env python
import numpy as np
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt

zdr = np.genfromtxt("zdr.txt")

# the histogram of the data
n, bins, patches = plt.hist(zdr, 50, normed=1, facecolor='green', alpha=0.75)

# add a 'best fit' line
#y = mlab.normpdf( bins, mu, sigma)
#l = plt.plot(bins, y, 'r--', linewidth=1)

plt.xlabel('ZDR')
plt.ylabel('Probability')
plt.title('Histogram of ZDR in ice')
#plt.axis([40, 160, 0, 0.03])
plt.grid(True)

plt.show()

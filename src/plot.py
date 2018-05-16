#!/usr/bin/env python3

import numpy as np
from matplotlib import pyplot as plt
import sys

filename = sys.argv[1]

vals = np.loadtxt(filename, dtype=int)
svals = sorted(vals)

#c,b, p = plt.hist(svals, bins='fd', normed=False, range=(0,50000) )
c,b, p = plt.hist(svals, bins='fd', normed=False )

plt.show()



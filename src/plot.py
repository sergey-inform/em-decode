#!/usr/bin/env python3

import numpy as np
from matplotlib import pyplot as plt
import sys

if len(sys.argv) > 1:
    filename = sys.argv[1]
else:
    filename = sys.stdin

vals = np.loadtxt(filename, dtype=int)
svals = sorted(vals)[:-1]

#c,b, p = plt.hist(svals, bins='fd', normed=False, range=(0,50000) )
c,b, p = plt.hist(svals, bins='fd', normed=False )

plt.show()



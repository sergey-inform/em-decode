#!/usr/bin/env python
"""
Plot data on timeline.

Data format:
<ts_utc> <col1> <col2> ... <coln>
"""

import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import datetime as dt

columns = ['ts', 'events', 'corrupt']
formats = ['u4', 'f4', 'f4'] 
file_ = sys.stdin

ts, events, corrupt =  np.loadtxt(file_, dtype={'names': columns, 'formats': formats}, unpack = True)

is_sorted = lambda a: np.all(a[:-1] <= a[1:])

if not is_sorted(ts):
    print("ts is not sorted")
    exit(1)

dates=[dt.datetime.fromtimestamp(_) for _ in ts]

ax1 = plt.subplot(2,1,1)
plt.plot_date(dates, corrupt/events, 'r.')
plt.title('Events timeline')
plt.ylabel('corrupt/events')

ax2 = plt.subplot(2,1,2, sharex=ax1)
plt.plot_date(dates, events, '-', xdate=True)
plt.xlabel('Time')
plt.ylabel('Total events')

plt.xticks( rotation=25 )
ax=plt.gca()
xfmt = mdates.DateFormatter('%Y-%m-%d %H:%M')
ax.xaxis.set_major_formatter(xfmt)

ax1.xaxis.set_visible(False)

plt.show()


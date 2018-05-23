#!/usr/bin/env python3

import numpy as np
import sys
import argparse

parser = argparse.ArgumentParser(usage='%(prog)s [-h] file file [file ...]', description=__doc__)
# Two or more arguments
parser.add_argument('file1', type=argparse.FileType('r'), nargs=1,  
        metavar='file', help=argparse.SUPPRESS)
parser.add_argument('file', type=argparse.FileType('r'), nargs='+',
        metavar='file')
parser.add_argument('--text', action='store_true',
        help='input files are text instead of binary')

args = parser.parse_args()
files=args.file1 + args.file


dtype = np.dtype( [('dt', '<u2'),('other', '<u2')])

data = []

if args.text:  # text input
    for f in files:
        data.append(np.loadtxt(f, dtype=np.dtype([('dt', '<u4')]), usecols=(0)))

else:  # binary input
    for f in files:
        data.append(np.fromfile(f, dtype=dtype, count=-1))


# process data
dt = [d['dt'] for d in data]

def sync_dt(*arrays, jitter=3):
    '''


    '''
    import numpy as np

    chunk = 64  # a number of continuous sync events
    bysize = sorted(arrays, key=lambda x: len(x), reverse=False)
    
    arr1 = bysize.pop()
    arr2 = bysize.pop()

    diff = (arr1[0:64] - arr2[:64]).astype('int16')
    ab = np.absolute(diff)
    ok = (ab < jitter).sum()


    print(arr1[:64], arr2[:64], diff,ab, ok)
    print(type(ok))

    return [1,2,3]

g = sync_dt(*dt)


cnt=0
try:
    for a in g:
        cnt +=1
        print(a)
    
    print(cnt)

except IndexError:
    print("unsync")
    sys.exit(1)


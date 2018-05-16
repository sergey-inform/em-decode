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

def sync_dt(*arrays, reciprocal_accuracy=1024):
    """
        A simple timestamp synchronization.

        Generator, which consumes iterable (list,ndarray) with numbers
        (timestamps or intervals between timestamps) and produces a list
        of matching numbers, Nones for not found matches.

        Method is robust and could find matches with given accuracy.
          
        In rare conditions (mix of large and small numbers) it may fail 
        to find any matches.
        
        PS: The complete triggerless timestamp synchronization requires 
        infinitly accurate clock or more complex mathematics. 
    """

    sizes = [len(x) for x in arrays]
   
    offset = [0] * len(arrays)
    accum  = [0] * len(arrays)

    enum = range(0, len(arrays))  # 0,1,2,3...

    for idx in range(0, min(sizes)):
        try:
            accum[:] = [ arrays[i][idx + offset[i]] for i in enum]
        except IndexError:
            break
        
        while True:  # catch-up cycle
            min_ = min(accum)
            max_ = min_ + min_ // reciprocal_accuracy
  
            fit_mask = [ x < max_ for x in accum]  # 

            if all(fit_mask):
                break
            else:  # unsync element found, try to catch up
                yield [accum[i] if fit_mask[i] else None for i in enum], offset

                for i,x in enumerate(fit_mask):
                    if x == True:
                        offset[i] += 1
                        accum[i] += arrays[i][idx + offset[i]]
            
        yield idx, accum,  offset
        
    raise StopIteration()


g = sync_dt(*dt)

for a in g:
    print(a)

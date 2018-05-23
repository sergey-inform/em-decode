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

def get_sync_dt(*arrays, reciprocal_accuracy=4*1024, jitter=2):
    """
        A simple timestamp synchronization.

        Generator, which consumes iterable (list,ndarray) with numbers
        (intervals between timestamps) and produces a list
        of matching numbers, Nones for not found matches.

        Method is robust and could find matches with given accuracy.
          
        In rare conditions (mix of large and small numbers) it may fail 
        to find any matches.
        
        PS: The complete triggerless timestamp synchronization requires 
        infinitly accurate clock or more complex mathematics. 

        Return tuple: 
            index, [offsets]
    """

    sizes = [len(x) for x in arrays]
  
    # TODO: use numpy and vector operations
     
    offset = [0] * len(arrays)
    accum  = [0] * len(arrays)  # accumulator, to handle missing or extra events
    idx = 0  # index for sync events

    enum = range(0, len(arrays))  # 0,1,2,3...

    while True:
        try:
            accum[:] = [ arrays[i][idx + offset[i]] for i in enum ]
        except IndexError:
            # check all data has been processed
            counts = [idx + offset[i] for i in enum]

            if counts == sizes:
                raise StopIteration()  # normal case
            else:
                raise IndexError("no more data")  # failed to sync
        
        while True:  # catch-up cycle
            min_ = min(accum)
            max_ = min_ + min_ // reciprocal_accuracy + jitter
  
            fit_mask = [ x <= max_ for x in accum]  # 
            
            if all(fit_mask):  # sync
                yield idx, accum, offset
                #yield idx, offset
                idx += 1
                break

            elif idx == 0:  # special case, skip until first sync
                offset[:] = [offset[i]+1 for i in enum]
                break

            else:  # unsync, try to catch up
                yield [accum[i] if fit_mask[i] else None for i in enum] # unsync

                for i,x in enumerate(fit_mask):
                    if x == True:
                        offset[i] += 1
                        try:
                            accum[i] += arrays[i][idx + offset[i]]
                        except IndexError as e:
                            e.args = ["failed to sync"]
                            raise e
            

g = get_sync_dt(*dt)

cnt=0
try:
    for a in g:
        cnt +=1
        print(a)
    
    print(cnt)

except IndexError:
    print("unsync")
    sys.exit(1)


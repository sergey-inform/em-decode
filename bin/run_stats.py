#!/usr/bin/env python
'''
Print event statistics for all data files in directory.
Expected filename format: "*_<unixtime>.<crate_id>.dat"

Output:
unixtime  total_events broken_events
'''

import sys
import glob
import signal
import subprocess32 as sp
import multiprocessing as mp
from time import sleep

PROG='./em-parse'
CRATE=int(sys.argv[1])
RUN_DIR=sys.argv[2]
TIMEOUT=3600  # seconds 

def work(filename):
    '''Defines the work unit on an input file'''
    signal.signal(signal.SIGINT, signal.SIG_IGN)  #ignore Ctrl-C

    out = sp.check_output([PROG, '{}'.format(filename), '-o', '/dev/null'], stderr=sp.STDOUT)

    
    #get timestamp from filename
    ts = filename.rsplit('.',2)[0].rsplit('-')[-1]

    #get output values
    cnt, corrupted = 0, 0
    for line in out.splitlines():
        if line.startswith('--'):
            break
        
        v, k = line.split()

        if k == "CNT_EM_EVENT_CORRUPTED":
            corrupted = int(v)
        elif k == "CNT_EM_EVENT":
            cnt = int(v)

    res = '{}\t{}\t{}'.format(ts, cnt, corrupted)

    return res

if __name__ == '__main__':
    #Specify files to be worked 
    # with typical shell syntax and glob module
    file_path = '{}/*/*.{:d}.dat'.format(RUN_DIR, CRATE)
    tasks = sorted(glob.glob(file_path))

    #Set up the parallel task pool to use all available processors
    count = mp.cpu_count()
    pool = mp.Pool(processes=count)

    #Run the jobs
    try:
        res = pool.imap(work, tasks)  # for each task run a work process

        for r in res:  # Without the timeout this blocking call ignores all signals. 
                           # BUG: https://bugs.python.org/issue8296
            print(r)
        
        pool.close()  # Every task is complete

    except KeyboardInterrupt:
        pool.terminate()

#    finally:
#        pool.join()


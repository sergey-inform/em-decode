#!/usr/bin/env python3
'''
Print event statistics for data files.

'''

import sys, os
import glob
import signal
import subprocess as sp
import multiprocessing as mp
from time import sleep
import logging
import argparse


PROG='./em-counts'
TIMEOUT=3600  # seconds 


def work(filename):
    '''Defines the work unit on an input file'''
    signal.signal(signal.SIGINT, signal.SIG_IGN)  #ignore Ctrl-C

    out = sp.check_output([PROG, '{}'.format(filename)])

    #get timestamp from filename
    ts = os.path.basename(filename)[:10]  # unixtime will be 10-byte long in next 268 years

    #get output values
    cnt, corrupted = 0, 0
    for line in out.splitlines():
        if line.startswith(b'--'):
            break
        try:        
            k, v = line.split()

            if k == b"CNT_EM_EVENT_CORRUPTED":
                corrupted = int(v)
            elif k == b"CNT_EM_EVENT":
                cnt = int(v)
            else:
                sys.stderr.write("{}\n".format(k))

        except ValueError:
            raise

    res = '{}\t{}\t{}\t{}'.format(ts, cnt, corrupted, filename)

    return res


if __name__ == '__main__':
    
    logging.basicConfig(level=logging.DEBUG, format='%(message)s')

    
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("fileglob", help="quoted file mask with * and **")
    args = parser.parse_args()


    #Specify files to be worked 
    # with typical shell syntax and glob module

    files = glob.iglob(args.fileglob, recursive=True)
    tasks = (_ for _ in files if os.path.isfile(_))

    #Set up the parallel task pool to use all available processors
    count = mp.cpu_count()
    logging.debug("Process count: {:d}".format(count))
    pool = mp.Pool(processes=count)

    #Run the jobs
    try:
        chunk = 2  # chunk size, default (1) is suboptimal
        res = pool.imap(work, tasks, chunk)  # for each task run a work process

        for r in res:  # Without the timeout this blocking call ignores all signals. 
                           # BUG: https://bugs.python.org/issue8296
            print(r)
            sys.stdout.flush()
        
        pool.close()  # Every task is complete

    except KeyboardInterrupt:
        pool.terminate()

#    finally:
#        pool.join()


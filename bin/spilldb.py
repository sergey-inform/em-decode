#!/usr/bin/env python3


def shardname(timestamp, format="%y%m%d-%H"):
    """
    Return string which is used as directory name
    to shard files on per-hour basis.
    
    Example:
        shardname("1521332622") is '180318-03'
    """
    from datetime import datetime

    return datetime.fromtimestamp(
                int(timestamp)
            ).strftime(format)
            

def allspills(rootdir, ext=".dat.gz"):
    """

    """
    from glob import iglob
    from os.path import basename

    spills = set()
    globpath = str(rootdir) + "/**/*" + str(ext)
    
    for filename in iglob(globpath, recursive=True):
        fn = basename(filename)[:-len(ext)] 
                    # rootdir/sub/1521331424.6.dat.gz -> '1521331424.6' 
        ts = int(fn[:10])  # '1521331424.6' -> 1521331424
        spills.add(ts)

    return spills

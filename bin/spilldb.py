#!/usr/bin/env python3

def name2ts(filename, ext=".dat.gz"):
    """
    '1521331424.6.dat.gz' -> 1521331424
    Return None if file doesn't match extension or can not be converted.
    """
    if str(filename).endswith(ext):
        try:
            return int((filename[:-len(ext)])[:10])
        except ValueError:
            return None
    else:
        return None


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

    spills = []
    globpath = str(rootdir) + "/**/*" + str(ext)
    
    for filename in iglob(globpath, recursive=True):
        fn = basename(filename)[:-len(ext)] 
                    # rootdir/sub/1521331424.6.dat.gz -> '1521331424.6' 
        ts = int(fn[:10])  # '1521331424.6' -> 1521331424
        spills.append(ts)

    #TODO: -> os.walk

    return set(spills)
    
def walkspills(rootdir, ext=".dat.gz"):
    """

    """
    from os import walk

    spills = []

    def handlerr(err):
        raise err


    #TODO: support globs
    for root, dirs, files in walk(rootdir,
                topdown=True, followlinks=True, onerror=handlerr):
        dirs.sort()
        timestamps = set([name2ts(_, ext) for _ in files])
        try:
            timestamps.remove(None)
        except KeyError:
            pass

        tslist = sorted(timestamps)
        if tslist:
            spills.extend(tslist)
    
    return spills


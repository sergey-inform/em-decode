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
            

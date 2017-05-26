#!/usr/bin/python
import sys

def hex2str(srchex):
    s=""
    if len(srchex)%2 > 0:
        return s

    for i in range(len(srchex/2)):
        intval = int(srchex[i*2,i*2+1])
        s+=chr(intval)

    return s

if __name__ == "__main__":
    srchex= sys.argv[1]
    
    print hex2str(srchex)



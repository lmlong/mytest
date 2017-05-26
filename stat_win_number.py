#!/usr/bin/python
# vim:fileencoding=utf8 

import sys
import logging
#import test_tools


def main_proc(input_file):
    stat_result={}
    tail_num = [99,88,77,66,55,44,33,22,11,999,888,777,666,555,444,333,222,111,123,234,345,456,567,678,789]
    exact_num=[520,1314,2017]

    line=""
    try:
        input_f = open(input_file)
        total=0
        #450 5 46 51 65 59 9 90 36 41 48
        for line in input_f:
            line=line.strip(" \n")
            tks=line.split(" ")
            for i in range(1, len(tks)):
                number=int(tks[i])
                if number >= 100:
                    tail = number % 1000
                    if tail in tail_num:
                        if tail not in stat_result:
                            stat_result[tail]=0
                        stat_result[tail]=stat_result[tail]+1
                elif  number >= 10:
                    tail = number % 100
                    if tail in tail_num:
                        if tail not in stat_result:
                            stat_result[tail]=0
                        stat_result[tail]=stat_result[tail]+1

                    
                if number in exact_num:
                        if number not in stat_result:
                            stat_result[number]=0
                        stat_result[number]=stat_result[number]+1

            total=total+1
            #break
    except Exception as e:
        print e, line

    #print result
    for num in sorted(stat_result):
        print "%d, %d, %d, %.5f"%(num, stat_result[num], total,  stat_result[num]*1.0/total)
                    

        

if __name__ == "__main__":
    '''
    logfile = sys.argv[0].rstrip(".py")+".log"
    logging.basicConfig(filename=logfile
                ,format="[%(asctime)s] %(message)s"
                ,datefmt='%Y-%m-%d %H:%M:%S'
                ,level=logging.INFO)
    '''
    if len(sys.argv) != 2:
        print "%s create_file"%(sys.argv[0])
        sys.exit(1)

    input_file = sys.argv[1]
    main_proc(input_file) 

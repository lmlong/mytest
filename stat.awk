#!/bin/awk -f
BEGIN {
    max=0;
    maxi=0;
}
{ 
    for(i=2;i<=NF;i++)
    { 
        if ($i>max) 
        {
            maxi=i;
            max=$i;
        } 
        if (i==NF)
        {
            stat[maxi]+=1;
            max=0;
            maxi=0;
        }
    }
}
END { 
    for (x in stat) 
    {
        print x, stat[x];
    }
}


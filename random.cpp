#include <sys/time.h>
#include <time.h>
#include <unistd.h>


#include <algorithm>

#include <boost/random.hpp>

#include "random.h"

uint32_t random_seed()
{
    struct timeval tv;  
    gettimeofday(&tv, NULL);  
    const uint32_t kPrime1 = 61631;
    const uint32_t kPrime2 = 64997; 
    const uint32_t kPrime3 = 111857;  
    return kPrime1 * static_cast<uint32_t>(getpid())
       + kPrime2 * static_cast<uint32_t>(tv.tv_sec)  
       + kPrime3 * static_cast<uint32_t>(tv.tv_usec);

}

int gen_uniform_int(int min, int max)
{
    uint32_t seed =  random_seed();
    boost::mt19937 rng(seed);
    boost::uniform_int<> intdist(min, max);
    boost::variate_generator< boost::mt19937&, boost::uniform_int <> > genint(rng ,intdist);
    return genint();
}

void shuffle_vecint(std::vector<int>& vecnums)
{
    if (vecnums.size() == 0)
        return;

    int seed = random_seed();
    boost::mt19937 rng(seed);
    for (int i=0;i<vecnums.size()-1;i++)
    {
        boost::uniform_int<> intdist(i, vecnums.size()-1);
        boost::variate_generator< boost::mt19937&, boost::uniform_int <> > genint(rng ,intdist);
        int idx = genint();
        if (idx != i)
        {
            std::swap(vecnums[i], vecnums[idx]);
        }
    }
}

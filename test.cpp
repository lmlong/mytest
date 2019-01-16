#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>
//open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//time
#include <sys/time.h>
#include <time.h>

//boost random
#include <boost/random.hpp>
#include "random.h"


#include "bitop.h"
#include <errno.h>

using namespace std;


int test_bitshift(int argc, char* argv[])
{
    printf("1<<0 == %#x\n", 1<<0);
    printf("1<<1 == %#x\n", 1<<1);
    return 0;
}

class A
{
public:
    A(int a1)
    {
        a = a1;
        std::cout<<"Con A with para:"<<a1<<","<<this<<endl;
    }
    A()
    {
        a = 0;
        std::cout<<"Con A with null"<<","<<this<<endl;
    }
    A(const A& that)
    {
        a = that.a;
        std::cout<<"Con A with ref:"<<that.a<<",this:"<<this<<",that:"<<&that<<endl;
    }
    ~A()
    {
        std::cout<<"deCon A:"<<a<<","<<this<<endl;
    }
    A& operator=(A &that)
    {
        std::cout << "=," <<"this:"<<this<<",that:"<<&that<<endl;
        a = that.a;
    }

    int a;
};

int funcref(A &a)
{
    cout<<"in funcref: &a:"<<&a<<endl;
}

int test_ref(int argc , char *argv[])
{
    A  a1(100);
    cout<<"in test_ref: a1:"<<&a1<<endl;
    funcref(a1);
}

int test_map(int argc, char* argv[])
{
    map<int, A> mapa;
    cout<<"check a"<<endl;
    A a1(10);
    /*
    mapa.insert(pair<int, A>(1, a1));
    */
    A &a2=a1;
    cout <<"&a1:"<<&a1<<",&a2:"<<&a2<<endl;
    vector<A> vec_a;
    vec_a.push_back(a1);
    a1.a=20;
    cout<<"after change:"<<vec_a[0].a<<endl;
    return 0;

}

map<int, A> mappa;
int test_map_deconstruct(int argc, char *argv[])
{
    A a(1);
    mappa[1]= a;
    return 0;
}

#include <openssl/md5.h>
unsigned get_hash_key(const char *d, unsigned l)
{
    static unsigned char buf[16];
    MD5((const unsigned char *)d, l, buf);
    unsigned u;
    memcpy(&u, buf + 12, 4);
    return u;
}

int hex2str(const string &hex, string &result)
{
    if (hex.size() % 2 != 0) return -1;
    result = "";
    char c;
    unsigned u;
    for (unsigned i = 0; i < hex.size() / 2; ++i)
    {
        sscanf(hex.c_str() + i * 2, "%2x", &u);
        c = u;
        result.append(&c, 1);
    }
    return 0;
}

std::string str2hex(const char* key, unsigned len)
{
    unsigned bucketnum = get_hash_key(key, len)%10000000;
    char hex[1024]={0};
    char buffer[8]={0};
    for(unsigned n=0; n < len ; ++n) {
        sprintf(buffer, "%02x", (unsigned char)key[n]);
        hex[2*n]=buffer[0];
        hex[2*n+1]=buffer[1];
    }
    hex[len*2]='\0';
    
    char buf[1024] = {0};
    snprintf(buf, 1024, "%u_%s", bucketnum, hex);
    return std::string(buf);
}

int test_hex2str(int argc, char *argv[])
{
    string hex = argv[1];
    string result; 
    hex2str(hex, result);
    cout<<result<<endl;

    return 0;
}
int test_str2hex(int argc, char *argv[])
{
    string str = argv[1];
    string result = str2hex(str.c_str(), str.size()); 
    cout<<result<<endl;

    return 0;
}

int test_strerror(int argc, char *argv[])
{
    int errorno = atoi(argv[1]);
    string result = strerror(errorno); 
    cout<<result<<endl;
}

int test_rand(int argc, char *argv[])
{
    printf("rand_max:%d. ", RAND_MAX);
    for (int i=0;i<100;i++)
    {
        int r = rand();
        printf("%10d ", r);
    }
    printf("\n");
    return 0;
}


void get_start_num(int total, int start, int num, int &real_start, int &real_num)
{// [start, end]    
    if (start < 0)  // 转成正的start
    {
        int end = total + start;
        start = end - num + 1;
        if (start < 0 && end < 0)  // 越界了
        {
            real_num = 0;
            real_start = 0;
        }
        else
        {
            if (start < 0) start = 0;
            if (end < 0) end = 0;

            real_start = start;
            real_num = end - start + 1;
        }
    }
    else
    {
        if (start > total - 1)   // 越界了
        {
            real_num = 0;
            real_start = 0;
        }
        else
        {
            int end = start + num - 1;
            if (end > total - 1) end = total - 1;

            real_start = start;
            real_num = end - start + 1;
        }
    }
}

int test_get_start_num(int argc, char* argv[])
{
    if (argc<3)
    {
        printf("param error\n");
        return -1;
    }
    int total = atoi(argv[1]);
    int start = atoi(argv[2]);
    int num = atoi(argv[3]);
    
    int r_start = -1;
    int r_num = -1;
    
    get_start_num(total, start, num, r_start, r_num);

    printf("total:%d, start:%d, num:%d, r_start:%d, r_num:%d\n",
            total, start, num, r_start, r_num);

    return 0;
}

//double CCreateRedPacketMineSweep::GetRate(int portion, int coin, GoGirlIntList &datalist)
double gen_redpacket(int portion, int coin, vector<int> &vecnums)
{
    int inc = 0; 
//    DEBUG_P(LOG_DEBUG, "CCreateRedPacketMineSweep::GetRate: func begin\n");
    int left = coin - portion;
    int tmpportion = portion;
    int external_val = 1; //每个人预留coin
    while (1)
    {    
        if (portion <= 0)
        {    
            break;
        }    
    
        int result = (left - inc) / (portion);
//      AsnInt *p = _cur_red_packet.coinlist.Append();
        int this_redpacket = 0;
        
        if (result < 1) 
        {    
//            DEBUG_P(LOG_DEBUG, "CCreateRedPacketMineSweep::GetRate: func begin111 coin=%d portion=%d\n", coin, portion);
            if (left - inc < 0) 
            {    
                this_redpacket = 0; 
            }    
            else 
            {    
                int tmpdata = ((left - inc) * 100) / tmpportion;
                int rnd = rand() % 100; 
                if (rnd < tmpdata)
                {    
                    this_redpacket = 1; 
                    inc += 1;   
                }    
                else 
                {    
                    this_redpacket = 0; 
                }    
//              DEBUG_P(LOG_DEBUG, "CCreateRedPacketMineSweep::GetRate: func begin111 coin=%d portion=%d tmpdata=%d rnd=%d *p=%d inc=%d\n", 
//                        coin, portion, tmpdata, rnd, (int)*p, inc);
            }    
        }    
        else 
        {    
 //           DEBUG_P(LOG_DEBUG, "CCreateRedPacketMineSweep::GetRate: func begin222\n");
            int large = ((left - inc) / (portion)) * 2; 
            int mod = 0; 
            if (large > 0) 
            {    
                mod = 1 + rand() % large;            
            }    
            else 
            {    
                mod = 0;    
            }    

            inc += mod;
            this_redpacket = mod;
//            DEBUG_P(LOG_DEBUG, "CCreateRedPacketMineSweep::GetRate: coin=%d inc=%d portion=%d large=%d mod=%d\n", coin, inc, portion, large, mod);
        }
        this_redpacket = this_redpacket + external_val;
        vecnums.push_back(this_redpacket);
        portion--;
    }

    return 1.0;
}

int gen_redpacket_seq_excl(int portion, int coin, int excl_bmp, int factor, vector<int> &vecnums)
{
    // portion min is 1
    // rand range: [0, coin - portion] 
    int r_portion = portion;
    int left_coin = coin;
    int this_portion = 0;
    
    // 分数不能大于金额
    if (portion > coin)
    {
        return 10;
    }
    //红包最大因子不能超过份数
    if ((factor >= portion*100) || (factor < 100))
    {
        return 11;
    }
    //不能排除全部的尾数
    excl_bmp &= 0x3FF;
    if (excl_bmp == 0x3FF )
    {
        return 12;
    }
    int rsv_factor = (factor + 100)/100;

    

    bool lastportion = false;
    for (int i=0; i < portion; i++)
    {
        if (left_coin < r_portion )
        {
            fprintf(stderr, "left_coin <= r_portion, left_coin = %d, r_portion:%d\n", left_coin, r_portion);
            return 20;
        }

        int avg_portion = left_coin / r_portion;
        int max_portion = (factor * avg_portion)/100;
        if (max_portion == 0) max_portion = 1;

        if (i == portion - 1)
        {
            this_portion = left_coin;
            lastportion = true;
        }
        else
        {
            int mod_factor = left_coin - r_portion * rsv_factor;
            if (mod_factor <= 0) mod_factor = 1;
            mod_factor =  (mod_factor < max_portion)?mod_factor:max_portion;
            int r = rand() % mod_factor;
            this_portion = (r==0)?1:r;
        }

        //尾数在排除列表中，则调整之: 随机找一个不被排除的尾数
        int tail = this_portion % 10;
        if (test_bit(excl_bmp, tail))
        {
            int adj_portion = 0;
            int b_digit = rand()%10;
            int aj_tail = b_digit;
            for (int j=b_digit; j<b_digit+10; j++)
            {
                aj_tail = j % 10;
                adj_portion = (this_portion / 10)*10 + aj_tail;
                if (!test_bit(excl_bmp, aj_tail) && (adj_portion > 0))
                {
                    break;
                }
            }
            //最后一份的调整，还必须反向调整，在已经生成的序列里挑一个，
            //满足两个条件: 不是雷 + 总和不变
            //一次调整失败，则继续尝试剩下的尾数
            if (lastportion)
            {
                bool adj_ok = false;
                int  aj_end = aj_tail + 10;
                for(int i=aj_tail;i<aj_end;i++)
                {
                    aj_tail = i % 10;
                    adj_portion = (this_portion / 10)*10 + aj_tail;
                    if (test_bit(excl_bmp, aj_tail) || (adj_portion <= 0))
                    {
                        continue;
                    }
                    int diff = tail - aj_tail;
                    for(vector<int>::iterator it = vecnums.begin();it!=vecnums.end();it++)
                    {
                        int p = *it;
                        p += diff;
                        int ptail = p % 10;
                        if (!test_bit(excl_bmp, ptail) && (p>0) )
                        {
                            *it = p;
                            adj_ok = true;
                            goto adjfi;
                        }
                    }
                }
                if (!adj_ok)
                {
                    fprintf(stderr, "adjust last portion failed!\n");
                    return 30;
                }
            }
adjfi:
            this_portion = adj_portion;
        }

            
            
        r_portion --;
        vecnums.push_back(this_portion);
        left_coin -= this_portion;
    }

    return 0;
}


/*
 *
 * minenum_rate[]:雷数概率 0,1,2, ..., portionnum
 * tailbmp: 尾数位图
 * portion_num: 红包份数
 * coin: 红包金额
 * factor: 最大红包因子 , 取值范围[100, portion_num*100)
 * lucky_num: 幸运数
 */
int gen_redpacket_seq_by_tail_mine(int minenum_rate[], int tailbmp, int lucky_num, int portion_num, int coin, int factor, vector<int> &vecnums)
{

    //tailbmp 至少要制定一个尾数 
    if ((tailbmp | ((1<<portion_num)-1)) == 0)
    {
        fprintf(stdout, "tailbmp is all zero");
        return 1;
    }


    // 分数不能大于金额
    if (portion_num > coin)
    {
        fprintf(stdout, "portion_num[%d] invalid, should less than coin[%d].\n", portion_num, coin);
        return 10;
    }
    //红包最大因子不能超过份数
    if ((factor >= portion_num*100) || (factor < 100))
    {
        printf("factor[%d] invalid, the correct range is [100, portion_num*1000).\n", factor);
        return 11;
    }
    int left_coin = coin;
    int left_portion_num = portion_num;
    int avg_portion = (left_coin+left_portion_num) / left_portion_num;
    int max_portion = (factor * avg_portion)/100;
    if (max_portion == 0) max_portion = 1;

    //概率检查, 雷数概率检查, 概率之和应该等于1000
    int sum_check = 0;
    for (int i=0;i<portion_num+1;i++)
    {
        sum_check += minenum_rate[i];
    }
    if (sum_check != 1000)
    {
        fprintf(stderr, "minenum_rate invalid. sum is not eqaul 1000\n");
        return 1;
    }

    //确定雷数
    int left_ceil = 0;
    int r = rand() % 1000;
    int tail_rep_num = 0;
    for (;tail_rep_num<portion_num+1; tail_rep_num ++)
    {
        int right_ceil = left_ceil + minenum_rate[tail_rep_num];
        if (r < right_ceil)
        {
            break;
        }
        left_ceil = right_ceil;
    }
    if ((tail_rep_num > 0) && (lucky_num>0))
    {
        if (test_bit(tailbmp, lucky_num % 10))
        {
            tail_rep_num --;
        }
    }


    //生成包含尾数的红包,n
    vector<int> vec_rep_portions;
    if (lucky_num > 0) 
    {
        vec_rep_portions.push_back(lucky_num);
        left_coin -= lucky_num;
        left_portion_num --; 
    }

    int trytimes=3*tail_rep_num;
    for (;;)
    {
        //确定尾数
        int r = rand();
        int tail_digit = 0;
        for (int i = r%10;i<r+10;i++)
        {
            int tail_bit=i%10;
            if (test_bit(tailbmp, tail_bit) )
            {
                tail_digit = tail_bit;
                break;
            }
        }

        r =  r % max_portion + 1; 
        r = (r/10)*10+tail_digit;
        //判断是否重复, 重复则重试生成 
        bool isrep = false;
        for(vector<int>::iterator it = vec_rep_portions.begin();it!=vec_rep_portions.end();it++)
        {
            int num = *it;
            if (num == r)
            {
                isrep = true;
                break;
            }
        }
        if (isrep || r == 0)
        {
            trytimes --;
            if (trytimes == 0)
            {
                
                fprintf(stdout, "gen mine failed!!");
                return 11;
            }
            else
            {
                continue;
            }
        }

        left_coin -= r;
        left_portion_num --; 
        vec_rep_portions.push_back(r);

        tail_rep_num --;
        if (tail_rep_num <=0)
        {
            break;
        }
    }

    /* DEBUG
    char seqstr[100]={0};
    int strn = 0;
    for(vector<int>::iterator it = vec_rep_portions.begin();it!=vec_rep_portions.end();it++)
    {

        strn += snprintf(seqstr+strn, 100-strn, " %d ", *it);
    }
    printf("left_portion_num:%d, left_coin:%d, tailbmp:%#x, factor:%d, rep_portions:%s\n"
            , left_portion_num, left_coin, tailbmp, factor, seqstr);
    */

    //生成剩下的份额
    int retry = 2;
    vector<int> vec_leftnums;
retry:
    vec_leftnums.clear();
    int ret = gen_redpacket_seq_excl(left_portion_num, left_coin, tailbmp, factor, vec_leftnums);
    if (ret)
    {
        if (retry)
        {
            goto retry;
            retry --;
        }
        return ret;
    }
     
    //合并结果
    int rep_pos = 0; 
    int left_pos = 0; 
    for(int i=0;i<portion_num;i++)
    {
        int p = -1;
        if (rep_pos < vec_rep_portions.size() && left_pos < vec_leftnums.size())
        {
            if (rand()%2)
            {
                p = vec_rep_portions[rep_pos];
                rep_pos ++;
            }
            else
            {
                p = vec_leftnums[left_pos];
                left_pos ++;
            }
        }
        else if (rep_pos < vec_rep_portions.size())
        {
            p = vec_rep_portions[rep_pos];
            rep_pos ++;
        }
        else /* (left_pos == vec_leftnums.size()) */
        {
            p = vec_leftnums[left_pos];
            left_pos ++;
        }
        vecnums.push_back(p);
    }
    random_shuffle(vecnums.begin(), vecnums.end());
    /*
    vecnums.reserve(vec_leftnums.size() + vec_rep_portions.size());
    vecnums.insert(vecnums.end(), vec_leftnums.begin(), vec_leftnums.end());
    vecnums.insert(vecnums.end(), vec_rep_portions.begin(), vec_rep_portions.end());
    */


    return 0;
}

int test_gen_redpacket_seq_by_tail_mine(int argc, char* argv[])
{
    int ret = 0;
    int minenum_rate[11] = {
         /*0*/500, /*1*/500, /*2*/0, /*3*/0, /*4*/0
        ,/*5*/0, /*6*/0, /*7*/0, /*8*/0, /*9*/0, /*10*/0
    }; 
    
    char tailstr[32]={0};
    char seqstr[100]={0};
    int strn = 0;

    int test_times = 10000000;
    int portion = 5;
    int coin = 5000;
    //int lucky_num = 2017;
    int lucky_num = 0;

    int tailarr[2] = {1,7};
    vector<int> vectail(tailarr, tailarr + sizeof(tailarr)/sizeof(int));
    int tailbmp = 0;
    for(vector<int>::iterator it = vectail.begin();it!=vectail.end();it++)
    {
        int tail = *it;
        tailbmp |= (1<<tail);
        strn += snprintf(tailstr+strn, 32-strn, "%d_", tail);
    }
    int factor = 217;

    vector<int> vecnums;

    srand(1703232754+time(NULL));

    int ok_times=0;
    int errtimes = 0;
    int errtimes_sum = 0;
    int errtimes_zero = 0;
    int errtimes_lucky = 0;
    int errtimes_portion = 0;
    for(int i=0;i<test_times;i++)
    {
        vecnums.clear();
        ret  = gen_redpacket_seq_by_tail_mine(minenum_rate, tailbmp, lucky_num, portion, coin, factor, vecnums);
        if (ret)
        {
            fprintf(stderr, "error. ret:%d\n", ret);
            continue;
            //return ret;
        } 

        bool ok = true;
        bool lucked = false;
        int sum = 0;
        int tail_num = 0;
        int zero_num = 0;
        strn = 0;
        bzero(seqstr, 100); 
        for(vector<int>::iterator it = vecnums.begin();it != vecnums.end();it++)
        {
            int p = *it;
            sum += p;
            int p_tail = p % 10;

            if (p == lucky_num)
            {
                lucked = true;
            }
                
            for(vector<int>::iterator it = vectail.begin();it!=vectail.end();it++)
            {
                int tail = *it;
                if (tail == p_tail)
                {
                    //fprintf(stdout, "hit tail!portion:%d, tail:%d\n", portion, tail);
                    tail_num ++;
                }
            }
            if (p == 0)
            {
                zero_num++;
            }
            strn += snprintf(seqstr+strn, 100-strn, "%d_", p);
        }

        if (vecnums.size()!=portion) { ok = false; errtimes_portion ++; }
        if (lucky_num > 0 && !lucked ) { ok = false; errtimes_lucky ++; }
        if (zero_num) { ok = false; errtimes_zero ++; }
        if (sum-coin) { ok = false; errtimes_sum ++; }
        if (!ok)
        {
            
            errtimes++;
            fprintf(stderr, "nok. lucky_num:%d, tailstr:%s, tail_num:%d, zero_num:%d, sum:%d, coin:%d, hasluck:%d, portion:%d, seqlen:%d, seqlist:%s\n"
                    ,lucky_num, tailstr ,tail_num, zero_num, sum, coin, lucked, portion, vecnums.size(), seqstr); 

            continue;
        }
        
        printf("%s\n", seqstr);
        ok_times ++;
    }
    fprintf(stdout, "coin:%d, portion:%d, tailbmp:%#x, factor:%d, test_times:%d, ok_times:%d, errtimes:%d, portion_err:%d, lucky_err:%d, zero_err:%d, sum_err:%d\n",
            coin, portion, tailbmp, factor, test_times, ok_times, errtimes, errtimes_portion, errtimes_lucky, errtimes_zero, errtimes_sum);


    return 0;
}

int test_gen_redpacket_seq_excl(int argc, char* argv[])
{
    int ret = 0;
    
    char tailstr[32]={0};
    char seqstr[100]={0};
    int strn = 0;

    int test_times = 10000000;
    int portion = 10;
    int coin = 1000;
    int tailarr[2] = {1,7};
    vector<int> vectail(tailarr, tailarr + sizeof(tailarr)/sizeof(int));
    int excl_bmp = 0;

    for(vector<int>::iterator it = vectail.begin();it!=vectail.end();it++)
    {
        int tail = *it;
        excl_bmp |= (1<<tail);
        strn += snprintf(tailstr+strn, 32-strn, "%d_", tail);
    }
    int factor = 217;
    vector<int> vecnums;


    srand(1703232754+time(NULL));
    
    int ok_times=0;
    for(int i=0;i<test_times;i++)
    {
        vecnums.clear();
        ret  = gen_redpacket_seq_excl(portion, coin, excl_bmp, factor, vecnums);
        if (ret)
        {
            fprintf(stderr, "error. ret:%d\n", ret);
            continue;
            //return ret;
        } 

        int sum = 0;
        int tail_num = 0;
        int zero_num = 0;
        strn = 0;
        bzero(seqstr, 100); 
        for(vector<int>::iterator it = vecnums.begin();it != vecnums.end();it++)
        {
            int portion = *it;
            sum += portion;
            int portion_tail = portion % 10;
                
            for(vector<int>::iterator it = vectail.begin();it!=vectail.end();it++)
            {
                int tail = *it;
                if (tail == portion_tail)
                {
                    fprintf(stdout, "hit tail!portion:%d, tail:%d\n", portion, tail);
                    tail_num ++;
                }
            }
            if (portion == 0)
            {
                zero_num++;
            }
            strn += snprintf(seqstr+strn, 100-strn, "%d_", portion);
        }
        if (tail_num || zero_num || (sum-coin))
        {
            fprintf(stderr, "excl_tail:%s, tail_num:%d, zero_num:%d, sum:%d, coin:%d, seqlist:%s\n"
                    ,tailstr ,tail_num, zero_num, sum, coin, seqstr); 

            continue;
        }
        printf("%s\n", seqstr);
        ok_times ++;
    }
    fprintf(stdout, "coin:%d, portion:%d, excl_bmp:%#x, factor:%d, test_times:%d, ok_times:%d\n",
            coin, portion, excl_bmp, factor, test_times, ok_times);
}


int gen_redpacket1(int portion, int coin, int factor, vector<int> &vecnums)
{
    // portion min is 1
    // rand range: [0, coin - portion] 
    int r_portion = portion;
    int left_coin = coin;
    int this_portion = 0;
    
    // 分数不能大于金额
    if (portion > coin)
    {
        return 1;
    }
    //红包最大因子不能超过份数
    if ((factor >= portion*100) || (factor < 100))
    {
        return 2;
    }
    //printf("left_coin:%d, r_portion:%d, factor:%d, avg_portion:%d, max_portion:%d\n"
    //        , left_coin, r_portion, factor, avg_portion, max_portion);


    for (int i=0; i < portion; i++)
    {
        int avg_portion = (left_coin+r_portion) / r_portion;
        int max_portion = (factor * avg_portion)/100;
        if (max_portion == 0) max_portion = 1;
        if (left_coin <= 0)
        {
            printf("left_coin <= 0, left_coin = %d\n", left_coin);
            return 3;
        }
        if (i == portion - 1)
        {
            this_portion = left_coin;
        }
        else
        {
            //至少要留下r_portion-1
            int mod_factor = left_coin - r_portion + 1;
            if (mod_factor > max_portion)
            {
                mod_factor = max_portion;
            }
            int r = rand() % mod_factor;
            this_portion = r + 1;//确保this_portion>0
            
            r_portion --;
        }
        vecnums.push_back(this_portion);
        left_coin -= this_portion;
    }

    return 0;
}

uint32_t gen_rand_seed()
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
void boost_random(vector<int> &vecnums)
{ 
    // create seed
    unsigned long seed = gen_rand_seed();
    // produces general pseudo random number series with the Mersenne Twister Algorithm
    boost::mt19937 rng ( seed );
    // uniform distribution on 1...6
    boost::uniform_int <> three(0 ,2);
    // connects distribution with random series
    boost::variate_generator< boost::mt19937&, boost::uniform_int <> > unsix(rng ,three);
    for(int i=0;i<3;i++)
    {
        vecnums.push_back(unsix());
    }

}
int test_boost_random(int argc, char* argv[])
{
    int test_times = 5000000;
    int stat[3][3] = {{0}};
    
    vector<int> vecnums;
    for (int i=0;i<test_times;i++)
    {   
        vecnums.clear();
        boost_random(vecnums);
        
        for(int i=0;i<3;i++) //i 数字
        {
            for(int j=0;j<3;j++) // j位置
            {
                int n=vecnums[j];
                stat[n][j] += 1; 
            }
        } 
    }

    for (int i=0;i<3;i++)
    {
        printf("%d %d %d %d\n", i+1, stat[i][0],stat[i][1],stat[i][2]);
    }
    return 0;
}

int gen_rfv_redpacket(int portion, int coin, int factor, vector<int> &vecnums)
{
    int avg_port = coin / portion;
    int var_port = 20; 
    int rsv_port = avg_port - var_port;
    if (rsv_port < 0)
    {   
        rsv_port = 0;
        
    }   
    int var_coin = coin - rsv_port * portion;
    
    if(0 != gen_redpacket1(portion,var_coin,factor,vecnums))
    {   
        printf("gen_redpacket1 failed! portion:%d, var_coin:%d, factor:%d\n", portion, var_coin, factor);
        return 1;
    }   

    for(size_t i = 0; i < vecnums.size(); i++)
    {   
        vecnums[i] += rsv_port;
    } 
    sort(vecnums.begin(), vecnums.end());
    //shuffle_vecint(vecnums);
}

int gen_lucky_redpacket(int portion, int coin, int factor, vector<int> &vecnums)
{
    //幸运数出现的概率
    map<int, int> map_lucky_rate;
    map_lucky_rate[2017] = 0;
    map_lucky_rate[1314] = 0;
    map_lucky_rate[381] = 0;

    int retry_times = 3;
    
    int useq = rand();

redo:
    vecnums.clear();
    retry_times --;
    if (retry_times < 0)
    {
        //DEBUG_P(LOG_ERROR, "CCreateRedPacketMineSweep::GetRateNew. reach retry limit!\n");
        printf("CCreateRedPacketMineSweep::GetRateNew. reach retry limit!useq:%d\n", useq);
        return false;
    }
    if(0 != gen_redpacket1(portion,coin,211,vecnums))
    {
        printf("CCreateRedPacketMineSweep::GetRateNew err, portion = %d, coin = %d, useq:%d\n", portion, coin, useq);
        return false;
    }

    for (unsigned i = 0; i < vecnums.size(); i++)
    {
        int c = vecnums[i];
        if (map_lucky_rate.find(c) != map_lucky_rate.end())
        {
            int prob = map_lucky_rate[c];
            int r = rand();
            r = r % 100; //[0,99]
            if (r > prob || prob == 0)  //不要出现, 重新生成
            {
                printf("CCreateRedPacketMineSweep::GetRateNew, regen! c:%d r:%d prob:%d, useq:%d, retry:%d\n", c, r, prob, useq, retry_times);
                goto redo;
            }
        }
    }

    return true;

}


int test_splitstr(int argc, char* argv[])
{
    string probstr = argv[1];
    map<int, int> testmap;
    string delim1 = "|";
    string delim2 = ":";
    size_t pos= 0;
    size_t posn = 0;
    probstr += delim1;
    for(;(posn=probstr.find(delim1, pos))!=std::string::npos;)
    {
        string t = probstr.substr(pos, posn);
        size_t pos1 = t.find(delim2);
        if (pos1 != std::string::npos)
        {
            string key = t.substr(0, pos1);
            string value = t.substr(pos1+1);
            if (key.length() > 0 && value.length() > 0)
            {
                int lucknum = atoi(key.c_str());
                int prob = atoi(value.c_str());
                testmap[lucknum] = prob;
            }
        }
        pos = posn+1;
    }

    for(map<int, int>::iterator it = testmap.begin(); it!= testmap.end(); it++)
    {
        printf("key:%d,value:%d\n", it->first, it->second);
    }
}

int test_gen_redpacket(int argc, char* argv[])
{
    srand(time(NULL));
    vector<int> vec_nums;
    // tail_digit,  mine num , num 
    int tail_stat[10][10] = {{0}};
    int stat[10] = {0};
    int test_times = 10000000;
//    int test_times = 1;
    if (argc!=3)
    {
        printf("%s coin portion\n", argv[0]);
        return 1;
    }
    int test_coin = atoi(argv[1]);
    int test_portion = atoi(argv[2]);

    char filen[32] = {0};
    snprintf(filen, 32, "redpacket_seq_%d_%d.dat", test_coin, test_times);
    FILE *file = fopen(filen, "w+"); 
    for (int i=0;i<test_times;i++)
    {
        vec_nums.clear();
        //gen_redpacket1(test_portion, test_coin, 211,  vec_nums);
        //gen_redpacket(test_portion, test_coin, vec_nums);
        //gen_rfv_redpacket(test_portion, test_coin,  153, vec_nums);
        bool genok = gen_lucky_redpacket(test_portion, test_coin, 211, vec_nums);
        if (!genok)
        {
            printf("gen_redpacket failed\n");
            continue;
        }
        
        int mine_number = 0; 
        int sum_check = 0;
        int zero_count = 0;
        for(vector<int>::iterator it = vec_nums.begin();it!=vec_nums.end();it++)
        {
            int portion = *it;
            sum_check += portion;
            if (portion == 0) zero_count++;
        }    
        if ((sum_check == test_coin) && !zero_count && (vec_nums.size() == test_portion))
        {
            fprintf(file, "%d ", sum_check);
            bzero(stat, sizeof(stat)); 
            for(vector<int>::iterator it = vec_nums.begin();it!=vec_nums.end();it++)
            {
                int portion = *it;
                int tail_digit = portion % 10;
                //统计尾数相同的情况
                stat[tail_digit] ++;
                
                fprintf(file, "%d ", portion);

            }    
            //以K为雷，统计n个雷的出现次数 
            for (int k=0;k<10;k++)
            {
                //stat[k]: k为雷出现的次数
                tail_stat[k][stat[k]] ++;
            }
            fprintf(file, "\n");
        }
        else
        {
            fprintf(stderr, "error!%d != %d, zero_count:%d,  vec_nums.size:%d\n", sum_check, 5000, zero_count, vec_nums.size());
            fprintf(stderr, "%d ", sum_check);
            for(vector<int>::iterator it = vec_nums.begin();it!=vec_nums.end();it++)
            {
                int portion = *it;
                fprintf(stderr, "%d ", portion);
            }    
            fprintf(stderr, "\n");
        }
    }
    fclose(file);


    fprintf(stdout, "\nSTAT:\n");
    for(int k=0;k<10;k++)
    {
        int sum = 0;
        for(int i=0;i<10;i++)
        {
            sum += tail_stat[k][i];
        }
        fprintf(stdout, "%d ", k);
        for(int i=0;i<10;i++)
        {
            //fprintf(file, "|%d/%d, %.4f ", tail_stat[k][i], sum, (double)1.0 * tail_stat[k][i]/sum);
            fprintf(stdout, "%.4f ",  (double)1.0 * tail_stat[k][i]/sum);
        }
        fprintf(stdout, "\n");
        fprintf(stdout, "%d ", k);
        for(int i=0;i<10;i++)
        {
            fprintf(stdout, "%d/%d ", tail_stat[k][i], sum);
        }
        fprintf(stdout, "\n");
    }


    return 0;
}

int test_sizeof(int argc, char* argv[])
{
    printf("sizeof(int)=%d\n", sizeof(int));
    printf("sizeof(long int)=%d\n", sizeof(long int));
    printf("sizeof(long long int)=%d\n", sizeof(long long int));
    printf("sizeof(unsigned)=%d\n", sizeof(unsigned));
    printf("sizeof(long unsigned)=%d\n", sizeof(long unsigned));
    printf("sizeof(long long unsigned)=%d\n", sizeof(long long unsigned));
    return 0;
}    

#if TESTMACRO
int test_macro(int argc, char* argv[])
{
    printf("where:%s,%d,%s\n", __FILE__, __LINE__, __func__);
    return 0;
}
#endif

//----------------
string g_log_name;
int g_log_level = 0;
FILE* g_log_f = NULL;
#define LOG_DEBUG 1
#define LOG_TRACE 2
#define LOG_INFO 4
#define LOG_ERROR 8
#define INIT_DPRINT(log_name, log_level)\
    do\
    {\
        if (log_name != g_log_name)\
        {\
            if (g_log_f) \
            {\
                fclose(g_log_f);\
            }\
            g_log_f = NULL;\
        }\
        if (g_log_f == NULL)\
        {\
            g_log_f = fopen(log_name, "a");\
        }\
        g_log_level = log_level;\
    }while(0)

#define DPRINT(log_level, fmt, ...) \
    do  \
    {   \
        struct timeval tv = {0};    \
        struct timezone tz = {0};   \
        gettimeofday(&tv, &tz); \
        struct tm* tmnow = localtime(&tv.tv_sec);\
        \
        FILE* log_f = (g_log_f == NULL)?stdout:g_log_f; \
        if (log_level >= g_log_level)\
        { \
            fprintf(log_f, "%4d-%02d-%02d %02d:%02d:%02d.%03d|%s+%d:%s|%d|"fmt\
                ,1900+tmnow->tm_year, tmnow->tm_mon+1, tmnow->tm_mday, tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec, tv.tv_usec/1000\
                ,__FILE__,__LINE__, __func__ , log_level,##__VA_ARGS__); \
        } \
    } while (0)

int test_log(int argc, char* argv[])
{
    INIT_DPRINT("test.log", LOG_DEBUG);
    DPRINT(LOG_ERROR, "ahahahahaha, %d\n", 1234);
    return 0;
}

int test_split(int argc, char* argv[])
{
    int post_mine0=0;
    int post_mine1=0;
    int post_mine2=0;
    int post_mine3=0;
    int post_mine4=0;
    int checksum = 0;
    bool rateerr = 0;
    string ratestr = argv[1];
    vector<int> vecrate;
    std::stringstream ss(ratestr);
    stringstream sstk;
    string tk;
    while(std::getline(ss, tk, ','))
    {
        //tk.erase(remove_if(tk.begin(), tk.end(), isspace), tk.end());
        //cout<<"["<<tk<<"]"<<endl;
        //string::iterator end_pos = remove(tk.begin(), tk.end(), ' ');
        //cout<<"["<<tk<<"]"<<endl;
        //tk.erase(end_pos, tk.end());
        tk.erase(remove(tk.begin(), tk.end(), ' '), tk.end());
        cout<<"["<<tk<<"]"<<endl;
        sstk.str(tk);
        sstk.clear();
        cout<<sstk.str()<<endl;
        int rate = 0;
        sstk >> rate;
        cout<<rate<<endl;
        if(0 > rate)
        {
            rateerr = rate;
            break;
        }
        vecrate.push_back(rate);
    }
    for(int i=0;i<vecrate.size();i++)
    {
        printf("%d|", vecrate[i]);
    }
    printf("\n");

    return 0;
}
void printfunc(int i)
{
    printf("in printfunc:%d\n", i);
}
int test_post_increment(int argc, char* argv[])
{
    int i=5;

    printf("before printfunc(i++): i:%d\n", i);
    printfunc(i++);
    printf("after:printfunc(i++): i:%d\n", i);
    printf("before printfunc(++i): i:%d\n", i);
    printfunc(++i);
    printf("after:printfunc(--i): i:%d\n", i);

    return 0;
}

int test_g_func2(int x)
{
    int y=5;
    y *= x;
    printf("func2 y:%d\n",y);
    return y;
}
int test_g_func1(int x)
{
    int y=5;
    y += x;
    y = test_g_func2(y);
    printf("func1 y:%d\n",y);
    return y;
}
int test_g(int argc, char* argv[])
{
    int i=5;
    i = test_g_func1(i);
    printf("i:%d\n");
    return i;
};

int test_insertemptyvec(int argc, char *argv[])
{
    vector<int> test1_vec;
    vector<int> test2_vec;

    test2_vec.insert(test2_vec.end(), test1_vec.begin(), test1_vec.end());
    printf("test2_vec.size:%lu\n", test2_vec.size());
    return 0;
}

int callstack(int &n)
{
    if (n <= 0) return 0;

    //4K
    int buf[1024] = {0};
    buf[1023]=0x12345678;
     
    return n + callstack(--n);

}
    
    
 


int test_stack(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("%s n\n", argv[0]);
        return 0;
    }

    int n = atoi(argv[1]) ;
    int sum = callstack(n) ;

    printf("sum: %d", sum);

}

int test_fori(int argc, char* argv[])
{
    for(int i=0;i<3;i++)
    {
        int pos=0;
        pos++;
        printf("%d\n", pos);
    }

}

int test_emptymap(int argc, char* argv[])
{
    map<int, int> mapi;
    map<int, int>::iterator it = mapi.find(123);
    if (it == mapi.end())
    {
        printf("not found\n");
    }
    else
    {
        printf("found\n");
    }


}

int test_sscanf(int argc, char* argv[])
{
    char *input = argv[1];
    int ia1 = 0; 
    int ia2 = 0; 
    int nn = 0;
    int n = sscanf(input, "%d, %*s, %d", &ia1,&ia2);
    printf("sscanf ret :%d, ia1:%d, ia2:%d, nn:%d\n", n, ia1, ia2,nn);
    return 0;
}
int test_fexist_1(int argc, char* argv[])
{
    char *filename = argv[1];
    int ret = access(filename, F_OK);
    if (ret)
    {
        printf("file not exist(access) or can not access! filename:%s, errno:%d\n", filename, errno);
    }
    else
    {
        printf("file exist (access)! filename:%s\n", filename);
    }
    
    return ret;
}
int test_fexist_2(int argc, char* argv[])
{
    char *filename = argv[1];
    struct stat st = {0};
    int ret = stat(filename, &st);
    if (ret)
    {
        printf("file not exist (stat)! filename:%s, errno:%d\n", filename, errno);
    }
    else
    {
        printf("file exist (stat)! filename:%s\n", filename);
    }
    
    return ret;
}

int main(int argc, char* argv[])
{
    
    //return test_bitshift(argc, argv);
    //return test_map(argc, argv);
    //return test_ref(argc, argv);
    //return test_map_deconstruct(argc, argv);
    //return test_hex2str(argc, argv);
    //return test_str2hex(argc, argv);
    //return test_rand(argc, argv);
    //return test_gen_redpacket(argc, argv);
    //return test_boost_random(argc, argv);
    //return test_strerror(argc, argv);
    //return test_gen_redpacket_seq_excl(argc, argv);
    //return test_gen_redpacket_seq_by_tail_mine(argc, argv);
    //return test_get_start_num(argc, argv);
    //return test_sizeof(argc, argv);
    //return test_log(argc, argv);
    //return test_macro(argc, argv);
    //return test_split(argc, argv);
    //return test_post_increment(argc, argv);
    //return test_g(argc, argv);
    //return test_insertemptyvec(argc, argv);
    //return test_splitstr(argc, argv);
    //return test_stack(argc, argv);
    //return test_fori(argc, argv);
    //return test_emptymap(argc, argv);
    //return test_sscanf(argc, argv);
    test_fexist_1(argc, argv);
    test_fexist_2(argc, argv);
}

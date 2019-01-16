#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <sys/file.h>
#include <unistd.h>
#include <stdint.h>


#include <vector>
#include <string>
#include <map>
#include <openssl/md5.h>

using namespace std;

static char syserrbuf[128] = {0};


int list_dir(char* dir, vector<string> &veclist)
{
    int ret = 0;

    struct stat dstat = {0};
    ret = stat(dir, &dstat);
    if (ret)
    {
        printf("list_dir: stat dir failed! dir:%s, errno:%d, errmsg:%s\n", dir, errno, strerror_r(errno, syserrbuf, 128));
        ret = 100;
        return ret; 
    }

    if (!S_ISDIR(dstat.st_mode))
    {
        printf("list_dir: not a dir! dir:%s\n", dir);
        ret = 200;
        return ret;
    }

    char cmd[128] = {0};
    snprintf(cmd, 128, "cd %s; find -printf '%%y %%P\\n'", dir);
    printf("list_dir: cmd:%s\n", cmd);
    FILE* fp = popen(cmd, "r");
    if (fp == NULL)
    {
        printf("list_dir: popen failed. cmd: %s\n", cmd);
        return -1;
    }

    char rbuf[512] = {0};
    for(;;)
    {
        char* rs = fgets(rbuf, 512, fp);
        if (rs == NULL)
        {
            printf("list_dir: fgets null, end\n");
            break;
        }
        int rlen = strlen(rbuf);
        rbuf[rlen-1]='\0';

        veclist.push_back(rbuf);
        
    } 

   
    pclose(fp); 

    return ret;
}

int get_file_key(const char* str)
{
    char delim = '_';
    char* pk = NULL;
    int key = 0;
    //printf("linestr:%s\n", str);

    pk = strrchr((char *)str, delim);
    if (pk == NULL)
    {
        return -1;
    } 
    
    pk++;
    //printf("keystr:%s\n", pk);

    char kstr[512] = {0};
    unsigned int ui = 0;
    int len = strlen(pk);
    if (len%2 != 0)
    {
        return -1;
    }
    for(int i=0;i<len/2;i++)
    {
        sscanf(pk+2*i, "%2x", &ui);   
        kstr[i]=(char)ui;
    }

    //printf("filestr:%s\n", kstr);
    pk = strchr(kstr, '_');
    pk = '\0';
    //printf("keystr:%s\n", kstr);
    sscanf(kstr,"%d", &key);
    //printf("keyval:%d\n", key);

    return key;    
}


int check_file(char* file,unsigned char(& md5)[16])
{
    ssize_t rsize = 0;
    int f = 0;
    char* buff = NULL;
    int ret = 0;
    char md5str[33] = {0};
    MD5_CTX md5ctx;
    MD5_Init(&md5ctx);

    f = open(file, O_RDONLY);
    if (f < 0 )
    {
        printf("check_file: file not exist! file:%s, err:%d, errmsg:%s\n", file, errno, strerror_r(errno, syserrbuf, 128));
        ret = -1001;
        goto end;
    }

    buff = (char *)malloc(16*4096);
    if (buff == NULL)
    {
        printf("check_file: malloc buf failed!\n");
        ret = -1002;
        goto end1;
        
    }
    for (;;)
    {
        memset(buff, 0, 16*4096);
        ssize_t r = read(f, buff, 4096*16);
        if (r < 0)
        {
            printf("check_file: read file failed! file:%s, errno%d, errmsg:%s\n", file, errno, strerror_r(errno, syserrbuf, 128));
            ret = -1003;
            break;
            
        }
        if (r == 0)
        {
            break;
        }

        rsize += r;

        MD5_Update(&md5ctx, buff, r);

    }

    MD5_Final(md5, &md5ctx);

    for (int i=0,pn=0; i<16; i++)
    {
        pn +=snprintf(md5str+pn, 33-pn, "%02x", (unsigned int)md5[i]); 
    }
    printf("check_file: ret:%d, file:%s, rsize:%ld, md5:%s\n", ret, file, rsize, md5str);

    free(buff);

end1:
    close(f);

end:
    return ret;

}

/*
 * ret: 0, 拷贝成功
 *       -100, 进入拷贝时 dst已经存在,
 *       -200, 源文件打开失败
 *       -201, 源文件不存在
 *       -300, 锁原文件失败
 *       -400, 创建临时目的文件失败
 *       -500, 读源文件失败
 *       -600, 写临时目的文件失败
 *       -700, 获得目的文件md5失败
 *       -701, 源文件和目的文件md5不一致
 *       -800, 重命名临时目的文件失败
 *       -801, 重命名临时目的文件时 原文件不存在
 *       -802, 重命名临时目的文件时 目的文件已存在
 *
 *  -100, -201, -801, -802 : 可以跳过迁移
 *  其他错误码需要参考日志进行处理
 *   
 */
int copy_file(char* srcfile, char* dstfile)
{
    ssize_t rsize = 0;
    ssize_t wsize = 0;
    int srcf = 0;
    int tmpdstf = 0;
    char* buf = NULL;
    int ret = 0;
    unsigned char md5[16] = {0};
    char md5str[33] = {0};
    MD5_CTX md5ctx;
    char tmpdstfile[512] = {0};
    snprintf(tmpdstfile, 512,  "%s.tmp", dstfile);
    unsigned char dstmd5[16] = {0};
    char dstmd5str[33] = {0};
    int src_stat = -1;
    int dst_stat = -1;

    //若dst存在,则退出
    
    struct stat fileinfo = {0};
    int rret = stat(dstfile,&fileinfo);
    if (rret == 0)
    {
        printf("copy_file: dstfile exist! dstfile:%s\n", dstfile);
        ret = -100;
        goto end;
    }
    srcf = open(srcfile, O_RDONLY);
    if (srcf < 0 )
    {
        printf("copy_file: file open failed! srcfile:%s, err:%d, errmsg:%s\n", srcfile, errno, strerror_r(errno, syserrbuf, 128));
        if (errno == ENOENT)
        {
            ret = -201;
        }
        ret = -200;
        goto end;
    }
    ret = flock(srcf, LOCK_EX|LOCK_NB);
    if (ret)
    {
        printf("copy_file: srcfile locked!! srcfile:%s, errno:%d, errmsg:%s\n", srcfile, errno, strerror_r(errno, syserrbuf, 128));
        ret = -300;
        goto end1;
    }

    tmpdstf = open(tmpdstfile, O_CREAT|O_WRONLY, S_IRWXU);
    if (tmpdstf < 0 )
    {
        printf("copy_file:  temp dstfile create failed! dstfile:%s, err:%d, errmsg:%s\n", tmpdstfile, errno, strerror_r(errno, syserrbuf, 128));
        ret = -400;
        goto end2;
    }


    MD5_Init(&md5ctx);


    buf = (char *)malloc(16*4096);
    for (;;)
    {
        memset(buf, 0, 16*4096);
        ssize_t r = read(srcf, buf, 4096*16);
        if (r < 0)
        {
            printf("copy_file: read srcfile failed! srcfile:%s, errno%d, errmsg:%s\n", srcfile, errno, strerror_r(errno, syserrbuf, 128));
            ret = -500;
            break;
            
        }
        if (r == 0)
        {
            break;
        }

        rsize += r;

        MD5_Update(&md5ctx, buf , r);
        
        ssize_t w = write(tmpdstf, buf, r);
        if (w<0)
        {
            printf("copy_file: write dstfile failed!  dstfile:%s, errno:%d, errmsg:%s\n", dstfile, errno, strerror_r(errno, syserrbuf, 128));
            ret = -600;
            break;
            
        }
        wsize += w;
        
    }
    free(buf);
    close(tmpdstf);

    if (ret!=0)
    {
        goto end2;
    }
    
    MD5_Final(md5, &md5ctx);
    for (int i=0,pn=0; i<16; i++)
    {
        pn +=snprintf(md5str+pn, 33-pn, "%02x", (unsigned int)md5[i]); 
    }
    printf("copyfile: ret:%d, srcfile:%s, rsize:%ld, dstfile:%s, wsize:%ld, md5:%s\n", ret, srcfile, rsize, dstfile, wsize, md5str);


    
    ret = check_file(tmpdstfile, dstmd5);
    if (ret)
    {
        printf("copyfile: checkf_file failed! tmpdstfile:%s, ret:%d\n", tmpdstfile, ret);
        ret = -700;
        goto end2;
    }
    
    if (memcmp(md5, dstmd5, 16))
    {
        for (int i=0,pn=0; i<16; i++)
        {
            pn +=snprintf(dstmd5str+pn, 33-pn, "%02x", (unsigned int)dstmd5[i]); 
        }
        printf("copyfile: dst md5 not equal src!!!. src md5:%s, dst md5:%s, srcfile:%s, dstfile:%s\n"
                , md5str, dstmd5str, srcfile, tmpdstfile);
        unlink(tmpdstfile);
        ret = -701;
        goto end2;
    }

    //此时src可能已经被删除了, 如果被删除了, 则删除tmp文件, 此时有可能源已经被disk删除了: 先加锁src, 不管加锁成功或失败, 删除src, 接着删除dst
    src_stat = stat(srcfile,&fileinfo);
    dst_stat = stat(dstfile,&fileinfo);
    if (src_stat == 0 && dst_stat != 0) //src存在, dst不存在
    {
        ret = rename(tmpdstfile, dstfile);
        if (ret)
        {
            printf("copyfile:  rename failed!!!. tmpdstfile:%s, errno:%d, errmsg:%s\n", tmpdstfile, errno, strerror_r(errno, syserrbuf, 128));
            ret = -800;
            unlink(tmpdstfile);
            goto end2;
        }
        ret = 0;
    }
    else //src不存在 或者 dst存在
    {
        if ( src_stat != 0)
        {
            printf("copy_file: src is missing when rename tmpdstfile, rename abort! srcfile:%s, src_stat:%d, dstfile:%s, dst_stat:%d\n", srcfile, src_stat, dstfile, dst_stat);
            ret = -801;
        }
        else if (dst_stat == 0)
        {
            printf("copy_file: dst exists when rename tmpdstfile, abort rename! srcfile:%s, src_stat:%d, dstfile:%s, dst_stat:%d\n", srcfile, src_stat, dstfile, dst_stat);
            ret = -802;
        }
        unlink(tmpdstfile);
    }

end2:
    flock(srcf, LOCK_UN);

end1:
    close(srcf);
    
    
end:
    return ret; 
}

int test_copy_file(char *srcfile, char* dstfile)
{
    return copy_file(srcfile, dstfile);
}

int make_dir(char *dir)
{
    int ret = 0;

    struct stat dstat = {0};
    ret = stat(dir, &dstat);
    if (ret == 0)
    {
        if (!S_ISDIR(dstat.st_mode)) // parent file   exist but not dir
        {
            printf("make_dir: not a dir! dir:%s\n", dir);
            ret = 200;
            return ret;
        }

        printf("make_dir: dir exist! dir:%s\n", dir);
        return 0;
    }

    if (errno != ENOENT)
    {
        printf("make_dir: stat dir failed! dir:%s, errno:%d, errmsg:%s\n", dir, errno, strerror_r(errno, syserrbuf, 128));
        ret = 100;
        return ret; 
    }

    //dir 不存在, 有上级目录则创建上级目录, 直到上级目录存在为止
    char *p = strrchr(dir, '/');
    if (p != NULL)
    {
        *p = '\0';
        ret = make_dir(dir);
        if (ret)
        {
            printf("make_dir:  make_dir failed. dir:%s, ret:%d\n", dir, ret);
            return ret;
        }
        *p='/';
    }

    ret = mkdir(dir, 0755);
    printf("make_dir:  syscall mkdir. dir:%s,ret:%d, errmsg:%s\n", dir, ret, (ret)?strerror_r(errno, syserrbuf, 128):"success");
    return ret;

}

int test_make_dir(char *dir)
{
    printf("test_make_dir: %s\n", dir);
    return make_dir(dir);
    
}
    

int copy_dir(char *srcdir, char* dstdir)
{
    int ret = 0;
    char wbuf[512] =  {0};
    int wn = 0;

    char srcdirname[128] = {0};
    char dstdirname[128] = {0};
    strncpy(srcdirname, srcdir, 127);
    for(char *p = strchr(srcdirname, '/');p!=NULL;p = strchr(srcdirname, '/'))
    {
        *p = '_'; 
    }
    strncpy(dstdirname, dstdir, 127);
    for(char *p = strchr(dstdirname, '/');p!=NULL;p = strchr(dstdirname, '/'))
    {
        *p = '_'; 
    }
    char taskname[256] = {0};
    snprintf(taskname, 256, "%s-%s", srcdirname, dstdirname);
    char lstfilen[128] = {0};
    snprintf(lstfilen, 128, "%s.lst", taskname);
    char prgfilen[128] = {0};
    snprintf(prgfilen, 128, "%s.prg", taskname);

    // 生成拷贝源列表:若有Prg文件和List文件, 则继续, 否则新建
    bool exist_lstfile = true;
    struct stat fstat = {0};
    ret = stat(lstfilen, &fstat);
    if (ret)
    {
        if (errno != ENOENT)
        {
            printf("copy_dir: stat file failed! file:%s, errno:%d, errmsg:%s\n", lstfilen, errno, strerror_r(errno, syserrbuf, 128));
            ret = 100;
            return ret; 
        }
        else
        {
            exist_lstfile = false;
        }
    }
    bool exist_prgfile = true;
    ret = stat(prgfilen, &fstat);
    if (ret)
    {
        if (errno != ENOENT)
        {
            printf("copy_dir: stat file failed! file:%s, errno:%d, errmsg:%s\n", prgfilen, errno, strerror_r(errno, syserrbuf, 128));
            ret = 100;
            return ret; 
        }
        else
        {
            exist_prgfile = false;
        }
    }
    
    vector<string> dvs;
    int prg = 0;
    if (exist_lstfile)
    {
        char linebuf[512] = {0};
        if (exist_prgfile)
        {
            FILE *prgf = fopen(prgfilen,"r");
            for(int i=0;NULL!=fgets(linebuf, 512, prgf);i++);
            
            int tsnow = 0;
            int n = sscanf(linebuf, "now:%d i:%d %*s", &tsnow, &prg);
            if (n != 2)
            {
                prg = 0;
                printf("copy_dir: get prg point from prg file failed.\n");
            }
            fclose(prgf);
        }
        printf("copy_dir: start from prg:%d\n", prg);
        FILE *listf = fopen(lstfilen, "r");
        for(int i=0;NULL!=fgets(linebuf, 512, listf);i++)
        {
            if (i<prg) continue;
            char *p = strchr(linebuf, '\n');
            if (p!=NULL) *p = '\0';
            dvs.push_back(linebuf);
        }
        fclose(listf);
    }
    else 
    {
        int lstfile = creat(lstfilen, 0755);
        if (lstfile<0)
        {
            printf("creat list file failed. filename: %s, errno:%d, errmsg:%s\n", lstfilen, errno, strerror_r(errno, syserrbuf, 128));
            return 100;
        }

#ifdef SORT_COPYLIST
        vector<string> vs;
        ret = list_dir(srcdir, vs);
        //sort
        map<int, string> mdir;
        for(size_t i=0;i<vs.size();i++)
        {
            int filekey  = get_file_key(vs[i].c_str());
            if (filekey<0)
            {
                dvs.push_back(vs[i]);
            }
            else
            {
                if (mdir.find(filekey) != mdir.end())
                {
                    dvs.push_back(vs[i]);
                }
                else
                {
                    mdir[filekey] =  vs[i];
                }
            }
        }

        for(map<int, string>::iterator it = mdir.begin();it!= mdir.end();it++)
        {
            dvs.push_back(it->second);
        }
#else
        ret = list_dir(srcdir, dvs);
#endif
        for(size_t i=0;i<dvs.size();i++)
        {
            wn = snprintf(wbuf, 512, "%s\n", dvs[i].c_str());
            write(lstfile, wbuf, wn);        
        }

        close(lstfile);
    }

    
    // 2. md5文件 srcdir-dstdir.md5  
    //  #srcdir dstdir
    //  #filename md5
    //
    // 3. 进度文件, 每copy1000个文件打印一个进度, srcdir-dstdir.prg
    // #srcdir dstdir      
    // #time   finished/total  percent
    FILE *prgfile = fopen(prgfilen, "a");
    if (prgfile == NULL)
    {
        printf("creat prg file failed. filename: %s, errno:%d, errmsg:%s\n", prgfilen, errno, strerror_r(errno, syserrbuf, 128));
        return 200;
    }


    char errfilen[128] = {0};
    snprintf(errfilen, 128, "%s.err", taskname);
    int errfile = creat(errfilen, 0755);
    if (errfile<0)
    {
        printf("creat err file failed. filename: %s, errno:%d, errmsg:%s\n", errfilen, errno, strerror_r(errno, syserrbuf, 128));
        return 300;
    }

    int step = 1000;
    char dstfile[512] = {0};
    char srcfile[512] = {0};

    vector<string> vserrdir;
    int copy_errcnt = 0;
    int copy_okcnt = 0;
    int copy_skipcnt = 0;
    int copy_dircnt = 0;
    for(size_t i=prg; i<dvs.size(); i++)
    {

        string& srcfileinfo = dvs[i];
        char filetype = srcfileinfo[0];
        const char *srcp = srcfileinfo.c_str() + 2;
        snprintf(dstfile, 512, "%s/%s", dstdir, srcp);
        bool inerrdir = false;
        for(size_t ii=0;ii<vserrdir.size();ii++)
        {
            string& errdir = vserrdir[ii];
            
            if (NULL != strstr(srcp, errdir.c_str()))
            {
                printf("src file is in err dir! srcfile:%s, errdir:%s\n", srcp, errdir.c_str());
                inerrdir = true;
                break;
            }
        }
        if (inerrdir) continue;

        snprintf(srcfile, 512, "%s/%s", srcdir, srcp);   

        if (filetype == 'd')
        {
            printf("copy_dir: call make_dir. dir:%s\n", dstfile);
            ret = make_dir(dstfile);
            if (ret)
            {
                vserrdir.push_back(dstfile);
            }
            copy_dircnt ++;

        }
        else if (filetype == 'f')
        {
            ret = copy_file(srcfile, dstfile);
            if (ret)
            {
                //write errfile
                wn = snprintf(wbuf, 512, "srcfile:%s dstfile:%s errcode:%d\n", srcfile, dstfile, ret);
                write(errfile, wbuf, wn);
                copy_errcnt ++;
            }
            else
            {
                copy_okcnt ++;
            }

        }
        else
        {
            printf("copy_dir: src file is not regular file. srcfile:%s, file type:%c\n", srcfile, filetype);
            copy_skipcnt ++;
        }

        // write prg file
        if (i%step == 0 || i == (dvs.size() -1))
        {
            int ts = time(NULL);
            wn = fprintf(prgfile, "now:%d i:%ld total:%ld err:%d skip:%d ok:%d dir:%d progress:%03.2f%%\n", 
                        ts, i+1, dvs.size(), copy_errcnt, copy_skipcnt, copy_okcnt, copy_dircnt, (100.0*(i+1))/dvs.size());
            fflush(prgfile);
        }

    }

    fclose(prgfile);
    close(errfile);


    return 0;
        
        
}
    



    

    

int test_list_dir(char *dir)
{
    int ret = 0;
    vector<string> vs; 
    ret = list_dir(dir, vs);
    if (ret)
    {
        return ret;
    }
    
    printf("dir: %s\n", dir);
    for(size_t i=0;i<vs.size();i++)
    {
        printf("%s\n", vs[i].c_str());
    }


    return 0;
}

#define CHECK_ARG_NUM(n)\
    do {\
        if (argc < n)\
        {\
            printf("insufficient parameters!");\
            exit(-1);\
        }\
    } while(0)

int main(int argc, char* argv[])
{
    int ret = 0;

    
    //CHECK_ARG_NUM(1);
    //char* dir = argv[1];
    //ret = test_list_dir(dir);

    //CHECK_ARG_NUM(1);
    //char* dir = argv[1];
    //ret = test_make_dir(dir);
    
    CHECK_ARG_NUM(2);
    char* srcdir = argv[1];
    char* dstdir = argv[2];
    ret = copy_dir(srcdir, dstdir);

    //CHECK_ARG_NUM(2);
    //char* srcfile = argv[1];
    //char* dstfile = argv[2];
    //ret = test_copy_file(srcfile, dstfile);

    //CHECK_ARG_NUM(1);
    //char* linestr=argv[1];
    //int val = get_file_key(linestr); (void)val;
    
    exit(ret);


    
}

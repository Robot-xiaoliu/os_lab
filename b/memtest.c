#define __LIBRARY__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/*
_syscall2( int, create_thread, unsigned long, start, void *, arg );
_syscall2( int, join_pthread, int, thread, void *, value_ptr );
_syscall1( int, end_thread, int, value );*/

int pthread_attr_init(void){ /*这个实验没有必要用到这个初始化函数*/
    return 0;
}
int pthread_create(int *thread, void *(*start)(void *), void *arg)
{
    int id = 0;
    if (!thread) 
        return EINVAL;

    id = create_thread((unsigned long)start,(int *)arg);

    if ( id < 0 ) 
        return errno;
    *thread = id;
    return id;
}
void pthread_exit(void *value_ptr)
{
    end_thread((int) value_ptr);
}
int pthread_join(int thread, void **value_ptr)    
{
    join_pthread( thread, value_ptr );
    return 0;
}

void memoryTest(void * arg)
{
    int *location = (int *)arg;
    int start_ptr = *location;
    int end_ptr = *(location + 1);
    int times = *(location + 2);
    int *totalCorrect_ptr = location + 3; /*主线程中记录测试成功的内存单元总个数的变量的指针*/
    int *totalTimes_ptr = location + 4; /*主线程中记录测试内存单元总个数的变量的指针*/
    char incorrectFlag = '0'; /*本次测试是否通过的标志，字符‘0’表示通过，‘1’表示测试没有通过*/
    char random = rand();
    char testData[5] = {0x00, 0xFF, 0x55, 0xAA, random};  /*用于测试的五数据，最后一个是没有被初始化的随机数random没有初始化*/
    char *i = (char *)start_ptr;
    int j = 0;
    int k = 0;

    for( ; i <= (char *)end_ptr; i++)
    {
        for(j = 0; j < times; j++)
        {
             if(*(location + 5) == 1)
                 pthread_exit(0);
             for(k = 0; k < 5; k++)  /*五种数据都通过才算是通过测试*/
             {
                 if(*(location + 5) == 1)/*如果主线程发了终止线程信号，要立刻终止该线程*/
                     pthread_exit(0);

                 *i = testData[k];
                 if(*i != testData[k])
                 {
                     incorrectFlag = '1';
                     break;
                 }
             }
             if(incorrectFlag == '1')/*times次测试中有一次不成功就算是没有通过测试*/
                 break;
        }
        if(*(location + 5) == 1) /*如果主线程发了终止线程信号，要立刻终止该线程*/
            pthread_exit(0);

        if(incorrectFlag == '0') /*通过时候测试通过的内存单元个数加一*/
            (*totalCorrect_ptr)++;
        (*totalTimes_ptr)++; /*完成的测试内存单元数要加一*/
        incorrectFlag = '0';/*标志复位*/
    }
    pthread_exit(0); /*线程安全退出*/

}

int main()
{
    char *command[6] = {"thread", "times", "go", "status", "abort", "exit"};/*所有的命令*/
    char *tmpCommand;
    int numOfThreads = 0;   /*线程总数*/
    int times = 0;           /*测试的次数*/
    int threads[64] = {0};  /*设置最大64个线程足够使用*/
    int addrSize = 0;

    /*每个线程有五个参数需要知道，依次为开始测试地址，
     *结束地址，每个地址测试次数，测试正常的内存单元个数，测试内存单元的总个数和主线程是否发出了终止线程的信号，把第一个参数地址传递过去，
     *其他的四个通过地址增加来共享主线程的数据，从而得到参数或者对参数内容进行读写
     */
    int args[64][6] = {{0}};
    int i = 0;
    int value = 0;  /*等待线程结束的返回之存放在这个变量中*/

    char *testAddr = (char *)malloc(0x000FFFFF); /*申请1MB内存用于测试*/
    
    while(1)
    {
        printf(">>>>");
        fflush(stdout);
        scanf("%s",tmpCommand);
        if( strcmp(tmpCommand, command[0]) == 0 )  /* "thread number" */
        {
            scanf("%d",&numOfThreads);
            /* 读到“thread number”,接着设置每个线程需要的开始和结束地址的参数
             * 再创建number个线程
             */
            addrSize = 0x000FFFFF / numOfThreads;
            args[0][0] = (int)testAddr;
            args[0][1] = args[0][0] + addrSize - 1;
            for(i = 1; i < numOfThreads - 1; i++)
            {
                args[i][0] = args[i - 1][1] + 1;
                args[i][1] = addrSize + args[i][0] - 1;
            }
            args[i][0] = args[i - 1][1] + 1;
            args[i][1] = (int)testAddr + 0x000FFFFF - 1;    /*测试结束的地址*/
            printf("Will use %d threads.\n",numOfThreads);
            continue;
        }

        if( strcmp(tmpCommand, command[1]) == 0 )  /* “times” */
        {
            scanf("%d",&times);
            for(i = 0; i < numOfThreads; i++)
            {
                args[i][2] = times;
            }
            printf("Each byte will be tested by %d times.\n",times);     
            continue;
        }
        if( strcmp(tmpCommand, command[2]) == 0 )  /* “go” */
        {
            for(i = 0; i < numOfThreads; i++) /*创建线程*/
                pthread_create(&threads[i], (void *)memoryTest, (void *)args[i]);
            continue;
        }
        if( strcmp(tmpCommand, command[3]) == 0 )  /* “status” */
        {
            for(i = 0; i < numOfThreads; i++)
            {
                /*为了简化操作，如果本次已经测试的总内存单元数(放在args[i][4])等于分配的总个数或者线程状态被设置为'1'了，可以简单的认为表示本线程已经结束*/
                if( args[i][4] == args[i][1] - args[i][0] + 1 || args[i][5] == 1 )
                    printf("Thread %d has exited.(%p-%p, %d/%d/%d)\n", i, args[i][0], args[i][1], args[i][3], args[i][4], args[i][1] - args[i][0] + 1);
                else
                    printf("Thread %d is running.(%p-%p, %d/%d/%d)\n", i, args[i][0], args[i][1], args[i][3], args[i][4], args[i][1] - args[i][0] + 1);
            }
            continue;
        }
        if( strcmp(tmpCommand, command[4]) == 0 )  /* “abort” */
        {
            /*线程终止状态设置为'1'，表示线程被人为退出*/
            for(i = 0; i < numOfThreads; i++)
                args[i][5] = 1;
            continue;
        }
        if( strcmp(tmpCommand, command[5]) == 0 )  /* “exit” */
        {
            for(i = 0; i < numOfThreads; i++) /*给每个线程的标志为设置为1，让其退出*/
            {
                args[i][5] = 1;  /*线程终止状态设置为'1'，表示线程被人为退出*/
            }

            /*标志为设置为1之后要等待每个线程真正的结束，主线程才能退出，否则主线程释放内存
             *线程再执行的时候会出现严重的错误
             */
            for(i = 0; i < numOfThreads; i++)
            {
                pthread_join(threads[i], (void *)value); 
            }
            return 0;
        }
    }
    return 0;
}
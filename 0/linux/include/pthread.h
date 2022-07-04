// 修改
/*
*    pthread.c
*
*    POSIX threads library based on linux-0.11
*
*     All rights reserved.
*/


#ifndef PTHREAD_H
#define PTHREAD_H


/*这个关于线程创建的信息结构，在实际中并没有用到，
   如果是创建用户级线程，这个东西是肯定要用的，估计*/
struct pthread_attr{
    size_t stacksize;
    int state;
    int sched_priority;
};

typedef struct pthread_attr pthread_attr_t;
typedef unsigned int pthread_t;


#ifdef  __cplusplus
extern "C" {
#endif

void pthread_exit(void *value_ptr);
int pthread_attr_init(pthread_attr_t *attr);
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg);
int pthread_join(pthread_t thread, void **value_ptr);

#ifdef  __cplusplus
}
#endif

#endif



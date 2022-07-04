// 修改
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#define __LIBRARY__
#include <unistd.h>

/*
*    pthread.c
*
*    POSIX threads library based on linux-0.11
*
*    Copyright (C) 2008  All rights reserved.
*/

_syscall2( int, create_thread, unsigned long, start, void *, arg );
_syscall2( int, syspthread_join, pthread_t, thread, void *, value_ptr );
_syscall1( int, endthread, int, value );

int pthread_attr_init(pthread_attr_t *attr){ /*这个实验没有必要用到这个初始化函数*/
    ;;;;;
    return 0;
}
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg){
    int id = 0;
    if (!thread) return EINVAL;

    id = create_thread((unsigned long)start,(int *)arg);

    /*printf("here after create_thread!\n");*/
    /*stop: goto stop;*/
    if ( id < 0 ) return errno;
        *thread = id;
    return id;
}

void pthread_exit(void *value_ptr){
    endthread((int) value_ptr);
}
int pthread_join(pthread_t thread, void **value_ptr){
    syspthread_join( thread, value_ptr );
    return 0;
}
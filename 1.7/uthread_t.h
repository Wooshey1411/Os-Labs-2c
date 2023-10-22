#pragma once
#include <ucontext.h>

typedef void* (*routine)(void*);

typedef struct Uthread{
    int id;
    void* stack_p;
    ucontext_t ucontext;
    routine thread_routine;
    void* args;
    void* ret_val;
    int lock;
    int wait_join;
    int on_exit;
} uthread_t;
#pragma once
#include "uthread_t.h"

enum codes{
    SUCCESS_CODE = 0,
    MEMORY_MAP_ERROR_CODE = -1,
    GET_CONTEXT_ERROR = -2,
};

int uthread_create(uthread_t* thread, void *(start_routine), void *arg);


int uthread_join(uthread_t * thread, void **returned_value);

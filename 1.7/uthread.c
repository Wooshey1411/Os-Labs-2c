#include "uthread.h"
#include "list_of_threads.h"
#define _GNU_SOURCE
#include <sched.h>
#include "sys/mman.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "sys/signal.h"
#include "sys/time.h"
#include <linux/futex.h>
#include "sys/syscall.h"
#include "errno.h"

#define STACK_SIZE (8*1024*1024)
#define ON_FUTEX_JOIN_WAIT 0xA
#define ON_FUTEX_RETURN_WAIT 0xB

context_node* list;
int is_scheduler_inited = 0;
int is_need_clean = 0;
ucontext_t* last_context;

int create_stack(void** memory){
    *memory = mmap(NULL,STACK_SIZE,PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1,0);
    if(*memory == MAP_FAILED){
        return MEMORY_MAP_ERROR_CODE;
    }
    int code = mprotect(*memory,STACK_SIZE,PROT_WRITE | PROT_READ);
    if(code != 0){
        return MEMORY_MAP_ERROR_CODE;
    }
    return SUCCESS_CODE;
}

void* start_thread(void* vid){
    int id = *(int*)vid;
    uthread_t* thread_st = get_context_by_id(list,id);
    void* ret_val = thread_st->thread_routine(thread_st->args);
    printf("routine done\n");
    while(thread_st->lock != 1){
        raise(SIGALRM);
    }
    thread_st->ret_val = ret_val;
    thread_st->lock = 2;


    delete_node_by_id(&list,id);
    int err;
    is_need_clean = 1;
    err = setcontext(&(get_context_by_pos(list,0)->ucontext));
    if(err == -1){
        perror("setcontext");
        exit(-1);
    }
    return NULL;
}

void schedule(){
    int error;
    ucontext_t *cur_context,*next_context;
    static int current_num = 0;
    if(is_need_clean){
        munmap(last_context->uc_stack.ss_sp,STACK_SIZE);
        is_need_clean = 0;
    }
    cur_context = &(get_context_by_pos(list,current_num % get_length(list))->ucontext);
    current_num++;
    next_context = &(get_context_by_pos(list,current_num % get_length(list))->ucontext);
    last_context = next_context;
    error = swapcontext(cur_context,next_context);
    if(error == -1){
        perror("swapcontext");
        abort();
    }
}

int uthread_create(uthread_t* thread, void *(start_routine), void *arg){
    static int threads_id = 0;
    void* stack;
    int code = create_stack(&stack);
    if(code == MEMORY_MAP_ERROR_CODE){
        return MEMORY_MAP_ERROR_CODE;
    }

    uthread_t* thread_structure = (uthread_t*)(stack + STACK_SIZE - sizeof(uthread_t));
    thread_structure->stack_p = stack+STACK_SIZE;

    code = getcontext(&thread_structure->ucontext);
    if(code == -1){
        if(munmap(stack,STACK_SIZE) != 0){
            perror("munmap");
            abort();
        }
        return GET_CONTEXT_ERROR;
    }

    thread_structure->ucontext.uc_stack.ss_sp = stack;
    thread_structure->ucontext.uc_stack.ss_size = STACK_SIZE - sizeof(ucontext_t);
    thread_structure->ucontext.uc_link = NULL;
    thread_structure->id = threads_id++;
    thread_structure->lock = -1;
    thread_structure->on_exit = 0;
    thread_structure->wait_join = 0;
    thread_structure->thread_routine = start_routine;
    thread_structure->args = arg;

    makecontext(&thread_structure->ucontext,(void*)start_thread,1,(void*)&(thread_structure->id));

    if(!is_scheduler_inited){
        uthread_t main_st;
        getcontext(&main_st.ucontext);
        main_st.id=-1;
        list = init_list(main_st);
        push(list,*thread_structure);
        struct sigaction mask;
        sigemptyset(&mask.sa_mask);
        mask.sa_flags = SA_SIGINFO;
        mask.sa_sigaction = schedule;
        code = sigaction(SIGALRM,&mask,NULL);
        if(code == -1){
            perror("sigaction");
            abort();
        }
        struct itimerval interval;
        interval.it_value.tv_sec = 0;
        interval.it_value.tv_usec = 100000;
        interval.it_interval = interval.it_value;
        code = setitimer(ITIMER_REAL,&interval,NULL);
        if(code == -1){
            perror("setitimer");
            abort();
        }
    is_scheduler_inited = 1;
    } else{
        push(list,*thread_structure);
    }
    thread = thread_structure;

    return SUCCESS_CODE;
}


int uthread_join(uthread_t * thread, void **returned_value){
    thread->lock = 1;
    while(thread->lock != 2){
        raise(SIGALRM);
    }



    if(*returned_value == NULL) {
        return SUCCESS_CODE;
    }
    *returned_value = thread->ret_val;
    return SUCCESS_CODE;
}
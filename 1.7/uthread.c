#include "uthread.h"
#include "list_of_threads.h"
#include "sys/mman.h"
#include <stdio.h>
#include <stdlib.h>
#include "sys/signal.h"
#include "sys/time.h"

#define STACK_SIZE (8*1024*1024)
#define FREQUENCY 3000

context_node* list;
int is_scheduler_inited = 0;


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
#ifdef DEBUG
    printf("routine done. id=%d\n",id);
#endif
    thread_st->ret_val = ret_val;

    thread_st->lock = 1;
    int err;
    while (1){}

    abort();// never achieve here.

    return NULL;
}

void schedule(){
    int error;
    ucontext_t *cur_context,*next_context;
    static int current_num = 0;
    if(current_num == 33){
        is_scheduler_inited = 1;
    }
    cur_context = &(get_context_by_pos(list,current_num % get_length(list))->ucontext);
    current_num++;
    next_context = &(get_context_by_pos(list,current_num % get_length(list))->ucontext);
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
    thread_structure->stack_p = stack;
#ifdef DEBUG
    printf("thread creation. stack=%p\n",thread_structure->stack_p);
#endif
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
        push(list,(uthread_t*)(stack + STACK_SIZE - sizeof(uthread_t)));
        struct sigaction mask;
        sigemptyset(&mask.sa_mask);
        mask.sa_flags = SA_SIGINFO| SA_STACK ;
        mask.sa_sigaction = schedule;
        code = sigaction(SIGALRM,&mask,NULL);
        if(code == -1){
            perror("sigaction");
            abort();
        }
        struct itimerval interval;
        interval.it_value.tv_sec = 0;
        interval.it_value.tv_usec = FREQUENCY;
        interval.it_interval = interval.it_value;
        code = setitimer(ITIMER_REAL,&interval,NULL);
        if(code == -1){
            perror("setitimer");
            abort();
        }
        is_scheduler_inited = 1;
    } else{
        sigset_t sigset,old_set;
        sigemptyset(&sigset);
        sigfillset(&sigset);
        sigprocmask(SIG_BLOCK,&sigset,&old_set);
        push(list,(uthread_t*)(stack + STACK_SIZE - sizeof(uthread_t)));
        sigprocmask(SIG_SETMASK,&old_set,NULL);
    }
    thread->stack_p = stack;
    thread->id = thread_structure->id;
    return SUCCESS_CODE;
}


int uthread_join(uthread_t* thread, void **returned_value){
    uthread_t* r_th = (uthread_t*)(thread->stack_p-sizeof(uthread_t) + STACK_SIZE);
    while(r_th->lock != 1){
        raise(SIGALRM);
    }
#ifdef DEBUG
    printf("gained value on id=%d\n",thread->id);
#endif
    int code = 0;

    if(returned_value != NULL) {
        *returned_value = r_th->ret_val;
    }
    r_th->lock = 2;
    sigset_t sigset,old_set;
    sigemptyset(&sigset);
    sigfillset(&sigset);
    sigprocmask(SIG_BLOCK,&sigset,&old_set);
    delete_node_by_id(&list,thread->id);
    code = munmap(thread->stack_p,STACK_SIZE);
    if(code != 0){
        perror("munmap");
        exit(-1);
    }
    sigprocmask(SIG_SETMASK,&old_set,NULL);
#ifdef DEBUG
    printf("joined successfully. id=%d\n",thread->id);
#endif
    return SUCCESS_CODE;
}

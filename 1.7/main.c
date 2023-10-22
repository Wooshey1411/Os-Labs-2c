#include <stdio.h>
#include "src/uthread.h"
#include "unistd.h"
#include "signal.h"

void* worker1(void* arg){
    for(int i = 0; i < 10; i++){
        printf("%d\n",i);
        sleep(2);
    }
    printf("Bye from worker1\n");
    return "BEBRA";
}

void* worker2(void* arg){
    while(1){
        printf("Hi, im worker2\n");
        sleep(2);
    }
}

int main() {
    uthread_t s1;
    uthread_t s2;
    printf("%d\n",getpid());
    uthread_create(&s1, worker1,"ABOBA");
    uthread_join(&s1,NULL);
}

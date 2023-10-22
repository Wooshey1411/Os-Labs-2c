#include <stdio.h>
#include "src/uthread.h"
#include "unistd.h"
#include "string.h"

void* worker1(void* arg){
    for(int i = 0; i < 10; i++){
        printf("%d\n",i);
        sleep(2);
    }
    printf("Bye from worker1\n");
    return "BEA";
}

void* worker2(void* arg){
    int num_arg = *((int*)arg);
    while(1){
        printf("Hi, im %d\n",num_arg);
        sleep(2);
    }
}

int main() {
    uthread_t s1;
    uthread_t s2;
    void* a;
    uthread_create(&s1, worker1,NULL);
    uthread_create(&s2, worker1,NULL);
    uthread_join(&s1,&a);
    printf("%s\n",(char*)a);
    void* b;

    uthread_join(&s2,&b);
    printf("%s\n",(char*)b);
}

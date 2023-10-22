#include "list_of_threads.h"
#include <malloc.h>

int global_length;

context_node* init_list(uthread_t value){
    context_node* node = (context_node*)malloc(sizeof(context_node));
    uthread_t* info = (uthread_t*) malloc(sizeof(uthread_t));
    info->id = value.id;
    node->thread_info = info;
    node->next = NULL;
    global_length = 1;
    return node;
}

void push(context_node* head, uthread_t* value){
    context_node* curr_node = head;

    if(curr_node == NULL){
        curr_node = (context_node*)malloc(sizeof(context_node));
        curr_node->next = NULL;
        curr_node->thread_info = value;
        return;
    }

    while (curr_node->next != NULL){
        curr_node = curr_node->next;
    }

    context_node* next_node = (context_node*)malloc(sizeof(context_node));
    next_node->thread_info = value;
    next_node->next = NULL;
    curr_node->next = next_node;
    global_length++;
}

void delete_node_by_pos(context_node** head, int pos){
    if(pos == 0){
        context_node* first_node = *head;
        *head = (*head)->next;
        free(first_node);
        return;
    }
    context_node* curr_node = *head;
    context_node* prev_node;
    int curr_pos = 0;
    while(curr_node != NULL && curr_pos < pos){
        prev_node = curr_node;
        curr_node = curr_node->next;
        curr_pos++;
    }
    if(curr_node == NULL){
        return;
    }
    prev_node->next = curr_node->next;
    free(curr_node);
    global_length--;
}

int get_length(context_node* node){
    int len = 0;
    context_node* curr_node = node;
    while(curr_node != NULL){
        len++;
        curr_node=curr_node->next;
    }
    return len;
}

uthread_t* get_context_by_pos(context_node* head, int pos){
    context_node* curr_node = head;
    int curr_pos = 0;
    while(curr_node != NULL && curr_pos < pos){
        curr_pos++;
        curr_node=curr_node->next;
    }
    if(curr_node == NULL){
        return NULL;
    }
    return (curr_node->thread_info);
}

uthread_t* get_context_by_id(context_node* head, int id){
    context_node* curr_node = head;
    while(curr_node != NULL && curr_node->thread_info->id != id){
        curr_node=curr_node->next;
    }
    if(curr_node == NULL){
        return NULL;
    }
    return (curr_node->thread_info);
}

void delete_node_by_id(context_node** head, int id){
    if(id == (*head)->thread_info->id){
        context_node* first_node = *head;
        *head = (*head)->next;
        free(first_node);
        return;
    }
    context_node* curr_node = *head;
    context_node* prev_node;
    while(curr_node != NULL && id != curr_node->thread_info->id){
        prev_node = curr_node;
        curr_node = curr_node->next;
    }
    if(curr_node == NULL){
        return;
    }
    prev_node->next = curr_node->next;
    free(curr_node);
    global_length--;
}

int get_length_from_global(){
    return global_length;
}
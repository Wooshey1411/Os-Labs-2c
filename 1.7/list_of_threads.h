#include "uthread_t.h"
#include <ucontext.h>

typedef struct Node{
    struct Node* next;
    uthread_t thread_info;
} context_node;

context_node* init_list(uthread_t value);

uthread_t* get_context_by_pos(context_node* head, int pos);

void push(context_node* head, uthread_t value);

void delete_node_by_pos(context_node** head, int pos);

void delete_node_by_id(context_node** head, int id);

int get_length(context_node* node);

uthread_t* get_context_by_id(context_node* head, int id);

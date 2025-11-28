#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdatomic.h>
     
int num_threads = 0;
int id_glob = 0;
pthread_mutex_t id_lock;
pthread_mutex_t stack_lock;

atomic_int id_cas = 0;
     
typedef struct node { 
     int node_id;      //a unique ID assigned to each node
     struct node *next;
} Node;

Node *top; // top of stack
_Atomic(Node *) top_cas;

/*Option 1: Mutex Lock*/
void push_mutex() { 
     Node *old_node;
     Node *new_node;
     new_node = malloc(sizeof(Node)); 

     //update top of the stack below
     //assign a unique ID to the new node

     pthread_mutex_lock(&id_lock);
     new_node->node_id = id_glob++;
     pthread_mutex_unlock(&id_lock);

     pthread_mutex_lock(&stack_lock);
     new_node->next = top;
     top = new_node;
     pthread_mutex_unlock(&stack_lock);

}

int pop_mutex() { 
     Node *old_node;
     Node *new_node;

     //update top of the stack below

     int id = -1;
     
     pthread_mutex_lock(&stack_lock);
     if (top != NULL) {
          old_node = top;
          top = top->next;
          id = old_node->node_id;
          free(old_node);
     }
     pthread_mutex_unlock(&stack_lock);

     return id;
}

/*Option 2: Compare-and-Swap (CAS)*/
void push_cas() { 
     Node *old_node;
     Node *new_node;
     new_node = malloc(sizeof(Node)); 
     
     //update top of the stack below
     //assign a unique ID to the new node
     
     new_node->node_id = atomic_fetch_add(&id_cas, 1);


     old_node = atomic_load(&top_cas);
     do {
          new_node->next = old_node; 
     } while (!atomic_compare_exchange_weak(&top_cas, &old_node, new_node));
     

}

int pop_cas() { 
     Node *old_node;
     Node *new_node;

     //update top of the stack below

     int id = -1;
     
     old_node = atomic_load(&top_cas);
     do {
          new_node = old_node->next;      
     } while (!atomic_compare_exchange_weak(&top_cas, &old_node, new_node));
     
     id = old_node->node_id;
     free(old_node);
     
     return id;
}
typedef struct {
     int id;
     int opt;
} Thread_args;

/* the thread function */
void *thread_func(void *args) { 
     /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */

     Thread_args *arg = (Thread_args *)args; //Only one arg allowed in thread func

     int opt = arg->opt;
     int my_id = arg->id;


     if( opt==0 ){
          push_mutex();push_mutex();pop_mutex();pop_mutex();push_mutex();
     }else{
          push_cas();push_cas();pop_cas();pop_cas();push_cas();
     }
     
     printf("Thread %d: exit\n", my_id);
     free(arg);
     pthread_exit(0);
}


int count_stack(Node *top) {
     int count = 0;
     Node *current = top;
     while (current != NULL) {
          count++;
          current = current->next;
     }
     return count;
}

void free_stack(Node **top) {
     Node *current = *top;
     while (current != NULL) {
          Node *temp = current;
          current = current->next;
          free(temp);
     }
}

void free_cas_stack(_Atomic(Node *) *top) {
     Node *current = atomic_load(top);
     while (current != NULL) {
          Node *temp = current;
          current = current->next;
          free(temp);
     }
     atomic_store(top, NULL);
}

int main(int argc, char *argv[])
{
     num_threads = atoi(argv[1]);


     /* Option 1: Mutex */ 

     pthread_mutex_init(&id_lock, NULL);
     pthread_mutex_init(&stack_lock, NULL);

     pthread_t *workers = malloc(num_threads * sizeof(pthread_t));
     for (int i = 0; i < num_threads; i++) { 

          Thread_args *args = malloc(sizeof(Thread_args));
          args->id = i;
          args->opt = 0;

          pthread_attr_t attr;
          pthread_attr_init(&attr);
          pthread_create(&workers[i], &attr, thread_func, args);
          pthread_attr_destroy(&attr);
     }
     for (int i = 0; i < num_threads; i++) 
          pthread_join(workers[i], NULL);

     //Print out all remaining nodes in Stack

     printf("Mutex: Remaining nodes %d\n", count_stack(top));

     /*free up resources properly */

     free_stack(&top);

     /* Option 2: CAS */
     pthread_t *cas_workers = malloc(num_threads * sizeof(pthread_t));

     for (int i = 0; i < num_threads; i++) { 
          
          Thread_args *args = malloc(sizeof(Thread_args));
          args->id = i;
          args->opt = 1;

          pthread_attr_t attr;
          pthread_attr_init(&attr);
          pthread_create(&cas_workers[i], &attr, thread_func, args);
          pthread_attr_destroy(&attr); 
     }
     for (int i = 0; i < num_threads; i++) 
          pthread_join(cas_workers[i], NULL);

     //Print out all remaining nodes in Stack
     printf("CAS: Remaining nodes %d\n", count_stack(atomic_load(&top_cas)));
     
     /*free up resources properly */
     free_cas_stack(&top_cas);

     pthread_mutex_destroy(&id_lock);
     pthread_mutex_destroy(&stack_lock);

     free(workers);
     free(cas_workers);


}
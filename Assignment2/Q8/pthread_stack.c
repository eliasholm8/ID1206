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

_Atomic(Node *) top_cas = NULL;

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

     if (top != NULL) {
          pthread_mutex_lock(&stack_lock);
          old_node = top;
          top = top->next;
          id = old_node->node_id;
          free(old_node);
          pthread_mutex_unlock(&stack_lock);
     }

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

     do {
          old_node = atomic_load(&top_cas);
          new_node->next = old_node; 
     } while (!atomic_compare_exchange_weak(&top_cas, &old_node, new_node));
     

}

int pop_cas() { 
     Node *old_node;
     Node *new_node;

     //update top of the stack below

     do {
          old_node = atomic_load(&top_cas);
          new_node = old_node->next;
          
     } while (!atomic_compare_exchange_weak(&top_cas, &old_node, new_node));
     
     int id = old_node->node_id;
     free(old_node);

     return id;
}

/* the thread function */
void *thread_func(int opt) { 
     /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */
     int my_id;

     if( opt==0 ){
          push_mutex();push_mutex();pop_mutex();pop_mutex();push_mutex();
     }else{
          push_cas();push_cas();pop_cas();pop_cas();push_cas();
     }
     
     printf("Thread %d: exit\n", my_id);
     pthread_exit(0);
}

int main(int argc, char *argv[])
{
     num_threads = atoi(argv[1]);

     /* Option 1: Mutex */ 
     pthread_t *workers;
     for (int i = 0; i < num_threads; i++) { 
          pthread_attr_t attr;
          pthread_attr_init(&attr);
          pthread_create(...); 
     }
     for (int i = 0; i < num_threads; i++) 
          pthread_join(...);

     //Print out all remaining nodes in Stack
     printf("Mutex: Remaining nodes \n");

     /*free up resources properly */

     /* Option 2: CAS */ 
          for (int i = 0; i < num_threads; i++) { 
          pthread_attr_t attr;
          pthread_attr_init(&attr);
          pthread_create(...); 
     }
     for (int i = 0; i < num_threads; i++) 
          pthread_join(...);

     //Print out all remaining nodes in Stack
     printf("CAS: Remaining nodes \n");
     
     /*free up resources properly */

}
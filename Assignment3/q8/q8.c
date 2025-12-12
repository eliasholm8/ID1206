#define ACCESSES 1000

#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

typedef struct Page { 
    int page_id;
    int reference_bit;
    int total_references;
    struct Page *next;
    bool is_active;
    // @other auxiliary 
} Page;

typedef struct LinkedList {
    Page *head;
    Page *tail;
    int size;
} LinkedList;

// @create an active list
LinkedList active_list = {NULL, NULL, 0};

// @create an inactive list
LinkedList inactive_list = {NULL, NULL, 0};

// Other global variables
Page *pages = NULL;
int *reference_string = NULL;

int number_of_pages = 0;
int sleep_time = 0;
int finish_state = 0;

pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

void add_to_list_tail(LinkedList *list, Page *page) {
    if (list->head == NULL) {
        list->head = page;
        list->tail = page;
    } else {
        list->tail->next = page;
        list->tail = page;
    }
    page->next = NULL;
    list->size++;

    if (list == &active_list) {
        page->is_active = true;
    } else {
        page->is_active = false;
    }
}

void add_to_list_head(LinkedList *list, Page *page) {
    if (list->head == NULL) {
        list->head = page;
        list->tail = page;
        page->next = NULL;
    } else {
        page->next = list->head;
        list->head = page;
    }
    list->size++;

    if (list == &active_list) {
        page->is_active = true;
    } else {
        page->is_active = false;
    }
}

void remove_from_list(LinkedList *list, Page *page) {
    Page *current = list->head;
    Page *previous = NULL;
    
    while (current != NULL) {
        if (current == page) {
            if (previous == NULL) {
                list->head = current->next;
                if (list->head == NULL) {
                    list->tail = NULL;
                }
            } else {
                previous->next = current->next;
                if (current->next == NULL) {
                    list->tail = previous;
                }
            }
            list->size--;
            page->next = NULL;
            return;
        }
        previous = current;
        current = current->next;
    }
}

void *player_thread_func() { 
    for (int i = 0; i < ACCESSES; i++) {
        int page_id = reference_string[i];
        Page *page = &pages[page_id];

        pthread_mutex_lock(&mutex_lock);
        
        page->reference_bit = 1;

        if (!page->is_active) {
            remove_from_list(&inactive_list, page);
            add_to_list_tail(&active_list, page);
        }
        else {
            remove_from_list(&active_list, page);
            add_to_list_tail(&active_list, page);
        }

        if (active_list.size > (int)(0.7 * number_of_pages)) {
            int pages_to_move = (int)(0.2 * number_of_pages);
            for (int j = 0; j < pages_to_move && active_list.head != NULL; j++) {
                Page *to_remove = active_list.head;
                remove_from_list(&active_list, to_remove);
                add_to_list_tail(&inactive_list, to_remove);
            }
        }

        pthread_mutex_unlock(&mutex_lock);
        usleep(10); 
    }

    pthread_mutex_lock(&mutex_lock);
    finish_state = 1;
    pthread_mutex_unlock(&mutex_lock);

    pthread_exit(0);
}


void *checker_thread_func() { 
     
    while (finish_state != 2) {
        pthread_mutex_lock(&mutex_lock);

        Page *current_page = active_list.head;
        while (current_page != NULL) {
            if (current_page->reference_bit == 1) {
                current_page->total_references += 1;
                current_page->reference_bit = 0;
            } 
            current_page = current_page->next;
        }

        if (finish_state == 1) {
            finish_state = 2;
        }

        pthread_mutex_unlock(&mutex_lock);
        usleep(sleep_time);
    }

    pthread_exit(0);
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <number_of_pages> <sleep_time>\n", argv[0]);
        return 1;
    }

    number_of_pages = atoi(argv[1]);
    sleep_time = atoi(argv[2]);

    // Initalize things.
    srand(time(NULL));

    pages = (Page *)malloc(sizeof(Page) * number_of_pages);
    for (int i = 0; i < number_of_pages; i++) {
        pages[i].page_id = i;
        pages[i].reference_bit = 0;
        pages[i].total_references = 0;
        pages[i].next = NULL;
        pages[i].is_active = false;

        if (inactive_list.head == NULL) {
            inactive_list.head = &pages[i];
            inactive_list.tail = &pages[i];
        } else {
            inactive_list.tail->next = &pages[i];
            inactive_list.tail = &pages[i];
        }
        inactive_list.size++;
    }

    reference_string = (int *)malloc(sizeof(int) * ACCESSES);
    for (int i = 0; i < ACCESSES; i++) {
        reference_string[i] = rand() % number_of_pages;
    }

    /* Create two workers */ 
    pthread_t player;   
    pthread_t checker;    

    pthread_create(&player, NULL, player_thread_func, NULL); 
    pthread_create(&checker, NULL, checker_thread_func, NULL); 
    
    pthread_join(player, NULL);
    pthread_join(checker, NULL);

    printf("Page_Id, Total_Referenced\n");
    for (int i = 0; i < number_of_pages; i++) {
        printf("%d, %d\n", pages[i].page_id, pages[i].total_references);
    }
    
    printf("Pages in active list: ");
    Page *current = active_list.head;
    while (current != NULL) {
        printf("%d ", current->page_id);
        current = current->next;
    }
    printf("\n");

    printf("Pages in inactive list: ");
    current = inactive_list.head;
    while (current != NULL) {
        printf("%d ", current->page_id);
        current = current->next;
    }
    printf("\n");

    /*free up resources properly */
    free(pages);
    free(reference_string);
}

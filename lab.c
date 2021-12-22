#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#define SUCCESS 0
#define NOT_PROCESS_SHARED 0

#define count_of_semaphores 2
#define count_of_lines 10
#define size_of_string 10
#define parent_thread 0
#define child_thread 1

typedef struct st_args_of_thread {
    sem_t *semaphores;
    char *text;
    int number_of_thread;
} args_of_thread;

int checkOfErrors (int result_of_action, char *info_about_error) {
    if (result_of_action != SUCCESS) {
        perror(info_about_error);
        return EXIT_FAILURE;
    }
    return SUCCESS;
}

int destroyOfSemaphore (sem_t *semaphores) {
    for (int i = 0; i < count_of_semaphores; i++) {
        errno = sem_destroy(&semaphores[i]);
        int result_of_destroying = checkOfErrors(errno, "Error of destroying of mutexes");
        if (result_of_destroying != SUCCESS) {
            return EXIT_FAILURE;
        }
    }
    return SUCCESS;
}

int initializeOfSemaphore (sem_t *semaphores) {
    errno = sem_init(&semaphores[parent_thread], NOT_PROCESS_SHARED, parent_thread);
    int result_of_init_of_semaphores = checkOfErrors(errno, "Error of initialization of semaphore number 1");
    if (result_of_init_of_semaphores != SUCCESS) {
        return EXIT_FAILURE;
    }

    errno = sem_init(&semaphores[child_thread], NOT_PROCESS_SHARED, child_thread);
    result_of_init_of_semaphores = checkOfErrors(errno, "Error of initialization of semaphores umber 2");
    if (result_of_init_of_semaphores != SUCCESS) {
        return EXIT_FAILURE;
    }

    return SUCCESS;
}

void *printText (args_of_thread *argumets) {
    char* text = argumets->text;
    int num_of_cur_semaphore = argumets->number_of_thread;
    int num_of_next_semaphore = (num_of_cur_semaphore + 1) % count_of_semaphores;

    for (int i = 0; i < count_of_lines; i++) {
        errno = sem_wait(&argumets->semaphores[num_of_next_semaphore]);
        int result_of_wait = checkOfErrors(errno, "Error of waiting of semaphore");
        if (result_of_wait != SUCCESS) {
            destroyOfSemaphore(argumets->semaphores);
            return (void *)EXIT_FAILURE;
        }
        printf("%s %d\n", text, i);
        errno = sem_post(&argumets->semaphores[num_of_cur_semaphore]);
        int result_of_post = checkOfErrors(errno, "Error of posting of semaphore");
        if (result_of_post != SUCCESS) {
            destroyOfSemaphore(argumets->semaphores);
            return (void *)EXIT_FAILURE;
        }
    }
    return SUCCESS;
}

int main (int  argc, char *argv[]) {
    sem_t semaphores[count_of_semaphores];
    char text_of_parent[size_of_string] = "Parent: ";
    char text_of_child[size_of_string] = "Child: ";
    pthread_t id_of_thread;

    args_of_thread args_of_parent,
            args_of_child;
    args_of_parent.text = text_of_parent;
    args_of_child.text = text_of_child;
    args_of_parent.semaphores = semaphores;
    args_of_child.semaphores = semaphores;
    args_of_parent.number_of_thread = parent_thread;
    args_of_child.number_of_thread = child_thread;

    errno = initializeOfSemaphore(semaphores);
    int result_of_init = checkOfErrors(errno, "Error of initialization of attributes of semaphores");
    if (result_of_init != SUCCESS) {
        exit(EXIT_FAILURE);
    }

    errno = pthread_create(&id_of_thread, NULL, printText, &args_of_child);
    int result_of_creating = checkOfErrors(errno, "Error of creating of thread");
    if (result_of_creating != SUCCESS) {
        destroyOfSemaphore(semaphores);
        exit(EXIT_FAILURE);
    }

    int result_of_print_parent;
    result_of_print_parent = (int)printText(&args_of_parent);

    int result_of_print_child;
    errno = pthread_join(id_of_thread, (void **)&result_of_print_child);
    int result_of_joining = checkOfErrors(errno, "Error of joining of thread");
    if (result_of_joining != SUCCESS) {
        destroyOfSemaphore(semaphores);
        exit(EXIT_FAILURE);
    }

    if (result_of_print_child != SUCCESS || result_of_print_parent != SUCCESS) {
        fprintf(stderr, "Error in printText() function");
        exit(EXIT_FAILURE);
    }

    int result_of_destroying = destroyOfSemaphore(semaphores);
    if (result_of_destroying != SUCCESS) {
        exit(EXIT_FAILURE);
    }

    return SUCCESS;
}

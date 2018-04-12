#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/shm.h>

#include "shm_com_sem.h"

int main(){
    void *shared_memory = (void *)0;
    struct shared_mem_st *shared_stuff;

    int shmid;
    int num_read;
    pid_t fork_result;
    sem_t *sem_queue, *sem_queue_empty, *sem_queue_full;
    // 获取共享内存区，并挂入内存
    shmid = shmget(IPC_PRIVATE, BUFSIZ,  IPC_CREAT);
    shared_memory = shmat(shmid, (void *)0, 0);

    // 将缓冲区指针转化为shared_mem_st类型
    shared_stuff = (struct shared_mem_st *) shared_memory;
    // 获取producer创建的3个信号量，根据名字“queue_mutex”，“queue_empty”和“queue_full”来识别
    sem_queue = sem_open(queue_mutex, O_CREAT, 0666, 1);
    sem_queue_empty = sem_open(queue_empty, O_CREAT, 0666, 3);
    sem_queue_full = sem_open(queue_full, O_CREAT, 0666, 0);

    // 创建了两个进程
    fork_result = fork();
    if (fork_result == -1){
        fprintf(stderr, "Fork failure\n");
    }
    if (fork_result == 0){ // 子进程
        while(1){
            // 信号量操作，打印消费内容及进程号，发现quit退出
            sem_wait(sem_queue_full);
            sem_wait(sem_queue);
            printf("id: %d, %s\n", getpid(), shared_stuff->buffer[shared_stuff->line_read]);
            shared_stuff->line_read++;
            sem_post(sem_queue);
            sem_post(sem_queue_empty);
        }
        // 释放信号量
        sem_close(sem_queue);
        sem_close(sem_queue_empty);
        sem_close(sem_queue_full);
    }else{
        while(1){
            // 信号量操作，打印消费内容及进程号，发现quit推出
            sem_wait(sem_queue_full);
            sem_wait(sem_queue);
            printf("id: %d, %s\n", getpid(), shared_stuff->buffer[shared_stuff->line_read]);
            shared_stuff->line_read++;
            sem_post(sem_queue);
            sem_post(sem_queue_empty);
        }
        // 释放信号量
        sem_close(sem_queue);
        sem_close(sem_queue_empty);
        sem_close(sem_queue_full);
    }
    exit(EXIT_SUCCESS);


    return 0;
}
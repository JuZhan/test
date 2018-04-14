#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/shm.h>

#include "shm_com_sem.h"

int main()
{
    void *shared_memory = (void *)0;
    struct shared_mem_st *shared_stuff;

    int shmid;
    int num_read;
    pid_t fork_result;
    sem_t *sem_queue, *sem_queue_empty, *sem_queue_full;
    // 获取共享内存区，并挂入内存
    shmid = shmget((key_t)1888, BUFSIZ, 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        perror("shmget failed");
        exit(1);
    }
    if ((shared_memory = shmat(shmid, 0, 0)) < (void *)0)
    { //若共享内存区映射到本进程的进程空间失败
        perror("shmat failed");
        exit(1);
    }
    // 将缓冲区指针转化为shared_mem_st类型
    shared_stuff = (struct shared_mem_st *)shared_memory;
    printf("ID: %d ==> MEN: %d\n", shmid, shared_memory);

    // 获取producer创建的3个信号量，根据名字“queue_mutex”，“queue_empty”和“queue_full”来识别
    sem_queue = sem_open("queue_mutex", 0);
    sem_queue_empty = sem_open("queue_empty", 0);
    sem_queue_full = sem_open("queue_full", 0);

    printf("READ: %s\n", shared_stuff->buffer[shared_stuff->line_read]);
    printf("WRITE: %s\n", shared_stuff->buffer[shared_stuff->line_write]);

    // 创建了两个进程
    fork_result = fork();
    if (fork_result == -1)
    {
        fprintf(stderr, "Fork failure\n");
    }
    if (fork_result == 0)
    { // 子进程
        while (1)
        {
            // 信号量操作，打印消费内容及进程号，发现quit退出
            sem_wait(sem_queue_full);
            sem_wait(sem_queue);
            printf("id: %d, %s\n", getpid(), shared_stuff->buffer[shared_stuff->line_read]);
            // 如果键入“quit”则推出
            if (strcmp(shared_stuff->buffer[shared_stuff->line_read], "quit") == 0)
            {
                break;
            }
            shared_stuff->line_read = (shared_stuff->line_read + 1) % NUM_LINE;
            sem_post(sem_queue);
            sem_post(sem_queue_empty);
        }
        // 释放信号量
        sem_unlink(sem_queue);
        sem_unlink(sem_queue_empty);
        sem_unlink(sem_queue_full);
    }
    else
    {
        while (1)
        {
            // 信号量操作，打印消费内容及进程号，发现quit推出
            sem_wait(sem_queue_full);
            sem_wait(sem_queue);
            printf("%d\n", shared_stuff->line_read);
            printf("id: %d, %s\n", getpid(), shared_stuff->buffer[shared_stuff->line_read]);
            // 如果键入“quit”则推出
            if (strcmp(shared_stuff->buffer[shared_stuff->line_read], "quit") == 0)
            {
                break;
            }
            shared_stuff->line_read = (shared_stuff->line_read + 1) % NUM_LINE;
            sem_post(sem_queue);
            sem_post(sem_queue_empty);
        }
        // 释放信号量
        sem_unlink(sem_queue);
        sem_unlink(sem_queue_empty);
        sem_unlink(sem_queue_full);

        waitpid(fork_result, NULL, 0); // 等待子进程结束
    }
    printf("Customer end");
    exit(EXIT_SUCCESS);

    return 0;
}
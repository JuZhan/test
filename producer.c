#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>

#include "shm_com_sem.h"

int main()
{
    void *shared_memory = (void *)0; // 共享内存（缓冲区指针）
    struct shared_mem_st *shared_stuff;
    // 将无类型共享存储去转换为shared_men_st类型的指针
    char key_line[256];

    int shmid;

    sem_t *sem_queue, *sem_queue_empty, *sem_queue_full;
    //  访问共享内存的互斥量、空缓冲区、满缓冲区信号量。皆为信号量指针

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
    shared_stuff = (struct shared_mem_st *)shared_memory;
    printf("ID: %d ==> MEN: %d\n", shmid, shared_memory);

    // 将缓冲区指针转换为shared_mem_st类型

    // 下面创建3个信号量
    sem_queue = sem_open(queue_mutex, O_CREAT, 0644, 1);
    sem_queue_empty = sem_open(queue_empty, O_CREAT, 0644, 1);
    sem_queue_full = sem_open(queue_full, O_CREAT, 0644, 0);

    // 读写指针初始化，开始时都指向第0行
    shared_stuff->line_write = 0;
    shared_stuff->line_read = 0;
    // 不断从控制台读入按键输入的字符行
    // Read and put input into buffer
    while (1)
    {
        // 指示可以输入，并用gets()读入按键行到key_line中
        printf("Enter your text('quit' for exit): ");
        gets(key_line);

        // 将输入的行写入缓冲区，要有信号量操作
        sem_wait(sem_queue_empty);
        sem_wait(sem_queue);
        strcpy(shared_stuff->buffer[shared_stuff->line_write], key_line);
        printf("%s\n", shared_stuff->buffer[shared_stuff->line_write]);
        // 如果键入“quit”则推出
        if (strcmp(shared_stuff->buffer[shared_stuff->line_write], "quit") == 0)
        {
            break;
        }
        shared_stuff->line_write = (shared_stuff->line_write + 1) % NUM_LINE;
        sem_post(sem_queue);
        sem_post(sem_queue_full);
    }
    sem_post(sem_queue);
    sem_post(sem_queue_full);

    // 因键入“quit”从前面while()循环中挑出到此处，程序退出前，释放信号量
    sem_unlink(sem_queue);
    sem_unlink(sem_queue_empty);
    sem_unlink(sem_queue_full);

    if (shmdt(shared_memory) == -1) //解除映射
    {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    //删除共享内存区
    if (shmctl(shmid, IPC_RMID, 0) == -1)
    {
        fprintf(stderr, "shmctl failed\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

void p(int semid) {
    struct sembuf pbuf;
    pbuf.sem_num = 0;
    pbuf.sem_op = -1;
    pbuf.sem_flg = SEM_UNDO;
    if ((semop(semid, &pbuf, 1)) == -1) {
        perror("p : semop failed");
        exit(1);
    }
}

void v(int semid) {
    struct sembuf vbuf;
    vbuf.sem_num = 0;
    vbuf.sem_op = 1;
    vbuf.sem_flg = SEM_UNDO;
    if ((semop(semid, &vbuf, 1)) == -1) {
        perror("v : semop failed");
        exit(1);
    }
}

int initsem(key_t skey)
{
    int status = 0, semid;
    if ((semid = semget(skey, 1, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
        // if EXIST, just open
        if (errno == EEXIST){
            semid = semget(skey, 1, 0);
        } 
    } else{
        // if create new, set status = 1
        status = semctl(semid, 0, SETVAL, 1);
    }
    if (semid == -1 || status == -1) {
        printf("initsem is failed\n");
        exit(1);
    } 
    return semid;
}
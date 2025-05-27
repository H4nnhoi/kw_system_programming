#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>


///////////////////////////////////////////////////////////////////////////
// p                                                                     //
///////////////////////////////////////////////////////////////////////////
// Input:                                                                //
//   - int semid : Semaphore ID                                          //
// Output:                                                               //
//   - None                                                              //
// Purpose:                                                              //
//   - Performs the "P" operation (wait) on the semaphore                //
//   - Decrements the semaphore value by 1                               //
//   - If the value is already 0, the calling process is blocked         //
///////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////
// v                                                                     //
///////////////////////////////////////////////////////////////////////////
// Input:                                                                //
//   - int semid : Semaphore ID                                          //
// Output:                                                               //
//   - None                                                              //
// Purpose:                                                              //
//   - Performs the "V" operation (signal) on the semaphore              //
//   - Increments the semaphore value by 1                               //
//   - Wakes up a blocked process if any are waiting                     //
///////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////
// initsem                                                               //
///////////////////////////////////////////////////////////////////////////
// Input:                                                                //
//   - key_t skey : Unique key to identify the semaphore set             //
// Output:                                                               //
//   - int       : Semaphore ID                                          //
// Purpose:                                                              //
//   - Initializes or accesses a semaphore set with the given key        //
//   - If the semaphore already exists (EEXIST), opens it                //
//   - If it doesn't exist, creates it and sets the initial value to 1   //
//   - Ensures a single binary semaphore is shared across processes      //
///////////////////////////////////////////////////////////////////////////
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
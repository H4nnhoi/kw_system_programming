#ifndef SEMAPHORE_UTILS_H
#define SEMAPHORE_UTILS_H

void p(int semid);
void v(int semid);
int initsem(key_t skey);

#endif
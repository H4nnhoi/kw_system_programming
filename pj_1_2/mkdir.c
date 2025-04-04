#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

mode_t old_umask;

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("error\n");
        return 1;
    }
    old_umask = umask(0);
    mkdir(argv[1], S_IRWXU | S_IRWXG | S_IRWXO);
    return 0;
}

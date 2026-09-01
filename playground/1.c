#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(){
    pid_t pid = fork(); //Como fork() duplica el proceso y retorna el PiD, se guarda en esta variable para poder saber cual proceso es cual
    
    if(pid < 0){
        perror("fork"); //PiDs menores a 0 indican que hubo algún error
        return 1;
    }
    else if(pid == 0){
        printf("Child process, PiD = %d\nMy Parent process PiD = %d\n", getpid(), getppid());
    }
    else if(pid > 0){
        printf("Parent process, PiD = %d\n", getpid());
    }
    
    return 0;


}

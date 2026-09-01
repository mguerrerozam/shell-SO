#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(){
    pid_t pid = fork();

    char *args[] = {"ls", "-l", NULL};

    if(pid < 0){
        perror("PiD error");
        return 1;
    }
    else if(pid > 0){
        waitpid(pid,NULL,0);
        printf("Done with child's task, returning to parent\n");
    }
    else if(pid == 0){
        execvp(args[0],args);
    }
}
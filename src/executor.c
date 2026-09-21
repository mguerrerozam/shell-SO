#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <string.h>


#include "executor.h"

int ejecutar_comando(char **args) {
    
    if (args == NULL || args[0] == NULL) {
        return 1; 
    }

   
    pid_t pid = fork();

    if (pid < 0) {
        perror("Error en fork");
        return -1;
    }

    if (pid == 0) {
       
        struct sigaction sa;
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, NULL);

        
        execvp(args[0], args);

        fprintf(stderr, "miShell: %s: %s\n", args[0], strerror(errno));
        _exit(127); 
    } 
    else {
       
        int status;
        
        
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            return -1;
        }

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
    }

    return 0;
}
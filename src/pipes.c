#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#include "../include/pipes.h"
#include "../include/executor.h"
#include "../include/redirections.h"
#include "../include/signals.h"

int iniciar_pipes(char ***comandos, int num_comandos) {
    // Si no hay comandos exito
    if (num_comandos <= 0) return 0;
    // Si hay solo uno no necesita pipe
    if (num_comandos == 1) {
        return ejecutar_comando(comandos[0]);
    }

    // Para n comandos necesitamos n - 1 pipes
    int num_pipes = num_comandos - 1;
    int pipes[comandos_maximos][2]; // Matriz con cada fila un extremo [lectura, escritura]
    pid_t pids[comandos_maximos]; // Matriz para guardar el identificador del proceso (PID)

    // se crean todos los pipes necesarios
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("Error al crear el pipe");
            return -1;
        }
    }


    // Por cada comando de la pipeline se crea un proceso hijo
    for (int i = 0; i < num_comandos; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("Error en fork");
            return -1;
        }

        if (pids[i] == 0) {
            
            
            // Si no es el primer comando, su entrada debe ser la lectura[0] del pipe anterior
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            // Si no es el ultimo comando, su salida debe ir en la escritura[1] del pipe actual
            if (i < num_pipes) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Se van cerrando los extremos de los pipes que ya no se utilizan
            for (int j = 0; j < num_pipes; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            restaurar_senales_hijo();          
            procesar_redirecciones(comandos[i]);

            // Se reemplaza el proceso actual por el comando real
            execvp(comandos[i][0], comandos[i]);
            
            // Ocurrió un error
            fprintf(stderr, "miShell: %s: %s\n", comandos[i][0], strerror(errno));
            _exit(127);
        }
    }

    // La shell, es decir el padre, cierra todos los pipes
    for (int i = 0; i < num_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }


    int status = 0;
    int status_ultimo = 0;

    // Se espera a que cada uno de los hijos termine
    for (int i = 0; i < num_comandos; i++) {
        waitpid(pids[i], &status, 0);
        
        // El pipeline retorna el codigo del último comando asi que se revisa 
        if (i == num_comandos - 1) {
            // Si el comando termino de forma normal se guarda su salida
            if (WIFEXITED(status)) {
                status_ultimo = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                // Si fue porque intervino una señal entonces se calcula su codigo
                status_ultimo = 128 + WTERMSIG(status);
            }
        }
    }

    // Le damos el resultado final a la shell
    return status_ultimo;
}
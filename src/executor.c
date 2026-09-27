#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "../include/executor.h"
#include "../include/redirections.h"
#include "../include/signals.h"

int crear_proceso(char **args) {
    // Validación de comando vacío
    if (args == NULL || args[0] == NULL) {
        return 1; 
    }

   // Se crea el proceso hijo, clonando la shell
    pid_t pid = fork();

    // Si retorna un número menor a 0 es porque no se pudo realizar el fork()
    if (pid < 0) {
        perror("Error en fork");
        return -1;
    }
    
    // Proceso del hijo
    if (pid == 0) {
        
        // Se separa al hijo en su propio grupo de procesos para evitar problemas con las señales de teclado
        setpgid(0, 0);

        // La shell ignora Ctrl + C por ende se llama a las señales para que el hijo si responda
        restaurar_senales_hijo();
        
        // Revisa si el comando tiene operadores como <, > o >> 
        procesar_redirecciones(args);

        // Se destruye el proceso actual, es decir el hijo y se reemplaza por el programa real
        execvp(args[0], args);

        // Se ejecuta si execvp() falló e impreme un mensaje de error
        fprintf(stderr, "miShell: %s: %s\n", args[0], strerror(errno));

        // El hijo muere 
        _exit(127); // 127 en linux es comando no encontrado

    } 
    
    // El padre retorna el identificador del proceso del hijo que clono
    return pid;
}

int ejecutar_comando(char **args) {

    // Se llama a la función para crear al hijo
    pid_t pid = crear_proceso(args);
    
    // Por si hubo un error
    if (pid < 0) {
        return -1;
    }
    
    // El padre, es decir la shell, espera a que el hijo termine de ejecutar el comando y recibe el resultado
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return -1;
    }

    // Se revisa como termino el proceso
    if (WIFEXITED(status)) {
        // Si termino de forma natural entonces se retorna el codigo entregado
        return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        // Termino por una señal externa
        fprintf(stderr, "\n");
        return 128 + WTERMSIG(status);
    }
    
    return 0; // Si todo sale bien finaliza el programa
 }
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
#include "../include/background.h"

int crear_proceso(char **args, int background) {
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
        
        // Si esta en segundo plano, se separa al hijo en su propio grupo de procesos para evitar problemas con las señales de teclado
        if (background) setpgid(0, 0);

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

    //Comprobación de plano del proceso (primero o segundo)
    //Se comprueba si el comando terminaba con "&", para marcarlo como proceso que irá a background además de quitar el carácter del arreglo de tokens
    int es_background = 0; 

    if (args != NULL){
        int i = 0;
        while(args[i] != NULL){ //Se avanza hasta encontrar el marcador de fin
            i++;
        }
    
        //Si hay al menos un token y el ultimo es "&", se trata de background
        if(i > 0 && strcmp(args[i - 1], "&") == 0){ //Se compara el ultimo token real
            es_background = 1; //Se marca que el comando es background
            args[i - 1] = NULL; //Se elimina el "&" para que execvp no lo reciba
        }
    }

    // Se llama a la función para crear al hijo
    pid_t pid = crear_proceso(args, es_background);
    
    // Por si hubo un error
    if (pid < 0) {
        return -1;
    }

    if (es_background){
        //Al entrar al condicional se reconstruyen los tokens del comando en texto para pasárselo a la función de agregar jobs
        char comando[256]; //Buffer donde se va a armar el texto completo
        comando[0] = '\0'; //Se deja vacio a proposito, strcat necesita saber donde empieza a escribir

        for(int j = 0; args[j] != NULL; j++){ //Se recorre cada palabra del comando
            strcat(comando, args[j]); //Se agrega la palabra actual al final del buffer
            if(args[j + 1] != NULL){ //Si todavia queda otra palabra despues se agrega un espacio antes de la siguiente
                strcat(comando, " "); 
             }
        }

        background_agregar_job(pid, comando); //Se agrega el proceso al arreglo de jobs 

        return 0;
    }

    else{
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
}
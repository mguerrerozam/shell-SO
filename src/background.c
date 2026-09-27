#include "../include/background.h"
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>

typedef enum{ //Estados posibles para un job
    EJECUTANDO,
    TERMINADO
} estadoJob;

typedef struct { //Struct de job para almacenar en su array
    pid_t pid;
    char comando[256];
    estadoJob estado;
} job;

job tabla_jobs[64]; //Arreglo que contendrá los jobs en EJECTUANDO

void manejador_sigchld(int senal){ //Función que contiene lo que se hará al recibir la señal
    (void)senal; //Para evitar warning
}

void background_iniciar() { //Función para inicializar el manejo de procesos en bg, se llama al iniciar la shell
    struct sigaction accion; 
    accion.sa_handler = manejador_sigchld;  //Indica que el manejador va a ser la función declarada arriba
    sigemptyset(&accion.sa_mask); 
    accion.sa_flags = 0;
    sigaction(SIGCHLD, &accion, NULL);
}
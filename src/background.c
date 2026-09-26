#include "../include/background.h"
#include <signal.h>
#include <stdio.h>

typedef enum{ //Estados posibles para un job
    EJECUTANDO,
    TERMINADO
} estadoJob;

typedef struct { //struct de job
    int pid;
    char comando[256];
    estadoJob estado;
} job;

void manejador_sigchld(int senal){ //Función que contiene lo que se hará al recibir la señal
}

void background_iniciar() { //Función para inicializar el manejo de procesos en bg, se llama al iniciar la shell
    struct sigaction accion; 
    accion.sa_handler = manejador_sigchld;  //Indica que el manejador va a ser la función declarada arriba
    sigemptyset(&accion.sa_mask); 
    accion.sa_flags = 0;
    sigaction(SIGCHLD, &accion, NULL);
}
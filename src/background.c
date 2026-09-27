#include "../include/background.h"
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#define MAX_JOBS 64 //Máximo de jobs que cabrán en el arreglo de jobs

typedef enum{ //Estados posibles para un job
    LIBRE, //Se distingue de "TERMINADO" porque el estado libre se da una vez que ya se avisó al usuario que el proceso finalizó con éxito
    EJECUTANDO,
    TERMINADO
} estadoJob;

typedef struct { //Struct de job para almacenar en su array
    pid_t pid;
    char comando[256];
    estadoJob estado;
} job;

job tabla_jobs[MAX_JOBS]; //Arreglo que contendrá los jobs en EJECTUANDO

void manejador_sigchld(int senal){ //Todo: Función que contiene lo que se hará al recibir la señal
    (void)senal; //Temporal para evitar warning
}

void background_iniciar() { //Función para inicializar el manejo de procesos en bg, se llama al iniciar la shell
    
    //Inicialización de manejo de señal sigchld
    struct sigaction accion; 
    accion.sa_handler = manejador_sigchld;  //Indica que el manejador va a ser la función declarada arriba
    sigemptyset(&accion.sa_mask); 
    accion.sa_flags = 0;
    sigaction(SIGCHLD, &accion, NULL);

    //Inicialización de arreglo de jobs
    for(int i = 0; i < MAX_JOBS; i++){
        tabla_jobs[i].estado = LIBRE; //Se deja en libre para facilitar la verificación inicial en la función de agregar jobs al array
    }
}

void background_agregar_job(pid_t pid, char comando[256]) { //Función que recibe PiD y texto al momento de ejecutar un comando, los añade al arreglo de procesos en background

    for(int i = 0; i < MAX_JOBS; i++){ //Busca la primera posición con un job en estado "LIBRE" para poder escribir cada dato sobre él
        if (tabla_jobs[i].estado == LIBRE){
            tabla_jobs[i].pid = pid;
            strcopy(tabla_jobs[i].comando, comando);
            tabla_jobs[i].estado = EJECUTANDO;
        }
    }
}
#include "../include/background.h"
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#define MAX_JOBS 64 //Máximo de jobs que cabrán en el arreglo de jobs

job tabla_jobs[MAX_JOBS]; //Arreglo que contendrá los jobs en EJECTUANDO

int cantidad_jobs = 0;

void manejador_sigchld(int senal){ //Función que contiene todo lo que se hará al recibir la señal, con el propósito de evitar procesos zombie
    (void)senal; //Para evitar warning puesto que la variable no se usa
    int status; //Espacio para el detalle de como termino cada hijo
    pid_t pid_terminado; //Guarda el pid que se va recogiendo en cada vuelta
    pid_terminado = waitpid(-1, &status, WNOHANG); //Se asgina el pid del hijo que acaba de terminar, wnohang para evitar que el padre quede congelado
    while(pid_terminado > 0){ //Condición para asegurar que lo de abajo se ejecute al haber capturado un hijo válido y sin errores
        for(int i = 0; i < 64; i++){ //Se busca ese pid dentro de la tabla
            if(tabla_jobs[i].estado == EJECUTANDO && tabla_jobs[i].pid == pid_terminado){
                tabla_jobs[i].estado = TERMINADO; //Se marca como terminado, pendiente de avisar
                break;
            }
        }
        pid_terminado = waitpid(-1, &status, WNOHANG);
    }
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
            tabla_jobs[i].pid = pid; //Asigna pid
            strcpy(tabla_jobs[i].comando, comando); //Asigna comando
            tabla_jobs[i].lectura_valida = 0; //Evita que un job nuevo herede datos de CPU de quien usó esa posición antes
            tabla_jobs[i].estado = EJECUTANDO; //Cambia el estado para que en una próxima iteración no usen su lugar
            cantidad_jobs++; //Aumenta número de jobs activos
            printf("[%d] %d\n", i + 1, pid); //Se avisa de inmediato el numero de job y el pid
            break; //Como ya se encontró lugar no es necesario seguir buscando
        }
    }
}

void background_avisar_terminados(){ //Función para actualizar estado de los jobs a "LIBRE" una vez ya se haya avisado de que terminó con éxito
    for(int i = 0; i < 64; i++){ //Se recorre toda la tabla
        if(tabla_jobs[i].estado == TERMINADO){ //Se encontro uno pendiente de avisar
            printf("[%d]+ Finalizado %s\n", i + 1, tabla_jobs[i].comando); //Se muestra el aviso
            tabla_jobs[i].estado = LIBRE; //Se libera la posicion para un job nuevo
            cantidad_jobs--; //Se resta un job activo
        }
    }
}

void background_listar_jobs(){ //Función para listar jobs activos
    for(int i = 0; i < 64; i++){ //Se recorre toda la tabla
        if(tabla_jobs[i].estado == EJECUTANDO){ //Solo se listan los que siguen corriendo
            printf("[%d] Ejecutando %s\n", i + 1, tabla_jobs[i].comando);
        }
    }
}
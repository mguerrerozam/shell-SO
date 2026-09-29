#ifndef BACKGROUND_H
#define BACKGROUND_H
#include <sys/types.h>
#include <time.h>
typedef enum{ //Estados posibles para un job
    LIBRE, //Se distingue de "TERMINADO" porque el estado libre se da una vez que ya se avisó al usuario que el proceso finalizó con éxito
    EJECUTANDO,
    TERMINADO
} estadoJob;

typedef struct { //Struct de job para almacenar en su array
    pid_t pid;
    char comando[256];
    estadoJob estado;
    unsigned long utime_anterior; //Ultima lectura de cpu en modo usuario, la usa pmon
    unsigned long stime_anterior; //Ultima lectura de cpu en modo sistema, la usa pmon
    time_t tiempo_anterior; //Momento de la ultima lectura, la usa pmon
    int lectura_valida; //Indica si ya existe una lectura anterior para calcular %CPU
} job;

extern job tabla_jobs[64];
extern int cantidad_jobs;
void background_iniciar();
void background_agregar_job(pid_t pid, char comando[256]);
void background_avisar_terminados();
void background_listar_jobs();
#endif

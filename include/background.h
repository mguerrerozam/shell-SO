#ifndef BACKGROUND_H
#define BACKGROUND_H
#include <sys/types.h>
void background_iniciar();
void background_agregar_job(pid_t pid, char comando[256]);
void background_avisar_terminados();
void background_listar_jobs();
#endif

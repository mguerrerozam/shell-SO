#ifndef BUILTINS_H
#define BUILTINS_H

#include "background.h"
#include "pmon.h"
//Devuelve 1 si el comando es un builtin, 0 si no
int es_builtin(const char *cmd);

//Ejecuta el builtin correspondiente, devuelve 0 si fue exitoso
int ejecutar_builtin(char **tokens);

#endif
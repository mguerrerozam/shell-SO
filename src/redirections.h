
#ifndef SHELL_SO_REDIRECTIONS_H
#define SHELL_SO_REDIRECTIONS_H
//bibliotecas necesarias
#include <fcntl.h>   //para open() y flags
#include <unistd.h>  //para dup2() y close()
#include <string.h>  //para strcmp()
#include <stdlib.h>
#include <stdio.h>

//funcion que procesa el arreglo de tokens
void procesar_redirecciones(char **args);
#endif //SHELL_SO_REDIRECTIONS_H

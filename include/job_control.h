//
// Created by pancho on 26-09-26.
//

#ifndef SHELL_SO_JOB_CONTROL_H
#define SHELL_SO_JOB_CONTROL_H

// bibliotecas necesarias para señales y control de terminal
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>

// funcion para traer un proceso pausado al primer plano (foreground)
void ejecutar_fg(pid_t pid_hijo);

// funcion para reanudar un proceso pausado en el fondo (background)
void ejecutar_bg(pid_t pid_hijo);

#endif //SHELL_SO_JOB_CONTROL_H

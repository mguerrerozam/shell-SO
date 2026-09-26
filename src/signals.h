//
// Created by pancho on 26-09-26.
//

#ifndef SHELL_SO_SIGNALS_H
#define SHELL_SO_SIGNALS_H

// bibliotecas necesarias para el manejo de señales
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

// funcion para blindar el proceso padre (la shell)
void configurar_senales_shell(void);

// funcion para quitar el blindaje a los procesos hijos
void restaurar_senales_hijo(void);


#endif //SHELL_SO_SIGNALS_H

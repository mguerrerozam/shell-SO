//
// Created by pancho on 26-09-26.
//
#include "job_control.h"

void ejecutar_fg(pid_t pid_hijo) {
    int status;

    // 1. entregar el control de la terminal (teclado y pantalla) al grupo del proceso hijo
    // STDIN_FILENO (0) representa la terminal actual
    tcsetpgrp(STDIN_FILENO, pid_hijo);

    // 2. enviar la señal sigcont para despertar al proceso pausado
    // el signo negativo (-pid) asegura que la señal vaya a todo el grupo (sirve si hay pipes encadenados)
    kill(-pid_hijo, SIGCONT);

    // 3. esperar a que el proceso termine o se vuelva a pausar con ctrl+z
    // la flag wuntraced es obligatoria para que waitpid detecte si el proceso se detiene nuevamente
    waitpid(pid_hijo, &status, WUNTRACED);

    // 4. el hijo termino o se pauso, la shell debe recuperar el control de la terminal
    // getpgrp() obtiene el id de grupo de la shell padre
    tcsetpgrp(STDIN_FILENO, getpgrp());
}

void ejecutar_bg(pid_t pid_hijo) {
    // 1. enviar la señal sigcont para despertar al proceso
    // al no usar tcsetpgrp, el proceso corre en las sombras y la shell conserva el teclado
    kill(-pid_hijo, SIGCONT);

    // imprimir el aviso estandar de que el proceso continuo en background
    printf("[%d] continuó en background\n", pid_hijo);
}
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include "../include/parser.h"
#include "../include/builtins.h"
#include "../include/executor.h"
#include "../include/pipes.h"
#include "../include/signals.h"
#include "../include/background.h"


//Se divide el array de tokens en subcomandos separados por "|".
static int separar_por_pipes(char **tokens, char **comandos[], int max) {
    int num = 0;
    comandos[num] = tokens;
    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "|") == 0) {
            tokens[i] = NULL;
            num++;
            if (num > max) break;
            comandos[num] = &tokens[i+1];
        }
    }
    return num + 1;
}



static void construir_promt(char *buf, size_t tam) {
    //Se intenta obtener la variable de entorno PWD
    char *pwd = getenv("PWD");
    if (!pwd) {
        //Si falla, consultamos directamente al sistema operativo con getcwd.
        char cwd[1024];
        pwd = getcwd(cwd, sizeof(cwd)) ? cwd : "?";
    }
    //Se escribe el promt formateado con el buffer proporcionado.
    snprintf(buf, tam, "miShell:%s$ ", pwd);
}

int main(void) {
    configurar_senales_shell();

    background_iniciar();

    char prompt[1024];
    char *linea;

    while (1) {
        //Se genera el texto del promt con la ruta actual de ejecucion.
        construir_promt(prompt, sizeof(prompt));
        background_avisar_terminados();

        //Se lee la entrada del usuario, Si retorna NULL, el usuario presiono Ctrl+D
        linea = leer_linea(prompt);
        if (!linea) {
            printf("\n");
            break;
        }
        //Se separa el string de entrada en tokens
        char **tokens = tokenizar_linea(linea);
        free(linea);//Se libera el string original.

        //Si el usuario presiona Enter o puso puros espacios, se vuelve a empezar
        if (!tokens || !tokens[0]) {
            liberar_tokens(tokens);
            continue;
        }

        //Detecta si el último token es '&'
        int background = 0;
        int n = 0;
        while (tokens[n] != NULL) n++; //Contamos cuantos tokens hay

        if (n > 0 && strcmp(tokens[n - 1], "&") == 0) {
            background = 1;
            tokens[n - 1] = NULL; // Eliminamos el '&' para que execvp no intente ejecutarlo como argumento
            n--;
        }

        // Construye un string para guardar el comando en jobs
        char cmd_str[256] = "";
        for (int i = 0; tokens[i] != NULL; i++) {
            if (i > 0) strncat(cmd_str, " ", sizeof(cmd_str) - strlen(cmd_str) - 1);
            strncat(cmd_str, tokens[i], sizeof(cmd_str) - strlen(cmd_str) - 1);
        }
        //Se ejecutan los built-in directamente en el proceso principal.
        if (es_builtin(tokens[0])) {
            ejecutar_builtin(tokens);
            liberar_tokens(tokens);
            continue;
        }

        //Se agrupan los tokens en subcomandos en caso de haber pipes (|).
        char **comandos[50];
        int num_comandos = separar_por_pipes(tokens, comandos, 50);

        if (!background) {
            //Foreground, la shell gestiona la ejecucion y se bloquea esperando (wait).
            if (num_comandos == 1)
                ejecutar_comando(comandos[0]);//Ejecucion simple
            else
                iniciar_pipes(comandos, num_comandos);//Ejecucion con redireccion IPC
        } else {
    
    // Comando simple
    if (num_comandos == 1) {
        pid_t pid = crear_proceso(comandos[0], 1); // Se crea el proceso hijo
        if (pid > 0) {
            setpgid(pid, pid); 
            background_agregar_job(pid, cmd_str); // Se añade el proceso a jobs
        }
    // Pipelines
    } else {
        pid_t pid = fork(); // proceso hijo para gestionar las pipes en segundo plano
        if (pid < 0) {
            perror("fork");
        } else if (pid == 0) {
            restaurar_senales_hijo();
            setpgid(0, 0);
            iniciar_pipes(comandos, num_comandos);
            exit(0); // termino con exito
        } else {
            background_agregar_job(pid, cmd_str); // se registra el resultado del proceso hijo
        }
    }
}
        //Limpiamos la memoria de los tokens de esta iteracion antes de la siguiente.
        liberar_tokens(tokens);
    }
    return 0;
}
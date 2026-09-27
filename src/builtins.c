#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/builtins.h"

static int builtin_cd(char **tokens) {
    char *destino;
    //Si no se proporciona un directorio destino, se asume el directorio HOME del usuario
    if (tokens[1] == NULL) {
         destino = getenv("HOME");
        if (!destino) {
            fprintf(stderr, "env var HOME not set\n");
            return 1;
        }
    } else {
        //De lo contrario, se utiliza la ruta proporcionada por el usuario.
        destino = tokens[1];
    }
    //Intenta cambiar al directorio destino mediante la llamada al sistema.
    if (chdir(destino) != 0) {
        perror("cd");//Imprime el error especifico si el cambio falla.
        return 1;
    }
    //Actualiza la variable de entorno PWD para que refleje el nuevo directorio de trabajo
    char buf[4096];
    if (getcwd(buf, sizeof(buf)) != NULL)
        setenv("PWD", buf, 1);

    return 0;
}

static int builtin_exit(char **tokens) {
    int codigo = 0;
    //Si el usuario pasa un argumento, se convierte a entero para usarlo como codigo de retorno
    if (tokens[1] != NULL) {
        codigo = atoi(tokens[1]);
    }
    exit(codigo);//Termina la shell con el codigo especificado (0 por defecto)
}
static int builtin_jobs(char **tokens) {
    (void)tokens;
    background_listar_jobs();
    return 0;
}

int es_builtin(const char *cmd)
{
    if (!cmd) return 0;
    //Compara el comando con la lista de built-ins soportados por la shell
    return (strcmp(cmd, "cd")   == 0 ||
            strcmp(cmd, "exit") == 0 ||
            strcmp(cmd, "jobs") == 0 ||  //Implementa Maxicin
            strcmp(cmd, "pmon") == 0);   //Implementa Maxicin
}

int ejecutar_builtin(char **tokens) {
    //Validación de seguridad para evitar segment fault si los tokens vienen vacios
    if (!tokens || !tokens[0]) return 1;
    //Se ejecuta la funcion segun se especifique el comando.
    if (strcmp(tokens[0], "cd") == 0)
        return builtin_cd(tokens);
    if (strcmp(tokens[0], "exit") == 0)
        return builtin_exit(tokens);
    if (strcmp(tokens[0], "jobs") == 0)
        return builtin_jobs(tokens);
    if (strcmp(tokens[0], "pmon") == 0)
        return ejecutar_pmon(tokens); //ToDo
    return 1;
}
#include "../include/redirections.h"

void procesar_redirecciones(char **args) {
    int i = 0;
    int fd;

    // recorre el arreglo hasta encontrar el fin de los argumentos
    while (args[i] != NULL) {

        // 1. redireccion de salida truncada (>)
        if (strcmp(args[i], ">") == 0) {

            // validacion de seguridad: si no hay archivo despues del simbolo
            if (args[i+1] == NULL) {
                fprintf(stderr, "Error sintáctico: falta archivo tras '>'\n");
                exit(1);
            }

            // se abre el archivo para escribir, se crea si no existe; si existe, se trunca
            // la flag O_WRONLY indica solo escritura; O_CREAT para crear el archivo si no existe; O_TRUNC para truncar
            // 0644 define los permisos rw-r--r-- (lectura y escritura; lectura; lectura)
            fd = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("Error abriendo archivo para redirección de salida");
                exit(1);
            }

            dup2(fd, STDOUT_FILENO); // se redirige stdout (descriptor 1) hacia el archivo
            close(fd);               // se limpia el descriptor original

            // eliminar el simbolo y el archivo del arreglo desplazando todo 2 posiciones a la izquierda
            int j = i;
            while (args[j+2] != NULL) {
                args[j] = args[j+2];
                j++;
            }
            args[j] = NULL; // marca el nuevo final del arreglo

            // no se hace i++ aqui porque el nuevo args[i] ahora contiene el argumento que estaba 2 posiciones mas adelante y se debe evaluarlo.
        }
        // 2. redireccion de salida en modo append (>>) (se escribe al final del archivo)
        else if (strcmp(args[i], ">>") == 0) {

            if (args[i+1] == NULL) {
                fprintf(stderr, "Error sintáctico: falta archivo tras '>>'\n");
                exit(1);
            }

            // se abre el archivo en modo append (O_APPEND protege los datos anteriores)
            fd = open(args[i+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd < 0) {
                perror("Error abriendo archivo para append");
                exit(1);
            }

            dup2(fd, STDOUT_FILENO); // se redirige stdout (descriptor 1)
            close(fd);

            // desplazar argumentos para ocultar la redireccion a execvp
            int j = i;
            while (args[j+2] != NULL) {
                args[j] = args[j+2];
                j++;
            }
            args[j] = NULL;
        }
        // 3. redireccion de entrada (<)
        else if (strcmp(args[i], "<") == 0) {

            if (args[i+1] == NULL) {
                fprintf(stderr, "Error sintáctico: falta archivo tras '<'\n");
                exit(1);
            }

            // se abre el archivo solo para lectura
            fd = open(args[i+1], O_RDONLY);
            if (fd < 0) {
                perror("Error abriendo archivo para redirección de entrada");
                exit(1);
            }

            dup2(fd, STDIN_FILENO);  // redirigir stdin (descriptor 0) hacia el archivo
            close(fd);

            // desplazar argumentos para ocultar la redireccion a execvp
            int j = i;
            while (args[j+2] != NULL) {
                args[j] = args[j+2];
                j++;
            }
            args[j] = NULL;
        }
        else {
            // si la palabra actual no es un operador, avanza a la siguiente
            i++;
        }
    }
}
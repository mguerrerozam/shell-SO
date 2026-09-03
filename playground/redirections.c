#include "redirections.h"

void procesar_redirecciones(char **args) {
    int i=0;
    int fd;

    //recorre el arreglo hasta encontrar el fin de los argumentos
    while (args[i] != NULL) {

        // 1/ redireccion de salida truncada (>)
        if (strcmp(args[i], ">") == 0) {

            //se abre el archivo para escribir, se crea si no existe; si existe, se trunca(se vacia y se escribe en blanco)
            //la flag wronly indica solo escritura; creat para crear el archivo si no existe; trunc para truncar el archivo si ya existe
            //0644 define los permisos rw-r--r-- (lectura y escritura; lectura; lectura)

            fd = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("Error abriendo archivo para redirección de salida");
                exit(1);
            }

            dup2(fd, 1); //se redirige stdout (descriptor 1) hacia el archivo
            close(fd); //se limpia el descriptor original

            //se corta el comando aqui para que execvp no lea el simbolo
            args[i] = NULL;
            break; //se asume una redireccion a la vez para simplificar

        }
        // 2. redireccion de salida en modo append (>>) (se escribe al final del archivo,)

        else if (strcmp(args[i], ">>") == 0) {


            // se abre el archivo en modo append
            fd = open(args[i+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd < 0) {
                perror("Error abriendo archivo para append");
                exit(1);
            }
            //redirigir stdout (descriptor 1)
            dup2(fd, 1);
            close(fd);

            args[i] = NULL;
            break;
        }
        // 3. Redirección de entrada (<)

        else if (strcmp(args[i], "<") == 0) {
            //se abre el archivo solo para lectura
            fd = open(args[i+1], O_RDONLY);
            if (fd < 0) {
                perror("Error abriendo archivo para redirección de entrada");
                exit(1);
            }
            // Redirigir stdin (descriptor 0) hacia el archivo
            dup2(fd, 0);
            close(fd);

            args[i] = NULL;
            break;
        }

        i++;
    }
}
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <signal.h> 
#include <time.h> 
#include "../include/background.h" 
#include "../include/pmon.h" 

static long ticks_por_segundo; //Cuantos ticks de cpu hay en un segundo real
static volatile sig_atomic_t bandera_refrescar = 0; //Se prende cuando llega SIGALRM
static volatile sig_atomic_t bandera_salir = 0; //Se prende cuando llega SIGINT dentro de pmon

static void manejador_sigalrm(int senal){ //Funcion que reacciona a la alarma
    (void)senal; 
    bandera_refrescar = 1; //Se marca que hay que redibujar la tabla
}

static void manejador_sigint_pmon(int senal){ //Funcion que reacciona a Ctrl+C dentro de pmon
    (void)senal; 
    bandera_salir = 1; //Se marca que hay que terminar el ciclo
}

static int leer_proc_stat(pid_t pid, char *estado, unsigned long *utime, unsigned long *stime){ //Lee estado y tiempos de cpu
    char ruta[64]; //Buffer para la ruta del archivo
    snprintf(ruta, sizeof(ruta), "/proc/%d/stat", pid); //Se arma la ruta con el pid

    FILE *archivo = fopen(ruta, "r"); //Se abre el archivo
    if(archivo == NULL){ //Si no se pudo abrir, el proceso ya no existe
        return -1; //Se avisa del error
    }

    char linea[512]; //Buffer para la linea completa
    if(fgets(linea, sizeof(linea), archivo) == NULL){ //Se lee la linea
        fclose(archivo); //Se cierra el archivo igual
        return -1; //Se avisa del error
    }
    fclose(archivo); //Ya se tiene la linea, se cierra el archivo

    char *cierre_parentesis = strrchr(linea, ')'); //Se busca el ultimo parentesis, por si el nombre tiene espacios
    if(cierre_parentesis == NULL){ //Si no aparece, el formato es invalido
        return -1; //Se avisa del error
    }

    sscanf(cierre_parentesis + 1, " %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", estado, utime, stime); //Se saltan los campos intermedios y se leen los que interesan

    return 0; //Todo salio bien
}

static long leer_proc_rss(pid_t pid){ //Lee la memoria residente en KB
    char ruta[64]; //Buffer para la ruta del archivo
    snprintf(ruta, sizeof(ruta), "/proc/%d/status", pid); //Se arma la ruta con el pid

    FILE *archivo = fopen(ruta, "r"); //Se abre el archivo
    if(archivo == NULL){ //Si no se pudo abrir, el proceso ya no existe
        return -1; //Se avisa del error
    }

    char linea[256]; //Buffer para cada linea leida
    long rss = -1; //Valor por defecto si no se encuentra el campo
    while(fgets(linea, sizeof(linea), archivo) != NULL){ //Se recorre linea por linea
        if(strncmp(linea, "VmRSS:", 6) == 0){ //Se busca la linea que empieza con VmRSS
            sscanf(linea, "VmRSS: %ld", &rss); //Se extrae el numero
            break; //Ya se encontro, no hace falta seguir
        }
    }

    fclose(archivo); //Se cierra el archivo
    return rss; //Se entrega la memoria encontrada
}

static const char *traducir_estado(char estado){ //Traduce el caracter de /proc a texto legible
    switch(estado){ //Se compara el caracter recibido
        case 'R': return "ejecutando"; //Proceso corriendo activamente
        case 'S': return "durmiendo"; //Proceso esperando algo
        case 'Z': return "zombie"; //Proceso terminado sin recoger
        case 'T': return "detenido"; //Proceso pausado
        default: return "desconocido"; //Cualquier otro caso
    }
}

static void dibujar_tabla(void){ //Redibuja la tabla completa de jobs activos
    printf("\033[H\033[J"); //Se limpia la pantalla antes de redibujar
    printf("PID\tCOMANDO\t\tESTADO\t\t%%CPU(aprox)\tRSS(KB)\n"); //Se imprime el encabezado

    for(int i = 0; i < 64; i++){ //Se recorre toda la tabla de jobs
        if(tabla_jobs[i].estado != EJECUTANDO){ //Solo interesan los jobs activos
            continue; //Se salta esta posicion
        }

        char estado_crudo; //Estado tal como lo entrega /proc
        unsigned long utime, stime; //Tiempos de cpu leidos ahora
        if(leer_proc_stat(tabla_jobs[i].pid, &estado_crudo, &utime, &stime) != 0){ //Se lee la ficha del proceso
            continue; //Si ya no existe, se omite en esta vuelta
        }
        long rss = leer_proc_rss(tabla_jobs[i].pid); //Se lee la memoria del proceso

        double porcentaje_cpu = 0.0; //Se asume cero mientras no haya una lectura previa
        time_t ahora = time(NULL); //Se guarda el momento actual

        if(tabla_jobs[i].lectura_valida){ //Solo se calcula si ya hubo una lectura anterior
            unsigned long delta_ticks = (utime + stime) - (tabla_jobs[i].utime_anterior + tabla_jobs[i].stime_anterior); //Cpu gastado entre lecturas
            double delta_cpu_segundos = (double)delta_ticks / ticks_por_segundo; //Se convierte a segundos
            double delta_real_segundos = difftime(ahora, tabla_jobs[i].tiempo_anterior); //Tiempo real transcurrido
            if(delta_real_segundos > 0){ //Se evita dividir por cero
                porcentaje_cpu = (delta_cpu_segundos / delta_real_segundos) * 100.0; //Se calcula el porcentaje
            }
        }

        tabla_jobs[i].utime_anterior = utime; //Se guarda la lectura actual para la proxima vuelta
        tabla_jobs[i].stime_anterior = stime; //Se guarda la lectura actual para la proxima vuelta
        tabla_jobs[i].tiempo_anterior = ahora; //Se guarda el momento de esta lectura
        tabla_jobs[i].lectura_valida = 1; //Ya existe una lectura previa para la proxima vez

        printf("%d\t%s\t%s\t%.1f\t\t%ld\n", tabla_jobs[i].pid, tabla_jobs[i].comando,
               traducir_estado(estado_crudo), porcentaje_cpu, rss); //Se imprime la fila de este job
    }
}

int ejecutar_pmon(char **tokens){ //Funcion principal de pmon, se llama desde los builtins
    int segundos = 2; //Valor por defecto si no se especifica
    if(tokens[1] != NULL){ //Si el usuario dio un argumento
        segundos = atoi(tokens[1]); //Se convierte a entero
        if(segundos <= 0){ //Si el valor no tiene sentido
            segundos = 2; //Se vuelve al valor por defecto
        }
    }

    ticks_por_segundo = sysconf(_SC_CLK_TCK); //Se calcula una sola vez cuanto vale un tick
    bandera_refrescar = 0; //Se reinicia la bandera de refresco
    bandera_salir = 0; //Se reinicia la bandera de salida

    struct sigaction accion_alarm; //Formulario para configurar SIGALRM
    accion_alarm.sa_handler = manejador_sigalrm; //Se indica la funcion reflejo
    sigemptyset(&accion_alarm.sa_mask); //Se deja vacia la mascara
    accion_alarm.sa_flags = 0; //Sin banderas especiales
    sigaction(SIGALRM, &accion_alarm, NULL); //Se registra la configuracion

    struct sigaction accion_int_nueva, accion_int_anterior; //Formularios para SIGINT
    accion_int_nueva.sa_handler = manejador_sigint_pmon; //Se indica la funcion reflejo
    sigemptyset(&accion_int_nueva.sa_mask); //Se deja vacia la mascara
    accion_int_nueva.sa_flags = 0; //Sin banderas especiales
    sigaction(SIGINT, &accion_int_nueva, &accion_int_anterior); //Se guarda la configuracion anterior de SIGINT

    dibujar_tabla(); //Se dibuja la tabla una vez de inmediato
    alarm(segundos); //Se pide la primera alarma

    while(!bandera_salir){ //Se repite hasta que llegue Ctrl+C
        pause(); //Se espera a que llegue cualquier señal

        if(bandera_refrescar){ //Si la señal que llego fue la alarma
            bandera_refrescar = 0; //Se apaga la bandera
            dibujar_tabla(); //Se redibuja la tabla
            alarm(segundos); //Se pide la siguiente alarma
        }
    }

    alarm(0); //Se cancela cualquier alarma pendiente
    sigaction(SIGINT, &accion_int_anterior, NULL); //Se restaura el comportamiento anterior de SIGINT

    return 0; //Se retorna el control a la shell
}
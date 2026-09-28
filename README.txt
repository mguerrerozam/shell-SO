# miShell

Shell simple para Linux, implementada en C, desarrollada para el curso de Sistemas
Operativos. Soporta el ciclo básico de una shell (leer, parsear, ejecutar), comandos
internos, redirección de entrada/salida, tuberías de largo arbitrario, ejecución de
procesos en segundo plano, manejo de señales, y un monitor de procesos propio (`pmon`)
basado en `/proc`.

## Integrantes

- Felipe de la Fuente — Parser y comandos internos (`cd`, `exit`)
- Javiera Aravena — Motor de ejecución y pipes (`fork`/`execvp`/`waitpid`, tuberías)
- Francisco Rubilar — Redirección y señales
- Maximiliano Guerrero — Procesos en background y `pmon`

## Requisitos

- `gcc` con soporte para C11 (se compila con `-std=gnu11`)
- `make`
- Biblioteca `readline` para el manejo de la línea de comandos. En Ubuntu/Debian:

```
sudo apt install libreadline-dev
```

## Compilación

```
make
```

Esto genera el ejecutable `mishell` en la raíz del proyecto. La compilación se realiza
con `gcc -Wall -Wextra -std=gnu11`, sin advertencias.

## Ejecución

```
./mishell
```

La shell muestra un prompt con el directorio actual (`miShell:/ruta/actual$`) y queda
esperando comandos. Para salir: `exit`, o Ctrl+D.

## Funcionalidades

- **Comandos internos:** `cd [dir]`, `exit [n]`, `jobs`, `pmon [segundos]`
- **Redirección:** `cmd > archivo`, `cmd >> archivo`, `cmd < archivo`, combinables
- **Pipes:** tuberías de largo arbitrario, `cmd1 | cmd2 | ... | cmdN`
- **Background:** `cmd &` no bloquea la shell, muestra `[N] PID` de inmediato y avisa
  cuando el proceso termina (`[N]+ Done cmd`)
- **Señales:** Ctrl+C no cierra la shell; si hay un comando en primer plano, lo termina
  a él en vez de a la shell. Los procesos en background no se ven afectados
- **pmon:** monitorea los procesos en background lanzados por la shell, refrescando la
  pantalla cada cierto número de segundos, mostrando PID, comando, estado, %CPU
  aproximado y memoria RSS. Se sale con Ctrl+C, sin cerrar la shell


## Estructura del proyecto

| Archivo(s)                          | Responsable   | Contenido                                  |
|--------------------------------------|---------------|---------------------------------------------|
| `mishell.c` / `mishell.h`            | Compartido    | Bucle principal, estructuras globales        |
| `parser.c` / `parser.h`              | Integrante 1  | Tokenización de la línea de comandos         |
| `builtins.c` / `builtins.h`          | Integrante 1  | `cd`, `exit`, y despacho de `jobs`/`pmon`    |
| `executor.c` / `executor.h`          | Integrante 2  | `fork`/`execvp`/`waitpid`, detección de `&`  |
| `pipes.c` / `pipes.h`                | Integrante 2  | Tuberías de N comandos                       |
| `redirections.c` / `redirections.h`  | Integrante 3  | `<`, `>`, `>>`                                |
| `signals.c` / `signals.h`            | Integrante 3  | SIGINT/SIGQUIT en la shell y en hijos         |
| `background.c` / `background.h`      | Integrante 4  | Tabla de jobs, manejador de SIGCHLD           |
| `pmon.c` / `pmon.h`                  | Integrante 4  | Monitor de procesos vía `/proc`               |

## Limitaciones conocidas



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/parser.h"
#include <readline/readline.h>
#include <readline/history.h>

#define TAMAÑO_INICIAL 8
//Verifica si el caracter es especial o no.
static int es_especial(char c) {
    return(c == '<' || c == '>' || c == '|' || c == '&');
}

static void agregar_token(char ***tokens, int *pos, int *cap,
                    const char *inicio, int largo) {
    if (largo <= 0) return; //Si la palabra es cero, aborta.

    char *tok = strndup(inicio, largo); //Copia el texto segun el largo.
    if (!tok) {perror("strndup"); exit(EXIT_FAILURE); } //Verifica si se quedó sin memoria

    if (*pos >= *cap - 1) {//Verifica que el arreglo siempre tenga al menos un bloque libre para el puntero NULL obligatorio
        //Aummento dinámico del tamaño del arreglo.
        *cap += TAMAÑO_INICIAL;
        *tokens = realloc(*tokens, (*cap) * sizeof(char *));
        if (!(*tokens)) { perror("realloc"); exit(EXIT_FAILURE); }
    }
    (*tokens)[(*pos)++] = tok; //Se guarda el tok en la ranura del arreglo actual, y le suma 1 para el proximo token.
}

char **tokenizar_linea(const char *line) {
    if (!line) return NULL;

    int capacidad = TAMAÑO_INICIAL;
    int posicion = 0;

    char **tokens = malloc(capacidad * sizeof(char *));
    if (!tokens) { perror("malloc"); exit(EXIT_FAILURE); }

    const char *p = line;

    while (*p != '\0') {
        //Caso 1: saltar espacios en blanco
        if (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
            p++;
            continue;
        }
        //Caso 2: Comillas simples
        if (*p == '\'') {
            p++; //Se salta la comilla de apertura.
            const char *inicio = p;
            while (*p != '\0' && *p != '\'') {p++;} //Avanza hasta encontrar la comilla de cierre.
            agregar_token(&tokens, &posicion, &capacidad, inicio, (int)(p - inicio));
            if (*p == '\'') p++; //Se salta la comilla de cierre.
            continue;
        }
        //Caso 3: Comillas dobles.
        if (*p == '"') { //Funcionamiento similar al del Caso 2.
            p++;
            const char *inicio = p;
            while (*p != '"' && *p != '\0') p++;
            agregar_token(&tokens, &posicion, &capacidad, inicio, (int)(p - inicio));
            if (*p == '"') p++;
            continue;
        }
        //Caso 4: Operador '>>'.
        if (*p == '>' && *(p + 1) == '>') {
            agregar_token(&tokens, &posicion, &capacidad, p,2);
            p += 2;
            continue;
        }
        //Caso 5: Operadores simples.
        if (es_especial(*p)) {
            agregar_token(&tokens, &posicion, &capacidad, p,1);
            p++;
            continue;
        }
        //Caso 6: Palabra normal
        {
            const char *inicio = p;
            while (*p != '\0'
                   && *p != ' '  && *p != '\t'
                   && *p != '\r' && *p != '\n'
                   && *p != '\'' && *p != '"'
                   && !es_especial(*p))
            {
                p++;
            }
            agregar_token(&tokens, &posicion, &capacidad, inicio,(int)(p - inicio));
        }
    }
    tokens[posicion] = NULL;
    return tokens;
}

char *leer_linea(const char *promt) {
    char *linea = readline(promt);
    if (!linea) return NULL; // EOF o Ctrl + D
    if (*linea != '\0') {
        add_history(linea);//No guardar lineas vacías en el historial
    }
    return linea;
}
//Liberacion de memoria.
void liberar_tokens(char **tokens)
{
    if (!tokens) return;
    for (int i = 0; tokens[i] != NULL; i++)
        free(tokens[i]);
    free(tokens);
}

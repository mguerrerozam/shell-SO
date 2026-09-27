#ifndef PARSER_H
#define PARSER_H

// Muestra el prompt y lee una línea con soporte de historial.
// Devuelve el string en heap
// Devuelve NULL en EOF (Ctrl+D) o error.
char *leer_linea(const char *prompt);

char **tokenizar_linea(const char *line);

// Libera el array devuelto por tokenizar_linea()
void liberar_tokens(char **tokens);

#endif
#include "shell.h"


char *shell_read_line(void) {
    char *buffer = NULL;
    size_t buff_size = 0;
    char cwd[BUFSIZ];


    getcwd(cwd, sizeof(cwd));
    printf("%s$>", cwd);
    if (getline(&buffer, &buff_size, stdin) == -1) {
        if (feof(stdin)) {
            printf("[EDF] End of file reached\n");
            free(buffer);
            exit(0);
        } else {
            printf("GetLine Failed\n");
        }
    }
    return buffer;
}

int main() {
    char *line;

    while (1) {
        line = shell_read_line();
        printf("%s\n", line);
        free(line);
    }
    return 0;
}
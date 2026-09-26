#include "signals.h"

void configurar_senales_shell(void) {
    struct sigaction sa;

    // sig_ign le dice al sistema que ignore la señal por completo
    sa.sa_handler = SIG_IGN;

    // se vacia la mascara de señales para no bloquear otras interrupciones por accidente
    sigemptyset(&sa.sa_mask);

    // sa_restart hace que las funciones de lectura (como getline o fgets) se reinicien
    // solas si son interrumpidas, evitando que la shell lea basura o se caiga
    sa.sa_flags = SA_RESTART;

    // aplicar la configuracion a sigint (ctrl+c)
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error ignorando SIGINT");
    }

    // aplicar la configuracion a sigquit (ctrl+\)
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        perror("Error ignorando SIGQUIT");
    }
    // Nueva protección para el Bonus: ignorar Ctrl+Z en la shell padre
    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        perror("Error ignorando SIGTSTP");
    }
}

void restaurar_senales_hijo(void) {
    struct sigaction sa;

    // sig_dfl restaura el comportamiento por defecto (que el proceso muera al recibir la señal)
    sa.sa_handler = SIG_DFL;

    // se vacia la mascara
    sigemptyset(&sa.sa_mask);

    // se limpian las banderas, el hijo no necesita sa_restart
    sa.sa_flags = 0;

    // se aplica la restauracion a sigint para que el comando en primer plano pueda ser cancelado
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error restaurando SIGINT");
    }

    // se restaura sigquit por precaucion
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        perror("Error restaurando SIGQUIT");
    }
    // Nueva restauración para el Bonus: que el hijo sí se pause con Ctrl+Z
    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        perror("Error restaurando SIGTSTP");
    }
}

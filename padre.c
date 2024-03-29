#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s [cpu|ram|disco] <PID>\n", argv[0]);
        return 1;
    }

    char monitor_type[BUFFER_SIZE];
    strcpy(monitor_type, argv[1]);

    int pid = atoi(argv[2]);

    int pipefd[2];
    pipe(pipefd);

    pid_t pid_child = fork();

    if (pid_child < 0) {
        perror("Error al hacer fork");
        return 1;
    }

    if (pid_child == 0) { // Proceso hijo
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]); 

        if (strcmp(monitor_type, "cpu") == 0) {
            execl("/usr/local/bin/cpu", "cpu", NULL);
        } else if (strcmp(monitor_type, "ram") == 0) {
            execl("/usr/local/bin/ram", "ram", NULL);
        } else if (strcmp(monitor_type, "disco") == 0) {
            execl("/usr/local/bin/disco", "disco", NULL);
        } else {
            printf("Tipo de monitor no válido\n");
            exit(1);
        }
    } else { 
        close(pipefd[1]); 

        char buffer[BUFFER_SIZE];
        int nbytes = read(pipefd[0], buffer, BUFFER_SIZE);
        buffer[nbytes] = '\0';

        printf("%s", buffer);

        close(pipefd[0]); 
        wait(NULL);
    }

    return 0;
}

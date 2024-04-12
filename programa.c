#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

void ejecutarSubproceso(char *subprograma, char *opcion) {
    int pipefd[2];
    pid_t pid;

    if (pipe(pipefd) == -1) {
        perror("Error al crear la tubería");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("Error al crear el subproceso");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { 
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]); 

        execlp(subprograma, subprograma, opcion, NULL);
        perror("Error al ejecutar el subprograma");
        exit(EXIT_FAILURE);
    } else { 
        close(pipefd[1]);
        char buffer[BUFFER_SIZE];
        ssize_t bytesLeidos;
        while ((bytesLeidos = read(pipefd[0], buffer, BUFFER_SIZE)) > 0) {
            write(STDOUT_FILENO, buffer, bytesLeidos);
        }

        close(pipefd[0]);
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "El subproceso no terminó correctamente\n");
            exit(EXIT_FAILURE);
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc < 2 && argc > 4) {
        fprintf(stderr, "Se debe utilizar de la siguiente manera: %s <cpu, disk, memoria> <argumento especifico> \n", >
        exit(EXIT_FAILURE);
    }
    char *recurso = argv[1];
    char *opcion = NULL;
    if (argc > 2) {
        opcion = argv[2];
    }
    
    if (strcmp(recurso, "cpu") == 0) {
        ejecutarSubproceso("./cpu", opcion);
    } else if (strcmp(recurso, "memoria") == 0) {
        ejecutarSubproceso("./memoria", opcion);
    } else if (strcmp(recurso, "disk") == 0) {
        if (opcion != NULL && (strcmp(opcion, "-tm") == 0 || strcmp(opcion, "-tg") == 0 || strcmp(opcion, "-tk") == 0)>
            ejecutarSubproceso("./disk", opcion);
        } else {
            fprintf(stderr, "Uso general:./programa disk <argumento> \n");
            fprintf(stderr, "Argumentos: <-tm, -tg, -tk> \n");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Recurso no válido: %s\n", recurso);
        exit(EXIT_FAILURE);
    }

    return 0;
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256

void get_cpu_usage(float *usage) {
    FILE *fp;
    char buffer[BUFFER_SIZE];
    float user, nice, system, idle;

    fp = popen("top -bn1 | grep '%Cpu' | awk '{print $2+$4}'", "r");
    if (fp == NULL) {
        perror("Error al ejecutar el comando top");
        exit(1);
    }

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        *usage = atof(buffer);
    } else {
        perror("Error al leer la salida de top");
        exit(1);
    }

    pclose(fp);
}

void get_most_used_process(int *pid, float *cpu_usage) {
    FILE *fp;
    char buffer[BUFFER_SIZE];
    float max_usage = 0;
    int max_pid;

    fp = popen("ps -eo pid,%cpu --sort=-%cpu | awk 'NR==2 {print $1}'", "r");
    if (fp == NULL) {
        perror("Error al ejecutar el comando ps");
        exit(1);
    }

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        max_pid = atoi(buffer);
        max_usage = *cpu_usage;
    } else {
        perror("Error al leer la salida de ps");
        exit(1);
    }

    *pid = max_pid;
    *cpu_usage = max_usage;

    pclose(fp);
}

int main(int argc, char *argv[]) {
    float cpu_usage = 0;
    int most_used_pid = 0;

    // Obtener el uso de la CPU
    get_cpu_usage(&cpu_usage);

    // Obtener el proceso con mayor uso de CPU
    get_most_used_process(&most_used_pid, &cpu_usage);

    // Imprimir resultados
    printf("Uso de CPU: %.2f%%\n", cpu_usage);
    printf("PID con mayor uso de CPU en los últimos 5 minutos: %d\n", most_used_pid);

    return 0;
}

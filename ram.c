#include <sys/sysinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

void escribir(int pipe_write, const char *result_buffer){
    if (write(pipe_write, result_buffer, strlen(result_buffer)) == -1) {
    perror("Error al escribir en la tubería");
    exit(EXIT_FAILURE);
}
}
unsigned long obtenerMemoriaFisica() {
    FILE* file = fopen("/proc/meminfo", "r");
    if (file == NULL) {
        perror("Error abriendo /proc/meminfo");
        exit(EXIT_FAILURE);
    }
    char buffer[BUFFER_SIZE];
    unsigned long total_mem = 0;
    while (fgets(buffer, sizeof(buffer), file)) {
        if (sscanf(buffer, "MemTotal: %lu kB", &total_mem) == 1) {
            break;
        }
    }
    fclose(file);
    return total_mem;
}
void obtenerEstadisticasMemoriaVirtual(int pipe_fd) {
    dup2(pipe_fd, STDOUT_FILENO);
    execlp("ps", "ps", "-eo", "pid,comm,%mem", "--no-headers", "--sort=-%mem", (char *)NULL);
    close(STDOUT_FILENO);
    perror("Error ejecutando execlp");
    exit(EXIT_FAILURE);
}

void obtenerEstadisticasMemoriaVirtualPorPID(int pipe_fd, int pid) {
    dup2(pipe_fd, STDOUT_FILENO);
    char pid_str[20];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);
    execlp("ps", "ps", "-p", pid_str, "-o", "pid,comm,%mem", "--no-headers", (char *)NULL);
    close(STDOUT_FILENO);
    perror("Error ejecutando execlp");
    exit(EXIT_FAILURE);
}
void obtenerEstadisticasMemoriaReal(int pipe_fd) {
    unsigned long total_mem = obtenerMemoriaFisica();
    FILE *ps_output = popen("ps -eo pid,comm,rss --sort=-rss", "r");
    if (ps_output == NULL) {
        perror("Error ejecutando ps");
        exit(EXIT_FAILURE);
    }
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), ps_output)) {
        unsigned long pid_val, rss_val;
        char comm_val[256];
        if (sscanf(buffer, "%lu %s %lu", &pid_val, comm_val, &rss_val) == 3) {
            double porcentaje = ((double)rss_val / total_mem) * 100;
            char datos[BUFFER_SIZE];
            snprintf(datos, BUFFER_SIZE, "%-10lu %-20s %-15.2f%%\n", pid_val, comm_val, porcentaje);
            escribir(pipe_fd, datos);
        }
    }
    pclose(ps_output);
}
void obtenerEstadisticasMemoriaRealPorPID(int pipe_fd, int pid) {
    unsigned long total_mem = obtenerMemoriaFisica();
    char pid_str[20];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);
    char ps_command[256];
    snprintf(ps_command, sizeof(ps_command), "ps -p %s -o pid,comm,rss", pid_str);
    FILE *ps_output = popen(ps_command, "r");
    if (ps_output == NULL) {
        perror("Error ejecutando ps");
        exit(EXIT_FAILURE);
    }
    char buffer[BUFFER_SIZE];
    fgets(buffer, sizeof(buffer), ps_output);
    while (fgets(buffer, sizeof(buffer), ps_output)) {
        unsigned long pid_val, rss_val;
        char comm_val[256];
        if (sscanf(buffer, "%lu %s %lu", &pid_val, comm_val, &rss_val) == 3) {
            double porcentaje = ((double)rss_val / total_mem) * 100;
            char datos[BUFFER_SIZE];
            snprintf(datos, BUFFER_SIZE, "%-10lu %-20s %-15.2f%%\n", pid_val, comm_val, porcentaje);
            escribir(pipe_fd, datos);
        }
    }
    pclose(ps_output);
}
void get_ram_stats(const char *unit, int pipe_write, int process_id){
    char encabezado[BUFFER_SIZE];
    snprintf(encabezado, BUFFER_SIZE, "%-10s %-20s %-15s\n", "PID", "Nombre", "% Utilización");
 
    escribir(pipe_write, encabezado);
    if (strcmp(unit, "-v") == 0) {
        if(process_id != 0){
            obtenerEstadisticasMemoriaVirtualPorPID(pipe_write, process_id);
        }
        else{
            obtenerEstadisticasMemoriaVirtual(pipe_write);
        }
    }
    if (strcmp(unit, "-r") == 0) {
        if(process_id != 0){
            obtenerEstadisticasMemoriaRealPorPID(pipe_write, process_id);
        }
        else{
            obtenerEstadisticasMemoriaReal(pipe_write);
        }
    }
    close(pipe_write);
}
int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Uso: %s [-r | -v] [PID]\n", argv[0]);
        return EXIT_FAILURE;
    }
    int process_id = 0;
    if(argc == 3){
        process_id = atoi(argv[2]);
    }

    int pipe_write = atoi(argv[1]);

    get_ram_stats(argv[1], pipe_write, process_id);

    return EXIT_SUCCESS;
}
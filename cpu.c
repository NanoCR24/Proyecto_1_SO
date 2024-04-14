
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define PROC_STAT "/proc/stat"
#define PROC_PID_STAT "/proc/%d/stat"
#define MAX_BUFFER_SIZE 1024

// Estructura para almacenar información sobre un proceso
typedef struct {
    int pid;
    char name[256];
    double cpu_utilization;
} ProcessInfo;

// Función para obtener la utilización total de la CPU
double getTotalCpuUtilization() {
    FILE *proc_stat;
    char buffer[MAX_BUFFER_SIZE];
    long sum = 0, idle = 0;

    proc_stat = fopen(PROC_STAT, "r");
    if (!proc_stat) {
        perror("Error al abrir /proc/stat");
        return -1.0;
    }

    fgets(buffer, sizeof(buffer), proc_stat);
    fclose(proc_stat);

    char *token = strtok(buffer, " ");
    int i = 0;
    while (token != NULL) {
        token = strtok(NULL, " ");
        if (token != NULL) {
            sum += atoi(token);
            if (i == 3)
                idle = atoi(token);
            i++;
        }
    }

    double totalCpuUtilization = 100.0 * (1.0 - ((double)idle / sum));
    return totalCpuUtilization;
}

// Función para obtener la información de un proceso específico
int getProcessInfo(int pid, ProcessInfo *processInfo) {
    char proc_pid_stat_path[MAX_BUFFER_SIZE];
    sprintf(proc_pid_stat_path, PROC_PID_STAT, pid);

    FILE *proc_pid_stat = fopen(proc_pid_stat_path, "r");
    if (!proc_pid_stat) {
        perror("Error al abrir /proc/<pid>/stat");
        return 0;
    }

    char buffer[MAX_BUFFER_SIZE];
    if (!fgets(buffer, sizeof(buffer), proc_pid_stat)) {
        perror("Error al leer /proc/<pid>/stat");
        fclose(proc_pid_stat);
        return 0;
    }
    fclose(proc_pid_stat);

    sscanf(buffer, "%*d %s", processInfo->name);
    processInfo->pid = pid;

    char *token = strtok(buffer, " ");
    int i = 1;
    while (i < 14 && token != NULL) {
        token = strtok(NULL, " ");
        i++;
    }

    if (!token) {
        perror("Error al obtener el tiempo de CPU para el proceso");
        return 0; // Indicador de error
    }

    long utime = atol(token);

    token = strtok(NULL, " ");
    if (!token) {
        perror("Error al obtener el tiempo de CPU para el proceso");
        return 0;
    }

    long stime = atol(token);

    long total_time = utime + stime;

    struct timespec boot_time;
    if (clock_gettime(CLOCK_BOOTTIME, &boot_time) != 0) {
        perror("Error al obtener el tiempo de arranque del sistema");
        return 0; // Indicador de error
    }

    struct timespec current_time;
    if (clock_gettime(CLOCK_REALTIME, &current_time) != 0) {
        perror("Error al obtener el tiempo actual");
        return 0; // Indicador de error
    }


    double process_elapsed_time = current_time.tv_sec - (double)utime / sysconf(_SC_CLK_TCK);
    if(process_elapsed_time<300){
	processInfo->cpu_utilization = -1;
	return 1;
    }
    double uptime = current_time.tv_sec - boot_time.tv_sec;
    double five_minutes_ago = uptime - 300;
    double processCpuUtilization = 100.0 * ((total_time / (double)sysconf(_SC_CLK_TCK)) / (current_time.tv_sec - five_minutes_ago)) / sysconf(_SC_NPROCESSORS_ONLN);
    processInfo->cpu_utilization = processCpuUtilization;

    return 1; 
}

int main(int argc, char *argv[]) {
    int pipe_write; // Descriptor de archivo del extremo de escritura de la tubería
    char result_buffer[MAX_BUFFER_SIZE];

    // Si no se proporcionan argumentos, mostrar la utilización total de la CPU
    if (argc == 1) {
        // Usar STDOUT_FILENO como el descriptor de archivo del extremo de escritura de la tubería
        pipe_write = STDOUT_FILENO;

        snprintf(result_buffer, MAX_BUFFER_SIZE, "La utilizacion total de CPU es: %.2f%%\n", getTotalCpuUtilization());
    } else if (argc == 2) {
        // Obtener el descriptor de archivo del extremo de escritura de la tubería desde el primer argumento
        pipe_write = atoi(argv[0]);

        int pid = atoi(argv[1]);
        ProcessInfo processInfo;
        if (getProcessInfo(pid, &processInfo)) {
		if (processInfo.cpu_utilization != -1) {
               		 snprintf(result_buffer, MAX_BUFFER_SIZE, "PID: %d\nNombre: %s\nPorcentaje de utilización de CPU: %.2f%%\n", processInfo.pid, processInfo.name, processInfo.cpu_utilization);
          	  } else {
                	snprintf(result_buffer, MAX_BUFFER_SIZE, "El proceso con PID %d no ha sido ejecutado en los últimos 5 minutos.\n", pid);
            	}
        } else {
            snprintf(result_buffer, MAX_BUFFER_SIZE, "El proceso con PID %d no existe.\n", pid);
	}
    } else {
        fprintf(stderr, "Uso incorrecto: %s [pid]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Escribir la información en el pipe
    if (write(pipe_write, result_buffer, strlen(result_buffer)) == -1) {
        perror("Error al escribir en el pipe");
        exit(EXIT_FAILURE);
    }

    if (argc == 2) {
        close(pipe_write); // Cerrar el descriptor de archivo del extremo de escritura de la tubería solo si no es STDOUT_FILENO
    }

    return 0;
}

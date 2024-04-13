#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void get_disk_stats(const char *unit, int pipe_write) {
    FILE *mounts_file = fopen("/proc/mounts", "r");
    if (mounts_file == NULL) {
        perror("Error al abrir /proc/mounts");
        exit(EXIT_FAILURE);
    }

    unsigned long long total_space = 0;
    unsigned long long free_space = 0;
    char device[256];
    char mount_point[256];
    char unit_string[4];

    char line[BUFFER_SIZE];

    while (fgets(line, BUFFER_SIZE, mounts_file) != NULL) {
        if (strstr(line, " / ") != NULL || strstr(line, " /root ") != NULL) {
            sscanf(line, "%s %s %*s %*s %*s %*s", device, mount_point);
            struct statvfs vfs;
            statvfs(mount_point, &vfs);
            total_space = (unsigned long long)vfs.f_blocks * vfs.f_frsize;
            free_space = (unsigned long long)vfs.f_bavail * vfs.f_frsize;
            break;
        }
    }

    fclose(mounts_file);
    if (strcmp(unit, "-tk") == 0) {
        total_space /= 1024;
        free_space /= 1024;
        strcpy(unit_string, "KiB");
    } else if (strcmp(unit, "-tg") == 0) {
        total_space /= (1024 * 1024 * 1024);
        free_space /= (1024 * 1024 * 1024);
        strcpy(unit_string, "GiB");
    } else {
        total_space /= (1024 * 1024);
        free_space /= (1024 * 1024);
        strcpy(unit_string, "MiB");
    }

    float disk_usage = ((float)(total_space - free_space) / (float)total_space) * 100.0;

    char result_buffer[BUFFER_SIZE];
    snprintf(result_buffer, BUFFER_SIZE, "Tamaño total: %llu %s\nEspacio libre: %llu %s\nPorcentaje de utilización: %.2f%%\n", total_space, unit_string, free_space, unit_string, disk_usage);

    if (write(pipe_write, result_buffer, strlen(result_buffer)) == -1) {
        perror("Error al escribir en la tubería");
        exit(EXIT_FAILURE);
    }

    close(pipe_write);
}

int main(int argc, char *argv[]) {
    if (argc != 2 || (strcmp(argv[1], "-tm") != 0 && strcmp(argv[1], "-tk") != 0 && strcmp(argv[1], "-tg") != 0)) {
        printf("Uso: %s [-tm | -tk | -tg]\n", argv[0]);
        return 1;
    }

    // Obtener el descriptor de archivo del extremo de escritura de la tubería desde el argumento
    int pipe_write = atoi(argv[1]);

    // Obtener las estadísticas del disco y escribirlas en la tubería
    get_disk_stats(argv[1], pipe_write);

    return 0;
}
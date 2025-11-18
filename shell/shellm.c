#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 1024
#define MAX_TOKENS (MAX_CMD_LEN / 2 + 1)
#define PATH_MAX 4096

// Verifica si es builtin
int es_builtin(char *cmd) {
    return strcmp(cmd, "cd") == 0 || strcmp(cmd, "exit") == 0;
}

// Reemplazar variables de entorno
int reemplazar_variables(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        char *token = args[i];
        // Ver si el token empieza $ (variable de entorno)
        if (token[0] == '$') {
            // Nombre de la variable de entorno 
            char *var_name = token + 1;
            // Obtiene el valor
            char *value = getenv(var_name);

            if (value) {
                args[i] = value;
            } else {
                fprintf(stderr, "error: var %s does not exist\n", var_name);
                return 0;
            }
        }
    }
    return 1;
}

// Ejecuta builtin
void ejecutar_builtin(char **args) {
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            char *home = getenv("HOME");
            if (home == NULL) {
                fprintf(stderr, "No se pudo determinar el directorio HOME.\n");
                return;
            }
            if (chdir(home) != 0) {
                perror("cd");
            }
        } else if (chdir(args[1]) != 0) {
            perror("cd");
        }
    } else if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }
}

// Devuelve ruta si el archivo es ejecutable
char *buscar_en_path(const char *cmd) {
    char *path_env = getenv("PATH");
    if (!path_env) return NULL;

    char *path_copy = strdup(path_env);
    char *saveptr;
    char *dir = strtok_r(path_copy, ":", &saveptr);

    while (dir) {
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);
        if (access(full_path, X_OK) == 0) {
            free(path_copy);
            return strdup(full_path);
        }
        dir = strtok_r(NULL, ":", &saveptr);
    }

    free(path_copy);
    return NULL;
}

// Ejecuta el comando externo (no builtin)
void ejecutar_externo(char **args, int fd_in, int fd_out, char *in_file, char *out_file, int bg) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        if (fd_in != -1) {
            if (dup2(fd_in, STDIN_FILENO) == -1) {
                perror("dup2 (entrada)");
                exit(1);
            }
            close(fd_in);
        } else if (bg) {
            // Si está en segundo plano y no se redirigió entrada, se usa /dev/null
            int dev_null = open("/dev/null", O_RDONLY);
            if (dev_null == -1) {
                perror("open /dev/null");
                exit(1);
            }
            if (dup2(dev_null, STDIN_FILENO) == -1) {
                perror("dup2 /dev/null");
                exit(1);
            }
            close(dev_null);
        }
        if (fd_out != -1) {
            if (dup2(fd_out, STDOUT_FILENO) == -1) {
                perror("dup2 (salida)");
                exit(1);
            }
            close(fd_out);
        }
        // Si existe y se puede ejcutar del tirón
        if (access(args[0], X_OK) == 0) {
            execv(args[0], args);
            perror("execv");
            exit(1);
        }

        // Si no, se busca el path y se ejecuta
        char *path_cmd = buscar_en_path(args[0]);
        if (path_cmd) {
            execv(path_cmd, args);
            perror("execv");
            free(path_cmd);
            exit(1);
        }

        // No encontrado
        fprintf(stderr, "Comando no encontrado: %s\n", args[0]);
        exit(1);
    } else {
        if (!bg) {
            // Espera
            waitpid(pid, NULL, 0);
        } else {
            printf("[ejecutando en segundo plano] PID: %d\n", pid);
        }
    }
}

void ejecutar_pipeline(char *comandos[], int num_comandos, int bg) {
    int i;
    int fd[2];
    int in_fd = -1;
    pid_t pids[num_comandos];

    int final_out_fd = -1;
    int initial_in_fd = -1;

    // Manejar redirección de entrada en el primer comando
    char *redir_in = strchr(comandos[0], '<');
    if (redir_in) {
        *redir_in = '\0';
        redir_in++;
        while (*redir_in == ' ') redir_in++;
        char *filename = strtok(redir_in, " \t\r\n");
        if (filename) {
            initial_in_fd = open(filename, O_RDONLY);
            if (initial_in_fd < 0) {
                perror("open <");
                return;
            }
        }
    }

    // Manejar redirección de salida en el último comando
    char *redir_out = strchr(comandos[num_comandos - 1], '>');
    if (redir_out) {
        *redir_out = '\0';
        redir_out++;
        while (*redir_out == ' ') redir_out++;
        char *filename = strtok(redir_out, " \t\r\n");
        if (filename) {
            final_out_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (final_out_fd < 0) {
                perror("open >");
                return;
            }
        }
    }

    for (i = 0; i < num_comandos; i++) {
        if (i < num_comandos - 1 && pipe(fd) < 0) {
            perror("pipe");
            return;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            return;
        } else if (pid == 0) {
            // Redirección de entrada
            if (i == 0 && initial_in_fd != -1) {
                dup2(initial_in_fd, STDIN_FILENO);
                close(initial_in_fd);
            } else if (in_fd != -1) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            // Redirección de salida
            if (i == num_comandos - 1 && final_out_fd != -1) {
                dup2(final_out_fd, STDOUT_FILENO);
                close(final_out_fd);
            } else if (i < num_comandos - 1) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }

            if (fd[0] != -1) close(fd[0]);

            // Tokeniza el comando individual
            char *args[MAX_TOKENS];
            int j = 0;
            char *saveptr;
            char *token = strtok_r(comandos[i], " \t\r\n", &saveptr);

            while (token && j < MAX_TOKENS - 1) {
                if (strchr(token, '=') != NULL) {
                    char *eq_pos = strchr(token, '=');
                    *eq_pos = '\0';
                    setenv(token, eq_pos + 1, 1);
                } else {
                    args[j++] = token;
                }
                token = strtok_r(NULL, " \t\r\n", &saveptr);
            }
            args[j] = NULL;

            if (!reemplazar_variables(args)) exit(1);
            if (es_builtin(args[0])) {
                ejecutar_builtin(args);
                exit(0);
            } else {
                ejecutar_externo(args, -1, -1, NULL, NULL, 0);
            }
            exit(0);
        } else {
            pids[i] = pid;

            if (in_fd != -1) close(in_fd);
            if (fd[1] != -1) close(fd[1]);

            in_fd = fd[0];
        }
    }

    if (initial_in_fd != -1) close(initial_in_fd);
    if (final_out_fd != -1) close(final_out_fd);

    if (!bg) {
        for (i = 0; i < num_comandos; i++) {
            waitpid(pids[i], NULL, 0);
        }
    } else {
        printf("[pipeline en segundo plano]\n");
    }
}


// Tokeniza línea y ejecuta comando
void ejecutar_linea(char *linea) {
    char *segmentos[10]; // Soporta hasta 10 comandos en pipeline
    int num_comandos = 0;
    int bg = 0;

    // Elimina el salto de línea
    linea[strcspn(linea, "\n")] = '\0';

    // Detecta si está en segundo plano
    char *amp = strrchr(linea, '&');
    if (amp && *(amp + 1) == '\0') {
        bg = 1;
        *amp = '\0'; // Quita el '&'
    }

    // Divide por '|'
    char *saveptr;
    char *token = strtok_r(linea, "|", &saveptr);
    while (token && num_comandos < 10) {
        while (*token == ' ') token++; // Quitar espacios iniciales
        segmentos[num_comandos++] = strdup(token);
        token = strtok_r(NULL, "|", &saveptr);
    }

    if (num_comandos == 1) {
        // Ejecuta normalmente sin pipeline
        char *cmd_line = segmentos[0];
        char *args[MAX_TOKENS];
        int i = 0;
        char *in_file = NULL;
        char *out_file = NULL;
        int fd_in = -1, fd_out = -1;

        char *saveptr2;
        char *tok = strtok_r(cmd_line, " \t\r\n", &saveptr2);
        while (tok && i < MAX_TOKENS - 1) {
            if (strcmp(tok, "<") == 0 && (tok = strtok_r(NULL, " \t\r\n", &saveptr2))) {
                fd_in = open(tok, O_RDONLY);
                if (fd_in < 0) {
                    perror("open");
                    return;
                }
                in_file = tok;
            } else if (strcmp(tok, ">") == 0 && (tok = strtok_r(NULL, " \t\r\n", &saveptr2))) {
                fd_out = open(tok, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out < 0) {
                    perror("open");
                    return;
                }
                out_file = tok;
            } else if (strchr(tok, '=') != NULL) {
                char *eq_pos = strchr(tok, '=');
                *eq_pos = '\0';
                setenv(tok, eq_pos + 1, 1);
            } else {
                args[i++] = tok;
            }
            tok = strtok_r(NULL, " \t\r\n", &saveptr2);
        }
        args[i] = NULL;

        if (args[0] == NULL) return;
        if (!reemplazar_variables(args)) return;

        if (es_builtin(args[0])) {
            ejecutar_builtin(args);
        } else {
            ejecutar_externo(args, fd_in, fd_out, in_file, out_file, bg);
        }
    } else {
        // Ejecutar pipeline
        ejecutar_pipeline(segmentos, num_comandos, bg);
    }

    // Liberar memoria
    for (int i = 0; i < num_comandos; i++) {
        free(segmentos[i]);
    }
}


int main() {
    char linea[MAX_CMD_LEN];

    while (1) {
        printf("> ");
        fflush(stdout);

        if (!fgets(linea, sizeof(linea), stdin)) {
            // EOF o error
            break;
        }

        ejecutar_linea(linea);
    }

    printf("\n");
    return 0;
}
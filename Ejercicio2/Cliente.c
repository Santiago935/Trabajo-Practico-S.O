// cliente.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>



int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in servidor;
    char mensaje[1024], respuesta[1024];
    char ip[16];
    int puerto;

    // Verificar que el usuario proporcione ip y opcional puerto
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <IP> [PUERTO]\n", argv[0]);
        exit(1);
    }

    snprintf(ip, sizeof(ip), "%s", argv[1]);

    if (argc == 3) {
        puerto = atoi(argv[2]);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");

        return 1;
    }

    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(puerto);
    if (inet_pton(AF_INET, ip, &servidor.sin_addr) <= 0) {
        fprintf(stderr, "Valor de IP invalida\n");

        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr*)&servidor, sizeof(servidor)) < 0) {
        perror("connect");

        close(sock);
        return 1;
    }

    int leido = read(sock, respuesta, sizeof(respuesta)-1);
    if (leido > 0) {
        respuesta[leido] = '\0';
        if (!strncmp(respuesta, "Servidor pendiente de mensaje", 29)) {
            printf("[Cliente] Conectado al servidor.\n");

        } else {
            printf("[Servidor] %s\n", respuesta);
            close(sock);
            return -1;
        }
    } else {
        printf("[Cliente] No se pudo leer confirmación del servidor.\n");

        close(sock);
        return -1;
    }

    while (1) {
        printf("> ");
        if (fgets(mensaje, sizeof(mensaje), stdin) == NULL) break;

        // Quita el newline
        mensaje[strcspn(mensaje, "\n")] = 0;

        if (strncmp(mensaje, "SALIR", 5) == 0) {
            write(sock, "SALIR", 5);
            break;
        }

        strcat(mensaje, "\n");

        write(sock, mensaje, strlen(mensaje));

        leido = read(sock, respuesta, sizeof(respuesta)-1);
        if (leido > 0) {
            respuesta[leido] = '\0';
            printf("[Servidor] %s\n", respuesta);
        } else {
            printf("[Cliente] El servidor cerró la conexión. Saliendo...\n");

            break;
        }
    }

    close(sock);
    printf("[Cliente] Desconectado.\n");

    return 0;
}


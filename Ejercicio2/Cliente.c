// cliente.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PUERTO 5000

int main() {
    int sock;
    struct sockaddr_in servidor;
    char mensaje[1024], respuesta[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(PUERTO);
    servidor.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&servidor, sizeof(servidor)) < 0) {
        perror("connect");
        return 1;
    }

    printf("[Cliente] Conectado al servidor.\n");

    while (1) {
        printf("> ");
        fgets(mensaje, sizeof(mensaje), stdin);

        if (strncmp(mensaje, "SALIR", 5) == 0) {
            write(sock, "SALIR", 5);
            break;
        }

        write(sock, mensaje, strlen(mensaje));
        int leido = read(sock, respuesta, sizeof(respuesta)-1);
        if (leido > 0) {
            respuesta[leido] = '\0';
            printf("[Servidor] %s", respuesta);
        }
    }

    close(sock);
    printf("[Cliente] Desconectado.\n");
    return 0;
}

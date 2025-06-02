#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>

#define N 5
#define HIJOS 4

typedef struct
{
    int datos[N];
    int interacciones[HIJOS];
    int finalizar;
} Mesa;

int main()
{

    // CREO MEMORIA COMPARTIDA
    Mesa *mesa = mmap(NULL, sizeof(Mesa), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *semA = mmap(NULL, sizeof(sem_t),
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS,
                       -1, 0);
    sem_t *semB = mmap(NULL, sizeof(sem_t),
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS,
                       -1, 0);

    sem_t *semC = mmap(NULL, sizeof(sem_t),
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS,
                       -1, 0);
    sem_t *semD = mmap(NULL, sizeof(sem_t),
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS,
                       -1, 0);

    if (mesa == MAP_FAILED || semA == MAP_FAILED || semB == MAP_FAILED || semC == MAP_FAILED || semD == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Inicializo la mesa
    int datos_iniciales[N] = {3, 10, 6, 2, 8};
    memcpy(mesa->datos, datos_iniciales, sizeof(datos_iniciales));
    memset(mesa->interacciones, 0, sizeof(mesa->interacciones));
    mesa->finalizar = 0;

    // INICIALIZO SEMAFOROS
    if (sem_init(semA, 1, 1) == -1 ||
        sem_init(semB, 1, 3) == -1 ||
        sem_init(semC, 1, 1) == -1 ||
        sem_init(semD, 1, 0) == -1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    // Declaro variable del menu y platos
    int opc;
    int platos[3] = {0, 0, 0};
    int num;
    // Menu de Platos
    
    while(1)
    {
        printf("\tMenu:\n1- Pastel de Papas.\n2- Guiso de lentejas.\n3- Locro.\n4- Salir.\n");
        // Espero que me ingrese una opción
        scanf("%d", &opc);
        // Valido las opciones de entradas
        if (opc > 0 && opc < 4)
        {


            switch (opc){
                case 1: num = 2;
                case 2: num = 5;
                case 3: num = 8;
            }
            pid_t cheff = fork();
            platos[opc]++;
            // Empieza el codigo
            if (cheff == 0)
            {

                pid_t cocineroCorta = fork();
                if (cocineroCorta == 0)
                {

                    sem_wait(semA);
                    sem_wait(semB);
                    sem_wait(semB);

                    for (int i = 0; i < N; i++)
                    {
                        mesa->datos[i] = mesa->datos[i] / num;

                        printf("Cocinero corta ingrediente %d\n", mesa->datos[i]);
                    }
                    mesa->interacciones[0]++;
                    sleep(10);

                    sem_post(semC);
                    sem_post(semB);
                    exit(0);
                }
                pid_t cocineroPica = fork();
                if (cocineroPica == 0)
                {

                    sem_wait(semB);
                    sem_wait(semD);

                    for (int i = 0; i < N; i++)
                    {
                        mesa->datos[i] = mesa->datos[i] * num;
                        printf("Cocinero pica ingrediente %d\n", mesa->datos[i]);
                    }

                    mesa->interacciones[1]++;
                    sleep(15);

                    sem_post(semA);
                    sem_post(semA);
                    exit(0);
                }

                pid_t cocineroCocina = fork();
                if (cocineroCocina == 0)
                {

                    sem_wait(semC);
                    sem_wait(semC);
                    sem_wait(semB);

                    for (int i = 0; i < N; i++)
                    {
                        mesa->datos[i] = mesa->datos[i] + num;
                        printf("Cocinero cocina ingrediente %d\n", mesa->datos[i]);
                    }
                    mesa->interacciones[2]++;
                    sleep(10);
                    sem_post(semD);
                    sem_post(semD);
                    exit(0);
                }

                pid_t cocineroEmpalta = fork();
                if (cocineroEmpalta == 0)
                {
                    sem_wait(semD);
                    sem_wait(semA);

                    // Simula ordenar ingredientes
                    for (int i = 0; i < N - 1; i++)
                        for (int j = 0; j < N - i - 1; j++)
                            if (mesa->datos[j] > mesa->datos[j + 1])
                            {
                                int tmp = mesa->datos[j];
                                mesa->datos[j] = mesa->datos[j + 1];
                                mesa->datos[j + 1] = tmp;
                            }

                    printf("Cocinero empalta ingredientes: ");

                    for (int i = 0; i < N; i++)
                    {

                        printf("%d ,", mesa->datos[i]);
                    }

                    mesa->interacciones[3]++;
                    sleep(20);

                    sem_post(semB);
                    sem_post(semC);
                    exit(0);
                }
                // Espera a los 4 cocineros
                for (int i = 0; i < HIJOS; i++)
                    wait(NULL);
                
                exit(0);
            }
            
            waitpid(cheff, NULL, 0);
        }
        else if (opc == 4)
        {

                printf("\n--- Informe final ---\n");
                for (int i = 1; i < 4; i++)
                    printf("Se pidieron en total del palto %d: %d\n", i, platos[i]);
                printf("\nInteracciones por cocinero:\n");
                printf("Cortar: %d\n", mesa->interacciones[0]);
                printf("Picar: %d\n", mesa->interacciones[1]);
                printf("Cocinar: %d\n", mesa->interacciones[2]);
                printf("Emplatar: %d\n", mesa->interacciones[3]);
            // Liberar recursos
            sem_destroy(semA);
            sem_destroy(semB);
            sem_destroy(semC);
            sem_destroy(semD);
            munmap(mesa, sizeof(Mesa));
            munmap(semA, sizeof(sem_t));
            munmap(semB, sizeof(sem_t));
            munmap(semC, sizeof(sem_t));
            munmap(semD, sizeof(sem_t));
            break;
        }
        else
            printf("Opcion Invalida. Ingrese Nuevamente..\n");

    }

    return 0;
}

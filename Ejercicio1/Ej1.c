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

void cortarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, sem_t* semEspera, sem_t* semSignal);
void picarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, sem_t* semEspera, sem_t* semSignal);
void cocinarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, sem_t* semEspera, sem_t* semSignal);
void emplatarIngredientes(int cantidadIngredientes, Mesa* mesa, sem_t* semEspera, sem_t* semSignal);

int main()
{
    //CREO MEMORIA COMPARTIDA
    Mesa* mesa = mmap(NULL, sizeof(Mesa), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mesa == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    sem_t* semA = mmap(NULL, sizeof(sem_t),
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        -1, 0);

    if (semA == MAP_FAILED)
    {
        perror("mmap");
        munmap(mesa, sizeof(Mesa));
        exit(EXIT_FAILURE);
    }

    sem_t* semB = mmap(NULL, sizeof(sem_t),
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        -1, 0);

    if (semB == MAP_FAILED)
    {
        perror("mmap");
        munmap(mesa, sizeof(Mesa));
        munmap(semA, sizeof(sem_t));
        exit(EXIT_FAILURE);
    }

    sem_t* semC = mmap(NULL, sizeof(sem_t),
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        -1, 0);

    if (semC == MAP_FAILED)
    {
        perror("mmap");
        munmap(mesa, sizeof(Mesa));
        munmap(semA, sizeof(sem_t));
        munmap(semB, sizeof(sem_t));
        exit(EXIT_FAILURE);
    }

    sem_t* semD = mmap(NULL, sizeof(sem_t),
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        -1, 0);

    if (semD == MAP_FAILED)
    {
        perror("mmap");
        munmap(mesa, sizeof(Mesa));
        munmap(semA, sizeof(sem_t));
        munmap(semB, sizeof(sem_t));
        munmap(semC, sizeof(sem_t));
        exit(EXIT_FAILURE);
    }

    // Inicializo la mesa
    int datos_iniciales[N] = {30, 10, 60, 20, 8};
    memcpy(mesa->datos, datos_iniciales, sizeof(datos_iniciales));
    memset(mesa->interacciones, 0, sizeof(mesa->interacciones));
    mesa->finalizar = 0;

    // INICIALIZO SEMAFOROS
    if (sem_init(semA, 1, 1) == -1)
    {
        munmap(mesa, sizeof(Mesa));
        munmap(semA, sizeof(sem_t));
        munmap(semB, sizeof(sem_t));
        munmap(semC, sizeof(sem_t));
        munmap(semD, sizeof(sem_t));
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    else if (sem_init(semB, 1, 0) == -1)
    {
        sem_destroy(semA);
        munmap(mesa, sizeof(Mesa));
        munmap(semA, sizeof(sem_t));
        munmap(semB, sizeof(sem_t));
        munmap(semC, sizeof(sem_t));
        munmap(semD, sizeof(sem_t));
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    else if (sem_init(semC, 1, 0) == -1)
    {
        sem_destroy(semA);
        sem_destroy(semB);
        munmap(mesa, sizeof(Mesa));
        munmap(semA, sizeof(sem_t));
        munmap(semB, sizeof(sem_t));
        munmap(semC, sizeof(sem_t));
        munmap(semD, sizeof(sem_t));
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    else if (sem_init(semD, 1, 0) == -1)
    {
        sem_destroy(semA);
        sem_destroy(semB);
        sem_destroy(semC);
        munmap(mesa, sizeof(Mesa));
        munmap(semA, sizeof(sem_t));
        munmap(semB, sizeof(sem_t));
        munmap(semC, sizeof(sem_t));
        munmap(semD, sizeof(sem_t));
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    // Declaro variable del menu y platos
    int opc = 0;
    int platos[3] = {0, 0, 0};
    int num;
    // Menu de Platos

    while (opc != 4)
    {
        printf("\n\tMenu:\n1- Pastel de Papas.\n2- Guiso de lentejas.\n3- Locro.\n4- Salir.\n");
        // Espero que me ingrese una opción
        scanf("%d", &opc);

        // Valido las opciones de entradas
        if (opc > 0 && opc < 4)
        {
            switch (opc)
            {
            case 1:
                num = 2;
                break;
            case 2:
                num = 5;
                break;
            case 3:
                num = 8;
                break;
            }
            pid_t cheff = fork();
            platos[opc - 1]++;
            // Empieza el codigo
            if (cheff == 0)
            {
                printf("\nPID cheff: %d\n", getpid());

                pid_t cocineroCorta = fork();
                if (cocineroCorta == 0)
                    cortarIngredientes(N, num, mesa, semA, semB);
                
                pid_t cocineroCocina = fork();
                if (cocineroCocina == 0)
                    cocinarIngredientes(N, num, mesa, semB, semC);

                pid_t cocineroPica = fork();
                if (cocineroPica == 0)
                    picarIngredientes(N, num, mesa, semC, semD);

                pid_t cocineroEmpalta = fork();
                if (cocineroEmpalta == 0)
                    emplatarIngredientes(N, mesa, semD, semA);

                // Espera a los 4 cocineros
                for (int i = 0; i < HIJOS; i++)
                    wait(NULL);

                exit(0);
            }

            waitpid(cheff, NULL, 0);
            // Reinicio mesa
            memcpy(mesa->datos, datos_iniciales, sizeof(datos_iniciales));
        }
        else if (opc == 4)
        {
            printf("\n--- Informe final ---\n");
            for (int i = 0; i < 3; i++)
                printf("Se pidieron en total del plato %d: %d\n", i + 1, platos[i]);
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
        }
        else
            printf("Opcion Invalida. Ingrese Nuevamente..\n");
    }

    return 0;
}

void cortarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, sem_t* semEspera, sem_t* semSignal)
{
    sem_wait(semEspera);
    printf("\nPID cortar: %d\n", getpid());

    for (int i = 0; i < cantidadIngredientes; i++)
    {
        mesa->datos[i] = mesa->datos[i] / num;

        printf("\nCocinero corta ingrediente %d", mesa->datos[i]);
    }
    mesa->interacciones[0]++;
    sleep(10);

    sem_post(semSignal);
    exit(0);
}

void picarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, sem_t* semEspera, sem_t* semSignal)
{
    sem_wait(semEspera);
    printf("\nPID picador: %d\n", getpid());

    for (int i = 0; i < cantidadIngredientes; i++)
    {
        mesa->datos[i] = mesa->datos[i] * num;
        printf("\nCocinero pica ingrediente %d", mesa->datos[i]);
    }

    mesa->interacciones[1]++;
    sleep(15);

    sem_post(semSignal);
    exit(0);
}

void cocinarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, sem_t* semEspera, sem_t* semSignal)
{
    sem_wait(semEspera);
    printf("\nPID cocinar: %d", getpid());

    for (int i = 0; i < cantidadIngredientes; i++)
    {
        mesa->datos[i] = mesa->datos[i] + num;
        printf("\nCocinero cocina ingrediente %d", mesa->datos[i]);
    }
    mesa->interacciones[2]++;
    sleep(10);
    sem_post(semSignal);

    exit(0);
}

void emplatarIngredientes(int cantidadIngredientes, Mesa* mesa, sem_t* semEspera, sem_t* semSignal)
{
    sem_wait(semEspera);
    printf("\nPID emplatador: %d", getpid());

    // Simula ordenar ingredientes
    for (int i = 0; i < cantidadIngredientes - 1; i++)
        for (int j = 0; j < cantidadIngredientes - i - 1; j++)
            if (mesa->datos[j] > mesa->datos[j + 1])
            {
                int tmp = mesa->datos[j];
                mesa->datos[j] = mesa->datos[j + 1];
                mesa->datos[j + 1] = tmp;
            }

    printf("\nCocinero emplata ingredientes: \n");

    for (int i = 0; i < cantidadIngredientes; i++)
    {
        printf("%d ", mesa->datos[i]);
    }

    mesa->interacciones[3]++;
    sleep(20);

    sem_post(semSignal);
    exit(0);
}
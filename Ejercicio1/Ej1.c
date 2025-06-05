#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define N 5
#define HIJOS 4

typedef struct
{
    int datos[N];
    int interacciones[HIJOS];
    int finalizar;
} Mesa;

// Operaciones para System V
struct sembuf wait_op = {0, -1, 0};
struct sembuf signal_op = {0, 1, 0};

void cortarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, int semEspera, int semSignal);
void picarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, int semEspera, int semSignal);
void cocinarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, int semEspera, int semSignal);
void emplatarIngredientes(int cantidadIngredientes, Mesa* mesa, int semEspera, int semSignal);

int main()
{
    Mesa* mesa = mmap(NULL, sizeof(Mesa), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mesa == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    int datos_iniciales[N] = {30, 10, 60, 20, 8};
    memcpy(mesa->datos, datos_iniciales, sizeof(datos_iniciales));
    memset(mesa->interacciones, 0, sizeof(mesa->interacciones));
    mesa->finalizar = 0;

    // Crear semaforos System V
    int semA = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    int semB = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    int semC = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    int semD = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);

    if (semA == -1 || semB == -1 || semC == -1 || semD == -1)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    semctl(semA, 0, SETVAL, 1);
    semctl(semB, 0, SETVAL, 0);
    semctl(semC, 0, SETVAL, 0);
    semctl(semD, 0, SETVAL, 0);

    int opc = 0;
    int platos[3] = {0, 0, 0};
    int num;

    while (opc != 4)
    {
        printf("\n\tMenu:\n1- Pastel de Papas.\n2- Guiso de lentejas.\n3- Locro.\n4- Salir.\n");
        scanf("%d", &opc);

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

                for (int i = 0; i < HIJOS; i++)
                    wait(NULL);

                exit(0);
            }

            waitpid(cheff, NULL, 0);
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

            semctl(semA, 0, IPC_RMID);
            semctl(semB, 0, IPC_RMID);
            semctl(semC, 0, IPC_RMID);
            semctl(semD, 0, IPC_RMID);
            munmap(mesa, sizeof(Mesa));
        }
        else
            printf("Opcion Invalida. Ingrese Nuevamente..\n");
    }

    return 0;
}

void cortarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, int semEspera, int semSignal)
{
    semop(semEspera, &wait_op, 1);
    printf("\nPID cortar: %d\n", getpid());

    for (int i = 0; i < cantidadIngredientes; i++)
    {
        mesa->datos[i] = mesa->datos[i] / num;
        printf("\nCocinero corta ingrediente %d", mesa->datos[i]);
    }
    mesa->interacciones[0]++;
    sleep(10);

    semop(semSignal, &signal_op, 1);
    exit(0);
}

void picarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, int semEspera, int semSignal)
{
    semop(semEspera, &wait_op, 1);
    printf("\nPID picador: %d\n", getpid());

    for (int i = 0; i < cantidadIngredientes; i++)
    {
        mesa->datos[i] = mesa->datos[i] * num;
        printf("\nCocinero pica ingrediente %d", mesa->datos[i]);
    }

    mesa->interacciones[1]++;
    sleep(15);

    semop(semSignal, &signal_op, 1);
    exit(0);
}

void cocinarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, int semEspera, int semSignal)
{
    semop(semEspera, &wait_op, 1);
    printf("\nPID cocinar: %d", getpid());

    for (int i = 0; i < cantidadIngredientes; i++)
    {
        mesa->datos[i] = mesa->datos[i] + num;
        printf("\nCocinero cocina ingrediente %d", mesa->datos[i]);
    }
    mesa->interacciones[2]++;
    sleep(10);
    semop(semSignal, &signal_op, 1);

    exit(0);
}

void emplatarIngredientes(int cantidadIngredientes, Mesa* mesa, int semEspera, int semSignal)
{
    semop(semEspera, &wait_op, 1);
    printf("\nPID emplatador: %d", getpid());

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
        printf("%d ", mesa->datos[i]);

    mesa->interacciones[3]++;
    sleep(20);

    semop(semSignal, &signal_op, 1);
    exit(0);
}

    exit(0);
}

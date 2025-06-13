#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <signal.h>

#define N 5
#define HIJOS 4
#define MAX_CHEFFS 100

typedef struct
{
    int datos[N];
    int interacciones[HIJOS];
    int finalizar;
} Mesa;

struct sembuf wait_op = {0, -1, 0};
struct sembuf signal_op = {0, 1, 0};

Mesa* mesa;
int shmid;
int semA, semB, semC, semD;
pid_t cheffsActivos[MAX_CHEFFS];
int cheffsCount = 0;

void cortarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, int semEspera, int semSignal);
void picarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, int semEspera, int semSignal);
void cocinarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, int semEspera, int semSignal);
void emplatarIngredientes(int cantidadIngredientes, Mesa* mesa, int semEspera, int semSignal);

void liberarRecursos() {
    semctl(semA, 0, IPC_RMID);
    semctl(semB, 0, IPC_RMID);
    semctl(semC, 0, IPC_RMID);
    semctl(semD, 0, IPC_RMID);
    shmdt(mesa);
    shmctl(shmid, IPC_RMID, NULL);
}

void manejar_salida(int sig) {
    for (int i = 0; i < cheffsCount; i++)
        killpg(cheffsActivos[i], SIGTERM);
    liberarRecursos();
    exit(0);
}

int main()
{
    signal(SIGINT, manejar_salida);
    signal(SIGTERM, manejar_salida);

    key_t key = ftok("/tmp", 65);
    shmid = shmget(key, sizeof(Mesa), 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    mesa = (Mesa*)shmat(shmid, NULL, 0);
    if (mesa == (void*)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    int datos_iniciales[N] = {30, 10, 60, 20, 8};
    memcpy(mesa->datos, datos_iniciales, sizeof(datos_iniciales));
    memset(mesa->interacciones, 0, sizeof(mesa->interacciones));
    mesa->finalizar = 0;

    semA = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semB = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semC = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semD = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);

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
            if (cheff == 0)
            {
                prctl(PR_SET_PDEATHSIG, SIGTERM);
                setpgid(0, 0); // nuevo grupo de procesos
                printf("\nPID cheff: %d\n", getpid());
                pid_t pid;

                if ((pid = fork()) == 0) cortarIngredientes(N, num, mesa, semA, semB);
                if ((pid = fork()) == 0) cocinarIngredientes(N, num, mesa, semB, semC);
                if ((pid = fork()) == 0) picarIngredientes(N, num, mesa, semC, semD);
                if ((pid = fork()) == 0) emplatarIngredientes(N, mesa, semD, semA);

                for (int i = 0; i < HIJOS; i++)
                    wait(NULL);

                exit(0);
            }

            // Padre
            setpgid(cheff, cheff);
            cheffsActivos[cheffsCount++] = cheff;
            waitpid(cheff, NULL, 0);
            memcpy(mesa->datos, datos_iniciales, sizeof(datos_iniciales));
            platos[opc - 1]++;
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
            liberarRecursos();
        }
        else
        {
            printf("Opcion Invalida. Ingrese Nuevamente..\n");
        }
    }

    return 0;
}

void cortarIngredientes(int cantidadIngredientes, int num, Mesa* mesa, int semEspera, int semSignal)
{
    prctl(PR_SET_PDEATHSIG, SIGTERM);
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
    prctl(PR_SET_PDEATHSIG, SIGTERM);
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
    prctl(PR_SET_PDEATHSIG, SIGTERM);
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
    prctl(PR_SET_PDEATHSIG, SIGTERM);
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

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

typedef struct {
    int datos[N];
    int interacciones[HIJOS];
    int finalizar;
} Mesa;

// Semáforos
struct sembuf wait_op = {0, -1, 0}; // wait
struct sembuf signal_op = {0, 1, 0}; // signal

Mesa* mesa;
int shmid;
int semA, semB, semC, semD;

pid_t cheffsActivos[MAX_CHEFFS];
int cheffsCount = 0;

// ----------Handlers------------

void manejar_salida_hijo(int sig) {
    printf("\n[Proceso %d] Finalizado abruptamente por SIGTERM\n", getpid());
    kill(0, SIGTERM);
    exit(0);
}

void instalar_handler_hijo(){
    struct sigaction sa = {0};

    sa.sa_handler = manejar_salida_hijo;
    sigaction(SIGTERM, &sa, NULL);
}

void liberarRecursos(){
    if (semA > 0) semctl(semA, 0, IPC_RMID);
    if (semB > 0) semctl(semB, 0, IPC_RMID);
    if (semC > 0) semctl(semC, 0, IPC_RMID);
    if (semD > 0) semctl(semD, 0, IPC_RMID);
    if (mesa) {
        shmdt(mesa);
        shmctl(shmid, IPC_RMID, NULL);
    }
}

void manejar_salida_padre(int sig) {
    printf("\n[PADRE %d] Finalizando por SIGTERM\n", getpid());
    // Limpia antes de salir
    if (semA > 0) semctl(semA, 0, IPC_RMID);
    if (semB > 0) semctl(semB, 0, IPC_RMID);
    if (semC > 0) semctl(semC, 0, IPC_RMID);
    if (semD > 0) semctl(semD, 0, IPC_RMID);
    if (shmid > 0)
    {
        shmdt(mesa);
        shmctl(shmid, IPC_RMID, NULL);
    }
    exit(0);
}

// ---------- FUNCIONES HIJOS ----------

void cortarIngredientes(int cantidad, int num, Mesa* mesa, int semEspera, int semSignal) {
    instalar_handler_hijo();
    prctl(PR_SET_PDEATHSIG, SIGTERM);
    semop(semEspera, &wait_op, 1);
    printf("\n[Cortar] PID: %d\n", getpid());

    for (int i = 0; i < cantidad; i++) {
        mesa->datos[i] /= num;
        printf(" Corta %d", mesa->datos[i]);
    }
    printf("\n");

    mesa->interacciones[0]++;
    sleep(10);

    semop(semSignal, &signal_op, 1);
    exit(0);
}

void picarIngredientes(int cantidad, int num, Mesa* mesa, int semEspera, int semSignal) {
    instalar_handler_hijo();
    prctl(PR_SET_PDEATHSIG, SIGTERM);
    semop(semEspera, &wait_op, 1);
    printf("\n[Picar] PID: %d\n", getpid());

    for (int i = 0; i < cantidad; i++) {
        mesa->datos[i] *= num;
        printf(" Pica %d", mesa->datos[i]);
    }
    printf("\n");

    mesa->interacciones[1]++;
    sleep(15);

    semop(semSignal, &signal_op, 1);
    exit(0);
}

void cocinarIngredientes(int cantidad, int num, Mesa* mesa, int semEspera, int semSignal) {
    instalar_handler_hijo();
    prctl(PR_SET_PDEATHSIG, SIGTERM);
    semop(semEspera, &wait_op, 1);
    printf("\n[Cocinar] PID: %d\n", getpid());

    for (int i = 0; i < cantidad; i++) {
        mesa->datos[i] += num;
        printf(" Cocina %d", mesa->datos[i]);
    }
    printf("\n");

    mesa->interacciones[2]++;
    sleep(10);

    semop(semSignal, &signal_op, 1);
    exit(0);
}

void emplatarIngredientes(int cantidad, Mesa* mesa, int semEspera, int semSignal) {
    instalar_handler_hijo();
    prctl(PR_SET_PDEATHSIG, SIGTERM);
    semop(semEspera, &wait_op, 1);
    printf("\n[Emplatar] PID: %d\n", getpid());

    for (int i = 0; i < cantidad - 1; i++) {
        for (int j = 0; j < cantidad - i - 1; j++) {
            if (mesa->datos[j] > mesa->datos[j + 1]) {
                int tmp = mesa->datos[j];
                mesa->datos[j] = mesa->datos[j + 1];
                mesa->datos[j + 1] = tmp;
            }
        }
    }

    printf("\nEmplatado: ");
    for (int i = 0; i < cantidad; i++) {
        printf("%d ", mesa->datos[i]);
    }
    printf("\n");

    mesa->interacciones[3]++;
    sleep(20);

    semop(semSignal, &signal_op, 1);
    exit(0);
}


int main() {
    signal(SIGINT, manejar_salida_padre);
    signal(SIGTERM, manejar_salida_padre);

    instalar_handler_hijo();

    key_t key = ftok("/tmp", 65);
    shmid = shmget(key, sizeof(Mesa), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    mesa = (Mesa*)shmat(shmid, NULL, 0);
    if (mesa == (void*)-1) {
        perror("shmat");
        exit(1);
    }

    int datos_iniciales[N] = {30, 10, 60, 20, 8};

    memcpy(mesa->datos, datos_iniciales, sizeof(datos_iniciales));

    memset(mesa->interacciones, 0, sizeof(mesa->interacciones));

    mesa->finalizar = 0;

    // crear semáforos
    semA = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semB = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semC = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semD = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);

    if (semA == -1 ||
        semB == -1 ||
        semC == -1 ||
        semD == -1) {
        perror("semget");
        exit(1);
    }

    semctl(semA, 0, SETVAL, 1);
    semctl(semB, 0, SETVAL, 0);
    semctl(semC, 0, SETVAL, 0);
    semctl(semD, 0, SETVAL, 0);

    int opc = 0;
    int platos[3] = {0, 0, 0};

    int num;

    while (opc != 4) {
        if (mesa->finalizar) break;

        printf("\n\tMenu:\n1- Pastel de Papas.\n2- Guiso de lentejas.\n3- Locro.\n4- Salir.\n");

        scanf("%d", &opc);

        if (opc > 0 && opc < 4) {
            int num = (opc == 1) ? 2 : (opc == 2) ? 5 : 8;

            pid_t cheff = fork();

            if (cheff == 0) {
                instalar_handler_hijo();
                prctl(PR_SET_PDEATHSIG, SIGTERM);
                if (setpgid(0, 0) == -1)
                {
                    perror("setpgid");
                    exit(1);
                }

                printf("\n[Cheff] PID: %d\n", getpid());

                if (fork() == 0) cortarIngredientes(N, num, mesa, semA, semB);
                if (fork() == 0) cocinarIngredientes(N, num, mesa, semB, semC);
                if (fork() == 0) picarIngredientes(N, num, mesa, semC, semD);
                if (fork() == 0) emplatarIngredientes(N, mesa, semD, semA);

                for (int i = 0; i < HIJOS; i++) wait(NULL);
                exit(0);
            }

            // En el padre
            if (setpgid(cheff, cheff) == -1) {
                perror("setpgid padre");

            }
            cheffsActivos[cheffsCount++] = cheff;

            waitpid(cheff, NULL, 0);
            memcpy(mesa->datos, datos_iniciales, sizeof(datos_iniciales));

            platos[opc - 1]++;
        }
        else if (opc == 4) {
            printf("\n--- Informe final ---\n");

            for (int i = 0; i < 3; i++) {
                printf("Se pidieron en total del plato %d: %d\n", i + 1, platos[i]);
            }

            printf("\nInteracciones por cocinero:\n");

            printf("Cortar: %d\n", mesa->interacciones[0]);
            printf("Picar: %d\n", mesa->interacciones[1]);
            printf("Cocinar: %d\n", mesa->interacciones[2]);
            printf("Emplatar: %d\n", mesa->interacciones[3]);

            liberarRecursos();
        }
        else {
            printf("Opción inválida. Ingrese nuevamente...\n");

        }
    }

    return 0;
}

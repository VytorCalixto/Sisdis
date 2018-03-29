#include <stdio.h>
#include <stdlib.h>
#include "smpl.h"

// Eventos
#define TEST 1
#define FAULT 2
#define REPAIR 3

// Descritor do nodo
typedef struct {
    int id;
    // Memória local do nodo aqui
    int *state;
} tnodo;

tnodo *nodo;


void resetaState(tnodo *nodos, int N) {
    for(int i = 0; i < N; ++i) {
        for(int j = 0; j < N; ++j) {
            nodo[i].state[j] = -1;
        }
    }
}

void imprimeState(tnodo *nodos, int N, int i) {
    printf("STATE do nodo %d\n", i);
    for(int k = 0; k < N; ++k) {
        printf("\tNodo %d: %d\n", k, nodos[i].state[k]);
    }
}

void teste(tnodo *nodos, int N, int i) {
    int j = (i + 1) % N;
    printf("Nodo %d irá testar o nodo %d\n", i, j);
    int s = status(nodos[j].id);
    while(s != 0 && j != i) {
        printf("\tNodo %d está falho\n", j);
        nodos[i].state[j] = 1;
        j = (j + 1) % N;
        printf("Nodo %d irá testar o nodo %d\n", i, j);
        s = status(nodos[j].id);
    }
    printf("\tNode %d está não-falho\n", j);
    nodos[i].state[j] = 0;

    // Atualiza STATE
    int k;
    for (int k = 0; k < N; ++k) {
        if(k != j) {
            nodos[i].state[k] = nodos[j].state[k];
        }
    }

    imprimeState(nodos, N, i);
}

int main(int argc, char* argv[]) {
    static int N, token, event, r, i;
    static char fa_name[5];
    static float testInterval = 30.0;

    if(argc != 2) {
        puts("Uso correto:\n\t tempo <num-nodos>");
        exit(1);
    }

    N = atoi(argv[1]);
    smpl(0, "program tempo");
    reset();
    stream(1);
    nodo = (tnodo *) malloc(sizeof(tnodo) * N);

    for(i = 0; i < N; ++i) {
        memset(fa_name, '\0', 5);
        sprintf(fa_name, "%d", i);
        nodo[i].id = facility(fa_name, 1);
        nodo[i].state = (int *) malloc(sizeof(int) * N);
    }

    resetaState(nodo, N);

    // Schedule inicial
    for(i = 0; i < N; ++i) {
        schedule(TEST, testInterval, i);
    }
    schedule(FAULT, 31.0, 1);
    schedule(REPAIR, 69.0, 1);

    while(time() <= (N*testInterval)) {
        cause(&event, &token);
        switch(event) {
            case TEST:
                if(status(nodo[token].id) != 0) break; //falho!
                printf("O nodo %d vai testar no tempo %5.1f\n", token, time());
                teste(nodo, N, token);
                schedule(TEST, testInterval, token);
                break;
            case FAULT:
                r = request(nodo[token].id, token, 0);
                if(r != 0) {
                    puts("Impossível falhar");
                    exit(1);
                }
                printf("O nodo %d falhou no tempo %5.1f\n", token, time());
                break;
            case REPAIR:
                release(nodo[token].id, token);
                printf("O nodo %d recuperou no tempo %5.1f\n", token, time());
                schedule(TEST, testInterval, token);
                break;
        }
    }
    
    free(nodo);

    return 0;
}

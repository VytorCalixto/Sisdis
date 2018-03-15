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

void teste(tnodo *nodos, int N, int i) {
    int j = (i + 1) % N;
    printf("Nodo %d irá testar o nodo %d\n", i, j);
    int s = status(nodos[j].id);
    while(s != 0 && j != i) {
        printf("Nodo %d está falho\n", j);
        nodos[i].state[j] = 1;
        j = (j + 1) % N;
        printf("Nodo %d irá testar o nodo %d\n", i, j);
        s = status(nodos[j].id);
    }
    printf("Node %d está não-falho\n", j);
    nodos[i].state[j] = 0;
}

int main(int argc, char* argv[]) {
    static int N, token, event, r, i;
    static char fa_name[5];

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
        int j;
        for (j = 0; j < N; ++j) {
            nodo[i].state[j] = -1;
        }
    }

    // Schedule inicial
    for(i = 0; i < N; ++i) {
        schedule(TEST, 30.0, i);
    }
    schedule(FAULT, 5.0, 2);
    schedule(FAULT, 31.0, 1);
    schedule(REPAIR, 59.0, 1);
    schedule(REPAIR, 59.0, 2);

    while(time() < 130.0) {
        cause(&event, &token);
        switch(event) {
            case TEST:
                if(status(nodo[token].id) != 0) break; //falho!
                printf("O nodo %d vai testar no tempo %5.1f\n", token, time());
                teste(nodo, N, token);
                schedule(TEST, 30.0, token);
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
                schedule(TEST, 30.0, token);
                break;
        }
    }
    
    free(nodo);

    return 0;
}

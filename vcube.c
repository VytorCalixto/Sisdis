#include <stdio.h>
#include <stdlib.h>
#include "smpl.h"
#include <math.h>
#include "cisj.c"

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
            nodo[i].state[j] = 0;
        }
    }
}

void imprimeState(tnodo *nodos, int N, int i) {
    printf("\tSTATE [ ", i);
    for(int k = 0; k < N; ++k) {
        printf("%d ", nodos[i].state[k]);
    }
    printf("]\n");
}

node_set* getTestNodes(tnodo *nodos, int N, int i) {
    int totalClusters = (int) ceil(log2(N));
    node_set* test_nodes = set_new(N);// no máximo, se só há um nodo não-falho, ele testa todos os outros
    int offset = 0;
    for(int j = 0; j < N; ++j) {
        if(j == i) continue;
        for(int k = 1; k <= totalClusters; ++k) {
            node_set* cluster = cis(j, k);
            for(int l = 0; l < cluster->size; ++l) {
                // "Primeiro" nodo do cluster
                int first = cluster->nodes[l];
                // Incrementa a cada novo evento, sendo que nodos não-falhos são inicializados com 0, logo (par=não falho, ímpar=falho)
                if(nodos[i].state[first] >= 0 && nodos[i].state[first] % 2 == 0 && first != i) { // Se o primeiro está não-falho, o nodo i não testa j
                    break;
                }

                if(first == i) { // Se nos "encontramos", o nodo i testa j
                    set_insert(test_nodes, j);
                }
            }
        }
    }

    return test_nodes;
}

void teste(tnodo *nodos, int N, int i, node_set* testNodes) {
    for(int j = 0; j < testNodes->offset; ++j) {
        int testNode = testNodes->nodes[j];
        int s = status(nodos[testNode].id);
        if(s != 0) { // Nodo está falho
            if(nodos[i].state[testNode] == -1) {
                nodos[i].state[testNode] = 1;
            } else if(nodos[i].state[testNode] % 2 == 0) { // Se par, estava não-falho antes
                ++nodos[i].state[testNode];
            }
        } else { // Nodo está não falho
            if(nodos[i].state[testNode] == -1) {
                nodos[i].state[testNode] = 0;
            } else if(nodos[i].state[testNode] % 2 == 1) { // Se ímpar, estava falho antes
                ++nodos[i].state[testNode];
            }
            // Busca novidades
            for(int k = 0; k < N; ++k) {
                if(nodos[testNode].state[k] > nodos[i].state[k]) { // Se é maior, tem novidade
                    nodos[i].state[k] = nodos[testNode].state[k];
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    static int N, token, event, r, i;
    static char fa_name[5];
    static float testInterval = 20.0;

    if(argc != 2) {
        puts("Uso correto:\n\t vcube <num-nodos>");
        exit(1);
    }

    N = atoi(argv[1]);
    smpl(0, "program vcube");
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
    schedule(FAULT, 22.0, 1);
    schedule(REPAIR, 42.0, 1);
    schedule(FAULT, 64.0, 3);

    int eventDiagnosis = 0;
    int testsCounter = 0;

    while(time() <= (4*testInterval)) {
        cause(&event, &token);
        switch(event) {
            case TEST:
                if(status(nodo[token].id) != 0) break; //falho!
                // Obtém os nodos de teste
                node_set* tokenTestNodes = getTestNodes(nodo, N, token);
                printf("- Nodo %d irá testar nodos ", token);
                for(int k = 0; k < tokenTestNodes->offset; ++k) {
                    printf("%d, ", tokenTestNodes->nodes[k]);
                }
                printf("em %5.1f\n", time());
                teste(nodo, N, token, tokenTestNodes);
                imprimeState(nodo, N, token);
                schedule(TEST, testInterval, token);
                if(eventDiagnosis > 0) testsCounter++;
                break;
            case FAULT:
                r = request(nodo[token].id, token, 0);
                if(r != 0) {
                    puts("Impossível falhar");
                    exit(1);
                } else {
                    if(eventDiagnosis == 1) {
                        puts("****************************************************************");
                        printf("%d testes executados até diagnóstico", testsCounter);
                    }
                    printf("\n===FALHA NO NODO %d EM %5.1f===\n\n", token, time());
                    testsCounter = 0;
                    eventDiagnosis = 1;
                }
                break;
            case REPAIR:
                release(nodo[token].id, token);
                schedule(TEST, testInterval, token);
                puts("****************************************************************");
                printf("%d testes executados até diagnóstico", testsCounter);
                printf("\n===RECUPERAÇÃO DO NODO %d em %5.1f===\n\n", token, time());
                testsCounter = 0;
                eventDiagnosis = 1;
                break;
        }
    }
    
    free(nodo);

    return 0;
}

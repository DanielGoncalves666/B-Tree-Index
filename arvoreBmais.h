/*
    Implementação de índice baseado em árvore B+.

    by Daniel Gonçalves 2022
*/

#ifndef __ARVORE_B_MAIS_H
#define __ARVORE_B_MAIS_H

#include<stdbool.h>

const double OCUPACAO = 0.5; // ocupação das folhas à princípio
const int TAM = 512; // tamanho das páginas
const int INDICADOR = 02658093618; // indica que o arquivo ja foi convertido 

// ================ Estruturas Gerais ================ //

/**
 * Estrutura para o Registro
*/
typedef struct{
    int nseq;
    char text[46]; // textos de até 45 caracteres
}registro;

const int N = (TAM - sizeof(folhaDisco)) / sizeof(registro); // quantidade de registros em uma folha

/**
 * Estrutura para o Rid
*/
typedef struct{
    int pagina;
    int slot;
}rid;

/**
 * Estrutura para entradas da estratégia 2
*/
typedef struct{
    int chave;
    rid r;
}entrada2;

/**
 * Estrutura para entradas da estratégia 3
*/
typedef struct{
    int chave;
    int qtd; 
        // a lista de rids é armazenada logo em sequência.
}entrada3;

// ================ Estruturas para nós ================ //

/**
 * Estrutura nó para armazenamento em disco
*/
typedef struct{
    bool filhosSaoFolha;
    int ocupacao;
    int pai;

    int ponteiros[N + 1]; // posição do arquivo onde a estrutura do nó em disco começa
    int chaves[N]; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    // N registros, cabe mais chaves nos nós
}noDisco;

/**
 * Estrutura nó para manipulação em memória
*/
typedef struct{
    noDisco no;
    bool noAlterado;

    noMemoria *pai;
    void **filhos;
}noMemoria;

// ================ Estruturas para folhas ================ //

/**
 * Estrutura folha em disco
*/
typedef struct{
    int ocupacao;
    long long pai, ant, prox;
}folhaDisco;

/**
 *  Estrutura folha em memória
*/
typedef struct{
    folhaDisco folha;
    registro *todos;
    bool folhaAlterada;

    noMemoria *pai;
    folhaDisco *ant, *prox;
}folhaMemoria;
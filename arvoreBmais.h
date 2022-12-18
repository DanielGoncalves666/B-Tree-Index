/*
    Implementação de índice baseado em árvore B+.

    by Daniel Gonçalves 2022
*/

#ifndef __ARVORE_B_MAIS_H
#define __ARVORE_B_MAIS_H

#include<stdbool.h>

const double OCUPACAO = 0.5; // ocupação das folhas à princípio
const int TAM = 512; // tamanho das páginas
const int INDICADOR = 2658093618; // indica que o arquivo ja foi convertido 

// ================ Estruturas Gerais ================ //

/**
 * Estrutura para o Registro
*/
typedef struct{
    int nseq;
    char text[46]; // textos de até 45 caracteres
}registro;

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
}noDisco; // ponteiros e chaves armazenados separadamente na página

/**
 * Estrutura nó para manipulação em memória
*/
typedef struct noMemoria{
    noDisco no;
    void *conteudo;
    bool noAlterado;

    struct noMemoria *pai;
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
    void *conteudo;
    bool folhaAlterada;

    noMemoria *pai;
    folhaDisco *ant, *prox;
}folhaMemoria;

#endif
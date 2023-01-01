/*
    Implementação de índice baseado em árvore B+.

    by Daniel Gonçalves 2022
*/

#ifndef __ARVORE_B_MAIS_H
#define __ARVORE_B_MAIS_H

#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

const int PAGE_SIZE = 512; // tamanho de uma página em bytes
const double OCUPACAO = 0.5; // ocupação das folhas à princípio
const int TAM_BUFFERPOOL = 100; // quantidade de frame do bufferpool

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
 * Estrutura a ser armazenada no inicio do arquivo de indice
 *      Ocupa a primeira pagina do arquivo de indice.
*/
typedef struct{
    int qtdFolhas; // número de páginas no arquivo de dados (indica o numero da proxima folha a ser inserida)
    int qtdNos; // numero de páginas no arquivo de indice (indica o numero do proximo nó a ser inserido)
    int profundidade;
    int noRaiz;
}auxFile;

// ================ Estruturas para nós ================ //

/**
 * Estrutura nó para armazenamento em disco
*/
typedef struct{
    bool filhosSaoFolha;
    int self;
    int ocupacao; // a ocupacao for -1, o nó é inválido
    int pai; // numero da pagina
}noDisco; // ponteiros e chaves armazenados separadamente na página


int converterArquivo();
int loadAuxInfo();
int writeAuxInfo(int fd, auxFile info);
int abrirArquivos(int tipo, char nome[31]);
void menuOperacoes();

#endif
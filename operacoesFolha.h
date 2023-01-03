
#ifndef __OPERACOES_FOLHA_H
#define __OPERACOES_FOLHA_H

#include"arvoreBmais.h"

/**
 * Estrutura folha em disco
*/
typedef struct{
    int self;
    int ocupacao;
    int pai, ant, prox; // numero da pagina
}folhaDisco;

const int REG_FOLHA = (PAGE_SIZE - sizeof(folhaDisco)) / sizeof(registro); // quantidade de registros em uma folha

int buscaFolhaAltUm(int chave);

int buscaBinariaFolha(int chave, registro *entradas, int e, int d);

int inserirAltUm(registro r);

int transferirRegistros(int frameDestino, int frameOrigem, int qtd, bool inicio);

void carregarFolhaDisco(int frame, folhaDisco *f);
void gravarFolhaDisco(int frame, folhaDisco f);

int obterPrimeiraChaveFolha(int frame, int *primeiraChave);
int obterUltimaChaveFolha(int frame, int *ultimaChave);

int inserirOrdenadoFolha(int frame, registro r);
int removerOrdenadoFolha(int frame, int chave, int flag, registro *r);

int redistribuicaoInsercaoFolha(int frame, registro r);
int splitInsercaoFolha(int frame, registro r);

#endif

#ifndef __OPERACOES_NO_H
#define __OPERACOES_NO_H

#include"arvoreBmais.h"
#include<stdbool.h>

/**
 * Estrutura nó para armazenamento em disco
*/
typedef struct noDisco{
    bool filhosSaoFolha;
    int self;
    int ocupacao; // a ocupacao for -1, o nó é inválido
    int pai; // numero da pagina
}noDisco; // ponteiros e chaves armazenados separadamente na página

const int CHAVES_NO = (PAGE_SIZE - sizeof(noDisco) - sizeof(int)) / (2 * sizeof(int)); // quantidade de chaves em um nó

int buscaBinariaNo(int chave, int *vetor, int e, int d);
int buscaBinariaNoPonteiro(int ponteiro, int *vetor, int e, int d);

void carregarNoDisco(int frame, noDisco *n);
void gravarNoDisco(int frame, noDisco n);

void atualizarPai(int pont, bool folha, int novoPai);
int atualizarPonteiroNo(int frame, int oldPont, int newPont);

int obterPonteiroNo(int frame, int which);
int obterChaveNo(int frame, int which);
int obterChaveReferenteNo(int framePai, int ponteiro, int *chave);
int obterNoIrmao(int frame, int ponteiro, int which);

int atualizarEntradaNo(int frame, int ponteiro, int chaveNova);
int adicionarEntradaNo(int frame, int ponteiro, int chave);
int removerEntradaNo(int frame, int ponteiro);

int redistribuicaoInsercaoNo(int frame, int ponteiro, int chave);
int splitInsercaoNo(int frame, int ponteiro, int chave);

#endif
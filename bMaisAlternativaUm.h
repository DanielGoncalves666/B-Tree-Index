
#ifndef __B_MAIS_ALTERNATIVA_UM_H
#define __B_MAIS_ALTERNATIVA_UM_H

/**
 * Estrutura folha em disco
*/
typedef struct{
    int self;
    int ocupacao;
    int pai, ant, prox; // numero da pagina
}folhaDisco;

int inserirFolha(int frame);
int removerFolha(int fd, int pageID);
int inserirNo(int frame);
int removerNo(int fd, int pageID);

int buscaFolhaAltUm(int chave);

int buscaBinariaFolha(int chave, registro *entradas, int e, int d);
int buscaBinariaNo(int chave, int *vetor, int e, int d);
int buscaBinariaNoPonteiro(int ponteiro, int *vetor, int e, int d);

int inserirAltUm(registro r);

int transferirRegistros(int frameDestino, int frameOrigem, int qtd, bool inicio);

void carregarFolhaDisco(int frame, folhaDisco *f);
void gravarFolhaDisco(int frame, folhaDisco f);
void carregarNoDisco(int frame, noDisco *n);
void gravarNoDisco(int frame, noDisco n);

int obterPrimeiraChaveFolha(int frame, int *primeiraChave);
int obterUltimaChaveFolha(int frame, int *ultimaChave);

void atualizarPai(int pont, bool folha, int novoPai);
int atualizarPonteiroNo(int frame, int oldPont, int newPont);

int obterPonteiroNo(int frame, int which);
int obterChaveNo(int frame, int which);

int atualizarEntradaNo(int frame, int ponteiro, int chaveNova);
int adicionarEntradaNo(int frame, int ponteiro, int chave);
int removerEntradaNo(int frame, int ponteiro);

int inserirOrdenadoFolha(int frame, registro r);
int removerOrdenadoFolha(int frame, int chave, int flag, registro *r);

int redistribuicaoInsercaoFolha(int frame, registro r);
int splitInsercaoFolha(int frame, registro r);

int redistribuicaoInsercaoNo(int frame, int ponteiro, int chave);

int obterNoIrmao(int frame, int ponteiro, int which);
int obterChaveReferenteNo(int framePai, int ponteiro, int *chave);

#endif

#ifndef __B_MAIS_ALTERNATIVA_UM_H
#define __B_MAIS_ALTERNATIVA_UM_H

#include"arvoreBmais.h"

/**
 * Estrutura folha em disco
*/
typedef struct{
    int ocupacao;
    int pai, ant, prox; // numero da pagina
}folhaDisco;

const int REG_FOLHA = (PAGE_SIZE - sizeof(folhaDisco)) / sizeof(registro); // quantidade de registros em uma folha
const int CHAVES_NO = (PAGE_SIZE - sizeof(noDisco) - sizeof(int)) / (2 * sizeof(int)); // quantidade de chaves em um nó

#endif
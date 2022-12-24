
#ifndef __B_MAIS_ALTERNATIVA_TRES_H
#define __B_MAIS_ALTERNATIVA_TRES_H

#include"arvoreBmais.h"

/**
 * Entrada no diretório da página
*/
typedef struct{
    int offset;
    int tamanho;
}directoryEntry;

/**
 * Estrutura para entradas da estratégia 3
*/
typedef struct{
    int chave;
    int qtd; 
    // a lista de rids é armazenada logo em sequência.
}entrada3;

/**
 * Estrutura folha em disco
*/
typedef struct{
    int ocupacao;
    int inicioAreaLivre;
    int pai, ant, prox; // numero da pagina
    // diretorio e registros armazenados separadamente
}folhaDisco;





#endif
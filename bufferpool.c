/**
 * Structures definitions and functions about a bufferpool.
 * 
 * by Daniel Gonçalves 2022
*/


#include<stdlib.h>
#include<stdbool.h>
#include"arvoreBmais.h"
#include"bufferpool.h"

#define _LARGEFILE64_SOURCE    
#include <sys/types.h>
#include <unistd.h>

extern const int PAGE_SIZE;

// ===== Page Table Structures ===== //

struct pageID{
    int fd; // file descriptor of the page origin file
    int pid; // page id
};

typedef struct pageTableEntry{
    int frame; // número do frame onde a página está (acaba sendo redundante) 
    pageID p;
}pTE;

// ===== Buffer Table Structures ===== //

typedef struct bufferTableEntry{
    int frame; // número do frame (acaba sendo redundante)
    bool dirty_bit;
    int pin_count;
}bTE;

// ===== Pilha ===== //

typedef struct{ // Política de Substituição: MRU (pilha)
    int *vetor;
    int topo;
}Pilha;

// ===== Bufferpool Structure ===== //

struct bufferpool{
    void **frames;
    int length; // quantidade de frames

    pTE *pageTable; // deixar como uma lista ordenada melhoraria o desempenho
    bTE *bufferTable;
    Pilha p;
};

// ===== Functions Prototypes ===== //

int push(bufferpool *b, int frame);
int pop(bufferpool *b);
int cortarPilha(bufferpool *b, int frame);
void incrementarPinCount(bufferpool *b, int i);

// ===== Bufferpool Functions ===== //

/**
 * criarBufferPool
 * ----------------
 * Entrada: inteiro, indicando a capacidade do bufferpool
 * Processo: cria um novo bufferPool
 * Saída: ponteiro para estrutura bufferpool, em sucesso, NULL, em fracasso
*/
bufferpool *criarBufferPool(int qtd)
{
    bufferpool *novo = malloc(sizeof(bufferpool));
    if(novo != NULL)
    {
        novo->frames = malloc(PAGE_SIZE * qtd);
        novo->length = qtd;

        novo->bufferTable = malloc(sizeof(bTE) * qtd);
        novo->pageTable = malloc(sizeof(pTE) * qtd);
        for(int i = 0; i < qtd; i++)
        {
            novo->bufferTable[i].frame = i;
            novo->bufferTable[i].dirty_bit = false;
            novo->bufferTable[i].pin_count = 0;

            novo->pageTable[i].frame = i;
            novo->pageTable[i].p.fd = -1;
            novo->pageTable[i].p.pid = -1;    
        }

        novo->p.topo = qtd - 1;
        novo->p.vetor = malloc(sizeof(int) * qtd);
        for(int i = qtd - 1; i >= 0; i--) // empilha todos os frames
        {
            novo->p.vetor[qtd - i + 1] = i;
        }

    }

    return novo;
}

/**
 * desalocarBufferBool
 * --------------------
 * Entrada: ponteiro para estrutura bufferpool
 * Processo: Persiste quaisquer frames com dirty_bit ativo e desaloca todas as estruturas
 * Saída: 0, em falha, 1, em sucesso
*/
int desalocarBufferPool(bufferpool *b)
{
    if(b == NULL)
        return 0;

    // persiste os frames antes de desalocar as estruturas
    for(int i = 0; i < b->length; i++)
    {
        if(b->bufferTable[i].dirty_bit)
            persistirFrame(b,i);
    }

    free(b->frames);
    free(b->p.vetor);
    free(b->pageTable);
    free(b->bufferTable);
    free(b);

    return 1;
}

/**
 * persistirFrame
 * ---------------
 * Entrada: ponteiro para estrutura bufferpool
 *          inteiro, indicando o frame
 * Processo: Realiza o processo de persistência em disco do frame indicado
 * Saída: 0, em falha, 1, em sucesso
*/
int persistirFrame(bufferpool *b, int i)
{
    if(b == NULL || i < 0 || i >= b->length)
        return 0;
        
    pTE aux = b->pageTable[i];
    if( lseek64(aux.p.fd, aux.p.pid * PAGE_SIZE, SEEK_SET) == -1) // coloca o apontador do arquivo no ínicio da página
        return 0;
    if( write(aux.p.fd, &(b->frames[i]), PAGE_SIZE) == -1) // sobreescreve as mudanças em disco
        return 0;
    
    desativarDirtyBit(b,i);
    cortarPilha(b,i);
    push(b,i);

    return 1;
}

/**
 * push
 * -----
 * Entrada: ponteiro pra estrutura bufferpool
 *          inteiro, indicando o elemento a ser colocado na pilha
 * Processo: empilha frame se a pilha não estiver cheia
 * Saída: 0, em falha, 1, em sucesso
*/
int push(bufferpool *b, int frame)
{
    Pilha p = b->p;
    if(p.topo == b->length -1)
        return 0; // pilha cheia

    p.topo++;
    p.vetor[p.topo] = frame;

    return 1;
}

/**
 * pop
 * ----
 * Entrada: ponteiro para estrutura bufferpool
 * Processo: Faz o pop do elemento do topo da pilha presente na estrutura bufferpool
 * Saída: -1, em falha, inteiro não negativo, em sucesso
*/
int pop(bufferpool *b)
{
    Pilha p = b->p;
    if(p.topo == -1)
        return -1; // pilha vazia
    
    int aux = p.vetor[p.topo];
    p.topo--;

    return aux;
}

/**
 * cortarPilha
 * ------------
 * Entrada: ponteiro para estrutura bufferpool
 *          inteiro, indicando o frame que deve ser removido da pilha
 * Processo: Varre a pilha a procura do frame indicado e o remove, realizando as operações necessárias para manter a integridade da pilha
 * Saída: 0, em falha, 1, em sucesso
*/
int cortarPilha(bufferpool *b, int frame)
{
    Pilha p = b->p;
    
    int i = -1;
    while(++i <= p.topo && p.vetor[i] != frame);

    if(i > p.topo)
        return 0;

    for(int h = i + 1; h <= p.topo; h++, i++)
        p.vetor[i] = p.vetor[h];
    
    p.topo--;

    return 1;
}

/**
 * carregarPagina
 * ---------------
 * Entrada: ponteiro para estrutura bufferpool
 *          inteiro, indicando o file descriptor do arquivo
 *          inteiro, indicando a página
 * Processo: Verifica se a página do arquivo solicitado já está no bufferpool, carregando ela e substituindo um frame utilizando MRU, caso não
 *           seja o caso. 
 * Saída: -1, em falha, inteiro não negativo em sucesso, indicando o frame
*/
int carregarPagina(bufferpool *b, int fd, int pid)
{
    pageID p = {fd,pid};
    pageID aux;

    if(b == NULL || fd == -1 || pid == -1)
        return -1;

    for(int i = 0; i < b->length; i++)
    {
        aux = b->pageTable[i].p;
        if(aux.fd == p.fd && aux.pid == p.pid) // página já carregada no bufferpool
        {
            incrementarPinCount(b,i);
            return i;
        }
    }

    // a página não foi encontrada no bufferpool
    int retorno = liberarFrame(b);
    
    lseek64(p.fd, p.pid * PAGE_SIZE, SEEK_SET);
    read(p.fd, b->frames[retorno], sizeof(PAGE_SIZE));
    b->pageTable[retorno].p = p;

    incrementarPinCount(b,retorno); // se chegou até aqui então retorno tinha pin_count = 0

    cortarPilha(b,retorno);

    return retorno;
}

/**
 * liberarFrame
 * ----------------
 * Entrada: ponteiro para estrutura bufferpool
 * Processo: Faz pop na pilha do bufferpool, persistindo o frame obtido se necessário, liberando-o para outro uso.
 * Saída: -1, em falha, inteiro não negativo indicando o frame livre, em sucesso
*/
int liberarFrame(bufferpool *b)
{
    int retorno = pop(b);
    if(retorno == -1)
        return -1; // não há espaço no bufferpool, aborta

    if(b->bufferTable[retorno].dirty_bit)
        persistirFrame(b, retorno);

    b->bufferTable[retorno].pin_count++; // incrementa o pin count
    
    return retorno;
}

/**
 * atualizarPageTable
 * -------------------
 * Entrada: ponteiro para estrutura bufferpool
 *          inteiro, indicando o frame
 *          inteiro, indicando o file descriptor do arquivo
 *          inteiro, indicando a página
 * Processo: Atualiza o pageID do frame indicado
 * Saída: nenhuma
*/
void atualizarPageTable(bufferpool *b, int frame, int fd, int pid)
{
    b->pageTable[frame].p.fd = fd;
    b->pageTable[frame].p.pid = pid;    
}

/**
 * obterAcessoFrame
 * -----------
 * Entrada: ponteiro para estrutura bufferpool
 *          inteiro, indicando o frame que se quer obter o conteudo
 * Processo: Acessa os frames do bufferpool e retorna o conteúdo do frame indicado
 * Saída: ponteiro genérico
*/
void *obterAcessoFrame(bufferpool *b, int i)
{
    return b->frames[i];
}

/**
 * ativarDirtyBit
 * ---------------
 * Entrada: ponteiro para estrutura bufferpool
 *          inteiro, indicando o frame que se quer ativar o dirty_bit
 * Processo: Ativa o dirty_bit do frame indicado
 * Saída: nada
*/
void ativarDirtyBit(bufferpool *b, int i)
{
    b->bufferTable[i].dirty_bit = true;
}

/**
 * desativarDirtyBit
 * ---------------
 * Entrada: ponteiro para estrutura bufferpool
 *          inteiro, indicando o frame que se quer desativar o dirty_bit
 * Processo: Ativa o dirty_bit do frame indicado
 * Saída: nada
*/
void desativarDirtyBit(bufferpool *b, int i)
{
    b->bufferTable[i].dirty_bit = false;
}


/**
 * incrementarPinCount
 * --------------------
 * Entrada: ponteiro para estrutura bufferpool
 *          inteiro, indicando um frame
 * Processo: Incrementa o pin_count do frame indicado
 * Saída: nenhuma
*/
void incrementarPinCount(bufferpool *b, int i)
{
    // incrementa quando carrega a pagina, obrigatoriamente
    int aux = ++(b->bufferTable[i].pin_count);
    if(aux == 1)
        cortarPilha(b,i);
}

/**
 * decrementarPinCount
 * --------------------
 * Entrada: ponteiro para estrutura bufferpool
 *          inteiro, indicando um frame
 * Processo: decrementa o pin_count do frame indicado
 * Saída: nenhuma
*/
void decrementarPinCount(bufferpool *b, int i)
{
    // o decremento é de responsabilidade do usuário
    if(b->bufferTable[i].pin_count == 0)
        return;

    int aux = --(b->bufferTable[i].pin_count);
    if(aux == 0)
        push(b,i);
}



// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// no processo de abortagem, tem que garantir a integridade da operação
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


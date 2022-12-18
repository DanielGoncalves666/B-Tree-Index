/**
 * Structures definitions and functions about a bufferpool.
 * 
 * by Daniel Gonçalves 2022
*/


#include<stdlib.h>
#include<stdbool.h>
#include"bufferpool.h"

// Bibliotecas requeridas pelas system calls
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

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
    if( lseek(aux.p.fd, aux.p.pid * PAGE_SIZE, SEEK_SET) == -1) // coloca o apontador do arquivo no ínicio da página
        return 0;
    if( write(aux.p.fd, &(b->frames[i]), PAGE_SIZE) == -1) // sobreescreve as mudanças em disco
        return 0;
    
    b->bufferTable[i].dirty_bit = false;
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
 *          estrutura pageID, indicando qual página de qual arquivo deve ser carregada
 * Processo: Verifica se a página do arquivo solicitado já está no bufferpool, carregando ela e substituindo um frame utilizando MRU caso não
 *           seja o caso. 
 * Saída: -1, em falha, inteiro não negativo em sucesso, indicando o frame
*/
int carregarPagina(bufferpool *b, pageID p)
{
    pageID aux;

    for(int i = 0; i < b->length; i++)
    {
        aux = b->pageTable[i].p;
        if(aux.fd == p.fd && aux.pid == p.pid) // página já carregada no bufferpool
        {
            b->bufferTable[i].pin_count++;
            return i;
        }
    }

    // a página não foi encontrada no bufferpool
    int retorno = pop(b);
    if(retorno == -1)
        return -1; // não há espaço no bufferpool, aborta

    if(b->bufferTable[retorno].dirty_bit)
        persistirFrame(b, retorno);
    
    lseek(p.fd, p.pid * PAGE_SIZE, SEEK_SET);
    read(p.fd, b->frames[retorno], sizeof(PAGE_SIZE));
    b->pageTable[retorno].p = p;

    bTE att = {retorno, false, 1};
    b->bufferTable[retorno] = att;

    cortarPilha(b,retorno);

    return retorno;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// no processo de abortagem, tem que garantir a integridade da operação
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


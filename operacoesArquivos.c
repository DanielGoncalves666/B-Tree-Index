

#include"arvoreBmais.h"
#include"operacoesArquivos.h"
#include"bufferpool.h"
#include"operacoesFolha.h"
#include"operacoesNo.h"

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>

extern int fd_Dados;
extern int fd_Indice;
extern bufferpool *b;
extern auxFile auxF;

// O BULKLOANDING PRECISA CONSIDERAR OS PONTEIROS
// OU IMPLEMENTAR UM DOS ALGORITMOS DE ORDENAÇÃO EM DISCO, ORDENAR AS FOLHAS ANTES E ENTÃO USAR O BULKLOADING

/**
 * inserirFolha
 * -------------
 * Entrada: inteiro, indicando o frame do bufferpool que a pagina a ser gravada no arquivo de dados está
 *              A folha deve ter sido preenchida antes de ser passada
 * Processo: Insere uma folha passado no final do arquivo de dados
 * Saída: -1, em falha, número inteiro não negativo indicando o número da página, em sucesso
*/
int inserirFolha(int frame)
{
    if(frame < 0) // fazer a verificação no outro extremo
        return -1;

	int pageID = auxF.qtdFolhas++; //obtém o pageID da pagina inserida e já incrementa o contador de folhas

	lseek64(fd_Dados, 0, SEEK_END);
	write(fd_Dados, obterAcessoFrame(b,frame) , PAGE_SIZE);
        
    atualizarPageTable(b, frame, fd_Dados, pageID);

    //ESCREVER EM DISCO

	return pageID;
}

/**
 * removerFolha
 * -------------
 * Entrada: inteiro, indicando file descriptor
 *          inteiro, indicando a página a ser removida
 * Processo: Remove (invalida) a folha dada no arquivo dado. No processo atualiza os ponteiros dos vizinhos.
 * Saída: 0, em falha, 1, em sucesso
*/
int removerFolha(int fd, int pageID)
{
    folhaDisco centralLeef;
    int frameCentral = -1;
    void *fc = NULL; 

    folhaDisco auxLeef;
    void *f = NULL;

    int frameEsquerdo = -1;
    int frameDireito = -1;

    if(fd == -1 || pageID < 0 || pageID >= auxF.qtdFolhas)
        return 0;

    frameCentral = carregarPagina(b,fd,pageID);
    if(frameCentral == -1)
        return 0;

    fc = obterAcessoFrame(b,frameCentral);
    carregarFolhaDisco(frameCentral,&centralLeef);

    if(centralLeef.ocupacao == -1)
        return 0;
    else
        centralLeef.ocupacao = -1; // invalida a pagina, ela continua existindo no arquivo mas é ignorada. Com isso a estrutura auxliar não é atualizada

    gravarFolhaDisco(frameCentral,centralLeef);

    frameEsquerdo = carregarPagina(b,fd,centralLeef.ant);
    frameDireito = carregarPagina(b,fd,centralLeef.prox);

    if(frameEsquerdo != -1)
    {
        f = obterAcessoFrame(b,frameEsquerdo);
        carregarFolhaDisco(frameEsquerdo,&auxLeef);

        auxLeef.prox = centralLeef.prox;
        gravarFolhaDisco(frameEsquerdo,auxLeef);
    }

    if(frameDireito != -1)
    {
        f = obterAcessoFrame(b,frameDireito);
        carregarFolhaDisco(frameDireito,&auxLeef);
    
        auxLeef.ant = centralLeef.ant;
        gravarFolhaDisco(frameDireito,auxLeef);
    }

    persistirFrame(b,frameCentral);
    persistirFrame(b,frameEsquerdo);
    persistirFrame(b,frameDireito);

    decrementarPinCount(b,frameCentral);
    decrementarPinCount(b,frameEsquerdo);
    decrementarPinCount(b,frameDireito);

    return 1;
}

/**
 * inserirNo
 * ----------
 * Entrada: inteiro, indicando o frame do bufferpool que a pagina a ser gravada no arquivo de indice está
 *              Assume-se que o No está totalmente preenchido
 * Processo: Insere o nó passado no final do arquivo de dados
 * Saída: -1, em falha, inteiro não negativo, em sucesso
*/
int inserirNo(int frame)
{
    if(frame < 0) // fazer a verificação no outro extremo
        return -1;

	int pageID = auxF.qtdNos++; //obtém o pageID da pagina inserida e já incrementa o contador de nós

	lseek64(fd_Indice, 0, SEEK_END);
	write(fd_Indice, obterAcessoFrame(b,frame) , PAGE_SIZE);

    atualizarPageTable(b, frame, fd_Indice, pageID);

        // ESCREVER EM DISCO

	return pageID;
}

/**
 * removerNo
 * ----------
 * Entrada: inteiro, indicando file descriptor
 *          inteiro, indicando a página a ser removida
 * Processo: Remove (invalida) o nó cujo pageID foi passado no arquivo indicado.
 * Saída: 0, em falha, 1, em sucesso
*/
int removerNo(int fd, int pageID)
{
    if(fd == -1 || pageID == -1)
        return 0;

    int frame = carregarPagina(b,fd,pageID);
    void *f = obterAcessoFrame(b,frame);
    noDisco n;

    carregarNoDisco(frame, &n);

    n.ocupacao = -1; // invalida o nó 
    gravarNoDisco(frame,n);

    persistirFrame(b,frame);
    decrementarPinCount(b,frame);

    return 1;
}







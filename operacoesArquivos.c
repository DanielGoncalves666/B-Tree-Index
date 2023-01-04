
#include"arvoreBmais.h"
#include"operacoesArquivos.h"
#include"bufferpool.h"
#include"operacoesFolha.h"
#include"operacoesNo.h"

#include <stdlib.h>
#include <string.h>

#define _LARGEFILE64_SOURCE   // para usar lseek64
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

extern int fd_Dados;
extern int fd_Indice;
extern bufferpool *b;
extern auxFile auxF;

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

/**
 * converterArquivo
 * -------------------
 * Entrada: inteiro, indicando o file descriptor do arquivo de dados
 * Processo: converte um arquivo de dados, contendo registros justapostos, em um arquivo separado por páginas com 
 *           ocupacao especificado por OCUPACAO
 * Saída: -2, para arquivo já convertido, -1, para falha na criação de arquivo temporário, ou um inteiro não 
 *            negativo, indicando o file descritor do arquivo criado
*/
int converterArquivo()
{
    folhaDisco f;

    int fd_Temp = open("temp.txt", O_CREAT | O_RDWR | O_TRUNC | __O_LARGEFILE, S_IRWXU | S_IRWXO);
    if(fd_Temp != -1)
    {
        int bytesLidos = 0;
        int qtd = OCUPACAO * REG_FOLHA; // quantidade de registros em cada folha
    
        void *bufferLeitura = malloc(sizeof(registro) * qtd);
        void *bufferEscrita = malloc(PAGE_SIZE);

        lseek(fd_Dados, 0, SEEK_SET);
        lseek(fd_Temp, 0, SEEK_SET);

        int cont = 0;
        while( (bytesLidos = read(fd_Dados, bufferLeitura, sizeof(registro) * qtd)) != 0)
        {
            // entradas
            memcpy(bufferEscrita, bufferLeitura, sizeof(registro) * (bytesLidos / sizeof(registro)));

            f.ocupacao = bytesLidos/ sizeof(registro);
            f.ant = cont == 0 ? -1 : cont - 1;
            f.prox = (bytesLidos / sizeof(registro)) < sizeof(registro) * qtd ? -1 : cont + 1;
            f.pai = -1;
            f.self = cont;
            memcpy(bufferEscrita + (PAGE_SIZE - sizeof(folhaDisco)), &f, sizeof(folhaDisco)); // estrutura que armazena informações da folha

            write(fd_Temp, bufferEscrita, PAGE_SIZE);
        }

        close(fd_Dados);
    }

    return fd_Temp;
}

/**
 * loadAuxInfo
 * ------------
 * Entrada: nenhuma
 * Processo: Carrega as informações auxiliares do arquivo de indice aberto no momento
 * Saída: 0, em falha, 1, em sucesso
*/
int loadAuxInfo()
{
    if(fd_Indice == -1)
        return 0;

    lseek64(fd_Indice, 0, SEEK_SET);
    read(fd_Indice, &auxF, sizeof(auxFile));

    return 1;
}

/**
 * writeAuxInfo
 * -------------
 * Entrada: estrutura do tipo auxFile
 * Processo: Grava a estrutura de informações auxiliares passada no arquivo cujo file descriptor foi passado
 * Saída: 0, em fracasso, 1, em sucesso
*/
int writeAuxInfo(int fd, auxFile info)
{
    if(fd == -1)
        return 0;

    lseek64(fd,0,SEEK_SET);
    write(fd,&auxF,sizeof(auxFile));
}

/**
 * bulkLoading
 * -------------
 * Entrada: nenhuma
 * Processo: Realiza a criação de um índice por meio de bulkLoading.
 * Saída: nenhuma
*/
void bulkLoading()
{
    int frame = liberarFrame(b);
    void *p = obterAcessoFrame(b,frame);
    noDisco no;

    no.filhosSaoFolha = true;
    no.ocupacao = 0;
    no.pai = -1;

    no.self = inserirNo(frame);
    gravarNoDisco(frame,no);

    auxF.noRaiz = no.self;
    auxF.profundidade = 1;

    // determina a quantidade de folhas
    struct stat st;
    fstat(fd_Dados, &st);
    auxF.qtdFolhas = st.st_size / PAGE_SIZE;

    int i;
    int primeira;
    int retorno;

    atualizarPonteiroPosicaoNo(frame, 0, i);
    // como está vazio, precisamo colocar uma folha sem chave associada

    // i serve como PID da página, sem necessidade de consultar o self
    for(i = 0; i < auxF.qtdFolhas; i++)
    {
        // só funciona corretamente se nõ tiver ocorrido adição de novas folhas no finalou se um algoritmo de 
        //      ordenação tiver sido aplicado
        obterPrimeiraChaveFolha(i,&primeira); // determina a primeira chave da folha

        retorno = adicionarEntradaNo(frame,i,primeira);
    
        if(retorno == -1)
        {
            retorno = splitInsercaoNo(frame,i,primeira);
            persistirFrame(b,frame);
            frame = carregarPagina(b,fd_Indice,retorno); // novo nó
        }
    }

    writeAuxInfo(fd_Indice,auxF);
}
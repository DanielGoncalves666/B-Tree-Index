/*
    Implementação de índice baseado em árvore B+

    by Daniel Gonçalves 2022
*/

#include<stdio.h>
#include"arvoreBmais.h"
#include"bMaisAlternativaUm.h"
#include"bufferpool.h"

#define _LARGEFILE64_SOURCE   // para usar lseek64
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

extern const int REG_FOLHA;

int fd_Dados = -1; // armazena o file descriptor do arquivo que contém os dados 
int fd_Indice = -1; // armazena o file descriptor do arquivo que contém o índice
bufferpool *b = NULL; // armazena o ponteiro para o bufferpool
auxFile auxF; // armazena as inforamções auxiliares do índice (ocupa a primeira pagina do arquivo)
    
int main()
{
    b = criarBufferPool(TAM_BUFFERPOOL);
    if(b == NULL)
    {
        fprintf(stderr, "\nNão foi possível criar o bufferpool\n");
        return 1;
    }

    int fd = 0; // armazena temporariamente o file descriptor do arquivo criado durante a conversão
    short int op = 0, op1 = 0; // usados para entrada da escolha do usuário
    char nome[31]; // para uso do rename

    do{
        printf("\n ==== Menu ==== \n");
        printf(" 1 - Paginar arquivo de dados.\n");
        printf(" 2 - Criar índice baseado em um arquivo de dados paginado.\n");
        printf(" 3 - Criar índice vazio.\n");
        printf(" 4 - Operar sobre índice existente.\n");
        printf(" 5 - Encerrar.\n");

        scanf("%hd",&op);

        switch(op)
        {
            case 1:
                printf("\n Atenção, caso o arquivo a ser inserido já tenha sido paginado registros inválidos serão criados e ocorrerá perda de dados. \n");
                printf("\n O ato de paginar não cria a estrutura de índice.\n");
                
                if(abrirArquivos(1, nome) == 0)
                    break;

                if( (fd = converterArquivo()) != -1)
                {
                    rename("temp.txt",nome);
                    fd_Dados = fd;
                } 

                break;
            case 2:

                if(abrirArquivos(2, nome) == 0)
                    break;

                auxF.noRaiz = 1; // pagina do nó raiz
                auxF.profundidade = 0; // nada foi criado
                auxF.qtdFolhas = 0;
                auxF.qtdNos = 0;

                writeAuxInfo(fd_Indice,auxF);

                op1 = 0;
                do{
                    printf("\n Arquivo identificado! Qual tipo de índice deve ser criado? ");
                    printf(" Árvore B+:\n");
                    printf(" \t1 - Alternativa 1 em NSEQ\n");
                    printf(" \t2 - Alternativa 3 em TEXT\n");
                    printf(" \t3 - Sair.\n");

                    scanf("%hd",&op1);

                    switch(op1)
                    {
                        case 1:  

                            break;
                        case 2:

                            break;
                        case 3:
                            break;
                        default:
                    }
                    
                }while(op1 != 3);

                close(fd_Indice);
                close(fd_Dados);
                break;
            case 3:
                if(abrirArquivos(3, nome) == 0)
                    break;

                auxF.noRaiz = 1; // pagina do nó raiz
                auxF.profundidade = 0; // nada foi criado
                auxF.qtdFolhas = 0;
                auxF.qtdNos = 0;

                writeAuxInfo(fd_Indice,auxF);

                close(fd_Indice);
                close(fd_Dados);
                break;
            case 4:
                if(abrirArquivos(4, nome) == 0)
                    break;

                loadAuxInfo();

                menuOperacoes();
                break;
            case 5:
                break;
            default:
                fprintf(stderr,"\nOpção inválida.\n");
        }

    }while(op != 5);


    // todas as entradas estão no nível da folha

    desalocarBufferPool(b);

    return 0;
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
 * abrirArquivos
 * --------------
 * Entrada: inteiro, indicando quais arquivos devem ser criados ou apenas abertos
 *          string, onde será armazenado o nome do arquivo de dados
 * Processo: Baseado no tipo passado, realiza operações de abertura ou criação de arquivos de dados e indice
 *           tipo 1 - abrir arquivo de dados
 *           tipo 2 - abrir arquivo de dados e criar arquivo de indice
 *           tipo 3 - criar arquivos de dados e de indice
 *           tipo 4 - abrir arquivos de dados e de indice
 * Saída: 0, em falha, 1, em sucesso
*/
int abrirArquivos(int tipo, char nome[31])
{
    char string[31]; // string auxiliar, para construção dos nomes dos arquivos de indice
                   // arquivo de indice alternativa 1: indice_alt1_nome
                   // arquivo de indice alternativa 3: indice_alt3_nome
                   // arquivo auxiliar: aux_alt3_nome

    if(tipo < 1 || tipo > 4)
        return 0;

    printf("\nEntre com o nome do arquivo de dados: ");
    scanf("%30s",nome);

    if(tipo != 3)
        fd_Dados = open(nome, O_RDWR | __O_LARGEFILE, S_IRWXU | S_IRWXO); // abre
    else
        fd_Dados = open(nome, O_CREAT | O_RDWR | O_TRUNC | __O_LARGEFILE, S_IRWXU | S_IRWXO); // cria

    if(fd_Dados == -1)
    {
        fprintf(stderr, "\nArquivo não encontrado ou não foi possível criá-li.\n");
        return 0;
    }

    if(tipo != 1)
    {
        strcpy(string,"indice_alt1_");
        strcat(string,nome);

        if(tipo == 4)
            fd_Indice = open(string, O_RDWR | __O_LARGEFILE, S_IRWXU | S_IRWXO);
        else
            fd_Indice = open(string, O_CREAT | O_RDWR | O_TRUNC | __O_LARGEFILE, S_IRWXU | S_IRWXO);
        
        if(fd_Indice == -1)
        {
            fprintf(stderr, "\nNão foi possível abrir ou criar o arquivo de indice.\n");
            return 0;
        }
    }

    return 1;    
}


void menuOperacoes()
{
    int op = 0;

    do{
        printf("\n ==== Operações sobre índice ==== \n");
        printf(" 1 - Imprimir informações do índice\n");
        printf(" 2 - Buscar por uma entrada.\n");
        printf(" 3 - Inserir uma entrada.\n");
        printf(" 4 - Remover uma entrada.\n");
        printf(" 5 - Retornar.\n");

        switch(op)
        {
            case 1:

                break;
            case 2:

                break;
            case 3:

                break;
            case 4: 

                break;
            case 5:
                break;
            default:
                fprintf(stderr,"\nOpção inválida.\n");
        }
    }while(op != 5);
}






/*
    Páginas para o arquivo de dados.
        Formato de página para registro de tamanho fixo: Packed

    Páginas os nós do índice:
        Packed

    Slots ocupados --> Área livre --> N (número de slots ocupados)
        No caso das folhas, adição de ponteiros pra antes e depois

    Páginas para as folhas da árvore usando alternativa 3:
        Formato de página para registro de tamanho variável - slide 6 de formatação (presença de diretório no final)

    Slots (alocados de forma não necessariamente linear) --> área livre --> diretório (offset,tamanho) --> N (número de slots) --> ponteiro para início do espaço livre

*/

/*
    Programa baseado na primeira implementação de GDB. Reune o código que contempla apenas a criação do Heap file e suas operações.

    by Daniel Gonçalves 2022
*/

#define _LARGEFILE64_SOURCE   // para usar lseek64

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

// Bibliotecas requeridas pelas system calls
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int totalRegistros = 0; // armazena a quantidade de registros no arquivo criado.(inclui inválidos)
int totalValidos = 0; // armazena a quantidade de registros que são válidos

typedef struct{
    int NSEQ;
    char TEXT[46]; // textos de até 45 caracteres
}registro;
// por questões de memória, a estrutura ocupa 52 bytes, invés de 50 bytes.

/* Protótipos */
int calcularQtdRegistros(int capacidade); // ok
void gerarTextoAleatorio(registro *r); // ok

int createHeapFile(int qtdRegistros); // ok
int readRandom(int fd, int NSEQ); // ok
void isrtAtEnd(int fd); // ok
int updateRandom(int fd, int NSEQ, char *text); // ok
int deleteRandom(int fd, int NSEQ); // ok

int invalidaAleatorio(int fd, int total);// ok

int main()
{
    int tamanho = 0; // armazena a capacidade da memória RAM, em GB, da máquina de testes
    int arquivo = -1; // armazena o ponteiro para o arquivo criado

    printf("Entre com o tamanho, em MB, do arquivo que deverá ser gerado: ");
    scanf("%d", &tamanho);

    totalRegistros = calcularQtdRegistros(tamanho);
    totalValidos = totalRegistros;
    if(totalRegistros < 0)
    {
        fprintf(stderr,"\nValor inválido para o tamanho do arquivo.\n");
        return 1;
    }


    arquivo = createHeapFile(totalRegistros);
    if(arquivo == -1)
    {
        fprintf(stderr, "\n Não foi possível criar o arquivo.\n");
        return 1;
    }

    printf("\nQuantidade total de Registros: %d\n", totalRegistros);

    close(arquivo);
    return 0;
}

/*
calcularQtdRegistros
-----------------------
Entrada: inteiro, indicando a capacidade em GB da máquina de testes
Processo: calcula a quantidade de registros que cabem em um arquivo com 5 vezes a RAM da máquina de testes
Saída: inteiro, indicando a quantidade de registros. Em caso de falha, retorna um número negativo.  
*/
int calcularQtdRegistros(int capacidade)
{
    if(capacidade <= 0)
        return -1;

    unsigned long long total = capacidade  * pow(2,20); // 2^20 é 1MB = 1048576
    int qtd = total / sizeof(registro);

    return qtd;
}

/*
createHeapFile
----------------
Entrada: inteiro, indicando a quantidade de registros que o arquivo precisa ter
Processo: Cria um arquivo e o preenche com a quantidade de registros passada.
Saída: inteiro, indicando o file descriptor do arquivo criado. Um valor negativo é retornado em caso de falha.
*/
int createHeapFile(int qtdRegistros)
{
    int MR = 1024 * 1024; // Mega Registros 
    registro *r = (registro *) malloc(sizeof(registro) * MR);
    
    int fd = open("HeapFile.txt", O_CREAT | O_RDWR | O_TRUNC | __O_LARGEFILE, S_IRWXU | S_IRWXO);
    if(fd != -1)
    {
        int nseq = 0;
        for(int h = 0; h < qtdRegistros / MR; h++)
        {
            for(int i = 0; i < MR;i++)
            {   
                r[i].NSEQ = nseq;
                gerarTextoAleatorio(&r[i]);
                nseq++;
            }
            write(fd, r, sizeof(registro) * MR);
        }

        if(qtdRegistros % MR != 0)
        {
            for(int i = 0; i < qtdRegistros % MR; i++)
            {
                r[i].NSEQ = nseq;
                gerarTextoAleatorio(&r[i]);
                nseq++;
            }

            write(fd, r, sizeof(registro) * (qtdRegistros % MR));
        }
    }

    free(r);

    return fd;
}

/*
gerarTextoAleatorio
--------------------
Entrada: ponteiro para um registro
Processo: gera, aleatoriamente, o texto para o registro passado
Saída: nenhuma
*/
void gerarTextoAleatorio(registro *r)
{
    for(int i = 0; i < 45; i++)
    {
        r->TEXT[i] = rand() % 93 + 33;
    }

    r->TEXT[45] = '\0';
}

/*
readRandom
------------
Entrada: inteiro, indicando o file descriptor do arquivo com registros
         inteiro, indicando o número do registro
Processo: Imprime as informações do registro informado.
Saída: inteiro, 0 para falha, 1 para sucesso
*/
int readRandom(int fd, int NSEQ)
{
    if(NSEQ < 0 || NSEQ >= totalRegistros)
        return 0;

    registro r;

    lseek64(fd, NSEQ * sizeof(registro), SEEK_SET);
    read(fd,&r,sizeof(registro));
    printf("%d %s\n", r.NSEQ, r.TEXT);

    return 1;
}


/*
isrtAtEnd
----------
Entrada: inteiro, indicando o file descriptor do arquivo com registros
Processo: insere um novo registro no final do arquivo
Saída: nenhuma
*/
void isrtAtEnd(int fd)
{
    registro r;
    r.NSEQ = totalRegistros;
    gerarTextoAleatorio(&r);

    lseek64(fd, 0, SEEK_END);
    if( write(fd, &r, sizeof(registro)) != -1)
    {
        totalRegistros++;
        totalValidos++;
    }
}

/*
updateRandom
-------------
Entrada: inteiro, indicando o file descriptor do arquivo com registros
         inteiro, indicando o número de sequência do registro a ser atualizado
         string, indicando o novo texto
Processo: Atualiza  registro NSEQ, se ele existir. Se o vetor recebido for maior que 45 caracteres o resto é ignorado.
Saída: inteiro, 0 para falha, 1 para sucesso
*/
int updateRandom(int fd, int NSEQ, char *text)
{
    if(NSEQ < 0 || NSEQ >= totalRegistros)
        return 0;

    registro r;

    lseek64(fd, NSEQ * sizeof(registro), SEEK_SET);
    read(fd, &r, sizeof(registro));

    int i;
    for(i = 0; i < 45 && i < strlen(text); i++)
    {
        r.TEXT[i] = text[i];
    }
    r.TEXT[i] = '\0';

    lseek64(fd, - sizeof(registro), SEEK_CUR);
    write(fd, &r, sizeof(registro));

    return 1;
}

/*
deleteRandom
--------------
Entrada: inteiro, indicando o file descriptor do arquivo com registros
         inteiro, indicando o número de sequência do registro a ser deletado
Processo: Deleta o arquivo com NSEQ especificado (torna o NSEQ um número negativo)
Saída: inteiro, 0 para falha, 1 para sucesso, 2 para registro deletado que foi restaurado
*/
int deleteRandom(int fd, int NSEQ)
{
    if(NSEQ < 0 || NSEQ >= totalRegistros)
        return 0;

    registro r;

    lseek64(fd, NSEQ * sizeof(registro), SEEK_SET);
    read(fd,&r, sizeof(registro));

    if(r.NSEQ <= 0) // inválidos permanecem válidos. Registro NSEQ=0 não pode ser invalidado pelo método utilizado
        return 2;

    r.NSEQ = abs(r.NSEQ) * -1;
    lseek64(fd, - sizeof(registro), SEEK_CUR);
    write(fd, &r, sizeof(registro));

    totalValidos--;
    return 1;
}

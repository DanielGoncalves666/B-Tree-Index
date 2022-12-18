/*
    Implementação de índice baseado em árvore B+

    by Daniel Gonçalves 2022
*/
#define _LARGEFILE64_SOURCE   // para usar lseek64

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include"arvoreBmais.h"

// Bibliotecas requeridas pelas system calls
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

const long long noRaiz = 0; //por padrão o nó raiz se encontrará no início do arquivo de índice
const int N = (TAM - sizeof(folhaDisco)) / sizeof(registro); // quantidade de registros em uma folha

int fd_Dados = -1; // armazena o file descriptor do arquivo que contém os dados 
int fd_Indice = -1; // armazena o file descriptor do arquivo que contém o índice

noMemoria *raiz = NULL; // ponteiro para a raiza da árvore B+

int main()
{
    int fd = 0;
    short int op = 0, op1 = 0; // usados para entrada da escolha do usuário
    char nome1[31], nome2[31]; // nome do arquivo de dados e do arquivo de índice, respectivamente.

    do{
        printf("\n ==== Menu ==== \n");
        printf(" 1 - Criar índice baseado em um arquivo.\n");
        printf(" 2 - Carregar índice.\n");
        printf(" 3 - Operar sobre índice carregado/criado.\n");
        printf(" 4 - Encerrar.\n");

        scanf("%hd",&op);

        switch(op)
        {
            case 1:
                printf("\nEntre com o nome do arquivo de dados: ");
                scanf("%30s",nome1);

                fd_Dados = open(nome1, O_RDWR | __O_LARGEFILE, S_IRWXU | S_IRWXO);
                if(fd_Dados == -1)
                {
                    fprintf(stderr, "\nArquivo não encontrado.\n");
                    break;
                }

                printf("Entre com o nome do arquivo do índice: ");
                scanf("%30s", nome2);

                fd_Indice = open(nome2, O_CREAT | O_RDWR | O_TRUNC | __O_LARGEFILE, S_IRWXU | S_IRWXO);
                if(fd_Indice == -1)
                {
                    fprintf(stderr, "\nNão foi possível criar/truncar o arquivo.\n");
                    break;
                }

                op1 = 0;
                do{
                    printf("\nArquivo Identificado. Qual tipo de índice deve ser criado? ");
                    printf(" Árvore B+:");
                    printf(" \t1 - Alternativa 1 em NSEQ\n");
                    printf(" \t2 - Alternativa 3 em TEXT\n");
                    printf(" \t3 - Sair.\n");

                    scanf("%hd",&op1);

                    switch(op1)
                    {
                        case 1:
                            if( (fd = converterArquivo()) >= 0)
                            {
                                rename("temp.txt",nome1);
                                fd_Dados = fd;
                            }    


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
            case 2:
                printf("Entre com o nome do arquivo do índice: ");
                scanf("%30s", nome2);

                fd_Indice = open("Indice.txt", O_RDWR | __O_LARGEFILE, S_IRWXU | S_IRWXO);
                if(fd_Indice == -1)
                {
                    fprintf(stderr,"\nNão foi possível abrir o arquivo de índice.\n");
                    return 1;
                }

                break;
            case 3:
                menuOperacoes();
                break;
            case 4:
                break;
            default:
                fprintf(stderr,"\nOpção inválida.\n");
        }

    }while(op != 4);

    
    // noMemoria raiz;
    // lseek64(fd, 0, SEEK_SET);
    // read(fd, &raiz, sizeof(no));
    // a raiz carregada é a versão dela em disco. Os ponteiros apontam para o disco, mas queremos também construir a árvore em memória

    // todas as entradas estão no nível da folha

    // se formos usar um formato de arquivo especifico, invés de um arquivo sem formatação, será melhor alterar as referências entre nós e folhas para rids

    return 0;
}



void menuOperacoes()
{
    int op = 0;

    do{
        printf("\n ==== Operações sobre índice ==== \n");
        printf(" 1 - Imprimir esquema do índice.\n");
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




noMemoria *alocarNo()
{
    noMemoria *novo = (noMemoria *) calloc(1,sizeof(noMemoria));

    if(novo != NULL)
    {
        novo->noAlterado = false;
        novo->pai = NULL;
        novo->filhos = (void **) calloc(N, sizeof(void *)); // talvez seja melhor alocar apenas a quantidade que existe 
        novo->conteudo = NULL;
    }

    return novo;
}

folhaMemoria *alocarFolha()
{
    folhaMemoria *novo = (folhaMemoria *) calloc(1,sizeof(folhaMemoria));

    if(novo != NULL)
    {
        novo->folhaAlterada = false;
        novo->pai = NULL;
        novo->ant = NULL;
        novo->prox = NULL;
        novo->conteudo = NULL;
    }

    return novo;
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
    int indicador = 0;
    folhaDisco f;

    lseek(fd_Dados, 0, SEEK_SET);
    read(fd_Dados, &indicador, sizeof(int));

    if(indicador == INDICADOR) // arquivo já foi convertido
        return -2;

    int fd_Temp = open("temp.txt", O_CREAT | O_RDWR | O_TRUNC | __O_LARGEFILE, S_IRWXU | S_IRWXO);
    if(fd_Temp != -1)
    {
        int bytesLidos = 0;
        int qtd = OCUPACAO * N; // quantidade de registros em cada folha
    
        void *bufferLeitura = malloc(sizeof(registro) * qtd);
        void *bufferEscrita = malloc(TAM);

        lseek(fd_Dados, 0, SEEK_SET);
        lseek(fd_Temp, 0, SEEK_SET);

        indicador = INDICADOR;
        write(fd_Temp, &indicador, sizeof(int));

        int cont = 0;
        while( (bytesLidos = read(fd_Dados, bufferLeitura, sizeof(registro) * qtd)) != 0)
        {
            // entradas
            memcpy(bufferEscrita, bufferLeitura, sizeof(registro) * (bytesLidos / sizeof(registro)));

            f.ocupacao = bytesLidos/ sizeof(registro);
            f.ant = cont == 0 ? -1 : cont - 1;
            f.prox = (bytesLidos / sizeof(registro)) < sizeof(registro) * qtd ? -1 : cont + 1;
            f.pai = -1;
            memcpy(bufferEscrita + (TAM - sizeof(folhaDisco)), &f, sizeof(folhaDisco)); // estrutura que armazena informações da folha

            write(fd_Temp, bufferEscrita, TAM);
        }

        close(fd_Dados);
    }

    return fd_Temp;
}

/**
 * bulkloading
 * ------------
 * Entrada: inteiro, indicando o file descriptor do arquivo de dados
 *          inteiro, indicando o file descriptor do arquivo de índice
 * Processo: Realiza a operação de bulkloading 
 * Saída: 
*/
int bulkloading(int fd_Dados, int fd_Indice)
{

}

/**
 * inserirFolha
 * -------------
 * Entrada:
 * Processo:
 * Saída: 0, para falha, 1, para sucesso
*/
int inserirFolha(registro r)
{
    // sugestão de deixar a raiz já carregada na memória, pelo menos
    
    int chave = 0;

    if(raiz == NULL)
    {
        raiz = alocarNo();
        if(raiz == 0)
            return 0;
            
        lseek(fd_Indice, 0, SEEK_SET);
        read(fd_Indice, raiz->conteudo, TAM);
    }

    noMemoria *aux = raiz;

    do
    {
        if(aux->no.filhosSaoFolha)
        {
            // filhos são folhas
        }
        else
        {
            int i;
            for(i = 0; i < aux->no.ocupacao; i++)
            {
                memcpy(&chave, aux->conteudo + sizeof(int) + (2 * sizeof(int)) * i, sizeof(int));
                
                if(r.nseq < chave)
                {
                    //vai para o nó da esquerda

                    if(aux->filhos[i] == NULL)
                    {
                        // o nó não foi carregado na memória ainda
                        if(carregarNo(aux,i) == 0)
                            return 0;
                    }

                    break;
                }
            }

            if(i == aux->no.ocupacao)
            {
                // segue o ponteiro do nó mais a direita

                if(aux->filhos[i] == NULL)
                {
                    // 
                }
                else
                {

                }
            }

            aux = aux->filhos[i];
        }
    }while(1);
    
}


/**
 * carregarNo
 * -----------
 * Entrada: ponteiro para estrutura noMemoria
 *          inteiro, indicando qual filho deve ser carregado na memória
 * Processo: Carrega o nó filho especifica por i do disco para a memória
 * Saída: 0, em falha, 1, em sucesso
*/
int carregarNo(noMemoria *m, int i)
{
    int pagina = 0;

    memcpy(&pagina, m->conteudo + (2 * sizeof(int)) * i, sizeof(int));
    
    noMemoria *novo = alocarNo();
    if(novo == 0)
        return 0;
            
    lseek(fd_Indice, pagina * TAM, SEEK_SET);
    read(fd_Indice, novo->conteudo, TAM);

    m->filhos[i] = novo;
    novo->pai = m;

    return 1;
}







/*
    funções:
        splitFolha
        splitNo
        redistribuicaoFolha
        redistribuicaoNo


*/

/**
 * redistribuicaoInserçãoFolha
 * --------------------
 * Entrada:
 * Processo:
 * Saída:
*/
int redistribuicaoInserçãoFolha(folhaMemoria *f, registro r)
{
    // tenta redistribuir à esquerda primeiro

    // verifica a folha a esquerda. Se ela não estiver cheia, faz a redistribuição
    // se não for possível tenta com a direita.
        // em ambos os casos verifica-se se o pai de ambas folhas é o mesmo
    // em caso de sucesso, é necessário alterar as entradas no pai
}


// houve tentativa de inserção em uma folha que estava cheia e a redistribuição falhou
/**
 * splitFolha
 * -----------
 * Entrada:
 * Processo:
 * Saída: 
*/
int splitFolha(folhaMemoria *f, registro r)
{
    // faz o split
    // atualiza o pai, e se necessario chama splitNo ou redistribuicaoNo
}

/**
 * splitNo
 * ----------
 * Entrada:
 * Processo:
 * Saída: 
*/
int splitNo(noMemoria *m, int entrada)
{

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


/*
    Qual a ocupação quando a árvore é criada? 50%? Permitir que o usuário especifique?

    Como converter o arquivo para que ele se encaixe corretamente na alternativa 1?
        Possível implementação: criar um arquivo temporário e ir armazenando as páginas convertidas nele. Excluir o arquivo original e renomear o arquivo temporário

        O_CREAT para criar

        int rename(const char *oldpath, const char *newpath);
            se um arquivo já existir ele é excluido


*/
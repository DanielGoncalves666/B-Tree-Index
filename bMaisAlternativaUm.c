

#include"bMaisAlternativaUm.h"

extern int fd_Dados;
extern int fd_Indice;
//extern int fd_Indice_Aux;
extern bufferpool *b;
extern auxFile auxF;

// O BULKLOANDING PRECISA CONSIDERAR OS PONTEIROS
// OU IMPLEMENTAR UM DOS ALGORITMOS DE ORDENAÇÃO EM DISCO, ORDENAR AS FOLHAS ANTES E ENTÃO USAR O BULKLOADING


/**
 * inserirFolha
 * -------------
 * Entrada:
 * Processo:
 * Saída
*/
int inserirFolha()
{
    // recebe a pagina a ser inserida
        // os registros precisam estar organizados na folha, assim como a estrutura dela com todos os campos preenchidos

    // determina onde deve ser escrito, escreve, e retorna o numero da pagina
    // incrementa a contagem de paginas na estrutura auxiliar
    // deixar a pagina no bufferpool?

}

/**
 * removerFolha
 * -------------
 * Entrada:
 * Processo:
 * Saída
*/
int removerFolha()
{
    // recebe o numero da pagina a ser removida
    // determina sua localização e invalida a pagina (-1 na ocupacao)
        // antes de invalidar deve corrigir os ponteiros das folhas vizinhas
            // a propria folha a ser removida possui o numero das paginas desses vizinhos
        // não altera a estrutura auxiliar no arquivo de indice, a pagina simplesmente permanece invalidada

    // retorna sucesso ou fracasso
}

/**
 * inserirNo
 * ----------
 * Entrada: 
 * Processo:
 * Saída:
*/
int inserirNo()
{
    // recebe a pagina a ser inserida e o numero da pagina de seu pai
    // determina onde deve ser escrito, escreve, e retorna o numero da pagina
    // incrementa a quantidade de nós na estrutura auxiliar

}

/**
 * removerNo
 * ----------
 * Entrada: 
 * Processo:
 * Saída:
*/
int removerNo()
{
    // recebe o numero da pagina a ser removida
    // invalida a pagina
        // como para ser removida ela deve estar vazia, não altera nenhum pai ou filho, essas mudanças ja foram feitas antes
            // talvez seja adequado deixar a alteração do nó pai para esta função ou para uma função auxiliar
}


/**
 * buscaFolhaAltUm
 * -------------------
 * Entrada: inteiro, indicando a chave que se busca
 * Processo: Percorre a árvore até encontrar a folha que a chave passada deve pertencer.
 * Saída: -1, em falha, inteiro não negativo, em sucesso (frame)
*/
int buscaFolhaAltUm(int chave)
{
    int i = 0;
    int frame = -1; // armazena o número do frame que estamos manipulando
    noDisco noAtual; // armazena a struct referente ao nó atual
    folhaDisco folhaAtual; // armazena a struct referente à folha atual
    void *f = NULL; // armazena a página em si
    int qtd = 0; // armazena a quantidade de entradas no nó ou folha em questão
    int *pontChaves = malloc(sizeof(int) * (2 * CHAVES_NO + 1)); // serve para armazenar as chaves e ponteiros de um nó

    frame = carregarPagina(b, fd_Indice, 0); // começa carregando a página do nó raiz
    f = obterFrame(b,frame); // obtém o ponteiro para o conteúdo do nó raiz

    do
    {
        memcpy(&noAtual, f + PAGE_SIZE - sizeof(noDisco), sizeof(noDisco)); // obtemos a estrutura noDisco do nó
        qtd = noAtual.ocupacao;
        memcpy(pontChaves, f, sizeof(int) * (2 * qtd + 1)); // copia-se os ponteiros e chaves para fácil acesso
       
        i = buscaBinariaNo(chave,pontChaves,0, 2 * qtd);

        decrementarPinCount(b,frame); // libera o frame do nó atual (é colocado na pilha)

        if(noAtual.filhosSaoFolha) // o formato das folhas é diferente, sendo necessário outro tratamento
        {
            frame = carregarPagina(b,fd_Dados, pontChaves[i]); // carrega para o bufferpool a folha correta
            f = obterFrame(b,frame);

            break;
        }
        else
        {            
            frame = carregarPagina(b,fd_Indice,pontChaves[i]); // carrega para o bufferpool o filho correto (um nó)
            f = obterFrame(b, frame);
        }
    }while(1);

    free(pontChaves);

    // à partir daqui temos uma folha na posição 'frame' do bufferpool
    return frame;

    // memcpy(&folhaAtual, f + PAGE_SIZE - sizeof(folhaDisco), sizeof(folhaDisco)); // obtemos a estrutura folhaDisco da folha
    // qtd = folhaAtual.ocupacao;
    // memcpy(entradas,f,sizeof(registro) * qtd);

    // int index = buscaBinaria(chave,entradas,0,qtd-1);
    // registro r = {-1,""};
    
    // free(pontChaves);
    // free(entradas);
    
    // decrementarPinCount(b,frame);
    // if(index == -1)
    //     return r;
    // else
    //     return entradas[index];
}

/**
 * buscaBinariaFolha
 * -------------
 * Entrada: inteiro, indicando a chave de busca
 *          ponteiro para registro, indicando o vetor de busca
 *          dois inteiros, indicando o limite esquerdo e direito, respectivamente
 * Processo: Procura por chave no vetor passado
 * Saída: -1, se não for encontrado, inteiro não negativo, indicando a posição do elemento
*/
int buscaBinariaFolha(int chave, registro *entradas, int e, int d)
{
    int meio = (e + d) / 2;

    if(entradas[meio].nseq == chave)
        return meio;
    
    if(e >= d)
        return -1;

    if(entradas[meio].nseq < chave)
        return buscaBinaria(chave,entradas,meio + 1, d);
    else
        return buscaBinaria(chave,entradas,e, meio - 1);
}

/**
 * buscaBinariaNo
 * ---------------
 * Entrada: inteiro, indicando a chave de busca
 *          ponteiro para interios, indicando o vetor que contém os ponteiros e as chaves do nó
 *          dois inteiros, indicando esquerda e direita
 * Processo: Usando a busca binária procura pela próximo nó (ou folha) em que a chave passada possivelmente estará armazenada
 * Saída: inteiro, indicando qual a posição do ponteiro a ser seguido
*/
int buscaBinariaNo(int chave, int *vetor, int e, int d)
{
    /*
        O vetor recebido possui o formato:
            pont chave pont chave pont chave pont
        As posições pares são ponteiros, começando com 0, e as ímpares são chaves.
        Os valores 'e' e 'd' indicam ponteiros.
        Para calcular o meio corretamente é necessário ignorar os ponteiros e para isso obtemos n do formato de número ímpar
        (2n + 1) de 'e + 1' e 'd - 1', e prosseguimos com a divisão por 2.
            NUM = 2n + 1 --> 2n = NUM - 1 --> n = (NUM - 1)/2
        
    */

    int meio = ( e/2 + (d - 2)/2) / 2; // meio em formato de n
    meio = 2 * meio + 1; // coloca de volta no formato de ímpar

    if(vetor[meio] == chave)
        return meio + 1; // retorna a posição do ponteiro à direita (como é igual, a chave com certeza estará ali)

    if(chave < vetor[meio])
    {
        if(meio - 1 == e) // se o ponteiro à esquerda for o primeiro da faixa considerada não há outra alternativa
            return e;
        else
            return buscaBinariaNo(chave,vetor,e, meio - 1); // se a chave for menor que a entrada em meio e não for o único ponteiro à esquerda
    }
    else
    {
        if(meio + 1 == d) 
            return d;
        else
            return buscaBinariaNo(chave,vetor,meio + 1, d);
    }
}


/**
 * inserirAltUm
 * ---------------------
 * Entrada: 
 * Processo: 
 * Saída: 
*/
int inserirAltUm(registro r)
{
    folhaDisco folhaAtual;
    void *f = NULL;
    int qtd = 0;
    registro *entradas = malloc(sizeof(registro) * REG_FOLHA); // serve para armazenar os registros de uma folha

    int frameFolha = buscaFolhaAltUm(r.nseq);
    f = obterFrame(b,frameFolha);
    memcpy(&folhaAtual, f + PAGE_SIZE - sizeof(folhaDisco), sizeof(folhaDisco)); // obtemos a estrutura folhaDisco da folha

    if(folhaAtual.ocupacao == REG_FOLHA) // folha cheia
    {
        if( redistribuicaoInsercao(r, frameFolha) == 0 )
        {
            // chama split
        }
    }   
    else
    {
        qtd = folhaAtual.ocupacao;
        memcpy(entradas,f,sizeof(registro) * qtd);

        registro aux;

        int i = -1;
        while (++i <= folhaAtual.ocupacao && r.nseq < entradas[i].nseq);

        if(i == folhaAtual.ocupacao) // adiciona no final
            entradas[i] = r;
        else // adiciona no meio e faz shift pra direita
        {
            for(; i < folhaAtual.ocupacao; i++)
            {   
                aux = entradas[i];
                entradas[i] = r;
                r = aux;
            }
            entradas[i] = r;
        }

        persistirFrame(b,frameFolha);
    }

    decrementarPinCount(b,frameFolha);

    free(entradas);

    return 1;
}

/**
 * redistribuicaoInsercao
 * -----------------------
 * Entrada: 
 * Processo: 
 * Saída: 
*/
int redistribuicaoInsercao(registro r, int frame)
{
    bool redistribuicaoRealizada = false;
    registro *entradas = malloc(sizeof(registro) * REG_FOLHA); // serve para armazenar os registros de uma folha
    int *pontChaves = malloc(sizeof(int) * (2 * CHAVES_NO + 1)); // serve para armazenar as chaves e ponteiros de um nó
    int i;
    registro aux;


    int folhaIrma = -1; // frame da folha irmã
    void *folhaAux = NULL; // ponteiro para o conteúdo do frame irmão
    folhaDisco folhaDiscoAux; // estrutura de dados da folha irmã
    
    int noFrame = -1; // frame do nó pai
    void *noPai = NULL; // ponteiro para o conteúdo do nó
    noDisco pai; // estrutura de dados do nó

    void *fA = NULL; // ponteiro para o conteúdo do frame atual
    folhaDisco folhaAtual; // estrutura de dados da folha atual
    

    fA = obterFrame(b,frame);
    memcpy(&folhaAtual, fA + PAGE_SIZE - sizeof(folhaDisco), sizeof(folhaDisco));
    memcpy(entradas,fA,folhaAtual.ocupacao * sizeof(folhaDisco));

    if(folhaAtual.ant != -1) // existir uma folha à esquerda
    {
        folhaIrma = carregarPagina(b, fd_Dados, folhaAtual.ant);
        folhaAux = obterFrame(b, folhaIrma);
        memcpy(&folhaDiscoAux, folhaAux + PAGE_SIZE - sizeof(folhaDisco), sizeof(folhaDisco));

        // verifica se as folhas possuem o mesmo pai
        if(folhaAtual.pai == folhaDiscoAux.pai)
        {
            if(folhaDiscoAux.ocupacao != REG_FOLHA) // há espaço na folha à esquerda
            {
                folhaDiscoAux.ocupacao++;
                memcpy(folhaAux + PAGE_SIZE - sizeof(folhaDisco),&folhaDiscoAux, sizeof(folhaDisco)); // att o numero de registros
                memcpy(folhaAux + sizeof(registro) * folhaDiscoAux.ocupacao,fA,sizeof(registro));
                    // tranfere um único registro para a irmã à esquerda

                int primeiro = entradas[0].nseq;
                // desloca as entradas pra esquerda
                for(i = 1; i < folhaAtual.ocupacao && r.nseq < entradas[i].nseq; i++) 
                    entradas[i-1] = entradas[i];

                entradas[i - 1] = r; // coloca no lugar certo

                memcpy(fA,entradas,sizeof(registro) * REG_FOLHA); // sobreescre o frame atual

                // carrega o nó pai
                noFrame = carregarPagina(b, fd_Indice, folhaAtual.pai);
                noPai = obterFrame(b,noFrame);
                memcpy(&pai, noPai + PAGE_SIZE - sizeof(noDisco), sizeof(noDisco)); // obtemos a estrutura noDisco do nó
                memcpy(pontChaves, noPai, sizeof(int) * (2 * pai.ocupacao + 1)); // copia-se os ponteiros e chaves para fácil acesso                

                for(i = 0; i < pai.ocupacao * 2; i+=2)
                {
                    if( primeiro == pontChaves[i + 1] ) 
                        pontChaves[i + 1] = entradas[0].nseq; // atualiza o nó pai
                }

                memcpy(noPai,pontChaves, sizeof(int) * (2 * pai.ocupacao + 1)); // sobreescre o frame do nó pai

                persistirFrame(b,folhaIrma);
                persistirFrame(b,frame);
                persistirFrame(b,noFrame);

                decrementarPinCount(b,folhaIrma);
                decrementarPinCount(b,frame);
                decrementarPinCount(b,noFrame);

                redistribuicaoRealizada = true;
            }
        }

        decrementarPinCount(b,folhaIrma);
    }

    if(folhaAtual.prox != -1 && !redistribuicaoRealizada) // existir uma folha à direita
    {
        folhaIrma = carregarPagina(b, fd_Dados, folhaAtual.prox);
        folhaAux = obterFrame(b, folhaIrma);
        memcpy(&folhaDiscoAux, folhaAux + PAGE_SIZE - sizeof(folhaDisco), sizeof(folhaDisco));

        // verifica se as folhas possuem o mesmo pai
        if(folhaAtual.pai == folhaDiscoAux.pai)
        {
            if(folhaDiscoAux.ocupacao != REG_FOLHA) // há espaço na folha à direita
            {
                folhaDiscoAux.ocupacao++;
                memcpy(folhaAux + PAGE_SIZE - sizeof(folhaDisco),&folhaDiscoAux, sizeof(folhaDisco)); // att o numero de registros
                memcpy(folhaAux + sizeof(registro) * folhaDiscoAux.ocupacao,fA,sizeof(registro));
                    // tranfere um único registro para a irmã à esquerda

                int primeiro = entradas[0].nseq;
                // desloca as entradas pra esquerda
                for(i = 1; i < folhaAtual.ocupacao && r.nseq < entradas[i].nseq; i++) 
                    entradas[i-1] = entradas[i];

                entradas[i - 1] = r; // coloca no lugar certo

                memcpy(fA,entradas,sizeof(registro) * REG_FOLHA); // sobreescre o frame atual

                // carrega o nó pai
                noFrame = carregarPagina(b, fd_Indice, folhaAtual.pai);
                noPai = obterFrame(b,noFrame);
                memcpy(&pai, noPai + PAGE_SIZE - sizeof(noDisco), sizeof(noDisco)); // obtemos a estrutura noDisco do nó
                memcpy(pontChaves, noPai, sizeof(int) * (2 * pai.ocupacao + 1)); // copia-se os ponteiros e chaves para fácil acesso                

                for(i = 0; i < pai.ocupacao * 2; i+=2)
                {
                    if( primeiro == pontChaves[i + 1] ) 
                        pontChaves[i + 1] = entradas[0].nseq; // atualiza o nó pai
                }

                memcpy(noPai,pontChaves, sizeof(int) * (2 * pai.ocupacao + 1)); // sobreescre o frame do nó pai

                persistirFrame(b,folhaIrma);
                persistirFrame(b,frame);
                persistirFrame(b,noFrame);

                decrementarPinCount(b,folhaIrma);
                decrementarPinCount(b,frame);
                decrementarPinCount(b,noFrame);

                redistribuicaoRealizada = true;
            }
        }
        decrementarPinCount(b,folhaIrma);
    }

    if(redistribuicaoRealizada)
        return 1;
    else
        return 0;

}

/**
 * splitInsercao
 * --------------
 * Entrada:
 * Processo:
 * Saída:
*/
int splitInsercao(registro r, int frame)
{
    bool splitRealizado = false;
    registro *entradas = malloc(sizeof(registro) * REG_FOLHA); // serve para armazenar os registros de uma folha
    int *pontChaves = malloc(sizeof(int) * (2 * CHAVES_NO + 1)); // serve para armazenar as chaves e ponteiros de um nó
    int i;
    registro aux;


    int folhaIrma = -1; // frame da folha irmã
    void *folhaAux = NULL; // ponteiro para o conteúdo do frame irmão
    folhaDisco folhaDiscoAux; // estrutura de dados da folha irmã
    
    int noFrame = -1; // frame do nó pai
    void *noPai = NULL; // ponteiro para o conteúdo do nó
    noDisco pai; // estrutura de dados do nó

    void *fA = NULL; // ponteiro para o conteúdo do frame atual
    folhaDisco folhaAtual; // estrutura de dados da folha atual
    

    fA = obterFrame(b,frame);
    memcpy(&folhaAtual, fA + PAGE_SIZE - sizeof(folhaDisco), sizeof(folhaDisco));
    memcpy(entradas,fA,folhaAtual.ocupacao * sizeof(folhaDisco));

    if(folhaAtual.ant != -1)
    {
        folhaIrma = carregarPagina(b, fd_Dados, folhaAtual.ant);
        folhaAux = obterFrame(b, folhaIrma);
        memcpy(&folhaDiscoAux, folhaAux + PAGE_SIZE - sizeof(folhaDisco), sizeof(folhaDisco));

        // verifica se as folhas possuem o mesmo pai
        if(folhaAtual.pai == folhaDiscoAux.pai)
        {

        }
    }

    if(folhaAtual.prox != -1 && !splitRealizado)
    {

    }



}

#include"operacoesFolha.h"
#include"operacoesNo.h"
#include"bufferpool.h"
#include"operacoesArquivos.h"
#include<stdlib.h>
#include<string.h> 

extern int fd_Dados;
extern int fd_Indice;
extern bufferpool *b;
extern auxFile auxF;

/**
 * buscaFolhaAltUm
 * -------------------
 * Entrada: inteiro, indicando a chave que se busca
 * Processo: Percorre a árvore até encontrar a folha que a chave passada deve pertencer.
 * Saída: -1, em falha, inteiro não negativo, em sucesso (frame)
*/
int buscaFolhaAltUm(int chave)
{
    int prox = 0; // armazena o indice que indica o ponteiro para o proximo no ou folha
    int frame = -1; // armazena o número do frame que estamos manipulando
    noDisco noAtual; // armazena a struct referente ao nó atual
    folhaDisco folhaAtual; // armazena a struct referente à folha atual
    void *f = NULL; // armazena a página em si
    int qtd = 0; // armazena a quantidade de entradas no nó ou folha em questão
    int *pontChaves = malloc(sizeof(int) * (2 * CHAVES_NO + 1)); // serve para armazenar as chaves e ponteiros de um nó

    frame = carregarPagina(b, fd_Indice, 0); // começa carregando a página do nó raiz
    f = obterAcessoFrame(b,frame); // obtém o ponteiro para o conteúdo do nó raiz

    do
    {
        carregarNoDisco(frame,&noAtual);
        qtd = noAtual.ocupacao;
        memcpy(pontChaves, f, sizeof(int) * (2 * qtd + 1)); // copia-se os ponteiros e chaves para fácil acesso
       
        prox = buscaBinariaNo(chave,pontChaves,0, 2 * qtd);

        decrementarPinCount(b,frame); // libera o frame do nó atual (é colocado na pilha)

        if(noAtual.filhosSaoFolha) // o formato das folhas é diferente, sendo necessário outro tratamento
        {
            frame = carregarPagina(b,fd_Dados, pontChaves[prox]); // carrega para o bufferpool a folha correta
            break;
        }
        else
        {            
            frame = carregarPagina(b,fd_Indice,pontChaves[prox]); // carrega para o bufferpool o filho correto (um nó)
            f = obterAcessoFrame(b, frame);
        }
    }while(1);

    free(pontChaves);

    // à partir daqui temos uma folha na posição 'frame' do bufferpool
    return frame;
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
        return buscaBinariaFolha(chave,entradas,meio + 1, d);
    else
        return buscaBinariaFolha(chave,entradas,e, meio - 1);
}



/**
 * inserirOrdenado
 * ----------------
 * Entrada: inteiro, indicando o frame que a folha está carregada
 *          registro, indicando o registro que será inserido
 * Processo: Insere o registro passado de forma ordenada na folha
 * Saída: -1, em falha, 0, em sucesso, 1, em sucesso com alteração do primeiro valor
*/
int inserirOrdenadoFolha(int frame, registro r)
{
    bool primeiro = false;
    registro aux;
    int i = 0;

    registro *entradas = malloc(sizeof(registro) * REG_FOLHA); // serve para armazenar os registros de uma folha

    void *f = NULL; 
    folhaDisco leaf; 

    f = obterAcessoFrame(b, frame);
    carregarFolhaDisco(frame,&leaf);

    if(leaf.ocupacao == REG_FOLHA)
    {
        free(entradas);
        return -1;
    }

    memcpy(entradas, f, sizeof(registro) * (leaf.ocupacao + 1) );

    i = -1;
    while(++i <= leaf.ocupacao && r.nseq < entradas[i].nseq);

    if(i == 0)
        primeiro = true;

    if(i == leaf.ocupacao) // adiciona no final
        entradas[i] = r;
    else // adiciona no meio e faz shift pra direita
    {
        for(; i < leaf.ocupacao; i++)
        {   
            aux = entradas[i];
            entradas[i] = r;
            r = aux;
        }
        entradas[i] = r;
    }

    leaf.ocupacao++;

    gravarFolhaDisco(frame,leaf);
    memcpy(f, entradas, sizeof(registro) * leaf.ocupacao); // sobreescreve as entradas

    free(entradas);

    if(primeiro)
        return 1;
    else
        return 0;
}


/**
 * removerOrdenadoFolha
 * ----------------
 * Entrada: inteiro, indicando o frame que a folha está carregada
 *          inteiro, indicando a chave do registro a ser removido
 *          inteiro, indicando se a remoção é do primeiro registro (-1), do último registro (1) ou se deve ser de acordo com a chave (0)
 *          ponteiro para registro, indicando onde deve-se armazenar o registro removido
 * Processo: Remove o registro especificado pela chave e flag. ( não atenta pela regra de 50%)
 * Saída: -1, em falha por inexistência, 1, em sucesso, 2, em sucesso com alteração do primeiro registro
*/
int removerOrdenadoFolha(int frame, int chave, int flag, registro *r)
{
    registro *entradas = malloc(sizeof(registro) * REG_FOLHA); // serve para armazenar os registros de uma folha
    int i = 0;

    void *f = NULL; 
    folhaDisco leaf; 

    f = obterAcessoFrame(b, frame);
    carregarFolhaDisco(frame,&leaf);

    if(leaf.ocupacao == 0)
    {
        free(entradas);
        return -1;
    }

    memcpy(entradas, f, sizeof(registro) * leaf.ocupacao);

    if(flag == -1)
    {
        for(i = 1; i < leaf.ocupacao; i++)
            entradas[i - 1] = entradas[i];
        
        leaf.ocupacao--;

        free(entradas);
        return 2;
    }
    else if(flag == 1)
    {
        *r = entradas[leaf.ocupacao - 1];
        leaf.ocupacao--;
    }
    else if(flag == 0)
    {
        i = buscaBinariaFolha(chave, entradas, 0, leaf.ocupacao - 1);
        if(i == -1)
        {
            free(entradas);
            return -1;
        }

        *r = entradas[i];

        for(i = ++i; i < leaf.ocupacao; i++)
            entradas[i - 1] = entradas[i];

        leaf.ocupacao--;

        if(i == 0)
        {
            free(entradas);
            return 2;
        }
    }

    gravarFolhaDisco(frame,leaf);
    memcpy(f, entradas, sizeof(registro) * leaf.ocupacao); // sobreescreve as entradas

    free(entradas);
    return 1;
}




/**
 * transferirRegistros
 * --------------------
 * Entrada: inteiro, indicando o frame da folha de destino
 *          inteiro, indicando o frame da folha de origem
 *          inteiro, indicando a quantidade de registros que devem ser transferidos. -1 indica todos os registros
 *          boolean, indicando se deve transferir os registros do começo da folha de origem ou do final.
 *                   Se forem os registros do começo de origem, eles são colocados no fim da folha destino.
 *                   Se forem os registros do final de origem, eles são colocados no início da folha destino.
 *                      Assume-se que os registros já estão ordenados.
 * Processo: Transfere 'qtd' registros de uma folha para a outra.
 * Saída: 0, em falha, 1, em sucesso
*/
int transferirRegistros(int frameDestino, int frameOrigem, int qtd, bool inicio)
{
    if(frameDestino < 0 || frameOrigem < 0)
        return 0;

    void *destino = obterAcessoFrame(b,frameDestino);
    void *origem = obterAcessoFrame(b,frameOrigem);

    folhaDisco fdestino, forigem;
    carregarFolhaDisco(frameDestino,&fdestino);
    carregarFolhaDisco(frameOrigem,&forigem);
    
    if(fdestino.ocupacao + qtd > REG_FOLHA || qtd > forigem.ocupacao)
        return 0;

    if(qtd == -1)
        qtd = forigem.ocupacao;

    if(inicio)
    {       
            // pega do começo da folha origem e coloca no final da folha destino
        memcpy(destino + sizeof(registro) * fdestino.ocupacao, origem, sizeof(registro) * qtd);
            // move o restante dos registros para o começo da folha
        memmove(origem, origem + sizeof(registro) * qtd, sizeof(registro) * (forigem.ocupacao - qtd));
    }
    else
    {
            // move os registros do destino de modo a abrir espaço para os registros do fim da folha de destino
        memmove(destino + sizeof(registro) * qtd, destino, sizeof(registro) * fdestino.ocupacao);
            // copia os registros do fim de origem para o começo de destino
        memcpy(destino, origem + sizeof(registro) * (forigem.ocupacao - qtd), sizeof(registro) * qtd);
    }

    fdestino.ocupacao += qtd;
    forigem.ocupacao -= qtd;

    gravarFolhaDisco(frameDestino,fdestino);
    gravarFolhaDisco(frameOrigem,forigem);

    return 1;
}



/**
 * carregarFolhaDisco
 * --------------------
 * Entrada: inteiro, indicando o frame da folha
 *          ponteiro para folhaDisco, indicando onde a estrutura deve ser carregada
 * Processo: Carrega a estrutura folhaDisco.
 * Saída: nenhuma
*/
void carregarFolhaDisco(int frame, folhaDisco *f)
{
    void *pont = obterAcessoFrame(b,frame);
    memcpy(f,pont + PAGE_SIZE - sizeof(folhaDisco), sizeof(folhaDisco));
}

/**
 * gravarFolhaDisco
 * -----------------
 * Entrada: inteiro, indicando o frame da folha
 *          folhaDisco, indicando a estrutura que deve ser gravada
 * Processo: Grava a estrutura folhaDiso na folha em memória
 * Saída: nenhuma
*/
void gravarFolhaDisco(int frame, folhaDisco f)
{
    void *pont = obterAcessoFrame(b,frame);
    memcpy(pont + PAGE_SIZE - sizeof(folhaDisco),&f, sizeof(folhaDisco));
}




/**
 * obterPrimeiraChaveFolha
 * -------------------
 * Entrada: inteiro, indicando o frame do bufferpool onde a folha está carregada
 *          ponteiro para inteiro, onde será armazenado o primeiro valor
 * Processo: Determina a menor chave da folha
 * Saída: 0, em falha, 1, em sucesso
*/
int obterPrimeiraChaveFolha(int frame, int *primeiraChave)
{
    int chave = 0;
    folhaDisco folha;
    registro r;
    void *f;
    
    f = obterAcessoFrame(b,frame);
    carregarFolhaDisco(frame,&folha);

    if(folha.ocupacao <= -1)
        return 0;

    memcpy(&r, f, sizeof(registro));

    *primeiraChave = r.nseq;

    return 1;
}

/**
 * obterUltimaChaveFolha
 * -------------------
 * Entrada: inteiro, indicando o frame do bufferpool onde a folha está carregada
 *          ponteiro para inteiro, onde será armazenado o ultimo valor
 * Processo: Determina a maior chave da folha
 * Saída: 0, em falha, 1, em sucesso
*/
int obterUltimaChaveFolha(int frame, int *ultimaChave)
{
    int chave = 0;
    folhaDisco folha;
    registro r;
    void *f;
    
    f = obterAcessoFrame(b,frame);
    carregarFolhaDisco(frame,&folha);

    if(folha.ocupacao <= -1)
        return 0;

    memcpy(&r, f + sizeof(registro) * (folha.ocupacao - 1), sizeof(registro));

    *ultimaChave = r.nseq;

    return 1;
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
    folhaDisco folha;
    
    int frameFolha = buscaFolhaAltUm(r.nseq);
    carregarFolhaDisco(frameFolha, &folha);

    if(folha.ocupacao == REG_FOLHA) // folha cheia
    {
        if( redistribuicaoInsercaoFolha(frameFolha,r) == 0 )
        {
            // chama split
        }
    }   
    else
    {
        inserirOrdenadoFolha(frameFolha,r);
        persistirFrame(b,frameFolha);
    }

    decrementarPinCount(b,frameFolha);

    return 1;
}

/**
 * redistribuicaoInsercaoFolha
 * -----------------------
 * Entrada: inteiro, indicando o frame cheio
 *          registro, indicando o registro a ser inserido
 * Processo: Realiza a redistribuição com uma das folhas irmãs se possível. Primeiro tenta redistribuir à esquerda, sempre.
 * Saída: 0, em falha na redistribuicao, 1, em sucesso
*/
int redistribuicaoInsercaoFolha(int frame, registro r)
{
    bool redistribuicaoRealizada = false;
    int chave = 0;
    registro removido;
    
    int noFrame = -1; // frame do nó pai

    folhaDisco folhaAtual; // estrutura de dados da folha atual

    int folhaIrma = -1; // frame da folha irmã
    folhaDisco folhaDiscoAux; // estrutura de dados da folha irmã

    carregarFolhaDisco(frame,&folhaAtual);

    if(folhaAtual.ant != -1) // existir uma folha à esquerda
    {
        folhaIrma = carregarPagina(b, fd_Dados, folhaAtual.ant);
        carregarFolhaDisco(folhaIrma,&folhaDiscoAux);

        // verifica se as folhas possuem o mesmo pai
        if(folhaAtual.pai == folhaDiscoAux.pai)
        {
            if(folhaDiscoAux.ocupacao != REG_FOLHA) // há espaço na folha à esquerda
            {
                obterPrimeiraChaveFolha(frame,&chave);
                if(r.nseq < chave) // se o registro a ser inserido for menor que o primeiro da folha atual, inserimos na folha à esquerda
                    inserirOrdenadoFolha(folhaIrma,r);
                else // removemos o primeiro registro da folha atual, o inserimos à esquerda e inserimos r na atual
                {
                    removerOrdenadoFolha(frame,r.nseq,-1,&removido);
                    inserirOrdenadoFolha(folhaIrma,removido); // insere na folha à esquerda
                    inserirOrdenadoFolha(frame,r); // insere o novo registro na folha atual
                }

                obterPrimeiraChaveFolha(frame,&chave);                
                noFrame = carregarPagina(b,fd_Indice,folhaAtual.pai);
                atualizarEntradaNo(noFrame,folhaAtual.self,chave); // atualiza a chave que se refere à folha atual;

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
        carregarFolhaDisco(folhaIrma,&folhaDiscoAux);

        // verifica se as folhas possuem o mesmo pai
        if(folhaAtual.pai == folhaDiscoAux.pai)
        {
            if(folhaDiscoAux.ocupacao != REG_FOLHA) // há espaço na folha à direita
            {
                obterUltimaChaveFolha(frame,&chave);
                if(r.nseq > chave) // se o registro a ser inserido for maior que o ultimo da folha atual, inserimos na folha à direita
                    inserirOrdenadoFolha(folhaIrma,r);
                else // removemos o ultimo registro da folha atual, o inserimos à direita e inserimos r na atual
                {
                    removerOrdenadoFolha(frame,r.nseq,1,&removido);
                    inserirOrdenadoFolha(folhaIrma,removido); // insere na folha à esquerda
                    inserirOrdenadoFolha(frame,r); // insere o novo registro na folha atual
                }

                obterPrimeiraChaveFolha(folhaIrma,&chave);                
                noFrame = carregarPagina(b,fd_Indice,folhaDiscoAux.pai);
                atualizarEntradaNo(noFrame,folhaDiscoAux.self,chave); // atualiza a chave que se refere a folha da direita;

                persistirFrame(b,folhaIrma);
                persistirFrame(b,frame);
                persistirFrame(b,noFrame);

                decrementarPinCount(b,frame);
                decrementarPinCount(b,noFrame);

                redistribuicaoRealizada = true;
            }
        }
        decrementarPinCount(b,folhaIrma);
    }

    return redistribuicaoRealizada;
}


/**
 * splitInsercaoFolha
 * --------------
 * Entrada: inteiro, indicando o frame cheio
 *          registro, indicando o registro a ser inserido
 * Processo: Realiza o split da folha em questao.
 * Saída: 0, em falha, 1, em sucesso
*/
int splitInsercaoFolha(int frame, registro r)
{
    int primeira;
    bool splitRealizado = false;
    
    folhaDisco folhaAtual, folhaNova, folhaIrma; 
    int frameNovo = -1, frameIrma = -1;

    carregarFolhaDisco(frame,&folhaAtual);

    // criação da nova folha
    frameNovo = liberarFrame(b);
    if(frameNovo == -1)
        return 0;

    // carregando folha irma, se existir
    frameIrma = carregarPagina(b,fd_Dados,folhaAtual.prox);

    // preenche informações da estrutura, menos o .self
    folhaNova.pai = folhaAtual.pai;
    folhaNova.ant = folhaAtual.self;
    folhaNova.prox = folhaAtual.prox;
    folhaNova.ocupacao = 0;
    gravarFolhaDisco(frameNovo,folhaNova);

    // transfere os registros para a nova folha
    transferirRegistros(frameNovo,frame,folhaAtual.ocupacao / 2, false); // transfere os registros finais da folha atual para a nova
    
    // atualiza a ocupacao
    folhaNova.ocupacao = folhaAtual.ocupacao / 2;
    gravarFolhaDisco(frameNovo,folhaNova);

    // atualiza a ocupacao da folha atual
    folhaAtual.ocupacao -= folhaNova.ocupacao;
    gravarFolhaDisco(frame,folhaAtual);

    int primeiroNovo = -1;
    obterPrimeiraChaveFolha(frameNovo, &primeiroNovo);
    if(r.nseq < primeiroNovo)
    {
        inserirOrdenadoFolha(frame,r);
        folhaAtual.ocupacao++;
    }
    else
    {
        inserirOrdenadoFolha(frameNovo,r);
        folhaNova.ocupacao++;
    }

    // gravamos a nova folha em disco
    folhaNova.self = inserirFolha(frameNovo);
    gravarFolhaDisco(frameNovo,folhaNova);

    // atualiza o ponteiro da folha atual
    folhaAtual.prox = folhaNova.self;
    gravarFolhaDisco(frame,folhaAtual);

    // atualiza a folha irma se ela existir
    if(frameIrma != -1)
    {
        carregarFolhaDisco(frameIrma,&folhaIrma);
        folhaIrma.ant = folhaNova.self;
        gravarFolhaDisco(frameIrma,folhaIrma);

        persistirFrame(b,frameIrma);
        decrementarPinCount(b,frameIrma);
    }

    noDisco pai;
    int framePai = carregarPagina(b,fd_Indice,folhaAtual.pai);

    // atualiza o pai
    obterPrimeiraChaveFolha(frame, &primeira);
    atualizarEntradaNo(framePai,folhaAtual.self, primeira);
    obterPrimeiraChaveFolha(frameNovo,&primeira);
    
    if(adicionarEntradaNo(framePai,folhaNova.self, primeira) == -1)
    {
        // nó cheio
        if(! redistribuicaoInsercaoNo(framePai, folhaNova.self, primeira))
        {
            // redistribuição falhou
            splitInsercaoNo(framePai, folhaNova.self, primeira);
        }
    }

    decrementarPinCount(b,frame);
    decrementarPinCount(b,frameNovo);

    persistirFrame(b,frameNovo);
    persistirFrame(b,frame);

    return 1;
}

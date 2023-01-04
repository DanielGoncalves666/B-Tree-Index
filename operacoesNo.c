
#include"operacoesNo.h"
#include"operacoesFolha.h"
#include"bufferpool.h"
#include"operacoesArquivos.h"

#include<stdlib.h>
#include<string.h>


extern int fd_Dados;
extern int fd_Indice;
extern bufferpool *b;
extern auxFile auxF;

/**
 * buscaBinariaNo
 * ---------------
 * Entrada: inteiro, indicando a chave de busca
 *          ponteiro para interios, indicando o vetor que contém os ponteiros e as chaves do nó
 *          dois inteiros, indicando esquerda e direita
 * Processo: Usando a busca binária procura pela próximo nó (ou folha) em que a chave passada possivelmente estará armazenada
 * Saída: inteiro, indicando qual a posição do ponteiro a ser seguido, -1, se não existir
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

    if(e >= d)
        return -1;

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
 * buscaBinariaNoPonteiro
 * ---------------
 * Entrada: inteiro, indicando o ponteiro que se busca
 *          ponteiro para inteiros, indicando o vetor que contém os ponteiros e as chaves do nó
 *          dois inteiros, indicando esquerda e direita
 * Processo: Procura pela posição em que o ponteiro passado se localiza
 * Saída: inteiro, indicando qual a posição do ponteiro, -1 se não existir
*/
int buscaBinariaNoPonteiro(int ponteiro, int *vetor, int e, int d)
{
    /*
        O vetor recebido possui o formato:
            pont chave pont chave pont chave pont
        As posições pares são ponteiros, começando com 0, e as ímpares são chaves.
        Os valores 'e' e 'd' indicam ponteiros.
        Para calcular o meio corretamente é necessário ignorar as chaves e para isso obtemos n do formato de número par
        (2n) de 'e' e 'd', e prosseguimos com a divisão por 2.
            NUM = 2n    -->    2n = NUM     -->    n = NUM/2
        
    */

    if(e >= d)
        return -1;

    int meio = ( e/2 + d/2) / 2; // meio em formato de n
    meio = 2 * meio; // coloca de volta no formato de par

    if(vetor[meio] == ponteiro)
        return meio; // retorna a posição do ponteiro

    if(ponteiro < vetor[meio])
        return buscaBinariaNoPonteiro(ponteiro,vetor,e, meio - 2);
    else
        return buscaBinariaNoPonteiro(ponteiro,vetor,meio + 2, d);
}

/**
 * carregarFolhaDisco
 * --------------------
 * Entrada: inteiro, indicando o frame do No
 *          ponteiro para noDisco, indicando onde a estrutura deve ser carregada
 * Processo: Carrega a estrutura noDisco.
 * Saída: nenhuma
*/
void carregarNoDisco(int frame, noDisco *n)
{
    void *pont = obterAcessoFrame(b,frame);
    memcpy(n,pont + PAGE_SIZE - sizeof(noDisco), sizeof(noDisco));
}

/**
 * gravarNoDisco
 * -----------------
 * Entrada: inteiro, indicando o frame do No
 *          noDisco, indicando a estrutura que deve ser gravada
 * Processo: Grava a estrutura noDisco no nó em memória
 * Saída: nenhuma
*/
void gravarNoDisco(int frame, noDisco n)
{
    void *pont = obterAcessoFrame(b,frame);
    memcpy(pont + PAGE_SIZE - sizeof(noDisco),&n, sizeof(noDisco));
}



/**
 * atualizarPai
 * ------------
 * Entrada: inteiro, indicando o ponteiro do nó ou folha cujo pai será atualizado
 *          boolean, indicando se é uma folha ou não
 *          inteiro, contendo o valor do novo pai
 * Process: Atualiza o pai de uma folha ou nó.
 * Saída: nenhuma
*/
void atualizarPai(int pont, bool folha, int novoPai)
{
    noDisco no;
    folhaDisco leef;

    int frame = carregarPagina(b, folha ? fd_Dados : fd_Indice, pont);

    if(folha)
    {
        carregarFolhaDisco(frame,&leef);
        leef.pai = novoPai;
        gravarFolhaDisco(frame,leef);
    }
    else
    {
        carregarNoDisco(frame, &no);
        no.pai = novoPai;
        gravarNoDisco(frame,no);
    }

    persistirFrame(b,frame);
    decrementarPinCount(b,frame);
}


/**
 * atualizarPonteiroNo
 * --------------------
 * Entrada: inteiro, indicando o frame em que está o nó
 *          inteiro, indicando o ponteiro a ser atualizado
 *          inteiro, indicando o novo valor para o ponteiro
 * Processo: Atualiza o ponteiro indicado.
 * Saída: 0, em falha, 1, em sucesso
*/
int atualizarPonteiroNo(int frame, int oldPont, int newPont)
{
    if(frame == -1)
        return 0;

    void *n = NULL; // ponteiro para o conteúdo do nó
    noDisco no; // estrutura de dados do nó
    int *pontChaves = malloc(sizeof(int) * (2 * CHAVES_NO + 1)); // serve para armazenar as chaves e ponteiros de um nó
    int i = 0;

    n = obterAcessoFrame(b,frame);
    carregarNoDisco(frame,&no);

    memcpy(pontChaves, n, sizeof(int) * (2 * no.ocupacao + 1)); // copia-se os ponteiros e chaves para fácil acesso 

    i = buscaBinariaNoPonteiro(oldPont,pontChaves,0, 2 * no.ocupacao);

    if(i == -1)
        return 0;

    memcpy(n + sizeof(int) * i, &newPont, sizeof(int)); // atualiza o ponteiro

    free(pontChaves);

    return 1;
}

/**
 * atualizarPonteiroPosicaoNo
 * --------------------
 * Entrada: inteiro, indicando o frame em que está o nó
 *          inteiro, indicando a posição do ponteiro a ser atualizado
 *          inteiro, indicando o novo valor para o ponteiro
 * Processo: Atualiza o ponteiro ma posição indicada.
 * Saída: 0, em falha, 1, em sucesso
*/
int atualizarPonteiroPosicaoNo(int frame, int pos, int newPont)
{
    if(frame == -1)
        return 0;

    void *n = NULL; // ponteiro para o conteúdo do nó
    noDisco no; // estrutura de dados do nó

    n = obterAcessoFrame(b,frame);
    carregarNoDisco(frame,&no);

    if(no.ocupacao * 2 < pos )
        return 0;

    memcpy(n, &newPont, sizeof(int) * (pos * 2));

    return 1;
}

/**
 * obterPondeiroNo
 * ---------------
 * Entrada: inteiro, indicando o frame onde o nó está carregado
 *          inteiro, indicando se é o primeiro ponteiro (0), ou o último(-1)
 * Processo: De acordo com o valor de 'which', determina o primeiro ou o último ponteiro do nó
 * Saída: -1, em falha, inteiro não negativo, em sucesso
*/
int obterPonteiroNo(int frame, int which)
{
    if(frame == -1 || (which != 0 && which != -1))
        return -1;

    noDisco no;

    void *f = obterAcessoFrame(b,frame);
    carregarNoDisco(frame,&no);

    int ponteiro = -1;

    if(which == 0)
        memcpy(&ponteiro, f, sizeof(int));
    else
        memcpy(&ponteiro, f + sizeof(int) * (2 * no.ocupacao), sizeof(int));

    return ponteiro;
}


/**
 * obterChaveNo
 * -------------
 * Entrada: inteiro, indicando o frame onde o nó está carregado
 *          inteiro, indicando se é a primeira chave (0), ou a última(-1)
 * Processo: De acordo com o valor de 'which', determina a primeira ou a última chave do nó
 * Saída: -1, em falha, inteiro não negativo, em sucesso
*/
int obterChaveNo(int frame, int which)
{
    if(frame == -1 || (which != 0 && which != -1))
        return -1;

    noDisco no;

    void *f = obterAcessoFrame(b,frame);
    carregarNoDisco(frame,&no);

    int chave = -1;

    if(which == 0)
        memcpy(&chave, f + sizeof(int), sizeof(int));
    else
        memcpy(&chave, f + sizeof(int) * (2 * no.ocupacao - 1), sizeof(int));

    return chave;
}

/**
 * obterChaveReferenteNO
 * -----------------------
 * Entrada: inteiro, indicando o frame em que o nó pai está carregado
 *          inteiro, indicando o ponteiro do nó que se busca
 *          ponteiro para inteiro, indicando onde a chave deve ser armazena, se existir 
 * Processo: Determina a chave que referencia o nó (chave anterior ao ponteiro) se um pai existir (não ser raiz). Caso o nó seja referenciado
 *           pelo primeiro ponteiro do pai, retorna -1;
 * Saída: -1, em falha, 0, se for o primeiro ponteiro, 1, em sucesso
*/
int obterChaveReferenteNo(int framePai, int ponteiro, int *chave)
{
    if(framePai == -1)
        return 0;

    int *pontChaves = NULL;
    int indexPont = -1;

    void *p = obterAcessoFrame(b,framePai);
    noDisco pai;
    carregarNoDisco(framePai,&pai);

    pontChaves = malloc(sizeof(int) * (2 * CHAVES_NO + 1));
    memcpy(pontChaves,p,sizeof(int) * (2 * pai.ocupacao + 1));
    
    indexPont = buscaBinariaNoPonteiro(ponteiro,pontChaves,0, 2 * pai.ocupacao);

    if(indexPont <= 0) // se -1, falha, se 0, então primeiro ponteiro (não tem chave atrelada pois é menor que a primeira chave)
        return indexPont;

    *chave = pontChaves[indexPont - 1]; // a chave atrelado ao ponteiro está uma posição antes

    free(pontChaves);
    
    return 1;
}

/**
 * obterNoIrmao
 * -------------
 * Entrada: inteiro, indicando o frame do nó
 *          inteiro, indicando o ponteiro que indica o nó
 *          inteiro, -1 ou 1, indicando se é o irmão à esquerda ou à direita
 * Processo: Obtém o ponteiro para o nó irmão, se existir
 * Saida: -2, se o nó atual é o raiz, -1, em falha, numero não negativo, indicando o ponteiro para o irmao
*/
int obterNoIrmao(int frame, int ponteiro, int which)
{
    if(frame == -1  || (which != 1 && which != -1))
        return -1;

    noDisco atual;
    carregarNoDisco(frame,&atual);

    if(atual.pai == -1)
        return -2;

    // se o pai existir, o carrega
    noDisco pai;
    int framePai = carregarPagina(b, fd_Indice, atual.pai);
    void *fP = obterAcessoFrame(b,framePai);
    carregarNoDisco(framePai,&pai);

    // copia o conteúdo do nó para fácil acesso  
    int *pontChaves = malloc(sizeof(int) * (2 * CHAVES_NO + 1));
    memcpy(pontChaves,fP,sizeof(int) * (2 * pai.ocupacao + 1));

    int index = buscaBinariaNoPonteiro(ponteiro, pontChaves, 0, pai.ocupacao * 2); // obtem o index do ponteiro no nó pai

    if(index == -1)
        return -1;

    index += 2 * which;
    if(index < 0 || index > (2 * pai.ocupacao));
        return -1;

    int retornar = pontChaves[index];

    free(pontChaves);

    return retornar;
}

/**
 * atualizarEntradaNo
 * --------------------
 * Entrada: inteiro, indicando o frame do bufferpool onde o nó está carregado
 *          inteiro, indicando o ponteiro da folha que requer a atualização do nó (pois ocorreu alteração no primeiro elemento)
 *          inteiro, contencdo a nova chave (a chave que sera atualizada vem antes do ponteiro passado)
 * Processo: Atualiza a chave referente a um determinado filho no nó.
 * Saída: 0, em falha, 1, em sucesso
*/
int atualizarEntradaNo(int frame, int ponteiro, int chaveNova)
{
    void *fp = NULL; // ponteiro para o conteúdo do nó
    noDisco pai; // estrutura de dados do nó
    int *pontChaves = malloc(sizeof(int) * (2 * CHAVES_NO + 1)); // serve para armazenar as chaves e ponteiros de um nó

    if(frame == -1)
        return 0;

    fp = obterAcessoFrame(b,frame);
    carregarNoDisco(frame,&pai);
    memcpy(pontChaves, fp, sizeof(int) * (2 * pai.ocupacao + 1)); // copia-se os ponteiros e chaves para fácil acesso                

    if(pontChaves[0] == ponteiro)
    {
        free(pontChaves);
        return 1; // alterações na primeira folha não requerem alterações no nó pai
    }

    int i;
    for(i = 2; i <= pai.ocupacao * 2; i+=2)
    {
        if( pontChaves[i] == ponteiro ) 
        {
            pontChaves[i - 1] = chaveNova;
            break;
        }
    }

    if(i > pai.ocupacao * 2)
    {
        free(pontChaves);
        return 0;
    }

    gravarNoDisco(frame,pai);

    free(pontChaves);
    return 1;
}

/**
 * adicionarEntradaNo
 * --------------------
 * Entrada: inteiro, indicando o frame do bufferpool onde o nó está carregado
 *          inteiro, indicando o ponteiro da folha que foi criada
 *          inteiro, contendo a chave a ser adicionada
 * Processo: Insere, de forma ordenada, um par chave/ponteiro no nó.
 * Saída: -1, se está cheio, 0, em falha, 1, em sucesso
*/
int adicionarEntradaNo(int frame, int ponteiro, int chave)
{
    void *fp = NULL; // ponteiro para o conteúdo do nó
    noDisco pai; // estrutura de dados do nó
    int *pontChaves = malloc(sizeof(int) * (2 * CHAVES_NO + 1)); // serve para armazenar as chaves e ponteiros de um nó
    int i = 0;

    if(frame == -1)
        return 0;

    fp = obterAcessoFrame(b,frame);
    carregarNoDisco(frame,&pai);
    memcpy(pontChaves, fp, sizeof(int) * (2 * pai.ocupacao + 1)); // copia-se os ponteiros e chaves para fácil acesso  

    if(pai.ocupacao == CHAVES_NO)
        return -1;

    for(i = (pai.ocupacao * 2) -1; i >= 1; i -= 2)
    {
        if(pontChaves[i] == chave)
        {
            free(pontChaves);
            return 0;
        }

        if(pontChaves[i] < chave) // encontrou a posição da folha original 
            break;

        pontChaves[i + 2] = pontChaves[i]; // desloca a chave
        pontChaves[i + 3] = pontChaves[i + 1]; // desloca o ponteiro acompanhante
    }

    pontChaves[i + 2] = chave;
    pontChaves[i + 3] = ponteiro;

    pai.ocupacao++;

    gravarNoDisco(frame,pai);
    memcpy(fp,pontChaves, sizeof(int) * (2 * pai.ocupacao + 1)); // sobreescre as entradas do nó pai

    free(pontChaves);

    return 1;
}

/**
 * removerEntradaNo
 * --------------------
 * Entrada: inteiro, indicando o frame do bufferpool onde o nó está carregado
 *          inteiro, indicando o ponteiro associado à chave que deve se removida (o ponteiro vem depois da chave)
 * Processo: Remove, sem deixar buracos, um par chave/ponteiro. Se o ponteiro passado for o primeiro, remove ele e a chave que vem em seguida.
 * Saída: 0, em falha, 1, em sucesso
*/
int removerEntradaNo(int frame, int ponteiro)
{
    void *fp = NULL; // ponteiro para o conteúdo do nó
    noDisco pai; // estrutura de dados do nó
    int *pontChaves = malloc(sizeof(int) * (2 * CHAVES_NO + 1)); // serve para armazenar as chaves e ponteiros de um nó
    int i = 0;

    if(frame == -1)
        return 0;

    fp = obterAcessoFrame(b,frame);
    carregarNoDisco(frame,&pai);
    memcpy(pontChaves, fp, sizeof(int) * (2 * pai.ocupacao + 1)); // copia-se os ponteiros e chaves para fácil acesso  

    if(pai.ocupacao == 0)
        return 0;

    i = buscaBinariaNoPonteiro(ponteiro,pontChaves,0,pai.ocupacao * 2);

    if(i == -1)
    {
        free(pontChaves);
        return 0;
    }
    
    // remove o ponteiro e a chave que vem em seguida
    if(i == 0)
    {
        for( i = i + 2; i < (pai.ocupacao * 2); i += 2)
        {
            pontChaves[i - 2] = pontChaves[i]; // desloca o ponteiro
            pontChaves[i - 1] = pontChaves[i + 1]; // desloca a chave
        }
        pontChaves[i - 2] = pontChaves[i]; // desloca o ultimo ponteiro (pois ele não tem uma chave que o segues)
    }
    else // remove o par chave/ponteiro
    {
        i--;
        for(i = i + 2; i < (pai.ocupacao * 2); i += 2)
        {
            pontChaves[i - 2] = pontChaves[i]; // desloca a chave
            pontChaves[i - 1] = pontChaves[i + 1]; // desloca o ponteiro
        }
    }

    pai.ocupacao--;

    gravarNoDisco(frame,pai);
    memcpy(fp,pontChaves, sizeof(int) * (2 * pai.ocupacao + 1)); // sobreescre as entradas do nó pai

    free(pontChaves);

    return 1;
}

/**
 * redistribuicaoInsercaoNo
 * -------------------------
 * Entrada: inteiro, indicando o frame do nó cheio
 *          inteiro, indicando o ponteiro a ser adicionado
 *          inteiro, indicando a chave a ser adicionada
 * Processo: Realiza a redistribuição com um dos nós irmãos se possível. Primeiro tenta redistribuir à esquerda, sempre.
 * Saida: 0, em falha, 1, em sucesso
*/
int redistribuicaoInsercaoNo(int frame, int ponteiro, int chave)
{
    bool redistribuicaoRealizada = false;
    int pontIrmao = -1; // ponteiro para o nó irmão
    
    int ponteiroDeslocado; // armazena o ponteiro a ser deslocado para o outro nó 
    int chaveReferente; // chave referente ao nó atual ou ao irmão (dependendo se é a esquerda ou direita)
    int novaChaveReferente; // chave que irá substituir a chave referente

    if(frame == -1)
        return 0;

    noDisco atual;
    carregarNoDisco(frame,&atual);

    if(atual.pai == -1)
        return 0;
    
    int frameIrmao = -1;
    noDisco irmao;

    int framePai = -1;
    noDisco pai;

    // irmão à esquerda
    pontIrmao = obterNoIrmao(frame,atual.self, -1);
    if(pontIrmao != -1)
    {
        frameIrmao = carregarPagina(b,fd_Indice,pontIrmao);
        carregarNoDisco(frameIrmao,&irmao);

        // se houver espaço
        if(irmao.ocupacao != CHAVES_NO)
        {
            framePai = carregarPagina(b,fd_Indice,atual.pai);
            carregarNoDisco(framePai,&pai);

            ponteiroDeslocado = obterPonteiroNo(frame,0); // primeiro ponteiro do nó atual
            obterChaveReferenteNo(framePai, atual.self, &chaveReferente); // chave referente ao nó atual no pai
                // se o nó atual fosse o primeiro, n ia nem entrar aqui
            adicionarEntradaNo(frameIrmao, ponteiroDeslocado, chaveReferente); // adiciona no nó irmão
            atualizarPai(ponteiroDeslocado,atual.filhosSaoFolha, irmao.self); // atualiza o pai do filho deslocado
            
            if(chave < obterChaveNo(frame,0))
            {
                // pontOriginal [ ChaveNova PonteiroNovo ] ChaveOriginal
                novaChaveReferente = chave;
                atualizarEntradaNo(framePai, atual.self, novaChaveReferente);

                // pontOriginal foi pro irmão
                // Chavenova foi pro pai
                // PonteiroNovo simplesmente substitui o ponteiro original
                atualizarPonteiroNo(frame,ponteiroDeslocado,ponteiro);
                atualizarPai(ponteiro,atual.filhosSaoFolha, atual.self);
            }
            else
            {
                // pontOriginal ChaveOriginal pontOriginal ....

                novaChaveReferente = obterChaveNo(frame,0); // obtem a primeira chave do nó atual
                atualizarEntradaNo(framePai,atual.self,novaChaveReferente); // atualiza o pai com a primeira chave do nó atual
                    // a chave atualiza se refere ao nó atual

                removerEntradaNo(frame,ponteiroDeslocado); // remove o primeiro ponteiro e a primeira entrada (desloca tudo pra esquerda)

                adicionarEntradaNo(frame, ponteiro, chave);  // adiciona a nova entrada
                atualizarPai(ponteiro,atual.filhosSaoFolha, atual.self);
            }

            redistribuicaoRealizada = true;
        }

        decrementarPinCount(b,frameIrmao);
        persistirFrame(b,frameIrmao);
    }

    // irmão à direita
    pontIrmao = obterNoIrmao(frame,atual.self,1);
    if(pontIrmao != -1 && !redistribuicaoRealizada)
    {
        frameIrmao = carregarPagina(b,fd_Indice,pontIrmao);
        carregarNoDisco(frameIrmao,&irmao);

        // se houver espaço
        if(irmao.ocupacao != CHAVES_NO)
        {
            framePai = carregarPagina(b,fd_Indice,atual.pai);
            carregarNoDisco(framePai,&pai);
            
            if(chave > obterChaveNo(frame,-1))
            {
                // ... pontOriginal [ChaveNova PontNovo]

                // "move" um par para o irmão
                ponteiroDeslocado = ponteiro;
                obterChaveReferenteNo(framePai,irmao.self,&chaveReferente); // chave referente ao nó irmão no pai
                adicionarEntradaNo(frameIrmao, ponteiroDeslocado, chaveReferente); // adiciona no nó irmão o par deslocado
                atualizarPai(ponteiroDeslocado, atual.filhosSaoFolha, irmao.self); // atualiza o pai do filho "deslocado"

                // atualiza a chave no pai
                novaChaveReferente = chave;
                atualizarEntradaNo(framePai, atual.self, novaChaveReferente);
            }
            else
            {
                // ... pontOriginal chaveOriginal pontOriginal
            
                // move um par para o irmão
                ponteiroDeslocado = obterPonteiroNo(frame,-1);
                obterChaveReferenteNo(framePai, irmao.self, &chaveReferente); // chave referente ao nó irmao no pai
                adicionarEntradaNo(frameIrmao, ponteiroDeslocado, chaveReferente); // adiciona no nó irmão o par deslocado
                atualizarPai(ponteiroDeslocado,atual.filhosSaoFolha,irmao.self); // atualiza o pai do filho deslocado

                // atualiza a chave no pai
                novaChaveReferente = obterChaveNo(frame,-1);
                atualizarEntradaNo(framePai,atual.self,novaChaveReferente);

                // exclui o último par do nó atual
                removerEntradaNo(frame,ponteiroDeslocado);

                // adiciona a nova entrada
                adicionarEntradaNo(frame, ponteiro, chave);  // adiciona a nova entrada
                atualizarPai(ponteiro,atual.filhosSaoFolha, atual.self);                      
            }

            redistribuicaoRealizada = true;
        }

        decrementarPinCount(b,frameIrmao);
        persistirFrame(b,frameIrmao);
    }   

    decrementarPinCount(b,frame);
    persistirFrame(b,frame);

    return redistribuicaoRealizada;
}


/**
 * splitInsercaoNo
 * ----------------
 * Entrada: inteiro, indicando o frame do nó cheio
 *          inteiro, indicando o ponteiro a ser adicionado
 *          inteiro, indicando a chave a ser adicionada
 * Processo:  Realiza o split do nó em questao.
 * Saida: -1, em falha, inteiro não negativo indicando o novo nó criado (desconsiderando splits acima), em sucesso
*/
int splitInsercaoNo(int frame, int ponteiro, int chave)
{
    if(frame == -1)
        return -1;

    int i;

    noDisco atual;
    carregarNoDisco(frame,&atual);

    int frameNovo = -1;
    noDisco noNovo;

    // criação do novo nó
    frameNovo = liberarFrame(b);
    void *n = obterAcessoFrame(b,frameNovo);
    if(frameNovo == -1)
        return -1;

    // preenche informações da estrutura, menos o .self
    noNovo.pai = atual.pai;
    noNovo.filhosSaoFolha = atual.filhosSaoFolha;
    noNovo.ocupacao = 0;
    gravarNoDisco(frameNovo,noNovo);

    void *p = obterAcessoFrame(b,frame);
    int *pontChaves = malloc(sizeof(int) * (2 * (CHAVES_NO + 1) + 1));// aloca espaço suficiente para uma entrada à mais
    memcpy(pontChaves,p,sizeof(int) * (2 * atual.ocupacao + 1)); // copia o conteúdo do nó cheio

    // começa na última chave
    for(i = (atual.ocupacao * 2) - 1; i >= 1; i -= 2)
    {
        if(pontChaves[i] < chave) // encontrou a posição da folha original 
            break;

        pontChaves[i + 2] = pontChaves[i]; // desloca a chave
        pontChaves[i + 3] = pontChaves[i + 1]; // desloca o ponteiro acompanhante
    }

    // insere o novo par
    pontChaves[i + 2] = chave;  
    pontChaves[i + 3] = ponteiro;

    int meio = (CHAVES_NO + 1) / 2; // determina a chave que será inserida no nó pai (aplica-se 2 * meio + 1 para obter o índice)

    if(i + 3 > 2 * meio + 1)
        atualizarPai(ponteiro,atual.filhosSaoFolha,atual.self); // o pai do par inserido é o nó atual

    memcpy(p, pontChaves, sizeof(int) * (2 * meio + 1));
    memcpy(n, pontChaves + sizeof(int) * (2 * meio + 2), sizeof(int) * (2 * meio + 1));

    atual.ocupacao = meio;
    noNovo.ocupacao = meio;
    gravarNoDisco(frame,atual);
    gravarNoDisco(frameNovo,noNovo);

    int pontNovoNo = inserirNo(frameNovo); // persiste o novo nó em disco
    atualizarPageTable(b,frameNovo,fd_Indice,pontNovoNo);

    noNovo.self = pontNovoNo; // finalmente preenche o self do nó novo
    gravarNoDisco(frameNovo,noNovo);

    for(i = 2 * meio + 2; i <= 2 * (CHAVES_NO + 1); i += 2) // atualiza o pai dos filhos do novoNo
        atualizarPai(pontChaves[i],atual.filhosSaoFolha,noNovo.self);

    int framePai = -1;
    void *fp = NULL;
    noDisco pai;
    int pontPai = -1;

    if(atual.pai == -1) // atual é o nó raiz
    {
        framePai = liberarFrame(b);
        fp = obterAcessoFrame(b,frameNovo);

        pontChaves[0] = atual.self;
        pontChaves[1] = pontChaves[2 * meio + 3];
        pontChaves[2] = noNovo.self;

        memcpy(fp, pontChaves, sizeof(int) * 3); // copia para o frame

        pai.filhosSaoFolha = false;
        pai.ocupacao = 1;
        pai.pai = -1;
        gravarNoDisco(framePai,pai);
        
        int pontPai = inserirNo(framePai); // persiste o pai em disco
        atualizarPageTable(b,framePai,fd_Indice,pontPai);

        pai.self = pontPai; // finalmente preenche o self do novo pai
        gravarNoDisco(framePai,pai);

        atualizarPai(atual.self, false, pai.self);
        atualizarPai(noNovo.self, false, pai.self);

        // atualiza a raiz
        auxF.profundidade++;
        auxF.noRaiz = pai.self;
        writeAuxInfo(fd_Indice,auxF);
    }
    else
    {
        framePai = carregarPagina(b,fd_Indice,atual.pai);
        
        atualizarEntradaNo(framePai, atual.self, pontChaves[1]);
        
        if(adicionarEntradaNo(framePai, noNovo.self, pontChaves[2 * meio + 3]) == -1)
        {
            // nó cheio
            if(! redistribuicaoInsercaoNo(framePai, noNovo.self, pontChaves[2 * meio + 3]))
            {
                // redistribuição falhou
                splitInsercaoNo(framePai, noNovo.self, pontChaves[2 * meio + 3]);
            }
        }
    }

    decrementarPinCount(b,frame);
    decrementarPinCount(b,frameNovo);
    decrementarPinCount(b,framePai);

    persistirFrame(b,frameNovo);
    persistirFrame(b,frame);
    persistirFrame(b,framePai);

    free(pontChaves);

    return frameNovo;
}
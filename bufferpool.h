#ifndef __BUFFERPOOL_H
#define __BUFFERPOOL_H

typedef struct bufferpool bufferpool;
typedef struct pageID pageID;

bufferpool *criarBufferPool(int qtd);
int desalocarBufferPool(bufferpool *b);
int persistirFrame(bufferpool *b, int i);
int carregarPagina(bufferpool *b, int fd, int pid);
int liberarFrame(bufferpool *b);
void atualizarPageTable(bufferpool *b, int frame, int fd, int pid);
void *obterAcessoFrame(bufferpool *b, int i);
void ativarDirtyBit(bufferpool *b, int i);
void desativarDirtyBit(bufferpool *b, int i);
void decrementarPinCount(bufferpool *b, int i);
// o incremento do pin count é de responsabilidade da função carregarPagina

#endif
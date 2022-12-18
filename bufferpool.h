
#ifndef __BUFFERPOOL_H
#define __BUFFERPOOL_H

#define PAGE_SIZE 512

typedef struct bufferpool bufferpool;
typedef struct pageID pageID;

bufferpool *criarBufferPool(int qtd);
int desalocarBufferPool(bufferpool *b);
int persistirFrame(bufferpool *b, int i);
int carregarPagina(bufferpool *b, pageID p);

#endif
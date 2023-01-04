
#ifndef __B_MAIS_ALTERNATIVA_UM_H
#define __B_MAIS_ALTERNATIVA_UM_H

int inserirFolha(int frame);
int removerFolha(int fd, int pageID);
int inserirNo(int frame);
int removerNo(int fd, int pageID);
int converterArquivo();
int loadAuxInfo();
int writeAuxInfo(int fd, auxFile info);

#endif
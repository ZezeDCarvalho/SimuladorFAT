/*------------------------------------------------------------+ 
 |         UNIFAL - Universidade Federal de Alfenas.          |
 |  BACHARELADO EM CIENCIA DA COMPUTACAO                      |
 |  Trabalho...: SIMULACAO DE SISTEMAS DE ARQUIVOS FAT        |
 |  Disciplina.: Estrutura de Dados I                         |
 |  Professor..: Luiz Eduardo da Silva                        |
 |  Alunos.....: Maria Jose Silva de Carvalho                 |
 |  Data.......: 12/07/2017                                   |
 +------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define TAM_GRANULO 3
#define TAM_MEMORIA 30
#define TRUE 1
#define FALSE 0

typedef struct noSet * ptnoSet;

typedef struct noSet {
    int inicio, fim;
    ptnoSet prox;
} noSet;

typedef struct noArq * ptnoArq;

typedef struct noArq {
    char nome[13];
    int caracteres;
    ptnoSet setores;
    ptnoArq prox;
} noArq;

typedef char memoria[TAM_MEMORIA][TAM_GRANULO];

void mostraSetores(ptnoSet S, char *n) {
    printf("%s = [", n);
    while (S) {
        printf("(%d,%d)", S->inicio, S->fim);
        S = S->prox;
        if (S) printf(" , ");
    }
    printf("]\n");
}

void mostraArquivos(ptnoArq A) {
    printf("Arquivos: \n");
    while (A) {
        printf("   %12s, %2d caracter(es). ", A->nome, A->caracteres);
        mostraSetores(A->setores, "Setores");
        A = A->prox;
    }
    printf("\n");
}

void mostraMemoria(memoria Memo) {
    int i, j;
    for (i = 0; i < TAM_MEMORIA; i++) {
        printf("%3d:[", i);
        for (j = 0; j < TAM_GRANULO - 1; j++)
            printf("%c,", Memo[i][j]);
        printf("%c]", Memo[i][TAM_GRANULO - 1]);
        if ((i + 1) % 10 == 0) printf("\n");
    }
}

void inicia(ptnoSet *Area, ptnoArq *Arq, memoria Memo) {
    int i, j;
    *Area = (ptnoSet) malloc(sizeof (noSet));
    (*Area)->inicio = 0;
    (*Area)->fim = TAM_MEMORIA - 1;
    (*Area)->prox = NULL;
    *Arq = NULL;
    for (i = 0; i < TAM_MEMORIA; i++)
        for (j = 0; j < TAM_GRANULO; j++)
            Memo[i][j] = ' ';
}

//localiza posicao do arquivo para inserir/deletar/apresentar  
void procuraPosicao(ptnoArq *arqAtual, ptnoArq *arqAnt, char **nomeArq) {
    while (*arqAtual && ((strcmp((*arqAtual)->nome, *nomeArq)) < 0)) {
        *arqAnt = *arqAtual;
        *arqAtual = (*arqAtual)->prox;
    }
}

/* void organizaAreaLivre (ptnoSet **Area){
      ptnoSet setor = **Area, auxArea = (**Area)->prox;*/
void organizaAreaLivre(ptnoSet *Area) {
    ptnoSet setor = *Area, auxArea = (*Area)->prox;
    int areaInicio;
    while (auxArea) {
        areaInicio = auxArea->inicio;
        //verifica se os setores sao seguidos e unifica-os
        if (setor->fim == (--areaInicio)) {
            setor->fim = auxArea->fim;
            setor->prox = auxArea->prox;
            free(auxArea);
            auxArea = setor-> prox;
        } else {
            setor = auxArea;
            auxArea = auxArea -> prox;
        }
    }
}

void gravaArquivo(ptnoArq *Arq, ptnoSet *Area, memoria Memo, char *nomeArq, char *texto) {
    int setor, granulo, k = 0, qtdSetDispo = 0;
    ptnoArq arqAtual = *Arq, arqAnt = NULL, auxArq = NULL;
    ptnoSet arLivre = *Area, auxSet = NULL, auxArea = NULL;
    int qtdSetores = (strlen(texto) / TAM_GRANULO);
    if (strlen(texto) % TAM_GRANULO) {
        qtdSetores++;
    } //setores disponiveis
    while (arLivre) {
        qtdSetDispo += arLivre->fim - arLivre->inicio;
        ++qtdSetDispo;
        arLivre = arLivre -> prox;
    } //numero de setores é maior que area disponivel
    if (qtdSetores > qtdSetDispo) {
        puts("Memória cheia!\n");
        return;
    }
    procuraPosicao(&arqAtual, &arqAnt, &nomeArq);
    //compara se já tem arquivo de mesmo nome na lista
    if (arqAtual) {
        if (!strcmp(arqAtual->nome, nomeArq)) {
            puts("Arquivo já existe!\n");
            return;
        }
    } //criacao do arquivo
    auxArq = (ptnoArq) malloc(sizeof (noArq));
    auxSet = (ptnoSet) malloc(sizeof (noSet));
    if (!auxArq || !auxSet) {
        puts("Memória computacional cheia!");
        return;
    } else {
        auxArq->caracteres = strlen(texto);
        strcpy(auxArq->nome, nomeArq);
        if (!arqAnt) { //inserir arquivo no comeco
            auxArq->prox = arqAtual;
            (*Arq) = auxArq;
        } else { //inserir no meio ou fim
            auxArq->prox = arqAtual;
            arqAnt->prox = auxArq;
        } //criacao dos setores do arquivo
        auxArq->setores = auxSet;
        while (qtdSetores) {
            arLivre = (*Area);
            auxSet->inicio = arLivre->inicio;
            //numero de setores seguidos é menor do preciso
            if (((arLivre->fim - arLivre->inicio) + 1) < qtdSetores) {
                auxSet->fim = arLivre->fim;
                *Area = arLivre->prox;
                free(arLivre);
            } else auxSet->fim = auxSet->inicio + (qtdSetores - 1);
            //gravacao na memoria
            for (setor = auxSet->inicio; setor <= auxSet->fim; setor++, --qtdSetores) {
                for (granulo = 0; granulo < TAM_GRANULO && texto[k]; granulo++) {
                    Memo[setor][granulo] = texto[k++];
                }
            } //alocacao de setores
            if (qtdSetores) {
                ptnoSet aux = (ptnoSet) malloc(sizeof (noSet));
                if (!aux) return;
                auxSet->prox = aux;
            } else { //organizacao da area livre
                arLivre->inicio = auxSet->fim;
                arLivre->inicio++;
                //setor ocupado
                if (arLivre->inicio > arLivre->fim) {
                    auxArea = arLivre;
                    (*Area) = arLivre->prox;
                    free(auxArea);
                } //arquivo totalmente gravado
                auxSet->prox = NULL;
            }
            auxSet = auxSet->prox;
        }
    }
}

void deletaArquivo(ptnoArq *Arq, ptnoSet *Area, memoria M, char *nomeArq) {
    int granulo, inSetor;
    ptnoArq arqAtual = *Arq, arqAnt = NULL;
    ptnoSet auxSet = NULL, setor = NULL, auxArea = *Area;
    if (!arqAtual) {
        puts("Não existe arquivos na memória!");
        return;
    } else {
        procuraPosicao(&arqAtual, &arqAnt, &nomeArq);
        if (!arqAtual || ((strcmp(arqAtual->nome, nomeArq)) > 0)) {
            puts("Arquivo não encontrado!");
            return;
        } else if (!strcmp(arqAtual->nome, nomeArq)) {
            if (arqAnt) { //verifica se arqAtual é o inicio
                arqAnt->prox = arqAtual->prox;
            } else (*Arq) = arqAtual->prox;
            while (arqAtual->setores) {
                auxSet = arqAtual->setores;
                for (inSetor = auxSet->inicio; inSetor <= auxSet->fim; inSetor++) {
                    for (granulo = 0; granulo < TAM_GRANULO && M[inSetor][granulo] != ' '; granulo++)
                        M[inSetor][granulo] = ' ';
                }
                arqAtual->setores = auxSet->prox;
                auxArea = (*Area);
                while (auxArea && (auxArea->inicio < auxSet->inicio)) {
                    setor = auxArea;
                    auxArea = auxArea->prox;
                }
                if (!setor) { //setor do arquivo é o inicio da area livre
                    auxSet->prox = (*Area);
                    (*Area) = auxSet;
                } else {
                    setor->prox = auxSet;
                    auxSet->prox = auxArea;
                } //funcao para organizar setores seguidos na area livre
                organizaAreaLivre(Area); //organizaAreaLivre(&Area)
            }
            free(arqAtual);
        }
    }
}

void apresentaArquivo(ptnoArq Arq, memoria M, char *nomeArq) {
    int setor, granulo;
    ptnoArq arqAtual = Arq, arqAnt = NULL;
    ptnoSet auxSet = NULL;
    if (!arqAtual) {
        puts("Não existe arquivos na memória!");
        return;
    } else {
        procuraPosicao(&arqAtual, &arqAnt, &nomeArq);
        if (arqAtual && (!strcmp(arqAtual->nome, nomeArq))) {
            auxSet = arqAtual->setores;
            printf("\n Setores  | Conteúdo \n");
            printf("----------+--------------------------\n");
            while (auxSet) {
                printf(" (%2d, %2d) | ", auxSet->inicio, auxSet->fim);
                for (setor = auxSet->inicio; setor <= auxSet->fim; setor++)
                    for (granulo = 0; granulo < TAM_GRANULO && M[setor][granulo] != ' '; granulo++) {
                        printf("%c", M[setor][granulo]);
                    }
                printf("\n");
                auxSet = auxSet-> prox;
            }
        } else {
            puts("Arquivo não encontrado!");
            return;
        }
    }
}

void ajuda() {
    printf("\nCOMANDOS\n");
    printf("----------\n");
    printf("G <arquivo.txt> <texto> <ENTER> \n");
    printf(" -Grava o <arquivo.txt> e conteudo <texto> no disco \n");
    printf("D <arquivo.txt> \n");
    printf(" -Deleta o <arquivo.txt> do disco \n");
    printf("A <arquivo.txt> \n");
    printf(" -Apresenta o conteudo do <arquivo.txt> \n");
    printf("M \n");
    printf(" -Mostra as estruturas utilizadas \n");
    printf("H \n");
    printf(" -Apresenta essa lista de comandos \n");
    printf("F \n");
    printf(" -Fim da simulacao \n");
}

/*----------------------------------------
 * CORPO PRINCIPAL DO PROGRAMA
 -----------------------------------------*/
int main() {
    ptnoSet Area;
    ptnoArq Arq;
    memoria Memo;
    char com[3];
    char nome[13];
    char texto[TAM_MEMORIA * TAM_GRANULO];

    inicia(&Area, &Arq, Memo);

    do {
        printf("\n>> ");
        scanf("%3s", com);
        com[0] = toupper(com[0]);
        switch (com[0]) {
            case 'G':
                scanf("%s %s", nome, texto);
                printf("nome = %s \n", nome);
                printf("texto = %s \n", texto);
                gravaArquivo(&Arq, &Area, Memo, nome, texto);
                break;
            case 'D':
                scanf("%s", nome);
                printf("nome = %s \n", nome);
                deletaArquivo(&Arq, &Area, Memo, nome);
                break;
            case 'A':
                scanf("%s", nome);
                printf("nome = %s\n", nome);
                apresentaArquivo(Arq, Memo, nome);
                break;
            case 'M':
                mostraSetores(Area, "Area");
                mostraArquivos(Arq);
                printf("Memoria: \n");
                mostraMemoria(Memo);
                break;
            case 'H':
                ajuda();
                break;
        }
    } while (com[0] != 'F');
    printf("\nFim da execucao\n\n");
    return (EXIT_SUCCESS);
}

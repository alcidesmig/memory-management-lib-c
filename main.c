/*
Os códigos foram baseados em: https://danluu.com/malloc-tutorial/ que é um tutorial disponibilizado pelos ministrantes da disciplina.
com refatoração e alterações nas funções e correção de erros

Alcides Mignoso e Silva
Matheus Victorello
*/

#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define SIZE_BLOCO sizeof(Bloco)

void * inicio = NULL;

// Struct para armazenar os blocos de memória alocados 
typedef struct bloco {
    size_t tamanho;
    struct bloco * prox;
    int livre;
    char inicio_dados[1];
} Bloco;

// Encontrar o endereço da struct
Bloco * pega_ponteiro_bloco(void * p) {
    Bloco * iterador = inicio;
    while (iterador) {
        printf("Procurando bloco ~ (%d == %d)\n", p, iterador->inicio_dados);
        if (iterador->inicio_dados == p) {
            return iterador;
        }
        iterador = iterador->prox;
    }
    return NULL;
}

void free(void * p) {
    // Pode ser feito o merge de blocos já liberados, utilizando o split
    Bloco * ponteiro_bloco_a_liberar = pega_ponteiro_bloco(p);
    printf("Liberado: %d\n", p);
    ponteiro_bloco_a_liberar->livre = 1;
}

// Função que tenta encontrar um bloco de memória livre que foi previamente realocado que caiba a quantidade de bytes que queremos alocar
Bloco * encontra_bloco_livre(Bloco ** ultimo, size_t tamanho) {
    Bloco * iterador = inicio;
    while (iterador && !(iterador->livre && iterador->tamanho >= tamanho)) {
        *(ultimo) = iterador;
        iterador = iterador->prox;
    }
    return iterador;
}

// Faz a requisição de <tamanho> bytes e adiciona na lista encadeada
Bloco * solicitar_espaco(Bloco * ultimo, size_t tamanho) {
    Bloco * bloco_;
    bloco_ = sbrk(0);
    void * requisicao = sbrk(tamanho + SIZE_BLOCO); // Faz a requisição

    assert((void * ) bloco_ == requisicao); // Verifica erro

    if (requisicao == (void * ) -1) { // Verifica erro no sbrk = caso a heap esteja cheia (ponteiro está no última posição)
        return NULL;
    }

    if (ultimo) { 
        ultimo->prox = bloco_;
    }

    bloco_->tamanho = tamanho;
    bloco_->prox = NULL;
    bloco_->livre = 0;
    return bloco_;
}

// Função que implementa a alocação dinâmica de memória
void * malloc(size_t tamanho) {
    Bloco * bloco_;

    if (tamanho <= 0) {
        return NULL;
    }

    if (!inicio) { // Primeira chamada do malloc
        bloco_ = solicitar_espaco(NULL, tamanho); // Solicita espaço para o primeiro bloco
        if (!bloco_) {
            return NULL;
        }
        inicio = bloco_; // Seta ele como o primeiro elemento da lista
    } else {
        Bloco * ultimo_bloco = inicio;
        bloco_ = encontra_bloco_livre(&ultimo_bloco, tamanho);
        if (!bloco_) { // Não encontrou bloco livre
            bloco_ = solicitar_espaco(ultimo_bloco, tamanho);
            if (!bloco_) {
                return NULL;
            }
        } else { // Else: encontrou bloco livre que pode ser reaproveitado
            // Pode-se fazer o splitting do bloco, deixando disponível a quantidade de memória que sobrou
            bloco_->livre = 0;
            printf("Reaproveitando\n");
        }
    }

    return (bloco_->inicio_dados);
}

// calloc = limpa a memória depois do malloc
void * calloc(size_t n_elem, size_t tamanho_) {
    size_t tamanho = n_elem * tamanho_;
    void * ptr = malloc(tamanho);
    memset(ptr, 0, tamanho); // Limpa a memória
    return ptr;
}

// Faz o realloc usando o malloc
void * realloc(void * p, size_t tamanho) {
    if (!p) {
        // Para ponteiro nulo, se iguala ao malloc
        return malloc(tamanho);
    }

    Bloco * bloco_recuperado = pega_ponteiro_bloco(p);
    if (bloco_recuperado->tamanho >= tamanho) {
        // Poderia fazer o split do bloco
        return p;
    }

    // Faz a alocação do novo espaço e copia os dados do antigo
    void *novo_ponteiro;
    novo_ponteiro = malloc(tamanho);
    if (!novo_ponteiro) { // Malloc falhou
        return NULL; 
    }
    memcpy(novo_ponteiro, p, bloco_recuperado->tamanho);
    free(p);
    return novo_ponteiro;
}

int main() {
    int * teste = (int*) malloc(sizeof(int));
    *teste = 5;
    printf("%d %d\n", *teste, teste);
    printf("Pós-malloc\n");
    free(teste);
    printf("Pós-free\n");
    teste = (int*) malloc(sizeof(int));
    printf("Novo endereço de teste: %d\n", teste);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//estrutura para armazenar informações de uma página
typedef struct {
    int R; //flag de página referenciada
    int M; //flag de página modificada
    time_t ultimo_acesso; // tempo do último acesso (para LRU)
} Pagina;

void lru(Pagina *memoria, int numero_paginas, int tamanho_pagina, unsigned int addr, char rw, int *page_fault, int *pagina_suja) {
    static int paginas_carregadas = 0; // quantidade de quadros ocupados
    static unsigned page_size = 0;
    static unsigned s = 0;

    if (page_size == 0) {
        page_size = tamanho_pagina * 1024;
        unsigned tmp = page_size;
        while (tmp > 1) {
            tmp = tmp >> 1;
            s++;
        }
    }

    //determinando o número da página associada ao endereço
    unsigned int pagina_atual = addr >> s;

    //verificando se a página já está carregada na memória
    for (int i = 0; i < paginas_carregadas; i++) {
        if ((memoria[i].R == pagina_atual)) { //simulando presença da página usando R como "número"
            memoria[i].ultimo_acesso = time(NULL);
            memoria[i].R = pagina_atual;
            if (rw == 'W') {
                memoria[i].M = 1;
            }
            return; //página já está na memória
        }
    }

    //caso a página não esteja na memória, incrementa o contador de page faults
    (*page_fault)++;

    //se ainda houver espaço na memória, apenas adiciona a nova página
    if (paginas_carregadas < numero_paginas) {
        memoria[paginas_carregadas].R = pagina_atual;
        memoria[paginas_carregadas].M = (rw == 'W') ? 1 : 0;
        memoria[paginas_carregadas].ultimo_acesso = time(NULL);
        paginas_carregadas++;
        return;
    }

    //se não houver espaço, substituir a página LRU (menor tempo), ou seja, o último acesso mais antigo
    int lru_index = 0;
    time_t mais_antigo = memoria[0].ultimo_acesso;

    for (int i = 1; i < numero_paginas; i++) {
        if (memoria[i].ultimo_acesso < mais_antigo) {
            mais_antigo = memoria[i].ultimo_acesso;
            lru_index = i;
        }
    }

    //verificando se a página que vai sair foi modificada, ou seja, se é uma página suja
    if (memoria[lru_index].M == 1) {
        (*pagina_suja)++;
    }

    //substitui pela nova página
    memoria[lru_index].R = pagina_atual;
    memoria[lru_index].M = (rw == 'W') ? 1 : 0;
    memoria[lru_index].ultimo_acesso = time(NULL);
}

void segundo_chance() {
}

void algoritmo_clock() { //função clock não pode ter o nome clock(), pois é uma função padrão da biblioteca time.h
}

typedef struct {
    unsigned int pagina;
    int M; // Página modificada
} Quadro;

typedef struct {
    unsigned int addr;
    char rw;
} Acesso;

void otimo(Pagina *memoria, int numero_paginas, int tamanho_pagina, Acesso *acessos, int total_acessos, int *page_fault, int *pagina_suja) {
    static int paginas_carregadas = 0;
    static unsigned page_size = 0;
    static unsigned s = 0;
    static Quadro *quadros = NULL;
    static int pos = 0;

    if (quadros == NULL) {
        quadros = (Quadro *)malloc(sizeof(Quadro) * numero_paginas);
    }

    if (page_size == 0) {
        page_size = tamanho_pagina * 1024;
        unsigned tmp = page_size;
        while (tmp > 1) {
            tmp = tmp >> 1;
            s++;
        }
    }

    while (pos < total_acessos) {
        unsigned int pagina_atual = acessos[pos].addr >> s;
        char rw = acessos[pos].rw;

        // Verifica se a página já está na memória
        int encontrada = 0;
        for (int i = 0; i < paginas_carregadas; i++) {
            if (quadros[i].pagina == pagina_atual) {
                if (rw == 'W') quadros[i].M = 1;
                encontrada = 1;
                break;
            }
        }

        if (!encontrada) {
            (*page_fault)++;

            // Se ainda há espaço na memória, insere diretamente
            if (paginas_carregadas < numero_paginas) {
                quadros[paginas_carregadas].pagina = pagina_atual;
                quadros[paginas_carregadas].M = (rw == 'W') ? 1 : 0;
                paginas_carregadas++;
            } else {
                // Substituição ótima
                int mais_distante = -1, indice_substituir = -1;

                for (int i = 0; i < numero_paginas; i++) {
                    int j;
                    for (j = pos + 1; j < total_acessos; j++) {
                        unsigned int prox_pag = acessos[j].addr >> s;
                        if (prox_pag == quadros[i].pagina) break;
                    }

                    if (j == total_acessos) {
                        // Página nunca mais será usada
                        indice_substituir = i;
                        break;
                    } else if (j > mais_distante) {
                        mais_distante = j;
                        indice_substituir = i;
                    }
                }

                if (quadros[indice_substituir].M == 1)
                    (*pagina_suja)++;

                quadros[indice_substituir].pagina = pagina_atual;
                quadros[indice_substituir].M = (rw == 'W') ? 1 : 0;
            }
        }

        pos++;
    }

    free(quadros);
}


int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Uso: %s <algoritmo> <arquivo do tipo .log> <tamanho de cada pagina em KB> <tamanho da memoria em MB>\n", argv[0]);
        return 1;
    }

    //lendo argumentos da linha de comando
    char algoritmo[20];
    strncpy(algoritmo, argv[1], sizeof(algoritmo) - 1);
    algoritmo[sizeof(algoritmo) - 1] = '\0';

    char *nome_arquivo = argv[2];
    int tamanho_pagina = atoi(argv[3]);
    int tamanho_memoria = atoi(argv[4]);

    //validando algoritmo
    if (strcmp(algoritmo, "LRU") != 0 && strcmp(algoritmo, "2nd") != 0 && strcmp(algoritmo, "clock") != 0 && strcmp(algoritmo, "otimo") != 0) {
        fprintf(stderr, "Erro: algoritmo inválido. Use: LRU, 2nd, clock ou otimo\n");
        return 1;
    }

    //validando tamanho de página
    if (tamanho_pagina != 8 && tamanho_pagina != 32) {
        fprintf(stderr, "Erro: tamanho de página inválido. Use: 8 ou 32 (em KB)\n");
        return 1;
    }

    //validando tamanho de memória
    if (tamanho_memoria != 1 && tamanho_memoria != 2) {
        fprintf(stderr, "Erro: tamanho de memória inválido. Use: 1 ou 2 (em MB)\n");
        return 1;
    }

    //abrindo o arquivo.log
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    unsigned int addr;
    char rw;
    int pagina_suja = 0;
    int page_fault = 0;
    int numero_paginas = tamanho_memoria * 1024 / tamanho_pagina; // n= de páginas na memória
    //printf("Número de páginas: %d\n", numero_paginas);
    // ^^^testar^^

    Pagina memoria[numero_paginas];

    // Se algoritmo for otimo tem que ler todos os acessos antes
Acesso *acessos = NULL;
int total_acessos = 0;

if (strcmp(algoritmo, "otimo") == 0) {
    // Primeira leitura
    unsigned int a;
    char r;
    while (fscanf(arquivo, "%x %c", &a, &r) == 2)
        total_acessos++;

    acessos = malloc(sizeof(Acesso) * total_acessos);
    rewind(arquivo); //func pra voltar tudo no arquivo maneiro né

    for (int i = 0; i < total_acessos; i++)
        fscanf(arquivo, "%x %c", &acessos[i].addr, &acessos[i].rw);

    // Executar algoritmo ótimo
    Pagina memoria_otima[numero_paginas];
    otimo(memoria_otima, numero_paginas, tamanho_pagina, acessos, total_acessos, &page_fault, &pagina_suja);
    free(acessos);
    fclose(arquivo);
}


    //leitura do arquivo linha por linha
    while (fscanf(arquivo, "%x %c", &addr, &rw) == 2) {
        printf("Endereço: 0x%08x, Tipo: %c\n", addr, rw);
        if (rw != 'R' && rw != 'W') {
            fprintf(stderr, "Erro: tipo de operação inválido '%c'. Use 'R' ou 'W'\n", rw);
            fclose(arquivo);
            return 1;
        }

        if (strcmp(algoritmo, "LRU") == 0) {
            lru(memoria, numero_paginas, tamanho_pagina,addr, rw, &page_fault, &pagina_suja);
        } else if (strcmp(algoritmo, "2nd") == 0) {
            printf("Algoritmo não foi implementado 2nd ainda!\n");
            //segundo_chance();
        } else if (strcmp(algoritmo, "clock") == 0) {
            printf("Algoritmo não foi implementado clock ainda!\n");
            //clock();
        } else if (strcmp(algoritmo, "otimo") == 0) {
            printf("Algoritmo não foi implementado ótimo ainda!\n");
            //otimo();
        }
    }

    //imprimindo resultados
    printf("Número de page faults: %d\n", page_fault);
    printf("Número de páginas sujas: %d\n", pagina_suja);  

    fclose(arquivo);
    return 0;
}

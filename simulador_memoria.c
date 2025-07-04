#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//estrutura para armazenar informações de uma página
typedef struct {
    int numero_pagina; //página atualmente armazenada no quadro
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

void segunda_chance() {
}

void algoritmo_clock(Pagina *memoria, int numero_paginas, int tamanho_pagina, unsigned int addr, char rw, int *page_fault, int *pagina_suja) {
    static int paginas_carregadas = 0;
    static int ponteiro = 0;
    static unsigned page_size = 0;
    static unsigned s = 0;

    //calculando s na primeira execução
    if (page_size == 0) {
        page_size = tamanho_pagina * 1024;
        unsigned tmp = page_size;
        while (tmp > 1) {
            tmp = tmp >> 1;
            s++;
        }
    }

    //determinando o número da página
    unsigned int pagina_atual = addr >> s;

    //verificando se a página já está carregada
    for (int i = 0; i < paginas_carregadas; i++) {
        if (memoria[i].numero_pagina == pagina_atual) {
            memoria[i].R = 1;
            if (rw == 'W') {
                memoria[i].M = 1;
            }
            memoria[i].ultimo_acesso = time(NULL);
            return; // Página já está na memória
        }
    }

    //caso de page fault
    (*page_fault)++;

    // Há espaço livre?
    if (paginas_carregadas < numero_paginas) {
        memoria[paginas_carregadas].numero_pagina = pagina_atual;
        memoria[paginas_carregadas].R = 1;
        memoria[paginas_carregadas].M = (rw == 'W') ? 1 : 0;
        memoria[paginas_carregadas].ultimo_acesso = time(NULL);
        paginas_carregadas++;
        return;
    }

    //substituição usando Clock
    while (1) {
        if (memoria[ponteiro].R == 0) {
            // Substituir esta página
            if (memoria[ponteiro].M == 1) {
                (*pagina_suja)++;
            }

            memoria[ponteiro].numero_pagina = pagina_atual;
            memoria[ponteiro].R = 1;
            memoria[ponteiro].M = (rw == 'W') ? 1 : 0;
            memoria[ponteiro].ultimo_acesso = time(NULL);

            ponteiro = (ponteiro + 1) % numero_paginas;
            break;
        } else {
            // Segunda chance
            memoria[ponteiro].R = 0;
            ponteiro = (ponteiro + 1) % numero_paginas;
        }
    }
}

void otimo() {
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
    int numero_paginas = tamanho_memoria * 1024 / tamanho_pagina; // número de páginas na memória
    //printf("Número de páginas: %d\n", numero_paginas);

    Pagina memoria[numero_paginas];

    //leitura do arquivo linha por linha
    while (fscanf(arquivo, "%x %c", &addr, &rw) == 2) {
        //printf("Endereço: 0x%08x, Tipo: %c\n", addr, rw);
        if (rw != 'R' && rw != 'W') {
            fprintf(stderr, "Erro: tipo de operação inválido '%c'. Use 'R' ou 'W'\n", rw);
            fclose(arquivo);
            return 1;
        }

        if (strcmp(algoritmo, "LRU") == 0) {
            lru(memoria, numero_paginas, tamanho_pagina,addr, rw, &page_fault, &pagina_suja);
        } else if (strcmp(algoritmo, "2nd") == 0) {
            printf("Algoritmo não foi implementado 2nd ainda!\n");
            //segunda_chance();
        } else if (strcmp(algoritmo, "clock") == 0) {
            algoritmo_clock(memoria, numero_paginas, tamanho_pagina,addr, rw, &page_fault, &pagina_suja);
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

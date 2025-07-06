#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//estrutura para armazenar informações de uma página
typedef struct {
    int numero_pagina; //página atualmente armazenada no quadro (para clock)
    int R; //flag de página referenciada
    int M; //flag de página modificada
    unsigned long ultimo_acesso; //tempo do último acesso (para LRU)
} Pagina;

//estrutura para armazenar informações de um quadro de página para o algoritmo ótimo
typedef struct {
    unsigned int pagina; //página atualmente armazenada no quadro (para o ótimo)
    int M; //flag de página modificada para o algoritmo ótimo
} Quadro;

//estrutura para armazenar informações de acesso usada no algoritmo ótimo
typedef struct {
    unsigned int addr;
    char rw;
} Acesso;

void lru(Pagina *memoria, int numero_paginas, int tamanho_pagina, unsigned int addr, char rw, int *page_fault, int *pagina_suja) {
    static int paginas_carregadas = 0; //quantidade de quadros ocupados
    static unsigned page_size = 0;
    static unsigned s = 0;
    static unsigned long time = 0; //contador de tempo simulado

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

    //incrementa o tempo simulado a cada acesso
    time++;

    //verificando se a página já está carregada na memória
    for (int i = 0; i < paginas_carregadas; i++) {
        if ((memoria[i].R == pagina_atual)) { //simulando presença da página usando R como "número"
            memoria[i].ultimo_acesso = time; //atualiza o tempo do último acesso
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
        memoria[paginas_carregadas].ultimo_acesso = time;
        paginas_carregadas++;
        return;
    }

    //se não houver espaço, substituir a página LRU (menor tempo), ou seja, o último acesso mais antigo
    int lru_index = 0;
    unsigned long mais_antigo = memoria[0].ultimo_acesso;

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
    memoria[lru_index].ultimo_acesso = time;
}

void segunda_chance(Pagina *memoria, int numero_paginas, int tamanho_pagina, unsigned int addr, char rw, int *page_fault, int *pagina_suja) {
    static int paginas_carregadas = 0;
    static unsigned page_size = 0;
    static unsigned s = 0;
    static int ponteiro = 0; //ponteiro circular

    if (page_size == 0) {
        page_size = tamanho_pagina * 1024;
        unsigned tmp = page_size;
        while (tmp > 1) {
            tmp = tmp >> 1;
            s++;
        }
    }

    unsigned int pagina_atual = addr >> s;

    //verificando se a página já está na memória
    for (int i = 0; i < paginas_carregadas; i++) {
        if (memoria[i].R == pagina_atual) {
            memoria[i].R = 1; // recebe segunda chance
            if (rw == 'W') memoria[i].M = 1;
            return;
        }
    }

    //caso de page fault
    (*page_fault)++;

    //se ainda há espaço na memória, insere diretamente
    if (paginas_carregadas < numero_paginas) {
        memoria[paginas_carregadas].R = pagina_atual;
        memoria[paginas_carregadas].M = (rw == 'W') ? 1 : 0;
        paginas_carregadas++;
        return;
    }

    //substituição com segunda chance
    while (1) {
        if (memoria[ponteiro].R == 0) {
            if (memoria[ponteiro].M == 1)
                (*pagina_suja)++;

            memoria[ponteiro].R = pagina_atual;
            memoria[ponteiro].M = (rw == 'W') ? 1 : 0;
            ponteiro = (ponteiro + 1) % numero_paginas;
            return;
        } else {
            memoria[ponteiro].R = 0; //limpa referência
            ponteiro = (ponteiro + 1) % numero_paginas;
        }
    }
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
            return; //página já está na memória
        }
    }

    //caso de page fault
    (*page_fault)++;

    //checando se há espaço livre na memória
    if (paginas_carregadas < numero_paginas) {
        memoria[paginas_carregadas].numero_pagina = pagina_atual;
        memoria[paginas_carregadas].R = 1;
        memoria[paginas_carregadas].M = (rw == 'W') ? 1 : 0;
        memoria[paginas_carregadas].ultimo_acesso = time(NULL);
        paginas_carregadas++;
        return;
    }

    //substituição usando clock
    while (1) {
        if (memoria[ponteiro].R == 0) {
            //substituindo esta página
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
            //segunda chance
            memoria[ponteiro].R = 0;
            ponteiro = (ponteiro + 1) % numero_paginas;
        }
    }
}

void otimo(Pagina *memoria, int numero_paginas, int tamanho_pagina, Acesso *acessos, int total_acessos, int *page_fault, int *pagina_suja) {
    int paginas_carregadas = 0;
    unsigned page_size = 0;
    unsigned s = 0;
    Quadro *quadros = NULL;
    int pos = 0;

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

        //verificando se a página já está na memória
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

            //caso ainda tenha espaço na memoria, insere direto
            if (paginas_carregadas < numero_paginas) {
                quadros[paginas_carregadas].pagina = pagina_atual;
                quadros[paginas_carregadas].M = (rw == 'W') ? 1 : 0;
                paginas_carregadas++;
            } else {
                //parte do algoritmo ótimo
                int mais_distante = -1, indice_substituir = -1;

                for (int i = 0; i < numero_paginas; i++) {
                    int j;
                    for (j = pos + 1; j < total_acessos; j++) {
                        unsigned int prox_pag = acessos[j].addr >> s;
                        if (prox_pag == quadros[i].pagina) break;
                    }

                    if (j == total_acessos) {
                        //página nunca mais será usada
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
        fprintf(stderr, "Uso: %s <algoritmo> <arquivo .log> <tamanho_pagina_KB> <tamanho_memoria_MB>\n", argv[0]);
        return 1;
    }

    char algoritmo[20];
    strncpy(algoritmo, argv[1], sizeof(algoritmo) - 1);
    algoritmo[sizeof(algoritmo) - 1] = '\0';

    char *nome_arquivo = argv[2];
    int tamanho_pagina = atoi(argv[3]);
    int tamanho_memoria = atoi(argv[4]);

    if (strcmp(algoritmo, "LRU") != 0 && strcmp(algoritmo, "2nd") != 0 && strcmp(algoritmo, "clock") != 0 && strcmp(algoritmo, "otimo") != 0) {
        fprintf(stderr, "Erro: algoritmo inválido. Use 'LRU', '2nd', 'clock' ou 'otimo'.\n");
        return 1;
    }

    if (tamanho_pagina != 8 && tamanho_pagina != 32) {
        fprintf(stderr, "Erro: tamanho de página inválido. Use 8 ou 32.\n");
        return 1;
    }

    if (tamanho_memoria != 1 && tamanho_memoria != 2) {
        fprintf(stderr, "Erro: tamanho de memória inválido. Use 1 ou 2.\n");
        return 1;
    }

    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo.");
        return 1;
    }

    unsigned int addr;
    char rw;
    int pagina_suja = 0;
    int page_fault = 0;
    int numero_paginas = (tamanho_memoria * 1024) / tamanho_pagina;
    Pagina memoria[numero_paginas];

    //se for algoritmo ótimo, processa todos os acessos de uma vez
    if (strcmp(algoritmo, "otimo") == 0) {
        int total_acessos = 0;
        unsigned int a;
        char r;

        while (fscanf(arquivo, "%x %c", &a, &r) == 2) total_acessos++;
        rewind(arquivo);

        Acesso *acessos = malloc(sizeof(Acesso) * total_acessos);
        for (int i = 0; i < total_acessos; i++)
            fscanf(arquivo, "%x %c", &acessos[i].addr, &acessos[i].rw);

        otimo(memoria, numero_paginas, tamanho_pagina, acessos, total_acessos, &page_fault, &pagina_suja);
        free(acessos);
        fclose(arquivo);
        goto relatorio; //pula o resto do código até o relatorio
    }

    while (fscanf(arquivo, "%x %c", &addr, &rw) == 2) {
        if (rw != 'R' && rw != 'W') {
            fprintf(stderr, "Erro: operação inválida. Use 'R' ou 'W'.\n");
            return 1;
        }
        if (strcmp(algoritmo, "LRU") == 0) {
            lru(memoria, numero_paginas, tamanho_pagina, addr, rw, &page_fault, &pagina_suja);
        } else if (strcmp(algoritmo, "clock") == 0) {
            algoritmo_clock(memoria, numero_paginas, tamanho_pagina, addr, rw, &page_fault, &pagina_suja);
        } else if (strcmp(algoritmo, "2nd") == 0) {
            segunda_chance(memoria, numero_paginas, tamanho_pagina, addr, rw, &page_fault, &pagina_suja);
        }
    }

    fclose(arquivo);

relatorio:
    printf("Executando o simulador...\n");
    printf("Arquivo de entrada: %s\n", nome_arquivo);
    printf("Tamanho da memória física: %d MB\n", tamanho_memoria);
    printf("Tamanho das páginas: %d KB\n", tamanho_pagina);
    printf("Algoritmo de substituição: %s\n", algoritmo);
    printf("Número de page faults: %d\n", page_fault);
    printf("Número de páginas sujas: %d\n", pagina_suja);
    return 0;
}
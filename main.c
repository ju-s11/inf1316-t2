#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Verifica número de argumentos
    if (argc != 5) {
        fprintf(stderr, "Uso: %s <algoritmo> <arquivo.log> <tamanho_pagina_KB> <tamanho_memoria_MB>\n", argv[0]);
        return 1;
    }

    // Lê argumentos da linha de comando
    char algoritmo[20];
    strncpy(algoritmo, argv[1], sizeof(algoritmo) - 1);
    algoritmo[sizeof(algoritmo) - 1] = '\0';

    char *nome_arquivo = argv[2];
    int tamanho_pagina = atoi(argv[3]);
    int tamanho_memoria = atoi(argv[4]);

    // Valida algoritmo
    if (strcmp(algoritmo, "LRU") != 0 &&
        strcmp(algoritmo, "2nd") != 0 &&
        strcmp(algoritmo, "clock") != 0 &&
        strcmp(algoritmo, "otimo") != 0) {
        fprintf(stderr, "Erro: algoritmo inválido. Use: LRU, 2nd, clock ou otimo\n");
        return 1;
    }

    // Valida tamanho de página
    if (tamanho_pagina != 8 && tamanho_pagina != 32) {
        fprintf(stderr, "Erro: tamanho de página inválido. Use: 8 ou 32 (em KB)\n");
        return 1;
    }

    // Valida tamanho de memória
    if (tamanho_memoria != 1 && tamanho_memoria != 2) {
        fprintf(stderr, "Erro: tamanho de memória inválido. Use: 1 ou 2 (em MB)\n");
        return 1;
    }

    // Abre o arquivo .log
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    // Lê o arquivo linha por linha
    unsigned int endereco;
    char tipo_acesso;
    while (fscanf(arquivo, "%x %c", &endereco, &tipo_acesso) == 2) {
        printf("Endereço: 0x%08x, Tipo: %c\n", endereco, tipo_acesso);
        // Aqui você vai futuramente simular o acesso
    }

    fclose(arquivo);
    return 0;
}

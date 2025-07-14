# Trabalho 2 de INF1316 - Sistemas Operacionais

# Enunciado
![Trabalho 2 - gerencia de memoria 2025-1_page-0001](https://github.com/user-attachments/assets/0fe92868-7266-4477-ad7b-cd06a3acd9d6)
![Trabalho 2 - gerencia de memoria 2025-1_page-0002](https://github.com/user-attachments/assets/8c244fee-b051-46aa-b6d5-01ad744b48c3)
![Trabalho 2 - gerencia de memoria 2025-1_page-0003](https://github.com/user-attachments/assets/a18072da-a107-4974-abc5-8500ec0dd76d)

## Comandos de execução e compilação:

### Compilação:
gcc -Wall -o <nome do executável> <nome do arquivo C>

exemplo:
gcc -Wall -o sim-virtual simulador_memoria.c 

### Execução:
./<nome do executável> <algoritmo> <arquivo log> <tamanho de cada página> <tamanho total da memória>

- Algoritmos: LRU, 2nd chance, clock ou ótimo
- Tamanhos de página (KB): 8 ou 32
- Tamanhos da memória (MB): 1 ou 2 

exemplo:
./sim-virtual LRU arquivo.log 8 2

## Observações:
Os arquivos de testes devem estar no mesmo diretório do programa em C ou no comando de execução deve-se informar o endereço completo do arquivo.

## Relatório Completo:
https://docs.google.com/document/d/1lHo6KqouUSpN_UNgo2pzi66tyFfdK7ErPy30T0Q4JVE/edit?usp=sharing

## Integrantes:
- Julia Guimarães Simão - 2211834
- Luiza Oliveira Régnier - 2211931

## Nota Obtida:
100.0/100.0

## Links importantes:
- Enunciado do trabalho do Prof. Dorgival Guedes Neto (UFMG) em que este trabalho foi inspirado: https://homepages.dcc.ufmg.br/~dorgival/cursos/so/tp3.html
- Slides de Gerência de Memória usados para entendimento deste trabalho: https://www-di.inf.puc-rio.br/~endler/courses/inf1316/transp/aulas-teoricas/Cap-4-Mem.pdf

#include <stdio.h>
#include "cache.h"
#include "memoria.h"

#define READ 0
#define WRITE 1

int main() {
    Cache cache = initializeCache(4, 2, 16, 10, 0, 1); // Exemplo de inicialização
    MainMemory memory = initializeMainMemory(100, 150);

    // Ler arquivo de entrada e processar operações
    FILE *file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("Erro ao abrir arquivo\n");
        return 1;
    }

    int address, operation;
    while (fscanf(file, "%d %d", &operation, &address) != EOF) {
        if (operation == READ) {
            int time = readFromCache(&cache, address, memory.readTime);
            printf("Leitura no endereço %d levou %d ns\n", address, time);
        } else if (operation == WRITE) {
            writeToCache(&cache, address, memory.writeTime);
            printf("Escrita no endereço %d realizada\n", address);
        }
    }

    fclose(file);
    return 0;
}


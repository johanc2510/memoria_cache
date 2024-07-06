#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Estruturas para a simulação da cache e memória principal
typedef struct {
    unsigned int tag;
    int valid;
    int dirty;
    int last_accessed;
    int access_count;
} CacheLine;

typedef struct {
    CacheLine *lines;
} CacheSet;

typedef struct {
    CacheSet *sets;
    int set_count;
    int lines_per_set;
    int block_size;
    int write_policy; // 0: write-through, 1: write-back
    int hit_time;
    int replacement_policy; // 0: LFU, 1: LRU, 2: Aleatória
} Cache;

typedef struct {
    int read_time;
    int write_time;
} MainMemory;

// Funções para inicializar e simular a cache
Cache* init_cache(int set_count, int lines_per_set, int block_size, int write_policy, int hit_time, int replacement_policy) {
    Cache *cache = (Cache *)malloc(sizeof(Cache));
    cache->set_count = set_count;
    cache->lines_per_set = lines_per_set;
    cache->block_size = block_size;
    cache->write_policy = write_policy;
    cache->hit_time = hit_time;
    cache->replacement_policy = replacement_policy;

    cache->sets = (CacheSet *)malloc(set_count * sizeof(CacheSet));
    for (int i = 0; i < set_count; i++) {
        cache->sets[i].lines = (CacheLine *)malloc(lines_per_set * sizeof(CacheLine));
        for (int j = 0; j < lines_per_set; j++) {
            cache->sets[i].lines[j].valid = 0;
            cache->sets[i].lines[j].dirty = 0;
            cache->sets[i].lines[j].last_accessed = 0;
            cache->sets[i].lines[j].access_count = 0;
        }
    }
    return cache;
}

void free_cache(Cache *cache) {
    for (int i = 0; i < cache->set_count; i++) {
        free(cache->sets[i].lines);
    }
    free(cache->sets);
    free(cache);
}

unsigned int get_set_index(unsigned int address, int set_count, int block_size) {
    return (address / block_size) % set_count;
}

unsigned int get_tag(unsigned int address, int set_count, int block_size) {
    return (address / block_size) / set_count;
}

void update_lru(CacheSet *set, int line_index) {
    for (int i = 0; i < set->lines[line_index].last_accessed; i++) {
        set->lines[i].last_accessed++;
    }
    set->lines[line_index].last_accessed = 0;
}

void access_cache(Cache *cache, MainMemory *memory, unsigned int address, char operation, int *read_count, int *write_count, int *hit_count, int *miss_count, int *write_back_count) {
    unsigned int set_index = get_set_index(address, cache->set_count, cache->block_size);
    unsigned int tag = get_tag(address, cache->set_count, cache->block_size);

    CacheSet *set = &cache->sets[set_index];
    int hit = 0;
    int empty_line_index = -1;
    int lru_index = -1;
    int lru_value = -1;

    for (int i = 0; i < cache->lines_per_set; i++) {
        CacheLine *line = &set->lines[i];
        if (line->valid && line->tag == tag) {
            hit = 1;
            line->access_count++;
            if (operation == 'W') {
                if (cache->write_policy == 1) {
                    line->dirty = 1;
                } else {
                    (*write_count)++;
                }
            }
            if (cache->replacement_policy == 1) {
                update_lru(set, i);
            }
            break;
        }
        if (!line->valid && empty_line_index == -1) {
            empty_line_index = i;
        }
        if (cache->replacement_policy == 1 && (lru_index == -1 || line->last_accessed > lru_value)) {
            lru_index = i;
            lru_value = line->last_accessed;
        }
    }

    if (!hit) {
        (*miss_count)++;
        int replace_index = (empty_line_index != -1) ? empty_line_index : (cache->replacement_policy == 2) ? rand() % cache->lines_per_set : lru_index;
        CacheLine *line = &set->lines[replace_index];
        if (line->valid && line->dirty) {
            (*write_back_count)++;
            (*write_count)++;
        }
        line->tag = tag;
        line->valid = 1;
        line->dirty = (operation == 'W');
        line->access_count = 1;
        if (cache->replacement_policy == 1) {
            update_lru(set, replace_index);
        }
        (*read_count)++;
    } else {
        (*hit_count)++;
    }
}

void simulate_cache(Cache *cache, MainMemory *memory, const char *input_file, const char *output_file) {
    FILE *input = fopen(input_file, "r");
    if (!input) {
        fprintf(stderr, "Erro ao abrir o arquivo de entrada\n");
        return;
    }
    FILE *output = fopen(output_file, "w");
    if (!output) {
        fprintf(stderr, "Erro ao abrir o arquivo de saída\n");
        fclose(input);
        return;
    }

    char operation;
    unsigned int address;
    int total_reads = 0;
    int total_writes = 0;
    int read_count = 0;
    int write_count = 0;
    int hit_count = 0;
    int miss_count = 0;
    int write_back_count = 0;
    int total_addresses = 0;

    while (fscanf(input, "%x %c", &address, &operation) == 2) {
        if (operation == 'R') {
            total_reads++;
        } else if (operation == 'W') {
            total_writes++;
        }
        access_cache(cache, memory, address, operation, &read_count, &write_count, &hit_count, &miss_count, &write_back_count);
        total_addresses++;
    }

    double hit_rate_read = (total_reads > 0) ? (double)hit_count / total_reads : 0.0;
    double hit_rate_write = (total_writes > 0) ? (double)hit_count / total_writes : 0.0;
    double hit_rate_total = (total_addresses > 0) ? (double)hit_count / total_addresses : 0.0;
    double average_access_time = (double)(hit_count * cache->hit_time + miss_count * (memory->read_time + cache->hit_time)) / total_addresses;

    fprintf(output, "Parâmetros de entrada:\n");
    fprintf(output, "Número de conjuntos: %d\n", cache->set_count);
    fprintf(output, "Linhas por conjunto: %d\n", cache->lines_per_set);
    fprintf(output, "Tamanho do bloco: %d\n", cache->block_size);
    fprintf(output, "Política de escrita: %d\n", cache->write_policy);
    fprintf(output, "Tempo de hit: %d ns\n", cache->hit_time);
    fprintf(output, "Política de substituição: %d\n", cache->replacement_policy);
    fprintf(output, "Tempo de leitura da memória principal: %d ns\n", memory->read_time);
    fprintf(output, "Tempo de escrita da memória principal: %d ns\n", memory->write_time);

    fprintf(output, "Total de endereços: %d\n", total_addresses);
    fprintf(output, "Total de leituras: %d\n", total_reads);
    fprintf(output, "Total de escritas: %d\n", total_writes);
    fprintf(output, "Total de escritas na memória principal: %d\n", write_count);
    fprintf(output, "Total de leituras na memória principal: %d\n", read_count);
    fprintf(output, "Taxa de acerto (leituras): %.4f (%d)\n", hit_rate_read, total_reads);
    fprintf(output, "Taxa de acerto (escritas): %.4f (%d)\n", hit_rate_write, total_writes);
    fprintf(output, "Taxa de acerto (global): %.4f (%d)\n", hit_rate_total, total_addresses);
    fprintf(output, "Tempo médio de acesso: %.4f ns\n", average_access_time);

    fclose(input);
    fclose(output);
}

int main() {
    int set_count = 8;
    int lines_per_set = 4;
    int block_size = 16;
    int write_policy = 1;
    int hit_time = 10;
    int replacement_policy = 1;

    Cache *cache = init_cache(set_count, lines_per_set, block_size, write_policy, hit_time, replacement_policy);

    MainMemory memory;
    memory.read_time = 100;
    memory.write_time = 100;

    simulate_cache(cache, &memory, "teste.cache", "saida_teste.txt");
    

    free_cache(cache);
    return 0;
}


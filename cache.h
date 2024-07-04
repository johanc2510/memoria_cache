#ifndef CACHE_H
#define CACHE_H

typedef struct {
    int valid;
    int tag;
    int *data;
    int lastUsed;  // for LRU
    int frequency; // for LFU
} CacheBlock;

typedef struct {
    CacheBlock *blocks;
} CacheSet;

typedef struct {
    CacheSet *sets;
    int numSets;
    int linesPerSet;
    int blockSize;
    int hitTime;
    int writePolicy; // 0: write-through, 1: write-back
    int replacementPolicy; // 0: LFU, 1: LRU, 2: Random
} Cache;

Cache initializeCache(int numSets, int linesPerSet, int blockSize, int hitTime, int writePolicy, int replacementPolicy);
int readFromCache(Cache *cache, int address, int readTime);
void writeToCache(Cache *cache, int address, int writeTime);

#endif // CACHE_H


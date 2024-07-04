#include <stdio.h>
#include <stdlib.h>
#include "cache.h"

Cache initializeCache(int numSets, int linesPerSet, int blockSize, int hitTime, int writePolicy, int replacementPolicy) {
    Cache cache;
    cache.numSets = numSets;
    cache.linesPerSet = linesPerSet;
    cache.blockSize = blockSize;
    cache.hitTime = hitTime;
    cache.writePolicy = writePolicy;
    cache.replacementPolicy = replacementPolicy;

    cache.sets = (CacheSet *)malloc(numSets * sizeof(CacheSet));
    
    int i,j;
    for (i = 0; i < numSets; i++) {
        cache.sets[i].blocks = (CacheBlock *)malloc(linesPerSet * sizeof(CacheBlock));
        for (j = 0; j < linesPerSet; j++) {
            cache.sets[i].blocks[j].valid = 0;
            cache.sets[i].blocks[j].data = (int *)malloc(blockSize * sizeof(int));
            cache.sets[i].blocks[j].lastUsed = 0;
            cache.sets[i].blocks[j].frequency = 0;
        }
    }
    return cache;
}

int readFromCache(Cache *cache, int address, int readTime) {
    int setIndex = (address / cache->blockSize) % cache->numSets;
    int tag = address / (cache->blockSize * cache->numSets);
    CacheSet set = cache->sets[setIndex];
	
	int i;
    for (i = 0; i < cache->linesPerSet; i++) {
        if (set.blocks[i].valid && set.blocks[i].tag == tag) {
            set.blocks[i].lastUsed = 0; // Update LRU
            set.blocks[i].frequency++; // Update LFU
            return cache->hitTime; // Cache hit
        }
    }

    // Cache miss: load block from main memory
    int blockIndex = -1;
    for (i = 0; i < cache->linesPerSet; i++) {
        if (!set.blocks[i].valid) {
            blockIndex = i;
            break;
        }
    }

    if (blockIndex == -1) {
        // Apply replacement policy
        if (cache->replacementPolicy == 0) {
            // LFU
            int minFreq = set.blocks[0].frequency;
            blockIndex = 0;
            int i;
            for (i = 1; i < cache->linesPerSet; i++) {
                if (set.blocks[i].frequency < minFreq) {
                    minFreq = set.blocks[i].frequency;
                    blockIndex = i;
                }
            }
        } else if (cache->replacementPolicy == 1) {
            // LRU
            int maxUsed = set.blocks[0].lastUsed;
            blockIndex = 0;
            for (i = 1; i < cache->linesPerSet; i++) {
                if (set.blocks[i].lastUsed > maxUsed) {
                    maxUsed = set.blocks[i].lastUsed;
                    blockIndex = i;
                }
            }
        } else {
            // Random
            blockIndex = rand() % cache->linesPerSet;
        }
    }

    // Load block from main memory
    set.blocks[blockIndex].valid = 1;
    set.blocks[blockIndex].tag = tag;
    set.blocks[blockIndex].lastUsed = 0;
    set.blocks[blockIndex].frequency = 1;

    return readTime; // Cache miss penalty
}

void writeToCache(Cache *cache, int address, int writeTime) {
    int setIndex = (address / cache->blockSize) % cache->numSets;
    int tag = address / (cache->blockSize * cache->numSets);
    CacheSet set = cache->sets[setIndex];

	int i;
    for (i = 0; i < cache->linesPerSet; i++) {
        if (set.blocks[i].valid && set.blocks[i].tag == tag) {
            if (cache->writePolicy == 0) {
                // Write-through
                // Directly write to main memory
                writeTime;
            } else {
                // Write-back
                set.blocks[i].lastUsed = 0; // Update LRU
                set.blocks[i].frequency++; // Update LFU
                return;
            }
        }
    }

    // Cache miss
    readFromCache(cache, address, writeTime); // Load block into cache
    writeToCache(cache, address, writeTime); // Write data
}


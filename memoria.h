#ifndef MEMORY_H
#define MEMORY_H

typedef struct {
    int readTime;
    int writeTime;
} MainMemory;

MainMemory initializeMainMemory(int readTime, int writeTime);

#endif // MEMORY_H


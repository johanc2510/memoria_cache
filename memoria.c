#include "memoria.h"

MainMemory initializeMainMemory(int readTime, int writeTime) {
    MainMemory memory;
    memory.readTime = readTime;
    memory.writeTime = writeTime;
    return memory;
}


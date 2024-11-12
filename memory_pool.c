#include <stdio.h>
#include <stdlib.h>

struct Pool {
    int size;               // Size of the memory pool
    int nBlocks;            // Number of blocks in the memory pool
    void *addr;              // memory address to the memory pool allocated 
    struct FreeBlock *list;
};

struct FreeBlock {
    size_t size;             // Size of the free block
    struct FreeBlock *next;  // Pointer to the next block in the first list
};


struct Pool *initPool(int poolSize, int blockSize)
{
    // 1. Check if poolSize is divisible by blockSize to get number of blocks in the pool
    if ((poolSize % blockSize) != 0) {
        printf("error: poolSize is not divisible by blockSize\n");
        exit(EXIT_FAILURE);
    }

    size_t num_blocks = poolSize / blockSize;

    // 2. Allocate and initialize pool structure
    struct Pool *poolPtr = (struct Pool *)malloc(sizeof(struct Pool));
    
    if (poolPtr == NULL) {
        printf("error: malloc failed allocating memory pool structure!\n");  // TODO: Better error!
        exit(EXIT_FAILURE);
    }

    poolPtr->size = poolSize;
    poolPtr->nBlocks = num_blocks;

    // 3. Allocate the contiguous block for the memory pool itself
    poolPtr->addr = malloc(poolSize);
    if (poolPtr->addr == NULL) {
        printf("error: malloc failed allocating memory pool!\n");
        free(poolPtr);  // Clean up allocated Pool structure
        exit(EXIT_FAILURE);
    }

    //4. Populate the free list by linking each block to the next
    poolPtr->list = (struct FreeBlock *)poolPtr->addr;  // Start of free list is at addr
    
    struct FreeBlock *current = poolPtr->list;
    for (size_t i = 0; i < num_blocks - 1; i++) {
        current->size = blockSize;
        current->next = (struct FreeBlock *)((char *)current + blockSize);   // Point to next block
        current = current->next;
    }
    current->size = blockSize;   // Last block
    current->next = NULL;        // End of free list

    return poolPtr;
}

int main()
{
    int poolSize = 1024;
    int blockSize = 128;

    struct Pool *pool = initPool(poolSize, blockSize);

    printf("Memory pool created with %d blocks of size %d\n", pool->nBlocks, blockSize);


    return 0;
}

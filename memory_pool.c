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

    // 4. Populate the free list by linking each block to the next
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

void *alloc(struct Pool *pool, size_t size)
{
    // 1. Check if the size is a multiple of the blocksize
    size_t blockSize = pool->size / pool->nBlocks;

    if ((size % blockSize) != 0) {
        printf("error: unable to allocate memory. size must be multiple of %d", blockSize);
        return NULL;
    }

    // 2. Search for enough contiguous blocks and allocate
    int nBlocksNeeded = size / blockSize;

    struct FreeBlock *current = pool->list;  // current node
    struct FreeBlock *prev = NULL;
    struct FreeBlock *start = NULL;  // Keep track of the start block or beginning of the allocation
    int contiguousCount = 0;

    while (current != NULL) {
        if (contiguousCount = 0) {
            start = current;      // start potential block allocation
        }
        contiguousCount++;

        if (contiguousCount == nBlocksNeeded) {
            if (prev == NULL) {
                pool->list = current->next;
            } else {
                prev->next = current->next;   // Link previous block to the next free block after allocated region
            }
            return (void *)start;
        }
        prev = current;
        current = current->next;
    }
    
    // 3. Return NULL if there is not enough contiguous blocks
    printf("error: insufficient contiguous memory available\n");
    return NULL;
}

int main()
{
    int poolSize = 1024;
    int blockSize = 128;

    struct Pool *pool = initPool(poolSize, blockSize);

    printf("Memory pool created with %d blocks of size %d\n", pool->nBlocks, blockSize);


    return 0;
}

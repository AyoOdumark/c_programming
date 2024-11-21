#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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
        fprintf(stderr, "error: poolSize is not divisible by blockSize\n");
        return NULL;
    }

    size_t num_blocks = poolSize / blockSize;

    // 2. Allocate and initialize pool structure
    struct Pool *poolPtr = (struct Pool *)malloc(sizeof(struct Pool));
    
    if (poolPtr == NULL) {
        fprintf(stderr, "error: malloc failed allocating memory pool structure!\n");
        return NULL;
    }

    poolPtr->size = poolSize;
    poolPtr->nBlocks = num_blocks;

    // 3. Allocate the contiguous block for the memory pool itself
    poolPtr->addr = malloc(poolSize);
    if (poolPtr->addr == NULL) {
        fprintf(stderr, "error: malloc failed allocating memory pool!\n");
        free(poolPtr);  // Clean up allocated Pool structure
        return NULL;
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
        fprintf(stderr, "error: unable to allocate memory. size must be multiple of %zu\n", blockSize);
        return NULL;
    }

    // 2. Search for enough contiguous blocks and allocate
    int nBlocksNeeded = size / blockSize;

    struct FreeBlock *current = pool->list;  // current node
    struct FreeBlock *prev = NULL;
    struct FreeBlock *start = NULL;  // Keep track of the start block or beginning of the allocation
    int contiguousCount = 0;

    while (current != NULL) {
        if (contiguousCount == 0) {
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

void dealloc(struct Pool *pool, struct FreeBlock *ptr)
{
    
    struct FreeBlock *current = pool->list;
    struct FreeBlock *prev = NULL;

    while (current != NULL && current < ptr) {
        prev = current;
        current = current->next;
    }

    ptr->next = current;

    if (prev == NULL) {
        pool->list = ptr;
    } else {
        prev->next = ptr;
    }
}

//Helper function to print the free list (for debugging purposes)
void printFreeList(struct Pool *pool) 
{
    struct FreeBlock *current = pool->list;
    printf("Free List:\n");
    while (current != NULL) {
        printf("Block at %p, Size: %zu\n", (void *)current, current->size);
        current = current->next;
    }
    printf("\n");
}

int main()
{
    int poolSize = 1024;
    int blockSize = 128;

    // Initialize the memory pool
    struct Pool *pool = initPool(poolSize, blockSize);
    printf("Memory pool created with %d blocks of size %d\n", pool->nBlocks, blockSize);
    printFreeList(pool);
    
    // Test 1: Allocation of one block
    void *block1 = alloc(pool, blockSize);
    if (block1 != NULL) {
        printf("Test 1 Passed: Allocated first block at %p\n", block1);
    } else {
        printf("Test 1 Failed: Allocation return NULL\n");
    }
    printFreeList(pool);

    // Test 2: Allocation of a second block and ensure it is different from first
    void *block2 = alloc(pool, blockSize);
    if (block2 != NULL && block1 != block2) {
        printf("Test 2 Passed: Allocated second block at %p\n", block2);
    } else {
        printf("Test 2 Failed: second allocation error\n");
    }
    printFreeList(pool);
    
    // Test 3: Fill up pool with allocations
    bool all_allocated = true;
    for (int i = 0; i < pool->nBlocks - 2; i++) {
        if (alloc(pool, blockSize) == NULL) {
            all_allocated = false;
            break;
        }
    }
    if (all_allocated) {
        printf("Test 3 Passed: All blocks allocated successfully\n");
    } else {
        printf("Test 3 Failed: Unable to allocate all blocks\n");
    }
    printFreeList(pool);
    
    // Test 4: Try allocating one more block than is available
    void *blockExtra = alloc(pool, blockSize);
    if (blockExtra == NULL) {
        printf("Test 4 Passed: Allocation returns NULL when pool is exhausted\n");
    } else {
        printf("Test 4 Failed: Allocation should not succeed when pool is exhausted\n");
    }
    printFreeList(pool);
   
    // Test 5: Deallocate one block and reallocate it
    dealloc(pool, (struct FreeBlock *)block1);
    printf("Test 5: Deallocated first block\n");
    printFreeList(pool);
    
    void *block1_realloc = alloc(pool, blockSize);
    if (block1_realloc == block1) {
        printf("Test 5 Passed: Reallocation of a deallocated block successful\n");
    } else {
        printf("Test 5 Failed: Deallocation or reallocation error!\n");
    }
    printFreeList(pool);
    
    // Test 6: Deallocate multiple blocks
    dealloc(pool, (struct FreeBlock *)block2);
    printf("Test 6: Deallocated second block\n");
    printFreeList(pool);

    // Test 7: Free all memory and validate the pool can be reused
    return 0;
}

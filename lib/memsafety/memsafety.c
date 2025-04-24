#include "memsafety.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
    #pragma section(".CRT$XCU", read)
    #define CONSTRUCTOR_INTERNAL __declspec(allocate(".CRT$XCU"))
#else
    #define CONSTRUCTOR_INTERNAL
#endif

static memoryNode *startingNode = NULL;
static memoryNode *endingNode = NULL;

static memoryNode *mem_node(void *pointer) {
    memoryNode *returnValue = (memoryNode *)malloc(sizeof(memoryNode));
    if (!returnValue) {
        printf("Heap allocation failure. Terminating\n");
        exit(1);
    }
    returnValue -> nextNode = NULL;
    returnValue -> pointer = pointer;
    return returnValue;
}

void node_printout(void) {
    if (!startingNode) {
        printf("Memory linked list is uninitialized\n");
        return;
    }
    memoryNode *currentNode = startingNode;
    while (currentNode -> nextNode != NULL) {
        printf("0x%p, ", currentNode -> pointer);
        currentNode = currentNode -> nextNode;
    }
    printf("0x%p\n", currentNode -> pointer);
    return;
}

void * ALLOCATOR_ATTRIBS malloc_ac(size_t size) {
    void *pointer = malloc(size);
    if (!pointer) {
        printf("Heap allocation failure. Terminating\n");
        exit(1);
    }
    if (!startingNode) { // if starting node has not been created, create it with the first allocated pointer.
        startingNode = mem_node(pointer);
        endingNode = startingNode;
        return pointer;
    }
    endingNode -> nextNode = mem_node(pointer);
    endingNode = endingNode -> nextNode;
    return pointer;
}

void * ALLOCATOR_ATTRIBS calloc_ac(size_t size1, size_t size2) {
    void *pointer = calloc(size1, size2);
    if (!pointer) {
        printf("Heap allocation failure. Terminating\n");
        exit(1);
    }
    if (!startingNode) { // if starting node has not been created, create it with the first allocated pointer.
        startingNode = mem_node(pointer);
        endingNode = startingNode;
        return pointer;
    }
    endingNode -> nextNode = mem_node(pointer);
    endingNode = endingNode -> nextNode;
    return pointer;
}

void *realloc_ac(void *pointer, size_t size) {
    if (!startingNode || !pointer) return NULL;
    void *returnPointer = pointer;
    memoryNode *currentNodeToReallocate = startingNode;
    while (currentNodeToReallocate -> pointer != pointer && currentNodeToReallocate -> nextNode) {
        currentNodeToReallocate = currentNodeToReallocate -> nextNode;
    }
    if (currentNodeToReallocate -> pointer == pointer) {
        returnPointer = realloc(pointer, size);
        if (!returnPointer) {
            printf("Heap allocation failure. Terminating\n");
            exit(1);
        }
        currentNodeToReallocate -> pointer = returnPointer;
        return returnPointer;
    }
    return NULL;
}

void free_ac(void *pointer) {
    memoryNode *currentNodeToFree = startingNode;
    memoryNode *previousNode = NULL;
    memoryNode *nextNode = NULL;
    if (!startingNode || !pointer) {
        return;
    }
    if (pointer == startingNode -> pointer) {
        startingNode = startingNode -> nextNode;
        free(pointer);
        free(currentNodeToFree);
        return;
    }
    while (currentNodeToFree -> pointer != pointer && currentNodeToFree -> nextNode) {
        previousNode = currentNodeToFree;
        currentNodeToFree = currentNodeToFree -> nextNode;
        nextNode = currentNodeToFree -> nextNode;
    }
    if (pointer == endingNode -> pointer) {
        free(pointer);
        free(endingNode);
        endingNode = previousNode;
        endingNode -> nextNode = NULL;
        return;
    }
    if (currentNodeToFree -> pointer == pointer) {
        free(pointer);
        free(currentNodeToFree);
        previousNode -> nextNode = nextNode;
    }
    return;
}

void mem_cc(void) { // call this at the end of main() to ensure any dangling pointers are handled.
    if (!startingNode) return;
    memoryNode *currentNodeToFree = startingNode;
    memoryNode *swapperNode = NULL;
    while (currentNodeToFree -> nextNode) {
        swapperNode = currentNodeToFree -> nextNode;
        free(currentNodeToFree -> pointer);
        free(currentNodeToFree);
        currentNodeToFree = swapperNode;
    }
    free(currentNodeToFree -> pointer);
    free(currentNodeToFree);
    startingNode = NULL;
    endingNode = NULL;
}

int CONSTRUCTOR register_mem_cc(void) {
    fopen("./THISRAN.txt", "w");
    return atexit(mem_cc);
}

CONSTRUCTOR_INTERNAL int (*memsafetyStartup)(void) = register_mem_cc;

#ifdef __cplusplus
}
#endif
#pragma once
#include <stdio.h>
#include <stdlib.h>

//  A warning; for anyone modifying this code, the startingNode linked list is not thread safe and should NOT be used in programs with volatile memory allocations.
//  ImGui does not use the smartMalloc and smartCalloc allocators because the linked list will decouple if used in threaded or volatile programs like ImGui.
//  Fortunately, ImGui handles its own memory allocation.
//  This allocator is only designed to be used in client code for tasks running on a single thread; its behavior is undefined for any other task!
//  This header can be safely used in a multi-threaded program as long as the startingNode linked list is only accessed from the primary thread.
//  Furthermore, malloc(), calloc(), and free() can be used alongside this allocator. This allocator is simply designed to clean up allocations at the end of runtime.
//  The smart allocators and smart free functions in this code can still be used to dynamically allocate and free memory as needed, provided it is done on one thread.

typedef struct memoryNode {
    void *pointer;
    struct memoryNode *nextNode;
} memoryNode;

static memoryNode *startingNode = NULL;

memoryNode *constructMemoryNode(void *pointer) {
    memoryNode *returnValue = (memoryNode *)malloc(sizeof(memoryNode));
    returnValue -> nextNode = NULL;
    returnValue -> pointer = pointer;
    return returnValue;
}

void printOutNodePointers() {
    if (!startingNode) return;
    memoryNode *currentNode = startingNode;
    while (currentNode -> nextNode != NULL) {
        printf("0x%p, ", currentNode -> pointer);
        currentNode = currentNode -> nextNode;
    }
    printf("0x%p\n", currentNode -> pointer);
    return;
}

void *smartMalloc(size_t size) {
    void *pointer = malloc(size);
    if (!startingNode) { // if starting node has not been created, create it with the first allocated pointer.
        startingNode = constructMemoryNode(pointer);
        return pointer;
    }
    memoryNode *currentNodeToAllocate = startingNode;
    while (currentNodeToAllocate -> nextNode) {
        currentNodeToAllocate = currentNodeToAllocate -> nextNode;
    }
    currentNodeToAllocate -> nextNode = constructMemoryNode(pointer);
    return pointer;
}

void *smartCalloc(size_t size1, size_t size2) {
    void *pointer = calloc(size1, size2);
    if (!startingNode) { // if starting node has not been created, create it with the first allocated pointer.
        startingNode = constructMemoryNode(pointer);
        return pointer;
    }
    memoryNode *currentNodeToAllocate = startingNode;
    while (currentNodeToAllocate -> nextNode) {
        currentNodeToAllocate = currentNodeToAllocate -> nextNode;
    }
    currentNodeToAllocate -> nextNode = constructMemoryNode(pointer);
    return pointer;
}

void *smartRealloc(void *pointer, size_t size) {
    if (!startingNode) return pointer;
    void *returnPointer = pointer;
    memoryNode *currentNodeToReallocate = startingNode;
    while (currentNodeToReallocate -> pointer != pointer && currentNodeToReallocate -> nextNode) {
        currentNodeToReallocate = currentNodeToReallocate -> nextNode;
    }
    if (currentNodeToReallocate -> pointer == pointer) {
        returnPointer = realloc(pointer, size);
        currentNodeToReallocate -> pointer = returnPointer;
    }
    return returnPointer;
}

void smartFree(void *pointer) {
    if (!startingNode) return;
    memoryNode *currentNodeToFree = startingNode;
    memoryNode *previousNode = NULL;
    memoryNode *nextNode = NULL;
    while (currentNodeToFree -> pointer != pointer && currentNodeToFree -> nextNode) {
        previousNode = currentNodeToFree;
        currentNodeToFree = currentNodeToFree -> nextNode;
        nextNode = currentNodeToFree -> nextNode;
    }
    if (currentNodeToFree -> pointer == pointer) {
        free(pointer);
        free(currentNodeToFree);
        previousNode -> nextNode = nextNode;
    }
    return;
}

void memoryCleanup() { // call this at the end of main() to ensure any dangling pointers are handled.
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
}
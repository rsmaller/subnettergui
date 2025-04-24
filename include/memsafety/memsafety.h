#ifdef __cplusplus
extern "C" {
#endif

#pragma once
#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
    #define ALLOCATOR_ATTRIBS
    #define CONSTRUCTOR
#elif defined(__GNUC__)
    #define ALLOCATOR_ATTRIBS __attribute__((malloc))
    #define CONSTRUCTOR __attribute__((constructor))
#else
    #define ALLOCATOR_ATTRIBS
    #define CONSTRUCTOR
#endif

//  A warning; for anyone modifying this code, the startingNode linked list is not thread safe and should NOT be used in programs with volatile memory allocations.
//  ImGui does not use the malloc_ac and calloc_ac allocators because the linked list will decouple if used in threaded or volatile programs like ImGui.
//  Fortunately, ImGui handles its own memory allocation.
//  This allocator is only designed to be used in client code for tasks running on a single thread; its behavior is undefined for any other task!
//  This header can be safely used in a multi-threaded program as long as the startingNode linked list is only accessed from the primary thread.
//  Furthermore, malloc(), calloc(), and free() can be used alongside this allocator. This allocator is simply designed to clean up allocations at the end of runtime.
//  The smart allocators and smart free functions in this code can still be used to dynamically allocate and free memory as needed, provided it is done on one thread.



typedef struct memoryNode {
    void *pointer;
    struct memoryNode *nextNode;
} memoryNode;

void * ALLOCATOR_ATTRIBS malloc_ac(size_t);

void * ALLOCATOR_ATTRIBS calloc_ac(size_t, size_t);

void mem_cc(void);

void *realloc_ac(void *, size_t);

void free_ac(void *);

void node_printout(void);

#ifdef __cplusplus
}
#endif
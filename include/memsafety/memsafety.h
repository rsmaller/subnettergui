#pragma once
#include <stdlib.h>
#include <string.h>

// memory management variables
int currentFreeArrayIndex = 0;
int freeArraySize = 4;
void **mallocedPointerArray = (void **)malloc(sizeof(void *) * (unsigned long)freeArraySize);
void *(smartMalloc)(size_t);
void *(smartCalloc)(size_t, size_t);

//  memory safe allocator functions
void *smartMalloc(size_t size) {
	void *mallocedPointer = malloc(size);
    if (currentFreeArrayIndex >= (int)(freeArraySize / 2)) {
        freeArraySize *= 2;
        mallocedPointerArray = (void **)realloc(mallocedPointerArray, sizeof(void *) * (unsigned long)freeArraySize);
    }
	mallocedPointerArray[currentFreeArrayIndex++] = mallocedPointer;
	return mallocedPointer;
}

void *smartCalloc(size_t size1, size_t size2) {
	void *callocedPointer = calloc(size1, size2);
    if (currentFreeArrayIndex >= (int)(freeArraySize / 2)) {
        freeArraySize *= 2;
        mallocedPointerArray = (void **)realloc(mallocedPointerArray, sizeof(void *) * (unsigned long)freeArraySize);
    }
	mallocedPointerArray[currentFreeArrayIndex++] = callocedPointer;
	return callocedPointer;
}

void *smartRealloc(void *oldMallocedPointer, size_t size) {
	void *newMallocedPointer = realloc(oldMallocedPointer, size);
	for (int i=0; i<freeArraySize; i++) {
		if (mallocedPointerArray[i] == oldMallocedPointer) {
			mallocedPointerArray[i] = newMallocedPointer;
		}
	}
	return newMallocedPointer;
}

void freeFromArray(void **arrayToFree) {
	for (int i=0; i<currentFreeArrayIndex; i++) {
		if (arrayToFree[i] != NULL) free(arrayToFree[i]);
	}
	free(arrayToFree);
}

void freeOnePointerFromArray(void **arrayToFree, void *pointerToFree) {
	for (int i=0; i<currentFreeArrayIndex; i++) {
		if (arrayToFree[i] == pointerToFree) {
			free(pointerToFree);
			arrayToFree[i] = NULL;
		}
	}
}

void smartFree(void *pointerToFree) {
	freeOnePointerFromArray(mallocedPointerArray, pointerToFree);
}

void memSafetyCleanUp() { // this must be called at the end of the main function
    freeFromArray(mallocedPointerArray);
}
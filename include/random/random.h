#pragma once
#include <assert.h>
#include <time.h>
#include <stdlib.h>

unsigned int getRandomInteger(unsigned int start, unsigned int end) {
	assert(end > start);
	unsigned long long int range = end - start + 1;
	return (unsigned int)(rand() % range + start);
}

unsigned int getRandomIPNumber() {
	return ((char)rand() << 24) + ((char)rand() << 16) + ((char)rand() << 8) + (char)rand();
}

unsigned short *getRandomIPv6Number() { // returns size 8 array for IPv6 number
	unsigned short *returnArray = (unsigned short *)calloc(8, 2);
	rand();
	for (int i=0; i<8; i++) {
		returnArray[i] = (unsigned short)(rand() % 0xffffU);
	}
	return returnArray;
}

unsigned short *getRandomMACNumber() { // returns size 8 array for IPv6 number
	unsigned short *returnArray = (unsigned short *)calloc(3, 2);
	rand();
	for (int i=0; i<3; i++) {
		returnArray[i] = (unsigned short)(rand() % 0xffffU);
	}
	return returnArray;
}

bool chance(unsigned int numerator, unsigned int denominator) {
	unsigned int generatedNumber = getRandomInteger(1, denominator);
	return generatedNumber <= numerator;
}

double testChance(int numerator, int denominator) {
	int totalTrue = 0;
	int numberOfTests = 100000;
	for (int i=0; i<numberOfTests; i++) {
		if (chance(numerator, denominator)) {
			totalTrue++;	
		}
	}
	return (double)totalTrue / (double)numberOfTests;
}

unsigned int getRandomCIDR() {
	return rand() % 33;
}

#pragma once
#include <assert.h>
#include <time.h>
#include <stdlib.h>

unsigned int getRandomInteger(unsigned int start, unsigned int end) {
	assert(end > start);
	unsigned int range = end - start + 1;
	return (unsigned int)((unsigned int)rand() % range + start);
}

unsigned short getRandomShort() {
	return (unsigned short)((unsigned short)rand() % (unsigned short)0xffffU);
}

unsigned int getRandomIPNumber() {
	return (unsigned int)((char)rand() << 24) + (unsigned int)((char)rand() << 16) + (unsigned int)((char)rand() << 8) + (unsigned int)(char)rand();
}

unsigned short *getRandomIPv6Number() { // returns size 8 array for IPv6 number
	unsigned short *returnArray = (unsigned short *)calloc(8, 2);
	rand();
	for (int i=0; i<8; i++) {
		returnArray[i] = (unsigned short)((unsigned short)rand() % (unsigned short)0xffffU);
	}
	return returnArray;
}

unsigned short *getRandomMACNumber() { // returns size 8 array for IPv6 number
	unsigned short *returnArray = (unsigned short *)calloc(3, 2);
	rand();
	for (int i=0; i<3; i++) {
		returnArray[i] = (unsigned short)((unsigned short)rand() % (unsigned short)0xffffU);
	}
	return returnArray;
}

bool chance(unsigned int numerator, unsigned int denominator) {
	unsigned int generatedNumber = getRandomInteger(1, denominator);
	return generatedNumber <= numerator;
}

double testChance(unsigned int numerator, unsigned int denominator) {
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
	return (unsigned int)rand() % 33;
}

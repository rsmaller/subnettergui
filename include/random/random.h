#pragma once
#include <assert.h>
#include <time.h>
#include <stdlib.h>

unsigned int getRandomInteger(unsigned long long int start, unsigned long long int end) {
	assert(end > start);
	unsigned long long int range = end - start + 1;
	return (unsigned int)(rand() % range);
}

unsigned int getRandomIPNumber() {
	return getRandomInteger(0U, ~0U);
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
	return getRandomInteger(0, 32);
}

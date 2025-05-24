#pragma once

#ifdef __cplusplus
	#include <assert.h>
	#define cast(x, type) (static_cast<type>(x))
#else
	#include <cassert>
	#define cast(x, type) ((type)x)
#endif

inline unsigned int getRandomInteger(const unsigned int start, const unsigned int end) {
	assert(end > start);
	const unsigned int range = end - start + 1;
	return cast(rand(), unsigned int) % range + start;
}

inline unsigned short getRandomShort() {
	return cast((cast(rand(), unsigned short) % cast(0xffffU, unsigned short)), unsigned short);
}

inline unsigned int getRandomIPNumber() {
	return cast(cast(rand(), char) << 24, unsigned int) + cast(cast(rand(), char) << 16, unsigned int) + cast(cast(rand(), char) << 8, unsigned int) + cast(cast(rand(),char),unsigned int);
}

inline unsigned short *getRandomIPv6Number() { // returns size 8 array for IPv6 number
	unsigned short *returnArray = cast(calloc(8, 2), unsigned short *);
	rand();
	for (int i=0; i<8; i++) {
		returnArray[i] = cast(cast(rand(), unsigned short) % cast(0xffffU, unsigned short), unsigned short);
	}
	return returnArray;
}

inline unsigned short *getRandomMACNumber() { // returns size 8 array for IPv6 number
	unsigned short *returnArray = cast(calloc(3, 2), unsigned short *);
	rand();
	for (int i=0; i<3; i++) {
		returnArray[i] = cast(cast(rand(), unsigned short) % cast(0xffffU, unsigned short), unsigned short);
	}
	return returnArray;
}

inline bool chance(const unsigned int numerator, const unsigned int denominator) {
	const unsigned int generatedNumber = getRandomInteger(1, denominator);
	return generatedNumber <= numerator;
}

inline double testChance(const unsigned int numerator, const unsigned int denominator) {
	int totalTrue = 0;
	const int numberOfTests = 100000; // NOLINT
	for (int i=0; i<numberOfTests; i++) {
		if (chance(numerator, denominator)) {
			totalTrue++;	
		}
	}
	return cast(totalTrue, double) / cast(numberOfTests, double);
}

inline unsigned int getRandomCIDR() {
	return cast(rand(),unsigned int) % 33U;
}

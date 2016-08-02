/*
 * six_to_eight.h
 */
#ifndef __SIX_TO_EIGHT_H__

/* Structure to handle three six bit words in 24 bits */
typedef struct fourWords {
	unsigned first:6;
	unsigned second:6;
	unsigned third:6;
	unsigned fourth:6;
} FOURWORDS;

int sixToEight(unsigned char *source, int sourceLength, unsigned char *dest);
int six_to_eight_test(void);

#define __SIX_TO_EIGHT_H__
#endif


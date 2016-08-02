/*
 * crc.h
 */
#ifndef __CRC_H__

/* Function prototypes */
unsigned int svissr_calcrc_8(unsigned char *ptr, int count);
unsigned int svissr_calcrc_6(unsigned char *ptr, int count);

#define __CRC_H__
#endif


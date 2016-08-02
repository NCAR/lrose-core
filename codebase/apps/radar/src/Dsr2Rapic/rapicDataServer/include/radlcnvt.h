#ifndef	__RADLCNVT_H
#define __RADLCNVT_H

// decoding routines
int RLE_6L_radl(char* ipbuffer, int ip_length, s_radl* radl);
// by default RLE_16L_radl will decode RLE16, ie maxval = 15
// Should be called with the rdrscan NumLevels parameter
int RLE_16L_radl(char* ipbuffer, int ip_length, s_radl* radl, int maxval = 15);

int DecodeBinaryRadl(unsigned char* ipbuffer, int ip_length, s_radl* radl);

// encoding routines
// Encode RLE16, RLE32, RLE64, RLE128 radials
int DeltaASCII(unsigned char *Radial, uchar *OutputRadl, int length, int maxopsize);
int SixLevelASCII(unsigned char *Radial, char *OutputRadl, int length, int maxopsize);
int EncodeBinaryRadl(unsigned char *Radial, unsigned char *OutputRadl, int length, int maxopsize);


#endif	/* __RADLCNVT_H */

#ifndef	__RAPICENCODE_H
#define __RAPICENCODE_H

#include <iostream>
#include <string>
#include <sstream>


typedef unsigned char uchar;

// Current implementation limited to 160 levels ASCII RLE format
int Encode_RLE_line(std::string &str, uchar *ipbuff, int ipsize, int lineno, int checkpos = -1);

int encodeArrayToRLEString(uchar* inarray, int width, int height, 
			int levels, std::string &outstring);
int Decode_RLE_line(std::string &str, 
		    uchar *opbuff, int maxopsize, int &lineno, int checkpos = -1);
int decodeArrayFromRLEString(uchar* outarray, int width, int height, 
			std::string &instring);

#endif	/* __RAPICENCODE_H */

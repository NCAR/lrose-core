// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
////////////////////////////////////////////////////////////////////
// sRadl class
//
// unpacked radial array class
//
// RAPIC code (c) BOM Australia
//
// Phil Purdam
//
// Jan 2000
//
/////////////////////////////////////////////////////////////////////

#include "sRadl.hh"
#include "Linebuff.hh"
#include <cstring>
#include <ctype.h>
#include <cstdio>
#include <iostream>
#include <math.h>
using namespace std;

#define  PI  3.14159265358979323846  /* pi *//* copy from math.h */
#define  DEG2RAD (PI/180.0)

const unsigned short sRadl::XLAT_16L[256] = {
  0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
  0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
  0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
  0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
  0xff,   0xfdfd, 0x0010, 0xff,   0xfd03, 0xff,   0x8303, 0x0011,
  0xfd02, 0x8302, 0x0012, 0x8100, 0x0013, 0xff00, 0x8000, 0xfdfe,
  0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
  0xff,   0xff,   0x0014, 0x0015, 0x80ff, 0x0016, 0x8001, 0x0017,
  0x83fd, 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006,
  0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E,
  0x000F, 0x0018, 0x0019, 0xff02, 0x8002, 0x8102, 0x8202, 0xff03,
  0x8003, 0x8103, 0x001a, 0xfefd, 0x83fe, 0x82fd, 0x001b, 0x001c,
  0xff,   0xfffd, 0x80fd, 0x81fd, 0xfefe, 0xfffe, 0x80fe, 0x81fe,
  0x82fe, 0xfdff, 0xfeff, 0xffff, 0x81ff, 0x82ff, 0x83ff, 0xfd00,
  0xfe00, 0x8200, 0x8300, 0xfd01, 0xfe01, 0xff01, 0x8101, 0x8201,
  0x8301, 0xfe02, 0x001d, 0xfe03, 0x001e, 0x8203, 0x001f, 0xff,
  0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 
  0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 
  0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 
  0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 
  0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 
  0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 
  0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 
  0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 
  0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 
  0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 
  0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 
  0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f, 
  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 
  0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f, 
  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 
  0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f 
};

const char sRadl::TestRadial[] = 
"%262B+13+5+7+6+7+5+7+8+6+6+6+5+7+15dejk2-77ll6l4l5l2l5%263B+12+7+6+6+7+6+7+6+6+7+6+6+6+15dejk2-85+-5l15%264B+13+5+7+7+6+6+7+5+9+4+7+5+6+16djjk1-99l9l%265B+13+5+7+6+7+7+5+6l+5+7+5+7+5+16djjp2-78l8l19l";

const char sRadl::A2NXlat[49] = {
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,
  0x10,0x11,0x12,0x13,0x14,0x15,0x16,
  0x20,0x21,0x22,0x23,0x24,0x25,0x26,
  0x30,0x31,0x32,0x33,0x34,0x35,0x36,
  0x40,0x41,0x42,0x43,0x44,0x45,0x46,
  0x50,0x51,0x52,0x53,0x54,0x55,0x56,
  0x60,0x61,0x62,0x63,0x64,0x65,0x66
};

const unsigned char sRadl::absolute[160] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
  '\"','\'','*', ',', ':', ';', '=', '\?',
  'Q', 'R', 'Z', '^', '_', 'z', '|', '~',  
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 
  0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 
  0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 
  0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 
  0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 
  0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 
  0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 
  0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 
  0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 
  0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 
  0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 
  0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 
  0xE8, 0xE9, 0xEa, 0xEb, 0xEc, 0xEd, 0xEe, 0xEf, 
  0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 
  0xF8, 0xF9, 0xFa, 0xFb, 0xFc, 0xFd, 0xFe, 0xFf
};

const char sRadl::Delta[256] = {
  '.', '>', 'T', 'X', 0, 0, 0, 0, 0, 0, 0, 0, 0, 'b', 'f', '<',
  '+', 'v', 'U', 'Y', 0, 0, 0, 0, 0, 0, 0, 0, 0, 'c', 'g', 'l',
  'q', 'w', 'V', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, ']', 'h', 'm',
  'r', 'x', ')', '&', 0, 0, 0, 0, 0, 0, 0, 0, 0, '@','\\', 'n',
  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
// convert nibble to ASCII
  'o', 's', '(', '$', 0, 0, 0, 0, 0, 0, 0, 0, 0, '!', '/', 'i',
  'p', 't', 'y', '{', 0, 0, 0, 0, 0, 0, 0, 0, 0, '[', 'd', 'j',
  '-', 'u', 'S', 'W', 0, 0, 0, 0, 0, 0, 0, 0, 0, 'a', 'e', 'k'
};


const char sRadl::SixLevelEncode[64] = {
  'A','B','C','D','E','F','G','A',
  'H','I','J','K','L','M','N','A',
  'O','P','Q','R','S','T','U','A',
  'V','W','X','Y','a','b','c','A',
  'd','e','f','g','h','i','j','A',
  'k','l','m','n','o','p','q','A',
  'r','s','t','u','v','w','x','A',
  'A','A','A','A','A','A','A','A'
};

sRadl::sRadl(int BuffSize /* = 0 */)

{

  az = az1 = az2 = 0;
  az_hr = 0;
  el = el1 = el2 = 0;
  startrng = 0;
  rngres = 0;
  data_size = 0;
  buffsize = 0;
  undefinedrng = 0;
  numlevels = 0;
  mode = INDEX;
  data_type = e_refl;
  bfdata_type = bf_none;
  radltype = SIMPLE;
  data = NULL;
  LvlTbl = NULL;
  Values = NULL;

  if (!BuffSize) {
    buffsize = RADLBUFFSIZE;
  } else {
    buffsize = BuffSize;
  }

  data = new unsigned char[buffsize];

}


sRadl::~sRadl()

{
  
  if (Values) delete[] Values;
  if (data) delete[] data;

}


// zero out this radial

void sRadl::Clear()

{

  az = az1 = az2 = el = el1 = el2 = 0;
  startrng = rngres = data_size = undefinedrng = 0;
  mode = INDEX;
  if (data) memset(data, 0, buffsize);
  if (Values) memset(Values, 0, buffsize * sizeof(float));

}
    		

int sRadl::IndexAtRange(float rng, float *idxrng)

{

  int intrng;
  int idx;

  if (!data_size) return -1;

  intrng = int(rng * 1000);// convert rng to metres
  intrng -= startrng;	// 
  if (intrng < 0) return 0;	// if < 0,  return 0

  if (intrng > (data_size * rngres)) {
    if (idxrng) {
      *idxrng = (startrng + ((data_size - 1) * rngres)) / 1000.;
      return data_size-1;	// if max,  return max
    }
    else return -1;
  }

  idx = intrng / rngres;
  if (idxrng) *idxrng = (startrng + (idx * rngres)) / 1000.;
  if (idx < buffsize) return idx;
  else return -1;

}



char sRadl::DataAtRange(float rng)

{

  int idx;
  if ((idx = IndexAtRange(rng)) != -1) return data[idx];
  else return 0;

}

// Convert the radial to a tangent plane (0deg el) radial
// Use a maxima fn where multiple src values map to one dest value 

void sRadl::TanPlaneRadl(float *cosel)

{

  float CosEl;
  int srcidx = 0, destidx = 0;
  
  if (!data_size) return;

  char  *tempdata = new char[data_size];
  float *tempval = 0;
  if (Values) tempval = new float[data_size];

  if (!cosel) CosEl = cos(el*DEG2RAD);
  else CosEl = *cosel;
  memset(tempdata, 0, data_size);
  if (Values) memset(tempval, 0, sizeof(float)*data_size);
  for (srcidx = 0; srcidx < data_size; srcidx++) {
    destidx = int(srcidx * CosEl);      
    if (Values) {
      if (Values[srcidx] > tempval[destidx]) {
	tempval[destidx] = Values[srcidx];
	tempdata[destidx] = data[srcidx];
      }
    }
    if (data[srcidx] > tempdata[destidx])
      tempdata[destidx] = data[srcidx];
  }
  data_size = destidx + 1;
  memcpy(data, tempdata, data_size);
  if (Values) memcpy(Values, tempval, data_size * sizeof(float));
  if (tempval) delete[] tempval;
  if (tempdata) delete[] tempdata;

}


// pad radial to given range

int sRadl::PadRadl(int padsize)

{
  
  int padno = 0;
    
  if (padsize <= data_size) return data_size;
  if (padsize > buffsize) padsize = buffsize;
  padno = padsize - data_size;
  memset(&data[data_size], 0, padno);
  if (Values) 
    for (int x = data_size; x < padsize; x++)
      Values[x] = 0;
  data_size = padsize;
  return padsize;

}    

void sRadl::RngRes2000to1000()

{

  unsigned char *tempbuff = new unsigned char[buffsize];
  unsigned char *ipdata, *opdata;
  int opcount = 0, ipcount = 0;
  
  if (rngres != 2000) {
    printf("sRadl::RngRes2000to1000 ERROR Current rngres = %d\n", rngres);
    return;
  }

  memcpy(tempbuff, data, data_size);	// copy 2000m data to tempbuff
  ipdata = tempbuff;	    // 2000m data
  opdata = data;	    // 1000m data
  while ((ipcount < data_size) && (opcount < buffsize)) {
    *opdata = *ipdata;  // copy 2000m data twice 
    opdata++;
    *opdata = *ipdata;
    opdata++;
    ipdata++;
    ipcount++;
    opcount += 2;
  }
  data_size = opcount;
  rngres = 1000;	
  delete[] tempbuff;

}

// convert from float array to index values

void sRadl::ThresholdFloat(float *floatarray,
			   int size,
			   LevelTable *thresh_table)

{

  int	x = 0, lastlvl;
  float	*inval, *nextlvl;
  unsigned char	*outval;
  unsigned char	lvl;
  
  if (size > buffsize) size = buffsize;    
  if (!thresh_table) thresh_table = LvlTbl;
  if (!thresh_table) return;
  if (floatarray)
    inval = floatarray;
  else {
    inval = Values; // if floatarray not passed, assume Values should be used
    if (!size)
      size = buffsize;
  }
  if (!inval) return;
  outval = data;
  lastlvl = thresh_table->numlevels - 1;
  for (x = 0; x < size; x++) {
    lvl = 0;
    nextlvl = thresh_table->Levels;
    nextlvl++;	// level compare to Levels[lvl+1]
    while ((lvl < lastlvl) &&
	   (*inval >= *nextlvl)) {
      lvl++;
      // if (lvl < lastlvl) 
      nextlvl++;	// don't step past last level entry
      // it is > last level, and will drop throug
    }
    *outval = lvl;
    outval++;
    inval++;
  }
  outval = &data[size-1];  // point to last value
  while ((size > 0) && (*outval == 0)) {
    size--;		    
    outval--;
  }
  data_size = size;

}


void sRadl::Encode16lvlAz(char *outstring)

{

  char *strpnt = outstring;
  TruncateData();
  sprintf(strpnt, "%%%03d", az / 10);
  strpnt += strlen(strpnt);
  DeltaASCII(data, strpnt, data_size);

}


void sRadl::Encode16lvlEl(char *outstring)

{

  char *strpnt = outstring;
  TruncateData();
  sprintf(strpnt, "%%%04.1f", (float)el / 10.0);
  strpnt += strlen(strpnt);
  DeltaASCII(data, strpnt, data_size);

}


void sRadl::Encode6lvlAz(char *outstring)

{

  char *strpnt = outstring;
  TruncateData();
  sprintf(strpnt, "%%%03d", az / 10);
  strpnt += strlen(strpnt);
  SixLevelASCII(data, strpnt, data_size);

}


void sRadl::Encode6lvlEl(char *outstring)

{

  char *strpnt = outstring;
  TruncateData();
  sprintf(strpnt, "%%%04.1f", (float)el / 10.0);
  strpnt += strlen(strpnt);
  SixLevelASCII(data, strpnt, data_size);

}


void sRadl::EncodeAz(char *outstring)

{

  if (numlevels == 6)
    Encode6lvlAz(outstring);
  else if (numlevels <= 160)
    Encode16lvlAz(outstring);
  else
    fprintf(stderr, "sRadl::EncodeEl - Binary data encode not implemented\n");

}


void sRadl::EncodeEl(char *outstring)

{
  if (numlevels == 6)
    Encode6lvlEl(outstring);
  else if (numlevels <= 160)
    Encode16lvlEl(outstring);
  else
    fprintf(stderr, "sRadl::EncodeEl - Binary data encode not implemented\n");
}


void sRadl::TruncateData()

{
  if (data_size == 0) return;
  unsigned char* lcldata = &data[data_size-1];
  while(data_size) {
    if(*lcldata--)
      break;
    data_size--;
  }
}

// static member functions

void sRadl::dump_radl(sRadl* radl)
{
  fprintf(stderr, "el, az, ngates: %g %g %d\n",
	  radl->el / 10.0, radl->az / 10.0, radl->data_size);
}

void sRadl::dump_radl_full(sRadl* radl)
{
  fprintf(stderr, "el, az, ngates: %g %g %d\n",
	  radl->el / 10.0, radl->az / 10.0, radl->data_size);
  int	x;
  char	c;
  for (x=0; x<radl->data_size; x++) {
    c = radl->data[x];
    fprintf(stderr, "%d ", c);
  }
  fprintf(stderr, "\n");
}

// read angle of radial, either %ddd or %ddd.t
// returns no of chars used

int sRadl::rd_radl_angle(char* instring,rdr_angle& angle)
{
  int degrees = 0;
  int tenths = 0;
  int n = 0;
  int args = 0;
  float el;
  int tmofs;
  
  angle = 0;
  //  if (sscanf(instring,"%%%d%n.%d%n",&degrees,&n,&tenths,&n) > 0)
  //    angle = (degrees * 10) + tenths;
  if ((args = sscanf(instring,"%*c%d%n.%d%n,%f,%d=%n",
		     &degrees,&n,&tenths,&n, &el, &tmofs, &n)) > 0)
    angle = (degrees * 10) + tenths;
  return n;
}

bool sRadl::null_radl(char* instring)
{
  rdr_angle angle;

  if (instring[rd_radl_angle(instring,angle)] == 0)
    return true;
  else 
    return false;
}

//////////////////////////////////////////////////
// Decode an ASCII string into an sradl structure

int sRadl::RLE_16L_radl(char* instring, sRadl* radl, int maxval)
{
  short  temp;
  short  first;
  short  second;
  unsigned char ch = 0;
  int  runcount;
  unsigned char  last = 0;
  int  n;
  unsigned char* ipbuffer;
  bool done = false;
  // char anglbuff[4];
  bool debug = false;
    
  //memcpy(anglbuff,ipbuffer,4);
  //ipbuffer = TestRadial;
  //memcpy(ipbuffer,anglbuff,4);

  ipbuffer = (unsigned char*)instring;
  if ((n = rd_radl_angle((char*)ipbuffer,radl->az)) > 0)
    ipbuffer += n;
  else 
    return -1;					// bad angle, fail convert
  radl->data_size = 0;				// init size
  do {
    ch = *ipbuffer;
    ipbuffer++;
    if ((ch == CTRLZ) || (ch == CR) || (ch == 0))
      break;
    if(isdigit(ch)) {				// process runcount
      runcount = ch - '0';
      while(isdigit(*ipbuffer)) {
	runcount *= 10;
	runcount += (*ipbuffer - '0');
	ipbuffer++;					// next char
      }
      if ((radl->data_size + runcount) > radl->buffsize) {
	runcount = (radl->buffsize - radl->data_size);
	done = true;
      }
      memset(radl->data+radl->data_size,last,runcount);
      radl->data_size += runcount;
    }
    else {
      temp = XLAT_16L[ch];
      if (temp & 0x8000) {	     /* delta encoded */
	second = temp & 0xff;
        first = (temp >> 8) & 0xff;
	if ((first & 0xf0) != 0xf0)	// if not negative
	  first &= 0x7f;			// strip delta flag from value
	last += first;
	radl->data[radl->data_size] = last;
	radl->data_size++;
	last += second;
	radl->data[radl->data_size] = last;
	radl->data_size++;
      }
      else {			     /* absolute value */
	last = temp;
	if (last <= maxval) {
	  radl->data[radl->data_size] = char(last);
	  radl->data_size++;
	}
      }
    }
    if (last > maxval) {
      if (debug) {
	printf("RLE_16 ERR n=%d ch=%c ip=%s\n",n,ch,instring);
	dump_radl(radl);
      }
      done = true;    // bad value, terminate conversion
      last = maxval;
    }
  } while(!done);
  //if (debug) dump_radl(radl);
  return 0;
}

// Decode an ASCII string into an sradl structure

int sRadl::RLE_6L_radl(char* ipbuffer, sRadl* radl)

{
  unsigned char    ThisByte;
  int     RptCount;
  float	  ftemp;
  int     n;
  bool done = false;
  
  radl->data_size = 0; // init size
  radl->az = 0;
  if (sscanf(ipbuffer," %%%f%n",&ftemp,&n) == 1) // get radial angle
    radl->az = int(ftemp * 10);
  else return -1; // bad angle, fail convert
  ipbuffer += n; // skip past % and angle parts
  RptCount = 0;
  while (!done) {
    ThisByte = *ipbuffer;
    if (ThisByte >= 'A' && ThisByte <= 'Y') 
      ThisByte -= 'A'; 
    else { 
      if (ThisByte >= 'a' && ThisByte <= 'x')  
	ThisByte -= 'H'; 
      else 
        ThisByte = 0xff;
    }
    if (ThisByte != 0xff) {
      ThisByte = A2NXlat[ThisByte];
      RptCount = 0;
      while (isdigit(*(ipbuffer+1))) {
        RptCount = (RptCount * 10) + (*(ipbuffer+1)-'0');
        ipbuffer++;
      }
      RptCount++;
      if ((radl->data_size + (RptCount*2)) > radl->buffsize) {
        RptCount = (radl->buffsize - radl->data_size) / 2;
	done = true;
      }
      while (RptCount--) {
        radl->data[radl->data_size] = ThisByte & 0x0f;
        radl->data[radl->data_size+1] = ThisByte >> 4;
        radl->data_size +=2;
      }
    }
    else 
      done = true;
    ipbuffer++;
    }
  return 0;
}

void sRadl::DeltaASCII(unsigned char *Radial, char *OutputRadl, int length)

{

  int  count;
  unsigned char oldval;
  unsigned char newval = 0;
  int  delta;
  bool first;
  int deltapair = 0;
  int RunCount;

  if (!(Radial && OutputRadl)) 
    return; 
  oldval = (unsigned char)200; // ensure first time in lays down an absolute
  first = true;
  RunCount = 0;
  for(count = 0; count < length; count++) {
    newval = *Radial++; // get next i/p value
    delta = newval - oldval; // get difference to last
		
    // ***** OUTSIDE +-3 allowable deviation
    if(delta > 3 || delta < -3) {	
      
      // we must establish an absolute value if OUTSIDE allowable deviation 
      
      // check and dump any prior run of a value
      if(RunCount != 0) 
        OutputRadl += sprintf(OutputRadl, "%d", RunCount);
      RunCount = 0;
      
      // check if prior delta was OK but not now
      if(!first) 								
        *OutputRadl++ = absolute[oldval]; // write absolute for prior value
      
      // write absolute for 'out of range' value
      *OutputRadl++ = absolute[newval]; 
      first = true;  // reset for a fresh delta pair
    }
    
    // ************ WITHIN +-3 allowable deviation
    else {			    
      // if not creating a delta pair, inc run count for a zero delta
      if(delta == 0 && first) 
        RunCount++;
      
      else {
        // check if delta pair started 
        if(first) { // NO
          first = false; // initially assume a new delta pair
          // check if an existing run count => this value is different
          if(RunCount != 0) {
            // if run count == 1 => prior delta was actually a zero -
	    // encode a delta pair
            if(RunCount == 1) { 
              *OutputRadl++ = Delta[delta & 0xf]; // encode 0*
              first = true; // dont start a new delta pair - we just wrote one!
            }
            else // Run count > 1 => dump run count
              OutputRadl += sprintf(OutputRadl, "%d", RunCount);
            
            RunCount = 0; // flush run count
          }
          if(!first)
            // NOTE must mask to prevent sign extension and -ve
	    // array indexing BAD!
            deltapair = (delta & 0xf) << 4; // create first part of delta pair
        }
        else { // delta pair started - finish it off
          deltapair |= (delta & 0x0f);
	  // lookup encoding value for this pair
          *OutputRadl++ = Delta[deltapair];
          first = true; // reset for new delta pair
        }
      }
    }
    oldval = newval;
  }
  if(!first)
    // pick up possible started delta pair 
    // and whack it onto the end of line as an absolute
    *OutputRadl++ = absolute[newval];
  if(RunCount != 0)   // whack possible runcount onto end of line
    OutputRadl += sprintf(OutputRadl, "%d", RunCount);
  
  *OutputRadl++ = 0;  // ensure final string is terminated!

}

void sRadl::SixLevelASCII(unsigned char *Radial,
			       char *OutputRadl, int length) 

{
  unsigned char newpair;
  int	RunCount = 0;

  // impossible value to achieve with 6 level data
  unsigned char lastpair = 0xff;

  while(length > 1) {  // note > 1 will work for odd length radials
    newpair = *Radial++ & 0x7;
    // build a six bit value for indexing into lookup table
    newpair |= (*Radial++ & 0x07) << 3;
    if(newpair > 63)
      return;
    if((newpair & 0x07) == 0x07) // ensure no level 7 exists in either bin
      return;
    if((newpair & 0x38) == 0x38)
      return;
    if(newpair == lastpair)
      RunCount++;
    else {
      if(RunCount) 
	OutputRadl += sprintf(OutputRadl, "%d", RunCount);
      RunCount = 0;
      *OutputRadl++ = SixLevelEncode[newpair];
    }
    lastpair = newpair;
    length -= 2;
  }
  if(RunCount) // pick up possible trailing runcount
    OutputRadl += sprintf(OutputRadl, "%d", RunCount);
  if(length == 1) { // odd length radial, pick up the final level
    newpair = *Radial++ & 0x7;
    *OutputRadl++ = SixLevelEncode[newpair];
  }
  *OutputRadl++ = 0;  // terminate end of string
}


/////////////////////////////////////////////////////////////////////////////
// binary radial encoding / decoding routines                               
// 
//  Since radar data invariably has lots of nulled bins, it makes more sense 
//  to compress these nulls in preference to attempting to detect runs of 
//  intensity levels.
//
//  The method used to achieve this Null Suppression is to include a run 
//  count after any occurence of a 0x00 value in the data stream. This is 
//  wasteful on isolated nulls but these are usually the exception rather 
//  than the norm. 
//  A single null is therefore encoded as:  0x00 0x01
//
//  The end of radial is marked by 0x00 0x00 which also serves as a
//  useful sync  marker if data corruption should occur.
//
//  A data section is implicit by writing data until a zero is encountered,
//  at which stage the zero is written then a run count is laid down.
//
//  The start of every radial holds a 16 bit count of how many bytes follow, 
//  including the count bytes, to describe the entire radial.
//
//
//  Examples:
//  ~~~~~~~~
//    A sample radial always starts as below: (spaces added for clarity)
//
//       @ XXX HI LO
//       |  |  |  |
//       |  |  |  '----------- LS byte for radial length
//       |  |  '-------------- MS byte for radial length
//       |  '----------------- radial's angle - ASCII character string
//       |                       (XXX for azimuth, XX.X for elevation)
//       '-------------------- start of binary radial character marker
//
//
//    A run count of 24 null bins would be represented as follows:
//
//    0x00 0x18 
//          |
//          '----------- count of 24 

int sRadl::EncodeBinaryRadl(unsigned char *Radial,
				unsigned char *OutputRadl, int length)

{

  int opsize = 0;
  unsigned char nullruncount = 0;
  unsigned char oneruncount = 0;
  unsigned char latestval = 0;
  unsigned char *pSizeLoc = OutputRadl; // remember start of output radial
  bool bAddVal = true;

  if (Radial == NULL || OutputRadl == NULL) 
    return 0; 
  
  OutputRadl += 2 * sizeof(unsigned char); // step over length marker bytes for now
  
  for(int count = 0; count < length; count++) {
    bAddVal = true; // by default add new bytes to output
    latestval = *Radial++; // get next i/p value

    if(latestval == 0x00) { // is it a null ?    
      if(oneruncount) { // have we been counting ones up to this byte ?
        *OutputRadl++ = oneruncount; // YES, write out the count value reached
        opsize++;                   
        oneruncount = 0; // reset run count - reached end of a null section
      }
      nullruncount++; // account for it
      // and make sure that we do write out the initial 0x00
      if(nullruncount == 1) {
        latestval = 0x00;          
      }
      // check that we have not exhausted our run count
      else if(nullruncount == 255) {
        latestval = nullruncount; // count will be added to output later
	// reset run count to which will also force next
	//input zero to be written to output
	nullruncount = 0;
      }  
      else
        bAddVal = false; // prevent writing out new value just yet
    }
    else if(latestval == 0x01) { // typical data code for spectral width moment
      if(nullruncount) { // have we been counting nulls up to this byte ?
        *OutputRadl++ = nullruncount; // YES, write out the count value reached
        opsize++;                   
        nullruncount = 0; // reset run count - reached end of a null section
      }
      oneruncount++; // account for it
      // and make sure that we do write out the initial 0x01
      if(oneruncount == 1) {
        latestval = 0x01;          
      }
      else if(oneruncount == 255) { // check that we have not exhausted our run count
        latestval = oneruncount; // count will be added to output later
	// reset run count to which will also force next
	// input zero to be written to output
        oneruncount = 0;
      }  
      else
        bAddVal = false; // prevent writing out new value just yet
    }
    else { // we don't have a null or a one
      if(nullruncount) { // have we been counting nulls up to this byte ?
        *OutputRadl++ = nullruncount; // YES, write out the count value reached
        opsize++;                   
        nullruncount = 0; // reset run count - reached end of a null section
      }
      if(oneruncount) { // have we been counting nulls up to this byte ?
        *OutputRadl++ = oneruncount; // YES, write out the count value reached
        opsize++;                   
        oneruncount = 0; // reset run count - reached end of a null section
      }
      // NOTE: we will write out the non-zero value later
    }

    if(bAddVal) {
      *OutputRadl++ = latestval; // write out the latest value 
      opsize++;                  // and bump the byte counter                 
    }
  }

  if(nullruncount) { // have we been counting nulls up to this byte ?
    latestval = nullruncount;       // remember last value written
    *OutputRadl++ = nullruncount;   // YES, write out the count value reached
    opsize++;                       // and bump the byte counter
    nullruncount = 0;               // reset run count
  }
  if(oneruncount) { // have we been counting nulls up to this byte ?
    latestval = oneruncount;       // remember last value written
    *OutputRadl++ = oneruncount;   // YES, write out the count value reached
    opsize++;                      // and bump the byte counter
    oneruncount = 0;               // reset run count
  }

  *OutputRadl++ = 0; // write line terminating nulls 
  opsize++;
  if(latestval) {
    *OutputRadl++ = 0; // only add second null if last data was not zero
    opsize++;
  }

  *pSizeLoc++ = (unsigned char)((opsize >> 8) & 0xff);
  *pSizeLoc++ = (unsigned char)(opsize & 0xff);

  return opsize + 2; // include size bytes for returned size
}

int sRadl::DecodeBinaryRadl(unsigned char* ipbuffer,
			     sRadl* radl)
  
{

  unsigned char ThisByte;
  unsigned char LastByte = 0;
  int bytestofollow = 0;
  bool firstpass = true;
  float Az, El;
  int   n;
  
  radl->data_size = 0; // init size
  radl->az = 0;
  if (sscanf((char*)ipbuffer,
	     "@%f,%f,%d=",&Az, &El, &n) == 3) {	// get radial angle
    radl->az = (short) (Az * 10);
    radl->el = (short) (El * 10);
  } 
  else 
    return -1;    // bad angle, fail convert
  ipbuffer += 17; // skip past @ and angle parts
  bytestofollow = *ipbuffer++;
  bytestofollow <<= 8;
  bytestofollow += *ipbuffer++;
  while (bytestofollow--) {
    ThisByte = *ipbuffer++; // read byte from buffer
    if(firstpass) {
      // we can't use LastByte until after the first pass through
      firstpass = false;
      radl->data[radl->data_size++] = ThisByte;
      LastByte = ThisByte;
    }
    else {

      if((ThisByte | LastByte) == 0) {
	// bump size down after we know we have got the end marker
        radl->data_size--;
        break;
      }
      // ThisByte is actually a run count if previous value was zero
      if(LastByte == 0x00) {
        LastByte = 0xff;          // prevent recursion into this routine
        while(--ThisByte) {       // generate as many zero's as required
          radl->data[radl->data_size++] = 0x00;
	  // safeguard against excessive length radials
          if(radl->data_size >= radl->buffsize)
            break;
        }
      }
      else if(LastByte == 0x01) {
        LastByte = 0xff;          // prevent recursion into this routine
        while(--ThisByte) {       // generate as many one's as required
          radl->data[radl->data_size++] = 0x01;
	  // safeguard against excessive length radialsa
          if(radl->data_size >= radl->buffsize)
            break;
        }
      }
      else { // non-zero prior value - this must be a data byte
        radl->data[radl->data_size++] = ThisByte;
        LastByte = ThisByte;
      }

    }
    // safeguard against excessive length radials
    if(radl->data_size >= radl->buffsize)
      break;
  }
  return 0;
}


///////////////////////////////////////////////////////////
// routine decodes a binary encoded radial
//
// returns: number of valid bins in opbuffer  (-1 => error)

int sRadl::DecodeBinaryRadl(unsigned char* opbuffer,
			    unsigned char *ipbuffer,
			    int maxsize, int &angle)
{
  unsigned char ThisByte;
  unsigned char LastByte = 0;
  int bytestofollow = 0;
  bool firstpass = true;
  float Az, El;
  int   n;
  int data_size;
  
  data_size = 0; // init size
  angle = 0;
  // get radial angle
  if (sscanf((char*)ipbuffer,"@%f,%f,%d=",&Az, &El, &n) == 3) {
    // angle = ftoi(Az);
    angle = (short) Az;
  } 
  else 
    return -1; // bad angle, fail convert
  ipbuffer += 17; // skip past @ and angle parts
  bytestofollow = *ipbuffer++;
  bytestofollow <<= 8;
  bytestofollow += *ipbuffer++;
  while (bytestofollow--) {
    ThisByte = *ipbuffer++; // read byte from buffer
    if(firstpass) {
      // we can't use LastByte until after the first pass through
      firstpass = false;
      opbuffer[data_size++] = ThisByte;
      LastByte = ThisByte;
    }
    else {

      if((ThisByte | LastByte) == 0) {
        data_size--; // bump size down after we know we have got the end marker
        break;
      }
      // ThisByte is actually a run count if previous value was zero
      if(LastByte == 0x00) {
        LastByte = 0xff;      // prevent recursion into this routine
        while(--ThisByte) {   // generate as many zero's as required
          opbuffer[data_size++] = 0x00;
	  // safeguard against excessive length radials
          if(data_size >= maxsize)
            break;
        }
      }
      else if(LastByte == 0x01) {
        LastByte = 0xff;       // prevent recursion into this routine
        while(--ThisByte) {    // generate as many one's as required
          opbuffer[data_size++] = 0x01;
	  // safeguard against excessive length radials
          if(data_size >= maxsize)
            break;
        }
      }
      else { // non-zero prior value - this must be a data byte
        opbuffer[data_size++] = ThisByte;
        LastByte = ThisByte;
      }
    }
    if(data_size >= maxsize) // safeguard against excessive length radials
      break;
  }
  return data_size;
}


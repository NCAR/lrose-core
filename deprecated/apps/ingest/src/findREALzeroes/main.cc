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
//
// Small program to fing the longest run of zeroes in a file.
// Needed for REAL data analysis. Users can then do something like :
//
// od -c -j 255530489 REAL_1_20090728_131419.lidar | & less
// 
// to see if there is indeed a long run of zeroes in the file.
//
// Niles Oien August 2009.
//

#include <cstdio>

int main(int argc, char *argv[]){

  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL){
    fprintf(stderr, "%s not found.\n", argv[1]);
    return -1;
  }

  unsigned long maxZeroLen = 0L;
  unsigned long zeroLen = 0L;
  unsigned long offset = 0L;
  unsigned long maxOffset = 0L;
  unsigned long startOffset = 0L;

  unsigned char b;

  while (1==fread(&b, sizeof(unsigned char), 1, fp)){
    offset++;

    if (b == 0){
      startOffset = offset - 1;
      zeroLen = 1;
      while (1==fread(&b, sizeof(unsigned char), 1, fp)){
	offset++;

	if (b == 0)
	  zeroLen++;
	else 
	  break;
      }
      if (zeroLen > maxZeroLen){
	maxZeroLen = zeroLen;
	maxOffset = startOffset;
      }
    }


  }

  fprintf(stdout, 
	  "Maximum run of zeros was %ld long starting at offset %ld\n", 
	  maxZeroLen, maxOffset);

  fclose(fp);


  return 0;

}

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
/////////////////////////////////////////////////////////////
//
// Filt2DMedian
//
// Two dimensional median filter for Interpolator program.
//
//

#include <stdlib.h> // For qsort
#include <cstdio>
#include <toolsa/umisc.h> 
#include <toolsa/mem.h>
#include <math.h>
using namespace std;

// file scope.
int FloatCompare(const void *a, const void *b);
                                                    
void Filt2DMedian(float *data, int nx, int ny, int WindowSize, int NumPasses,
		  float bad)
{

  // Allocate living room.

  int i,MaxPixels;
  float *w,*buff;

  // This is the most pixels we will consider at once.
  MaxPixels = 1 + 4*WindowSize + 4*WindowSize*WindowSize;
  
  w = (float *) umalloc(nx*ny*sizeof(float));
  buff = (float *) umalloc(MaxPixels*sizeof(float));

  if ((w==NULL) || (buff == NULL)){
    fprintf(stderr,"Malloc failed\n");
    exit(-1);
  }


  int Pass;
  for(Pass=0;Pass<NumPasses;Pass++){

    int ix,iy;

    for(ix=0; ix<nx; ix++){
      for(iy=0; iy<ny; iy++){

	int ixx,iyy,NumPixels;
	NumPixels=0;
	for(ixx=ix-WindowSize; ixx <= ix+WindowSize; ixx++){
	  for(iyy=iy-WindowSize; iyy <= iy+WindowSize; iyy++){

	    if ((ixx > -1) && (iyy > -1) &&
		(ixx < nx) && (iyy < ny)){

	      if (data[iyy*nx+ixx]!=bad){
		buff[NumPixels]=data[iyy*nx+ixx];
		NumPixels++;
	      } // if not bad
	    } // if inside
	  } // iyy
	} // ixx

	if (NumPixels==0){
	  w[iy*nx+ix]=bad;
	} else {
	  qsort(buff, NumPixels, sizeof(float),
		&FloatCompare);        
	  w[iy*nx+ix]=buff[int(floor(NumPixels/2))];
	}
      } // iy
    } // ix

    // Copy working array into data.
    for(i=0;i<nx*ny;i++){
      data[i]=w[i];
    }

  } // End of pass loop.

  ufree(w); ufree(buff);

}


int FloatCompare(const void *a, const void *b)
{

  // Cast pointers as correct type.

  float *aa, *bb;

  aa = (float *) a;
  bb = (float *) b;

  if (*aa < *bb) return -1;
  if (*bb < *aa) return  1;
  return 0;

}



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

#include <math.h>

int PearsonsGetMean(float *x,
		    float *y,
		    int Num,
		    float Bad,
		    float *meanX, 
		    float *meanY);

////////////////////////////////////////////////////////
//
// Main routine. Calculates Pearsons co-efficient r, scale and bias for
// arrays x and y. Both array are of size Num. A value of 'Bad' in
// either array means that the point is discarded (a good x[j] that
// is paired with a bad y[j] will be ignored, and vice-versa).
//
// Returns 0 if everything OK, -1 if there are not enough data.
// If -1 is returned, r, scale and bias are set to 0.
//

int Pearsons(float *x, 
	     float *y, 
	     int Num, 
	     float Bad,
	     float *r, 
	     float *scale, 
	     float *bias){

  //
  // Compute means. Return if there are no valid data.
  //

  float meanX, meanY;

  if (PearsonsGetMean(x,y,Num,Bad, &meanX, &meanY)){
    *r = 0.0; *scale = 0.0; *bias = 0.0;
    return -1;
  }

  //
  // Compute the necessary sums.
  //
  float sxy=0.0, sxx=0.0, syy=0.0;

  for (int i=0; i < Num; i++){
    if ((x[i]==Bad) || (y[i] == Bad)) continue;
    sxy = sxy + (x[i]-meanX)*(y[i]-meanY);
    sxx = sxx + (x[i]-meanX)*(x[i]-meanX);
    syy = syy + (y[i]-meanY)*(y[i]-meanY);
  }

  //
  // Compute r, scale and bias, if we can do so without error.
  //
  if (sxx*syy < 0.0000001){
    *r = 0.0; *scale = 0.0; *bias = 0.0;
    return -1;
  }

  *r = sxy/sqrt(sxx*syy);
  *scale = sqrt(syy/sxx);


  int num = 0;
  float Tot = 0.0;
  for(int i=0; i<Num; i++){
    if ((x[i]==Bad) || (y[i] == Bad)) continue;
    float predY = *scale * x[i];
    Tot = Tot + (y[i] - predY);
    num++;
  }
  *bias = Tot / (float)num;

  return 0;

}

/////////////////////////////////////
//
// Small routine to get the mean, discarding invalid
// data points. Returns 0 if OK, else -1 (no data).
//
int PearsonsGetMean(float *x,
		     float *y,
		     int Num,
		     float Bad,
		     float *meanX, 
		     float *meanY){

  float totX = 0.0, totY = 0.0;
  int num=0;
  
  for (int k=0; k<Num; k++){
    if ((x[k]==Bad) || (y[k] == Bad)) continue;
    totX = totX + x[k];
    totY = totY + y[k];
    num++;
  }
  
  if (num == 0){
    return -1;
  }

  *meanY = totY/(float)num;
  *meanX = totX/(float)num;

  return 0;

}



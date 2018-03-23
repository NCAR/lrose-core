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
#include <stdio.h>
#include <stdlib.h>
#include "../include/kd/fileoper.hh"

#include "../include/kd/datatype.hh"

// Allocates memory
KD_real **KD_read_input_file(char *filename, int *dimension, int *numpoints)
{
  int j,k;

  FILE *fp = fopen(filename,"r"); 

  if (fp == NULL)
    {
      *dimension = -1;
      return 0;
    }
  
  fscanf(fp, "%d\n",numpoints);  /* read number of points */
  fscanf(fp, "%d\n",dimension);  /* read dimension */

  KD_real **A = new KD_real*[*numpoints];

  for (k=0; k < *numpoints; k++)
    A[k] = new KD_real[*dimension];

  double val;
  for (k=0; k<*numpoints; k++)
    {
      for (j=0; j < *dimension; j++) {
	fscanf(fp, "%lg", &val);
	A[k][j] = val;
      }
    }

  fclose(fp);
  return(A);
}



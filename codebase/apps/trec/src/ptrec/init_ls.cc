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
/***************************************************************************
 * init_ls
 ****************************************************************************/

#include "trec.h"

void  init_ls(dimension_data_t *dim_data,
	      float ***ucones,
	      int iel,
	      float **npts,
	      float **sumx,
	      float **sumy,
	      float **sumx2,
	      float **sumy2,
	      float **sumxy,
	      float **sumu,
	      float **sumux,
	      float **sumuy,
	      short ***ioct,
	      float **meanu)

{

  int i,j,l;

  for(i=0; i< dim_data->nx; i++) {

      for(j=0; j < dim_data->ny; j++) {
	
	npts[i][j]=0.;
	sumx[i][j]=0.;
	sumy[i][j]=0.;
	sumx2[i][j]=0.;
	sumy2[i][j]=0.;
	sumxy[i][j]=0.;
	sumu[i][j]=0.;
	sumux[i][j]=0.;
	sumuy[i][j]=0.;
	meanu[i][j]=0.;
	ucones[i][j][iel] = BAD;

	for(l=0; l<4; l++) {
	  ioct[i][j][l]=0;
	}

      } /* j */

    } /*  i  */

}


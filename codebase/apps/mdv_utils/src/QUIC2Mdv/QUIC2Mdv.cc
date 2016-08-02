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
// Jiffy to Read QUIC PLUME output
// (c) UCAR 2006. All rights reserved.
// F. Hage. Oct. 2006.
//  NOTE: GRID Dimensions compiled in as defines.
//  
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "MdvFactory.hh"  // Our QUIC data to MDV converter class.


// From RONG 
#define QUIC_NUMX 300
#define QUIC_NUMY 300
#define QUIC_NUMZ 50
#define QUIC_NUMT 30
#define DELTA_T 30


int main(int argc, char **argv)

{
	FILE *fin;
	int numx = QUIC_NUMX; // From Rong
	int numy = QUIC_NUMY;
	int numz = QUIC_NUMZ;
	int numtimes = QUIC_NUMT;

	double *data; 

    int read_header(FILE *f);
    int read_3darray(double *array, FILE *f);
	
	if(argc != 2) {
		fprintf(stderr,"Usage: Jiffy QUIC_filename\n");
		exit(-1);
	}

	if((fin = fopen(argv[1],"r")) == NULL) {
			perror("Jiffy");
			exit(-1);
	}

    read_header(fin);

	if((data = (double *) calloc((QUIC_NUMX*QUIC_NUMY*QUIC_NUMZ),sizeof(double))) == NULL) {
			perror("Jiffy: calloc");
			exit(-1);
	}

	// Everything seems happy - Proceed to instantiate Mdv Factory Object.
	MdvFactory MdvF;
	MdvF.BuildHeaders();
	time_t now = time(0);

	for(int i = 0; i < numtimes; i++ ) {
	   fprintf(stderr," Reading Time %d: ",i);
	   read_3darray(data,fin);
	   fprintf(stderr," ... ");
	   MdvF.PutData(data,numy,numx,numz);
	   fprintf(stderr," ... ");
	   MdvF.WriteFile(now + (i * DELTA_T), "./");
	   fprintf(stderr,"Finished\n");
	}

	exit(0);

}

// Read an array, surrounded by fortran records.
int read_3darray(double *array, FILE * f) 
{
	int num_elems;
	int fort_reclen;
	
	// Read Fortran record length
	if((num_elems = fread(&fort_reclen,sizeof(int),1,f)) != 1 ) {
			perror("Jiffy fread");
			return(-1);
	}
#ifdef DEBUG
	fprintf(stderr,"1st Fortran Record Length %d\n",fort_reclen);
#endif

	// Read array
	if((num_elems = fread(array,1,fort_reclen,f)) != fort_reclen ) {
			perror("Jiffy fread");
			return(-2);
	}
	num_elems = fort_reclen / sizeof(double);
	
#ifdef DEBUG_FULL
	for(int i=0; i < num_elems; i++) {
		if(array[i] != 0.0) fprintf(stderr,"Index %d : %g\n",i,array[i]);
	}
#endif

	// Read Fortran record length
	if((num_elems = fread(&fort_reclen,sizeof(int),1,f)) != 1 ) {
			perror("Jiffy");
			return(-3);
	}
#ifdef DEBUG
	fprintf(stderr,"2nd Fortran Record Length %d\n",fort_reclen);
#endif
	return 0;
}

// Read in the header info
int read_header(FILE *fin)
{
	int num_elems;
	int fort_reclen;
	int numx = QUIC_NUMX; // From Rong
	int numy = QUIC_NUMY;
	int numz = QUIC_NUMZ;
	int numtimes = QUIC_NUMT;

	double xcoord[512];
	double ycoord[512];
	double zcoord[64];

	// Read Fortran record length
	if((num_elems = fread(&fort_reclen,sizeof(int),1,fin)) != 1 ) {
			perror("Jiffy");
			exit(-1);
	}
#ifdef DEBUG
	fprintf(stderr,"1st Fortran Record Length %d\n",fort_reclen);
#endif

	// Read X coord
	if((num_elems = fread(xcoord,sizeof(double),numx,fin)) != numx ) {
			perror("Jiffy");
			exit(-1);
	}


	// Read y coord
	if((num_elems = fread(ycoord,sizeof(double),numy,fin)) != numy ) {
			perror("Jiffy");
			exit(-1);
	}


	// Read X coord
	if((num_elems = fread(zcoord,sizeof(double),numz,fin)) != numz ) {
			perror("Jiffy");
			exit(-1);
	}
	// Read Fortran record length
	if((num_elems = fread(&fort_reclen,sizeof(int),1,fin)) != 1 ) {
			perror("Jiffy");
			exit(-1);
	}
#ifdef DEBUG
	fprintf(stderr,"2st Fortran Record Length %d\n",fort_reclen);
#endif

#ifdef DEBUG
	for(int i = 0; i < QUIC_NUMZ; i++) {
		fprintf(stderr,"Coord %d: %g %g %g\n",i,xcoord[i],ycoord[i],zcoord[i]);
	}
#endif

	return 0;

}

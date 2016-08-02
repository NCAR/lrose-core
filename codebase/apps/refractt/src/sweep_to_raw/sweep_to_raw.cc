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
// SUMMARY:      ingest refractivity variables from a dorade sweepfile
//		 corrects data for small but important biases introduced by VIRAQs
//		 outputs the result in a binary file
// AUTHORS:      Joe VanAndel and Frederic Fabry
// ORG:          National Center for Atmospheric Research;  McGill University
// E-MAIL:       vanandel@ucar.edu; frederic.fabry@mcgill.ca
//
// ORIG-DATE:     21-Jul-05 at 09:00:00
// LAST-MOD:      Jul-05 by Frederic Fabry
//
//
#if __GNUC__ >= 3
#include <iostream>
using namespace std;
#else
#include <iostream.h>
#endif

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#ifndef N_XTRACT_H
#include        <refractt/nrutil.h>    /* Include & defines for SQR */
#include        <refractt/n_xtract.h>    /* Include & defines for this program */
#endif

#include <refractt/dd_sweepfiles.hh>

#include <refractt/DataInput.hh>  // SPOL sweepfile utility

#define	    OffsetAboveAverage	0.2	/* Noise threshold set to .2*10 dB above average */
#define	    DBmNoiseMax	-106.

/*****************************************************************************/

int main ( int argc, char *argv[] )
{

    int		year, month, day, hour, minute, sec ;
    float	elev_angle ;
    time_t	time_cur_scan ;

    bool inSweep=false;
    int swpbeams = 0;
    float bin0range = 0. ;
    float binspacing = 150. ;
    float *az;
    float *el;
    long *unixTime;
    long *nanoSecs;
    float *niq,*aiq,*dm, *vr, *dbz;
    struct tm *scan_time ;
    char	tmp_str[80] ;
    int		i, j, k, azim, left_az, right_az, r, prev_r, next_r, offset ;
    float	target_azim,  max_range ;
    float	left_az_weight, right_az_weight, sum, tmp_float ;
    float	min_dist_l, min_dist_r, prev_r_weight, next_r_weight ;
    float	power, av_noise_niq, noise_i, noise_q ;
    float	*rawi, *rawq, *ranges ;
    int 	numRays,numGates, numGatesOut ;
    char        site_name[20], inputFile[200], file_out[200] ;
    FILE	*fp_tmp ;

/**** Part 0: Read command line parameters (Frederic Fabry's contribution) ****/

    if( (argc < 2) || (argc > 3) ) {
	printf("Usage: sweep_to_raw InputSweepFile [NumberGatesOut]\n") ;
	exit(99) ;
    }
    else {
	strcpy(inputFile, argv[1]) ;
	if( argc == 3 )
	     numGatesOut = atoi(argv[2]) ;
	else
	    numGatesOut = 0 ;
    }


/**** Part 1: Read data from file (Joe VanAndel's contribution) ****/    

    DFileInput dfi(inputFile);

    if (!dfi.OK) {
	printf( "Cannot read sweep file %s\n", inputFile ) ;
	exit( 1 ) ;
    }
    for (;;) {
        dd_mapper *map = dfi.get_beam();
        if (inSweep && (!map || map->new_sweep())) {
            inSweep = false;
            break;
        }
        if (!map) {
 
	    printf( "Cannot access sweepfile %s\n", inputFile ) ;
	    exit( 2 ) ;
        }
        // first ray processing
        if (!inSweep) {
            inSweep = true;
            numRays = map->rays_in_sweep();
	    assert(numRays > 0);
            numGates = map->number_of_cells();
            
	    bin0range = map->meters_to_first_cell() ;
	    binspacing = map->meters_between_cells() ;
            strncpy(site_name, map->radd->site_name,20);
            
	    max_range = (numGates - 1) * binspacing + bin0range ;
            az = new float[numRays];
            el = new float[numRays];
            unixTime = new long[numRays];
            nanoSecs = new long[numRays];
            niq = new float[numRays*numGates];
            aiq = new float[numRays*numGates];
	    rawi = new float[numRays*numGates];
	    rawq = new float[numRays*numGates];
            dm = new float[numRays*numGates];
            vr = new float[numRays*numGates];
            dbz = new float[numRays*numGates];
	    ranges = new float[numGates] ;
	    for( r = 0 ; r < numGates ; r++ )
		ranges[r] = bin0range + binspacing * r ;
        } // end first ray processing

        // retrieve scaled arrays of data for NIQ, AIQ, DM

        float bv;

        float *tPtr = &niq[swpbeams*numGates];
        if (!(map->return_field("NIQ", tPtr, &bv) ||
	      map->return_field("NIQ_V", tPtr, &bv)))
	{
	  printf("Cannot access 'NIQ' or 'NIQ_V' field\n");
	}
	

        tPtr = &aiq[swpbeams*numGates];
        if (!(map->return_field("AIQ", tPtr, &bv) ||
	      map->return_field("AIQ_V", tPtr, &bv)))
	{
	  printf("Cannot access 'AIQ' or 'AIQ_V' field\n");
	}

        tPtr = &dm[swpbeams*numGates];
        if (!(map->return_field("DM", tPtr, &bv)))
        {
	  printf("Cannot access 'dm' field\n");
	}
    

        tPtr = &vr[swpbeams*numGates];
        if (!(map->return_field("VR", tPtr, &bv) ||
	      map->return_field("VE", tPtr, &bv)))
	{
	  printf("Cannot access 'VR' or 'VE' field\n") ;
	}

        tPtr = &dbz[swpbeams*numGates];
        if (!(map->return_field("DBZ", tPtr, &bv) ||
	      map->return_field("DZ", tPtr, &bv)))
	{
	  printf("Cannot access 'DBZ' or 'DZ' field\n") ;
        }

        az[swpbeams] = map->azimuth();
        el[swpbeams] = map->elevation();

        unixTime[swpbeams] = map->unix_time();
        nanoSecs[swpbeams] = 1000*map->microsecond();

        swpbeams++;

    }

    elev_angle = el[numRays/2] ;

//   cout << "Processed " << numRays << endl;

//    for (r= 0; r < numRays; ++r) 
//    {
    //        cout << az[r] << endl;
    //    }


/**** Part 2: Data corrections (Frederic Fabry):
- Identify bad beams (PIRAQ initializations or buffer problems)
- Remove from NIQ/AIQ the DC contribution from PIRAQ/VIRAQ ****/

/* First, some preparation work: rescaling of NIQ */
	tmp_float = .025 ;  /* #### Working hypothesis: NIQ scaled in 1/4 dB */
	for( i = 0 ; i < numRays * numGates ; i++ )
	    niq[i] *= tmp_float ;

/* Testing of bad beams (code driven by IHOP's data collection experience) #### */
/* Flag bad beams by invalidating values and giving impossible number to azim
   so it can't be used in the remapping in part 3 */


/* Pre-computation of average NIQ of far away echoes (hopefully noise; watch for test pulse anyway) */
	for( i = 0, k = 0, av_noise_niq = 0. ; i < numRays * numGates ; i++ ) {
	    j = (i % numGates) ;
	    if( (j >= 9 * numGates / 10) && (niq[i] != INVALID) && (dm[i] < DBmNoiseMax) ) {
		av_noise_niq += pow( (double)10., (double)niq[i] ) ;
		k++ ;
	    }
	    if( niq[i] != INVALID ) {
		rawi[i] = pow( (double) 10.,  (double) niq[i] ) * cos(aiq[i]*DEGTORAD) ;
		rawq[i] = pow( (double) 10., (double) niq[i] ) * sin(aiq[i]*DEGTORAD) ;
	    }
	    else {
		rawi[i] = 0. ;
		rawq[i] = 0. ;
	    }
	}
	if (k > 1)
	    av_noise_niq = log10(av_noise_niq / (float)k) ;
	else
	    av_noise_niq = -VERY_LARGE ;

/* Get the best estimate on the average NIQ/AIQ vector introduced by PIRAQ */
	noise_i = 0. ;
	noise_q = 0. ;
	for( i = 0, k = 0 ; i < numRays * numGates ; i++ )
	    if( niq[i] < av_noise_niq + OffsetAboveAverage ) {
		noise_i += rawi[i] ;
		noise_q += rawq[i] ;
		k++ ;
	    }
	if( k > 1 ) {
	    noise_i /= (float)k ;
	    noise_q /= (float)k ;
	}

/* Subtract it from the NIQ/AIQ in vector form (rawi, rawq) */
	for( i = 0 ; i < numRays * numGates ; i++ ) {
	    if( niq[i] != INVALID ) {
		rawi[i] -= noise_i ;
		rawq[i] -= noise_q ;
	    }
	}


/**** Part 3: Output those variables ****/

/* Get scan time information */
	scan_time = gmtime( &unixTime[0] ) ;
	year = scan_time->tm_year + 1900 ;
 	month = scan_time->tm_mon + 1 ;
	day = scan_time->tm_mday ;
	hour = scan_time->tm_hour ;
	minute = scan_time->tm_min ;
	sec = scan_time->tm_sec ;
	time_cur_scan = unixTime[0] ;
	sprintf(file_out, "%s.dat", inputFile) ;

	if( (numGatesOut <= 0) || (numGatesOut > numGates) )
	    numGatesOut = numGates ;

/* Write the binary file */
	if ((fp_tmp = fopen(file_out, "wb")) != NULL ) {
	    fwrite( scan_time, sizeof(struct tm), 1, fp_tmp ) ;
	    fwrite( &numRays, sizeof(int), 1, fp_tmp ) ;
	    fwrite( &numGatesOut, sizeof(int), 1, fp_tmp ) ;
	    fwrite( &bin0range, sizeof(float), 1, fp_tmp ) ;
	    fwrite( &binspacing, sizeof(float), 1, fp_tmp ) ;
	    fwrite( &noise_i, sizeof(float), 1, fp_tmp ) ;
	    fwrite( &noise_q, sizeof(float), 1, fp_tmp ) ;
	    for( i = 0 ; i < numRays ; i++ ) {
		fwrite( &az[i], sizeof(float), 1, fp_tmp ) ;
		fwrite( &el[i], sizeof(float), 1, fp_tmp ) ;
		fwrite( &dbz[i*numGates], sizeof(float), numGatesOut, fp_tmp ) ;
		fwrite( &vr[i*numGates], sizeof(float), numGatesOut, fp_tmp ) ;
		fwrite( &dm[i*numGates], sizeof(float), numGatesOut, fp_tmp ) ;
		fwrite( &niq[i*numGates], sizeof(float), numGatesOut, fp_tmp ) ;
		fwrite( &aiq[i*numGates], sizeof(float), numGatesOut, fp_tmp ) ;
	    }
	    printf("%s written: Rays = %d; Gates = %d out of %d\n", file_out, numRays, numGatesOut, numGates) ;
	    fclose( fp_tmp ) ;
	}
	else
	    printf("Failed to open %s.\n", file_out) ;

/* Clean-up of allocated variables */

      delete [] az;
      delete [] el;
      delete [] unixTime;
      delete [] nanoSecs;
      delete [] niq;
      delete [] aiq;
      delete [] rawi ;
      delete [] rawq ;
      delete [] dm;
      delete [] vr;
      delete [] dbz;
      delete [] ranges;
      exit( 0 ) ;
}


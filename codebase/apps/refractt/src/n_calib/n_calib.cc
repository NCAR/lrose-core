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
/***************************** n_calib.c ***********************************/

/* This program identifies `reliable' ground targets (for the purpose of
   phase measurement), computes a ground echo reliability factor to 
   help identify good vs bad targets, and provides a calibration for the
   computation of N.  This information is required for n_xtract, the
   real-time/research program that derives N from ground targets, to work.
   See n_xtract.c for details about what this is all about.
   Frederic Fabry and Joe Vanandel, September 2005

Usage: n_calib paramaters_file
   It is assumed that some ground echo files readable by routines in
   n_input.c are within reach as they may be used either for computing
   ground echo reliability (how good a target they appear to be for our
   purposes) or for calibration (e.g. what are the normal target phase
   expected for a known meteorological situation).

   Code that should be changed for uses in systems other than the
   McGill Radar is indicated by the string @@@@ , and has been isolated
   in subroutines for the most part to make the task easier for you.

BUGS, NOTES, AND POSSIBLE CHANGES:
- The program has, at this point, only limited ways of recognizing and
  rejecting unwanted ground echoes coming via sidelobes or from multi-body
  scattering.  They look as solid as the real thing, even if they are of
  course weaker.

------------------------------------------------------------------------------
*/

#define		N_CALIB

/* Declaration and includes */
#include	<refractt/nrutil.h>		/* Include & defines for Numerical Recipes */
#include	<refractt/n_xtract.h>		/* Include & defines shared with n_xtract */
#include        <refractt/n_input.h>

#ifdef DUMP_NC    /* conditionally dump netCDF versions of structures */
#include "n_util.h"
int Dump_nc = 0;
#define NameDifFromRef "diff_from_ref.nc"
#endif

/* (Not really) Large arrays and global variables */
char		fname_in[NUMSCANS][PATH_MAX] ;
float		*sum_a, *sum_b, *sum_p, *fluct_snr, *mean_snr, *old_snr ;
float		elev_angle ;
int		*pixel_count ;
struct T_data   *raw_phase ;
struct R_info   *calib_ref ;
struct T_data   *dif_from_ref ;		/* [NumAzim*NumRangeBins] ; */ 
struct T_data   *dif_prev_scan ;	/* [NumAzim*NumRangeBins] ; */
struct stat	latest_data ;
int		az_cor, bridge_detected ;	/* @@@@ */
int		year, month, day, hour, minute, sec ;
int		gnd_filter ;
FILE            *fp ;

/* Parameter file entries */
char		NameOfRun[PATH_MAX], Author[PATH_MAX], ProjectName[PATH_MAX] ;
char		DataVersion[PATH_MAX], SubVersion[PATH_MAX], DebugOutputFile[PATH_MAX] ;
char		RTDataFileName[PATH_MAX], ResListFile[PATH_MAX], ResFirstFile[PATH_MAX], ResLastFile[PATH_MAX] ;
char		RefFileName[PATH_MAX], FontName[PATH_MAX], DestinationPath[PATH_MAX], StatFileName[PATH_MAX] ;
char		LatestIQ[PATH_MAX], LatestNPolar[PATH_MAX], LatestNCart[PATH_MAX], LatestDNPolar[PATH_MAX] ;
char		LatestDNCart[PATH_MAX], McGillOutputN[PATH_MAX], TmpFile[PATH_MAX] ;
char		GeogOverlay[PATH_MAX] ;
int		RealTimeMode, UseCircularQ, AdaptDisplay, DoCartesianN, DoMapDiff ;
int		SwitchEndianIn, SwitchEndianOut, DoRelax, DebugLevel ;
int		NumAzim, NumRangeBins, IQPerAngle, RMin ;
int		NOutNumAzim, NOutNumRanges, NumMapDiff ;
int		DNOutNumAzim, DNOutNumRanges, CartesianX, CartesianY ;
int		DurationHistory, NumColors ;
int		DataVersionID ;
float		Longitude, Latitude, AvTargetHeight, FirstRange ;
float		GateSpacing, Frequency, PRF, AzimFluct, BeamWidth ;
float		NSmoothingSideLen, NErrorThresh, DNSmoothingSideLen, DNErrorThresh ;
float		MinConsistency, CartesianResol, MinNTrace, MaxNTrace ;
float		MinNDisplay, MaxNDisplay, SideLobePow ;

/* Empty shells needed to compile unused routines in n_xtract.c  #### Should be fixed */
struct T_info	*target ;
int		program_mode ;
time_t		time_cur_scan ;
char		first_f[PATH_MAX], last_f[PATH_MAX] ;
float		new_av_n, ap_indx ;


/**************************************************************************/

int main(int argc, char* argv[])
{
	int		j, k ;
	int		mask_exist, iopt, file_count ;
	char		search_path[PATH_MAX] ;
	char		first_file[PATH_MAX], last_file[PATH_MAX] ;


	mask_exist = startup(argc, argv) ;	/* Some set-up */

#ifdef DUMP_NC
	if (getenv("DUMP_NC")) {
	    Dump_nc = 1;
	}
#endif

/*	debug_out( 0, -1, &(NameOfRun[0]), &(last_f[PATH_MAX]), "New debug_out attempt of global variables\n" ) ; */
	do {
	    iopt = get_menu_entry( mask_exist ) ;  /* Ask what to do */

	    switch (iopt) {
		case 0:				/* Quit program */
		    /* Close things */
		    printf( "\nI hope things will go well.  Bye!\n\n" ) ;
		    exit(0) ;
		    break ;

		case 1:				/* Cookbook instructions */
		    break ;

		case 2:				/* Background information */
		    break ;

		case 3:				/* Build reliability info */
		    j = get_file_set( search_path, first_file, last_file ) ;
		    if( j == 0 ) {
		        file_count = build_file_list( search_path, first_file, last_file, fname_in[0], NUMSCANS, PATH_MAX ) ;
			add_search_path( file_count, search_path, PATH_MAX ) ;
		    }
		    else
			file_count = read_list( first_file, fname_in[0] ) ;
		    if( file_count < 3 ) {
			printf("At least 3 scans are required to get some kind of echo reliability\n") ;
			exit(2) ;
		    }
		    iopt = confirm_do_reliability( mask_exist, file_count ) ;
		    if ( iopt == 1 )  {
			find_reliable_targets( file_count ) ;
			mask_exist = TRUE ;
		    }
		    break;

		case 4:				/* Do refractivity calibration */
		    j = get_file_set( search_path, first_file, last_file ) ;
		    if( j == 0 ) {
		        file_count = build_file_list( search_path, first_file, last_file, fname_in[0], NUMSCANS, PATH_MAX ) ;
			add_search_path( file_count, search_path, PATH_MAX ) ;
		    }
		    else
			file_count = read_list( first_file, fname_in[0] ) ;
		    iopt = confirm_do_calibration( file_count ) ;
		    if ( iopt == 1 )
			calib_targets( file_count ) ;
		    break ;

		case 5:				/* Adjust calibration for propagation */
/*		    j = get_file_set( search_path, first_file, last_file ) ;
 		    if( j == 0 ) {
			file_count = build_file_list( search_path, first_file, last_file, fname_in[0], NUMSCANS, PATH_MAX ) ;
			add_search_path( file_count, search_path, PATH_MAX ) ;
		    }
		    else
			file_count = read_list( first_file, fname_in[0] ) ;
		    if( file_count < 6 ) {
			printf("At least 6 scans are required to get any kind of propagation dependence information (and even then, you won't do much).\n") ;
			exit(2) ;
		    }
		    iopt = confirm_do_adjustment( file_count ) ;
		    if ( iopt == 1 )
			adjust_targets( file_count ) ;
*/		    break;

	    }
	} while (1) ;
}


/**************************************************************************/

/* startup():  Initializes n_calib.
   - Prints start-up message (well... it _is_ an important task!);
   - Allocates and initializes all the needed global variables;
   - Checks for the presence of an existing calibration file.
   Input: None.
   Output: Allocated calib_ref array and status of presence of mask.
   Called by: main() */

int startup(int argc, char* argv[])
{

/* Start up */
	printf( "N_CALIB %s:  Determines and calibrates valid ground echoes used to get N.\n", N_VERSION ) ;
	printf( "Frederic Fabry sends greetings (%s)\n\n", MONTH_YEAR ) ;
	printf( "Note: it is best to stop the real-time task of extracting N if it is running.\n\n" ) ;

/* Read parameters file */
	if( argc == 2 )
	    get_params( argv[1] ) ;
	else {
	    printf("Usage: n_calib parameter_file\n") ;
	    printf("\twhere \"parameter_file\" contains all the data processing parameters used by n_calib and n_xtract.\n\n") ;
	    exit(1) ;
	}

/* Create main data array */
        calib_ref = (struct R_info *)  calloc( NumAzim * NumRangeBins, sizeof(struct R_info ) ) ;
        raw_phase = (struct T_data *)  calloc( NumAzim * NumRangeBins, sizeof(struct T_data ) ) ;
        dif_from_ref = (struct T_data *)  calloc( NumAzim * NumRangeBins, sizeof(struct T_data ) ) ;
        dif_prev_scan = (struct T_data *)  calloc( NumAzim * NumRangeBins, sizeof(struct T_data ) ) ;
        if( dif_prev_scan == NULL ) {
            printf("Out of memory allocating calib_ref and (I,Q) arrays.  Quitting.\n") ;
            exit( 3 ) ;
        }
	zed = (float *) malloc( NumRangeBins * NumAzim * sizeof( float ) ) ;
	vel = (float *) malloc( NumRangeBins * NumAzim * sizeof( float ) ) ;
	sw = (float *) malloc( NumRangeBins * NumAzim * sizeof( float ) ) ;
	snr = (float *) malloc( NumRangeBins * NumAzim * sizeof( float ) ) ;
	old_snr = (float *) malloc( NumRangeBins * NumAzim * sizeof( float ) ) ;
	mean_snr = (float *) malloc( NumRangeBins * NumAzim * sizeof( float ) ) ;
	fluct_snr = (float *) malloc( NumRangeBins * NumAzim * sizeof( float ) ) ;
	if( fluct_snr == NULL ) {
	    printf( "Out of memory allocating arrays vel/sw/snr.  Quitting.\n" ) ;
	    exit( 3 ) ;
	}

	pixel_count = (int *) malloc( NumRangeBins * NumAzim * sizeof( int ) ) ;
	sum_a = (float *) malloc( NumRangeBins * NumAzim * sizeof( float ) ) ;
	sum_b = (float *) malloc( NumRangeBins * NumAzim * sizeof( float ) ) ;
	sum_p = (float *) malloc( NumRangeBins * NumAzim * sizeof( float ) ) ;
	if( sum_p == NULL ) {
	    printf( "Out of memory allocating arrays sum_a/sum_b/sum_p.  Quitting.\n" ) ;
	    exit( 3 ) ;
	}

	memset( raw_phase, 0, sizeof(struct T_data) * NumRangeBins * NumAzim ) ;

/* Check for the presence of an existing calibration file */
        if ((fp = fopen( RefFileName, "rb" )) != NULL) {
            fclose( fp ) ;
	    return( TRUE ) ;
        }
	else
	    return(FALSE) ;
}


/*****************************************************************************/

/* get_menu_entry( mask_exist ):  Asks if reliability determination or
   calibration is wanted.
   Input: Presence of an existing calibration file.
   Output: Menu entry requested.
   Called by: main() */

int get_menu_entry( int mask_exist )
{
	int		iopt ;

	iopt = 0 ;
	printf("\nMenu options:\n") ;
	printf("\t0 - Quit\n") ;
	printf("\t1 - What should I do?  (Cookbook help)\n") ;
	printf("\t2 - What is this all about?  (Background information)\n") ;
	printf("\t3 - Compute ground echo reliability information\n") ;
	if( mask_exist ) {
	    printf("\t4 - Perform refractivity calibration of ground clutter info\n") ;
	    printf("\t5 - Adjust calibration for changes in propagation  (Not implemented)\n") ;
	}

	do {
	    printf( "\nYour option: ") ;
	    scanf( "%d", &iopt ) ;
	} while (( iopt < 0 ) || ( iopt > 3 - 2 * mask_exist )) ;

	return( iopt ) ;
}


/*****************************************************************************/

/* get_file_set( search_path, first_file, last_file ):  Gets the directory
   path of the data files to be used as well as the name of the first and
   last file to be used for calibration.
   Input: None.
   Output: First & last file to be used.
   Called by: main() */

int get_file_set( char *search_path, char *first_file, char *last_file )
{
	int	opt, i ;

	printf("\nEnter filename (and path) of file list, or - to\nenter first and last file names: ") ;
	scanf("%s", first_file ) ;
	if( first_file[0] != '-' )
	    opt = 1 ;
	else {
	    printf("\nEnter directory path of input files ('./' for current): ") ;
	    scanf( "%s", search_path ) ;
	    i = strlen(search_path) ;
	    if( search_path[i-1] != '/' ) {
		search_path[i] = '/' ;
		search_path[i+1] = 0 ;
	    }
	    printf("Enter name of first calibration file: ") ;
	    scanf( "%s", first_file ) ;
	    printf("Enter name of last calibration file: ") ;
	    scanf( "%s", last_file ) ;
	    opt = 0 ;
	}
	return( opt ) ;
}


/*****************************************************************************/

/* add_search_path( file_count, search_path, PATH_MAX ):
   Just adds the search path to the file list (nothing glamorous).
   Called by: main() */

void add_search_path( int file_count, char *search_path, int size_filename )
{
	int		j, k ;

	if( file_count == 0 ) {
	    printf("No file found matching criteria\n") ;
	    exit(1) ;
	}

	for( j = 0 ; j < file_count ; j++ ) {
	    for( k = strlen(fname_in[j]) ; k >= 0 ; k-- )
		fname_in[j][k+strlen(search_path)] = fname_in[j][k] ;
	    for( k = 0 ; k < strlen(search_path) ; k++ )
		fname_in[j][k] = search_path[k] ;
	}
}


/*****************************************************************************/

/* confirm_do_reliability( mask_exist, file_count ) :
   Ask user for confirmation concerning the computation of ground echo
   reliability.
   Input: Info necessary to print question.
   Output: Confirmation (or not) of action.
   Called by: main() */

int confirm_do_reliability( int mask_exist, int file_count )
{
	int		iopt, j ;

	if( mask_exist ) {
	    printf( "\n*************************** WARNING ****************************\n") ;
	    printf( "\nYou have chosen: 3 - Compute ground echo reliability information\n\n") ;
	    printf( "However, there is a file called " ) ;
	    printf( RefFileName ) ;
	    printf( "\nin this directory, which I suppose contains some calibration information.\n") ;
	    printf( "This process will destroy that file and the current calibration it contains.\n" ) ;
	    printf( "*************************** WARNING ****************************\n\nAlso: " ) ;
	}

	printf( "The following %d data files will be used to compute echo reliability:\n", file_count ) ;
	for ( j = 0 ; j < file_count ; j++ )
	    if( (j < 8) || (file_count <= 10) )
		printf("\t%s\n", fname_in[j] ) ;
	    else if( j == 8 )
		printf("\t...\n\t%s\n", fname_in[file_count-1] ) ;

	printf( "\nMake sure they are all at the same elevation angle!\n") ;
	printf( "\nShall we:\n" ) ;
	printf( "\t0 - Abort for now\n" ) ;
	printf( "\t1 - Proceed with the computation of target reliability\n" ) ;
	do {
	    iopt = 0 ;
	    printf( "\nYour option: ") ;
	    scanf( "%d", &iopt ) ;
	} while (( iopt < 0 ) || ( iopt > 1 )) ;

	return( iopt ) ;
}


/*****************************************************************************/

/* find_reliable_targets( file_count ):  Go through a list of files containing
   ground echo phase data and use them to determine their suitability (or their
   reliability) for the computation of N.  Specifically, this involves
   determining whether the phase of targets are stable (or at least
   are consistent) in time or if they fluctuate (and if yes, by how much).
   Good targets are those whose phase is relatively constant with time;
   others are either precipitation/clear air/insect clutter, or they are
   from ground targets that move (e.g. vegetation, lakes).
   Input: Set of files to process.
   Output: Updates RefFileName.
   Called by: main() */

void find_reliable_targets( int file_count )
{
	int		j, k, file_num, r, az, offset, dr, daz, meas_az ;
	float		tmp_a, tmp_b, n_value, ncp, norm ;
	float		*av_i, *av_q, *phase_targ ;
	float		local_pow, near_pow, contamin_pow, sum_pow, side_pow ;
	double		smooth_i, smooth_q ;
	float		strength_correction, side_correction ;
	FILE            *fp ;
	char		c ;

/* Initialize some variables */
	memset( fluct_snr, 0, sizeof(float) * NumAzim * NumRangeBins ) ;
	memset( mean_snr, 0, sizeof(float) * NumAzim * NumRangeBins ) ;
	memset( pixel_count, 0, sizeof(int) * NumAzim * NumRangeBins ) ;
	memset( sum_a, 0, sizeof(float) * NumAzim * NumRangeBins ) ;
	memset( sum_b, 0, sizeof(float) * NumAzim * NumRangeBins ) ;
	memset( sum_p, 0, sizeof(float) * NumAzim * NumRangeBins ) ;
	memset( dif_from_ref, 0, sizeof(struct T_data) * NumAzim * NumRangeBins ) ;
	av_i = (float *) malloc( NumRangeBins * NumAzim * sizeof( float ) ) ;
	av_q = (float *) malloc( NumRangeBins * NumAzim *sizeof( float ) ) ;
	phase_targ = (float *) malloc( NumRangeBins * NumAzim * sizeof( float ) ) ;
	if( phase_targ == NULL ) {
	    printf( "Out of memory allocating array phase_targ.  Quitting.\n" ) ;
	    exit( 3 ) ;
	}

/* Read the first file; raw_phase */
	printf("\nGoing through all the files:\n   1 - %s   \r", fname_in[0]);
	fflush(stdout) ;
	stat( fname_in[0], &latest_data ) ;
	switch (DataVersionID) {
	    case MCG_PRE_VIRAQ:
		j = read_data_previraq( fname_in[0] ) ;
		break ;
	    case MCG_VIRAQ:
		j = read_data_viraq( fname_in[0] ) ;
		break ;
	    case MCG_DUALP:
		j = read_data_dualp( fname_in[0] ) ;
		break ;
	    case SPOL_VIRAQ:
		j = read_data_spol( fname_in[0] ) ;
		break ;
	    case SPOL_RVP8:
		j = read_data_spol_rvp8( fname_in[0] ) ;
		break ;
	}
	if( j == FALSE ) {
	    printf("\nAt least one of the scans,\n") ;
	    printf("%s\n", fname_in[file_num] ) ;
	    printf("appear to be bad (few targets).\n" ) ;
	    printf("Please make sure that the files to be used are all good.  Quitting.\n") ;
	    exit(4) ;
	}

	for( offset = 0 ; offset < NumAzim * NumRangeBins ; offset++ ) {
	    av_i[offset] = raw_phase[offset].inphase;
	    av_q[offset] = raw_phase[offset].quadrature;
	}

/* Then proceed more slowly with the next ones: */
	for( file_num = 1 ; file_num < file_count ; file_num++ ) {
	    printf("   %d - %s   \r", file_num+1, fname_in[file_num]) ;
	    fflush(stdout) ;

/* - Transfer old I/Q data temporarily in dif_prev_scan */
	    memcpy( dif_prev_scan, raw_phase, NumAzim*NumRangeBins*sizeof(struct T_data) ) ;
	    memcpy( old_snr, snr, NumAzim*NumRangeBins*sizeof(float) ) ;

/* - Read I/Q, vel, sw and snr arrays of latest file */
	    stat( fname_in[file_num], &latest_data ) ;
	    switch (DataVersionID) {
		case MCG_PRE_VIRAQ:
		    j = read_data_previraq( fname_in[file_num] ) ;
		    break ;
		case MCG_VIRAQ:
		    j = read_data_viraq( fname_in[file_num] ) ;
		    break ;
		case MCG_DUALP:
		    j = read_data_dualp( fname_in[file_num] ) ;
		    break ;
		case SPOL_VIRAQ:
		    j = read_data_spol( fname_in[file_num] ) ;
		    break ;
		case SPOL_RVP8:
		    j = read_data_spol_rvp8( fname_in[file_num] ) ;
		    break ;
	    }
	    if( j == FALSE ) {
		printf("\nAt least one of the scans,\n") ;
		printf("%s\n", fname_in[file_num] ) ;
		printf("appear to be bad (few targets).\n" ) ;
		printf("Please make sure that the files to be used are all good.  Quitting.\n") ;
		exit(4) ;
	    }

/* - Compute phase difference wrt previous scan (now in dif_prev_scan_i/q).
     Normalize I & Q, with some snr-based weighting.
     (Somewhat similar to beginning of get_quality()) */
	    for( az = 0, offset = 0 ; az < NumAzim ; az++ )
		for( r = 0 ; r < NumRangeBins ; r++, offset++ ) {
		    av_i[offset] += raw_phase[offset].inphase;
		    av_q[offset] += raw_phase[offset].quadrature;
		    tmp_a = dif_prev_scan[offset].inphase ;
		    tmp_b = dif_prev_scan[offset].quadrature ;
		    dif_prev_scan[offset].inphase = tmp_a * raw_phase[offset].inphase + tmp_b * raw_phase[offset].quadrature ;
		    dif_prev_scan[offset].quadrature = tmp_a * raw_phase[offset].quadrature - tmp_b * raw_phase[offset].inphase ;
		    norm = sqrt( SQR(dif_prev_scan[offset].inphase) + SQR(dif_prev_scan[offset].quadrature) ) ;
		    if( snr[offset] != INVALID ) {
			strength_correction = 1. / (1. + pow(10., -.1*snr[offset])) ;
			if( strength_correction != 0. )
			{
			  if (strength_correction > 0.001)
			    norm /= strength_correction ;
			  else
			    norm = VERY_LARGE;
			}
			
		    }
		    if( norm != 0. ) {
			dif_prev_scan[offset].inphase /= norm ;
			dif_prev_scan[offset].quadrature /= norm ;
		    }
		}
#ifdef DUMP_NC
        if (Dump_nc){
            char fname[80];
            sprintf(fname,"diff_prev_scan%d.nc", file_num);

            dump_T_data(dif_prev_scan, "difference from previous",
                        fname);
        }
#endif

/* - Smooth (a lot!) dif_prev_scan in dif_from_ref.  As a result,
     dif_from_ref will map changes of phase over large areas (due to
     meteorology) which could then be compensated for later.  First do it
     at close range (average across all azimuths and a few range gates). */
	    for( r = RMin ; r <= RMin + NumRangeBins / 10 ; r++ ) {
		tmp_a = 0. ;
		tmp_b = 0. ;
		for ( az = 0, offset = r ; az < NumAzim ; az++, offset += NumRangeBins ) {
		    tmp_a += dif_prev_scan[offset].inphase ;
		    tmp_b += dif_prev_scan[offset].quadrature ;
		}
		dif_from_ref[r].inphase = tmp_a ;
		dif_from_ref[r].quadrature = tmp_b ;
	    }
	    for( r = RMin ; r < RMin + NumRangeBins / 10 ; r++ ) {
		tmp_a = dif_from_ref[r-1].inphase + dif_from_ref[r].inphase + dif_from_ref[r+1].inphase ;
		tmp_b = dif_from_ref[r-1].quadrature + dif_from_ref[r].quadrature + dif_from_ref[r+1].quadrature ;
		norm = sqrt( SQR(tmp_a) + SQR(tmp_b) ) ;
		if( norm != 0.) {
		    tmp_a /= norm ;
		    tmp_b /= norm ;
		    }
		for( az = 1, offset = NumRangeBins+r ; az < NumAzim ; az++, offset += NumRangeBins ) {
		    dif_from_ref[offset].inphase = tmp_a ;
		    dif_from_ref[offset].quadrature = tmp_b ;
		}
	    }
	    for( r = RMin ; r < RMin + NumRangeBins / 10 ; r++ ) {
		dif_from_ref[r].inphase = dif_from_ref[r+NumRangeBins].inphase ;
		dif_from_ref[r].quadrature = dif_from_ref[r+NumRangeBins].quadrature ;
	    }

/*   And then to ranges further away (sector average). */
	    for( r = RMin + NumRangeBins / 10 ; r < NumRangeBins ; r++ ) {
		smooth_i = 0. ;
		smooth_q = 0. ;
		for( dr = -NumRangeBins/10 ; dr <= NumRangeBins/10 ; dr++ ) {
		    if( r+dr < NumRangeBins )
			for( daz = -NumAzim/16 ; daz <= NumAzim/16 ; daz++ ) {
			    meas_az = daz ;
			    if( meas_az < 0 )  meas_az += NumAzim ;
			    if( meas_az >= NumAzim )  meas_az -= NumAzim ;
			    offset = meas_az * NumRangeBins + r + dr ;
			    smooth_i += dif_prev_scan[offset].inphase ;
			    smooth_q += dif_prev_scan[offset].quadrature ;
			}
		}
		dif_from_ref[r].inphase = smooth_i ;
		dif_from_ref[r].quadrature = smooth_q ;

		for( az = 1 ; az < NumAzim ; az++ ) {
		    for( dr = -NumRangeBins/10 ; dr <= NumRangeBins/10 ; dr++ )
			if( r+dr < NumRangeBins ) {
			    meas_az = az - 1 - NumAzim/16 ;
			    if( meas_az < 0 )  meas_az += NumAzim ;
			    offset = meas_az * NumRangeBins + r + dr ;
			    smooth_i -= dif_prev_scan[offset].inphase ;
			    smooth_q -= dif_prev_scan[offset].quadrature ;

			    meas_az = az + NumAzim/16 ;
			    if( meas_az >= NumAzim )  meas_az -= NumAzim ;
			    offset = meas_az * NumRangeBins + r + dr ;
			    smooth_i += dif_prev_scan[offset].inphase ;
			    smooth_q += dif_prev_scan[offset].quadrature ;
			}
		    offset = az * NumRangeBins + r ;
		    dif_from_ref[offset].inphase = smooth_i ;
		    dif_from_ref[offset].quadrature = smooth_q ;
		}

		for( az = 0 ; az < NumAzim ; az++ ) {
		    offset = az * NumRangeBins + r ;
		    norm = sqrt( SQR(dif_from_ref[offset].inphase) + SQR(dif_from_ref[offset].quadrature) ) ;
		    if( norm != 0 ) {
			dif_from_ref[offset].inphase /= norm ;
			dif_from_ref[offset].quadrature /= norm ;
		    }
		}
	    }
#ifdef DUMP_NC
        if (Dump_nc){
            dump_T_data(dif_from_ref, "difference from reference",
                               NameDifFromRef);
        }
#endif

/* - Smooth phase field done.  Now compensate the previously computed
     phase difference field for the average change caused by meteorology
     (i.e. the smoothed phase field). */

	    for( az = 0, offset = 0 ; az < NumAzim ; az++ )
		for( r = 0 ; r < NumRangeBins ; r++, offset++ ) {
		    tmp_a = dif_prev_scan[offset].inphase ;
		    tmp_b = dif_prev_scan[offset].quadrature ;
                    dif_prev_scan[offset].inphase = tmp_a * dif_from_ref[offset].inphase + tmp_b * dif_from_ref[offset].quadrature ;
                    dif_prev_scan[offset].quadrature = tmp_b * dif_from_ref[offset].inphase - tmp_a * dif_from_ref[offset].quadrature ;
		}

/* - Sum up the dif_prev_scan_i/q in sum_a/b and sum_p (to be used later to
     compute, in conjunction with sum_a/b the actual reliability). */
	    for( az = 0, offset = 0 ; az < NumAzim ; az++ )
		for( r = 0 ; r < NumRangeBins ; r++, offset++ ) {
		    sum_a[offset] += dif_prev_scan[offset].inphase ;
		    sum_b[offset] += dif_prev_scan[offset].quadrature ;
		    sum_p[offset] += SQR(dif_prev_scan[offset].inphase) + SQR(dif_prev_scan[offset].quadrature) ;
		    if( (snr[offset] != INVALID) && (old_snr[offset] != INVALID) ) {
			fluct_snr[offset] += fabs(snr[offset] - old_snr[offset]) ;
			mean_snr[offset] += snr[offset] ;
			pixel_count[offset]++ ;
		    }
		}
	}  /* For all files */

/* Do the actual computation of reliability based on how "coherently" the
   dif_prev_scan were summed up.  The calculation is very much like
   that of normalized lag 1 power (NCP) in pulse-pair computation. */
	printf("Now completing the calculation of reliability...\n") ;
	for( az = 0, offset = 0 ; az < NumAzim ; az++ )
	    for( r = 0 ; r < NumRangeBins ; r++, offset++ ) {
		calib_ref[offset].av_i = 0. ;
		calib_ref[offset].av_q = 0. ;
		calib_ref[offset].dr = 0. ;
		calib_ref[offset].dNdz_sensitivity = 0. ;
		calib_ref[offset].phase_er = VERY_LARGE ;
		if( (av_i[offset] == 0.) && (av_q[offset] == 0.) )
		    phase_targ[offset] = INVALID ;
		else
		    phase_targ[offset] = atan2( av_q[offset], av_i[offset] ) / DEGTORAD ;
		if( (sum_p[offset] == 0.) || (pixel_count[offset] == 0) ) {
		    calib_ref[offset].strength = INVALID ;
		}
		else {
		    calib_ref[offset].strength = mean_snr[offset] / (float)pixel_count[offset] ;
		    ncp = ( SQR(sum_a[offset]) + SQR(sum_b[offset]) ) / sum_p[offset] / pixel_count[offset] ;
		    if( (ncp != 0.) && ( r >= RMin ) ) {
			if( pixel_count[offset] > 1 ) {
			    ncp = (ncp - 1. / sqrt((float)pixel_count[offset])) / (1. - 1. / sqrt((float)pixel_count[offset])) ;
			    if( ncp < .001 )  ncp = .001 ;
			    if( ncp > .9999 )  ncp = .9999 ;
			}
			else
			    ncp = .5 ;

/* Add to it a correction for ground echo SNR and its variability */
			ncp *= 1. / ( 1. + pow(10., -.1*(calib_ref[offset].strength)) ) ;
			fluct_snr[offset] /= (float)pixel_count[offset] ;
			strength_correction = exp(-.001 * pow((double) fluct_snr[offset], (double) 4.)) / exp(-.002) ;
			if( strength_correction > 1. )  strength_correction = 1. ;
			if( strength_correction < .1 )  strength_correction = .1 ;
			ncp *= strength_correction ;
			if( ncp > 0. )
		            calib_ref[offset].phase_er = sqrt(-2. * log( ncp ) / ncp) / DEGTORAD ;
			else {
			    ncp = 0. ;
			    calib_ref[offset].phase_er = VERY_LARGE ;
			}
			calib_ref[offset].av_i = ncp ;
			calib_ref[offset].av_q = 0. ;
		    }
		}
	    }

	fp = fopen("debug_phasecalib.dat", "wb") ;   /* #### */
	fwrite( phase_targ, sizeof( float ), NumAzim * NumRangeBins, fp ) ;
	fclose( fp ) ;
	fp = fopen("debug_phasecalib.ppm", "wb") ;
	fprintf(fp, "P5 %d %d 255 ", NumRangeBins, NumAzim) ;
	for( offset = 0 ; offset < NumRangeBins*NumAzim ; offset++ ) {
	    if( phase_targ[offset] == INVALID )
		c = 0 ;
	    else
		c = (char)(phase_targ[offset]*127./180.) ;
	    fwrite( &c, sizeof(char), 1, fp ) ;
	}
	fclose( fp ) ;

/* And try to eliminate (or at least dampen the accuracy of) echoes that
   could be the result of sidelobes.  Echoes that are around or below
   SideLobePow dB of the integrated power strength for all azimuths at a
   given range are affected, as well as those that are immediately next to
   much stronger targets. */
	contamin_pow = exp(log(4.)*SQR(360./(float)NumAzim/BeamWidth)) ;
	for( az = 0, offset = 0 ; az < NumAzim ; az++ )	/* First, look for mainlobe contamination on nearby azimuths */
	    for( r = RMin, offset += RMin ; r < NumRangeBins ; r++, offset++ )
		if( calib_ref[offset].av_i > 0. ) {
		    local_pow = pow(10., .1 * calib_ref[offset].strength ) ;
		    j = (az + NumAzim - 1) % NumAzim ;
		    near_pow = pow(10., .1 * calib_ref[j*NumRangeBins+r].strength) ;
		    j = (az + 1) % NumAzim ;
		    near_pow += pow(10., .1 * calib_ref[j*NumRangeBins+r].strength) ;
		    near_pow /= local_pow ;
		    if( near_pow > 2.5*contamin_pow ) {
		        side_correction = exp(-.5*SQR((near_pow-2.5*contamin_pow)/(2.5*contamin_pow))) ;
			if( side_correction < .1 )  side_correction = .1 ;
			calib_ref[offset].av_i *= side_correction ;
			if( calib_ref[offset].av_i != 0. )
			    calib_ref[offset].phase_er = sqrt(-2. * log( calib_ref[offset].av_i ) / calib_ref[offset].av_i) / DEGTORAD ;
			else
			    calib_ref[offset].phase_er = VERY_LARGE ;
		    }
	 	}

	contamin_pow = 4. ; /* #### Assumption: PulseLength = GateSpacing with a matched filter */
	for( az = 0, offset = 0 ; az < NumAzim ; az++ )	/* Then, look for possible range sidelobes */
	    for( r = RMin, offset += RMin ; r < NumRangeBins ; r++, offset++ )
		if( calib_ref[offset].av_i > 0. ) {
		    local_pow = pow(10., .1 * calib_ref[offset].strength ) ;
		    if( r == 0 )
			near_pow = VERY_LARGE ;
		    else
			near_pow = pow(10., .1 * calib_ref[offset-1].strength ) ;
		    if( r < NumRangeBins - 1 )
			near_pow += pow(10., .1 * calib_ref[offset+1].strength ) ;
		    near_pow /= local_pow ;
		    if( near_pow > 2.5*contamin_pow ) {
		        side_correction = exp(-.5*SQR((near_pow-2.5*contamin_pow)/(2.5*contamin_pow))) ;
			if( side_correction < .1 )  side_correction = .1 ;
			calib_ref[offset].av_i *= side_correction ;
			if( calib_ref[offset].av_i != 0. )
			    calib_ref[offset].phase_er = sqrt(-2. * log( calib_ref[offset].av_i ) / calib_ref[offset].av_i) / DEGTORAD ;
			else
			    calib_ref[offset].phase_er = VERY_LARGE ;
		    }
	 	}

       for( r = RMin ; r < NumRangeBins ; r++ ) {       /* Finally
handle 360 deg sidelobes */
           sum_pow = 0. ;
           for( az = 0, offset = r ; az < NumAzim ; az++, offset +=
NumRangeBins )
               sum_pow += pow( 10., .1 * snr[offset] ) ;
           side_pow = sum_pow * pow( 10., .1 * (SideLobePow)) ;
           for( az = 0, offset = r ; az < NumAzim ; az++, offset +=
NumRangeBins ) {
               if( calib_ref[offset].av_i > 0. ) {
                   side_correction = exp( -side_pow * pow( 10.,
-.1*calib_ref[offset].strength ) ) ;
                   calib_ref[offset].av_i *= side_correction ;
                   if( calib_ref[offset].av_i != 0. )
                       calib_ref[offset].phase_er = sqrt(-2. * log(
calib_ref[offset].av_i ) / calib_ref[offset].av_i) / DEGTORAD ;
                   else
                       calib_ref[offset].phase_er = VERY_LARGE ;
               }
           }
       }

/* Archive results */
	fp = fopen( RefFileName, "wb" ) ;
	n_value = INVALID ;
	fwrite( &n_value, sizeof( float ), 1, fp ) ;
	fwrite( calib_ref, sizeof( struct R_info ), NumAzim * NumRangeBins, fp ) ;
	fclose( fp ) ;
	printf( "Task completed.         \n" ) ;
	free( phase_targ ) ;
}


/*****************************************************************************/

/* confirm_do_calibration( file_count ) :
   Ask user for confirmation concerning the computation of calibration of
   ground echo phase data.
   Input: Info necessary to print question.
   Output: Confirmation (or not) of action.
   Called by: main() */

int confirm_do_calibration( int file_count )
{
	int		iopt, j ;

	printf( "\n*************************** WARNING ****************************\n" ) ;
	printf( "\nYou have chosen: 4 - Perform calibration of ground clutter info\n\n" ) ;
	printf( "Just to make sure in case you may regret it: the program will use data\n" ) ;
	printf( "from the following %d files to perform the calibration:\n", file_count ) ;

	for ( j = 0 ; j < file_count ; j++ )
	    if( (j < 8) || (file_count <= 10) )
		printf("\t%s\n", fname_in[j] ) ;
	    else if( j == 8 )
		printf("\t...\n\t%s\n", fname_in[(file_count-1)] ) ;

	printf( "Shall we:\n" ) ;
	printf( "\t0 - Abort for now\n" ) ;
	printf( "\t1 - Proceed with the calibration of targets\n" ) ;
	do {
	    iopt = 0 ;
	    printf( "\nYour option: ") ;
	    scanf( "%d", &iopt ) ;
	} while (( iopt < 0 ) || ( iopt > 1 )) ;

	return( iopt ) ;
}


/**************************************************************************/

/* calib_targets( file_count ) :
   Computes the refractive index associated with the period of interest based
   on user input and the average phase of each targets for that period.
   Input: File list of volume scans to use for that determination
   Output: Updated reference file
   Called by: main() */

void calib_targets( int file_count )
{
	int             j, count, r, az, offset, file_num ;
	float           temperature, pressure, dew_point, vapor_pres, n_value ;
	float		norm, norm2 ;
	FILE            *fp ;

/* Get surface data */
	printf( "I will proceed with the calibration as soon as you tell me this:\n" ) ;
	printf( "Do you prefer to enter (1) N directly or (2) P, T, and Td: " ) ;
	scanf( "%i", &j ) ;
	if( j != 1 ) {
	    printf( "For the period covered by the data files, what was the field average:\n" ) ;
	    printf( "\tTemperature (C): " ) ;
	    scanf( "%f", &temperature ) ;
	    printf( "\tDew point temperature (C): " ) ;
	    scanf( "%f", &dew_point ) ;
	    printf( "\tStation (NOT sea level!) pressure (mb): " ) ;
	    scanf( "%f", &pressure ) ;
	    vapor_pres = 6.112 * exp(17.67 * dew_point / (dew_point + 243.5)) ;
	    n_value = 77.6 * pressure / (temperature + 273.16) + 373250 * vapor_pres / SQR(temperature + 273.16) ;
	}
	else {
	    printf( "For the period covered by the data files, what was the field average N: " ) ;
	    scanf( "%f", &n_value ) ;
	}

/* Initialize some variables */
	printf( "\nCalibrating ground targets for N = %5.2f...\n", n_value ) ;
	memset( sum_a, 0, sizeof(float) * NumAzim * NumRangeBins ) ;
	memset( sum_b, 0, sizeof(float) * NumAzim * NumRangeBins ) ;

/* Read phase data and normalize */
	printf("Going through all the files:\n") ;
	for( file_num = 0 ;  file_num < file_count ; file_num++ ) {
	    printf("   %d - %s   \r", file_num+1, fname_in[file_num]) ;
	    fflush(stdout) ;
	    stat( fname_in[file_num], &latest_data ) ;
	    switch (DataVersionID) {
		case MCG_PRE_VIRAQ:
		    j = read_data_previraq( fname_in[file_num] ) ;
		    break ;
		case MCG_VIRAQ:
		    j = read_data_viraq( fname_in[file_num] ) ;
		    break ;
		case MCG_DUALP:
		    j = read_data_dualp( fname_in[file_num] ) ;
		    break ;
		case SPOL_VIRAQ:
		    j = read_data_spol( fname_in[file_num] ) ;
		    break ;
		case SPOL_RVP8:
		    j = read_data_spol_rvp8( fname_in[file_num] ) ;
		    break ;
	    }
	    if( j == FALSE ) {
		printf("At least one of the scans, %s\n", fname_in[file_num] ) ;
		printf("appear to be bad (few targets).\n" ) ;
		printf("Please make sure that the files to be used are all good.  Quitting.\n") ;
		exit(4) ;
	    }

	for( az = 0, offset = 0 ; az < NumAzim ; az++ )
	    for( r = 0 ; r < NumRangeBins ; r++, offset++ ) {
		norm = sqrt( SQR(raw_phase[offset].inphase) + SQR(raw_phase[offset].quadrature) ) ;
		if( norm != 0. ) {
		    raw_phase[offset].inphase /= norm ;
		    raw_phase[offset].quadrature /= norm ;
		}
	    }

/* Average phase data by averaging I and Q */
	for( az = 0, offset = 0 ; az < NumAzim ; az++ )
	    for( r = 0 ; r < NumRangeBins ; r++, offset++ ) {
		sum_a[offset] += raw_phase[offset].inphase ;
		sum_b[offset] += raw_phase[offset].quadrature ;
	    }
	}

/* Update reference information: read, update, rewrite */
	printf("\nUpdating reference information...\n") ;
	fp = fopen( RefFileName, "rb" ) ;
	fseek( fp, sizeof(float), SEEK_SET ) ;
	fread( calib_ref, sizeof( struct R_info ), NumRangeBins * NumAzim, fp ) ;
	fclose( fp ) ;

	for( offset = 0 ; offset < NumAzim * NumRangeBins ; offset++ ) {
	    norm2 = sqrt(SQR(calib_ref[offset].av_i) + SQR(calib_ref[offset].av_q) ) ;
	    if( norm2 != 0. ) {
		norm = sqrt(SQR(sum_a[offset]) + SQR(sum_b[offset])) / norm2 ;
		if( norm != 0. ) {
		    calib_ref[offset].av_i = sum_a[offset] / norm ;
		    calib_ref[offset].av_q = sum_b[offset] / norm ;
		}
		else {
		    calib_ref[offset].av_i = 0. ;
		    calib_ref[offset].av_q = 0. ;
		    calib_ref[offset].phase_er = VERY_LARGE ;
		}
	    }
	}
	fp = fopen( RefFileName, "wb" ) ;
	fwrite( &n_value, sizeof( float ), 1, fp ) ;
	fwrite( calib_ref, sizeof( struct R_info ), NumRangeBins * NumAzim, fp ) ;
	fclose( fp ) ;
	printf( "Calibration completed.\n" ) ;
}


/*****************************************************************************/

/* confirm_do_adjustment( file_count ) :
   Ask user for confirmation concerning the computation of the calibration 
   adjustment for changes in propagation conditions (dN/dz).
   Input: Info necessary to print question.
   Output: Confirmation (or not) of action.
   Called by: main() */

int confirm_do_adjustment( int file_count )
{
	int		iopt, j ;

	printf( "\nYou have chosen: 5 - Adjust calibration for changes in propagation\n\n" ) ;
	printf( "Just to make sure in case you may regret it: the program will use data\n" ) ;
	printf( "from the following %d files to perform the calibration adjustment:\n", file_count ) ;

	for ( j = 0 ; j < file_count ; j++ )
	    if( (j < 8) || (file_count <= 10) )
		printf("\t%s\n", fname_in[j] ) ;
	    else if( j == 8 )
		printf("\t...\n\t%s\n", fname_in[(file_count-1)] ) ;

	printf( "Shall we:\n" ) ;
	printf( "\t0 - Abort for now\n" ) ;
	printf( "\t1 - Proceed with the calibration adjustment\n" ) ;
	do {
	    iopt = 0 ;
	    printf( "\nYour option: ") ;
	    scanf( "%d", &iopt ) ;
	} while (( iopt < 0 ) || ( iopt > 1 )) ;

	return( iopt ) ;
}


/**************************************************************************/

/* adjust_targets( file_count ) :
   Attempt to compute some kind of adjustment or correction factor for the
   phase of the target as propagation conditions (such as dN/dz) changes.
   Input: File list of volume scans to use for that determination
   Output: Updated reference file
   Called by: main() */

void adjust_targets( int file_count )
{
}


/**************************************************************************/

void error_out( char *text, int fatal_flag )
{
	if( fatal_flag != FALSE ) {
	    printf( "N_CALIB error: %s.\n", text ) ;
	    exit( fatal_flag ) ;
	}
	else
	    printf( text ) ;
}
	

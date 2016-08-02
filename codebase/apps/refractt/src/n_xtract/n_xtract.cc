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
/* This program computes a field of index of refraction using a set of
   ground echo data scans as well as calibration information created by the
   calibration program (n_calib.c).  This program includes both the
   research as well as the real-time version of the code.
   By Frederic Fabry, March 2003.

Usage: n_xtract params_file
   where params_file is the filename of the parameters file that determines the
   mode of operation of n_xtract (research or real-time) as well as various user
   changeable options and variables.

   Code that should be changed for uses in systems other than the
   McGill Radar is indicated by the string @@@@ , and has been isolated
   in subroutines for the most part to make the task easier for you.

   There are a few `goto' in the code to make MY task easier.
   You don't like them?  Tough.  My programming doesn't always follow
   approved practices...    :-)

   Special requirements: a noticeable amount of RAM (~ 100 bytes per radar
   range cell (number of azimuths * number of range bins); ~ 20 MB for the
   McGill system).  It can also gobble disk space if you use the circular
   queue (useful to select scans of interest for final archiving).

   Complete set of files needed for n_xtract to run:
n_xtract.h	Parameter and structure definition file
n_xtract.c	The main program (this file)
n_input.c	Module of subroutines
n_input_mcgill.c  "    "      "
n_input_spol.cc	  "    "      "
n_smooth.c	  "    "      "
n_output.c	  "    "      "
n_output_spol.cc  "    "      "
n_calib.c	Phase reference builder program
n_7x12.fnt	Font used in some McGill graphics products @@@@
n_geog.ovr	Geographical overlay used in some McGill products @@@@
RefFileName	Reference file (name is in params_file) built
		by n_calib
RTDataFileName	In the real-time version, it is the file in which the latest
		input data comes to (defined in params_file)
params_file	The file that contains all the parameters.

Just compile n_xtract.c and n_calib.c with a math library using
"gcc" or your favorite C/C++ compiler.  The main program only uses C
code, but data input and product output routines may use C++.

BUGS/WATCH-OUT:
- On some workstations (SGI), n_xtract will misbehave if compiled with "cc"
  using the -O option but not if no optimization is asked (compiler bug?)

NOTES AND POSSIBLE CHANGES:
- Quality: is OK to detect ground vs other; not good for "good ground"
  vs "bad ground"; also fooled by zero-v weather.
  I.e., don't expect miracles from the quality parameter alone.

- Accept the fact that 2 km resol is the most you will ever get from the
  absolute diff data and that 1 km is the _best_ for 30-min data.  Even then,
  the price on data noise and artifacts will be heavy.
  While the technique could sense N differences at smaller scales, those are
  extremely difficult to map with certainty given the large phase signal
  (and associated folding) and the fact that not all targets are at the
  same height and fixed.

- Discover the bugs left!  I am sure there a few still hiding...

- Have a more intelligent way of setting the N scale (using forecasts?)

- Allow the possibility of detecting propagation environment using phase data?


-------------------------------------------------------------------------------

                      ***** ALGORITHM DESCRIPTION *****

Assumptions:
Either: Terrain is perfectly flat and targets are all at the same height;
Or: Targets are roughly at the same height and dN/dz is constant in time close
to the ground.
Violation of these assumptions will result in noise or bias in the
measurement of N.

Goal: 
Uses the change in the phase of ground echoes to determine changes in the
refractive index with respect to a reference time for which the field
of refractive index is assumed to be known.

Challenge, or why the program is so big:
The phase data from ground targets can be very noisy and contaminated by
weather echoes (and the refractive index is derived from the _slope_ of this
noisy field).  Most importantly, the signal in phase is very large (the
phase change in time can span up to 2 Nyquist intervals per km at S-band)
and careful processing has to be performed to retrieve sparse information that
must then be converted to a field.

Calibration:
A reference map has to be generated using "calibration scans" for which
N is assumed to be constant (in McGill's case, a foggy period just after some
long stratiform rain).  Stable echoes were identified and their phase as well
as their reliability were measured.  Calibration is done using n_calib.

This program is the fifth generation of n_xtract (with 2 1/2 that never really
worked because the problem was to messy for them).  To get some background
information on some of the physical principles behind the extraction of
refractivity (or index of refraction) using ground targets as well as some
flavors of the problems this program is facing to retrieve this information,
read "On the extraction of near-surface index of refraction using radar phase
measurements from ground targets" by Fabry et al., Journal of Atmospheric and
Oceanic Technology, Vol 14 (1997), pages 978-987.  While the paper describes
techniques and ideas used in the first generation of n_xtract, it will give
you a good introduction to the problem and some understanding of why some
things were done in a given way.  A newer manuscript, "Ground Echo Radar
Meteorology: From ground targets to refractive index", is in the final stages
of preparation.

The program, as designed, contains both the real-time as well as the
research version of n_xtract.  The real-time version is an infinite
loop that:
- Waits for the phase data to be available
- Reads the data and prepare the various input fields
- Smooths the phase data as well as possible
- Constructs the refractivity and retractivity trend maps from the smmothed
  data
- Generates and archives various outputs

The research version is very similar, except that it uses a list of files
to process as opposed to waiting for new data to appear in a given file.

			-------------------------
			  ALGORITHM DESCRIPTION
			-------------------------

As soon as a new volume scan is available (real-time) or once the program
is ready to process the next file (research), the following process starts:

I * Data reading and evaluation.
The data from the new volume scan are read, and the average phase of
targets as well as their "quality" is determined.  The quality parameter
uses various information like target SNR, velocity and spectrum width
to determine if the target observed is a suitable ground target for
the purposes of this program.  As coded, the quality parameter does not
do miracles, but it can identify whether we are dealing with a moving
weather echo or a probable ground target.
Another evaluation done is whether there is any coverage, i.e. if the
radar was transmitting.  If that is not the case, the process is
aborted for this scan.

II * Computation of phase differences.
Once the phase of targets have been measured, they are used in combination
with similar measurements made at the previous scan and at a calibration
step to generate fields of phase differences.  These fields of phase
Differences (mapped between -180 and 180 deg) will be used as the input
to the derivation of refractive index fields.

Then, for both the scan-to-scan and the scan-to-reference phase difference
fields, the program will do the following:

III * Smoothing of the phase difference field.
The field of phase differences between two successive scans (used to
observe small time scale changes in refractivity between succesive scans or
over 30 min) and the field of phase differences between the current scan and
the reference scan (used o obtain true N measurements) are then smoothed.
Ideally, one would want to dealias the phase data directly.
However, given the magnitude of the task (multiple foldings occur easily,
even between successive scans) and the noisy nature of the data, one has
to adopt a conservative approach.  Hence we sacrifice resolution for
accuracy.  However, phase is a difficult quantity to smooth because of its
periodic nature.  This can be further complicated if there is a sharp
systematic trend in range.
Therefore the smoothing itself is a more complicated task than it first
appears, and some of the most sophisticated routines of this program are
devoted to that task.  Also generated is the field of expected errors (or
uncertainty) on each smoothed phase estimate.

IV * Extraction of N (or Delta-N) measurements and their error.
Once the smoothed phase field is computed, the value of N (or Delta-N) is
computed from the slope in range of the smoothed phase field along each
radial.  An associated error is also computed from the error in phase.

V * Relaxation/diffusion of N measurements over a 2-D field.
If requested, the field of N (or Delta-N) obtained can be "relaxed".
The N info at this point consists of N measurements in data-rich regions and
an error associated with each measurement.  This routine, one of the slowest
part of the code, performs a "diffusion" of the N measurement
over a 2-D polar field.  An iterative two step process is made:
1 - Over regions where N information is available, values of N are being nudged
towards the value expected from the phase data, the strength of the nudging
being inversely proportional to the error on the N measurement.
2 - A diffusion process is made where the value of N at any point in the
field is set to the average of itself and its immediate neighbors.
While part 1 forces the information where it is known, part 2 diffuses it
out.  After several iterations, the smoothing starts to exert its influence
outside the smoothing region defined in the parameter file, and the relaxation
is stopped.

VI * Archival of data and generation of products.
Since we now have what we looked for, it is time to save all that work
and generate the various outputs required by the user.

Now that I've got everyone completely confused, I'll stop.   :-)

				-----------------

Overall performance:
My experience with the current algorithm is that it is performing
reasonably well given the quality and the difficulties associated with 
the input data (massive folding, etc.).  You will see that considerable
efforts are spent doing data smoothing and dealiasing, and these efforts are
paying (it is very slow, but it is very necessary).
You want to improve on this?  Fine with me!  Have fun...

------------------------------------------------------------------------------
*/

#define		N_XTRACT

/* Declaration and includes */
#include	<refractt/nrutil.h>	/* Include & defines for Numerical Recipes */
#include	<refractt/n_xtract.h>	/* Include & defines for this program */

#include	<refractt/n_input.h>
#include	<refractt/n_input_mcgill.h>
#ifdef NOTDEF
#include	<refractt/n_input_spol.cc>
#include	"n_smooth.c"
#include	"n_output.c"
#endif

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
#define		Wavelength	(C_VACUUM/Frequency)

/* Large arrays and global variables */
struct T_data	*dif_from_ref ;    /* [NumAzim*NumRangeBins] ; */
struct T_data	*dif_prev_scan ;   /* [NumAzim*NumRangeBins] ; */
struct T_data	*raw_phase ;	   /* [NumAzim*NumRangeBins] ; */
struct T_data	*smooth_dif_ref ;  /* [NumAzim*NumRangeBins] ; */
struct T_data	*smooth_dif_scan ; /* [NumAzim*NumRangeBins] ; */
struct T_info	*target ;
struct R_info	*calib_ref ;
float		*n_array_polar ;   /* [NOutNumAzim*NOutNumRanges] ; */
float		*dn_array_polar ;  /* [DNOutNumAzim*DNOutNumRanges] ; */
float		*n_array_xy ;	   /* [CartesianY*CartesianX] ; */
float		*dn_array_xy ;	   /* [CartesianY*CartesianX] ; */
float		*n_er_array_polar ;  /* [NOutNumAzim*NOutNumRanges] ; */
float		*dn_er_array_polar ; /* [DNOutNumAzim*DNOutNumRanges] ; */
float		*n_er_array_xy ;     /* [CartesianY*CartesianX] ; */
float		*dn_er_array_xy ;    /* [CartesianY*CartesianX] ; */
char		*last_phase_dif ;  /* [NumMapDiff][PATH_MAX] */ ;
int		az_cor, bridge_detected ;  /* @@@@ */
struct stat	old_latest_data, latest_data ;
float           new_av_n, old_av_n, ref_n, n_from_slope, n_change, ap_indx ;
int             year, month, day, hour, minute, sec ;
time_t		time_cur_scan ;
int		just_started = TRUE ;
char		*data_in_name, OutputDir[PATH_MAX] ;
FILE            *fp ;

#ifdef DUMP_NC
int		Dump_nc = 0;
#endif
float		elev_angle ;
int		gnd_filter ;
char		RealFileName[PATH_MAX] ;

/* Naming conventions:
ALL_CAPS: Constants set by #define, mostly in n_xtract.h
SomeCaps: Variables read from the user-changeable parameters file
nocaps: Other variables
*/
/* Important or often-used global variables:
calib_ref[]:	Reference info (done by n_calib) for each target:
   av_i/av_q:		Average I/Q of target for N = Reference value (ref_n);
   strength:		SNR of target at reference time;
dif_from_ref[]: Phase difference between current and reference target phase.
dif_prev_scan[]: Phase (in I/Q form) difference between 2 successive scans.
dn_array_polar[]: Resulting N-trend field in polar coordinates.
dn_array_xy[]:	Resulting N-trend field in cartesian coordinates.
dn_er_array_polar[]: Resulting N-trend error field in polar coordinates.
dn_er_array_xy[]: Resulting N-trend error field in cartesian coordinates.
fp:		General purpose file pointer.
gnd_filter:	Flag of ground echo filter
just_started:	Flag set during the first pass through (for initialization).
last_phase_dif[]: Filename of phase info for the past few scans.
n_array_polar[]: Resulting N field in polar coordinates.
n_array_xy[]:	Resulting N field in cartesian coordinates.
n_er_array_polar[]: Resulting N error field in polar coordinates.
n_er_array_xy[]: Resulting N error field in cartesian coordinates.
n_change:	Delta-N of N over the field (derived from d(Phase)/dr).
n_from_slope:	Average value of N over the field (derived from d(Phase)/dr).
new_av_n:	Average value of N over the field (derived from paths).
num_paths:	Total number of path measurements of N stored in rays[].
num_slopes:	# of low-res N measurement derived from synthetic dealiasing.
old_av_n:	Average value of N over the field on the previous scan.
raw_phase[]:	In-phase (I) and quadrature (Q) components of phase of target.
ref_n:		Reference value of N at calibration time.
smooth_dif_ref[]: Smoothed phase dif between current and reference target phase.
smooth_dif_scan[]: Smoothed phase dif (in I/Q form) between 2 successive scans.
snr/sw/vel/zh[]: SNR/spectrum width/velocity/reflectivity info from each target.
target[]:	Information on targets at each azimuth-range cell:
   dealias_flag:	Flags progress in phase dealiasing;
   phase:		Dealiased scan-to-reference phase difference;
   phase_diff:		Dealiased scan-to-scan difference;
   phase_to_dealias: 	Scan-to-reference phase difference to be dealiased;
			Then: first guess of dealiasing (direct method);
   quality:		Quality of the target;
   stability_count:	For how long have we tried this dealiasing value;
   trust_level:		Score of how much we trust our dealiasing estimate;
   trust_trend:		Change in trust level between scans;
*/

/**************************************************************************/

int main(int argc, char *argv[])
{
	char	data_filename[PATH_MAX], data_filename_Z[PATH_MAX] ;
	int	j, scan_count, valid ;
#ifdef DUMP_NC
	if (getenv("DUMP_NC")) {
	    Dump_nc = 1;
	}
#endif

/* Determine mode of operation; set-up arrays, etc. */
/*	mallopt( M_DEBUG, TRUE ) ;	To use with "-lmalloc" */
	scan_count = startup(argc, argv) ;

/* Main loop, research mode: */
	if( RealTimeMode == FALSE ) {
	    for( j = 0 ; j < scan_count ; j++ ) {
		strcpy( data_filename, data_in_name+j*PATH_MAX ) ;
		stat( data_filename, &latest_data ) ;
		if( (DebugLevel & 0xF) >= VOLUBLE )
		    debug_print("\nProcessing file\n") ;
		if( get_targets( data_filename ) == TRUE ) {   /* Read */
		    printf("\rProcessing %d - %s ", j+1, data_filename) ;
		    strcpy(RealFileName, data_filename) ; /* supply RealFileName, just as in real-time mode */
		    fflush( stdout ) ;
		    dif_phase() ;		/* Compute phase diff. fieds */
		    valid = fit_phases() ;	/* Fit N & DN field to phase data */
		    save_info(valid) ;		/* Save program vars */
		    save_research(j) ;		/* Results in distinct files */
		    generate_products(valid) ;	/* Generate outputs */
		    just_started = FALSE ;
		}
	    }
	    free_arrays() ;
	    system( "rm -f targets.tmp.*" ) ;
	    printf("\n\n\nN extraction completed!\n\n") ;
	    exit( 0 ) ;
	}

/* Main loop, real-time mode (forever): */
	else {
	    sprintf( data_filename, RTDataFileName ) ;
	    if( (DataVersionID == MCG_VIRAQ) || (DataVersionID == MCG_DUALP) )
		sprintf( data_filename_Z, "%s.Z", RTDataFileName ) ;
	    do {
		if( (DebugLevel & 0xF) >= VOLUBLE )
		    debug_print("\nStarting wait for new volume scan\n") ;
		if( (DataVersionID == MCG_VIRAQ) || (DataVersionID == MCG_DUALP) )
		    wait_rt_data( data_filename_Z ) ; /* Wait for data */
		else
		    wait_rt_data( data_filename ) ;

		if( (DebugLevel & 0xF) >= VOLUBLE )
		    debug_print("\nReading targets...\n") ;
		if( get_targets( data_filename ) == TRUE ) {	/* Read */

		    if( (DebugLevel & 0xF) >= VOLUBLE )
			debug_print("\nComputing phase differences...\n") ;
		    dif_phase() ;		/* Compute phase diff. fieds */

		    if( (DebugLevel & 0xF) >= VOLUBLE )
			debug_print("\nGenerating N field...\n") ;
		    valid = fit_phases() ;    /* Fit N & DN field to phase data */

		    if( (DebugLevel & 0xF) >= VOLUBLE )
			debug_print("\nSaving info...\n") ;
		    save_info(valid) ;		/* Save program vars */

		    if( (DebugLevel & 0xF) >= VOLUBLE )
			debug_print("\nGenerating products...\n") ;
		    generate_products(valid) ;	/* Generate outputs */

		    just_started = FALSE ;
		}
	    } while (1) ;
	}
}


/**************************************************************************/

/* startup(command line params):  Initializes the whole process:
   - Determines the mode of operation (RESEARCH vs REAL-TIME mode);
   - Prints start-up message (well... it _is_ an important task!);
   - Allocates and initializes all the needed global variables;
   - Loads the data from the reference (calibration) file.
   Input: Command line parameters
   Output: Lots of status variables used later
   Called by: main() */

int startup( int argc, char *argv[] )
{
	int	scan_count ;
	char	tmp_str[200] ;
	time_t	t ;
        
/* Read parameters file */
	if( argc == 2 )
	    get_params( argv[1] ) ;
	else {
	    printf("Usage: n_xtract parameter_file\n") ;
	    printf("\twhere \"parameter_file\" contains all the data processing parameters used by n_calib and n_xtract.\n\n") ;
	    exit(1) ;
	}

/* Start up */
	printf( "N_XTRACT Version %s:\n", N_VERSION) ;
	printf( "Extracts near-surface index of refraction from ground echoes.\n" ) ;
	printf( "Frederic Fabry sends greetings (%s)\n\n", MONTH_YEAR) ;
	new_av_n = INVALID ;

/* If research, get the list of raw data files to be processed */
	if( RealTimeMode == FALSE ) {
	    data_in_name = (char *) malloc(NUMSCANS * PATH_MAX * sizeof(char)) ;
	    if( data_in_name == NULL ) {
		sprintf( tmp_str, "Out of memory allocating file list array." ) ;
		error_out( tmp_str, 3 ) ;
	    }
	    if( ResListFile[0] != 0 ) {
		printf("Research mode: Reading files from the list in %s\n", ResListFile) ;
		scan_count = read_list( ResListFile, data_in_name ) ;
	    }
	    else {
		printf("Research mode: Reading files %s to %s\n", ResFirstFile, ResLastFile) ;
		scan_count = get_file_list( ResFirstFile, ResLastFile, data_in_name ) ;
	    }
	    if( scan_count == 0 ) {
		sprintf( tmp_str, "No file found matching the start to end file criteria" ) ;
		error_out( tmp_str, 2 ) ;
	    }
	    printf("\nHere we go!\n\n") ;
	}
	else
	    scan_count = -1 ;

/* Put some status information on disk */
	if( RealTimeMode == FALSE ) {
	    fp = fopen( StatFileName, "at" ) ;
	    if (fp == NULL) {
		sprintf(tmp_str, "Cannot open %s\n" , StatFileName);
		error_out( tmp_str, 2 );
	    }
	    time( &t );
	    fprintf( fp, "\n\t\t\t-------------------\n" ) ;
	    fprintf( fp, "\nN_XTRACT %s (research) started for data from %s to %s\n", N_VERSION, data_in_name, data_in_name+PATH_MAX*(scan_count-1) ) ;
	    fprintf( fp, "NameOfRun: %s; Author: %s; ProjectName: %s; DataVersion: %s/%s; Date: %s\n", NameOfRun, Author, ProjectName, DataVersion, SubVersion, ctime( &t ) ) ;
	    fprintf( fp, "ParameterFile: %s; RefFileName: %s; DestinationPath: %s; NSmoothingSideLen: %g; DNSmoothingSideLen: %g; MinConsistency: %g; DoMapDiff: %d; DoCartesianN: %d\n", argv[1], RefFileName, DestinationPath, NSmoothingSideLen, DNSmoothingSideLen, MinConsistency, DoMapDiff, DoCartesianN ) ;
	}
	else {
	    fp = fopen( StatFileName, "at" ) ;
	    if (fp == NULL) {
		sprintf(tmp_str, "Cannot open %s\n" , StatFileName);
		error_out( tmp_str, 2 );
	    }
	    fprintf( fp, "\n\t\t\t-------------------\n" ) ;
	    time( &t );
	    fprintf( fp, "\nN_XTRACT %s (real-time) started on %s", N_VERSION, ctime( &t ) ) ;
	    fprintf( fp, "NameOfRun: %s; Author: %s; ProjectName: %s; DataVersion: %s/%s; Date: %s\n", NameOfRun, Author, ProjectName, DataVersion, SubVersion, ctime( &t ) ) ;
	    fprintf( fp, "ParameterFile: %s; RefFileName: %s; DestinationPath: %s; NSmoothingSideLen: %g; DNSmoothingSideLen: %g; MinConsistency: %g; DoMapDiff: %d; DoCartesianN: %d\n", argv[1], RefFileName, DestinationPath, NSmoothingSideLen, DNSmoothingSideLen, MinConsistency, DoMapDiff, DoCartesianN ) ;
	}
	fclose( fp ) ;
	remove( LatestNPolar ) ;
	old_latest_data.st_mtime = 0 ;

/* Create main data arrays */
	if( DoMapDiff )
	    last_phase_dif = (char *)  malloc( NumMapDiff * PATH_MAX * sizeof( char ) ) ;
	dif_from_ref = (struct T_data *)  malloc( NumAzim * NumRangeBins * sizeof( struct T_data ) ) ;
	dif_prev_scan = (struct T_data *)  malloc( NumAzim * NumRangeBins * sizeof( struct T_data ) ) ;
	raw_phase = (struct T_data *)  malloc( NumAzim * NumRangeBins * sizeof( struct T_data ) ) ;
	smooth_dif_ref = (struct T_data *)  malloc( NumAzim * NumRangeBins * sizeof( struct T_data ) ) ;
	smooth_dif_scan = (struct T_data *)  malloc( NumAzim * NumRangeBins * sizeof( struct T_data ) ) ;
	n_array_polar = (float *)  malloc( NOutNumAzim * NOutNumRanges * sizeof( float ) ) ;
	dn_array_polar = (float *)  malloc( NOutNumAzim * NOutNumRanges * sizeof( float ) ) ;
	n_array_xy = (float *)  malloc( CartesianX * CartesianY * sizeof( float ) ) ;
	dn_array_xy = (float *)  malloc( CartesianX * CartesianY * sizeof( float ) ) ;
	n_er_array_polar = (float *)  malloc( NOutNumAzim * NOutNumRanges * sizeof( float ) ) ;
	dn_er_array_polar = (float *)  malloc( NOutNumAzim * NOutNumRanges * sizeof( float ) ) ;
	n_er_array_xy = (float *)  malloc( CartesianX * CartesianY * sizeof( float ) ) ;
	dn_er_array_xy = (float *)  malloc( CartesianX * CartesianY * sizeof( float ) ) ;
	target = (struct T_info *)  malloc( NumAzim * NumRangeBins * sizeof( struct T_info ) ) ;
	calib_ref = (struct R_info *)  malloc( NumAzim * NumRangeBins * sizeof( struct R_info ) ) ;

	if( calib_ref == NULL ) {
	    sprintf( tmp_str, "Out of memory allocating global arrays" ) ;
	    error_out( tmp_str, 3 ) ;
	}
	memset( raw_phase, 0, sizeof(struct T_data) * NumRangeBins * NumAzim ) ;
	memset( target, 0, sizeof(struct T_info) * NumRangeBins * NumAzim ) ;
	if( DoMapDiff )
	    memset( last_phase_dif, 0, sizeof(char) * NumMapDiff * 80 ) ;

/* Load reference target info */
	fp = fopen( RefFileName, "rb" ) ;
	if( fp == NULL ) {
	    sprintf( tmp_str, "Cannot open reference file %s\n", RefFileName ) ;
	    error_out( tmp_str, 3 ) ;
	    return( 0 ) ;
	}

	fread( &ref_n, sizeof( float ), 1, fp ) ;
	fread( calib_ref, sizeof( struct R_info ), NumRangeBins * NumAzim, fp ) ;
	fclose( fp ) ;

	if( DebugLevel != QUIET ) {
	    sprintf( tmp_str, "Reference N = %6.2f\n", ref_n ) ;
	    debug_print( tmp_str ) ;
	}

	return( scan_count ) ;
}


/**************************************************************************/

/* wait_rt_data():  Waits for the raw radar data file to be updated.
   The code is split in two because the McGill real-time (and code-compiling)
   environment is very different from NCAR's.
   Input: None
   Output: Quits when new data is available for processing
   Called by: main()  */

void wait_rt_data( char *data_filename )
{
	char	tmp_str[120] ;

/* In the McGill computing environment */
#if( N_ENVIRONMENT == 0 )
	old_av_n = new_av_n ;
	stat( data_filename, &latest_data ) ;

/* Wait for a change in the time stamp of the data file (coming by ftp) @@@@ */
	while(( latest_data.st_mtime == old_latest_data.st_mtime ) || ( latest_data.st_size <= 0 )) {
	    sleep( 10 ) ;
	    stat( data_filename, &latest_data ) ;
	}

/* Change has occured: Wait until file has the same size for 5 s */
	while( latest_data.st_mtime > time(NULL) - 5 ) { ; /* @@@@ Enough? #### */
	    if( DebugLevel != QUIET )
		debug_print("\nWaiting for file to be completely written \n" ) ;
	    sleep( 2 ) ;
	    stat( data_filename, &latest_data ) ;
	}

/* New file has arrived; proceed to next routine */
	memcpy( &old_latest_data, &latest_data, sizeof(struct stat) ) ;

	if( (DataVersionID == MCG_VIRAQ) || (DataVersionID == MCG_DUALP) ) {
	    sprintf( tmp_str, "rm %s", data_filename ) ;
	    tmp_str[strlen(tmp_str)-2] = 0 ;	/* Cut .Z extension */
	    system( tmp_str ) ;
	    sprintf( tmp_str, "uncompress %s", data_filename ) ;
	    system( tmp_str ) ;
	}


/* In the NCAR computing environment */
#elif( N_ENVIRONMENT == 1 || N_ENVIRONMENT == 2 )

	old_av_n = new_av_n ;
	stat( data_filename, &latest_data ) ;

restart:
/* Wait for a change in the time stamp of the data file */
	while(( latest_data.st_mtime == old_latest_data.st_mtime ) || ( latest_data.st_size <= 0 )) {
	    if( (DebugLevel & 0xF) >= VOLUBLE ) {
		if (latest_data.st_mtime == old_latest_data.st_mtime) {
		    sprintf(tmp_str, "\nWaiting for new ground echo file after %s\n",
			asctime(gmtime(&latest_data.st_mtime)) ) ;
		    debug_print( tmp_str ) ;
		}
		else {
		    debug_print("Waiting for non-zero length file\n");
		}
	    }
	    sleep( 10 ) ;
	    stat( data_filename, &latest_data ) ;
	}

/* #ifdef NOTDEF  (####Restore?) we don't produce the link until the file is completely written */
/* Change has occured: Wait until file has the same size for 5 s */
	while( latest_data.st_mtime > time(NULL) - 5 ) { ; /* @@@@ Enough? #### */
	    if( DebugLevel != QUIET )
		debug_print("\nWaiting for file to be completely written \n" ) ;
	    sleep( 2 ) ;
	    stat( data_filename, &latest_data ) ;
	}

/* New file has arrived; proceed to next routine */
	memcpy( &old_latest_data, &latest_data, sizeof(struct stat) ) ;

/* we need the real name of the file for later, since this link may
 * change while we're processing it.  (race conditions are *so* nasty!)
 */
	memset(RealFileName, 0, PATH_MAX);
    
	if (readlink(data_filename, RealFileName, PATH_MAX) < 0) {
	    fprintf(stderr, "readlink call failed\n");
	    perror("readlink");
	    goto restart;
	}
    	strcpy(RealFileName, data_filename) ;
	if( DebugLevel != QUIET ) {
	    sprintf( tmp_str, "\nProcessing %s \n", RealFileName ) ;
	    debug_print( tmp_str ) ;
	}
#endif	/* N_ENVIRONMENT */

}


/**************************************************************************/

/* free_arrays():  Free all allocated arrays.
   Input: None
   Output: None
   Called by: main()  */

void free_arrays( void )
{
	free( calib_ref ) ;
	free( target ) ;
	free( dn_er_array_xy ) ;
	free( n_er_array_xy ) ;
	free( dn_er_array_polar ) ;
	free( n_er_array_polar ) ;
	free( dn_array_xy ) ;
	free( n_array_xy ) ;
	free( dn_array_polar ) ;
	free( n_array_polar ) ;
	free( smooth_dif_scan ) ;
	free( smooth_dif_ref ) ;
	free( dif_prev_scan ) ;
	free( dif_from_ref ) ;
	if( DoMapDiff )
	    free( last_phase_dif ) ;
	free( data_in_name ) ;
}

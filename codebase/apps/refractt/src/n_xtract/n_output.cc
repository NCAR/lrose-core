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
/* n_output.c:  Module of the programs n_xtract (to extract refractive index
   fields from ground targets) and n_calib (to produce calibration files for
   the former program).  This module takes care of all the outputs from
   n_xtract (including product generation).

   Code that should be changed for uses in systems other than the
   McGill Radar is indicated by the string @@@@ , and has been isolated
   in subroutines for the most part to make the task easier for you.

------------------------------------------------------------------------------
*/

/* Includes and global variables used primarily here */
#ifndef N_XTRACT_H
#include        <refractt/n_xtract.h>	/* Include & defines for this program */
#endif
#include <refractt/nrutil.h>
char		mon[12][4] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
				"JUL", "AUG", "SEP", "OCT", "NOV", "DEC" } ;
float		n_history[144] = {0} ;	/* @@@@ */


#define		MinNWinter	294.	/* Define display limits */
#define		MaxNWinter	(MinNWinter+42.)
#define		MinNSpring	280.
#define		MaxNSpring	(MinNSpring+70.)
#define		MinNSummer	300.
#define		MaxNSummer	(MinNSummer+70.)
#define		MinNFall	295.
#define		MaxNFall	(MinNFall+70.)


/**************************************************************************/

/* generate_products() :  Generate the various products required and save
   or transmit them to the outside world. (@@@@: Product set is system-
   dependant, especially when it comes to the format of the output)
   As opposed to save_info() which saves the information required by the
   program for its normal operation, generate_products() builds and saves
   the images and data required by the user (VIII).
   Input: N (and maybe DeltaN) field at current time; dealiased phase data.
   Output: All the output required by the user out of this process.
   Called by: main() */

void generate_products(int valid)
{
	int		j, k, x, y, day_indx, num_pts ;
	float		tmp_float ;
	short int	*cartesian_n ;
	unsigned short	tmp_ushort ;
	short		tmp_short ;
	struct Map_header header ;
	float		min_value, max_value, n_value, ref_value, remainder ;
	static float	old_min_n ;

	if( DebugLevel != QUIET)
	    debug_print("Generate products\n") ;

/* Save latest maps of N and Delta-N (polar & cartesian) */
	fp = fopen( LatestNPolar, "wb" ) ;
	fwrite( NameOfRun, sizeof(char), PATH_MAX, fp ) ;
	fwrite( Author, sizeof(char), PATH_MAX, fp ) ;
	fwrite( ProjectName, sizeof(char), PATH_MAX, fp ) ;
	fwrite( DataVersion, sizeof(char), PATH_MAX, fp ) ;
	fwrite( SubVersion, sizeof(char), PATH_MAX, fp ) ;
	fwrite( &NOutNumAzim, sizeof(int), 1, fp ) ;
	fwrite( &NOutNumRanges, sizeof(int), 1, fp ) ;
	tmp_float = (float)NumRangeBins * GateSpacing / NOutNumRanges ;
	fwrite( &tmp_float, sizeof(float), 1, fp ) ;
	num_pts = NOutNumAzim * NOutNumRanges ;

	for( k = 0 ; k < num_pts ; k++ ) {
	    tmp_ushort = (unsigned short int)(100. * n_array_polar[k]) ;
	    fwrite( &tmp_ushort, sizeof(unsigned short int), 1, fp ) ;
	}
	for( k = 0 ; k < num_pts ; k++ ) {
	    if( n_er_array_polar[k] > 655.35 )
		tmp_ushort = 65535 ;
	    else
	        tmp_ushort = (unsigned short int)(100. * n_er_array_polar[k]) ;
	    fwrite( &tmp_ushort, sizeof(unsigned short int), 1, fp ) ;
	}
	fclose( fp ) ;
	fp = fopen( LatestDNPolar, "wb" ) ;
	fwrite( NameOfRun, sizeof(char), PATH_MAX, fp ) ;
	fwrite( Author, sizeof(char), PATH_MAX, fp ) ;
	fwrite( ProjectName, sizeof(char), PATH_MAX, fp ) ;
	fwrite( DataVersion, sizeof(char), PATH_MAX, fp ) ;
	fwrite( SubVersion, sizeof(char), PATH_MAX, fp ) ;
	fwrite( &DNOutNumAzim, sizeof(int), 1, fp ) ;
	fwrite( &DNOutNumRanges, sizeof(int), 1, fp ) ;
	tmp_float = (float)NumRangeBins * GateSpacing / DNOutNumRanges ;
	fwrite( &tmp_float, sizeof(float), 1, fp ) ;
	num_pts = DNOutNumAzim * DNOutNumRanges ;
	for( k = 0 ; k < num_pts ; k++ ) {
	    tmp_short = (short int)(100. * dn_array_polar[k]) ;
	    fwrite( &tmp_short, sizeof(short int), 1, fp ) ;
	}
	for( k = 0 ; k < num_pts ; k++ ) {
	    if( dn_er_array_polar[k] > 655.35 )
		tmp_ushort = 65535 ;
	    else
	        tmp_ushort = (unsigned short int)(100. * dn_er_array_polar[k]) ;
	    fwrite( &tmp_ushort, sizeof(unsigned short int), 1, fp ) ;
	}
	fclose( fp ) ;

	if( DoCartesianN == TRUE ) {
	    fp = fopen( LatestNCart, "wb" ) ;
	    fwrite( NameOfRun, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( Author, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( ProjectName, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( DataVersion, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( SubVersion, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( &CartesianX, sizeof(int), 1, fp ) ;
	    fwrite( &CartesianY, sizeof(int), 1, fp ) ;
	    fwrite( &CartesianResol, sizeof(float), 1, fp ) ;
	    num_pts = CartesianX * CartesianY ;
	    for( k = 0 ; k < num_pts ; k++ ) {
		if( n_array_xy[k] == INVALID )
		    tmp_ushort = 0 ;
		else
		    tmp_ushort = (unsigned short int)(100. * n_array_xy[k]) ;
		fwrite( &tmp_ushort, sizeof(unsigned short int), 1, fp ) ;
	    }
	    for( k = 0 ; k < num_pts ; k++ ) {
		if( n_er_array_xy[k] > 655.35 )
		    tmp_ushort = 65535 ;
		else
		    tmp_ushort = (unsigned short int)(100. * n_er_array_xy[k]) ;
		fwrite( &tmp_ushort, sizeof(unsigned short int), 1, fp ) ;
	    }
	    fclose( fp ) ;
	    fp = fopen( LatestDNCart, "wb" ) ;
	    fwrite( NameOfRun, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( Author, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( ProjectName, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( DataVersion, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( SubVersion, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( &CartesianX, sizeof(int), 1, fp ) ;
	    fwrite( &CartesianY, sizeof(int), 1, fp ) ;
	    fwrite( &CartesianResol, sizeof(float), 1, fp ) ;
	    for( k = 0 ; k < num_pts ; k++ ) {
		if( dn_array_xy[k] == INVALID )
		    tmp_short = -32768 ;
		else
		    tmp_short = (short int)(100. * dn_array_xy[k]) ;
		fwrite( &tmp_short, sizeof(short int), 1, fp ) ;
	    }
	    for( k = 0 ; k < num_pts ; k++ ) {
		if( dn_er_array_xy[k] > 655.35 )
		    tmp_ushort = 65535 ;
		else
		    tmp_ushort = (unsigned short int)(100. * dn_er_array_xy[k]) ;
		fwrite( &tmp_ushort, sizeof(unsigned short int), 1, fp ) ;
	    }
	    fclose( fp ) ;
	}

/* Save product for McGill PPS:  N field map (@@@@)*/
	if( OUTFIT == MCGILL ) {	/* #### Move to n_output_mcgill? */
	    if( RealTimeMode != FALSE ) {
		cartesian_n = (short int *) malloc( CartesianX * CartesianY * sizeof(short int) ) ;
		if( cartesian_n != NULL ) {
		    memset(cartesian_n, 0, CartesianX * CartesianY * sizeof(short int) ) ;
		    day_indx = month * 100 + day ;	/* Date-dependent scale */
		    if( (day_indx > 1200) || (day_indx <= 315) ) {
			min_value = MinNWinter ;
			max_value = MaxNWinter ;
		    }
		    else if( day_indx < 600 ) {
			min_value = MinNSpring ;
			max_value = MaxNSpring ;
		    }
		    else if( day_indx < 900 ) {
			min_value = MinNSummer ;
			max_value = MaxNSummer ;
		    }
		    else {
			min_value = MinNFall ;
			max_value = MaxNFall ;
		    }
		    
		    remainder = 0. ;
		    for ( y = 0, k = 0 ; y < CartesianY ; y++ )
			for ( x = 0 ; x < CartesianX ; x++, k++ ) {
			    n_value = n_array_xy[k] ;
			    cartesian_n[k] = 0 ;
			    if(((x & 1) || (y & 1)) && ( n_value <= 0. ))
				ref_value = 0. ;
			    else if( n_value != INVALID ) {
				remainder += fabs( n_value ) ;
				ref_value = min_value + floor((remainder-min_value)*14./(max_value-min_value)) * (max_value-min_value) / 14. ;
				remainder -= ref_value ;
				cartesian_n[k] = (short int)((ref_value-max_value)*14/(min_value-max_value)) ;
				if( cartesian_n[k] < 1 )  cartesian_n[k] = 1 ;
				if( cartesian_n[k] > 14 )  cartesian_n[k] = 14 ;
			    }
			    else
				ref_value = 0. ;
			}

		    memset(&header, 0, sizeof(struct Map_header) ) ;
		    header.type = 28 ;  header.prod_id = 1 ;
		    header.time = hour * 100 + minute ;
		    header.hour = hour ; header.minute = minute; header.second = sec ;
		    header.day = day ; header.month = month ; header.year = year ;
		    header.ires = 1 ;
		    header.id_radar = 1 ;
		    strncpy(header.radar_source, "SMcG\0", 4 ) ;
		    strncpy(header.prod_name, "N_FIELD \0", 8 ) ;
		    strncpy(header.units_name, "n-1 ppm \0", 8 ) ;
		    sprintf(header.prod_pars, "Refractivite brute") ;
		    header.numb_int_levels = 16 ;
		    header.data_in_corners = FALSE ;
		    header.high_res_flag = 1 ;
		    header.map_resol = CartesianResol / 1000. ;
		    header.row_dim = CartesianX ; header.col_dim = CartesianY ;
		    header.group_type = 1 ;
		    header.metprm = 10 ;
		    for( j = 1 ; j < 16 ; j++ ) {
			header.int_thresholds[j] = max_value + (min_value-max_value)*(j-1)/14 ;
			header.cov_km2[j] = 0 ;
		    }
		    fp = fopen( McGillOutputN, "wb" ) ;
		    fwrite( &header, 1024, 1, fp ) ;
		    for( k = 0 ; k < CartesianY*CartesianX ; k += CartesianX )
			fwrite( &cartesian_n[k], sizeof(short int), 240, fp ) ;
		    fclose( fp ) ;
		    free( cartesian_n ) ;
		}
	    }
	}

/* Save product for McGill PPS:  debug "full N field" product (@@@@) */
	generate_full_n_prod(valid) ;

#if( SPOL_VIRAQ_WRITE_SWEEP )
	write_data_spol( RealFileName, DestinationPath ) ;
#endif

/* Generate, if required, some web output products */


/* Save special status information (@@@@: Tailored for McGill uses) */
	if( (OUTFIT == MCGILL) && (RealTimeMode != FALSE) ) {
	    fp = fopen( "bridge_status.txt", "wt" ) ;
	    fprintf( fp, "%d,%d\n", -bridge_detected, -2-az_cor ) ;
	    if( bridge_detected == TRUE )
		fprintf( fp, "The bridge was found at azimuth %d (instead of 263.0)\n", 265+az_cor ) ;
	    else
		fprintf( fp, "Bridge not found.  Old azimuth (%d) assumed\n", 265+az_cor ) ;
	    fclose( fp ) ;
	    fp = fopen( "bridge_log.txt","at" ) ;
	    fprintf( fp, "%4d %02d %02d  %02d %02d  %d %d\n", year, month, day, hour, minute, -bridge_detected, -2-az_cor ) ;
	    fclose( fp ) ;

/* Send the products generated to the outside world (@@@@) */
	    if( valid )
		system("/usr/people/frederic/ground/ftp_to_z >nul &") ;
	}

}


/**************************************************************************/

/* generate_full_n_prod() : Generates a McGill PPS product (480*480 array)
   that contains the current N map, n-map difference,
   h-hr history, and some computations using surface observations.
   Input: N (and maybe DeltaN or edge map) field at current time
   Output: The product in question in a file
   Called by: generate_products() */

void generate_full_n_prod(int valid)
{
	int		T_SCALE = FALSE, TD_SCALE = TRUE ;
	int		ProdX = 720, ProdY = 480, PosZV = 480 ;
	int		i, j, k, x, old_x, y, day_indx, col, tmp_i, path_count ;
	char		tmp_str[1000] ;
	short int	*map_array, k_short ;
	struct Map_header header ;
	struct NFont	font ;
	float		min_value, max_value, n_value, angle ;
	float		scale_t, scale_n_dry, scale_n_wet, scale_step, scale ;
	float		td, vapor, tmp_f ;
	time_t		time_requested, t_diff ;

	time_t		tsec;
	struct tm	*curTime;
	float		*velo, *refl ;
	int		r, polar_indx ;
	static int	old_joss_hour ;
	float		range, azimuth ;

	static time_t	time_prev_scan ;
	int		o_yr, o_mo, o_da, o_hr, o_mn ;
	struct tm	*req_time, res_time ;
	static float	p, t, st, e, se, old_p, old_t, old_e ;
	static int	old_hour, cur_hour, n_scale_mode ;
	static float	n_dry, s_n_dry, n_wet, s_n_wet, n_val ;
	static float	scale_t_indx, scale_e_indx, dn_dry, dn_wet ;
	float		s_n_from_t, s_n_from_e ;
	float		td_set[16] = {-999., -20., -10., -4., 0., 4., 7., 10.,
				     12., 14., 16., 18., 20., 22., 24., 26. } ;
	char		col_map[16][3] ;
	char		fullname[PATH_MAX];
	static int	img_count = 0 ;

/* Load the font and set-up destination array */
	fp = fopen( FontName, "rb" ) ;
	if (fp == NULL) {
	    error_out("Can't open font file", 7);
	}

	fread( &font, sizeof(struct NFont), 1, fp ) ;
	font.size_x = 7 ;
	font.size_y = 12 ;
	fclose( fp ) ;

	map_array = (short int *) calloc( ProdX * ProdY, sizeof(short int) ) ;
	if( map_array == NULL )  return ;

/* Fill header information */
	if( OUTFIT == MCGILL ) {
	    ProdX = 480 ;
	    memset(&header, 0, sizeof(struct Map_header) ) ;
	    header.type = 28 ;  header.prod_id = 2;
	    header.time = hour * 100 + minute ;
	    header.hour = hour ; header.minute = minute; header.second = sec ;
	    header.day = day ; header.month = month ; header.year = year ;
	    header.ires = 1 ;
	    header.id_radar = 1 ;
	    strncpy(header.radar_source, "SMcG\0", 4 ) ;
	    strncpy(header.prod_name, "N_FIELD\0", 8 ) ;
	    strncpy(header.units_name, "N unit\0", 8 ) ;
	    sprintf(header.prod_pars, "Avec info surface") ;
	    header.numb_int_levels = 16 ;
	    header.data_in_corners = TRUE ;
	    header.high_res_flag = 1 ; header.map_resol = 0.5 ;
	    header.row_dim = ProdX ; header.col_dim = ProdY ;
	    header.group_type = 1 ;
	    header.metprm = 11 ;

	    day_indx = month * 100 + day ;	/* Date-dependent scale */
	    if( (day_indx > 1200) || (day_indx <= 315) ) {
		min_value = MinNWinter ;
		max_value = MaxNWinter ;
	    }
	    else if( day_indx < 600 ) {
		min_value = MinNSpring ;
		max_value = MaxNSpring ;
	    }
	    else if( day_indx < 900 ) {
		min_value = MinNSummer ;
		max_value = MaxNSummer ;
	    }
	    else {
		min_value = MinNFall ;
		max_value = MaxNFall ;
	    }
	}

	else if( OUTFIT == NCAR ) {
	    min_value = MinNDisplay ;
	    max_value = MaxNDisplay ;
	}
	header.int_thresholds[0] = 0. ;
	for( j = 1 ; j < 16 ; j++ ) {
	    header.int_thresholds[j] = max_value + (min_value-max_value)*(j-1)/14 ;
	    header.cov_km2[j] = (float)(8 - j) ;
	}

/* Plot the current N field map */
	sprintf( tmp_str, "%02d-%s-%4d %02d:%02dZ\n", day, mon[month-1], year, hour, minute ) ;
	write_text(361-font.size_x*9, 228, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	write_text(360-font.size_x*9, 228, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	if( valid ) {
	    for( y = 10 ; y < CartesianY ; y++ )
		for( x = 0, k = (y+240-10)*ProdX+240 ; x < CartesianX ; x++, k++ ) {
		    n_value = n_array_xy[y*CartesianX+x] ;
		    if( (n_value > 0.) && (n_er_array_xy[y*CartesianX+x] <= NErrorThresh) ) {
			col = 1 + (int)(floor((n_value-max_value) * 14. / (min_value-max_value))) ;
			if( col < 1 )  col = 1 ;
			if( col > 14 )  col = 14 ;
			if( k < ProdX*ProdY )
			    map_array[k] = col ;
		    }
		}
	}

	write_text(361-font.size_x*12, 242, "CURRENT REFRACTIVITY MAP", &font, 15, -1, map_array, ProdX, ProdY) ;

/* And its scale */
	for( x = 300 ; x <= 420 ; x++ ) {
	    n_value = min_value + (x-300)*(max_value-min_value)/(420-300) ;
	    col = 1 + (int)(floor((n_value-max_value) * 14. / (min_value-max_value))) ;
	    if( col < 1 )  col = 1 ;
	    if( col > 14 )  col = 14 ;
	    for( y = 450 ; y <= 460 ; y++ )
		map_array[y*ProdX+x] = col ;
	}
	for( k = 450*ProdX+300 ; k < 450*ProdX+420 ; k++ )
	    map_array[k] = 15 ;
	for( ; k < 461*ProdX+420 ; k += ProdX )
	    map_array[k] = 15 ;
	for( ; k >= 461*ProdX+300 ; k-- )
	    map_array[k] = 15 ;
	for( ; k >= 450*ProdX+299 ; k -= ProdX )
	    map_array[k] = 15 ;
	sprintf( tmp_str, "%03d", (int)min_value ) ;
	write_text(301-(int)(font.size_x*1.5), 437, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	map_array[449*ProdX+300] = 15;  map_array[448*ProdX+300] = 15;
	sprintf( tmp_str, "%03d", (int)((11*min_value+3*max_value)/14) ) ;
	write_text(326-(int)(font.size_x*1.5), 437, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	map_array[449*ProdX+325] = 15;  map_array[448*ProdX+325] = 15;
	sprintf( tmp_str, "%03d", (int)((min_value+max_value)/2) ) ;
	write_text(361-(int)(font.size_x*1.5), 437, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	map_array[449*ProdX+360] = 15;  map_array[448*ProdX+360] = 15;
	sprintf( tmp_str, "%03d", (int)((3*min_value+11*max_value)/14) ) ;
	write_text(396-(int)(font.size_x*1.5), 437, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	map_array[449*ProdX+395] = 15;  map_array[448*ProdX+395] = 15;
	sprintf( tmp_str, "%03d", (int)max_value ) ;
	write_text(421-(int)(font.size_x*1.5), 437, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	map_array[449*ProdX+420] = 15;  map_array[448*ProdX+420] = 15;

/* Plot the N-scan trend map */
	if( DoMapDiff == TRUE ) {
	    for( y = 10 ; y < CartesianY ; y++ )
		for( x = 0, k = (y-10)*ProdX+240 ; x < CartesianX ; x++, k++ ) {
		    n_value = dn_array_xy[y*CartesianX+x] ;
		    if( (n_value > VALID_MIN) && (dn_er_array_xy[y*CartesianX+x] <= DNErrorThresh) ) {
			col = (int)(floor( 8. - n_value )) ;
			if( col < 1 )  col = 1 ;
			if( col > 14 )  col = 14 ;
			if( k < ProdX*ProdY )
			    map_array[k] = col ;
		    }
		}
	}

	if( OUTFIT == MCGILL ) 
	    sprintf(tmp_str, "%d-MIN REFRACTIVITY TREND", NumMapDiff*5) ;
	else
	    sprintf(tmp_str, "%d-SCAN REFRACTIVITY CHANGE", NumMapDiff) ;
	write_text(361-(int)(font.size_x*12.5), 2, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;

/* And its scale */
	for( x = 300 ; x <= 420 ; x++ ) {
	    n_value = -7.5 + (x-300)*15./(420-300) ;
	    col = (int)(floor( 8. - n_value )) ;
	    if( col < 1 )  col = 1 ;
	    if( col > 14 )  col = 14 ;
	    for( y = 210 ; y <= 220 ; y++ )
		map_array[y*ProdX+x] = col ;
	}
	for( k = 210*ProdX+300 ; k < 210*ProdX+420 ; k++ )
	    map_array[k] = 15 ;
	for( ; k < 221*ProdX+420 ; k += ProdX )
	    map_array[k] = 15 ;
	for( ; k >= 221*ProdX+300 ; k-- )
	    map_array[k] = 15 ;
	for( ; k >= 210*ProdX+299 ; k -= ProdX )
	    map_array[k] = 15 ;
	write_text(301-(int)(font.size_x*2.5), 197, "-7.5", &font, 15, -1, map_array, ProdX, ProdY) ;
	map_array[209*ProdX+300] = 15;  map_array[208*ProdX+300] = 15;
	write_text(337-(int)(font.size_x*1.5), 197, "-3", &font, 15, -1, map_array, ProdX, ProdY) ;
	map_array[209*ProdX+336] = 15;  map_array[208*ProdX+336] = 15;
	write_text(361-(int)(font.size_x*.5), 197, "0", &font, 15, -1, map_array, ProdX, ProdY) ;
	map_array[209*ProdX+360] = 15;  map_array[208*ProdX+360] = 15;
	write_text(385-(int)(font.size_x*.5), 197, "3", &font, 15, -1, map_array, ProdX, ProdY) ;
	map_array[209*ProdX+384] = 15;  map_array[208*ProdX+384] = 15;
	write_text(421-(int)(font.size_x*1.5), 197, "7.5", &font, 15, -1, map_array, ProdX, ProdY) ;
	map_array[209*ProdX+420] = 15;  map_array[208*ProdX+420] = 15;
	write_text(290-font.size_x*5, 203, "Drier", &font, 15, -1, map_array, ProdX, ProdY) ;
	write_text(290-font.size_x*6, 215, "Warmer", &font, 15, -1, map_array, ProdX, ProdY) ;
	write_text(430, 203, "Wetter", &font, 15, -1, map_array, ProdX, ProdY) ;
	write_text(430, 215, "Colder", &font, 15, -1, map_array, ProdX, ProdY) ;

	if( OUTFIT == NCAR ) {
/* Add reflectivity and velocity */
	    velo = (float *) malloc( NumAzim * NumRangeBins * sizeof( float ) ) ;
	    refl = (float *) malloc( NumAzim * NumRangeBins * sizeof( float ) ) ;
	    if( refl == NULL ) {
		sprintf( tmp_str, "Out of memory allocating arrays in generate_full_n_prod") ;
        	error_out( tmp_str, 3 ) ;
            }

	    fp = fopen( "/tmp/Z_V.tmp", "rb" ) ;
	    for( k = 0 ; k < NumRangeBins * NumAzim ; k++ ) {
		fread( &refl[k], sizeof(float), 1, fp ) ;
		fread( &velo[k], sizeof(float), 1, fp ) ;
	    }
	    fclose( fp ) ;
	    for( y = 5 ; y < 230 ; y++ )	/* For each pixel, get polar coord */
		for( x = 0, k = y*ProdX+PosZV ; x < 240 ; x++, k++ ) {
		    range = sqrt( SQR(y-(CartesianY-21)/2.) + SQR(x-(CartesianX-1)/2.) ) * CartesianResol ;
		    r = (int)(range / GateSpacing) ;
		    if( (r < NumRangeBins) ) {  /* && (range <= 61000.) ) { */
			azimuth = atan2( (x-(CartesianX-1)/2.), (CartesianY-21)/2.-y ) / DEGTORAD ;
			if( azimuth < 0. )  azimuth += 360. ;
			polar_indx = (int)(azimuth / 360. * NumAzim ) ;
			if( polar_indx >= NumAzim )  polar_indx = NumAzim - 1 ;
			polar_indx = polar_indx * NumRangeBins + r ;
			col = (int)(floor( (refl[polar_indx]+25.) / 5. )) ;
			if( col > 14 )
			    col = 0 ;
			else if( col < 1. )
			    col = 15 ;
			map_array[k] = col ;
			col = (int)(floor( (velo[polar_indx]+32.) / 4. )) ;
			if( col < 1 )  col = 1 ;
			if( col > 14 )  col = 14 ;
			map_array[k+240*ProdX] = col ;
		    }
		}

/* And their scale */
	    for( x = PosZV+60 ; x <= PosZV+180 ; x++ ) {
		n_value = -20. + 70./120. * (x-PosZV-60+.5) ;
		col = (int)((n_value+25) / 5.);
		if( col > 14 )
		    col = 0 ;
		else if( col < 1 )
		    col = 15 ;
		for( y = 210 ; y <= 220 ; y++ )
		    map_array[y*ProdX+x] = col ;
		n_value = -28. + 56./120. * (x-PosZV-60+.5) ;
		col = (int)(floor( (n_value+32.) / 4. )) ;
		if( col < 1 )  col = 1 ;
		if( col > 14 )  col = 14 ;
		for( y = 450 ; y <= 460 ; y++ )
		    map_array[y*ProdX+x] = col ;
	    }
	    for( k = 210*ProdX+60+PosZV ; k < 210*ProdX+180+PosZV ; k++ )
		map_array[k] = 15 ;
	    for( ; k < 221*ProdX+PosZV+180 ; k += ProdX )
		map_array[k] = 15 ;
	    for( ; k >= 221*ProdX+PosZV+60 ; k-- )
		map_array[k] = 15 ;
	    for( ; k >= 210*ProdX+PosZV+59 ; k -= ProdX )
		map_array[k] = 15 ;

	    for( k = 450*ProdX+PosZV+60 ; k < 450*ProdX+PosZV+180 ; k++ )
		map_array[k] = 15 ;
	    for( ; k < 461*ProdX+PosZV+180 ; k += ProdX )
		map_array[k] = 15 ;
	    for( ; k >= 461*ProdX+PosZV+60 ; k-- )
		map_array[k] = 15 ;
	    for( ; k >= 450*ProdX+PosZV+59 ; k -= ProdX )
		map_array[k] = 15 ;
	
	    write_text(PosZV+61-(int)(font.size_x*1.5), 224, "-20", &font, 15, -1, map_array, ProdX, ProdY) ;
	    map_array[222*ProdX+PosZV+60] = 15;  map_array[223*ProdX+PosZV+60] = 15;
	    write_text(PosZV+94-(int)(font.size_x*.5), 224, "0", &font, 15, -1, map_array, ProdX, ProdY) ;
	    map_array[222*ProdX+PosZV+94] = 15;  map_array[223*ProdX+PosZV+94] = 15;
	    write_text(PosZV+128-(int)(font.size_x*1.), 224, "20", &font, 15, -1, map_array, ProdX, ProdY) ;
	    map_array[222*ProdX+PosZV+128] = 15;  map_array[223*ProdX+PosZV+128] = 15;
	    write_text(PosZV+163-(int)(font.size_x*1.), 224, "40", &font, 15, -1, map_array, ProdX, ProdY) ;
	    map_array[222*ProdX+PosZV+163] = 15;  map_array[223*ProdX+PosZV+163] = 15;
	    write_text(PosZV+188, 211, "dBZ", &font, 15, -1, map_array, ProdX, ProdY) ;
	    write_text(PosZV+77, 2, "REFLECTIVITY", &font, 0, -1, map_array, ProdX, ProdY) ;
	    write_text(PosZV+78, 1, "REFLECTIVITY", &font, 15, -1, map_array, ProdX, ProdY) ;

	    write_text(PosZV+69-(int)(font.size_x*1.5), 464, "-24", &font, 15, -1, map_array, ProdX, ProdY) ;
	    map_array[462*ProdX+PosZV+68] = 15;  map_array[463*ProdX+PosZV+68] = 15;
	    write_text(PosZV+95-(int)(font.size_x*1.5), 464, "-12", &font, 15, -1, map_array, ProdX, ProdY) ;
	    map_array[462*ProdX+PosZV+94] = 15;  map_array[463*ProdX+PosZV+94] = 15;
	    write_text(PosZV+120-(int)(font.size_x*.5), 464, "0", &font, 15, -1, map_array, ProdX, ProdY) ;
	    map_array[462*ProdX+PosZV+120] = 15;  map_array[463*ProdX+PosZV+120] = 15;
	    write_text(PosZV+146-(int)(font.size_x*1.), 464, "12", &font, 15, -1, map_array, ProdX, ProdY) ;
	    map_array[462*ProdX+PosZV+146] = 15;  map_array[463*ProdX+PosZV+146] = 15;
	    write_text(PosZV+171-(int)(font.size_x*1.), 464, "24", &font, 15, -1, map_array, ProdX, ProdY) ;
	    map_array[462*ProdX+PosZV+171] = 15;  map_array[463*ProdX+PosZV+171] = 15;
	    write_text(PosZV+188, 451, "m/s", &font, 15, -1, map_array, ProdX, ProdY) ;
	    write_text(PosZV+91, 242, "VELOCITY", &font, 0, -1, map_array, ProdX, ProdY) ;
	    write_text(PosZV+92, 241, "VELOCITY", &font, 15, -1, map_array, ProdX, ProdY) ;
	    free( refl ) ;
	    free( velo ) ;
	}

/* Add geographical overlay to all maps */
	fp = fopen( GeogOverlay, "rb" ) ;
	if( fp != NULL ) {
	    do {
		fread( &x, sizeof(int), 1, fp ) ;
		j = fread( &y, sizeof(int), 1, fp ) ;
		if( (j != 0) && (y >= 20+10 ) && (y < 250) && (x < 480) ) {
		    map_array[(y-10)*ProdX+x] = 15 ;
		    map_array[(y+240-10)*ProdX+x] = 15 ;
		    if( OUTFIT == NCAR ) {
			map_array[(y-10)*ProdX+x+PosZV-240] = 15 ;
			map_array[(y+240-10)*ProdX+x+PosZV-240] = 15 ;
		    }
		}
	    } while( j != 0 ) ;
	    fclose( fp ) ;
	}
	else {
	    for( j = 20000 ; j <= 60000 ; j += 20000 )
		for( angle = 0. ; angle < 360. ; angle += 100000. / j ) {
		    y = (int)(110 + (j / CartesianResol) * cos(angle*DEGTORAD));
		    x = (int)(360 + (j / CartesianResol) * sin(angle*DEGTORAD));
		    if( (y > 0) && (y < 230) && (x >= 240) && (x < 480) ) {
			map_array[y*ProdX+x] = 15 ;
			map_array[(y+240)*ProdX+x] = 15 ;
			if( OUTFIT == NCAR ) {
			    map_array[y*ProdX+x+PosZV-240] = 15 ;
			    map_array[(y+240)*ProdX+x+PosZV-240] = 15 ;
			    if( (angle == 90.) && (j == 60000) ) {
				write_text(x-7, y-12, "60", &font, 15, -1, map_array, ProdX, ProdY) ;
				write_text(x-7, y-12+240, "60", &font, 15, -1, map_array, ProdX, ProdY) ;
			    }
			}
		    }
		}
	    for( angle = 0. ; angle < 360 ; angle += 30. )
		for (j = 2000 ; j <= 60000 ; j += 2000 ) {
		    y = (int)(110 + (j / CartesianResol) * cos(angle*DEGTORAD));
		    x = (int)(360 + (j / CartesianResol) * sin(angle*DEGTORAD));
		    if( (y > 0) && (y < 230) && (x >= 240) && (x < 480) ) {
			map_array[y*ProdX+x] = 15 ;
			map_array[(y+240)*ProdX+x] = 15 ;
			if( OUTFIT == NCAR ) {
			    map_array[y*ProdX+x+PosZV-240] = 15 ;
			    map_array[(y+240)*ProdX+x+PosZV-240] = 15 ;
			}
		    }
		}
	}

/* Plot 12-hr history graph */
	sprintf(tmp_str, "%d-HR HISTORY OF MEAN N", DurationHistory) ;
	write_text(120-(int)(font.size_x*strlen(tmp_str)/2), 242, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	for( i = MIN_N_DISPLAY ; i <= MAX_N_DISPLAY ; i += N_STEP_DISPLAY ) {
	    k = (int) (464 - 2 * (i - MinNTrace)) ;
	    if( (k >= 270) && (k <= 470) ) {
		sprintf(tmp_str, "%03d -", i) ;
		write_text(50-font.size_x*5, k, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
		if( k < 450 ) {
		    sprintf(tmp_str, "- %03d", i) ;
		    write_text(192, k, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
		}
	    }
	}
	sprintf( tmp_str, "%02d:%02d", hour, minute ) ;
	write_text(175, 258, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	sprintf( tmp_str, "%02d:%02d", (hour + 24 - DurationHistory) % 24, minute ) ;
	write_text(31, 258, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	for( y = 270, k = 270 * ProdX + 48 ; y <= 470 ; y++, k+= ProdX )
	    map_array[k] = 15 ;
	for( x = 48, k = 470 * ProdX + 48 ; x < 192 ; x++, k++ )
	    map_array[k] = 15 ;
	for( y = 470 ; y >= 270 ; y--, k-= ProdX )
	    map_array[k] = 15 ;
	x = 48 + (62-minute) / 5 ;
	for( ; x < 192 ; x+= 12 )
	    for( y = 280 ; y < 470 ; y += 10 )
		map_array[y*ProdX+x] = 15 ;

	/* Read the old N history */
	if( (RealTimeMode != FALSE) && (just_started == TRUE) ) {
	    sprintf( tmp_str, "tail -150 status_debug.txt > %s", TmpFile ) ;
	    system( tmp_str ) ;
	    fp = fopen( TmpFile, "rt" ) ;
	    req_time = &res_time ;
	    time_prev_scan = time_cur_scan ;
	    do {
 //		k = fscanf( fp, "%d%d%d%d%d%d%d%f%f%f%f%f", &o_yr, &o_mo, &o_da, &o_hr, &o_mn, &tmp_i, &old_av_n, &tmp_f, &tmp_f, &tmp_f, &tmp_f ) ;
		k = fscanf( fp, "%d%d%d%d%d%d%f%f%f%f%f", &o_yr, &o_mo, &o_da, &o_hr, &o_mn, &tmp_i, &old_av_n, &tmp_f, &tmp_f, &tmp_f, &tmp_f ) ;
		if( k == 12 ) {
		    res_time.tm_year = o_yr - 1900 ;
		    res_time.tm_mon = o_mo - 1 ;
		    res_time.tm_mday = o_da ;
		    res_time.tm_hour = o_hr ;
		    res_time.tm_min = o_mn ;
		    res_time.tm_sec = 0 ;
		    res_time.tm_isdst = 0 ;
		    if( OUTFIT == MCGILL )
			time_requested = mktime( req_time ) - 5 * 3600 ;
		    else
			time_requested = mktime( req_time ) - 6 * 3600 ; /* @@@@ TZ dependant */
		    t_diff = time( NULL ) - time_requested - (DurationHistory*25) ; /* @@@@ */
		    if( (k != 0) && (t_diff > 0) && (t_diff < DurationHistory*3600) ) {
			j = 144 - t_diff / (DurationHistory*25) ;
			if( (j >= 0) && (j<144) )
			    n_history[j] = old_av_n ;
		    time_prev_scan = time_requested ;
		    }
		}
	    } while( k == 12 ) ;
	    fclose( fp ) ;
	    unlink( TmpFile ) ;
	}
	else if( just_started == TRUE )
	    time_prev_scan = time_cur_scan ;

	k = (time_cur_scan / (DurationHistory*25)) -  (time_prev_scan / (DurationHistory*25)) ;
	if( k > 0 ) {
	    for( j = 0 ; j < 144-k ; j++ )
		n_history[j] = n_history[j+k] ;
	    for( ; j < 144 ; j++ )
		n_history[j] = 0. ;
	    n_history[143] = new_av_n ;
	}
	time_prev_scan += (DurationHistory*25) * k ;

	for( j = 0 ; j < 144 ; j++ ) {
	    k = (int)(floor( 2 * n_history[j] + .5 )) ;
	    if( k < 2*MinNTrace-10 )  k = (int)(2*MinNTrace-10) ;
	    if( k > 2*MaxNTrace+10 )  k = (int)(2*MaxNTrace+10) ;
	    k = (470 - (int)(k-2*MinNTrace)) * ProdX + 48 + j ;
	    while( k < 0 )  k += ProdX ;
	    while( k >= ProdX*ProdY )  k -= ProdX ;
	    if( n_history[j] != 0. )  map_array[k] = 6 ;
	}

/* Write (and, if necessary, get) station-related information */
	if( (OUTFIT == MCGILL) || !strcmp(SubVersion, "IHOP") ) {
	    write_text(120-(int)(font.size_x*12.5), 2, "FROM SURFACE OBSERVATIONS", &font, 15, -1, map_array, ProdX, ProdY) ;
	    if( RealTimeMode != FALSE ) {
		time_requested = time( NULL ) - 900 ;
		req_time = gmtime( &time_requested ) ;
	    }
	    else {
		res_time.tm_year = year - 1900 ;
		res_time.tm_mon = month - 1 ;
		res_time.tm_mday = day ;
		if( OUTFIT == MCGILL ) {
		    res_time.tm_hour = hour - 5;
		    res_time.tm_min = minute + 30 ;
		}
		else {
		    res_time.tm_hour = hour ;
		    res_time.tm_min = minute ;
		}
		res_time.tm_sec = 0 ;
		res_time.tm_isdst = 0 ;
		req_time = &res_time ;
		time_requested = mktime( req_time ) ;
		req_time = gmtime( &time_requested ) ;
	    }
	    if( (OUTFIT == MCGILL) && (RealTimeMode == FALSE) )  goto SkipStat ; /* #### Temporary patch */
	    if( (OUTFIT == MCGILL) && (RealTimeMode != FALSE) ) { /* #### Work on non-real time version */
		if( just_started == TRUE ) {
		    time_requested -= 3600 ;
		    req_time = gmtime( &time_requested ) ;
		    cur_hour = req_time->tm_hour ;
		    get_station( req_time, &p, &t, &st, &e, &se ) ;
		    n_dry = 77.6 * p * exp( -AvTargetHeight * 9.81 / 287. / (273.16+t) ) / (273.16+t) ;
		    n_wet = 373000. * e / SQR(273.16+t) ;
		    s_n_from_t = sqrt( SQR( 77.6 * p * exp( -AvTargetHeight * 9.81 / 287. / (273.16+t) ) / SQR(273.16+t) * st) + SQR(746000. * e / pow(273.16+t, 3) * st) ) ;
		    if( (p == INVALID) || (e == INVALID) || (st == INVALID) )
			scale_t_indx = 0. ;
		    else
			scale_t_indx = .5 * s_n_from_t ;
		    s_n_from_e = 373000. / SQR(273.16+t) * se ;
		    if( (t == INVALID) || (se == INVALID ))
			scale_e_indx = 0. ;
		    else
			scale_e_indx = .5 * s_n_from_e ;
		    time_requested += 3600 ;
		    req_time = gmtime( &time_requested ) ;
		    n_scale_mode = T_SCALE ;
		}
		if( cur_hour != req_time->tm_hour ) {
		    old_hour = cur_hour ;
		    cur_hour = req_time->tm_hour ;
		    old_p = p ;  old_t = t ;  old_e = e ;
		    p = INVALID ;  t = INVALID ;  e = INVALID ;
		}
	    }
	    else {
		old_p = p ;  old_t = t ;  old_e = e ;
		p = INVALID ;  t = INVALID ;  e = INVALID ;
		old_hour = cur_hour ;
		cur_hour = (int)((time_requested % 86400) / 60) ;
		cur_hour += 40 * (cur_hour / 60) ;
	    }

	    if( (p == INVALID) || (t == INVALID) || (e == INVALID) ) {
		get_station( req_time, &p, &t, &st, &e, &se ) ;
		if( OUTFIT == NCAR )
		    if( just_started == TRUE ) {
			old_p = p ;  old_t = t ;  old_e = e ; old_hour = cur_hour ;
		    }
		n_val = n_dry ;
		n_dry = 77.6 * p * exp( -AvTargetHeight * 9.81 / 287. / (273.16+t) ) / (273.16+t) ;
		if( (p == INVALID) || (t == INVALID) )
		    n_dry = INVALID ;
		dn_dry = (n_dry - n_val) ;
		if( (n_dry == INVALID) || (n_val == INVALID) )
		    dn_dry = INVALID ;
		s_n_dry = 77.6 * p * exp( -AvTargetHeight * 9.81 / 287. / (273.16+t) ) / SQR(273.16+t) * st ;
		if( (n_dry == INVALID) || (st == INVALID) )
		    s_n_dry = INVALID ;
		n_val = n_wet ;
		n_wet = 373000. * e / SQR(273.16+t) ;
		if( (e == INVALID) || (t == INVALID) )
		    n_wet = INVALID ;
		dn_wet = (n_wet - n_val) ;
		if( (n_wet == INVALID) || (n_val == INVALID) )
		    dn_wet = INVALID ;
		s_n_wet = sqrt( SQR(746000. * e / pow(273.16+t, 3) * st) + SQR(373000. / SQR(273.16+t) * se) ) ;
		if( (st == INVALID) || (se == INVALID) )
		    s_n_wet = INVALID ;
		s_n_from_t = sqrt( SQR(s_n_dry) + SQR(746000. * e / pow(273.16+t, 3) * st) ) ;
		if( (s_n_dry == INVALID) || (st == INVALID) || (e == INVALID) ) {
		    s_n_from_t = INVALID ;
		    scale_t_indx *= .8 ;
		}
		else
		    scale_t_indx = .6 * scale_t_indx + .4 * s_n_from_t ;
		s_n_from_e = 373000. / SQR(273.16+t) * se ;
		if( (st == INVALID) || (se == INVALID ) ) {
		    s_n_from_e = INVALID ;
		    scale_e_indx *= .8 ;
		}
		else
		    scale_e_indx = .6 * scale_e_indx + .4 * s_n_from_e ;
		if( scale_t_indx > scale_e_indx )
		    n_scale_mode = T_SCALE ;
		else
		    n_scale_mode = TD_SCALE ;
	    }
	    if( OUTFIT == MCGILL )
		sprintf( tmp_str, "%02d00Z: ", old_hour ) ;
	    else
		sprintf( tmp_str, "%04dZ: ", old_hour ) ;
	    if( old_p != INVALID )
		sprintf( &tmp_str[7], "P=%6.1f ", old_p ) ;
	    else
		sprintf( &tmp_str[7], "P= N/A   " ) ;
	    if( old_t != INVALID )
		sprintf( &tmp_str[16], "T=%+4.1f  ", old_t ) ;
	    else
		sprintf( &tmp_str[16], "T= N/A   " ) ;
	    if( old_e != INVALID ) {
		td = 0. ;
		vapor = 6.112 * exp(17.67 * td / ( td + 243.5 )) ;
		do {
		    td = td + log( old_e / vapor ) / log(2.) * 11. ;
		    vapor = 6.112 * exp(17.67 * td / ( td + 243.5 )) ;
		} while (fabs(old_e-vapor) > .0001 * old_e) ;
		sprintf( &tmp_str[24], "Td=%+4.1f", td ) ;
	    }
	    else
		sprintf( &tmp_str[24], "Td= N/A" ) ;
/*	sprintf( tmp_str, "%02d00Z: P=%6.1f T=%+4.1f e=%4.2f", old_hour, old_p, old_t, old_e ) ; */
	    write_text(10, 28, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	    if( OUTFIT == MCGILL )
		sprintf( tmp_str, "%02d00Z: ", cur_hour ) ;
	    else
		sprintf( tmp_str, "%04dZ: ", cur_hour ) ;
	    if( p != INVALID )
		sprintf( &tmp_str[7], "P=%6.1f ", p ) ;
	    else
		sprintf( &tmp_str[7], "P= N/A   " ) ;
	    if( t != INVALID )
		sprintf( &tmp_str[16], "T=%+4.1f  ", t ) ;
	    else
		sprintf( &tmp_str[16], "T= N/A   " ) ;
	    if( e != INVALID ) {
		td = 0. ;
		vapor = 6.112 * exp(17.67 * td / ( td + 243.5 )) ;
		do {
		    td = td + log( e / vapor ) / log(2.) * 11. ;
		    vapor = 6.112 * exp(17.67 * td / ( td + 243.5 )) ;
		} while (fabs(e-vapor) > .0001 * e) ;
		sprintf( &tmp_str[24], "Td=%+4.1f", td ) ;
	    }
	    else
		sprintf( &tmp_str[24], "Td= N/A" ) ;
	    write_text(10, 42, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	    write_text(0, 66, "Theory: N = 77.6 P/T + 373000 e/T\261", &font, 15, -1, map_array, ProdX, ProdY) ;
	    write_text(58, 82, "Density term    Wet term", &font, 15, -1, map_array, ProdX, ProdY) ;
	    if( OUTFIT == MCGILL )
		sprintf( tmp_str, "%02d00Z:   ", cur_hour ) ;
	    else
		sprintf( tmp_str, "%04dZ:   ", cur_hour ) ;
	    if( n_dry != INVALID )
		sprintf( &tmp_str[9], "%5.1f ", n_dry ) ;
	    else
		sprintf( &tmp_str[9], " N/A  " ) ;
	    if( s_n_dry != INVALID )
		sprintf( &tmp_str[15], "\260%4.1f  ", s_n_dry ) ;
	    else
		sprintf( &tmp_str[15], "\260 N/A  " ) ;
	    if( n_wet != INVALID )
		sprintf( &tmp_str[22], "%5.1f ", n_wet ) ;
	    else
		sprintf( &tmp_str[22], " N/A  " ) ;
	    if( s_n_wet != INVALID )
		sprintf( &tmp_str[28], "\260%4.1f", s_n_wet ) ;
	    else
		sprintf( &tmp_str[28], "\260 N/A" ) ;
	    write_text(1, 100, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	    if( OUTFIT == MCGILL )
		sprintf( tmp_str, "1-hr change:   " ) ;
	    else
		sprintf( tmp_str, "1-scan change: " ) ;
	    if( dn_dry != INVALID )
		sprintf( &tmp_str[15], "%+5.1f        ", dn_dry ) ;
	    else
		sprintf( &tmp_str[15], "  N/A         " ) ;
	    if( dn_wet != INVALID )
		sprintf( &tmp_str[28], "%+5.1f", dn_wet ) ;
	    else
		sprintf( &tmp_str[28], "  N/A" ) ;
	    write_text(1, 118, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	    if( (n_dry != INVALID) && (n_wet != INVALID) ) {
		if( OUTFIT == MCGILL )
		    sprintf( tmp_str, "Expected N at %02d00Z:  %5.1f ", cur_hour, n_dry+n_wet ) ;
		else
		    sprintf( tmp_str, "Expected N at %04dZ:  %5.1f %+5.1f ", cur_hour, n_dry+n_wet, dn_dry+dn_wet ) ;
		if( (s_n_dry != INVALID) && (s_n_wet != INVALID) )
		    sprintf( &tmp_str[28], "\260%4.1f", sqrt(SQR(s_n_dry)+SQR(s_n_wet)) ) ;
		write_text(1, 145, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	    }
SkipStat:
	    if( valid ) {
		sprintf( tmp_str, "Observed N at %02d%02dZ:  %5.1f ", hour, minute, new_av_n ) ;
		sprintf( &tmp_str[28], "%+6.2f", n_change ) ;
	    }
	    else
		sprintf( tmp_str, "Observed N at %02d%02dZ:   N/A", hour, minute ) ;
	    write_text(1, 163, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;

/* Add the surrogate T or RH scale */
	    if( (t != INVALID) && (p != INVALID) ) {
		if( n_scale_mode == T_SCALE ) {
		    scale = td ;
		    scale_step = 10. ;
		    j = 0 ;
		    for( scale_t = -40. ; scale_t < 40. ; scale_t += 2. ) {
			n_value = 77.6 * p * exp( -AvTargetHeight * 9.81 / 287. / (273.16+t) ) / (273.16+scale_t) + 3.73E5 * 6.112 * exp(17.67 * scale / ( scale + 243.5 )) / SQR(273.16+scale_t) ;
			if( (n_value >= min_value) && (n_value <= max_value) && (scale_t >= scale-10.) ) {
			    x = 300 + (int)((n_value-min_value)*120/(max_value-min_value)) ;
			    map_array[462*ProdX+x] = 15 ;
			    map_array[463*ProdX+x] = 15 ;
			    if( ((int)(scale_t+40.) % 10) == 0 ) {
				map_array[464*ProdX+x] = 15 ;
				sprintf( tmp_str, "%d", (int)scale_t ) ;
				if( scale_t < 0. )  x -= 4 ;	/* #### Looks */
				write_text((int)(x+1-font.size_x*.5*strlen(tmp_str)), 464, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
				j++ ;
			    }
			}
		    }
		    sprintf( tmp_str, "T at Td=%d\257:", (int)floor(scale+.5) ) ;
		    write_text(287-font.size_x*strlen(tmp_str), 463, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
		}
		else {
		    if( (old_t != INVALID) && (RealTimeMode != FALSE) )
			scale_t = floor( 1.5 * t - 0.5 * old_t + 0.5 ) ;
		    else
			scale_t = floor( t + 0.5 ) ;
		    for( scale = -30. ; scale < 25. ; scale += 1. ) {
			if( (scale < 0.) && (((int)scale+100) % 2 == 1) )
			    scale += 1. ;
			n_value = 77.6 * p * exp( -AvTargetHeight * 9.81 / 287. / (273.16+t) ) / (273.16+scale_t) + 3.73E5 * 6.112 * exp(17.67 * scale / ( scale + 243.5 )) / SQR(273.16+scale_t) ;
			if( (n_value >= min_value) && (n_value <= max_value) && (scale < scale_t+5.) ) {
			    x = 300 + (int)((n_value-min_value)*120/(max_value-min_value)) ;
			    map_array[462*ProdX+x] = 15 ;
			    map_array[463*ProdX+x] = 15 ;
			}
		    }
		    for( j = 0, old_x = 250 ; j < 16 ; j++ ) {
			scale = td_set[j] ;
			n_value = 77.6 * p * exp( -AvTargetHeight * 9.81 / 287. / (273.16+t) ) / (273.16+scale_t) ;
			if( j != 0 )
			    n_value += 3.73E5 * 6.112 * exp(17.67 * scale / ( scale + 243.5 )) / SQR(273.16+scale_t) ;
			if( (n_value >= min_value) && (n_value <= max_value) && (td_set[j] < scale_t+5.) ) {
			    x = 300 + (int)((n_value-min_value)*120/(max_value-min_value)) ;
			    map_array[464*ProdX+x] = 15 ;
			    if( j != 0 ) {
				sprintf( tmp_str, "%d", (int)floor(td_set[j]+.5) ) ;
				if( td_set[j] < 0. )  x -= 4 ;  /* #### For looks */
			    }
			    else {
				sprintf( tmp_str, "Dry" ) ;
				map_array[462*ProdX+x] = 15 ;
				map_array[463*ProdX+x] = 15 ;
			    }
			    if( x+1-font.size_x*.5*strlen(tmp_str) > old_x + 2 ) {
				write_text((int)(x+1-font.size_x*.5*strlen(tmp_str)), 464, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
				old_x = x+1+(int)(font.size_x*.5*strlen(tmp_str)) ;
			    }
			}
		    }
		    sprintf( tmp_str, "Td at %d\257C:", (int)scale_t ) ;
		    write_text(287-font.size_x*strlen(tmp_str), 464, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
		}
	    }
	}
	else {
	    if( valid )
		sprintf( tmp_str, "Observed N at %02d%02dZ:  %5.1f", hour, minute, new_av_n ) ;
	    else
		sprintf( tmp_str, "Observed N at %02d%02dZ:   N/A", hour, minute ) ;
	    write_text(1, 163, tmp_str, &font, 15, -1, map_array, ProdX, ProdY) ;
	}

/* Write product to a file */
	fp = fopen( "phases.dat", "wb" ) ;
	fwrite( &header, 1024, 1, fp ) ;
	for( k = 0 ; k < ProdX*ProdY ; k += ProdX )
	    fwrite( &map_array[k], sizeof(short int), ProdX, fp ) ;
	fclose( fp ) ;

	col_map[0][0] = 255 ;  col_map[0][1] = 255 ;  col_map[0][2] = 255 ;
	col_map[1][0] = 0 ;  col_map[1][1] = 255 ;  col_map[1][2] = 255 ;
	col_map[2][0] = 0 ;  col_map[2][1] = 191 ;  col_map[2][2] = 191 ;
	col_map[3][0] = 0 ;  col_map[3][1] = 32 ;  col_map[3][2] = 255 ;
	col_map[4][0] = 0 ;  col_map[4][1] = 0 ;  col_map[4][2] = 150 ;
	col_map[5][0] = 0 ;  col_map[5][1] = 255 ;  col_map[5][2] = 0 ;
	col_map[6][0] = 0 ;  col_map[6][1] = 140 ;  col_map[6][2] = 0 ;
	col_map[7][0] = 0 ;  col_map[7][1] = 80 ;  col_map[7][2] = 0 ;
	col_map[8][0] = 255 ;  col_map[8][1] = 255 ;  col_map[8][2] = 0 ;
	col_map[9][0] = 255 ;  col_map[9][1] = 128 ;  col_map[9][2] = 0 ;
	col_map[10][0] = 255 ;  col_map[10][1] = 0 ;  col_map[10][2] = 0 ;
	col_map[11][0] = 255 ;  col_map[11][1] = 191 ;  col_map[11][2] = 191 ;
	col_map[12][0] = 255 ;  col_map[12][1] = 100 ;  col_map[12][2] = 255 ;
	col_map[13][0] = 191 ;  col_map[13][1] = 0 ;  col_map[13][2] = 191 ;
	col_map[14][0] = 100 ;  col_map[14][1] = 0 ;  col_map[14][2] = 100 ;
	col_map[15][0] = 0 ;  col_map[15][1] = 0 ;  col_map[15][2] = 0 ;
	
	fp = fopen( "n_display.ppm", "wb" ) ;
	fprintf(fp, "P6 %d 480 255 ", ProdX) ;
	for( k = 0 ; k < ProdX*ProdY ; k++ )
	    fprintf(fp, "%c%c%c", col_map[map_array[k]][0], col_map[map_array[k]][1], col_map[map_array[k]][2]) ;
	fclose( fp ) ;
	if( OUTFIT == NCAR ) {
/*	    tsec = time(NULL);
	    curTime = gmtime(&tsec);
*/
	    img_count++ ;
	    img_count = (img_count % 1000) ;
	    sprintf(tmp_str, "convert n_display.ppm %sn_display.%04d%02d%02d%02d%02d%03d.png", DestinationPath, year, month, day, hour, minute, img_count ) ;
/*		curTime->tm_year+1900,
		curTime->tm_mon+1,
		curTime->tm_mday,
		curTime->tm_hour,
		curTime->tm_min,
		curTime->tm_sec);  */
	    system(tmp_str) ;
/* Deceased code to send to JOSS Catalog
//	if( old_joss_hour != curTime->tm_hour ) {
//	    old_joss_hour = curTime->tm_hour ;
//	    fp = fopen( "/tmp/refract.out/send_to_joss", "wt" ) ;
//	    fprintf( fp, "ftp -n ftp.joss.ucar.edu <<   !\n" ) ;
//	    fprintf( fp, "  user anonymous  frederic@radar.mcgill.ca\n" ) ;
//	    fprintf( fp, "  prompt off\n" ) ;
//	    fprintf( fp, "  lcd /tmp/refract.out/\n" ) ;
//	    fprintf( fp, "  cd pub/incoming/catalog/ihop\n") ;
//	    fprintf( fp, "  bin\n") ;
//	    fprintf( fp, "  put n_display.%04d%02d%02d%02d%02d%02d.png research.S-Pol.%04d%02d%02d%02d%02d.Refractivity.png\n",
//		curTime->tm_year+1900,
//		curTime->tm_mon+1,
//		curTime->tm_mday,
//		curTime->tm_hour,
//		curTime->tm_min,
//		curTime->tm_sec,
//		curTime->tm_year+1900,
//		curTime->tm_mon+1,
//		curTime->tm_mday,
//		curTime->tm_hour,
//		curTime->tm_min) ;
//	    fprintf( fp, "  close\n" ) ;
//	    fprintf( fp, "  quit\n" ) ;
//	    fprintf( fp, "!\n" ) ;
//	    fclose( fp ) ;
//	    system( "/tmp/refract.out/send_to_joss >nul" ) ;
//	}
End of deceased code */
	}
	free( map_array ) ;

/* Save .gif image of product if in research mode */
	if( (RealTimeMode == FALSE) && (OUTFIT != NCAR) ) {
	    sprintf(tmp_str, "convert n_display.ppm %s%04d%02d%02d.%02d%02d.gif", DestinationPath, year, month, day, hour, minute ) ;
	    system(tmp_str) ;
	}

}


/**************************************************************************/

/* write_text():  Write a text using a loaded font on a map (of short int).
   Input: Text, position and color info, destination array and its size
   Output: The text is written on the array
   Called by: generate_full_n_prod() */

void write_text( int pos_x, int pos_y, char *text, struct NFont *font,
	int fg_col, int bg_col, short int *dest_map, int dest_sx, int dest_sy )
{
	int		i, k, x, y, offset ;
	int		mask[8] ;
	unsigned char	c ;

	if( pos_y < 0 )  return ;

	for( i = font->size_x-2, mask[font->size_x-1] = 1 ; i >= 0 ; i-- )
	    mask[i] = mask[i+1] + mask[i+1] ;

	for( i = 0 ; i < strlen(text) ; i++ ) {
	    c = (unsigned char) text[i] ;
	    k = font->size_y * (int)c ;
	    for( y = 0 ; y < font->size_y ; y++, k++ ) {
		if( (y+pos_y) >= dest_sy )  return ;
		for( x = 0, offset = (y+pos_y)*dest_sx+pos_x ; x < font->size_x ; x++ ) {
		    if( (pos_x + x) < dest_sx ) {
			if( font->bitmap[k] & mask[x] )
			    dest_map[offset++] = fg_col ;
			else if( bg_col != -1 )
			    dest_map[offset++] = bg_col ;
			else
			    offset++ ;
		    }
		}
	    }
	    pos_x += font->size_x ;
	}
}


/**************************************************************************/

/* compute_test_factors( coherence, unison ) :
   Compute some parameters used to understand better N field change:
   average N change (from the difference field itself, not through N
   computation), coherence factor (a measurement of wind?), and the unison
   factor (predictor of weird cases?). @@@@: More used for debugging and
   understanding things than anything quantitatively useful; McGill-tailored.
   Input: Target information
   Output: Test factors
   Called by: generate_products() */

#define		RangeGetDeltaN		160
#define		RangeRefPhase		16
#define		RangeMiddleGuess	64
#define		RingHalfWidth		8
#define		RangeStCoherence	8
#define		RangeEnCoherence	160

void compute_test_factors( float *coherence, float *unison )
{
	int		az, r, offset, k, pos[2] ;
	double		super_sum_i, super_sum_q, norm ;
	float		*change_i, *change_q ;
	float		phase1, phase2, phase3, base_slope ;
	float		prev_i, prev_q, local_coherence ;
	int		count ;
	char		tmp_str[1000] ;
	FILE		*fp_anaprop ;

/* Compute anaprop detection index (strong positive in anaprop) */
/*
	base_slope = (new_av_n - ref_n) / 1000000 * GateSpacing / Wavelength * 720 ;
	fp_anaprop = fopen("anaprop_triplets.dat", "rb") ;
	if( fp_anaprop != NULL ) {
	    super_sum_i = 0 ;  super_sum_q = 0 ;
	    do {
		k = fread(pos, sizeof(int), 2, fp_anaprop) ;
		if( k == 2 ) {
		    phase2 = atan2( dif_from_ref[pos[0]*NumRangeBins+pos[1]].quadrature, dif_from_ref[pos[0]*NumRangeBins+pos[1]].inphase ) / DEGTORAD ;
		    phase1 = atan2( dif_from_ref[pos[0]*NumRangeBins+pos[1]-4].quadrature, dif_from_ref[pos[0]*NumRangeBins+pos[1]-4].inphase ) / DEGTORAD + 4 * base_slope ;
		    phase3 = atan2( dif_from_ref[pos[0]*NumRangeBins+pos[1]+4].quadrature, dif_from_ref[pos[0]*NumRangeBins+pos[1]+4].inphase ) / DEGTORAD - 4 * base_slope ;
		    while( (phase3-phase1) < -180 )  phase3 += 360 ;
		    while( (phase3-phase1) >= 180 )  phase3 -= 360 ;
		    super_sum_i += cos((phase1-phase2)*DEGTORAD) + cos((phase3-phase2)*DEGTORAD) ;
		    super_sum_q += sin((phase1-phase2)*DEGTORAD) + sin((phase3-phase2)*DEGTORAD) ;
		}
	    } while( k == 2 ) ;
	    fclose( fp_anaprop ) ;
	    ap_indx = atan2( super_sum_q, super_sum_i) / DEGTORAD ;
	}
	else
	    ap_indx = 0 ;
*/

/* Normalize difference I & Q to unity length */
	change_i = (float *) malloc( NumRangeBins * NumAzim * sizeof( float )) ;
	change_q = (float *) malloc( NumRangeBins * NumAzim * sizeof( float )) ;
	if( change_q == NULL ) {
	    sprintf( tmp_str, "Out of memory allocating `change_q'\n" ) ;
	    error_out( tmp_str, 0 ) ;
	    *unison = 0 ;  *coherence = 0 ;
	    return ;
	}
	for( az = 0, offset = 0 ; az < NumAzim ; az++ )
	    for( r = 0 ; r < NumRangeBins ; r++, offset++ ) {
		change_i[offset] = dif_prev_scan[offset].inphase ;
		change_q[offset] = dif_prev_scan[offset].quadrature ;
		norm = sqrt( SQR(change_i[offset]) + SQR(change_q[offset]) ) ;
		if( norm > 0 ) {
		    change_i[offset] /= norm ;
		    change_q[offset] /= norm ;
		}
	    }

/* First calculation: compute un-normalized unison. */
	super_sum_i = 0 ;			/* Do far ring */
	super_sum_q = 0 ;
	count = 0 ;
	for( az = 0 ; az < NumAzim ; az++ ) {
	    offset = az * NumRangeBins + RangeGetDeltaN ;
	    for( r = -RingHalfWidth ; r <= RingHalfWidth ; r++ ) {
		if( (change_i[offset+r] != 0) || (change_q[offset+r] != 0)) {
		    count++ ;
		    super_sum_i += change_i[offset+r] / 32 ;
		    super_sum_q += change_q[offset+r] / 32 ;
		}
	    }
	}
	if( count != 0 ) {
	    *unison = sqrt( SQR(super_sum_q) + SQR(super_sum_i) ) / count * 32 ;
	}
	else {
	    *unison = 0 ;
	}

/* Now the coherence: compute "pulse-pair" of the difference field;
   lag 1 / lag 0 will give the "NCP" or normalized coherence of the field. */
	if( super_sum_i == 0 && super_sum_q == 0 ) {
	    *unison = 0 ;  *coherence = 0 ;
	    free( change_q ) ; free( change_i ) ;
	    return ;
	}
	count = -1 ;
	prev_i = 0 ; prev_q = 0 ;
	super_sum_i = 0 ;  super_sum_q = 0 ;
	for( r = RangeStCoherence ; r <= RangeEnCoherence ; r++ )
	    for( az = 0 ; az < NumAzim ; az++ ) {
		offset = az * NumRangeBins + r ;
		if( change_i[offset] + change_q[offset] != 0 ) {
		    count++ ;
		    super_sum_i += change_i[offset] * prev_i + change_q[offset] * prev_q ;
		    super_sum_q += change_q[offset] * prev_i - change_i[offset] * prev_q ;
		    prev_i = change_i[offset] / 32 ;
		    prev_q = change_q[offset] / 32 ;
		}
	    }
	*coherence = sqrt( SQR(super_sum_i) + SQR(super_sum_q) ) / count * 32 ;

/* And finally, unison: just compute local coherence for normalization */
	count = -1 ;
	prev_i = 0; prev_q = 0 ;
	super_sum_i = 0 ;  super_sum_q = 0 ;
	for( r = - RingHalfWidth ; r <= RingHalfWidth ; r++ )
	    for( az = 0 ; az < NumAzim ; az++ ) {
		offset = az * NumRangeBins + r + RangeGetDeltaN ;
		if( change_i[offset] + change_q[offset] != 0 ) {
		    count++ ;
		    super_sum_i += change_i[offset] * prev_i + change_q[offset] * prev_q ;
		    super_sum_q += change_q[offset] * prev_i - change_i[offset] * prev_q ;
		    prev_i = change_i[offset] / 32 ;
		    prev_q = change_q[offset] / 32 ;
		}
	    }
	if( count > 1 ) {
	    local_coherence = sqrt( SQR(super_sum_i) + SQR(super_sum_q) ) / count * 32 ;
	    *unison /= local_coherence ;	/* Empirical scaling follows */
	    *unison /= pow(local_coherence, -.3 -.2*local_coherence) ;
	}

	free( change_q ) ;
	free( change_i ) ;
}


/**************************************************************************/

/* save_info() :  Saves all the data and maps required for the proper
   future operation of the program.
   Input: N (and maybe DeltaN) field at current time; dealiased phase data.
   Output: Some files required for later use and status information.
   Called by: main() */

void save_info(int valid)
{
	int		j, hr_round, min_round ;
	static int	map_num ;
	float		coherence, unison ;
	char		fname_out[80] ;

/* Save status information */
	if (DebugLevel != QUIET)
	    debug_print("Save N info\n") ;
	fp = fopen( StatFileName, "at" ) ;
	compute_test_factors( &coherence, &unison ) ;
	fprintf( fp, "%4d %02d %02d  %02d %02d  %5.2f  %+5.3f %5.4f %5.4f %+5.3f\n", year, month, day, hour, minute, new_av_n, n_change, coherence, unison, ap_indx ) ;
	fclose( fp ) ;

/* If data not valid, reload old target info? #### */

/* Save quality-weighted phase information as requested: Deceased code 
//	if( (RealTimeMode != FALSE) && (UseCircularQ) ) {
//	    min_round = minute - ( minute % 5 ) ;    @@@@ Is rounding a good idea?
//	    hr_round = hour ;
//	    sprintf( fname_out, "%scirc%02d%02d.target", QDestPath, hr_round, min_round ) ;
//	}
//	else
//	    sprintf( fname_out, LatestDataIn ) ;
//	fp = fopen( fname_out, "wb" ) ;
//	fprintf( fp, "GndTarg " ) ;
//	fwrite( &sec, 4, 1, fp ) ;
//	fwrite( &minute, 4, 1, fp ) ;
//	fwrite( &hour, 4, 1, fp ) ;
//	fwrite( &day, 4, 1, fp ) ;
//	fwrite( &month, 4, 1, fp ) ;
//	fwrite( &year, 4, 1, fp ) ;
//	for( j = 0 ; j < NumAzim * NumRangeBins ; j += NumRangeBins  )
//	    fwrite( &dif_from_ref[j], sizeof( struct T_data ), NumRangeBins, fp ) ;
//	fclose( fp ) ;
// End of deceased code */

/* Save smoothed phase into a circular queue of its own */
	if( (DoMapDiff == TRUE) && (RealTimeMode != FALSE) ) {
	    map_num++ ;
	    if( (map_num < 0 ) || (map_num >= NumMapDiff) )
		map_num = 0 ;
	    sprintf( fname_out, "targets.tmp.%d", map_num + 1 ) ;
	    for( j = NumMapDiff-1 ; j > 0 ; j-- )
		strcpy( last_phase_dif+j*PATH_MAX, last_phase_dif+(j-1)*PATH_MAX ) ;
	    strcpy( last_phase_dif, fname_out ) ;
	    fp = fopen( fname_out, "wb" ) ;
	    for( j = 0 ; j < NumAzim*NumRangeBins ; j += NumRangeBins )
		fwrite( &smooth_dif_scan[j], sizeof(struct T_data), NumRangeBins, fp ) ;
	    fclose( fp ) ;
	}

}


/**************************************************************************/

/* save_research :  Save the maps generated by the research version of
   n_xtract.reduced.
   Called by: main()  */

void save_research(int map_num)
{
	char		tmp_str[1000] ;
	int		k, y, num_pts ;
	time_t		curtime ;
	float		tmp_float ;
	short int	tmp_short ;
	unsigned short int tmp_ushort ;

/* Save latest maps of N and Delta-N (polar & cartesian) */
	sprintf( tmp_str, "%spolar_n.%04d%02d%02d.%02d%02d", DestinationPath, year, month, day, hour, minute ) ;
	fp = fopen( tmp_str, "wb" ) ;
    if (!fp) {
        char errStr[200];
        sprintf(errStr, "can not open %s", tmp_str);
        error_out(errStr, 1);
    }
        
	fwrite( NameOfRun, sizeof(char), PATH_MAX, fp ) ;
	fwrite( Author, sizeof(char), PATH_MAX, fp ) ;
	fwrite( ProjectName, sizeof(char), PATH_MAX, fp ) ;
	fwrite( DataVersion, sizeof(char), PATH_MAX, fp ) ;
	fwrite( SubVersion, sizeof(char), PATH_MAX, fp ) ;
	fwrite( &year, sizeof(int), 1, fp ) ;
	fwrite( &month, sizeof(int), 1, fp ) ;
	fwrite( &day, sizeof(int), 1, fp ) ;
	fwrite( &hour, sizeof(int), 1, fp ) ;
	fwrite( &minute, sizeof(int), 1, fp ) ;
	fwrite( &new_av_n, sizeof(float), 1, fp ) ;
	fwrite( &Latitude, sizeof(float), 1, fp ) ;
	fwrite( &Longitude, sizeof(float), 1, fp ) ;
	fwrite( &DoRelax, sizeof(int), 1, fp ) ;
	fwrite( RefFileName, sizeof(char), PATH_MAX, fp ) ;
	fwrite( &NSmoothingSideLen, sizeof(float), 1, fp ) ;
	fwrite( &MinConsistency, sizeof(float), 1, fp ) ;
	curtime = time(NULL) ;
	fwrite( &curtime, sizeof(time_t), 1, fp ) ;
	fwrite( &NOutNumAzim, sizeof(int), 1, fp ) ;
	fwrite( &NOutNumRanges, sizeof(int), 1, fp ) ;
	tmp_float = (float)NumRangeBins * GateSpacing / NOutNumRanges ;
	fwrite( &tmp_float, sizeof(float), 1, fp ) ;
	num_pts = NOutNumAzim * NOutNumRanges ;

	for( k = 0 ; k < num_pts ; k++ ) {
	    tmp_ushort = (unsigned short int)(100. * n_array_polar[k]) ;
	    fwrite( &tmp_ushort, sizeof(unsigned short int), 1, fp ) ;
	}
	for( k = 0 ; k < num_pts ; k++ ) {
	    if( n_er_array_polar[k] > 655.35 )
		tmp_ushort = 65535 ;
	    else
		tmp_ushort = (unsigned short int)(100. * n_er_array_polar[k]) ;
	    fwrite( &tmp_ushort, sizeof(unsigned short int), 1, fp ) ;
	}
	fclose( fp ) ;

	sprintf( tmp_str, "%spolar_dn.%04d%02d%02d.%02d%02d", DestinationPath, year, month, day, hour, minute ) ;
	fp = fopen( tmp_str, "wb" ) ;
	fwrite( NameOfRun, sizeof(char), PATH_MAX, fp ) ;
	fwrite( Author, sizeof(char), PATH_MAX, fp ) ;
	fwrite( ProjectName, sizeof(char), PATH_MAX, fp ) ;
	fwrite( DataVersion, sizeof(char), PATH_MAX, fp ) ;
	fwrite( SubVersion, sizeof(char), PATH_MAX, fp ) ;
	fwrite( &year, sizeof(int), 1, fp ) ;
	fwrite( &month, sizeof(int), 1, fp ) ;
	fwrite( &day, sizeof(int), 1, fp ) ;
	fwrite( &hour, sizeof(int), 1, fp ) ;
	fwrite( &minute, sizeof(int), 1, fp ) ;
	fwrite( &n_change, sizeof(float), 1, fp ) ;
	fwrite( &Latitude, sizeof(float), 1, fp ) ;
	fwrite( &Longitude, sizeof(float), 1, fp ) ;
	fwrite( &DoRelax, sizeof(int), 1, fp ) ;
	fwrite( RefFileName, sizeof(char), PATH_MAX, fp ) ;
	fwrite( &NSmoothingSideLen, sizeof(float), 1, fp ) ;
	fwrite( &MinConsistency, sizeof(float), 1, fp ) ;
	fwrite( &curtime, sizeof(time_t), 1, fp ) ;
	fwrite( &DNOutNumAzim, sizeof(int), 1, fp ) ;
	fwrite( &DNOutNumRanges, sizeof(int), 1, fp ) ;
	tmp_float = (float)NumRangeBins * GateSpacing / DNOutNumRanges ;
	fwrite( &tmp_float, sizeof(float), 1, fp ) ;
	num_pts = DNOutNumAzim * DNOutNumRanges ;
	for( k = 0 ; k < num_pts ; k++ ) {
	    tmp_short = (short int)(100. * dn_array_polar[k]) ;
	    fwrite( &tmp_short, sizeof(short int), 1, fp ) ;
	}
	for( k = 0 ; k < num_pts ; k++ ) {
	    if( dn_er_array_polar[k] > 655.35 )
		tmp_ushort = 65535 ;
	    else
		tmp_ushort = (unsigned short int)(100. * dn_er_array_polar[k]) ;
	    fwrite( &tmp_ushort, sizeof(unsigned short int), 1, fp ) ;
	}
	fclose( fp ) ;

	if( DoCartesianN == TRUE ) {
	    sprintf( tmp_str, "%sn.%04d%02d%02d.%02d%02d", DestinationPath, year, month, day, hour, minute ) ;
	    fp = fopen( tmp_str, "wb" ) ;
	    fwrite( NameOfRun, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( Author, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( ProjectName, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( DataVersion, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( SubVersion, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( &year, sizeof(int), 1, fp ) ;
	    fwrite( &month, sizeof(int), 1, fp ) ;
	    fwrite( &day, sizeof(int), 1, fp ) ;
	    fwrite( &hour, sizeof(int), 1, fp ) ;
	    fwrite( &minute, sizeof(int), 1, fp ) ;
	    fwrite( &new_av_n, sizeof(float), 1, fp ) ;
	    fwrite( &Latitude, sizeof(float), 1, fp ) ;
	    fwrite( &Longitude, sizeof(float), 1, fp ) ;
	    fwrite( &DoRelax, sizeof(int), 1, fp ) ;
	    fwrite( RefFileName, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( &NSmoothingSideLen, sizeof(float), 1, fp ) ;
	    fwrite( &MinConsistency, sizeof(float), 1, fp ) ;
	    fwrite( &curtime, sizeof(time_t), 1, fp ) ;
	    fwrite( &CartesianX, sizeof(int), 1, fp ) ;
	    fwrite( &CartesianY, sizeof(int), 1, fp ) ;
	    fwrite( &CartesianResol, sizeof(float), 1, fp ) ;
	    num_pts = CartesianX * CartesianY ;
	    for( k = 0 ; k < num_pts ; k++ ) {
		if( n_array_xy[k] == INVALID )
		    tmp_ushort = 0 ;
		else
		    tmp_ushort = (unsigned short int)(100. * n_array_xy[k]) ;
		fwrite( &tmp_ushort, sizeof(unsigned short int), 1, fp ) ;
	    }
	    for( k = 0 ; k < num_pts ; k++ ) {
		if( n_er_array_xy[k] > 655.35 )
		    tmp_ushort = 65535 ;
		else
		    tmp_ushort = (unsigned short int)(100. * n_er_array_xy[k]) ;
		fwrite( &tmp_ushort, sizeof(unsigned short int), 1, fp ) ;
	    }
	    fclose( fp ) ;

	    sprintf( tmp_str, "%sdn.%04d%02d%02d.%02d%02d", DestinationPath, year, month, day, hour, minute ) ;
	    fp = fopen( tmp_str, "wb" ) ;
	    fwrite( NameOfRun, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( Author, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( ProjectName, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( DataVersion, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( SubVersion, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( &year, sizeof(int), 1, fp ) ;
	    fwrite( &month, sizeof(int), 1, fp ) ;
	    fwrite( &day, sizeof(int), 1, fp ) ;
	    fwrite( &hour, sizeof(int), 1, fp ) ;
	    fwrite( &minute, sizeof(int), 1, fp ) ;
	    fwrite( &n_change, sizeof(float), 1, fp ) ;
	    fwrite( &Latitude, sizeof(float), 1, fp ) ;
	    fwrite( &Longitude, sizeof(float), 1, fp ) ;
	    fwrite( &DoRelax, sizeof(int), 1, fp ) ;
	    fwrite( RefFileName, sizeof(char), PATH_MAX, fp ) ;
	    fwrite( &NSmoothingSideLen, sizeof(float), 1, fp ) ;
	    fwrite( &MinConsistency, sizeof(float), 1, fp ) ;
	    fwrite( &curtime, sizeof(time_t), 1, fp ) ;
	    fwrite( &CartesianX, sizeof(int), 1, fp ) ;
	    fwrite( &CartesianY, sizeof(int), 1, fp ) ;
	    fwrite( &CartesianResol, sizeof(float), 1, fp ) ;
	    for( k = 0 ; k < num_pts ; k++ ) {
		tmp_short = (unsigned short int)(100. * dn_array_xy[k]) ;
		fwrite( &tmp_short, sizeof(short int), 1, fp ) ;
	    }
	    for( k = 0 ; k < num_pts ; k++ ) {
		if( (dn_er_array_xy[k] > 655.35) || (dn_er_array_xy[k] == INVALID) )
		    tmp_ushort = 65535 ;
		else
		    tmp_ushort = (unsigned short int)(100. * dn_er_array_xy[k]) ;
		fwrite( &tmp_ushort, sizeof(unsigned short int), 1, fp ) ;
	    }
	    fclose( fp ) ;
	}

/* Save smoothed phase into a circular queue of its own */
	if( DoMapDiff == TRUE ) {
	    sprintf( tmp_str, "targets.tmp.%d", (map_num % NumMapDiff) + 1 ) ;
	    for( k = NumMapDiff-1 ; k > 0 ; k-- )
		strcpy( last_phase_dif+k*PATH_MAX, last_phase_dif+(k-1)*PATH_MAX ) ;
	    strcpy( last_phase_dif, tmp_str ) ;
	    fp = fopen( tmp_str, "wb" ) ;
	    for( k = 0 ; k < NumAzim*NumRangeBins ; k += NumRangeBins )
		fwrite( &smooth_dif_scan[k], sizeof(struct T_data), NumRangeBins, fp ) ;
	    fclose( fp ) ;
	}
}


/**************************************************************************/

/* error_out( text, fatal_flag ) :  Outputs an error message to various
   places; aborts the process if error is fatal.
   Called by: main(), startup(), and all routines doing memory allocation */

void error_out( char *text, int fatal_flag )
{
	FILE 		*fp_tmp ;

	fp_tmp = fopen( StatFileName, "at" ) ;
    if (fp_tmp) fprintf( fp_tmp, "\t%s.", text ) ;
	if( fatal_flag != FALSE ) {
	    printf( "N_XTRACT error: %s.\n", text ) ;
	    if (fp_tmp) fprintf( fp_tmp, "  Aborted!.\n") ;
	}
	else {
	    if (fp_tmp) fprintf( fp_tmp, "\n") ;
	    if( DebugLevel != QUIET)
		debug_print( text ) ;
	}
	if (fp_tmp) fclose( fp_tmp ) ;
	if( fatal_flag != FALSE )
	    exit( fatal_flag ) ;
}



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
/*************************** n_viewcalib.c *********************************/

/* This program displays the fields computed by n_calib.c on a PPM image
   Frederic Fabry, June 2006

Usage: n_viewcalib paramaters_file

------------------------------------------------------------------------------
*/

#define		N_VIEWCALIB

/* Declaration and includes */
#include	<refractt/n_input.h>		/* Parameter-reading code shared with n_xtract & n_xtract.h */
#include        <refractt/n_xtract.h>

#define		BLACK_INDEX  0
#define		WHITE_INDEX 121
#define		BLUE_PURPLE_INDEX 110
#define		FIRST_COLOR_INDEX 1
#define		DEEP_RED_INDEX    11
#define		LAST_COLOR_INDEX 120  /* (white isn't really a color) */
#define		TRUENCOLORS	122

#define		MinSNR		-10
#define		MaxSNR		70
#define		SNRScaleStep	10
#define		MinQual		0.
#define		MaxQual		1.
#define		QualScaleStep	.1

/* (Not really) Large arrays and global variables */
char		fname_in[NUMSCANS][PATH_MAX] ;
float		*sum_a, *sum_b, *sum_p, *fluct_snr, *mean_snr, *old_snr ;
float		elev_angle, ref_n ;
int		*pixel_count ;
struct T_data   *raw_phase ;
struct R_info   *calib_ref ;
struct T_data   *dif_from_ref ;		/* [NumAzim*NumRangeBins] ; */ 
struct T_data   *dif_prev_scan ;	/* [NumAzim*NumRangeBins] ; */
struct stat	latest_data ;
int		az_cor, bridge_detected ;	/* @@@@ */
int		year, month, day, hour, minute, sec ;
int		gnd_filter ;
short int	rgb[TRUENCOLORS][3] ;
struct NFont	font ;
FILE            *fp ;
char		*data ;			/* The output image */
short int	*dummy ;

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

/* Empty shells needed to compile unused routines in n_input.c  #### Should be fixed */
/* int read_data_dualp( char *data_filename ) {}
int read_data_viraq( char *data_filename ) {}
int read_data_previraq( char *data_filename ) {}
int read_data_spol( char *data_filename ) {}
*/

/**************************************************************************/

/* Display routines */

void setcolors()
{
     int   color,irow,icol,i;
     float stp ;
     unsigned  long  valuemask,istat;
/*  application   variables    */
    int nfile,icount,ip,ip1,idum;
  unsigned long  depth;

/**  colour scheme  **/

    rgb[BLACK_INDEX][0] = 0 ;  rgb[BLACK_INDEX][1] = 0 ;
    rgb[BLACK_INDEX][2] = 0 ;  rgb[WHITE_INDEX][0] = 255 ;
    rgb[WHITE_INDEX][1] = 255 ;  rgb[WHITE_INDEX][2] = 255 ;

    for (i = FIRST_COLOR_INDEX ; i <= LAST_COLOR_INDEX ; i++) {
	stp = 6. * ((float)(i - FIRST_COLOR_INDEX + 1) / (LAST_COLOR_INDEX - FIRST_COLOR_INDEX + 1)) ;
/*	printf("Color %d: stp = %5.3f\n", i, stp) ; */
	if( stp <= 1. ) {
	    rgb[i][0] = (short int)(128 + stp*127) ;   /* violet to red */
            rgb[i][1] = 0 ;
            rgb[i][2] = (short int)(128 - stp*128) ;
	}
	else if( stp < 2. ) {
	    rgb[i][0] = 255 ;
	    rgb[i][1] = (short int)((stp-1)*255) ;
	    rgb[i][2] = 0 ;
	}
	else if( stp == 2. ) {
	    rgb[i][0] = 247 ;
	    rgb[i][1] = 247 ;
	    rgb[i][2] = 0 ;
	}
	else if( stp <= 3. ) {
	    rgb[i][0] = (short int)((3.-stp)*255) ;
	    rgb[i][1] = 255 ;
	    rgb[i][2] = 0 ;
	}
	else if( stp <= 4. ) {
	    rgb[i][0] = 0 ;
	    rgb[i][1] = (short int)(255 - (stp-3.)*127) ;
	    rgb[i][2] = (short int)((stp-3.)*128) ;
	}
	else if( stp <= 5. ) {
	    rgb[i][0] = 0 ;
	    rgb[i][1] = (short int)((5.-stp)*128) ;
	    rgb[i][2] = 255 + (short int)((stp-5.)*127) ;
	}
	else if( stp <= 6. ) {
	    rgb[i][0] = (short int)((stp-5.)*128) ;
	    rgb[i][1] = 0 ;
	    rgb[i][2] = 255 - (short int)((stp-5.)*127) ;
	}

/* Fancier color scheme, but that does not look nice of this noisy field:
	stp = 6. * (1. - (float)(i - FIRST_COLOR_INDEX) / (LAST_COLOR_INDEX - FIRST_COLOR_INDEX + 1)) ;
	if( stp < .5 ) {
	    rgb[i][0] = 0 ;
	    rgb[i][1] = 0 ;
	    rgb[i][2] = (short int)(128 + stp * 254) ;
	}
	else if( stp <= 1. ) {
	    rgb[i][0] = (short int)(256 * (stp - .5)) ;
	    rgb[i][1] = rgb[i][0] ;
	    rgb[i][2] = 255 ;
	}
	else if( stp <= 2. ) {
	    rgb[i][0] = 0 ;
	    rgb[i][1] = (short int)(exp(log(64) * (2. - stp) + log(255) * (stp - 1.))) ;
	    rgb[i][2] = rgb[i][1] ;
	}
	else if( stp <= 3. ) {
	    rgb[i][0] = 0 ;
	    rgb[i][1] = (short int)(exp(log(64) * (3. - stp) + log(255) * (stp - 2.))) ;
	    rgb[i][2] = 0 ;
	}
	else if( stp <= 4. ) {
	    rgb[i][0] = (short int)(exp(log(64) * (4. - stp) + log(255) * (stp - 3.))) ;
	    rgb[i][1] = rgb[i][0] ;
	    rgb[i][2] = 0 ;
	}
	else if( stp <= 4.5 ) {
	    rgb[i][0] = (short int)(128 + 254 * (stp - 4.)) ;
	    rgb[i][1] = 0 ;
	    rgb[i][2] = 0 ;
	}
	else if( stp <= 5. ) {
	    rgb[i][0] = 255 ;
	    rgb[i][1] = (short int)(256 * (stp - 4.5)) ;
	    rgb[i][2] = rgb[i][1] ;
	}
	else if( stp <= 5.5 ) {
	    rgb[i][0] = (short int)(128 + 254 * (stp - 5.)) ;
	    rgb[i][1] = 0 ;
	    rgb[i][2] = (short int)(96 + 190 * (stp - 5.)) ;
	}
	else if( stp <= 6. ) {
	    rgb[i][0] = 255 ;
	    rgb[i][1] = (short int)(256 * (stp - 5.5)) ;
	    rgb[i][2] = (short int)(191 + 128 * (stp - 5.5)) ;
	}   */

/* Black and white scheme */
	rgb[i][0] = (short int)(stp * 255 / 6) ;
	rgb[i][1] = rgb[i][0] ;
	rgb[i][2] = rgb[i][0] ;
    }

}

/* ----------------------------- Other subroutines -----------------------  */


void set_pixel( int i, float value ) 
{
    static float rest ;
    int pix_value ;

    if(( rest < 0 ) || ( rest >= 1 ))  rest = .5 ;
    rest += value ;
    pix_value = (int)(floor( rest )) ;
    rest -= pix_value ;
    data[i*3] = rgb[pix_value][0] ;
    data[i*3+1] = rgb[pix_value][1] ;
    data[i*3+2] = rgb[pix_value][2] ;
}


/* write_text():  Write a text using a loaded font on a map (of short int).
   Input: Text, position and color info, destination array and its size
   Output: The text is written on the array
   Called by: generate_full_n_prod() */

void write_text( int pos_x, int pos_y, char *text, struct NFont *font,
        int fg_col, int bg_col, short int *dest_map, int size_x, int size_y )
{
        int             i, k, x, y ;
        int             mask[8] ;

        for( i = font->size_x-2, mask[font->size_x-1] = 1 ; i >= 0 ; i-- )
            mask[i] = mask[i+1] + mask[i+1] ;

        for( i = 0 ; i < strlen(text) ; i++ ) {
            k = font->size_y * (int)text[i] ;
            for( y = 0 ; y < font->size_y ; y++, k++ )
                for( x = 0 ; x < font->size_x ; x++ )
                    if( font->bitmap[k] & mask[x] ) {
			data[(size_x*(y+pos_y)+pos_x+x)*3] = rgb[fg_col][0] ;
			data[(size_x*(y+pos_y)+pos_x+x)*3+1] = rgb[fg_col][1] ;
			data[(size_x*(y+pos_y)+pos_x+x)*3+2] = rgb[fg_col][2] ;
		    }
                    else if( bg_col != -1 ) {
			data[(size_x*(y+pos_y)+pos_x+x)*3] = rgb[bg_col][0] ;
			data[(size_x*(y+pos_y)+pos_x+x)*3+1] = rgb[bg_col][1] ;
			data[(size_x*(y+pos_y)+pos_x+x)*3+2] = rgb[bg_col][2] ;
		    }
            pos_x += font->size_x ;
        }
}


/* Draw lines */

void hor_line( int beg_x, int beg_y, int end_x, int end_y, int col, int size_x )
{
	int		i ;

	for( i = beg_x ; i <= end_x ; i++ ) {
	    data[(size_x*beg_y+i)*3] = rgb[col][0] ;
	    data[(size_x*beg_y+i)*3+1] = rgb[col][1] ;
	    data[(size_x*beg_y+i)*3+2] = rgb[col][2] ;
	}
}

void ver_line( int beg_x, int beg_y, int end_x, int end_y, int col, int size_x ) 
{
        int             i ;

        for( i = beg_y ; i <= end_y ; i++ ) {
	    data[(size_x*i+beg_x)*3] = rgb[col][0] ;
	    data[(size_x*i+beg_x)*3+1] = rgb[col][1] ;
	    data[(size_x*i+beg_x)*3+2] = rgb[col][2] ;
	}
}

void rectangle( int beg_x, int beg_y, int step_x, int step_y, int col, int size_x )
{
	hor_line( beg_x, beg_y, beg_x+step_x, beg_y, col, size_x ) ;
	ver_line( beg_x+step_x, beg_y, beg_x+step_x, beg_y+step_y, col, size_x ) ;
	hor_line( beg_x, beg_y+step_y, beg_x+step_x, beg_y+step_y, col, size_x ) ;
	ver_line( beg_x, beg_y, beg_x, beg_y+step_y, col, size_x ) ;
}


/**************************************************************************/

/* startup():  Initializes n_viewcalib.
   - Prints start-up message (well... it _is_ an important task!);
   - Allocates and initializes all the needed global variables;
   - Checks for the presence of an existing calibration file.
   Input: None.
   Output: Allocated calib_ref array and status of presence of mask.
   Called by: main() */

int startup(int argc, char* argv[])
{

/* Start up */
	printf( "N_VIEWCALIB %s:  Generates images of the fields in the calibration file.\n", N_VERSION ) ;
	printf( "Frederic Fabry sends greetings (%s)\n\n", MONTH_YEAR ) ;

/* Read parameters file */
	if( argc == 2 )
	    get_params( argv[1] ) ;
	else {
	    printf("Usage: n_viewcalib parameter_file\n") ;
	    printf("\twhere \"parameter_file\" contains all the data processing parameters used by n_calib and n_xtract.\n\n") ;
	    exit(1) ;
	}

/* Create main data array */
        calib_ref = (struct R_info *)  calloc( NumAzim * NumRangeBins, sizeof(struct R_info ) ) ;
        if( calib_ref == NULL ) {
            printf("Out of memory allocating calib_ref array.  Quitting.\n") ;
            exit( 3 ) ;
        }

/* More initializations: load colors, font */
	setcolors() ;
	fp = fopen( FontName, "rb" ) ;
	if (fp == NULL) {
	    error_out("Can't open font file", 7);
	}
	fread( &font, sizeof(struct NFont), 1, fp ) ;
	font.size_x = 7 ;
	font.size_y = 12 ;
	fclose( fp ) ;

/* Check for the presence of an existing calibration file and load it */
        if ((fp = fopen( RefFileName, "rb" )) != NULL) {
	    fread( &ref_n, sizeof( float ), 1, fp ) ;
	    fread( calib_ref, sizeof( struct R_info ), NumRangeBins * NumAzim, fp ) ;
	    fclose( fp ) ;
	    return( TRUE ) ;
        }
	else
	    return(FALSE) ;
}


/*****************************************************************************/

void error_out( char *text, int fatal_flag )
{
	if( fatal_flag != FALSE ) {
	    printf( "N_VIEWCALIB error: %s.\n", text ) ;
	    exit( fatal_flag ) ;
	}
	else
	    printf( text ) ;
}
	

/**************************************************************************/

int main(int argc, char* argv[])
{
	int		i, j, k, x, y, col, offset ;
	int		ProdX, ProdY, SizeScale ;
	float		snr, quality ;
	char		tmp_str[100] ;

/* Load calibration file */
	if( startup(argc, argv) != TRUE )
	    printf("Calibration file not found\n") ;

/* Set-up destination image */
	ProdX = 2 * NumRangeBins + 80 ;
	ProdY = NumAzim + 120 ;
	data = (char *) calloc( ProdX * ProdY * 3, sizeof(char) ) ;
	if( data == NULL )
	    error_out("Could not allocate destination image. Exiting.", 8) ;

/* Plot data */
	for( y = 0, offset = 0 ; y < NumAzim ; y++ ) {
	    for( x = 0, k = ProdX*(y+5)+5 ; x < NumRangeBins ; x++, k++, offset++ ) {
		col = (int)(FIRST_COLOR_INDEX + (calib_ref[offset].strength - MinSNR) * (LAST_COLOR_INDEX - FIRST_COLOR_INDEX) / (MaxSNR - MinSNR)) ;
		if( col < FIRST_COLOR_INDEX )
		    col = FIRST_COLOR_INDEX ;
		if( col > LAST_COLOR_INDEX )
		    col = LAST_COLOR_INDEX ;
		set_pixel( k, col ) ;
		quality = sqrt(calib_ref[offset].av_i*calib_ref[offset].av_i + calib_ref[offset].av_q*calib_ref[offset].av_q) ;
		col = (int)(FIRST_COLOR_INDEX + (quality - MinQual) * (LAST_COLOR_INDEX - FIRST_COLOR_INDEX) / (MaxQual - MinQual)) ;
		if( col < FIRST_COLOR_INDEX )
		    col = FIRST_COLOR_INDEX ;
		if( col > LAST_COLOR_INDEX )
		    col = LAST_COLOR_INDEX ;
		set_pixel( k + NumRangeBins + 70, col ) ;
	    }
	}

/* Plot the color part of the scales */
	SizeScale = NumRangeBins * 2 / 3 ;
	for( i = 0 ; i < SizeScale ; i++ ) {
	    snr = MinSNR + (MaxSNR - MinSNR) * i / SizeScale ;
	    col = (int)(FIRST_COLOR_INDEX + (snr - MinSNR) * (LAST_COLOR_INDEX - FIRST_COLOR_INDEX) / (MaxSNR - MinSNR)) ;
	    if( col < FIRST_COLOR_INDEX )
		col = FIRST_COLOR_INDEX ;
	    if( col > LAST_COLOR_INDEX )
		col = LAST_COLOR_INDEX ;
	    for( j = 75 + NumAzim; j < 95 + NumAzim ; j++ )
		set_pixel( j*ProdX+i+(NumRangeBins-SizeScale)/2, col ) ;

	    quality = MinQual + (MaxQual - MinQual) * i / SizeScale ;
	    col = (int)(FIRST_COLOR_INDEX + (quality - MinQual) * (LAST_COLOR_INDEX - FIRST_COLOR_INDEX) / (MaxQual - MinQual)) ;
	    if( col < FIRST_COLOR_INDEX )
		col = FIRST_COLOR_INDEX ;
	    if( col > LAST_COLOR_INDEX )
		col = LAST_COLOR_INDEX ;
	    for( j = 75 + NumAzim; j < 95 + NumAzim ; j++ )
		set_pixel( j*ProdX+i+(NumRangeBins-SizeScale)/2+NumRangeBins+70, col ) ;
	}

/* Add legends, etc */
	sprintf( tmp_str, "SNR (dB)" ) ;
	write_text( (int)(5 + NumRangeBins/2 - 3.5 * strlen(tmp_str)), NumAzim + 35, tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;
	sprintf( tmp_str, "Quality of ground target" ) ;
	write_text( (int)(75 + 3*NumRangeBins/2 - 3.5 * strlen(tmp_str)), NumAzim + 35, tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;
	rectangle( 4,4, NumRangeBins+1, NumAzim+1, WHITE_INDEX, ProdX ) ;
	rectangle( NumRangeBins+74,4, NumRangeBins+1, NumAzim+1, WHITE_INDEX, ProdX ) ;

	for( i = 5 ; i <= NumAzim + 8 ; i += NumAzim/8 ) {
	    hor_line( 0, i, 4, i, WHITE_INDEX, ProdX ) ;
	    hor_line( 5+NumRangeBins, i, 9+NumRangeBins, i, WHITE_INDEX, ProdX ) ;
	    hor_line( 70+NumRangeBins, i, 74+NumRangeBins, i, WHITE_INDEX, ProdX ) ;
	    hor_line( 75+2*NumRangeBins, i, 79+2*NumRangeBins, i, WHITE_INDEX, ProdX ) ;
	}
	sprintf( tmp_str, "NORTH" ) ;  write_text( (int)(ProdX/2-3.5*strlen(tmp_str)),0,tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;
	sprintf( tmp_str, "NE" ) ;  write_text( (int)(ProdX/2-3.5*strlen(tmp_str)),(int)(NumAzim/8),tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;
	sprintf( tmp_str, "EAST" ) ;  write_text( (int)(ProdX/2-3.5*strlen(tmp_str)),(int)(NumAzim/4),tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;
	sprintf( tmp_str, "SE" ) ;   write_text( (int)(ProdX/2-3.5*strlen(tmp_str)),(int)(NumAzim*3/8),tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;
	sprintf( tmp_str, "SOUTH" ) ;  write_text( (int)(ProdX/2-3.5*strlen(tmp_str)),(int)(NumAzim/2),tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;
	sprintf( tmp_str, "SW" ) ;  write_text( (int)(ProdX/2-3.5*strlen(tmp_str)),(int)(NumAzim*5/8),tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;
	sprintf( tmp_str, "WEST" ) ;  write_text( (int)(ProdX/2-3.5*strlen(tmp_str)),(int)(NumAzim*3/4),tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;
	sprintf( tmp_str, "NW" ) ;  write_text( (int)(ProdX/2-3.5*strlen(tmp_str)),(int)(NumAzim*7/8),tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;
	sprintf( tmp_str, "NORTH" ) ;  write_text( (int)(ProdX/2-3.5*strlen(tmp_str)),(int)(NumAzim),tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;

	for( j = 0; j <= NumRangeBins*GateSpacing ; j += 10000 ) {
	    i = 5 + (int)((float)j / GateSpacing) ;
	    ver_line( i, 0, i, 4, WHITE_INDEX, ProdX ) ;
	    ver_line( i, NumAzim+5, i, NumAzim+9, WHITE_INDEX, ProdX ) ;
	    ver_line( i+NumRangeBins+70, 0, i+NumRangeBins+70, 4, WHITE_INDEX, ProdX ) ;
	    ver_line( i+NumRangeBins+70, NumAzim+5, i+NumRangeBins+70, NumAzim+9, WHITE_INDEX, ProdX ) ;
	    sprintf( tmp_str, "%d", (j/1000) ) ;
	    write_text( (int)(i - 3.5*strlen(tmp_str)), NumAzim+13, tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;
	    write_text( (int)(i - 3.5*strlen(tmp_str)+NumRangeBins+70), NumAzim+13, tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;
	}
	sprintf( tmp_str, "km" ) ;
	write_text( i + 15, NumAzim+13, tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;

	rectangle( (NumRangeBins-SizeScale)/2, NumAzim+75, SizeScale, 20, WHITE_INDEX, ProdX) ;
	for( snr = MinSNR ; snr <= MaxSNR ; snr += SNRScaleStep ) {
	    i = (int)((NumRangeBins-SizeScale)/2 + (snr-MinSNR)*SizeScale/(MaxSNR-MinSNR)) ;
	    ver_line( i, NumAzim+96, i, NumAzim+100, WHITE_INDEX, ProdX ) ;
	    sprintf( tmp_str, "%d", (int)snr ) ;
	    write_text( (int)(i - 3.5*strlen(tmp_str)), NumAzim+104, tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;
	}
	sprintf( tmp_str, "dB" ) ;
	write_text( i + 15, NumAzim+104, tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;

	rectangle( (NumRangeBins-SizeScale)/2+NumRangeBins+70, NumAzim+75, SizeScale, 20, WHITE_INDEX, ProdX) ;
	for( quality = MinQual ; quality <= MaxQual ; quality += QualScaleStep ) {
	    i = (int)((NumRangeBins-SizeScale)/2 + (quality-MinQual)*SizeScale/(MaxQual-MinQual)) + NumRangeBins+70 ;
	    ver_line( i, NumAzim+96, i, NumAzim+100, WHITE_INDEX, ProdX ) ;
	    sprintf( tmp_str, "%3.1f", quality ) ;
	    write_text( (int)(i - 3.5*strlen(tmp_str)), NumAzim+104, tmp_str, &font, WHITE_INDEX, -1, dummy, ProdX, ProdY ) ;
	}

/* Write the image file; initiate display */
	fp = fopen( "tmp.ppm", "wb" ) ;
	if ( !fp) {
    	    fprintf(stderr, "can't open graphic ppm file ");
	    exit(1);
	}
	fprintf(fp, "P6 %d %d 255 ", ProdX, ProdY) ;
	fwrite( data, ProdX*ProdY, 3, fp ) ;
	fclose( fp ) ;
	system( "display tmp.ppm" ) ;
//	unlink( "tmp.ppm" ) ;
}



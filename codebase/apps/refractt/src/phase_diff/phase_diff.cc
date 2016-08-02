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
/* 
 *  dif_phase.c - Displays the phase difference of low level echoes
 *  		  between two times
 */

#include <stdio.h>
#include        <stdlib.h>
#include        <math.h>
#include        <string.h>
#include        <sys/stat.h>
#include        <time.h>
#include        <unistd.h>
#include <refractt/n_xtract.h>


/* Defines */
#define		NUMAZIM		360
#define		NUMRANGEBINS	450	/* #### */
#define		IMGWIDTH	(2*NUMRANGEBINS + 80) 
#define		IMGHEIGHT	(NUMAZIM + 120)
#define		TRUENCOLORS	122
#define		SIZESCALE	(NUMRANGEBINS*2/3)

#define         MinNIQ          -50
#define		MaxNIQ		10
#define         NIQScaleStep    10
#define         RangeNIQ        (MaxNIQ - MinNIQ)

/* The code below "plots" the phase difference and power fields on a graphics
 * array.  For this, it uses the defined colors:  0 = background/black; 
 * FIRST_COLOR_INDEX to LAST_COLOR_INDEX = six color bands; WHITE_INDEX =
 * foreground/white.  For phase, I am using the whole color scale (1-120) so
 * that there is no "break" in the color scheme when phase folds from +180 to
 * -180.  For power, to avoid confusion between very weak and very strong
 * NIQs, I only use the colors between 6 (deep red) and 55 (blue-violet).
 * [Starting to see familiar numbers?]
*/


#define        BLACK_INDEX  0
#define        WHITE_INDEX 121
#define        BLUE_PURPLE_INDEX 110
#define        FIRST_COLOR_INDEX 1
#define        DEEP_RED_INDEX    11
#define        LAST_COLOR_INDEX 120  /* (white isn't really a color) */

/* Global variables, part 1 (processing related) */
char  data[IMGWIDTH*IMGHEIGHT][3] ;
int   filetype[2], azcor[2] ;
int   bridge_detected, gnd_filter ;
float *vel, *sw, *snr, elev_angle, FirstRange ;
float *zed ;
int   year, month, day, hour, minute, sec ;
struct T_data   *raw_phase ;
short int *dummy ;
time_t time_cur_scan ;

float sum_i1[NUMRANGEBINS*NUMAZIM], sum_q1[NUMRANGEBINS*NUMAZIM] ;
float sum_i2[NUMRANGEBINS*NUMAZIM], sum_q2[NUMRANGEBINS*NUMAZIM] ;
void error_out( char *text, int fatal_flag ) {}
void debug_print( char *text ) {printf( text );}

/* Global variables, part 2 (display related) */
unsigned int img_depth,img_width,img_height;
int  offset,bitmap_pad,bytes_per_line;
short int rgb[TRUENCOLORS][3] ;
/* unsigned long plane_masks[8],nplanes,pixels[NCOLORS];
int  frame_width,frame_height,frame_depth;
*/

/* Global variables, part 3 (parameters) */
int  NumAzim = 360, NumRangeBins = 450, DebugLevel = 0 ;
float GateSpacing = 150. ;
char FontName[PATH_MAX] ;

/* ------------------------------ X Display routines ---------------------  */

void setup()
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
    data[i][0] = rgb[pix_value][0] ;
    data[i][1] = rgb[pix_value][1] ;
    data[i][2] = rgb[pix_value][2] ;
}


float get_snr( int azimuth, int range, int element, FILE *fp )
{
        unsigned short  c ;

        fseek( fp, azimuth*12960+2*range+12284, SEEK_SET ) ;
        fread( &c, 2, 1, fp ) ;
        return( c * 1.28 / 8 ) ;
}


int get_drift(FILE *fp)         /* @@@@ McGill only issue */
{
        int             j, k, pos_max ;
        static int      drift_value = 0 ;
        float           row[7], max_power, max_second ;

/* Compute log(Power) around the bridge on highway 40 */
        for( j = 0 ; j < 7 ; j++ ) {
            row[j] = 0 ;
            for( k = 27 ; k < 36 ; k++ )
                row[j] += get_snr( 261+j, k, 0, fp ) ;
        }

/* Find azimuth of maximum power */
        max_power = 0 ;
        max_second = 0 ;
        for( j = 0 ; j < 7 ; j++ )
            if( row[j] > max_power ) {
                max_second = max_power ;
                max_power = row[j] ;
                pos_max = j ;
            }
            else if( row[j] > max_second )
                max_second = row[j] ;

/* If greater than ~11 dB over its nearest competitor, that's the bridge.
    Compute offset from expected position (az = 265) and return it.
    Otherwise, assume weather is a problem and return old value.
    Note:  True azimuth of bridge is 263.0 deg. */
        if( max_power > max_second + 100 ) {
            bridge_detected = TRUE ;
            drift_value = pos_max - 4 ;
        }
        else
            bridge_detected = FALSE ;
        return( drift_value ) ;
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
			data[size_x*(y+pos_y)+pos_x+x][0] = rgb[fg_col][0] ;
			data[size_x*(y+pos_y)+pos_x+x][1] = rgb[fg_col][1] ;
			data[size_x*(y+pos_y)+pos_x+x][2] = rgb[fg_col][2] ;
		    }
                    else if( bg_col != -1 ) {
			data[size_x*(y+pos_y)+pos_x+x][0] = rgb[bg_col][0] ;
			data[size_x*(y+pos_y)+pos_x+x][1] = rgb[bg_col][1] ;
			data[size_x*(y+pos_y)+pos_x+x][2] = rgb[bg_col][2] ;
		    }
            pos_x += font->size_x ;
        }
}


/* Draw lines */

void hor_line( int beg_x, int beg_y, int end_x, int end_y, int col )
{
	int		i ;

	for( i = beg_x ; i <= end_x ; i++ ) {
	    data[IMGWIDTH*beg_y+i][0] = rgb[col][0] ;
	    data[IMGWIDTH*beg_y+i][1] = rgb[col][1] ;
	    data[IMGWIDTH*beg_y+i][2] = rgb[col][2] ;
	}
}

void ver_line( int beg_x, int beg_y, int end_x, int end_y, int col ) 
{
        int             i ;

        for( i = beg_y ; i <= end_y ; i++ ) {
	    data[IMGWIDTH*i+beg_x][0] = rgb[col][0] ;
	    data[IMGWIDTH*i+beg_x][1] = rgb[col][1] ;
	    data[IMGWIDTH*i+beg_x][2] = rgb[col][2] ;
	}
}

void rectangle( int beg_x, int beg_y, int step_x, int step_y, int col )
{
	hor_line( beg_x, beg_y, beg_x+step_x, beg_y, col ) ;
	ver_line( beg_x+step_x, beg_y, beg_x+step_x, beg_y+step_y, col ) ;
	hor_line( beg_x, beg_y+step_y, beg_x+step_x, beg_y+step_y, col ) ;
	ver_line( beg_x, beg_y, beg_x, beg_y+step_y, col ) ;
}


/* ------------------------------ Main program ---------------------------  */

main(int argc, char **argv)
{
    int    i, j, k ;
    int    file1, file2, reffile ;
    struct stat  file_info ;
    int	   yr1,mo1,da1,hr1,mi1, yr2,mo2,da2,hr2,mi2 ;
    char   tmp_str[80] ;
    FILE   *fp1, *fp2, *fp3 ;
    int    phase_iq[NUMRANGEBINS][2] ;
    float  ref_n, tmp_a, tmp_b, phase, powr ;
    int	   az, r, offset, x, y, az_cor[2], meas_az ;
    float  *change_i, *change_q, *dif_phase_i, *dif_phase_q ;
    struct NFont font ;
    int    histog[2*RangeNIQ+1], histog2[2*RangeNIQ+1], total_count ;
    int    median, mode ;

    /* Check that the num range bins values match */

    if (NumRangeBins != NUMRANGEBINS)
    {
      fprintf(stderr, "Mismatched number of range bins:\n");
      fprintf(stderr, "NumRangeBins = %d\n", NumRangeBins);
      fprintf(stderr, "NUMRANGEBINS = %d\n", NUMRANGEBINS);
      
      exit(0);
    }
    
/* Set-up colors */
    setup() ;
    char *font_env = getenv("REFR_FONT_FILE");
    
    if (font_env == 0)
      sprintf(FontName, "/home/refractt/src/refractt/n_7x12.fnt");
    else
      sprintf(FontName, font_env);
    
    fp1 = fopen( FontName, "rb" ) ;
    if (fp1 == NULL) {
    	fprintf(stderr, "can't open %s\n", FontName);
	exit(-1);
    }
    fread( &font, sizeof(struct NFont), 1, fp1 ) ;
    font.size_x = 7 ;
    font.size_y = 12 ;
    fclose( fp1 ) ;


/* Read the arguments, figure out which files to process */
    if( argc != 3 ) {
	printf("Usage 'phase_diff file1 file2'\n") ;
	exit(1) ;
    }
        zed = (float *) malloc( NUMRANGEBINS * NUMAZIM * sizeof( float ) ) ;
        vel = (float *) malloc( NUMRANGEBINS * NUMAZIM * sizeof( float ) ) ;
        sw = (float *) malloc( NUMRANGEBINS * NUMAZIM * sizeof( float ) ) ;
        snr = (float *) malloc( NUMRANGEBINS * NUMAZIM * sizeof( float ) ) ;
	raw_phase = (struct T_data *)  malloc( NUMAZIM * NUMRANGEBINS * sizeof( struct T_data ) ) ;
        if( raw_phase == NULL ) {
            printf( "Out of memory allocating arrays vel/sw/snr" ) ;
        }

	fp1 = fopen (argv[1], "rb" ) ;
	if( fp1 == NULL ) {
	    printf( "File %s not found.\n", argv[1] ) ;
	    exit( 1 ) ;
	}
	fclose( fp1 ) ;
	fp1 = fopen (argv[2], "rb" ) ;
        if( fp1 == NULL ) {
            printf( "File %s not found.\n", argv[2] ) ;
            exit( 1 ) ;
        }
        fclose( fp1 ) ;

#if (N_ENVIRONMENT ==1)
	read_data_spol( argv[1] ) ;
#elif (N_ENVIRONMENT == 2)
	read_data_spol_rvp8(argv[1]);
#else
	read_data_previraq( argv[1] ) ;
#endif
	yr1 = year ;
	mo1 = month ;
	da1 = day ;
	hr1 = hour ;
	mi1 = minute ;
	for( i = 0 ; i < NUMAZIM*NUMRANGEBINS ; i++ ) {
	    sum_i1[i] = raw_phase[i].inphase ;
	    sum_q1[i] = raw_phase[i].quadrature ;
	}

#if (N_ENVIRONMENT ==1)
	read_data_spol( argv[2] ) ;
#elif (N_ENVIRONMENT == 2)
	read_data_spol_rvp8(argv[2]);
#else
	read_data_previraq( argv[2] ) ;
#endif
        yr2 = year ;
        mo2 = month ;
        da2 = day ;
        hr2 = hour ;
        mi2 = minute ;
        for( i = 0 ; i < NUMAZIM*NUMRANGEBINS ; i++ ) {
            sum_i2[i] = raw_phase[i].inphase ;
            sum_q2[i] = raw_phase[i].quadrature ;
        }

/* Read the data files, compute I & Q differences */
    change_i = (float *) malloc( NUMRANGEBINS * NUMAZIM * sizeof( float ) ) ;
    change_q = (float *) malloc( NUMRANGEBINS * NUMAZIM * sizeof( float ) ) ;
    dif_phase_i = (float *) malloc( NUMRANGEBINS * NUMAZIM * sizeof( float ) ) ;
    dif_phase_q = (float *) malloc( NUMRANGEBINS * NUMAZIM * sizeof( float ) ) ;
    if( dif_phase_q == NULL ) {
	printf("Out of memory allocating work arrays.\n" ) ;
        fclose( fp1 ) ; fclose( fp2 ) ; fclose( fp3 ) ;
        exit( 4 ) ;
    }
    for( az = 0, i = 0 ; az < NUMAZIM ; az++ )
	for( r = 0 ; r < NUMRANGEBINS ; r++, i++ ) {
	    tmp_a = sum_i1[i] ;
	    tmp_b = sum_q1[i] ;
	    change_i[i] = sum_i2[i] * tmp_a + sum_q2[i] * tmp_b ;
	    change_q[i] = sum_i2[i] * tmp_b - sum_q2[i] * tmp_a ;
	    dif_phase_i[i] = sum_i1[i]*sum_i1[i] ;
	    dif_phase_q[i] = sum_q1[i]*sum_q1[i] ;
	}

/* Do the actual phase difference computation and display on display array */

    for( i = 0 ; i <= 2*RangeNIQ ; i++ )
	histog[i] = 0 ;
    total_count = 0 ;
    
    for( y = 0, offset = 0 ; y < NUMAZIM ; y++ )
	for( x = 0, k = IMGWIDTH*(y+5)+5 ; x < NUMRANGEBINS ; x++, k++, offset++ ) {
	    /* Plots the phase difference by first
	     * computing phase (-180 -> 180), remapping it to the
	     * color space (1 -> 60) so that positive phase differences are
	     * red-shifted (hence the 60.5 - ###),
	     * unnecessarily do additional bound checks,
	     * and then set the pixel.
	     */
	    if(( change_i[offset] != 0 ) || ( change_q[offset] != 0 )) {
		phase = atan2( change_q[offset], change_i[offset] ) / DEGTORAD ;
		tmp_a = (LAST_COLOR_INDEX+.5) - (LAST_COLOR_INDEX-FIRST_COLOR_INDEX+1) * (phase+180)/360. ;
		if( tmp_a < FIRST_COLOR_INDEX )  tmp_a = FIRST_COLOR_INDEX ;
		if( tmp_a > LAST_COLOR_INDEX)  tmp_a = LAST_COLOR_INDEX ;
		set_pixel( k, tmp_a ) ;
	    }
	    if(( dif_phase_i[offset] != 0 ) || ( dif_phase_q[offset] != 0 )) {
		/* Compute NIQ properly (for a change), 
		   and remap between 0 and 1. */
                tmp_a = (10. * log10(
			 sqrt(dif_phase_i[offset]*dif_phase_i[offset] +
			      dif_phase_q[offset]*dif_phase_q[offset]))
			 - MinNIQ ) / RangeNIQ ;
                if( tmp_a >= 1. ) {
                    j = WHITE_INDEX ;        /* White if NIQ > Max */
		    tmp_a = 1 ;
		}
                else if( tmp_a > 0. )
		/* Remap to 6-55, with red (color #6) being
		   reserved for the strongest targets */
                    j = (int)((LAST_COLOR_INDEX+.5) - (LAST_COLOR_INDEX-FIRST_COLOR_INDEX+1) * tmp_a) ;
                else {
		/* Blue-purple for weak ones */
                    j = LAST_COLOR_INDEX ;
		    tmp_a = 0 ;
		}

                set_pixel( k+NUMRANGEBINS+70, j ) ;
		histog[(int)(2.*RangeNIQ*tmp_a)]++ ;
		total_count++ ;
	    } // end if non-zero dif_phase
	} // end for

/* Process histogram: compute median and mode */
    for( i = 0, j = 0 ; j < total_count/2 ; i++ )
	j += histog[i] ;
    median = i - 1 ;

    for( i = 1, j = 0 ; i < 2*RangeNIQ ; i++ ) {
	histog2[i] = histog[i-1] + histog[i] + histog[i+1] ;
	if( histog2[i] > j ) {
	    j = histog2[i] ;  mode = i ;
	}
    }

/*  printf("NIQ histogram:\n") ;
    for( i = 0 ; i <= 2*RangeNIQ; i++ )
	printf("%d:\t%d\t", i, histog[i]) ;
    printf("\nMedian = %d; Mode = %d \n", median, mode) ;  */
   
/* Redisplay whose NIQ is within 2 dB of selected threshold as mostly black */
    for( y = 0, offset = 0 ; y < NUMAZIM ; y++ )
	for( x = 0, k = IMGWIDTH*(y+5)+5 ; x < NUMRANGEBINS ; x++, k++, offset++ ) {
	    if(( dif_phase_i[offset] != 0 ) || ( dif_phase_q[offset] != 0 )) {
		/* Compute NIQ properly (for a change), 
		   and remap between 0 and 1. */
                tmp_a = (10. * log10(
			 sqrt(dif_phase_i[offset]*dif_phase_i[offset] +
			      dif_phase_q[offset]*dif_phase_q[offset]))
			 - MinNIQ ) / RangeNIQ ;
                if( tmp_a <= ((float)(median+4)/(2.*RangeNIQ)) )
		    if( ((x%2)==1) || ((y%2) == 1) ) {
		         set_pixel( k+NUMRANGEBINS+70, 0 ) ;
			 set_pixel( k, 0 ) ;
		}
	    }
    }

/* Add "graphics" part of scale */
    for( i = 0 ; i < SIZESCALE ; i++ ) {
        phase = -180 + 360 * i / SIZESCALE ;
        tmp_a = (LAST_COLOR_INDEX+.5) - (LAST_COLOR_INDEX-FIRST_COLOR_INDEX+1) * (phase+180)/360. ;
	if( tmp_a < FIRST_COLOR_INDEX )  tmp_a = FIRST_COLOR_INDEX ;
	if( tmp_a > LAST_COLOR_INDEX)  tmp_a = LAST_COLOR_INDEX ;
        for( j = 75 + NUMAZIM; j < 95 + NUMAZIM ; j++ ) {
            set_pixel( j*IMGWIDTH+i+(NUMRANGEBINS-SIZESCALE)/2, tmp_a ) ;
     	}
	powr = RangeNIQ * i / SIZESCALE ;
	tmp_a = (powr -0) * (LAST_COLOR_INDEX-FIRST_COLOR_INDEX+1) / RangeNIQ  ;
	tmp_a = (LAST_COLOR_INDEX+.5) - tmp_a ;
	if( tmp_a < FIRST_COLOR_INDEX )  tmp_a = WHITE_INDEX ;
	else if( tmp_a > LAST_COLOR_INDEX)  tmp_a = LAST_COLOR_INDEX ;
	for( j = 75 + NUMAZIM; j < 95 + NUMAZIM ; j++ ) {
	    set_pixel( j*IMGWIDTH+i+(NUMRANGEBINS-SIZESCALE)/2+NUMRANGEBINS+70, tmp_a ) ;
	}
    }


/* Add legends, etc */
    sprintf( tmp_str, "Phase difference map" ) ;
    write_text( (int)(5 + NUMRANGEBINS/2 - 3.5 * strlen(tmp_str)), NUMAZIM + 35, tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
    sprintf( tmp_str, "%4d/%02d/%02d %02d:%02d  to  %4d/%02d/%02d %02d:%02d", yr1,mo1,da1,hr1,mi1, yr2,mo2,da2,hr2,mi2 ) ;
    write_text( (int)(5 + NUMRANGEBINS/2 - 3.5 * strlen(tmp_str)), NUMAZIM + 52, tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
    sprintf( tmp_str, "Relative power of (I,Q) vector (NIQ)" ) ;
    write_text( (int)(75 + 3*NUMRANGEBINS/2 - 3.5 * strlen(tmp_str)), NUMAZIM + 35, tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
//    sprintf( tmp_str, "%4d/%02d/%02d %02d:%02d", yr1,mo1,da1,hr1,mi1, ref_n ) ;
    sprintf( tmp_str, "%4d/%02d/%02d %02d:%02d", yr1,mo1,da1,hr1,mi1) ;
    write_text( (int)(75 + 3*NUMRANGEBINS/2 - 3.5 * strlen(tmp_str)), NUMAZIM + 52, tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
    rectangle( 4,4, NUMRANGEBINS+1, NUMAZIM+1, WHITE_INDEX ) ;
    rectangle( NUMRANGEBINS+74,4, NUMRANGEBINS+1, NUMAZIM+1, WHITE_INDEX ) ;

    for( i = 5 ; i <= NUMAZIM + 8 ; i += NUMAZIM/8 ) {
	hor_line( 0, i, 4, i, WHITE_INDEX ) ;
	hor_line( 5+NUMRANGEBINS, i, 9+NUMRANGEBINS, i, WHITE_INDEX ) ;
	hor_line( 70+NUMRANGEBINS, i, 74+NUMRANGEBINS, i, WHITE_INDEX ) ;
	hor_line( 75+2*NUMRANGEBINS, i, 79+2*NUMRANGEBINS, i, WHITE_INDEX ) ;
    }
    sprintf( tmp_str, "NORTH" ) ;  write_text( (int)(IMGWIDTH/2-3.5*strlen(tmp_str)),0,tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
    sprintf( tmp_str, "NE" ) ;  write_text( (int)(IMGWIDTH/2-3.5*strlen(tmp_str)),(int)(NUMAZIM/8),tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
    sprintf( tmp_str, "EAST" ) ;  write_text( (int)(IMGWIDTH/2-3.5*strlen(tmp_str)),(int)(NUMAZIM/4),tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
    sprintf( tmp_str, "SE" ) ;   write_text( (int)(IMGWIDTH/2-3.5*strlen(tmp_str)),(int)(NUMAZIM*3/8),tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
    sprintf( tmp_str, "SOUTH" ) ;  write_text( (int)(IMGWIDTH/2-3.5*strlen(tmp_str)),(int)(NUMAZIM/2),tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
    sprintf( tmp_str, "SW" ) ;  write_text( (int)(IMGWIDTH/2-3.5*strlen(tmp_str)),(int)(NUMAZIM*5/8),tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
    sprintf( tmp_str, "WEST" ) ;  write_text( (int)(IMGWIDTH/2-3.5*strlen(tmp_str)),(int)(NUMAZIM*3/4),tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
    sprintf( tmp_str, "NW" ) ;  write_text( (int)(IMGWIDTH/2-3.5*strlen(tmp_str)),(int)(NUMAZIM*7/8),tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
    sprintf( tmp_str, "NORTH" ) ;  write_text( (int)(IMGWIDTH/2-3.5*strlen(tmp_str)),(int)(NUMAZIM),tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;

    for( j = 0; j <= NUMRANGEBINS*GateSpacing ; j += 10000 ) {
	i = 5 + (int)((float)j / GateSpacing) ;
	ver_line( i, 0, i, 4, WHITE_INDEX ) ;
	ver_line( i, NUMAZIM+5, i, NUMAZIM+9, WHITE_INDEX ) ; 
	ver_line( i+NUMRANGEBINS+70, 0, i+NUMRANGEBINS+70, 4, WHITE_INDEX ) ;
	ver_line( i+NUMRANGEBINS+70, NUMAZIM+5, i+NUMRANGEBINS+70, NUMAZIM+9, WHITE_INDEX ) ;
	sprintf( tmp_str, "%d", (j/1000) ) ;
	write_text( (int)(i - 3.5*strlen(tmp_str)), NUMAZIM+13, tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
	write_text( (int)(i - 3.5*strlen(tmp_str)+NUMRANGEBINS+70), NUMAZIM+13, tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
    }
    sprintf( tmp_str, "km" ) ;
    write_text( i + 15, NUMAZIM+13, tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;

    rectangle( (NUMRANGEBINS-SIZESCALE)/2, NUMAZIM+75, SIZESCALE, 20, WHITE_INDEX) ;
    for( phase = -180 ; phase <= 180 ; phase += 45 ) {
	i = (int)((NUMRANGEBINS-SIZESCALE)/2 + (phase+180)*SIZESCALE/360) ;
	ver_line( i, NUMAZIM+96, i, NUMAZIM+100, WHITE_INDEX ) ;
        sprintf( tmp_str, "%d", (int)phase ) ;
	write_text( (int)(i - 3.5*strlen(tmp_str)), NUMAZIM+104, tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
    }
    sprintf( tmp_str, "deg" ) ;
    write_text( i + 15, NUMAZIM+104, tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;

    rectangle( (NUMRANGEBINS-SIZESCALE)/2+NUMRANGEBINS+70, NUMAZIM+75, SIZESCALE, 20, WHITE_INDEX) ;
    for( powr = MinNIQ ; powr <= MaxNIQ ; powr += NIQScaleStep ) {
	i = (int)((NUMRANGEBINS-SIZESCALE)/2 + (powr-MinNIQ)*SIZESCALE/RangeNIQ) + NUMRANGEBINS+70 ;
	ver_line( i, NUMAZIM+96, i, NUMAZIM+100, WHITE_INDEX ) ;
        sprintf( tmp_str, "%d", (int)powr ) ;
	write_text( (int)(i - 3.5*strlen(tmp_str)), NUMAZIM+104, tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;
    }
sprintf( tmp_str, "dB" );
write_text( i + 15, NUMAZIM+104, tmp_str, &font, WHITE_INDEX, -1, dummy, IMGWIDTH, IMGHEIGHT ) ;

    fp1 = fopen( "tmp.ppm", "wb" ) ;
    if ( !fp1) {
    	fprintf(stderr, "can't open graphic ppm file ");
	exit(1);
    }
    fprintf(fp1, "P6 %d %d 255 ", IMGWIDTH, IMGHEIGHT) ;
    fwrite( data, IMGWIDTH*IMGHEIGHT, 3, fp1 ) ;
    fclose( fp1 ) ;
    system( "display tmp.ppm" ) ;
    unlink( "tmp.ppm" ) ;

    free( snr ) ;
    free( sw ) ;
    free( vel ) ;
#if( NCAR_SPOL )
    free( zed ) ;
#endif

}



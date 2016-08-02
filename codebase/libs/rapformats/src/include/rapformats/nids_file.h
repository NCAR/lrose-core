/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/************************************************************************
 * NIDS_FILE.H Structures & definitions for nids format files 
 * F. Hage Oct 1998.  From Jim Coowie Example - NCAR/COMET
 */

#ifndef NidsFileINCLUDED
#define NidsFileINCLUDED 

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>

/*
 * Maximums for NIDS products
*/

#define NIDS_MXBINS          460
#define NIDS_MXROWLEN        920
#define NIDS_MXRLELEN        230
#define NIDS_MXRADIALS       400


/* NIDS Messgae Header - This is Common to all NIDS PRODUCTS ?*/
typedef struct {
     short   mcode;    /* Message Code */
     short   mdate;    /* Message Date - Days since 12/31/69 */
     int     mtime;    /* Seconds after 12AM GMT */
     int     mlength;  /* Message Lengths - Bytes, including header */
     short   msource;  /* Source ID of Sender */
     short   mdest;    /* Destination ID */
     short   nblocks;  /* Number of blocks in the messge */
     short   divider;  /* Should be 0xffff */
     int     lat;      /* Lat of Radar degrees * 1000 */
     int     lon;      /* Long of Radar degrees * 1000 */
     short   height;   /* Height of Radar - FEET MSL */
     short   pcode;    /* Nids Product Code */
     short   mode;     /*  0=Maint, 1 = Clear Air, 2=Precip */
     short   vcp;      /* Volume Coverage Pattern */
     short   seqnum;   /* Sequence Number of request */
     short   vscan;    /* Volume scan number - Resets at 80 */
     short   vsdate;   /* Scan Date - Days since 12/31/69 */

     /* Broken into 2 shorts to avoid struct size padding */
     short   vstime;   /* Seconds since 12AM GMT - MSW */
     unsigned short vstim2;  /* Seconds since 12AM GMT - LSW  */
     short   pgdate;   /* Prod Generation Date - Days since 12/31/69 */
     int     pgtime;   /* Prod Generation Time - Seconds since 12AM GMT */
     short   pd1;      /* Product Dependant int 1 */
     short   pd2;      /* Product Dependant int 2 */
     short   elevnum;  /* Elevation Number */
     short   pd[24];   /* Product Dependant shorts */
     short   nmaps;    /* Number of map pieces */
     int     soffset;  /* Offset to Symbology data - 2byte halfwords*/
     int     goffset;  /* Offset to Graphic data - 2byte halfwords*/
     int     toffset;  /* Offset to Tabular data - 2byte halfwords*/
     short   bdivider; /* Block Divider = 0xffff */
     short   blockid;  /* Block ID  */
     int     blength;  /* Block Length  */
     short   nlayers;  /* Number of layers */
     short   ldivider; /* Layer Divider = 0xffff */
     int     lendat;   /* Length of data layer */
} NIDS_header_t; 

/* RASTER FORMAT NIDS FILES */
typedef struct { 
     short num_bytes; /* Number of bytes of RLE Data in this row */
     /* num_bytes number of RLE data bytes follow this in the file */
     /* Each RLE byte has 4 bit Run (MSB) and 4 bit value (LSB) */
} NIDS_row_header_t;

typedef struct { 
    short packet_code1;  /* Packet ID - usually -0xBA0F or 0xBA07 */
    short packet_code2;  /* Packet ID - usually -0x8000 */
    short packet_code3;  /* Packet ID - usually -0x00C0 */
    short x_start;       /* Starting location of data (km/4) */
    short y_start;       /* Starting location of data (km/4) */
    short x_scale;       /* Scaling factor for grid (1 to 67) */
    short x_scale_fract; /* factor for grid - PUP-only */
    short y_scale;       /* Scaling factor for grid (1 to 67) */
    short y_scale_fract; /* factor for grid - PUP-only */
    short num_rows;      /* Number of Rows in this product (1 to 464) */
    short packing_descriptor;  /* "Defines packing format 2"  */
    /* num_rtows number of NIDS_beam_header_t follow this in the file */
} NIDS_raster_header_t;

/* RADIAL FORMAT NIDS FILES */
typedef struct { 
     short num_halfwords; /* Number of bytes of RLE Data / 2 in this beam */
     short radial_start_angle; /* Degrees * 10 - (0 to 3599) */
     short radial_delta_angle; /* Degrees * 10 - (0 to 20)   */
     /* num_halfwords * 2 number of RLE data bytes follow this in the file */
     /* Each RLE byte has 4 bit Run (MSB) and 4 bit value (LSB) */
} NIDS_beam_header_t;

typedef struct { 
    short packet_code;  /* Packet ID - Usually 0xAF1F */
    short first_r_bin;  /* Index number of the first range bin */
    short num_r_bin;    /* Number of range bins in each radial beam */
    short i_center;     /* I coordinate of center of sweep (km/4) */
    short j_center;     /* J coordinate of center of sweep (km/4) */
    short scale_factor; /* Number of pixels per range bin * 1000 */
    short num_radials;  /* Number of radials in this product */
    /* num_radials number of NIDS_beam_header_t follow this in the file */
} NIDS_radial_header_t;

/*
 * Byte swapping
 */

/* BE_to_mess_header - convert to native from BE */
extern void NIDS_BE_to_mess_header(NIDS_header_t *nhead);

/* BE_from_mess_header - convert from native to BE */
extern void NIDS_BE_from_mess_header(NIDS_header_t *nhead);

/* BE_to_raster_header - convert to native from BE  */
extern void NIDS_BE_to_raster_header(NIDS_raster_header_t *rhead);

/* BE_from_raster_header - convert from native to BE  */
extern void NIDS_BE_from_raster_header(NIDS_raster_header_t *rhead);

/* BE_to_beam_header - convert to native from BE  */
extern void NIDS_BE_to_beam_header(NIDS_beam_header_t *bhead);

/* BE_from_beam_header - convert from native to BE  */
extern void NIDS_BE_from_beam_header(NIDS_beam_header_t *bhead);

/* BE_to_radial_header - convert to native from BE  */
extern void NIDS_BE_to_radial_header(NIDS_radial_header_t *rhead);

/* BE_from_radial_header - convert from native to BE  */
extern void NIDS_BE_from_radial_header(NIDS_radial_header_t *rhead);

/* BE_to_row_header - convert to native from BE */
void NIDS_BE_to_row_header(NIDS_row_header_t *rowhead);

/* BE_from_row_header - convert from native to BE */
void NIDS_BE_from_row_header(NIDS_row_header_t *rowhead);

/*
 * printing
 */

/*
 * print message header
 */
extern void NIDS_print_mess_hdr(FILE *out, const char *spacer,
				NIDS_header_t *mhead);

/*
 * print raster header
 */
extern void NIDS_print_raster_hdr(FILE *out, const char *spacer,
				  NIDS_raster_header_t *rhead);

/*
 * print beam header
 */
extern void NIDS_print_beam_hdr(FILE *out, const char *spacer,
				NIDS_beam_header_t *bhead);

/*
 * print row header
 */

void NIDS_print_row_hdr(FILE *out, const char *spacer,
			NIDS_row_header_t *rowhead);

/*
 * print radial header
 */
extern void NIDS_print_radial_hdr(FILE *out, const char *spacer,
				  NIDS_radial_header_t *rhead);


/* non-standard byte swapping routines */

extern int NIDS_host_is_big_endian(void);
extern void NIDS_Reverse_4byte_vals(unsigned int* array, int num);
extern void NIDS_Reverse_2byte_vals(unsigned short* array, int num);

extern void swap_nids_header(NIDS_header_t *head);
extern void swap_nids_row_header(NIDS_row_header_t *head);
extern void swap_nids_beam_header(NIDS_beam_header_t *head);
extern void swap_nids_raster_header(NIDS_raster_header_t *head);
extern void swap_nids_radial_header(NIDS_radial_header_t *head);  

#ifdef __cplusplus
}
#endif

#endif

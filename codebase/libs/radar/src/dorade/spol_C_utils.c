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
/* 	$Id: spol_C_utils.c,v 1.6 2018/10/13 22:42:50 dixon Exp $	 */

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/time.h>
# include <string.h>
# include <dirent.h>
# include <ctype.h>
# include <sys/types.h>

# include <radar/dorade/dorade_includes.h>

# ifdef BYTE_ORDER

# if BYTE_ORDER == LITTLE_ENDIAN
int LittleEndian=YES;
# else
int LittleEndian=NO;
# endif

# else
int LittleEndian=NO;
# endif

static char *Stypes[] = { "CAL", "PPI", "COP", "RHI", "VER",
			  "TAR", "MAN", "IDL", "SUR", "AIR", "???" };

# define    type_long  3
# define  type_double  5
# define    type_char  1
# define   type_float  4
# define   type_short  2
# define     type_int  6

# define   offs_ndx 0
# define     id_ndx 1
# define   size_ndx 2
# define    dim_ndx 3
# define    rpt_ndx 4
# define    skp_ndx 5
# define  sparc_ndx 6
# define  ndx_count 7

# define PX2(x) (sig_swap2((&x)))

char *dts_print();
void crackers();
void piraq_crackers();
void swack_short();
void swack_long();
void swack_double();

void piraq_crack_header();
void piraq_crack_radar();
short sig_swap2();


/* c------------------------------------------------------------------------ */

long
time_now()
{
    struct timeval tp;
    struct timezone tzp;

    gettimeofday(&tp,&tzp);
    return((long)tp.tv_sec);
}
/* c------------------------------------------------------------------------ */

char *str_terminate(dst, srs, n)
  CHAR_PTR srs, dst;
  int n;
{
    int m=n;
    char *a=srs, *c=dst;

    *dst = '\0';

    if(n < 1)
	  return(dst);
    /*
     * remove leading blanks
     */
    for(; m && *a == ' '; m--,a++);
    /*
     * copy m char or to the first null
     */
    for(; m-- && *a; *c++ = *a++);
    /*
     * remove trailing blanks
     */
    for(*c='\0'; dst < c && *(c-1) == ' '; *(--c)='\0');

    return(dst);
}
/* c------------------------------------------------------------------------ */

double angdiff( a1, a2 )
  float a1, a2;
{
    double d=a2-a1;

    if( d < -270. )
	  return(d+360.);
    if( d > 270. )
	  return(d-360.);
    return(d);
}
/* c------------------------------------------------------------------------ */

void nex_crack_drdh (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int dst_ndx = sparc_ndx;
   int item_count=48;

   static int crk_drdh [][7] = {
      {    0,    3,    4,    1,    1,    1,    0},
      {    4,    2,    2,    1,   14,   14,    4},
      {    6,    2,    2,    1,    1,    1,    6},
      {    8,    2,    2,    1,    1,    1,    8},
      {   10,    2,    2,    1,    1,    1,   10},
      {   12,    2,    2,    1,    1,    1,   12},
      {   14,    2,    2,    1,    1,    1,   14},
      {   16,    2,    2,    1,    1,    1,   16},
      {   18,    2,    2,    1,    1,    1,   18},
      {   20,    2,    2,    1,    1,    1,   20},
      {   22,    2,    2,    1,    1,    1,   22},
      {   24,    2,    2,    1,    1,    1,   24},
      {   26,    2,    2,    1,    1,    1,   26},
      {   28,    2,    2,    1,    1,    1,   28},
      {   30,    2,    2,    1,    1,    1,   30},
      {   32,    4,    4,    1,    1,    1,   32},
      {   36,    2,    2,    1,   32,   32,   36},
      {   38,    2,    2,    1,    1,    1,   38},
      {   40,    2,    2,    1,    1,    1,   40},
      {   42,    2,    2,    1,    1,    1,   42},
      {   44,    2,    2,    1,    1,    1,   44},
      {   46,    2,    2,    1,    1,    1,   46},
      {   48,    2,    2,    1,    1,    1,   48},
      {   50,    2,    2,    1,    1,    1,   50},
      {   52,    2,    2,    1,    1,    1,   52},
      {   54,    2,    2,    1,    1,    1,   54},
      {   56,    2,    2,    1,    1,    1,   56},
      {   58,    2,    2,    1,    1,    1,   58},
      {   60,    2,    2,    1,    1,    1,   60},
      {   62,    2,    2,    1,    1,    1,   62},
      {   64,    2,    2,    1,    1,    1,   64},
      {   66,    2,    2,    1,    1,    1,   66},
      {   68,    2,    2,    1,    1,    1,   68},
      {   70,    2,    2,    1,    1,    1,   70},
      {   72,    2,    2,    1,    1,    1,   72},
      {   74,    2,    2,    1,    1,    1,   74},
      {   76,    2,    2,    1,    1,    1,   76},
      {   78,    2,    2,    1,    1,    1,   78},
      {   80,    2,    2,    1,    1,    1,   80},
      {   82,    2,    2,    1,    1,    1,   82},
      {   84,    2,    2,    1,    1,    1,   84},
      {   86,    2,    2,    1,    1,    1,   86},
      {   88,    2,    2,    1,    1,    1,   88},
      {   90,    2,    2,    1,    1,    1,   90},
      {   92,    2,    2,    1,    1,    1,   92},
      {   94,    2,    2,    1,    1,    1,   94},
      {   96,    2,    2,    1,    1,    1,   96},
      {   98,    2,    2,    1,    1,    1,   98},
   };
   crackers(srs, dst, item_count, ndx_count, crk_drdh
	    , offs_ndx, dst_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void nex_crack_nmh (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int item_count=8;

   static int crk_nexrad_message_header [][7] = {
      {    0,    2,    2,    1,    1,    1,    0},
      {    2,    1,    1,    1,    1,    1,    2},
      {    3,    1,    1,    1,    1,    1,    3},
      {    4,    2,    2,    1,    2,    2,    4},
      {    6,    2,    2,    1,    1,    1,    6},
      {    8,    3,    4,    1,    1,    1,    8},
      {   12,    2,    2,    1,    2,    2,   12},
      {   14,    2,    2,    1,    1,    1,   14},
   };

   crackers(srs, dst, item_count, ndx_count, crk_nexrad_message_header
	    , offs_ndx, sparc_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void nex_crack_rda (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int item_count=27;

   static int crk_rda_status_info [][7] = {
      {    0,    2,    2,    1,   40,   27,    0},
      {    2,    2,    2,    1,    1,    1,    2},
      {    4,    2,    2,    1,    1,    1,    4},
      {    6,    2,    2,    1,    1,    1,    6},
      {    8,    2,    2,    1,    1,    1,    8},
      {   10,    2,    2,    1,    1,    1,   10},
      {   12,    2,    2,    1,    1,    1,   12},
      {   14,    2,    2,    1,    1,    1,   14},
      {   16,    2,    2,    1,    1,    1,   16},
      {   18,    2,    2,    1,    1,    1,   18},
      {   20,    2,    2,    1,    1,    1,   20},
      {   22,    2,    2,    1,    1,    1,   22},
      {   24,    2,    2,    1,    1,    1,   24},
      {   26,    2,    2,    1,    1,    1,   26},
      {   28,    2,    2,    1,    1,    1,   28},
      {   30,    2,    2,    1,    1,    1,   30},
      {   32,    2,    2,    1,    1,    1,   32},
      {   34,    2,    2,    1,    1,    1,   34},
      {   36,    2,    2,    1,    1,    1,   36},
      {   38,    2,    2,    1,    1,    1,   38},
      {   40,    2,    2,    1,    1,    1,   40},
      {   42,    2,    2,    1,    1,    1,   42},
      {   44,    2,    2,    1,    1,    1,   44},
      {   46,    2,    2,    1,    1,    1,   46},
      {   48,    2,    2,    1,    1,    1,   48},
      {   50,    2,    2,    1,    1,    1,   50},
      {   52,    2,    2,   14,   14,    1,   52},
   };

   crackers(srs, dst, item_count, ndx_count, crk_rda_status_info
	    , offs_ndx, sparc_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void piraq_crack_header (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int item_count=31;

   static int crk_prq_ray_hdr [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    2,    2,    1,    3,    3,    4},
      {    6,    2,    2,    1,    1,    1,    6},
      {    8,    2,    2,    1,    1,    1,    8},
      {   10,    4,    4,    1,    3,    3,   12},
      {   14,    4,    4,    1,    1,    1,   16},
      {   18,    4,    4,    1,    1,    1,   20},
      {   22,    1,    1,    1,    1,    1,   24},
      {   23,    1,    1,    1,    1,    1,   25},
      {   24,    2,    2,    1,    1,    1,   26},
      {   26,    6,    4,    1,    1,    1,   28},
      {   30,    2,    2,    1,    1,    1,   32},
      {   32,    4,    4,    1,    8,    8,   36},
      {   36,    4,    4,    1,    1,    1,   40},
      {   40,    4,    4,    1,    1,    1,   44},
      {   44,    4,    4,    1,    1,    1,   48},
      {   48,    4,    4,    1,    1,    1,   52},
      {   52,    4,    4,    1,    1,    1,   56},
      {   56,    4,    4,    1,    1,    1,   60},
      {   60,    4,    4,    1,    1,    1,   64},
      {   64,    1,    1,    1,    1,    1,   68},
      {   65,    4,    4,    1,    2,    2,   72},
      {   69,    4,    4,    1,    1,    1,   76},
      {   73,    1,    1,    1,    1,    1,   80},
      {   74,    1,    1,    1,    1,    1,   81},
      {   75,    1,    1,    1,    1,    1,   82},
      {   76,    6,    4,    1,    1,    1,   84},
      {   80,    1,    1,    1,    1,    1,   88},
      {   81,    4,    4,    1,    2,    2,   92},
      {   85,    4,    4,    1,    1,    1,   96},
      {   89,    1,    1,  100,  100,    1,  100},
   };

   if(LittleEndian) {
      piraq_crackers(srs, dst, item_count, ndx_count, crk_prq_ray_hdr
		     , offs_ndx, sparc_ndx, limit);
   }
   else {
      crackers(srs, dst, item_count, ndx_count, crk_prq_ray_hdr
	       , offs_ndx, sparc_ndx, limit);
   }
}
# ifdef obsolete
# endif
/* c------------------------------------------------------------------------ */

void piraq_crack_radar (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
# ifdef obsolete
   int item_count=25;

   static int crk_prq_radar_hdr [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    2,    2,    1,    3,    3,    4},
      {    6,    2,    2,    1,    1,    1,    6},
      {    8,    2,    2,    1,    1,    1,    8},
      {   10,    1,    1,    8,    8,    1,   10},
      {   18,    1,    1,    1,    1,    1,   18},
      {   19,    4,    4,    1,   23,   18,   20},
      {   23,    4,    4,    1,    1,    1,   24},
      {   27,    4,    4,    1,    1,    1,   28},
      {   31,    4,    4,    1,    1,    1,   32},
      {   35,    4,    4,    1,    1,    1,   36},
      {   39,    4,    4,    1,    1,    1,   40},
      {   43,    4,    4,    1,    1,    1,   44},
      {   47,    4,    4,    1,    1,    1,   48},
      {   51,    4,    4,    1,    1,    1,   52},
      {   55,    4,    4,    1,    1,    1,   56},
      {   59,    4,    4,    1,    1,    1,   60},
      {   63,    4,    4,    1,    1,    1,   64},
      {   67,    4,    4,    1,    1,    1,   68},
      {   71,    4,    4,    1,    1,    1,   72},
      {   75,    4,    4,    1,    1,    1,   76},
      {   79,    4,    4,    1,    1,    1,   80},
      {   83,    4,    4,    1,    1,    1,   84},
      {   87,    4,    4,    6,    6,    1,   88},
      {  111,    1,    1,  960,  960,    1,  112},
   };
# else
   int item_count=27;

   static int crk_prq_radar_hdr [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    2,    2,    1,    3,    3,    4},
      {    6,    2,    2,    1,    1,    1,    6},
      {    8,    2,    2,    1,    1,    1,    8},
      {   10,    1,    1,    8,    8,    1,   10},
      {   18,    1,    1,    1,    1,    1,   18},
      {   19,    4,    4,    1,   23,   20,   20},
      {   23,    4,    4,    1,    1,    1,   24},
      {   27,    4,    4,    1,    1,    1,   28},
      {   31,    4,    4,    1,    1,    1,   32},
      {   35,    4,    4,    1,    1,    1,   36},
      {   39,    4,    4,    1,    1,    1,   40},
      {   43,    4,    4,    1,    1,    1,   44},
      {   47,    4,    4,    1,    1,    1,   48},
      {   51,    4,    4,    1,    1,    1,   52},
      {   55,    4,    4,    1,    1,    1,   56},
      {   59,    4,    4,    1,    1,    1,   60},
      {   63,    4,    4,    1,    1,    1,   64},
      {   67,    4,    4,    1,    1,    1,   68},
      {   71,    4,    4,    1,    1,    1,   72},
      {   75,    4,    4,    1,    1,    1,   76},
      {   79,    4,    4,    1,    1,    1,   80},
      {   83,    4,    4,    1,    1,    1,   84},
      {   87,    4,    4,    1,    1,    1,   88},
      {   91,    4,    4,    1,    1,    1,   92},
      {   95,    4,    4,    4,    4,    1,   96},
      {  111,    1,    1,  960,  960,    1,  112},
   };
# endif

   if(LittleEndian) {
      piraq_crackers(srs, dst, item_count, ndx_count, crk_prq_radar_hdr
		     , offs_ndx, sparc_ndx, limit);
   }
   else {
      crackers(srs, dst, item_count, ndx_count, crk_prq_radar_hdr
	       , offs_ndx, sparc_ndx, limit);
   }
}
/* c------------------------------------------------------------------------ */

void ddin_crack_vold (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int item_count=18;

   static int crk_volume_d [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    3,    4,    1,    1,    1,    4},
      {    8,    2,    2,    1,    2,    2,    8},
      {   10,    2,    2,    1,    1,    1,   10},
      {   12,    3,    4,    1,    1,    1,   12},
      {   16,    1,    1,   20,   20,    1,   16},
      {   36,    2,    2,    1,    6,    6,   36},
      {   38,    2,    2,    1,    1,    1,   38},
      {   40,    2,    2,    1,    1,    1,   40},
      {   42,    2,    2,    1,    1,    1,   42},
      {   44,    2,    2,    1,    1,    1,   44},
      {   46,    2,    2,    1,    1,    1,   46},
      {   48,    1,    1,    8,    8,    1,   48},
      {   56,    1,    1,    8,    8,    1,   56},
      {   64,    2,    2,    1,    4,    4,   64},
      {   66,    2,    2,    1,    1,    1,   66},
      {   68,    2,    2,    1,    1,    1,   68},
      {   70,    2,    2,    1,    1,    1,   70},
   };

   crackers(srs, dst, item_count, ndx_count, crk_volume_d
	    , offs_ndx, sparc_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void ddin_crack_radd (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int item_count=53;

   static int crk_radar_d [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    3,    4,    1,    1,    1,    4},
      {    8,    1,    1,    8,    8,    1,    8},
      {   16,    4,    4,    1,    8,    8,   16},
      {   20,    4,    4,    1,    1,    1,   20},
      {   24,    4,    4,    1,    1,    1,   24},
      {   28,    4,    4,    1,    1,    1,   28},
      {   32,    4,    4,    1,    1,    1,   32},
      {   36,    4,    4,    1,    1,    1,   36},
      {   40,    4,    4,    1,    1,    1,   40},
      {   44,    4,    4,    1,    1,    1,   44},
      {   48,    2,    2,    1,    2,    2,   48},
      {   50,    2,    2,    1,    1,    1,   50},
      {   52,    4,    4,    1,    3,    3,   52},
      {   56,    4,    4,    1,    1,    1,   56},
      {   60,    4,    4,    1,    1,    1,   60},
      {   64,    2,    2,    1,    4,    4,   64},
      {   66,    2,    2,    1,    1,    1,   66},
      {   68,    2,    2,    1,    1,    1,   68},
      {   70,    2,    2,    1,    1,    1,   70},
      {   72,    4,    4,    1,    7,    7,   72},
      {   76,    4,    4,    1,    1,    1,   76},
      {   80,    4,    4,    1,    1,    1,   80},
      {   84,    4,    4,    1,    1,    1,   84},
      {   88,    4,    4,    1,    1,    1,   88},
      {   92,    4,    4,    1,    1,    1,   92},
      {   96,    4,    4,    1,    1,    1,   96},
      {  100,    2,    2,    1,    2,    2,  100},
      {  102,    2,    2,    1,    1,    1,  102},
      {  104,    4,    4,    1,   10,   10,  104},
      {  108,    4,    4,    1,    1,    1,  108},
      {  112,    4,    4,    1,    1,    1,  112},
      {  116,    4,    4,    1,    1,    1,  116},
      {  120,    4,    4,    1,    1,    1,  120},
      {  124,    4,    4,    1,    1,    1,  124},
      {  128,    4,    4,    1,    1,    1,  128},
      {  132,    4,    4,    1,    1,    1,  132},
      {  136,    4,    4,    1,    1,    1,  136},
      {  140,    4,    4,    1,    1,    1,  140},
      {  144,    3,    4,    1,    1,    1,  144},
      {  148,    1,    1,    8,    8,    1,  148},
      {  156,    3,    4,    1,    1,    1,  156},
      {  160,    4,    4,    1,   29,    9,  160},
      {  164,    4,    4,    1,    1,    1,  164},
      {  168,    4,    4,    1,    1,    1,  168},
      {  172,    4,    4,   11,   11,    1,  172},
      {  216,    4,    4,   11,   11,    1,  216},
      {  260,    4,    4,    1,    1,    1,  260},
      {  264,    4,    4,    1,    1,    1,  264},
      {  268,    4,    4,    1,    1,    1,  268},
      {  272,    4,    4,    1,    1,    1,  272},
      {  276,    3,    4,    1,    1,    1,  276},
      {  280,    1,    1,   20,   20,    1,  280},
   };

   crackers(srs, dst, item_count, ndx_count, crk_radar_d 
	    , offs_ndx, sparc_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void ddin_crack_frib (srs, dst, limit)
  char *srs, *dst;
  int limit;
{

   int item_count=33;

   static int crk_field_radar_i [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    3,    4,    1,    2,    2,    4},
      {    8,    3,    4,    1,    1,    1,    8},
      {   12,    4,    4,    1,   30,   14,   12},
      {   16,    4,    4,    1,    1,    1,   16},
      {   20,    4,    4,    1,    1,    1,   20},
      {   24,    4,    4,    1,    1,    1,   24},
      {   28,    4,    4,    1,    1,    1,   28},
      {   32,    4,    4,    1,    1,    1,   32},
      {   36,    4,    4,    1,    1,    1,   36},
      {   40,    4,    4,    5,    5,    1,   40},
      {   60,    4,    4,    1,    1,    1,   60},
      {   64,    4,    4,    5,    5,    1,   64},
      {   84,    4,    4,    5,    5,    1,   84},
      {  104,    4,    4,    1,    1,    1,  104},
      {  108,    4,    4,    5,    5,    1,  108},
      {  128,    4,    4,    1,    1,    1,  128},
      {  132,    3,    4,    1,    5,    5,  132},
      {  136,    3,    4,    1,    1,    1,  136},
      {  140,    3,    4,    1,    1,    1,  140},
      {  144,    3,    4,    1,    1,    1,  144},
      {  148,    3,    4,    1,    1,    1,  148},
      {  152,    4,    4,    1,    6,    6,  152},
      {  156,    4,    4,    1,    1,    1,  156},
      {  160,    4,    4,    1,    1,    1,  160},
      {  164,    4,    4,    1,    1,    1,  164},
      {  168,    4,    4,    1,    1,    1,  168},
      {  172,    4,    4,    1,    1,    1,  172},
      {  176,    2,    2,    1,    4,    4,  176},
      {  178,    2,    2,    1,    1,    1,  178},
      {  180,    2,    2,    1,    1,    1,  180},
      {  182,    2,    2,    1,    1,    1,  182},
      {  184,    1,    1,   80,   80,    1,  184},
   };

   crackers(srs, dst, item_count, ndx_count, crk_field_radar_i
	    , offs_ndx, sparc_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void ddin_crack_parm (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int item_count=30;

   static int crk_parameter_d [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    3,    4,    1,    1,    1,    4},
      {    8,    1,    1,    8,    8,    1,    8},
      {   16,    1,    1,   40,   40,    1,   16},
      {   56,    1,    1,    8,    8,    1,   56},
      {   64,    2,    2,    1,    2,    2,   64},
      {   66,    2,    2,    1,    1,    1,   66},
      {   68,    4,    4,    1,    1,    1,   68},
      {   72,    2,    2,    1,    4,    4,   72},
      {   74,    2,    2,    1,    1,    1,   74},
      {   76,    2,    2,    1,    1,    1,   76},
      {   78,    2,    2,    1,    1,    1,   78},
      {   80,    1,    1,    8,    8,    1,   80},
      {   88,    4,    4,    1,    3,    3,   88},
      {   92,    4,    4,    1,    1,    1,   92},
      {   96,    4,    4,    1,    1,    1,   96},
      {  100,    3,    4,    1,    2,    2,  100},
      {  104,    3,    4,    1,    1,    1,  104},
      {  108,    1,    1,    8,    8,    1,  108},
      {  116,    3,    4,    1,    2,    2,  116},
      {  120,    3,    4,    1,    1,    1,  120},
      {  124,    4,    4,    1,    1,    1,  124},
      {  128,    3,    4,    1,    1,    1,  128},
      {  132,    1,    1,   32,   32,    1,  132},
      {  164,    3,    4,    1,    1,    1,  164},
      {  168,    1,    1,   32,   32,    1,  168},
      {  200,    3,    4,    1,    1,    1,  200},
      {  204,    4,    4,    1,    3,    3,  204},
      {  208,    4,    4,    1,    1,    1,  208},
      {  212,    4,    4,    1,    1,    1,  212},
   };

   crackers(srs, dst, item_count, ndx_count, crk_parameter_d
	    , offs_ndx, sparc_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void ddin_crack_celv (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int item_count=4;

   static int crk_cell_d [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    3,    4,    1,    2,    2,    4},
      {    8,    3,    4,    1,    1,    1,    8},
      {   12,    4,    4,    1,    1,    1,   12},
   };

   crackers(srs, dst, item_count, ndx_count, crk_cell_d
	    , offs_ndx, sparc_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void ddin_crack_cfac (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int item_count=18;

   static int crk_correction_d [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    3,    4,    1,    1,    1,    4},
      {    8,    4,    4,    1,   16,   16,    8},
      {   12,    4,    4,    1,    1,    1,   12},
      {   16,    4,    4,    1,    1,    1,   16},
      {   20,    4,    4,    1,    1,    1,   20},
      {   24,    4,    4,    1,    1,    1,   24},
      {   28,    4,    4,    1,    1,    1,   28},
      {   32,    4,    4,    1,    1,    1,   32},
      {   36,    4,    4,    1,    1,    1,   36},
      {   40,    4,    4,    1,    1,    1,   40},
      {   44,    4,    4,    1,    1,    1,   44},
      {   48,    4,    4,    1,    1,    1,   48},
      {   52,    4,    4,    1,    1,    1,   52},
      {   56,    4,    4,    1,    1,    1,   56},
      {   60,    4,    4,    1,    1,    1,   60},
      {   64,    4,    4,    1,    1,    1,   64},
      {   68,    4,    4,    1,    1,    1,   68},
   };

   crackers(srs, dst, item_count, ndx_count, crk_correction_d
	    , offs_ndx, sparc_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void ddin_crack_rktb (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int item_count=7;

   static int crk_rot_ang_table [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    3,    4,    1,    1,    1,    4},
      {    8,    4,    4,    1,    1,    1,    8},
      {   12,    3,    4,    1,    4,    4,   12},
      {   16,    3,    4,    1,    1,    1,   16},
      {   20,    3,    4,    1,    1,    1,   20},
      {   24,    3,    4,    1,    1,    1,   24},
   };

   crackers(srs, dst, item_count, ndx_count, crk_rot_ang_table
	    , offs_ndx, sparc_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void ddin_crack_sswbLE (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int item_count=17;

   static int crk_super_SWIB [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    3,    4,    1,    8,    8,    4},
      {    8,    3,    4,    1,    1,    1,    8},
      {   12,    3,    4,    1,    1,    1,   12},
      {   16,    3,    4,    1,    1,    1,   16},
      {   20,    3,    4,    1,    1,    1,   20},
      {   24,    3,    4,    1,    1,    1,   24},
      {   28,    3,    4,    1,    1,    1,   28},
      {   32,    3,    4,    1,    1,    1,   32},
      {   36,    1,    1,    8,    8,    1,   36},
      {   44,    5,    8,    1,    2,    2,   48},
      {   52,    5,    8,    1,    1,    1,   56},
      {   60,    3,    4,    1,   34,    5,   64},
      {   64,    3,    4,    1,    1,    1,   68},
      {   68,    3,    4,    1,    1,    1,   72},
      {   72,    3,    4,    7,    7,    1,   76},
      {  100,    3,    4,   24,   24,    1,  104},
   };

   crackers(srs, dst, item_count, ndx_count, crk_super_SWIB
	    , sparc_ndx, offs_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void ddin_crack_sswb (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int item_count=17;

   static int crk_super_SWIB [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    3,    4,    1,    8,    8,    4},
      {    8,    3,    4,    1,    1,    1,    8},
      {   12,    3,    4,    1,    1,    1,   12},
      {   16,    3,    4,    1,    1,    1,   16},
      {   20,    3,    4,    1,    1,    1,   20},
      {   24,    3,    4,    1,    1,    1,   24},
      {   28,    3,    4,    1,    1,    1,   28},
      {   32,    3,    4,    1,    1,    1,   32},
      {   36,    1,    1,    8,    8,    1,   36},
      {   44,    5,    8,    1,    2,    2,   48},
      {   52,    5,    8,    1,    1,    1,   56},
      {   60,    3,    4,    1,   34,    5,   64},
      {   64,    3,    4,    1,    1,    1,   68},
      {   68,    3,    4,    1,    1,    1,   72},
      {   72,    3,    4,    7,    7,    1,   76},
      {  100,    3,    4,   24,   24,    1,  104},
   };

   crackers(srs, dst, item_count, ndx_count, crk_super_SWIB
	    , offs_ndx, sparc_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void ddin_crack_swib (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int item_count=9;

   static int crk_sweepinfo_d [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    3,    4,    1,    1,    1,    4},
      {    8,    1,    1,    8,    8,    1,    8},
      {   16,    3,    4,    1,    2,    2,   16},
      {   20,    3,    4,    1,    1,    1,   20},
      {   24,    4,    4,    1,    3,    3,   24},
      {   28,    4,    4,    1,    1,    1,   28},
      {   32,    4,    4,    1,    1,    1,   32},
      {   36,    3,    4,    1,    1,    1,   36},
   };

   crackers(srs, dst, item_count, ndx_count, crk_sweepinfo_d
	    , offs_ndx, sparc_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void ddin_crack_ryib (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int item_count=13;

   static int crk_ray_i [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    3,    4,    1,    3,    3,    4},
      {    8,    3,    4,    1,    1,    1,    8},
      {   12,    3,    4,    1,    1,    1,   12},
      {   16,    2,    2,    1,    4,    4,   16},
      {   18,    2,    2,    1,    1,    1,   18},
      {   20,    2,    2,    1,    1,    1,   20},
      {   22,    2,    2,    1,    1,    1,   22},
      {   24,    4,    4,    1,    4,    4,   24},
      {   28,    4,    4,    1,    1,    1,   28},
      {   32,    4,    4,    1,    1,    1,   32},
      {   36,    4,    4,    1,    1,    1,   36},
      {   40,    3,    4,    1,    1,    1,   40},
   };

   crackers(srs, dst, item_count, ndx_count, crk_ray_i
	    , offs_ndx, sparc_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void ddin_crack_asib (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int item_count=20;

   static int crk_platform_i [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    3,    4,    1,    1,    1,    4},
      {    8,    4,    4,    1,   18,   18,    8},
      {   12,    4,    4,    1,    1,    1,   12},
      {   16,    4,    4,    1,    1,    1,   16},
      {   20,    4,    4,    1,    1,    1,   20},
      {   24,    4,    4,    1,    1,    1,   24},
      {   28,    4,    4,    1,    1,    1,   28},
      {   32,    4,    4,    1,    1,    1,   32},
      {   36,    4,    4,    1,    1,    1,   36},
      {   40,    4,    4,    1,    1,    1,   40},
      {   44,    4,    4,    1,    1,    1,   44},
      {   48,    4,    4,    1,    1,    1,   48},
      {   52,    4,    4,    1,    1,    1,   52},
      {   56,    4,    4,    1,    1,    1,   56},
      {   60,    4,    4,    1,    1,    1,   60},
      {   64,    4,    4,    1,    1,    1,   64},
      {   68,    4,    4,    1,    1,    1,   68},
      {   72,    4,    4,    1,    1,    1,   72},
      {   76,    4,    4,    1,    1,    1,   76},
   };

   crackers(srs, dst, item_count, ndx_count, crk_platform_i
	    , offs_ndx, sparc_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void ddin_crack_qdat (srs, dst, limit)
  char *srs, *dst;
  int limit;
{
   int item_count=8;

   static int crk_qparamdata_d [][7] = {
      {    0,    1,    1,    4,    4,    1,    0},
      {    4,    3,    4,    1,    1,    1,    4},
      {    8,    1,    1,    8,    8,    1,    8},
      {   16,    3,    4,    1,    2,    2,   16},
      {   20,    3,    4,    1,    1,    1,   20},
      {   24,    2,    2,    4,    8,    2,   24},
      {   32,    2,    2,    4,    4,    1,   32},
      {   40,    4,    4,    4,    4,    1,   40},
   };

   crackers(srs, dst, item_count, ndx_count, crk_qparamdata_d
	    , offs_ndx, sparc_ndx, limit);
}
/* c------------------------------------------------------------------------ */

void crackers(srs, dst, item_count, ndx_cnt, tbl, srs_ndx, dst_ndx, limit)
  char *srs, *dst;
  const int item_count, ndx_cnt;
  int *tbl;
  int srs_ndx, dst_ndx, limit;
{
   int itm, *row = tbl, offs;
   char *ss=srs, *tt=dst;
   char *deep6 = 0;


   for(itm=0; itm < item_count;) {

      switch(*(row + id_ndx)) {

       case type_char:		/* just copy it */
	 memcpy(tt, ss, *(row + rpt_ndx));
	 break;

       case type_short:

	 if(*(row + rpt_ndx) > 1) {
	    swack_short(ss, tt, *(row + rpt_ndx));
	 }
	 else {			/* just swap one item */
	    *tt++ = *(ss+1);
	    *tt++ = *ss;
	 }
	 break;

       case type_long:
       case type_float:
       case type_int:

	 if(*(row + rpt_ndx) > 1) {
	    swack_long(ss, tt, *(row + rpt_ndx));
	 }
	 else {			/* just swap one item */
	    *tt++ = *(ss+3);
	    *tt++ = *(ss+2);
	    *tt++ = *(ss+1);
	    *tt++ = *ss;
	 }
	 break;

       case type_double:
	 swack_double(ss, tt, *(row + rpt_ndx));
	 break;

       default:
	 *deep6 = 0;
	 break;

      }
      itm += *(row + skp_ndx);
      row = tbl + itm * ndx_cnt;
      offs = *(row + srs_ndx);
      if( limit > 0 && offs >= limit )
	  { break; }
      ss = srs + (*(row + srs_ndx));
      tt = dst + (*(row + dst_ndx));
   }
}
/* c------------------------------------------------------------------------ */

void piraq_crackers
(srs, dst, item_count, ndx_cnt, tbl, srs_ndx, dst_ndx, limit)
  char *srs, *dst;
  int item_count, ndx_cnt, *tbl;
  int srs_ndx, dst_ndx, limit;
{
   int itm, *row = tbl;
   char *ss=srs, *tt=dst;

   for(itm=0; itm < item_count;) {

      switch(*(row + id_ndx)) {

       case type_char:		/* just copy it */
	 memcpy(tt, ss, *(row + rpt_ndx));
	 break;

       case type_short:

	 memcpy(tt, ss, *(row + rpt_ndx) * sizeof(short));
	 break;

       case type_long:
       case type_float:
       case type_int:

	 memcpy(tt, ss, *(row + rpt_ndx) * sizeof(long));
	 break;

       case type_double:	/* same as unpacking two floats? */

	 memcpy(tt, ss, *(row + rpt_ndx) * sizeof(double));
	 break;

       default:
	 break;

      }
      itm += *(row + skp_ndx);
      row = tbl + itm * ndx_cnt;
      ss = srs + (*(row + srs_ndx));
      tt = dst + (*(row + dst_ndx));
   }
}
/* c------------------------------------------------------------------------ */

void swack_short(ss, tt, nn)
  char *ss, *tt;
  int nn;
{
   for(; nn--;) {
      *tt++ = *(ss+1);
      *tt++ = *ss;
      ss += 2;
   }
}
/* c------------------------------------------------------------------------ */

void swack2(ss, tt)
  char *ss, *tt;
{
   *tt++ = *(ss+1);
   *tt = *ss;
}
/* c------------------------------------------------------------------------ */

void swack_long(ss, tt, nn)
  char *ss, *tt;
  int nn;
{
   for(; nn--;) {
      *tt++ = *(ss+3);
      *tt++ = *(ss+2);
      *tt++ = *(ss+1);
      *tt++ = *ss;
      ss += 4;
   }
}
/* c------------------------------------------------------------------------ */

void swack4(ss, tt)
  char *ss, *tt;
{
   *tt++ = *(ss+3);
   *tt++ = *(ss+2);
   *tt++ = *(ss+1);
   *tt = *ss;
}
/* c------------------------------------------------------------------------ */

void swack_double(ss, tt, nn)
  char *ss, *tt;
  int nn;
{
   for(; nn--;) {
      *tt++ = *(ss+7);
      *tt++ = *(ss+6);
      *tt++ = *(ss+5);
      *tt++ = *(ss+4);
      *tt++ = *(ss+3);
      *tt++ = *(ss+2);
      *tt++ = *(ss+1);
      *tt++ = *ss;
      ss += 8;
   }
}
/* c------------------------------------------------------------------------ */

int swap4( char *ov )		/* swap integer*4 */
{
    union {
	long newval;
	char nv[4];
    }u;
    u.nv[3] = *ov++; u.nv[2] = *ov++; u.nv[1] = *ov++; u.nv[0] = *ov++;
    return(u.newval);
}
/* c------------------------------------------------------------------------ */

short sig_swap2( ov )		/* swap integer*2 */
  twob *ov;
{
    union {
	short newval;
	char nv[2];
    }u;
    u.nv[1] = ov->one; u.nv[0] = ov->two;
    return(u.newval);
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

int ffb_read( fin, buf, count )
    FILE * fin;
  int count;
  char *buf;
{
    /* fortran-binary read routine
     */
    long int size_rec=0, rlen1, rlen2=0, nab;

    /* read the record size */
    rlen1 = fread((void *)&nab, 1, sizeof(nab), fin );

    if( rlen1 < sizeof(nab))
	  return(rlen1);

    if(LittleEndian) {
      swack4((char *) &nab, (char *) &size_rec);
    }
    else {
       size_rec = nab;
    }

    if( size_rec > 0 ) {
	/* read the record
	 * (the read may be less than the size of the record)
	 */
	rlen2 = size_rec <= count ? size_rec : count;

	rlen1 = fread( (void *)buf, sizeof(char), rlen2, fin );
	if( rlen1 < 1 )
	      return(rlen1);
	rlen2 = rlen1 < size_rec ?
	      size_rec-rlen1 : 0; /* set up skip to end of record */
    }
    else
	  rlen1 = 0;
    
    rlen2 += sizeof(size_rec);

    /* skip thru the end of record */
    rlen2 = fseek( fin, rlen2, SEEK_CUR );
    rlen2 = ftell( fin );
    return(rlen1);
}
/* c------------------------------------------------------------------------ */

int ffb_write( fout, buf, count )
     FILE * fout;
     int count;
     char *buf;
{
    /* fortran-binary write routine
     */
    size_t size_rec=count, blip;

    if(LittleEndian) {
      swack4((char *) &size_rec, (char *) &blip);
    }
    else {
       blip = size_rec;
    }
    /* write the record length */
    fwrite( (void *)&blip, sizeof(blip), 1, fout );

    /* write the record */
    if( size_rec > 0 ) {
      fwrite( (void *)buf, size_rec, 1, fout );
    }
    /* write the record length */
    fwrite( (void *)&blip, sizeof(blip), 1, fout );

    return( size_rec );
}
/* c------------------------------------------------------------------------ */

int fb_write( fout, buf, count )
  int fout, count;
  char *buf;
{
    /* fortran-binary write routine
     */
    long int size_rec=count, rlen1, rlen2=0, blip;

    if(LittleEndian) {
      swack4((char *) &size_rec, (char *) &blip);
    }
    else {
       blip = size_rec;
    }
    /* write the record length */
    rlen1 = write(fout, &blip, sizeof(blip));
    if( rlen1 < sizeof(size_rec))
	  return(rlen1);

    if( size_rec > 0 ) {
	/* write the record */
	rlen1 = write (fout, buf, size_rec);
	if( rlen1 < 1 )
	      return(rlen1);
    }
    /* write the record length */
    rlen2 = write (fout, &blip, sizeof(blip));
    if( rlen2 < sizeof(blip))
	  return(rlen2);

    else if(size_rec == 0)
	  return(0);
    else
	  return(rlen1);
}
/* c------------------------------------------------------------------------ */

int fb_read( fin, buf, count )
  int fin, count;
  char *buf;
{
    /* fortran-binary read routine
     */
    long int size_rec=0, rlen1, rlen2=0, nab;

    /* read the record size */
    rlen1 = read (fin, &nab, sizeof(nab));

    if( rlen1 < sizeof(nab))
	  return(rlen1);

    if(LittleEndian) {
       swack4( (char*)&nab, (char*)&size_rec);
    }
    else {
       size_rec = nab;
    }

    if( size_rec > 0 ) {
	/* read the record
	 * (the read may be less than the size of the record)
	 */
	rlen2 = size_rec <= count ? size_rec : count;

	rlen1 = read (fin, buf, rlen2);	/* read it! */
	if( rlen1 < 1 )
	      return(rlen1);
	rlen2 = rlen1 < size_rec ?
	      size_rec-rlen1 : 0; /* set up skip to end of record */
    }
    else
	  rlen1 = 0;
    
    rlen2 += sizeof(size_rec);

    /* skip thru the end of record */
    rlen2 = lseek( fin, rlen2, 1 );
    return(rlen1);
}
/* c------------------------------------------------------------------------ */

int dd_tokens(att, str_ptrs)
  char *att, *str_ptrs[];
{
    int nargs=0;
    char *b=att, *dlims=" \t\n"; /* blank, tab, and new line */

    for(b=strtok(b, dlims);  b; b = strtok(NULL, dlims)) {
	str_ptrs[nargs++] = b;
    }
    return(nargs);
}
/* c------------------------------------------------------------------------ */

int dd_tokenz(att, str_ptrs, dlims)
  char *att, *str_ptrs[], *dlims; /* token delimiters */
{
    int nargs=0;
    char *b=att;
    char *strtok();

    for(b=strtok(b, dlims); b;  b = strtok(NULL, dlims)) {
	str_ptrs[nargs++] = b;
    }
    return(nargs);
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

# ifndef SOLO_LIST_MGMT
# define SOLO_LIST_MGMT
# define SLM_CODE

struct solo_list_mgmt {
    int num_entries;
    int sizeof_entries;
    int max_entries;
    char **list;
};
/* c------------------------------------------------------------------------ */

struct solo_str_mgmt {
    struct solo_str_mgmt *last;
    struct solo_str_mgmt *next;
    char *at;
};
# endif
/* c------------------------------------------------------------------------ */

struct solo_list_mgmt *
solo_malloc_list_mgmt(sizeof_entries)
  int sizeof_entries;
{
    struct solo_list_mgmt *slm;

    slm = (struct solo_list_mgmt *)malloc(sizeof(struct solo_list_mgmt));
    memset(slm, 0, sizeof(struct solo_list_mgmt));
    slm->sizeof_entries = sizeof_entries;
    return(slm);
}
/* c------------------------------------------------------------------------ */

void solo_unmalloc_list_mgmt(slm)
  struct solo_list_mgmt *slm;
{
    int ii;

    if(!slm)
	  return;

    for(ii=0; ii < slm->max_entries; ii++) {
	free(*(slm->list +ii));
    }
    free(slm);
}
/* c------------------------------------------------------------------------ */

void solo_reset_list_entries(which)
  struct solo_list_mgmt *which;
{
    int ii;

    if(!which || !which->num_entries)
	  return;

    for(ii=0; ii < which->num_entries; ii++) {
	strcpy(*(which->list +ii), " ");
    }
    which->num_entries = 0;
}
/* c------------------------------------------------------------------------ */

void solo_add_list_entry(which, entry, len)
  struct solo_list_mgmt *which;
  char *entry;
  int len;
{
    int ii;
    char *a, *c, *str_terminate();

    if(!which)
	  return;

    if(!len) len = strlen(entry);

    if(which->num_entries >= which->max_entries) {
	which->max_entries += 30;
	if(which->list) {
	    which->list = (char **)realloc
		  (which->list, which->max_entries*sizeof(char *));
	}
	else {
	    which->list = (char **)malloc(which->max_entries*sizeof(char *));
	}
	for(ii=which->num_entries; ii < which->max_entries; ii++) {
	    *(which->list+ii) = a = (char *)malloc(which->sizeof_entries+1);
	    *a = '\0';
	}
    }
    len = len < which->sizeof_entries ? len : which->sizeof_entries;
    c = *(which->list+which->num_entries++);

    if((a = entry)) {
	for(; *a && len--; *c++ = *a++);
    }
    *c = '\0';
}
/* c------------------------------------------------------------------------ */

char *
solo_list_entry(which, entry_num)
  struct solo_list_mgmt *which;
  int entry_num;
{
    char *c;

    if(!which || entry_num >= which->num_entries || entry_num < 0)
	  return(NULL);

    c = *(which->list+entry_num);
    return(c);
}
/* c------------------------------------------------------------------------ */

void solo_sort_strings(char **, int);

/* c------------------------------------------------------------------------ */

int get_tmp_swp_files(dir, lm)
  char *dir;
  struct solo_list_mgmt *lm;
{
    /* tries to create a list of files in a directory
     */
    DIR *dir_ptr;
    struct dirent *dp;
    char mess[256];
    char *aa, *bb, *tmp = ".tmp";

    lm->num_entries = 0;

    if(!(dir_ptr = opendir(dir))) {
	sprintf(mess, "Cannot open directory %s\n", dir);
	printf( "%s", mess );
	return(-1);
    }

    for(;;) {
	dp=readdir(dir_ptr);
	if(dp == NULL ) {
	    break;
	}
	aa = dp->d_name;
	if(strncmp(aa, "swp", 3))
	   { continue; }	/* does not compare! */
	bb = aa + strlen(aa) - strlen(tmp);
	if(!strncmp(bb, tmp, strlen(tmp))) {
	  /* nab "swp.*.tmp" files */
	  solo_add_list_entry(lm, dp->d_name, strlen(dp->d_name));
	}

    }
    closedir(dir_ptr);
    if(lm->num_entries > 1)
	  solo_sort_strings(lm->list, lm->num_entries);

    return(lm->num_entries);
}
/* c------------------------------------------------------------------------ */

int generic_sweepfiles( dir, lm, prefix, suffix, not_this_suffix )
  char *dir, *prefix, *suffix;
  struct solo_list_mgmt *lm;
  int not_this_suffix;
{
    /* tries to create a list of files in a directory
     */
    DIR *dir_ptr;
    struct dirent *dp;
    char mess[256];
    char *aa, *bb, *pfx="swp", *sfx="";
    int pfx_len = strlen( prefix );
    int sfx_len = strlen( suffix );
    int mark;

    if( !pfx_len )
      { pfx_len = strlen( pfx ); }
    else if (!strncmp (prefix, "NONE", 4)) {
      pfx_len = 0;
    }
    else
      { pfx = prefix; }

    if( sfx_len )
      { sfx = suffix; }

    lm->num_entries = 0;

    if(!(dir_ptr = opendir(dir))) {
	sprintf(mess, "Cannot open directory %s\n", dir);
	printf( "%s", mess );
	return(-1);
    }

    for(;;) {
	dp=readdir(dir_ptr);
	if(dp == NULL ) {
	    break;
	}
	aa = dp->d_name;
	if(pfx_len && strncmp(aa, pfx, pfx_len))
	   { continue; }	/* does not have this prefix */

	if( sfx_len ) {
	  bb = aa + strlen(aa) - sfx_len;
	  mark = !strncmp( bb, sfx, sfx_len );

	  if( not_this_suffix ) {
	    if( mark )
	      { continue; } /* don't want files with this suffix */
	  }
	  else if( !mark )
	    { continue; }/* doesn't have this suffix */
	}

	/* passes all tests */
	solo_add_list_entry(lm, dp->d_name, strlen(dp->d_name));
    }
    closedir(dir_ptr);
    if(lm->num_entries > 1)
	  solo_sort_strings(lm->list, lm->num_entries);

    return(lm->num_entries);
}
/* c------------------------------------------------------------------------ */

char *solo_modify_list_entry(which, entry, len, entry_num)
  struct solo_list_mgmt *which;
  char *entry;
  int len;
{
    char *a, *c, *str_terminate();

    if(!which || entry_num > which->num_entries || entry_num < 0)
	  return(NULL);

    if(!len) len = strlen(entry);

    if(entry_num == which->num_entries) { /* this entry doesn't exist
					   * but it can be added */
	solo_add_list_entry(which, entry, len);
	a = *(which->list+entry_num);
	return(a);
    }
    len = len < which->sizeof_entries ? len : which->sizeof_entries;
    a = c = *(which->list+entry_num);

    for(a=entry; *a && len--; *c++ = *a++);
    *c = '\0';
    return(a);
}
/* c------------------------------------------------------------------------ */

int get_swp_files(dir, lm)
  char *dir;
  struct solo_list_mgmt *lm;
{
    /* tries to create a list of files in a directory
     */
    DIR *dir_ptr;
    struct dirent *dp;
    char mess[256];
    char *aa, *bb, *tmp = ".tmp";

    lm->num_entries = 0;

    if(!(dir_ptr = opendir(dir))) {
	sprintf(mess, "Cannot open directory %s\n", dir);
	printf( "%s", mess );
	return(-1);
    }

    for(;;) {
	dp=readdir(dir_ptr);
	if(dp == NULL ) {
	    break;
	}
	aa = dp->d_name;
	if(strncmp(aa, "swp", 3))
	   { continue; }	/* does not compare! */
	bb = aa + strlen(aa) - strlen(tmp);
	if(!strncmp(bb, tmp, strlen(tmp)))
	    { continue; }	/* ignore ".tmp" files */

	solo_add_list_entry(lm, dp->d_name, strlen(dp->d_name));
    }
    closedir(dir_ptr);
    if(lm->num_entries > 1)
	  solo_sort_strings(lm->list, lm->num_entries);

    return(lm->num_entries);
}
/* c------------------------------------------------------------------------ */

void solo_sort_strings(sptr, ns)
  char **sptr;
  int ns;
{
    int ii, jj;
    char *keep;

    for(ii=0; ii < ns-1; ii++) {
	for(jj=ii+1; jj < ns; jj++) {
	    if(strcmp(*(sptr+jj), *(sptr+ii)) < 0) {
		keep = *(sptr+ii);
		*(sptr+ii) = *(sptr+jj);
		*(sptr+jj) = keep;
	    }
	}
    }
}
/* c------------------------------------------------------------------------ */

int dd_hrd16_uncompressx( ss, dd, flag, empty_run, wmax )
  short *ss, *dd;
  int flag, *empty_run, wmax;
{
    /*
     * routine to unpacks actual data assuming MIT/HRD compression where:
     * ss points to the first 16-bit run-length code for the compressed data
     * dd points to the destination for the unpacked data
     * flag is the missing data flag for this dataset that is inserted
     *     to fill runs of missing data.
     * empty_run pointer into which the number of missing 16-bit words
     *    can be stored. This will be 0 if the last run contained data.
     # wmax indicate the maximum number of 16-bit words the routine can
     *    unpack. This should stop runaways.
     */
    int n, wcount=0;

    while(*ss != 1) {		/* 1 is an end of compression flag */
	n = *ss & 0x7fff;	/* nab the 16-bit word count */
	if(wcount+n > wmax) {
	    printf("Uncompress failure %d %d %d\n"
		   , wcount, n, wmax);
	    break;
	}
	else {
	    wcount += n;		/* keep a running tally */
	}
	if( *ss & 0x8000 ) {	/* high order bit set implies data! */
	    *empty_run = 0;
	    ss++;
	    for(; n--;) {
		*dd++ = *ss++;
	    }
	}	
	else {			/* otherwise fill with flags */
	    *empty_run = n;
	    ss++;
	    for(; n--;) {
		*dd++ = flag;
	    }
	}
    }
    return(wcount);
}
/* c------------------------------------------------------------------------ */

int dd_hrd16LE_uncompressx( ss, dd, flag, empty_run, wmax )
  unsigned short *ss, *dd;
  int flag, *empty_run, wmax;
{
    /*
     * routine to unpacks actual data assuming MIT/HRD compression where:
     * ss points to the first 16-bit run-length code for the compressed data
     * dd points to the destination for the unpacked data
     * flag is the missing data flag for this dataset that is inserted
     *     to fill runs of missing data.
     * empty_run pointer into which the number of missing 16-bit words
     *    can be stored. This will be 0 if the last run contained data.
     # wmax indicate the maximum number of 16-bit words the routine can
     *    unpack. This should stop runaways.
     */
    int n, wcount=0;
    unsigned short rlcw;
    unsigned char *aa, *bb;
    
    aa = (unsigned char *)&rlcw;

    for(;;) {	
       bb = (unsigned char *)ss;
       *aa = *(bb+1);
       *(aa+1) = *bb;		/* set run length code word "rlcw" */
       if(rlcw == 1) { break; }	/* 1 is the end of compression flag */

       n = rlcw & 0x7fff;	/* nab the 16-bit word count */
       if(wcount+n > wmax) {
	  printf("Uncompress failure %d %d %d\n"
		 , wcount, n, wmax);
	  break;
       }
       else {
	  wcount += n;		/* keep a running tally */
       }
       if( rlcw & 0x8000 ) {	/* high order bit set implies data! */
	  *empty_run = 0;
	  ss++;
	  swack_short((char*)ss, (char*)dd, n);
	  ss += n;
	  dd += n;
	  
       }	
       else {			/* otherwise fill with flags */
	  *empty_run = n;
	  ss++;
	  for(; n--;) {
	     *dd++ = flag;
	  }
       }
    }
    return(wcount);
}
/* c------------------------------------------------------------------------ */

int dd_compress( src, dst, flag, n )
  unsigned short *src, *dst, flag;
  int n;
{
    /* implement hrd compression of 16-bit values
     * and return the number of 16-bit words of compressed data
     */
    int kount=0, wcount=0, data_run = 0;
    unsigned short *ss=src, *dd=dst;
    unsigned short *rlcode = NULL, *end=src+n-1;

    if(n < 2) {
	printf("Trying to compress less than 2 values\n");
	exit(1);
    }

    for(;ss < end;) {
	/* for each run examine the first two values
	 */
	kount = 2;
	rlcode = dd++;
	if(*(ss+1) != flag || *ss != flag) { /* data run */
	    data_run = YES;
	    *dd++ = *ss++;
	    *dd++ = *ss++;
	}
	else { /* flag run */
	    data_run = NO;
	    ss += 2;
	}

	for(;ss < end;) { /* for rest of the run */
	    if(data_run) {
		if(*(ss-1) == flag && *ss == flag && kount > 2) {
		    /* break data run
		     */
		    *rlcode = SIGN16 | --kount;
		    wcount += kount+1; /* data plus code word */
		    ss--;
		    dd--;
		    break;
		}
		/* continue the data run */
		kount++;
		*dd++ = *ss++;
	    }
	    else { /* flag run */
		if(*ss != flag) { /* break flag run */
		    *rlcode = kount;
		    wcount++; /* code word only */
		    break;
		}
		ss++;
		kount++; /* continue flag run */
	    }
	}
    }
    /* now look at the last value
     */
    if(data_run) { /* just stuff it no matter what it is */
	if(ss == end) {
	    *dd++ = *ss;
	    kount++;
	}
	*rlcode = SIGN16 | kount;
	wcount += kount +1;
    }
    else if(*ss == flag) {
	*rlcode = ++kount;
	wcount++;
    }
    else { /* there is one last datum at the end of a flag run */
	if(kount == 2) {	/* special case for just two flags */
	    *rlcode = SIGN16 | 3;
	    *dd++ = flag;
	    *dd++ = flag;
	    *dd++ = *ss;
	    wcount += 4;
	}
	else {
	    *rlcode = --kount;
	    wcount++;
	    *dd++ = SIGN16 | 2;
	    *dd++ = flag;
	    *dd++ = *ss;
	    wcount += 3;
	}
    }
    *dd++ = END_OF_COMPRESSION;
    wcount++;
    return(wcount);
}
/* c------------------------------------------------------------------------ */

int
dd_scan_mode(str)
  char *str;
{
    char uc_scan_mode[16];
    int max_scan_mode = 10, ii, nn;
    char *aa;

    if( !str )
      { return(-1); }
    if( !(nn = strlen(str)))
      { return(-1); }
    
    aa = uc_scan_mode;
    strcpy( aa, str );
    for(; !*aa ; aa++ )
      {	*aa = (char)toupper((int)(*aa)); }

    for( ii = 0; ii < max_scan_mode; ii++ )
      {
	if( !strcmp( uc_scan_mode, Stypes[ii] ))
	  { return( ii ); }
      }
    return( -1 );
}
/* c------------------------------------------------------------------------ */

char *
dd_scan_mode_mne(scan_mode, str)
  int scan_mode;
  char *str;
{
    int max_scan_mode = 10;

    scan_mode = scan_mode < 0 || scan_mode > max_scan_mode
	  ? max_scan_mode : scan_mode;

    strcpy(str, Stypes[scan_mode]);
    return(str);
}
/* c------------------------------------------------------------------------ */

char *
dd_radar_type_ascii(radar_type, str)
  int radar_type;
  char *str;
{
    int max_radar_types = 10;
    static char *Rtypes[] = { "GROUND", "AIR_FORE", "AIR_AFT", "AIR_TAIL"
			      , "AIR_LF", "SHIP", "AIR_NOSE", "SATELLITE"
			      , "LIDAR_MOVING", "LIDAR_FIXED", "UNKNOWN"
			      };

    radar_type = radar_type < 0 || radar_type > max_radar_types
	  ? max_radar_types : radar_type;

    strcpy(str, Rtypes[radar_type]);
    return(str);
}
/* c------------------------------------------------------------------------ */

char *slash_path( dst, srs )
  char *dst, *srs;
{
    int n;

    if(srs && (n=strlen(srs))) {
	strcpy(dst,srs);
	if(dst[n-1] != '/' ) {
	    dst[n] = '/';
	    dst[n+1] = '\0';
	}
    }
    else
	  *dst = '\0';
    return(dst);
}
/* c----------------------------------------------------------------------- */
/* routines used in tranformations between (lat,lon,alt) and (x,y,z) */
/* c------------------------------------------------------------------------ */

int d_inSector( double ang, double ang1, double ang2)
{
    // assumes sector defined from ang1 clockwise to ang2

    if(ang1 > ang2)		// crosses 360.
	{ return(ang >= ang1 || ang < ang2); }

    return(ang >= ang1 && ang < ang2);
}

/* c------------------------------------------------------------------------ */

double d_angdiff( a1, a2 )
  double a1, a2;
{
    double d=a2-a1;

    if( d < -180. )
	  return(d+360.);
    if( d > 180. )
	  return(d-360.);
    return(d);
}
/* c------------------------------------------------------------------------ */

double
dd_earthr(lat)
  double lat;
{
    static double *earth_r=NULL;

    double major=6378388;	/* radius in meters */
    double minor=6356911.946;
    double tt;
    double d, theta=0, x, y;
    int ii, nn;
    
    if(!earth_r) {
	earth_r = (double *)malloc(20*sizeof(double));

	for(ii=0; theta < 90.; ii++, theta += 5.) {
	    /* create an entry every 5 degrees
	     */
	    tt = tan(RADIANS(theta));
	    d = sqrt(1.+SQ(tt*major/minor));
	    x = major/d;
	    y = x*tt;
	    *(earth_r +ii) = sqrt(SQ(x) + SQ(y));
	}
    }
    nn = fabs(lat*.2);
    d = nn < 18 ? *(earth_r +nn) : minor;
    d *= .001;			/* km.! */
    return(d);
}
/*c----------------------------------------------------------------------*/

int loop_ll2xy_v3( double *plat, double *plon, double *palt
		   , double *x, double *y, double *z
		   , double olat, double olon, double oalt
		   , double R_earth, int num_pts )
{
    /* calculate (x,y,z) of (plat,plon) relative to (olat,olon) */
    /* all dimensions in km. */

    /* transform to earth coordinates and then to lat/lon/alt */

    /* These calculations are from the book
     * "Aerospace Coordinate Systems and Transformations"
     * by G. Minkler/J. Minkler
     * these are the ECEF/ENU point transformations
     */

    int nn = num_pts;
    double delta_o, lambda_o, R_p, R_p_pr, delta_p, lambda_p;
    double xe, ye, ze, sinLambda, cosLambda, sinDelta, cosDelta;
    double h, a, b, c;


    h = R_earth + oalt;
    delta_o = RADIANS( olat );	/* read delta sub oh */
    lambda_o = RADIANS( olon );

    sinLambda = sin( lambda_o );
    cosLambda = cos( lambda_o );
    sinDelta = sin( delta_o );
    cosDelta = cos( delta_o );
    /*
    printf( "\n" );
     */
    
    for(; nn--; plat++, plon++, palt++, x++, y++, z++ ) {

       R_p = R_earth + (*palt);
       delta_p = RADIANS( *plat );
       lambda_p = RADIANS( *plon );
       R_p_pr = R_p * cos( delta_p );

       xe = R_p * sin( delta_p );
       ye = -R_p_pr * sin( lambda_p );
       ze = R_p_pr * cos( lambda_p );

	/* transform to ENU coordinates */

       a = -h * sinDelta + xe;
       b =  h * cosDelta * sinLambda + ye;
       c = -h * cosDelta * cosLambda + ze;

       *x = -cosLambda * b  -sinLambda * c;
       *y = cosDelta * a  +  sinLambda * sinDelta * b
	 -cosLambda * sinDelta * c;
       *z = sinDelta * a  -sinLambda * cosDelta * b
	 +cosLambda * cosDelta * c;
       /*
       printf( "%f %f %f      %f %f %f\n", *plat, *plon, *palt, *x, *y, *z );
	*/
    }	
    return num_pts;
}
/*c----------------------------------------------------------------------*/

int loop_xy2ll_v3( double *plat, double *plon, double *palt
		    , double *x, double *y, double *z
		    , double olat, double olon, double oalt
		    , double R_earth, int num_pts )
{
    /* calculate (plat,plon) of a point at (x,y) relative to (olat,olon) */
    /* all dimensions in km. */

    /* transform to earth coordinates and then to lat/lon/alt */

    /* These calculations are from the book
     * "Aerospace Coordinate Systems and Transformations"
     * by G. Minkler/J. Minkler
     * these are the ECEF/ENU point transformations
     */

    int nn = num_pts;
    double delta_o, lambda_o, delta_p, lambda_p;
    double xe, ye, ze, sinLambda, cosLambda, sinDelta, cosDelta;
    double h;


    h = R_earth + oalt;
    delta_o = RADIANS( olat );	/* read delta sub oh */
    lambda_o = RADIANS( olon );

    sinLambda = sin( lambda_o );
    cosLambda = cos( lambda_o );
    sinDelta = sin( delta_o );
    cosDelta = cos( delta_o );
    /*
    printf( "\n" );
     */
    
    for(; nn--; plat++, plon++, palt++, x++, y++, z++ ) {

	/* transform to earth coordinates */

	xe = h * sinDelta + cosDelta * (*y) + sinDelta * (*z);

	ye = -h * cosDelta * sinLambda   -cosLambda * (*x)
	  + sinLambda * sinDelta * (*y) -sinLambda * cosDelta * (*z);

	ze = h * cosDelta * cosLambda   -sinLambda * (*x)
	  -cosLambda * sinDelta * (*y) + cosLambda * cosDelta * (*z);

	lambda_p = atan2( -ye, ze );
	delta_p = atan2( xe, sqrt( ye * ye + ze * ze ));

	*plat = DEGREES( delta_p );
	*plon = DEGREES( lambda_p );
	*palt = sqrt( xe * xe + ye * ye + ze * ze ) - R_earth;
	/*
	printf( "%f %f %f      %f %f %f\n", *plat, *plon, *palt, *x, *y, *z );
	 */
    }	
    return num_pts;
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */





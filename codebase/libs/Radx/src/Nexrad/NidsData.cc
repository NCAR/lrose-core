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
/******************************************************************************
 * Support Routines for NIDS Files 
 * F. Hage Oct 1998 NCAR/RAP
 */

#include <Radx/Radx.hh>
#include <Radx/RadxBuf.hh>
#include <Radx/RadxTime.hh>
#include <Radx/NidsData.hh>
#include <cstdio>
#include <zlib.h>

/******************************************************************************
 * _NIDS_BE_to_mess_header - convert to native from BE
 */

void _NIDS_BE_to_mess_header(_NIDS_header_t *mhead) 
{
  _NIDS_BE_to_array_16(&mhead->mcode, 4);
  _NIDS_BE_to_array_32(&mhead->mtime, 8);
  _NIDS_BE_to_array_16(&mhead->msource, 8);
  _NIDS_BE_to_array_32(&mhead->lat, 8);
  _NIDS_BE_to_array_16(&mhead->height, 20);
  _NIDS_BE_to_array_32(&mhead->pgtime, 4);
  _NIDS_BE_to_array_16(&mhead->pd1, 56);
  _NIDS_BE_to_array_32(&mhead->soffset, 12);
  _NIDS_BE_to_array_16(&mhead->bdivider, 4);
  _NIDS_BE_to_array_32(&mhead->blength, 4);
  _NIDS_BE_to_array_16(&mhead->nlayers, 4);
  _NIDS_BE_to_array_32(&mhead->lendat, 4);
}

/******************************************************************************
 * BE_from_mess_header - convert from native to BE
 */

void _NIDS_BE_from_mess_header(_NIDS_header_t *mhead) 
{
  _NIDS_BE_from_array_16(&mhead->mcode, 4);
  _NIDS_BE_from_array_32(&mhead->mtime, 8);
  _NIDS_BE_from_array_16(&mhead->msource, 8);
  _NIDS_BE_from_array_32(&mhead->lat, 8);
  _NIDS_BE_from_array_16(&mhead->height, 20);
  _NIDS_BE_from_array_32(&mhead->pgtime, 4);
  _NIDS_BE_from_array_16(&mhead->pd1, 56);
  _NIDS_BE_from_array_32(&mhead->soffset, 12);
  _NIDS_BE_from_array_16(&mhead->bdivider, 4);
  _NIDS_BE_from_array_32(&mhead->blength, 4);
  _NIDS_BE_from_array_16(&mhead->nlayers, 4);
  _NIDS_BE_from_array_32(&mhead->lendat, 4);
}

/******************************************************************************
 * BE_to_raster_header - convert to native from BE 
 */

void _NIDS_BE_to_raster_header(_NIDS_raster_header_t *rhead) 
{
  _NIDS_BE_to_array_16(rhead, sizeof(_NIDS_raster_header_t));
}

/******************************************************************************
 * BE_from_raster_header - convert from native to BE 
 */

void _NIDS_BE_from_raster_header(_NIDS_raster_header_t *rhead) 
{
  _NIDS_BE_from_array_16(rhead, sizeof(_NIDS_raster_header_t));
}

/******************************************************************************
 * BE_to_beam_header - convert to native from BE 
 */

void _NIDS_BE_to_beam_header(_NIDS_beam_header_t *bhead) 
{
  _NIDS_BE_to_array_16(bhead, sizeof(_NIDS_beam_header_t));
}

/******************************************************************************
 * BE_from_beam_header - convert from native to BE 
 */

void _NIDS_BE_from_beam_header(_NIDS_beam_header_t *bhead) 
{
  _NIDS_BE_from_array_16(bhead, sizeof(_NIDS_beam_header_t));
}


/******************************************************************************
 * BE_to_radial_header - convert to native from BE 
 */

void _NIDS_BE_to_radial_header(_NIDS_radial_header_t *rhead) 
{
  _NIDS_BE_to_array_16(rhead, sizeof(_NIDS_radial_header_t));
}

/******************************************************************************
 * BE_from_radial_header - convert from native to BE 
 */

void _NIDS_BE_from_radial_header(_NIDS_radial_header_t *rhead) 
{
  _NIDS_BE_from_array_16(rhead, sizeof(_NIDS_radial_header_t));
}


/******************************************************************************
 * BE_to_row_header - convert to native from BE 
 */

void _NIDS_BE_to_row_header(_NIDS_row_header_t *rowhead) 
{
  _NIDS_BE_to_array_16(rowhead, sizeof(_NIDS_row_header_t));
}

/******************************************************************************
 * BE_from_row_header - convert from native to BE 
 */

void _NIDS_BE_from_row_header(_NIDS_row_header_t *rowhead) 
{
  _NIDS_BE_from_array_16(rowhead, sizeof(_NIDS_row_header_t));
}


/*******************************************************************************
 * print message header
 */

void _NIDS_print_mess_hdr(FILE *out, const char *spacer, _NIDS_header_t *mhead)

{
  
  int i;
  time_t utime;

  fprintf(out, "\n-----------------------------------\n");
  fprintf(out, "%sNIDS MESSAGE HEADER\n", spacer);

  fprintf(out, "%s  message code: %d\n", spacer, mhead->mcode);
  utime = mhead->mdate * 86400 + mhead->mtime;

  fprintf(out, "%s  message time: %s\n", spacer, RadxTime::strm(utime).c_str());
  fprintf(out, "%s  message length: %d\n", spacer, mhead->mlength);
  fprintf(out, "%s  message source: %d\n", spacer, mhead->msource);
  fprintf(out, "%s  message dest: %d\n", spacer, mhead->mdest);
  fprintf(out, "%s  message blocks: %d\n", spacer, mhead->nblocks);
  
  fprintf(out, "%s\nProduct Description Block:\n", spacer);
  fprintf(out, "%s  latitude: %f\n", spacer, mhead->lat * 0.001);
  fprintf(out, "%s  longitude: %f\n", spacer, mhead->lon * 0.001);
  fprintf(out, "%s  height: %d\n", spacer, mhead->height);
  fprintf(out, "%s  code: %d\n", spacer, mhead->pcode);
  fprintf(out, "%s  mode: %d\n", spacer, mhead->mode);
  fprintf(out, "%s  VCP: %d\n", spacer, mhead->vcp);
  fprintf(out, "%s  seq num: %d\n", spacer, mhead->seqnum);
  fprintf(out, "%s  vscan num: %d\n", spacer, mhead->vscan);

  utime = mhead->vsdate * 86400 + mhead->vstime * 65536 + mhead->vstim2;
  fprintf(out, "%s  scan time: %s\n", spacer, RadxTime::strm(utime).c_str());

  utime = mhead->pgdate * 86400 + mhead->pgtime;
  fprintf(out, "%s  prod gen time: %s\n", spacer, RadxTime::strm(utime).c_str());

  fprintf(out, "%s  elnum: %d\n", spacer, mhead->elevnum);
  fprintf(out, "%s  pd1: %d\n", spacer, mhead->pd1);
  fprintf(out, "%s  pd2: %d\n", spacer, mhead->pd2);
  fprintf(out, "%s  pd3: %d\n", spacer, mhead->pd[0]);
  fprintf(out, "%s  pd4: %d\n", spacer, mhead->pd[17]);
  fprintf(out, "%s  pd5: %d\n", spacer, mhead->pd[18]);
  fprintf(out, "%s  pd6: %d\n", spacer, mhead->pd[19]);
  fprintf(out, "%s  pd7: %d\n", spacer, mhead->pd[20]);
  fprintf(out, "%s  pd8: %d\n", spacer, mhead->pd[21]);
  fprintf(out, "%s  pd9: %d\n", spacer, mhead->pd[22]);
  fprintf(out, "%s  pd10: %d\n", spacer, mhead->pd[23]);
  fprintf(out, "%s  data levels: ", spacer);

  for (i = 2; i < 17; i++ ) {
    Radx::ui08 msb, lsb;
    double level;
    msb = (mhead->pd[i] & 0xff00) >> 8;
    lsb = (mhead->pd[i] & 0x00ff);
    level = lsb;
    if (msb & 0x01) {
      level *= -1.0;
    }
    if (msb & 0x10) {
      level /= 10.0;
    }
    fprintf(out, "%d:%g ", i-1, level);
  }
  fprintf(out, "\n");
  fprintf(out, "%s  nmaps: %d\n", spacer, mhead->nmaps);
  fprintf(out, "%s  sym off: %d\n", spacer, mhead->soffset);
  fprintf(out, "%s  gra off: %d\n", spacer, mhead->goffset);
  fprintf(out, "%s  tab off: %d\n", spacer, mhead->toffset);
  
  fprintf(out, "%s\nProduct Symbology Block:\n", spacer);
  fprintf(out, "%s  block ID: %d\n", spacer, mhead->blockid );
  fprintf(out, "%s  block length: %d\n", spacer, mhead->blength);
  fprintf(out, "%s  nlayers: %d\n", spacer, mhead->nlayers);
  fprintf(out, "%s  layer length: %d\n", spacer, mhead->lendat); 
  fprintf(out, "\n");
  
}

/*******************************************************************************
 * print raster header
 */

void _NIDS_print_raster_hdr(FILE *out, const char *spacer,
                            _NIDS_raster_header_t *rhead)
  
{
  
  fprintf(out, "\n-----------------------------------\n");
  fprintf(out, "%sNIDS RASTER HEADER\n", spacer);
  
  fprintf(out, "%s  packet_code1: %x\n", spacer, rhead->packet_code1);
  fprintf(out, "%s  packet_code2: %x\n", spacer, rhead->packet_code2);
  fprintf(out, "%s  packet_code3: %x\n", spacer, rhead->packet_code3);
  fprintf(out, "%s  x_start: %d\n", spacer, rhead->x_start);
  fprintf(out, "%s  y_start: %d\n", spacer, rhead->y_start);
  fprintf(out, "%s  x_scale: %d\n", spacer, rhead->x_scale);
  fprintf(out, "%s  y_scale: %d\n", spacer, rhead->y_scale);
  fprintf(out, "%s  x_scale_fract: %d\n", spacer, rhead->x_scale_fract);
  fprintf(out, "%s  y_scale_fract: %d\n", spacer, rhead->y_scale_fract);
  fprintf(out, "%s  num_rows: %d\n", spacer, rhead->num_rows);
  fprintf(out, "%s  packing_descriptor: %d\n", spacer,
	  rhead->packing_descriptor);
  fprintf(out, "\n");

}
  
/*******************************************************************************
 * print beam header
 */

void _NIDS_print_beam_hdr(FILE *out, const char *spacer,
                          _NIDS_beam_header_t *bhead)
  
{

  fprintf(out, "\n-----------------------------------\n");
  fprintf(out, "%sNIDS BEAM HEADER\n", spacer);

  fprintf(out, "%s  num_halfwords: %d\n", spacer, bhead->num_halfwords);
  fprintf(out, "%s  radial_start_angle: %g\n", spacer,
	  bhead->radial_start_angle / 10.0);
  fprintf(out, "%s  radial_delta_angle: %g\n", spacer,
	  bhead->radial_delta_angle / 10.0);
  fprintf(out, "\n");

}
  
/*******************************************************************************
 * print row header
 */

void _NIDS_print_row_hdr(FILE *out, const char *spacer,
                         _NIDS_row_header_t *rowhead)
  
{

  fprintf(out, "\n-----------------------------------\n");
  fprintf(out, "%sNIDS ROW HEADER\n", spacer);
  fprintf(out, "%s  num_bytes: %d\n", spacer, rowhead->num_bytes);

}
  
/*******************************************************************************
 * print radial header
 */

void _NIDS_print_radial_hdr(FILE *out, const char *spacer,
                            _NIDS_radial_header_t *rhead)

{

  fprintf(out, "\n-----------------------------------\n");
  fprintf(out, "%sNIDS RADIAL HEADER\n", spacer);

  fprintf(out, "%s  packet_code: %x\n", spacer, rhead->packet_code);
  fprintf(out, "%s  first_r_bin: %d\n", spacer, rhead->first_r_bin);
  fprintf(out, "%s  num_r_bin: %d\n", spacer, rhead->num_r_bin);
  fprintf(out, "%s  i_center: %d\n", spacer, rhead->i_center);
  fprintf(out, "%s  j_center: %d\n", spacer, rhead->j_center);
  fprintf(out, "%s  scale_factor: %d\n", spacer, rhead->scale_factor);
  fprintf(out, "%s  num_radials: %d\n", spacer, rhead->num_radials);
  fprintf(out, "\n");

}
  
/*************************************
 * non-standard byte swapping routines
 */

#define _NIDS_REVERSE_SHORT(arg) (((arg)  << 8) | ((arg) >> 8))
#define _NIDS_REVERSE_INT(arg) (((arg) << 24) | (((arg) & 0xFF00) << 8) | (((arg) >> 8) & 0xFF00) | ((arg) >> 24))


/******************************************************************************
 * NIDS_host_is_big_endian(); Returns 1 if true, 0 otherwise
 */

int _NIDS_host_is_big_endian(void)
{
  union {
    unsigned short    sh_int;
    unsigned char     bytes[2];
  } int_union;
  
  int_union.sh_int = 1;
  if (int_union.bytes[1] != 0) return (1);
  else return (0);
}

/******************************************************************************
 * NIDS_Reverse_4byte_vals:
 *
 */

void _NIDS_Reverse_4byte_vals(Radx::ui32* array, int num)
{
  Radx::ui32  value;
  while (num--) {
    value = *array;
    *array =  _NIDS_REVERSE_INT(value);
    array++;
  }
}
/******************************************************************************
 * NIDS_Reverse_2byte_vals:
 *
 */

void _NIDS_Reverse_2byte_vals(Radx::ui16* array, int num)
{
  Radx::ui16 value;
  while (num--) {
    value = *array;
    *array =  _NIDS_REVERSE_SHORT(value);
    array++;
  }
} 

/******************************************************************************
 * SWAP_NIDS_ROW_HEADER: DO Big-endian to Little Endian connversion
 */

void swap_nids_row_header(_NIDS_row_header_t *head) 
{
  _NIDS_Reverse_2byte_vals((Radx::ui16 *) &head->num_bytes,1);
}


/******************************************************************************
 * SWAP_NIDS_BEAM_HEADER: DO Big-endian to Little Endian connversion
 */

void swap_nids_beam_header(_NIDS_beam_header_t *head) 
{
  _NIDS_Reverse_2byte_vals((Radx::ui16 *) &head->num_halfwords,3);
}

/******************************************************************************
 * SWAP_NIDS_RASTER_HEADER: DO Big-endian to Little Endian connversion
 */

void swap_nids_raster_header(_NIDS_raster_header_t *head) 
{
  _NIDS_Reverse_2byte_vals((Radx::ui16 *) &head->packet_code1,11);
}

/******************************************************************************
 * SWAP_NIDS_RDIAL_HEADER: DO Big-endian to Little Endian connversion
 */

void swap_nids_radial_header(_NIDS_radial_header_t *head) 
{
  _NIDS_Reverse_2byte_vals((Radx::ui16 *) &head->packet_code,7);
}

/******************************************************************************
 * SWAP_NIDS_HEADER: DO Big-endian to Little Endian connversion
 */

void swap_nids_header(_NIDS_header_t *head) 
{
  _NIDS_Reverse_2byte_vals((Radx::ui16 *) &head->mcode,2);
  _NIDS_Reverse_4byte_vals((Radx::ui32 *) &head->mtime,2);
  _NIDS_Reverse_2byte_vals((Radx::ui16 *) &head->msource,4);
  _NIDS_Reverse_4byte_vals((Radx::ui32 *) &head->lat,2);
  _NIDS_Reverse_2byte_vals((Radx::ui16 *) &head->height,10);
  _NIDS_Reverse_4byte_vals((Radx::ui32 *) &head->pgtime,1);
  _NIDS_Reverse_2byte_vals((Radx::ui16 *) &head->pd1,28);
  _NIDS_Reverse_4byte_vals((Radx::ui32 *) &head->soffset,3);
  _NIDS_Reverse_2byte_vals((Radx::ui16 *) &head->bdivider,2);
  _NIDS_Reverse_4byte_vals((Radx::ui32 *) &head->blength,1);
  _NIDS_Reverse_2byte_vals((Radx::ui16 *) &head->nlayers,2);
  _NIDS_Reverse_4byte_vals((Radx::ui32 *) &head->lendat,1);
}


static int _NIDS_BigEnd = 1;

/***********************************************
 * _NIDS_BE_reverse()
 *
 * Reverses the sense of this library. Therefore,
 * is called once, SmallEndian values are set.
 * If called twice, goes back to BigEndian.
 */

void _NIDS_BE_reverse(void)

{
  _NIDS_BigEnd = !_NIDS_BigEnd;
}

/************************************************
 * Return 1 if host is big_endian and 0 otherwise
 *
 * For debugging, if FORCE_SWAP is set, this routine will
 * always return FALSE, forcing a swap.
 */

int
  _NIDS_BE_is_big_endian()
{
  
#ifdef FORCE_SWAP

  return (0);

#else

  union 
  {
    Radx::ui16    d;
    Radx::ui08     bytes[2];
  }
  short_int;

  short_int.d = 1;
  if (short_int.bytes[1] != 0)
    return (_NIDS_BigEnd);
  else
    return (!_NIDS_BigEnd);

#endif
}

/****************************************************
 * _NIDS_BE_swap_array_64()
 *
 * Performs an in-place 64-bit value byte swap.
 *
 * Array must be aligned.
 *
 * Returns the number of bytes converted.
 */

Radx::si32
  _NIDS_BE_swap_array_64(void *array, Radx::ui32 nbytes)

{
  char *ptr = (char*) array;
  Radx::ui32 i, l1,l2, ndoubles;

  /* check for little or big endian */
  if(_NIDS_BE_is_big_endian()) {
    return (0);
  }

  ndoubles = nbytes / 8;

  for (i = 0; i < ndoubles; i++) {

    /* Copy the 8 bytes to 2 ui32's - Reversing 1st & 2nd */
    /* PTR                 L1      L2      */
    /* 1 2 3 4 5 6 7 8 ->  5 6 7 8 1 2 3 4 */
    memcpy((void*)&l2,(void*)ptr,4);
    memcpy((void*)&l1,(void*)(ptr+4),4);


    /* Reverse the 4 bytes of each ui32 */
    /* 5 6 7 8  -> 8 7 6 5  */
    l1 = (((l1 & 0xff000000) >> 24) |
	  ((l1 & 0x00ff0000) >> 8) |
	  ((l1 & 0x0000ff00) << 8) |
	  ((l1 & 0x000000ff) << 24));

    /* 1 2 3 4 -> 4 3 2 1 */
    l2 = (((l2 & 0xff000000) >> 24) |
	  ((l2 & 0x00ff0000) >> 8) |
	  ((l2 & 0x0000ff00) << 8) |
	  ((l2 & 0x000000ff) << 24));


    /* Copy the reversed value back into place */
    memcpy((void*)ptr,(void*)&l1,4);
    memcpy((void*)(ptr+4),(void*)&l2,4);

    ptr+=8;  /* Move to the next 8 byte value */
  }

  return (nbytes);
}

/********************
 * _NIDS_BE_from_fl64()           
 * Converts a single fl64
 */                   

void _NIDS_BE_from_fl64(Radx::fl64 *dst, Radx::fl64 *src)                       
 
{
  memcpy( dst, src, sizeof(Radx::fl64));
  _NIDS_BE_swap_array_64(dst, sizeof(Radx::fl64));   
}
 
/******************************
 *  _NIDS_BE_to_fl64       
 *  Converts a single fl64
 */

void _NIDS_BE_to_fl64(Radx::fl64 *src, Radx::fl64 *dst)

{
  memcpy( dst, src, sizeof(Radx::fl64) );
  _NIDS_BE_swap_array_64(dst, sizeof(Radx::fl64));
}

/********************************
 *  _NIDS_BE_from_si64 
 *  Converts a single si64
 */

Radx::si64 _NIDS_BE_from_si64(Radx::si64 x)

{
  _NIDS_BE_swap_array_64((void *) &x, sizeof(Radx::si64));
  return (x);
}    

/******************************
 *  _NIDS_BE_to_si64 
 *  Converts a single si64
 */

Radx::si64 _NIDS_BE_to_si64(Radx::si64 x)

{
  _NIDS_BE_swap_array_64(&x, sizeof(Radx::si64));
  return (x);
}    

/*****************************************************
 * _NIDS_BE_swap_array_32()
 *
 * Performs an in-place 32-bit word byte swap, if necessary, to produce
 * BE representation from machine representation, or vice-versa.
 *
 * Array must be aligned.
 *
 * Returns the number of bytes converted.
 */

Radx::si32
  _NIDS_BE_swap_array_32(void *array, Radx::ui32 nbytes)
     
{
  Radx::ui32 i, l, nlongs;
  Radx::ui32 *this_long;
  Radx::ui32 *array32 = (Radx::ui32 *) array;

  /* check for little or big endian */
  if(_NIDS_BE_is_big_endian()) {
    return (0);
  }
  
  nlongs = nbytes / sizeof(Radx::ui32);
  this_long = array32;
  
  for (i = 0; i < nlongs; i++) {

    l = *this_long;
    
    *this_long = (((l & 0xff000000) >> 24) |
		  ((l & 0x00ff0000) >> 8) |
		  ((l & 0x0000ff00) << 8) |
		  ((l & 0x000000ff) << 24));
    
    this_long++;

  }

  return (nbytes);

}

/********************
 * _NIDS_BE_from_fl32()           
 * Converts a single fl32
 */                   

void _NIDS_BE_from_fl32(Radx::fl32 *dst, Radx::fl32 *src)                       
 
{
  memcpy( dst, src, sizeof(Radx::fl32) );
  _NIDS_BE_swap_array_32(dst, sizeof(Radx::fl32));   
}
 
/******************************
 *  _NIDS_BE_to_fl32       
 *  Converts a single fl32
 */

void _NIDS_BE_to_fl32(Radx::fl32 *src, Radx::fl32 *dst)

{
  memcpy( dst, src, sizeof(Radx::fl32) );
  _NIDS_BE_swap_array_32(dst, sizeof(Radx::fl32));
}

/********************************
 *  _NIDS_BE_from_si32 replaces htonl()
 *  Converts a single si32
 */

Radx::si32 _NIDS_BE_from_si32(Radx::si32 x)

{
  _NIDS_BE_swap_array_32((void *) &x, sizeof(Radx::si32));
  return (x);
}    

/******************************
 *  _NIDS_BE_to_si32 replaces ntohl()
 *  Converts a single si32
 */

Radx::si32 _NIDS_BE_to_si32(Radx::si32 x)

{
  _NIDS_BE_swap_array_32(&x, sizeof(Radx::si32));
  return (x);
}    

/*****************************************************
 * _NIDS_BE_swap_array_16()
 *
 * Performs an in-place 16-bit word byte swap, if necessary, to produce
 * BE representation from machine representation, or vice-versa.
 *
 * Array must be aligned.
 *
 * Returns the number of bytes converted.
 *
 */

Radx::si32
  _NIDS_BE_swap_array_16(void *array, Radx::ui32 nbytes)
     
{

  Radx::ui32 i, l, nlongs, nshorts;
  Radx::ui32 *this_long;
  Radx::ui16 *array16 = (Radx::ui16 *) array;
  Radx::ui16 s;

  /* check for little or big endian */
  if(_NIDS_BE_is_big_endian()) {
    return (0);
  }
  
  nlongs = nbytes / sizeof(Radx::ui32);
  this_long = (Radx::ui32 *)array16;

  for (i = 0; i < nlongs; i++) {
    
    l = *this_long;
    
    *this_long = (((l & 0xff000000) >> 8) |
                  ((l & 0x00ff0000) << 8) |
                  ((l & 0x0000ff00) >> 8) |
                  ((l & 0x000000ff) << 8));

    this_long++;
  }
  
  if (nlongs * sizeof(Radx::ui32) != nbytes) {
    nshorts = nbytes / sizeof(Radx::ui16);
    s = array16[nshorts-1];
    array16[nshorts-1]= (((s & 0xff00) >> 8) | ((s & 0x00ff) << 8));
  }

  return (nbytes);
  
}

/********************************
 *  _NIDS_BE_from_si16 replaces htons()
 *  Converts a single si16
 */

Radx::si16 _NIDS_BE_from_si16(Radx::si16 x)

{
  _NIDS_BE_swap_array_16(&x, sizeof(Radx::si16));
  return (x);
}    

/******************************
 *  _NIDS_BE_to_si16 replaces ntohs()
 *  Converts a single si16
 */

Radx::si16 _NIDS_BE_to_si16(Radx::si16 x)

{
  _NIDS_BE_swap_array_16(&x, sizeof(Radx::si16));
  return (x);
}    

/********************************
 *  _NIDS_BE_from_ui32
 *  Converts a single ui32
 */

Radx::ui32 _NIDS_BE_from_ui32(Radx::ui32 x)

{
  _NIDS_BE_swap_array_32(&x, sizeof(Radx::ui32));
  return (x);
}    

/******************************
 *  _NIDS_BE_to_ui32
 *  Converts a single ui32
 */

Radx::ui32 _NIDS_BE_to_ui32(Radx::ui32 x)

{
  _NIDS_BE_swap_array_32(&x, sizeof(Radx::ui32));
  return (x);
}    

/********************************
 *  _NIDS_BE_from_Radx::ui16
 *  Converts a single Radx::ui16
 */

Radx::ui16 _NIDS_BE_from_ui16(Radx::ui16 x)

{
  _NIDS_BE_swap_array_16(&x, sizeof(Radx::ui16));
  return (x);
}    

/******************************
 *  _NIDS_BE_to_ui16
 *  Converts a single ui16
 */

Radx::ui16 _NIDS_BE_to_ui16(Radx::ui16 x)

{
  _NIDS_BE_swap_array_16(&x, sizeof(Radx::ui16));
  return (x);
}    

//////////////////////////////////////////////////////////////////////////
// get a line from the buffer
// reads until next line feed
// returns NULL on failure, or line

char *_NIDS_read_line(RadxBuf &inBuf, size_t &offset, char *line, size_t maxLen)
  
{
  
  for (size_t ii = 0; ii < maxLen; ii++, offset++) {

    // out of data?
    
    if (offset >= inBuf.getLen()) {
      line[ii] = '\0';
      if (ii == 0) {
        // no data
        return NULL;
      } else {
        return line;
      }
    }

    char cc = ((char *) inBuf.getPtr())[offset];
    line[ii] = cc;
    
    if (cc == '\n') {
      if (ii < maxLen - 1) {
        line[ii+1] = '\0';
      }
      offset++;
      return line;
    }

  } // ii

  line[maxLen-1] = '\0';
  return line;

}

//////////////////////////////////////////////////////////////////////////
// read data from the buffer
// returns number of characters read

size_t _NIDS_read_buf(RadxBuf &inBuf, size_t &offset, 
                      unsigned char *readBuf, size_t maxLen)
  
{
  
  int nread = 0;
  for (size_t ii = 0; ii < maxLen; ii++, offset++, nread++) {
    
    // out of data?
    
    if (offset >= inBuf.getLen()) {
      return nread;
    }

    unsigned char cc = ((unsigned char *) inBuf.getPtr())[offset];
    readBuf[nread] = cc;
    
  }

  return nread;

}

/********************************************************************
 * NIDS decompression utility for data compressed with zlib
 * output stripping header and leaving raw NIDS data
 ********************************************************************/

int _NIDS_uncompress(RadxBuf &inBuf,
                     RadxBuf &outBuf,
                     string &radarName)
  
{

  radarName.clear();

  size_t inOffset = 0;
  unsigned char inArray[1000];

  unsigned char soh[101];
  unsigned char wmo[101];
  unsigned char awip[101];

  if (_NIDS_read_line(inBuf, inOffset, (char *) soh, 100) == NULL) {
    cerr << "ERROR - _NIDS_uncompress, bad data file" << endl;
    return -1;
  }

  /*
   * Account for both raw input (0x01) and WXP input (**)
   */
  
  // int wxp = 0;
  int off = 0;
  if(soh[0] == 0x78 && soh[1] == 0x9C) {

    off = strlen((char *) soh);
    memcpy(inArray, soh, off);

  } else if (soh[0] == 0x78 && soh[1] == 0xDA) {
    
    /* Fix for bogus compression indicator PP 6/13/2002 */

    off = strlen((char *) soh);
    memcpy(inArray, soh, off);

  } else if(soh[0] == '\n') {

    if (_NIDS_read_line(inBuf, inOffset, (char *) soh, 100) == NULL) {
      cerr << "ERROR - _NIDS_uncompress, bad data file" << endl;
      return -1;
    }

    int len = strlen((char *) soh)-7;
    memcpy(wmo, soh+3, len);
    wmo[len] = 0;
    if (_NIDS_read_line(inBuf, inOffset, (char *) awip, 100) == NULL) {
      cerr << "ERROR - _NIDS_uncompress, bad data file" << endl;
      return -1;
    }
  
    // wxp = 1;

  } else if(soh[0] == '*') {

    int len = strlen((char *) soh) -7;
    memcpy(wmo, soh+3, len);
    wmo[len] = 0;
    if (_NIDS_read_line(inBuf, inOffset, (char *) awip, 100) == NULL) {
      cerr << "ERROR - _NIDS_uncompress, bad data file" << endl;
      return -1;
    }
    
    // wxp = 1;

  } else if(soh[0] == 0x01) {

    unsigned char seq[101];
    if (_NIDS_read_line(inBuf, inOffset, (char *) seq, 100) == NULL) {
      cerr << "ERROR - _NIDS_uncompress, bad data file" << endl;
      return -1;
    }
    if (_NIDS_read_line(inBuf, inOffset, (char *) wmo, 100) == NULL) {
      cerr << "ERROR - _NIDS_uncompress, bad data file" << endl;
      return -1;
    }
    if (_NIDS_read_line(inBuf, inOffset, (char *) awip, 100) == NULL) {
      cerr << "ERROR - _NIDS_uncompress, bad data file" << endl;
      return -1;
    }

    string awipStr((char *) awip);
    if (awipStr.size() >= 6) {
      radarName = awipStr.substr(3, 3);
    }
    
  }
  
  int iret = 0;

  z_stream zs;
  zs.total_out = 4000;
  unsigned char outArray[10000];
  unsigned char tmpArray[1000];
  
  for(int i = 0; iret != Z_STREAM_END || zs.total_out == 4000; i++){
    /* 
     * Read in a block of data
     */
    size_t insize = _NIDS_read_buf(inBuf, inOffset, inArray + off, 1000-off);
    int len = insize + off;

    /* 
     * Check for 789C byte sequence denoting zlib compression
     * If data are not compressed, pass through raw data
     */

    if(i == 0 && (inArray[0] != 0x78 || ((inArray[1] != 0x9C) && 
                                         (inArray[1] != 0xDA)) )) {
      outBuf.add(inArray, insize);
      while ((insize = _NIDS_read_buf(inBuf, inOffset, inArray, 1000)) > 0) {
        outBuf.add(inArray, insize);
      }
      inflateEnd(&zs);
      return 0;
    }

    zs.avail_in = len;
    zs.avail_out = 10000;
    zs.next_in = inArray;
    zs.next_out = outArray;

    /*
     *  Check to see if 4000 byte block has been read and reinitialize
     */
    if(i == 0 || iret == Z_STREAM_END){
      zs.zalloc = NULL;
      zs.zfree = NULL;
      inflateInit(&zs);
    }

    /*
     *  Inflate NIDS data
     */
    iret = inflate(&zs, Z_STREAM_END);
    if(iret < 0) break;
    off = zs.avail_in;
    /*
     *  Raw NIDS output
     */
    if (i == 0) {
      outBuf.add(outArray+54, 10000-zs.avail_out-54);
    } else {
      outBuf.add(outArray, 10000-zs.avail_out);
    }

    /*
     * Move remaining data that still is compressed and prepared 
     * for next inflate
     */

    memcpy(tmpArray, inArray+len-off, off);
    memcpy(inArray, tmpArray, off);

  }

  inflateEnd(&zs);

  return 0;

}

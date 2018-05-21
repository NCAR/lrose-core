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
/******************************************************************************
 * NIDS_FILE.C Support Routines for NIDS Files 
 * F. Hage Oct 1998 NCAR/RAP
 */

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <rapformats/nids_file.h>

/******************************************************************************
 * BE_to_mess_header - convert to native from BE
 */

void NIDS_BE_to_mess_header(NIDS_header_t *mhead) 
{
     BE_to_array_16(&mhead->mcode, 4);
     BE_to_array_32(&mhead->mtime, 8);
     BE_to_array_16(&mhead->msource, 8);
     BE_to_array_32(&mhead->lat, 8);
     BE_to_array_16(&mhead->height, 20);
     BE_to_array_32(&mhead->pgtime, 4);
     BE_to_array_16(&mhead->pd1, 56);
     BE_to_array_32(&mhead->soffset, 12);
     BE_to_array_16(&mhead->bdivider, 4);
     BE_to_array_32(&mhead->blength, 4);
     BE_to_array_16(&mhead->nlayers, 4);
     BE_to_array_32(&mhead->lendat, 4);
}

/******************************************************************************
 * BE_from_mess_header - convert from native to BE
 */

void NIDS_BE_from_mess_header(NIDS_header_t *mhead) 
{
     BE_from_array_16(&mhead->mcode, 4);
     BE_from_array_32(&mhead->mtime, 8);
     BE_from_array_16(&mhead->msource, 8);
     BE_from_array_32(&mhead->lat, 8);
     BE_from_array_16(&mhead->height, 20);
     BE_from_array_32(&mhead->pgtime, 4);
     BE_from_array_16(&mhead->pd1, 56);
     BE_from_array_32(&mhead->soffset, 12);
     BE_from_array_16(&mhead->bdivider, 4);
     BE_from_array_32(&mhead->blength, 4);
     BE_from_array_16(&mhead->nlayers, 4);
     BE_from_array_32(&mhead->lendat, 4);
}

/******************************************************************************
 * BE_to_raster_header - convert to native from BE 
 */

void NIDS_BE_to_raster_header(NIDS_raster_header_t *rhead) 
{
     BE_to_array_16(rhead, sizeof(NIDS_raster_header_t));
}

/******************************************************************************
 * BE_from_raster_header - convert from native to BE 
 */

void NIDS_BE_from_raster_header(NIDS_raster_header_t *rhead) 
{
     BE_from_array_16(rhead, sizeof(NIDS_raster_header_t));
}

/******************************************************************************
 * BE_to_beam_header - convert to native from BE 
 */

void NIDS_BE_to_beam_header(NIDS_beam_header_t *bhead) 
{
     BE_to_array_16(bhead, sizeof(NIDS_beam_header_t));
}

/******************************************************************************
 * BE_from_beam_header - convert from native to BE 
 */

void NIDS_BE_from_beam_header(NIDS_beam_header_t *bhead) 
{
     BE_from_array_16(bhead, sizeof(NIDS_beam_header_t));
}


/******************************************************************************
 * BE_to_radial_header - convert to native from BE 
 */

void NIDS_BE_to_radial_header(NIDS_radial_header_t *rhead) 
{
     BE_to_array_16(rhead, sizeof(NIDS_radial_header_t));
}

/******************************************************************************
 * BE_from_radial_header - convert from native to BE 
 */

void NIDS_BE_from_radial_header(NIDS_radial_header_t *rhead) 
{
     BE_from_array_16(rhead, sizeof(NIDS_radial_header_t));
}


/******************************************************************************
 * BE_to_row_header - convert to native from BE 
 */

void NIDS_BE_to_row_header(NIDS_row_header_t *rowhead) 
{
     BE_to_array_16(rowhead, sizeof(NIDS_row_header_t));
}

/******************************************************************************
 * BE_from_row_header - convert from native to BE 
 */

void NIDS_BE_from_row_header(NIDS_row_header_t *rowhead) 
{
     BE_from_array_16(rowhead, sizeof(NIDS_row_header_t));
}


/*******************************************************************************
 * print message header
 */

void NIDS_print_mess_hdr(FILE *out, const char *spacer, NIDS_header_t *mhead)

{

  int i;
  time_t utime;

  fprintf(out, "\n-----------------------------------\n");
  fprintf(out, "%sNIDS MESSAGE HEADER\n", spacer);

  fprintf(out, "%s  message code: %d\n", spacer, mhead->mcode);
  utime = mhead->mdate * 86400 + mhead->mtime;

  fprintf(out, "%s  message time: %s\n", spacer, utimstr(utime));
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
  fprintf(out, "%s  scan time: %s\n", spacer, utimstr(utime));

  utime = mhead->pgdate * 86400 + mhead->pgtime;
  fprintf(out, "%s  prod gen time: %s\n", spacer, utimstr(utime));

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
    ui08 msb, lsb;
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

void NIDS_print_raster_hdr(FILE *out, const char *spacer,
			   NIDS_raster_header_t *rhead)

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

void NIDS_print_beam_hdr(FILE *out, const char *spacer,
			 NIDS_beam_header_t *bhead)

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

void NIDS_print_row_hdr(FILE *out, const char *spacer,
			NIDS_row_header_t *rowhead)

{

  fprintf(out, "\n-----------------------------------\n");
  fprintf(out, "%sNIDS ROW HEADER\n", spacer);
  fprintf(out, "%s  num_bytes: %d\n", spacer, rowhead->num_bytes);

}
  
/*******************************************************************************
 * print radial header
 */

void NIDS_print_radial_hdr(FILE *out, const char *spacer,
			 NIDS_radial_header_t *rhead)

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

#define NIDS_REVERSE_SHORT(arg) (((arg)  << 8) | ((arg) >> 8))
#define NIDS_REVERSE_INT(arg) (((arg) << 24) | (((arg) & 0xFF00) << 8) | (((arg) >> 8) & 0xFF00) | ((arg) >> 24))


/******************************************************************************
 * NIDS_host_is_big_endian(); Returns 1 if true, 0 otherwise
 */

int NIDS_host_is_big_endian(void)
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

void NIDS_Reverse_4byte_vals(unsigned int* array, int num)
{
    unsigned int  value;
    while (num--) {
      value = *array;
      *array =  NIDS_REVERSE_INT(value);
      array++;
    }
}
/******************************************************************************
 * NIDS_Reverse_2byte_vals:
 *
 */

void NIDS_Reverse_2byte_vals(unsigned short* array, int num)
{
    unsigned short  value;
    while (num--) {
      value = *array;
      *array =  NIDS_REVERSE_SHORT(value);
      array++;
    }
} 

/******************************************************************************
 * SWAP_NIDS_ROW_HEADER: DO Big-endian to Little Endian connversion
 */

void swap_nids_row_header(NIDS_row_header_t *head) 
{
  NIDS_Reverse_2byte_vals((unsigned short *) &head->num_bytes,1);
}


/******************************************************************************
 * SWAP_NIDS_BEAM_HEADER: DO Big-endian to Little Endian connversion
 */

void swap_nids_beam_header(NIDS_beam_header_t *head) 
{
  NIDS_Reverse_2byte_vals((unsigned short *) &head->num_halfwords,3);
}

/******************************************************************************
 * SWAP_NIDS_RASTER_HEADER: DO Big-endian to Little Endian connversion
 */

void swap_nids_raster_header(NIDS_raster_header_t *head) 
{
  NIDS_Reverse_2byte_vals((unsigned short *) &head->packet_code1,11);
}

/******************************************************************************
 * SWAP_NIDS_RDIAL_HEADER: DO Big-endian to Little Endian connversion
 */

void swap_nids_radial_header(NIDS_radial_header_t *head) 
{
     NIDS_Reverse_2byte_vals((unsigned short *) &head->packet_code,7);
}

/******************************************************************************
 * SWAP_NIDS_HEADER: DO Big-endian to Little Endian connversion
 */

void swap_nids_header(NIDS_header_t *head) 
{
     NIDS_Reverse_2byte_vals((unsigned short *) &head->mcode,2);
     NIDS_Reverse_4byte_vals((unsigned int *) &head->mtime,2);
     NIDS_Reverse_2byte_vals((unsigned short *) &head->msource,4);
     NIDS_Reverse_4byte_vals((unsigned int *) &head->lat,2);
     NIDS_Reverse_2byte_vals((unsigned short *) &head->height,10);
     NIDS_Reverse_4byte_vals((unsigned int *) &head->pgtime,1);
     NIDS_Reverse_2byte_vals((unsigned short *) &head->pd1,28);
     NIDS_Reverse_4byte_vals((unsigned int *) &head->soffset,3);
     NIDS_Reverse_2byte_vals((unsigned short *) &head->bdivider,2);
     NIDS_Reverse_4byte_vals((unsigned int *) &head->blength,1);
     NIDS_Reverse_2byte_vals((unsigned short *) &head->nlayers,2);
     NIDS_Reverse_4byte_vals((unsigned int *) &head->lendat,1);
}



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
/*********************************************************************
 * retrieval.c
 *
 * controls retrieval of data from grid plane
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1994
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "mdv2ascii.h"

static ui08 **Planes = NULL;
static si32 Nx, Ny, Nz;
static si32 XX, YY, ZZ;
static double Scale, Bias;
static ui08 missing_data_value;

int retrieve_next(double *val)
     
{
  
  int index;
  ui08 byte_val;

  if (XX == Nx) {
    XX = 0;
    YY++;
  }

  if (YY == Ny) {
    YY = 0;
    ZZ++;
  }

  if (ZZ == Nz) {
    return(0);
  }

  if (Glob->start_row == BOT) {
    index = YY * Nx + XX;
  } else {
    index = (Ny - YY - 1) * Nx + XX;
  }

  byte_val = *(Planes[ZZ] + index);

  if (Glob->badSet){
    if (byte_val == missing_data_value){
      *val = Glob->bad;
      XX++;
      return(1);
    }
  }

  *val = (double) byte_val * Scale + Bias;

  if (*val < Glob->min_output_val) {
    *val = Glob->min_output_val;
  }

  if (*val > Glob->max_output_val) {
    *val = Glob->max_output_val;
  }

  XX++;

  return (1);
  
}


void init_retrieval(vol_file_handle_t *v_handle,
		    si32 plane_num,
		    si32 field_num)
     
{
  
  field_params_t *fparams;

  Nx = v_handle->vol_params->cart.nx;
  Ny = v_handle->vol_params->cart.ny;
  Nz = 1;
  
  YY = 0;
  XX = 0;
  ZZ = 0;
  
  Planes = (ui08 **) umalloc (Nz * sizeof(ui08*));
  Planes[0] = v_handle->field_plane[Glob->field_num][Glob->plane_num];

  fparams = v_handle->field_params[Glob->field_num];
  Scale = (double) fparams->scale / (double) fparams->factor;
  Bias = (double) fparams->bias / (double) fparams->factor;
  missing_data_value = (ui08) fparams->missing_val;

  
  return;

}

void init_retrieval_comp(vol_file_handle_t *v_handle,
			 si32 field_num)
     
{
  
  int i, j;
  ui08 *tmp_plane;
  ui08 *c, *p;
  field_params_t *fparams;

  Nx = v_handle->vol_params->cart.nx;
  Ny = v_handle->vol_params->cart.ny;
  Nz = 1;
  
  YY = 0;
  XX = 0;
  ZZ = 0;

  /* malloc plane */

  if (Planes != NULL) {
    ufree(Planes);
  }
  Planes = (ui08 **) umalloc2 (Nz, Nx * Ny, sizeof(ui08));

  /* copy in first plane */
  
  memcpy(Planes[0], v_handle->field_plane[Glob->field_num][0],
	 Nx * Ny * sizeof(ui08));

  /* compute composite - max at any ht */

  for (i = 1; i < v_handle->vol_params->cart.nz; i++) {
    tmp_plane = v_handle->field_plane[Glob->field_num][i];
    p = tmp_plane;
    c = Planes[0];
    for (j = 0; j < Nx * Ny; j++, c++, p++) {
      if (*p > *c) {
	*c = *p;
      }
    }
  }

  fparams = v_handle->field_params[Glob->field_num];
  Scale = (double) fparams->scale / (double) fparams->factor;
  Bias = (double) fparams->bias / (double) fparams->factor;
  missing_data_value = (ui08) fparams->missing_val;

  return;

}

void init_retrieval_vol(vol_file_handle_t *v_handle,
		        si32 field_num)
     
{
  int i;
  field_params_t *fparams;

  Nx = v_handle->vol_params->cart.nx;
  Ny = v_handle->vol_params->cart.ny;
  Nz = v_handle->vol_params->cart.nz;
  
  YY = 0;
  XX = 0;
  ZZ = 0;
  
  Planes = (ui08 **) umalloc (Nz * sizeof(ui08*));
  
  for (i = 0; i < Nz; i++) {
     Planes[i] = v_handle->field_plane[Glob->field_num][i];
  }

  fparams = v_handle->field_params[Glob->field_num];
  Scale = (double) fparams->scale / (double) fparams->factor;
  Bias = (double) fparams->bias / (double) fparams->factor;
  missing_data_value = (ui08) fparams->missing_val;
 
  return;

}


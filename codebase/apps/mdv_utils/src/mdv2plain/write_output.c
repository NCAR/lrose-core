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
 * write_output()
 *
 * writes the output file
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1995
 *
 * Mike Dixon
 *
 *********************************************************************/

#include <sys/stat.h>

#include <titan/radar.h>
#include <toolsa/ldata_info.h>

#include "mdv2plain.h"

int write_output(vol_file_handle_t *v_handle, LDATA_handle_t *output_ldata)

{

  char dir_path[MAX_PATH_LEN];
  char file_path[MAX_PATH_LEN];
  char call_str[BUFSIZ];

  ui08 vip_lookup[255];
  ui08 *dp;

  int iplane, iy, i, j;

  si32 delta_dp;
  si32 nbytes_array;

  double vip_thresholds[7] = {-30.0, 18.0, 30.0, 41.0, 46.0, 51.0, 56.0};
  fl64 scale, bias, dbz;

  cart_params_t *cart;
  field_params_t *fparams;
  struct stat dir_stat;
  FILE *out;

  cart = &v_handle->vol_params->cart;
  fparams = v_handle->field_params[Glob->params.field_num];

  if (Glob->params.debug >= DEBUG_EXTRA)
    fprintf(stderr, "*** Creating output file\n");
  
  /*
   * compute output dir path
   */
  
  sprintf(dir_path, "%s%s%.4d%.2d%.2d",
	  Glob->params.output_dir, PATH_DELIM,
	  (int) v_handle->vol_params->mid_time.year,
	  (int) v_handle->vol_params->mid_time.month,
	  (int) v_handle->vol_params->mid_time.day);
  
  /*
   * create directory, if needed
   */
    
  if (0 != stat(dir_path, &dir_stat)) {
    if (Glob->params.debug >= DEBUG_EXTRA)
      fprintf(stderr, "    Creating directory <%s>\n",
	      dir_path);
    
    if (mkdir(dir_path, 0755)) {
      fprintf(stderr, "ERROR - %s:write_output\n", Glob->prog_name);
      fprintf(stderr, "Trying to make output dir\n");
      perror(dir_path);
      return(R_FAILURE);
    }
  }
  
  /*
   * create output file path
   */
  
  sprintf(file_path, "%s%s%.2d%.2d%.2d.%s",
	  dir_path, PATH_DELIM,
	  (int) v_handle->vol_params->mid_time.hour,
	  (int) v_handle->vol_params->mid_time.min,
	  (int) v_handle->vol_params->mid_time.sec,
	  Glob->params.output_file_ext);
  
  /*
   * open file
   */
  
  if (Glob->params.debug >= DEBUG_EXTRA)
    fprintf(stderr, "    Openning file <%s>\n",
	    file_path);
  
  if ((out = fopen(file_path, "w")) == NULL) {
    fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
    fprintf(stderr, "Cannot open output file for writing\n");
    perror(file_path);
    return (R_FAILURE);
  }

  /*
   * if required, prepare lookup to translate into vip levels
   */

  scale = (fl64) fparams->scale / (fl64) fparams->factor;
  bias = (fl64) fparams->bias / (fl64) fparams->factor;

  if (Glob->params.vip_output) {

    if (Glob->params.debug >= DEBUG_EXTRA)
      fprintf(stderr, "    Translating output into VIP levels\n");
    
    for (i = 0; i < 255; i++) {
      dbz = (double) i * scale + bias;
      vip_lookup[i] = 0;
      for (j = 6; j > 0; j--) {
	if (dbz >= vip_thresholds[j]) {
	  vip_lookup[i] = j;
	  break;
	}
      } /* j */
    } /* i */

  } /* if (Glob->params.vip_output) */

  /*
   * write grid
   */

  nbytes_array =
    ((Glob->params.end_plane - Glob->params.start_plane + 1) *
     cart->ny * cart->nx);

  if (Glob->params.fortran_output) {

    if (Glob->params.debug >= DEBUG_EXTRA)
      fprintf(stderr, "    Writing leading FORTRAN record length to file\n");
    
    if (ufwrite((char *) &nbytes_array,
		(int) sizeof(si32), 1, out) != 1) {
      fprintf(stderr, "ERROR - %s:write_output\n", Glob->prog_name);
      fprintf(stderr, "Cannot complete leading FORTRAN rec length write to output file\n");
      perror(file_path);
      fclose(out);
      return (R_FAILURE);
    }
    
  }
  
  /*
   * Write the scale and bias.
   */

  if (Glob->params.output_scale_flag)
  {
    if (Glob->params.debug >= DEBUG_EXTRA)
      fprintf(stderr, "    Writing scale and bias to file\n");
    
    if (ufwrite((char *)&scale, (int)sizeof(fl64), 1, out) != 1)
    {
      fprintf(stderr, "ERROR - %s:write_output\n", Glob->prog_name);
      fprintf(stderr, "Cannot complete scale write to output file\n");
      perror(file_path);
      fclose(out);
      return (R_FAILURE);
    }
    
    if (ufwrite((char *)&bias, (int)sizeof(fl64), 1, out) != 1)
    {
      fprintf(stderr, "ERROR - %s:write_output\n", Glob->prog_name);
      fprintf(stderr, "Cannot complete bias write to output file\n");
      perror(file_path);
      fclose(out);
      return (R_FAILURE);
    }
    
  }
  
  for (iplane = Glob->params.start_plane;
       iplane <= Glob->params.end_plane; iplane++) {

    if (Glob->params.debug >= DEBUG_EXTRA)
      fprintf(stderr, "    Writing plane %d to output file\n", iplane);
    
    /*
     * if required, prepare to translate into vip levels
     */
    
    if (Glob->params.vip_output) {
      
      dp = v_handle->field_plane[Glob->params.field_num][iplane];

      for (i = 0; i < cart->nx * cart->ny; i++, dp++) {
	*dp = vip_lookup[*dp];
      } /* i */
      
    } /* if (Glob->params.vip_output) */

    if (Glob->params.output_data_origin == BOTLEFT) {

      dp = v_handle->field_plane[Glob->params.field_num][iplane];
      delta_dp = cart->nx;

    } else {

      dp = v_handle->field_plane[Glob->params.field_num][iplane] +
	((cart->ny - 1) * cart->nx);
      delta_dp = -cart->nx;

    }

    for (iy = 0; iy < cart->ny; iy++) {

      if (ufwrite((char *) dp,
		  (int) sizeof(ui08),
		  (int) cart->nx,
		  out) != cart->nx) {
	fprintf(stderr, "ERROR - %s:write_output\n", Glob->prog_name);
	fprintf(stderr, "Cannot complete row %d write to output file\n", iy);
	perror(file_path);
	fclose(out);
	return (R_FAILURE);
      }
      
      dp += delta_dp;

    } /* iy */
    
  } /* iplane */

  if (Glob->params.fortran_output) {

    if (Glob->params.debug >= DEBUG_EXTRA)
      fprintf(stderr, "    Writing trailing FORTRAN record length\n");
    
    if (ufwrite((char *) &nbytes_array,
		(int) sizeof(si32), 1, out) != 1) {
      fprintf(stderr, "ERROR - %s:write_output\n", Glob->prog_name);
      fprintf(stderr, "Cannot complete trailing FORTRAN rec len write to output file\n");
      perror(file_path);
      fclose(out);
      return (R_FAILURE);
    }
    
  }
  
  fclose (out);
  
  /*
   * compress file if required
   */
  
  if (Glob->params.compress_output) {
    sprintf(call_str, "%s %s", Glob->params.compress_command, file_path);
    system(call_str);
  }
  
  /*
   * Write the LDATA file, if requested.
   */

  if (Glob->params.use_ldata_info)
  {
    if (Glob->params.compress_output)
      LDATA_info_write(output_ldata,
		       Glob->params.output_dir,
		       Rfrtime2utime(&v_handle->vol_params->mid_time),
		       Glob->params.compress_file_ext,
		       "",
		       "",
		       0,
		       (int *)NULL);
    else
      LDATA_info_write(output_ldata,
		       Glob->params.output_dir,
		       Rfrtime2utime(&v_handle->vol_params->mid_time),
		       Glob->params.output_file_ext,
		       "",
		       "",
		       0,
		       (int *)NULL);
  }
  
  return (R_SUCCESS);
  
}


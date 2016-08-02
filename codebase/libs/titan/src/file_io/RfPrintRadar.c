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
/*************************************************************************
 *
 * RfRadarPrint.c
 *
 * Radar file printing routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * January 1995
 *
 **************************************************************************/

#include <titan/radar.h>

#define BOOL_STR(a) (a == FALSE ? "false" : "true")

static int Z_in_deg = FALSE;

/*-------------------------------
 */

void RfPrintCartParams(FILE *out,
		       const char *spacer,
		       cart_params_t *cart)
     
{

  cart_float_params_t fl_cart;

  RfDecodeCartParams(cart, &fl_cart);
  
  fprintf(out, "%sCartesian grid parameters : \n", spacer);

  fprintf(out, "%s  nbytes_char : %ld\n",
	  spacer, (long) cart->nbytes_char);

  fprintf(out, "%s  cart latitude : %g\n",
	  spacer, fl_cart.latitude);
  fprintf(out, "%s  cart longitude : %g\n",
	  spacer, fl_cart.longitude);
  fprintf(out, "%s  cart rotation : %g\n",
	  spacer, fl_cart.rotation);

  fprintf(out, "%s  nx, ny, nz : %d, %d, %d\n",
	  spacer,
	  cart->nx, cart->ny, cart->nz);

  fprintf(out, "%s  minx, miny, minz : %g, %g, %g\n",
	  spacer,
	  fl_cart.minx, fl_cart.miny, fl_cart.minz);
  
  fprintf(out, "%s  dx, dy, dz : %g, %g, %g\n", spacer,
	  fl_cart.dx, fl_cart.dy, fl_cart.dz);
  
  fprintf(out, "%s  radarx, radary, radarz : %g, %g, %g\n",
	  spacer,
	  fl_cart.radarx, fl_cart.radary, fl_cart.radarz);
  
  fprintf(out, "%s  dz_constant: %s\n", spacer,
	  BOOL_STR(cart->dz_constant));

  fprintf(out, "%s  x units : %s\n", spacer, cart->unitsx);
  fprintf(out, "%s  y units : %s\n", spacer, cart->unitsy);
  fprintf(out, "%s  z units : %s\n", spacer, cart->unitsz);

  if (strstr(cart->unitsz, "deg")) {
    Z_in_deg = TRUE;
  } else {
    Z_in_deg = FALSE;
  }

}

/*------------------------------------
 */

void RfPrintFieldParams(FILE *out,
			const char *spacer,
			si32 field_num,
			field_params_t *fparams,
			si32 *field_params_offset,
			si32 nplanes,
			si32 **plane_offset)
     
{
  
  si32 iplane;
  double scale, bias;

  fprintf(out, "%sParams for field number %d: \n",
	  spacer, field_num);
  fprintf(out, "\n");
  
  fprintf(out, "%s  field params offset : %ld\n", spacer,
	  (long) field_params_offset[field_num]);

  if (fparams->encoded == TRUE) {
    fprintf(out, "%s  data is run-length encoded\n", spacer);
  }
  
  scale = (float) fparams->scale / (float) fparams->factor;
  bias = (float) fparams->bias / (float) fparams->factor;
  
  fprintf(out, "%s  scale : %g\n", spacer, scale);
  fprintf(out, "%s  bias : %g\n", spacer, bias);
  fprintf(out, "%s  missing data value : %d\n", spacer,
	  fparams->missing_val);
  fprintf(out, "%s  noise value : %g\n", spacer,
	  (float) fparams->noise * scale + bias);
  fprintf(out, "%s  transform : %s\n", spacer, fparams->transform);
  fprintf(out, "%s  name : %s\n", spacer, fparams->name);
  fprintf(out, "%s  units : %s\n", spacer, fparams->units);
  fprintf(out, "%s  nbytes_char : %ld\n", spacer,
	  (long) fparams->nbytes_char);

  fprintf(out, "\n");
  fprintf(out, "%s  Plane offsets : \n", spacer);
  
  for (iplane = 0; iplane < nplanes; iplane++) {
    fprintf(out, "%s    Plane number %ld, offset %ld\n", spacer,
	    (long) iplane,
	    (long) plane_offset[field_num][iplane]);
  } /* iplane */
  
  fprintf(out, "\n");

  return;

}

/*------------------------------------
 */

void RfPrintPlaneHeights(FILE *out,
			 const char *spacer,
			 si32 nplanes,
			 si32 **plane_heights,
			 double scalez)
     
{

  char *units;
  si32 iplane;
  
  if (Z_in_deg) {
    units = "deg";
  } else {
    units = "km";
  }

  fprintf(out, "\n");
  fprintf(out, "%sPlane heights (lower, mid, upper) : \n\n",
	  spacer);

  for (iplane = 0; iplane < nplanes; iplane++) {
    fprintf(out, "%s  Plane %5ld, heights (%s) = %10.3f %10.3f %10.3f\n",
	    spacer, (long) iplane, units,
	    (double) plane_heights[iplane][PLANE_BASE_INDEX] /scalez,
	    (double) plane_heights[iplane][PLANE_MIDDLE_INDEX] / scalez,
	    (double) plane_heights[iplane][PLANE_TOP_INDEX] / scalez);
  }
  
  fprintf(out, "\n");

}

/*------------------------------------
 */

void RfPrintFloatPlaneHeights(FILE *out,
			      const char *spacer,
			      si32 nplanes,
			      double **plane_heights)
     
{

  si32 iplane;
  
  fprintf(out, "\n");
  fprintf(out, "%sPlane heights (lower, mid, upper) : \n\n",
	  spacer);
  
  for (iplane = 0; iplane < nplanes; iplane++) {
    fprintf(out, "%s  Plane %5ld, heights (km) = %10.3f %10.3f %10.3f\n",
	    spacer, (long) iplane,
	    plane_heights[iplane][PLANE_BASE_INDEX],
	    plane_heights[iplane][PLANE_MIDDLE_INDEX],
	    plane_heights[iplane][PLANE_TOP_INDEX]);
  }
  
  fprintf(out, "\n");

}

/*------------------------------------
 */

void RfPrintRadarElevations(FILE *out,
			    const char *spacer,
			    const char *label,
			    si32 nelevations,
			    si32 *radar_elevations)
     
{
  
  si32 ielev;

  fprintf(out, "\n%s%s : \n\n", spacer, label);

  for (ielev = 0; ielev < nelevations; ielev++) {
    fprintf(out, "%s  Elev. number %ld = %g deg.\n",
	    spacer, (long) ielev,
	    (double) radar_elevations[ielev] / DEG_FACTOR);
  }
  
  fprintf(out, "\n");

}

/*--------------------------
 */

void RfPrintRadTime(FILE *out,
		    const char *spacer,
		    const char *label,
		    radtim_t *radtim)
     
{

  fprintf(out, "%s%s : %.4d/%.2d/%.2d %.2d:%.2d:%.2d\n",
	  spacer, label,
	  radtim->year,
	  radtim->month,
	  radtim->day,
	  radtim->hour,
	  radtim->min,
	  radtim->sec);
  
}

/*-----------------------------
 */

void RfPrintVolParams(FILE *out,
		      const char *spacer,
		      vol_params_t *vol_params)
     
{

  char buf[VOL_PARAMS_NOTE_LEN];
  char *token;

  fprintf(out, "%sRadar volume parameters\n", spacer);
  fprintf(out, "\n");

  fprintf(out, "%s  Note:\n", spacer);
  strncpy(buf, vol_params->note, VOL_PARAMS_NOTE_LEN);
  buf[VOL_PARAMS_NOTE_LEN - 1] = '\0';
  token = strtok(buf, "\n");
  while (token != NULL) {
    fprintf(out, "%s    %s\n", spacer, token);
    token = strtok(NULL, "\n");
  }
  fprintf(out, "\n");

  fprintf(out, "%s  Dates and times : \n", spacer);
  RfPrintRadTime(out, spacer, "    File  ", &vol_params->file_time);
  RfPrintRadTime(out, spacer, "    Start ", &vol_params->start_time);
  RfPrintRadTime(out, spacer, "    Mid   ", &vol_params->mid_time);
  RfPrintRadTime(out, spacer, "    End   ", &vol_params->end_time);
  fprintf(out, "\n");

  fprintf(out, "%s  Radar parameters : \n", spacer);
  fprintf(out, "%s    radar_id : %d\n", spacer,
	  vol_params->radar.radar_id);
  fprintf(out, "%s    altitude (meters) : %d\n", spacer,
	  vol_params->radar.altitude);
  fprintf(out, "%s    latitude (deg) : %g\n", spacer,
	  (float) vol_params->radar.latitude / 1000000.0);
  fprintf(out, "%s    longitude (deg) : %g\n", spacer,
	  (float) vol_params->radar.longitude / 1000000.0);
  fprintf(out, "%s    nelevations : %d\n", spacer,
	  vol_params->radar.nelevations);
  fprintf(out, "%s    nazimuths : %d\n", spacer,
	  vol_params->radar.nazimuths);
  fprintf(out, "%s    ngates : %d\n", spacer,
	  vol_params->radar.ngates);
  fprintf(out, "%s    gate_spacing (meters) : %g\n", spacer,
	  (float) vol_params->radar.gate_spacing / 1000.0);
  fprintf(out, "%s    start_range (meters) : %g\n", spacer,
	  (float) vol_params->radar.start_range / 1000.0);
  fprintf(out, "%s    delta_azimuth (deg) : %g\n", spacer,
	  (float) vol_params->radar.delta_azimuth / 1000000.0);
  fprintf(out, "%s    start_azimuth (deg) : %g\n", spacer,
	  (float) vol_params->radar.start_azimuth / 1000000.0);
  fprintf(out, "%s    beam_width (deg) : %g\n", spacer,
	  (float) vol_params->radar.beam_width / 1000000.0);
  fprintf(out, "%s    samples_per_beam : %d\n", spacer,
	  vol_params->radar.samples_per_beam);
  fprintf(out, "%s    pulse_width (micro_seconds) : %g\n", spacer,
	  (float) vol_params->radar.pulse_width / 1000.0);
  fprintf(out, "%s    prf (Hz) : %g\n", spacer,
	  (float) vol_params->radar.prf / 1000.0);
  fprintf(out, "%s    wavelength (cm) : %g\n", spacer,
	  (float) vol_params->radar.wavelength / 10000.0);
  fprintf(out, "%s    n missing rays : %d\n", spacer,
	  vol_params->radar.nmissing);
  fprintf(out, "%s    radar name : %s\n", spacer, vol_params->radar.name);
  fprintf(out, "%s    nbytes_char : %ld\n", spacer,
	  (long) vol_params->radar.nbytes_char);
  fprintf(out, "\n");

  sprintf(buf, "%s  ", spacer);
  RfPrintCartParams(out, buf, &vol_params->cart);
  
  fprintf(out, "%s   nfields : %ld\n", spacer, (long int)vol_params->nfields);
  
  return;
    
}


void RfPrintVhandle(FILE *out,
		    const char *spacer,
		    vol_file_handle_t *v_handle)
{
  int i;
  char *next_spacer;
  
  next_spacer = (char *)malloc(strlen(spacer) + 4);
  sprintf(next_spacer, "%s   ", spacer);
  
  fprintf(out, "\n");
  fprintf(out, "%sVolume File Handle:\n", spacer);
  
  fprintf(out, "%s   prog_name : <%s>\n", spacer,
	  v_handle->prog_name);
  fprintf(out, "%s   vol_file_path : <%s>\n", spacer,
	  v_handle->vol_file_path);
  fprintf(out, "%s   vol_file_label : <%s>\n", spacer,
	  v_handle->vol_file_label);
  fprintf(out, "%s   handle_initialized : %s\n", spacer,
	  BOOL_STR(v_handle->handle_initialized));
  
  RfPrintVolParams(out, next_spacer,
		   v_handle->vol_params);
  
  RfPrintRadarElevations(out, next_spacer,
			 "Elev ",
			 v_handle->vol_params->radar.nelevations,
			 v_handle->radar_elevations);
  RfPrintPlaneHeights(out, next_spacer,
		      v_handle->vol_params->cart.nz,
		      v_handle->plane_heights,
		      (double)v_handle->vol_params->cart.scalez);
  
  for (i = 0; i < v_handle->vol_params->nfields; i++)
    RfPrintFieldParams(out, next_spacer,
		       i, v_handle->field_params[i],
		       v_handle->field_params_offset,
		       v_handle->nplanes_allocated,
		       v_handle->plane_offset);

  fprintf(out, "%s   params_allocated:         %d\n", spacer,
	  v_handle->params_allocated);
  fprintf(out, "%s   arrays_allocated:         %d\n", spacer,
	  v_handle->arrays_allocated);
  fprintf(out, "%s   nelevations_allocated:    %d\n", spacer,
	  v_handle->nelevations_allocated);
  fprintf(out, "%s   nfields_allocated:        %d\n", spacer,
	  v_handle->nfields_allocated);
  fprintf(out, "%s   nplanes_allocated:        %d\n", spacer,
	  v_handle->nplanes_allocated);
  
  return;
}

		  
void RfPrintScanTable(FILE *out, const char *spacer, scan_table_t *table)
     
{
  
  int ielev, iaz;
  scan_table_az_t *az;
  scan_table_elev_t *elev;

  fprintf(out, "\n");
  fprintf(out, "%sScan table:\n", spacer);

  fprintf(out, "%s  use_azimuth_table : %s\n", spacer,
	  BOOL_STR(table->use_azimuth_table));
  fprintf(out, "%s  extend_below : %s\n", spacer,
	  BOOL_STR(table->extend_below));
  fprintf(out, "%s  missing_data_index : %ld\n", spacer,
	  (long) table->missing_data_index);

  fprintf(out, "%s  nelevations : %ld\n", spacer, (long) table->nelevations);
  fprintf(out, "%s  nazimuths : %ld\n", spacer, (long) table->nazimuths);
  fprintf(out, "%s  ngates : %ld\n", spacer, (long) table->ngates);
  fprintf(out, "%s  nbeams_vol : %ld\n", spacer, (long) table->nbeams_vol);

  fprintf(out, "%s  delta_azimuth (deg) : %g\n", spacer,
	  table->delta_azimuth);
  fprintf(out, "%s  start_azimuth (deg) : %g\n", spacer,
	  table->start_azimuth);
  fprintf(out, "%s  beam_width (deg) : %g\n", spacer,
	  table->beam_width);
  fprintf(out, "%s  gate_spacing (km) : %g\n", spacer,
	  table->gate_spacing);
  fprintf(out, "%s  start_range (km) : %g\n", spacer,
	  table->start_range);

  fprintf(out, "\n");
  fprintf(out, "%s  N elevations: %ld\n", spacer, (long) table->nelevations);

  fprintf(out, "%s    Elevations:\n", spacer);
  for (ielev = 0; ielev < table->nelevations; ielev++) {
    fprintf(out, "%s      %d: %g\n", spacer, ielev,
	    table->elev_angles[ielev]);
  }

  fprintf(out, "%s    Extended elev array:\n", spacer);
  for (ielev = 0; ielev < table->nelevations + 2; ielev++) {
    fprintf(out, "%s      %d: %g\n", spacer, ielev,
	    table->ext_elev_angles[ielev]);
  }
  
  fprintf(out, "%s    Elev limits:\n", spacer);
  for (ielev = 0; ielev < table->nelevations + 1; ielev++) {
    fprintf(out, "%s      %d: %g\n", spacer, ielev,
	    table->elev_limits[ielev]);
  }

  if (table->use_azimuth_table) {

    elev = table->elevs;
    for (ielev = 0; ielev < table->nelevations; ielev++, elev++) {
      fprintf(out, "%s    Start beam_num: %ld\n", spacer,
	      (long) elev->start_beam_num);
      fprintf(out, "%s    End   beam_num: %ld\n", spacer,
	      (long) elev->end_beam_num);
      fprintf(out, "%s    N azimuths: %ld\n", spacer, (long) elev->naz);
      fprintf(out, "%s    Az reference: %g\n", spacer, elev->az_reference);
      fprintf(out, "%s    Azimuths:\n", spacer);
      az = elev->azs;
      for (iaz = 0; iaz < elev->naz + 1; iaz++, az++) {
	if (iaz < elev->naz) {
	  fprintf(out, "%s      beam_num: %3ld, rel_limit: "
		  "%10.4f, angle: %10.4f\n", spacer,
		  (long) az->beam_num,
		  elev->rel_az_limits[iaz], az->angle);
	} else {
	  fprintf(out, "%s                     rel_limit: %10.4f\n",
		  spacer, elev->rel_az_limits[iaz]);
	}
      } /* iaz */
    } /* ielev */

  } /* if (table->use_azimuth_table) */

  return;

}


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
 * mdv_radar.c
 *
 * mdv radar struct routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1997
 *
 **************************************************************************/

#include <dataport/bigend.h>
#include <Mdv/mdv/mdv_radar.h>

#define BOOL_STR(a) (a == 0 ? "false" : "true")

void MDVPrintRadarGrid(FILE *out,
		       char *spacer,
		       MDV_radar_grid_t *grid)
     
{

  fprintf(out, "%sMDV radar grid parameters : \n", spacer);

  fprintf(out, "%s  nbytes_char : %ld\n",
	  spacer, (long) grid->nbytes_char);

  fprintf(out, "%s  grid latitude : %g\n",
	  spacer, grid->latitude);
  fprintf(out, "%s  grid longitude : %g\n",
	  spacer, grid->longitude);
  fprintf(out, "%s  grid rotation : %g\n",
	  spacer, grid->rotation);

  fprintf(out, "%s  nx, ny, nz : %d, %d, %d\n",
	  spacer,
	  grid->nx, grid->ny, grid->nz);

  fprintf(out, "%s  minx, miny, minz : %g, %g, %g\n",
	  spacer,
	  grid->minx, grid->miny, grid->minz);
  
  fprintf(out, "%s  dx, dy, dz : %g, %g, %g\n", spacer,
	  grid->dx, grid->dy, grid->dz);
  
  fprintf(out, "%s  radarx, radary, radarz : %g, %g, %g\n",
	  spacer,
	  grid->radarx, grid->radary, grid->radarz);
  
  fprintf(out, "%s  dz_constant: %s\n", spacer,
	  BOOL_STR(grid->dz_constant));

  fprintf(out, "%s  x units : %s\n", spacer, grid->unitsx);
  fprintf(out, "%s  y units : %s\n", spacer, grid->unitsy);
  fprintf(out, "%s  z units : %s\n", spacer, grid->unitsz);


}

/*------------------------------------
 */

void MDVPrintRadarElevations(FILE *out,
			     char *spacer,
			     char *label,
			     si32 nelevations,
			     fl32 *radar_elevations)
     
{
  
  si32 ielev;
  
  fprintf(out, "\n%s%s : \n\n", spacer, label);
  
  for (ielev = 0; ielev < nelevations; ielev++) {
    fprintf(out, "%s  Elev. number %ld = %g deg.\n",
	    spacer, (long) ielev,
	    (double) radar_elevations[ielev]);
  }
  
  fprintf(out, "\n");

}

/*------------------------------------
 */

void MDVPrintRadarField(FILE *out,
			char *spacer,
			si32 field_num,
			MDV_radar_field_t *field)
     
{
  
  fprintf(out, "%sParams for field number %d: \n",
	  spacer, field_num);
  fprintf(out, "\n");
  
  if (field->encoded) {
    fprintf(out, "%s  data is run-length encoded\n", spacer);
  }
  
  fprintf(out, "%s  scale : %g\n", spacer, field->scale);
  fprintf(out, "%s  bias : %g\n", spacer, field->bias);
  fprintf(out, "%s  missing data value : %d\n", spacer,
	  field->missing_val);
  fprintf(out, "%s  noise value : %g\n", spacer,
	  (float) field->noise * field->scale + field->bias);
  fprintf(out, "%s  transform : %s\n", spacer, field->transform);
  fprintf(out, "%s  name : %s\n", spacer, field->name);
  fprintf(out, "%s  units : %s\n", spacer, field->units);
  fprintf(out, "%s  nbytes_char : %ld\n", spacer,
	  (long) field->nbytes_char);

  fprintf(out, "\n");

  return;

}

/*-----------------------------
 */

void MDVPrintRadarParams(FILE *out,
			 char *spacer,
			 MDV_radar_params_t *rparams)
     
{

  fprintf(out, "%s  MDV radar volume parameters\n", spacer);
  fprintf(out, "\n");
  
  fprintf(out, "%s    nelevations : %d\n", spacer,
	  rparams->nelevations);
  fprintf(out, "%s    nazimuths : %d\n", spacer,
	  rparams->nazimuths);
  fprintf(out, "%s    ngates : %d\n", spacer,
	  rparams->ngates);

  fprintf(out, "%s    radar_id : %d\n", spacer,
	  rparams->radar_id);

  fprintf(out, "%s    samples_per_beam : %d\n", spacer,
	  rparams->samples_per_beam);

  fprintf(out, "%s    altitude (km) : %g\n", spacer,
	  rparams->altitude);
  fprintf(out, "%s    latitude (deg) : %g\n", spacer,
	  rparams->latitude);
  fprintf(out, "%s    longitude (deg) : %g\n", spacer,
	  rparams->longitude);

  fprintf(out, "%s    gate_spacing (km) : %g\n", spacer,
	  rparams->gate_spacing);
  fprintf(out, "%s    start_range (km) : %g\n", spacer,
	  rparams->start_range);
  
  fprintf(out, "%s    delta_azimuth (deg) : %g\n", spacer,
	  rparams->delta_azimuth);
  fprintf(out, "%s    start_azimuth (deg) : %g\n", spacer,
	  rparams->start_azimuth);
  fprintf(out, "%s    beam_width (deg) : %g\n", spacer,
	  rparams->beam_width);

  fprintf(out, "%s    pulse_width (micro_seconds) : %g\n", spacer,
	  rparams->pulse_width);
  fprintf(out, "%s    prf (Hz) : %g\n", spacer,
	  rparams->prf);
  fprintf(out, "%s    wavelength (cm) : %g\n", spacer,
	  rparams->wavelength);
  fprintf(out, "%s    nyquist_freq (s-1) : %g\n", spacer,
	  rparams->nyquist_freq);

  fprintf(out, "%s    radar name : %s\n", spacer, rparams->name);

  fprintf(out, "%s    nbytes_char : %ld\n", spacer,
	  (long) rparams->nbytes_char);
  fprintf(out, "\n");

  return;
    
}

/*
 * byte-swapping routines
 */

void BE_to_MDV_radar_params(MDV_radar_params_t *rparams)

{

  si32 nbytes_char;

  nbytes_char = BE_to_si32(rparams->nbytes_char);

  BE_to_array_32(rparams,
		 sizeof(MDV_radar_params_t) - nbytes_char);

}

void BE_from_MDV_radar_params(MDV_radar_params_t *rparams)

{

  BE_from_array_32(rparams,
		   sizeof(MDV_radar_params_t) - rparams->nbytes_char);

}

void BE_to_MDV_radar_field(MDV_radar_field_t *rfield)

{

  si32 nbytes_char;

  nbytes_char = BE_to_si32(rfield->nbytes_char);

  BE_to_array_32(rfield,
		 sizeof(MDV_radar_field_t) - nbytes_char);

}

void BE_from_MDV_radar_field(MDV_radar_field_t *rfield)

{

  BE_from_array_32(rfield,
		   sizeof(MDV_radar_field_t) - rfield->nbytes_char);

}

void BE_to_MDV_radar_grid(MDV_radar_grid_t *rgrid)

{

  si32 nbytes_char;

  nbytes_char = BE_to_si32(rgrid->nbytes_char);

  BE_to_array_32(rgrid,
		 sizeof(MDV_radar_grid_t) - nbytes_char);

}

void BE_from_MDV_radar_grid(MDV_radar_grid_t *rgrid)

{

  BE_from_array_32(rgrid,
		   sizeof(MDV_radar_grid_t) - rgrid->nbytes_char);

}


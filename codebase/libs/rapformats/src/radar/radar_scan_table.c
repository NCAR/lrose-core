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
 * RadarScanTable.c
 *
 * Scan table access routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Sept 1995
 *
 **************************************************************************/

#include <toolsa/toolsa_macros.h>
#include <toolsa/mem.h>
#include <rapformats/radar_scan_table.h>

#define MAX_SEQ 256
#define MAX_LINE 256

#define BOOL_STR(a) (a == FALSE ? "false" : "true")

static si32 get_ang_index(fl32 search_ang,
			  si32 n_ang,
			  fl32 *ang_limits,
			  fl32 bottom_limit);

static int get_next_line(FILE *table_file,
			 char *line,
			 int *line_num);

/*******************
 * RadarInitScanTable()
 *
 * Initialize the scan table - set all members to 0
 */

void RadarInitScanTable(radar_scan_table_t *table) {

  MEM_zero(*table);

}

/*************************
 * RadarAllocScanTableElevs()
 *
 * Allocate the elevation structures
 */

void RadarAllocScanTableElevs(radar_scan_table_t *table, int nelevations) {
  
  if (table->use_azimuth_table) {
    table->elevs = (radar_scan_table_elev_t *) ucalloc
      (nelevations, sizeof(radar_scan_table_elev_t));
    table->ext_elevs = (radar_scan_table_elev_t *) ucalloc
      (nelevations + 2, sizeof(radar_scan_table_elev_t));
  }
  
  table->elev_angles = (fl32 *) ucalloc
    (nelevations, sizeof(fl32));
  
  table->elev_limits = (fl32 *) ucalloc
    (nelevations + 1, sizeof(fl32));
  
  table->ext_elev_angles = (fl32 *) ucalloc
    (nelevations + 2, sizeof(fl32));
  
}

/****************************
 * RadarAllocScanTableAzArrays()
 *
 * Allocate the azimuth table
 */

void RadarAllocScanTableAzArrays(radar_scan_table_t *table,
				 int ielev, int nazimuths)
     
{

  radar_scan_table_elev_t *elev = table->elevs + ielev;

  elev->azs = (radar_scan_table_az_t *) ucalloc
    (nazimuths, sizeof(radar_scan_table_az_t));
  
  elev->rel_az_limits = (fl32 *) ucalloc
    (nazimuths + 1, sizeof(fl32));
  
}

/*************************
 * RadarFreeScanTableArrays()
 *
 * Free up
 */

void RadarFreeScanTableArrays(radar_scan_table_t *table)
     
{

  int ielev;
  radar_scan_table_elev_t *elev = table->elevs;

  if (table->elevs) {

    for (ielev = 0; ielev < table->nelevations; ielev++, elev++) {
      if (elev->rel_az_limits)
	ufree(elev->rel_az_limits);
      if (elev->azs)
	ufree(elev->azs);
    } /* ielev */

    ufree(table->elevs);

  } /* if (table->use_azimuth_table) */

  if (table->ext_elevs)
    ufree(table->ext_elevs);

  if (table->elev_angles)
    ufree(table->elev_angles);

  if (table->elev_limits)
    ufree(table->elev_limits);  

  if(table->ext_elev_angles)
    ufree(table->ext_elev_angles);

}

/*************************************************************************
 *
 * RadarReadScanTable()
 *
 * Reads in scan table.
 *
 * Allocates memory as required.
 *
 * Returns 0 or -1
 *
 **************************************************************************/

#define THIS_ROUTINE "RadarReadScanTable"

int RadarReadScanTable(radar_scan_table_t *table,
		       const char *file_path,
		       const char *calling_routine)
     
{

  char calling_sequence[MAX_SEQ];
  char line[MAX_LINE];
  int line_num = 0;
  int beam_num = 0;
  int ielev, iaz;
  long n;
  fl32 min_az;
  radar_scan_table_az_t *az;
  radar_scan_table_elev_t *elev;
  FILE *table_file;

  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

  /*
   * initialize
   */

  RadarInitScanTable(table);

  /*
   * set use_azimuth_table flag
   */

  table->use_azimuth_table = TRUE;

  /*
   * open file
   */

  if ((table_file = fopen(file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR: %s\n", calling_sequence);
    fprintf(stderr, "Cannot open table file\n");
    perror(file_path);
    return (-1);
  }

  /*
   * read in nelevations
   */

  if (get_next_line(table_file, line, &line_num)) {
    fprintf(stderr, "ERROR: %s\n", calling_sequence);
    fprintf(stderr, "Reading file %s, line %d\n", file_path, line_num);
    fclose(table_file);
    return (-1);
  }
  
  if (sscanf(line, "nelevations %ld", &n) != 1) {
    fprintf(stderr, "ERROR: %s\n", calling_sequence);
    fprintf(stderr, "Reading file %s, line %d\n", file_path, line_num);
    fprintf(stderr, "Token 'nelevations'\n");
    fclose(table_file);
    return (-1);
  }
  table->nelevations = n;
  
  /*
   * alloc for elevs
   */

  RadarAllocScanTableElevs(table, table->nelevations);

  /*
   * load up elevs
   */

  table->max_azimuths = 0;
  elev = table->elevs;
  for (ielev = 0; ielev < table->nelevations; ielev++, elev++) {

    /*
     * set beam_num
     */

    elev->start_beam_num = beam_num;
    
    /*
     * read in elevation angle
     */
    
    if (get_next_line(table_file, line, &line_num)) {
      fprintf(stderr, "ERROR: %s\n", calling_sequence);
      fprintf(stderr, "Reading file %s, line %d\n", file_path, line_num);
      fclose(table_file);
      return (-1);
    }
    
    if (sscanf(line, "elevation %g", &table->elev_angles[ielev]) != 1) {
      fprintf(stderr, "ERROR: %s\n", calling_sequence);
      fprintf(stderr, "Reading file %s, line %d\n", file_path, line_num);
      fprintf(stderr, "Token 'elevation'\n");
      fclose(table_file);
      return (-1);
    }

    /*
     * read in naz
     */
    
    if (get_next_line(table_file, line, &line_num)) {
      fprintf(stderr, "ERROR: %s\n", calling_sequence);
      fprintf(stderr, "Reading file %s, line %d\n", file_path, line_num);
      fclose(table_file);
      return (-1);
    }
    
    if (sscanf(line, "nazimuths %ld", &n) != 1) {
      fprintf(stderr, "ERROR: %s\n", calling_sequence);
      fprintf(stderr, "Reading file %s, line %d\n", file_path, line_num);
      fprintf(stderr, "Token 'nazimuths'\n");
      fclose(table_file);
      return (-1);
    }
    elev->naz = n;

    table->max_azimuths = MAX(table->max_azimuths, elev->naz);
    table->nazimuths = table->max_azimuths;

    /*
     * alloc for azimuths
     */

    RadarAllocScanTableAzArrays(table, ielev, elev->naz);
    
    /*
     * read in azimuths
     */
  
    min_az = 1000000.0;
    az = elev->azs;
    for (iaz = 0; iaz < elev->naz; iaz++, az++) {

      /*
       * set beam_num
       */

      elev->end_beam_num = beam_num;
      az->beam_num = beam_num;
      beam_num++;
      
      if (get_next_line(table_file, line, &line_num)) {
	fprintf(stderr, "ERROR: %s\n", calling_sequence);
	fprintf(stderr, "Reading file %s, line %d\n", file_path, line_num);
	fclose(table_file);
	return (-1);
      }
      
      if (sscanf(line, "%g", &az->angle) != 1) {
	fprintf(stderr, "ERROR: %s\n", calling_sequence);
	fprintf(stderr, "Reading file %s, line %d\n", file_path, line_num);
	fprintf(stderr, "Reading azimuth line\n");
	fclose(table_file);
	return (-1);
      }
      
      min_az = MIN(min_az, az->angle);
      
    } /* iaz */

  } /* ielev */

  /*
   * set number of indices in table
   */
  
  table->nbeams_vol = beam_num;

  fclose(table_file);

  return (0);

}

#undef THIS_ROUTINE

/*
 * Compute limits of elevation influence.
 *
 * The lower limits are computed. One extra limit is included, the
 * lower limit of the angle above the max, which is effectively the
 * upper limit of the max angle.
 */

void RadarComputeScanTableElevLimits(radar_scan_table_t *table)
     
{

  si32 ielev;
  si32 nelevations = table->nelevations;
  fl32 bottom_limit;
  fl32 top_limit;

  if (table->extend_below) {
    bottom_limit = -10.0;
  } else {
    bottom_limit = (table->elev_angles[0] -
		    (table->elev_angles[1] - table->elev_angles[0]) / 2.0);
  }
  
  top_limit = (table->elev_angles[nelevations - 1] +
	       (table->elev_angles[nelevations - 1] -
		table->elev_angles[nelevations - 2]) / 2.0);
  
  /*
   * load up elev limits
   */
  
  table->elev_limits[0] = bottom_limit;
  
  for (ielev = 1; ielev < nelevations; ielev++) {
    table->elev_limits[ielev] = 
      (table->elev_angles[ielev - 1] +
       table->elev_angles[ielev]) / 2.0;
  }

  table->elev_limits[nelevations] = top_limit;

}

/*
 * Compute extended elevation array
 *
 * This array has 2 extra elements, one at each end. These are
 * used for the decision of whether the elevation angle is close
 * enough to the edge elevations to be used or not.
 */

void RadarComputeScanTableExtElev(radar_scan_table_t *table)
     
{

  si32 ielev;
  si32 nelevations = table->nelevations;

  for (ielev = 0; ielev < nelevations; ielev++) {
    table->ext_elev_angles[ielev + 1] = table->elev_angles[ielev];
  }
  
  if (table->extend_below) {
    table->ext_elev_angles[0] = -10.0;
  } else {
    table->ext_elev_angles[0] =
      table->elev_angles[0] - 
      ((table->elev_angles[0] - table->elev_limits[0]) * 2.0);
  }

  table->ext_elev_angles[nelevations + 1] =
    table->elev_angles[nelevations - 1] +
    ((table->elev_limits[nelevations] -
      table->elev_angles[nelevations - 1]) * 2.0);
  
}

/*
 * Compute limits of azimuth influence.
 *
 * The lower limits are computed. One extra limit is included, the
 * lower limit of the angle above the max, which is effectively the
 * upper limit of the max angle.
 */

void RadarComputeScanTableAzLimits(radar_scan_table_t *table)
     
{

  si32 ielev, iaz;
  si32 naz;
  fl32 bottom_limit;
  fl32 top_limit;
  radar_scan_table_az_t *az;
  radar_scan_table_elev_t *elev;

  elev = table->elevs;
  for (ielev = 0; ielev < table->nelevations; ielev++, elev++) {
    
    naz = elev->naz;
    
    bottom_limit = (elev->azs[0].angle -
		    (elev->azs[1].angle - elev->azs[0].angle) / 2.0);
    
    top_limit = (elev->azs[naz - 1].angle +
		 (elev->azs[naz - 1].angle -
		  elev->azs[naz - 2].angle) / 2.0);
    /*
     * load up az limits
     */
    
    elev->az_reference = bottom_limit;
    
    elev->rel_az_limits[0] = bottom_limit;
    
    az = elev->azs;
    for (iaz = 1; iaz < naz; iaz++, az++) {
      elev->rel_az_limits[iaz] = 
	(elev->azs[iaz - 1].angle +
	 elev->azs[iaz].angle) / 2.0;
    }

    elev->rel_az_limits[naz] = top_limit;

    /*
     * compute the limits relative to the lower limit
     */

    for (iaz = 0; iaz < naz + 1; iaz++) {
      elev->rel_az_limits[iaz] -= bottom_limit;
    }

  } /* ielev */

}

/*
 * Load up the extended elev structs.
 *
 * This array has 2 extra elements, one at each end. These are
 * used in the computation of the lookup tables.
 */

void RadarLoadScanTableExtElevs(radar_scan_table_t *table)
     
{
  
  int ielev;

  /*
   * copy the elevs array
   */

  for (ielev = 0; ielev < table->nelevations; ielev++) {
    table->ext_elevs[ielev + 1] = table->elevs[ielev];
  } /* ielev */

  /*
   * duplicate the end points
   */

  table->ext_elevs[0] = table->ext_elevs[1];
  table->ext_elevs[table->nelevations + 1] =
    table->ext_elevs[table->nelevations];

}

/*
 * get az index in table given elevation struct and azimuth angle
 */

si32 RadarScanTableAng2AzNum(radar_scan_table_elev_t *elev,
			     fl32 az_ang)
     
{
  
  return (get_ang_index(az_ang,
			elev->naz,
			elev->rel_az_limits,
			elev->az_reference));

}

/*
 * get elev index in table given elevation angle
 */

si32 RadarScanTableAng2ElNum(radar_scan_table_t *table,
			     fl32 elev_ang)
     
{
  
  return (get_ang_index(elev_ang,
			table->nelevations,
			table->elev_limits, 0.0));

}

/*
 * get beam_num in table given elevation and azimuth angles
 */

si32 RadarScanTableAngs2BeamNum(radar_scan_table_t *table,
				fl32 elev_ang, fl32 az_ang)
     
{
  
  si32 jelev, jaz;
  radar_scan_table_elev_t *elev;
  
  jelev = get_ang_index(elev_ang,
			table->nelevations,
			table->elev_limits, 0.0);
  
  if (jelev < 0) {

    return (-1L);

  } else {

    elev = table->elevs + jelev;
    jaz = get_ang_index(az_ang,
			elev->naz,
			elev->rel_az_limits,
			elev->az_reference);
    
    if (jaz >= 0) {
      return (elev->azs[jaz].beam_num);
    } else {
      return (-1L);
    }

  } /* if (jelev < 0) */
  
}

void RadarPrintScanTable(FILE *out, const char *spacer, radar_scan_table_t *table)
     
{
  
  int ielev, iaz;
  radar_scan_table_az_t *az;
  radar_scan_table_elev_t *elev;

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

/*
 * get the angle index, given a set of angle limits and
 * the minimim angle
 */

static si32 get_ang_index(fl32 search_ang,
			  si32 n_ang,
			  fl32 *ang_limits,
			  fl32 bottom_limit)
     
{
  
  si32 iang;
  fl32 rel_ang;

  /*
   * compute search ang relative to lower limit
   */

  rel_ang = search_ang - bottom_limit;

  /*
   * place between 0 and 360
   */

  while (rel_ang > 360.0) {
    rel_ang -= 360.0;
  }

  while (rel_ang < 0.0) {
    rel_ang += 360.0;
  }

  /*
   *  if ang is outside limits, return -1
   */

  if (rel_ang < ang_limits[0] ||
      rel_ang > ang_limits[n_ang]) {
    return (-1L);
  }

  /*
   * compute a starting point assuming evenly distributed
   * angles
   */

  iang = (si32) ((rel_ang / ang_limits[n_ang]) * n_ang);
  
  if (iang > n_ang - 1) {
    iang = n_ang - 1;
  } else if (iang < 0) {
    iang = 0;
  }
  
  if (ang_limits[iang] < rel_ang) {

    /*
     * starting point is below search point
     */

    while (ang_limits[iang + 1] < rel_ang) {
      iang++;
      if (iang > n_ang - 1) {
	iang = n_ang - 1;
	break;
      }
    }

  } else {

    /*
     * starting point is above search point
     */

    while (ang_limits[iang] > rel_ang) {
      iang--;
      if (iang < 0) {
	iang = 0;
	break;
      }
    }

  }

  return (iang);

}

/*
 * get_next_line()
 *
 * Gets next line which is not blank and does not start
 * with #
 */

static int get_next_line(FILE *table_file,
			 char *line,
			 int *line_num)
     
{
  
  char token[MAX_LINE];
  
  while (fgets(line, MAX_LINE, table_file) != NULL) {
    
    (*line_num)++;
    
    if (line[0] != '#') {
      if (sscanf(line, "%s", token) == 1) {
	return (0);
      }
    }

  }

  return (-1);

}

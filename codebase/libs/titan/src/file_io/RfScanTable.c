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
 * RfScanTable.c
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

#include <titan/file_io.h>
#include <titan/scan_table.h>
#include <toolsa/toolsa_macros.h>

#define MAX_SEQ 256
#define MAX_LINE 256

#define BOOL_STR(a) (a == FALSE ? "false" : "true")

static si32 get_ang_index(double search_ang,
			  si32 n_ang,
			  double *ang_limits,
			  double bottom_limit);

static int get_next_line(FILE *table_file,
			 char *line,
			 int *line_num);

/*
 * allocation routines
 */

void RfAllocScanTableElevs(scan_table_t *table) {
  
  if (table->use_azimuth_table) {
    table->elevs = (scan_table_elev_t *) umalloc
      ((table->nelevations) * sizeof(scan_table_elev_t));
    table->ext_elevs = (scan_table_elev_t *) umalloc
      ((table->nelevations + 2) * sizeof(scan_table_elev_t));
  }
  
  table->elev_angles = (double *) umalloc
    ((table->nelevations) * sizeof(double));
  
  table->elev_limits = (double *) umalloc
    ((table->nelevations + 1) * sizeof(double));
  
  table->ext_elev_angles = (double *) umalloc
    ((table->nelevations + 2) * sizeof(double));
  
}

void RfAllocScanTableAzArrays(scan_table_t *table,
			      int ielev)
     
{

  scan_table_elev_t *elev = table->elevs + ielev;

  elev->azs = (scan_table_az_t *) umalloc
    (elev->naz * sizeof(scan_table_az_t));
  
  elev->rel_az_limits = (double *) umalloc
    ((elev->naz + 1) * sizeof(double));
  
}

void RfFreeScanTableArrays(scan_table_t *table)
     
{

  int ielev;
  scan_table_elev_t *elev = table->elevs;

  if (table->use_azimuth_table) {

    for (ielev = 0; ielev < table->nelevations; ielev++, elev++) {
      ufree(elev->rel_az_limits);
      ufree(elev->azs);
    } /* ielev */

    ufree(table->elevs);
    ufree(table->ext_elevs);

  } /* if (table->use_azimuth_table) */

  ufree(table->elev_angles);  
  ufree(table->elev_limits);  
  ufree(table->ext_elev_angles);

}

/*************************************************************************
 *
 * RfReadScanTable()
 *
 * Reads in scan table.
 *
 * Allocates memory as required.
 *
 * Returns R_SUCCESS or R_FAILURE
 *
 **************************************************************************/

#define THIS_ROUTINE "RfReadScanTable"

int RfReadScanTable(scan_table_t *table,
		    const char *file_path,
		    const char *calling_routine)
     
{

  char calling_sequence[MAX_SEQ];
  char line[MAX_LINE];
  int line_num = 0;
  int beam_num = 0;
  int ielev, iaz;
  long n;
  double min_az;
  scan_table_az_t *az;
  scan_table_elev_t *elev;
  FILE *table_file;

  /*
   * set up calling sequence
   */

  sprintf(calling_sequence, "%s:%s",
	  calling_routine, THIS_ROUTINE);

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
    return (R_FAILURE);
  }

  /*
   * read in nelevations
   */

  if (get_next_line(table_file, line, &line_num)) {
    fprintf(stderr, "ERROR: %s\n", calling_sequence);
    fprintf(stderr, "Reading file %s, line %d\n", file_path, line_num);
    fclose(table_file);
    return (R_FAILURE);
  }
  
  if (sscanf(line, "nelevations %ld", &n) != 1) {
    fprintf(stderr, "ERROR: %s\n", calling_sequence);
    fprintf(stderr, "Reading file %s, line %d\n", file_path, line_num);
    fprintf(stderr, "Token 'nelevations'\n");
    fclose(table_file);
    return (R_FAILURE);
  }
  table->nelevations = n;
  
  /*
   * alloc for elevs
   */

  RfAllocScanTableElevs(table);

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
      return (R_FAILURE);
    }
    
    if (sscanf(line, "elevation %lg", &table->elev_angles[ielev]) != 1) {
      fprintf(stderr, "ERROR: %s\n", calling_sequence);
      fprintf(stderr, "Reading file %s, line %d\n", file_path, line_num);
      fprintf(stderr, "Token 'elevation'\n");
      fclose(table_file);
      return (R_FAILURE);
    }

    /*
     * read in naz
     */
    
    if (get_next_line(table_file, line, &line_num)) {
      fprintf(stderr, "ERROR: %s\n", calling_sequence);
      fprintf(stderr, "Reading file %s, line %d\n", file_path, line_num);
      fclose(table_file);
      return (R_FAILURE);
    }
    
    if (sscanf(line, "nazimuths %ld", &n) != 1) {
      fprintf(stderr, "ERROR: %s\n", calling_sequence);
      fprintf(stderr, "Reading file %s, line %d\n", file_path, line_num);
      fprintf(stderr, "Token 'nazimuths'\n");
      fclose(table_file);
      return (R_FAILURE);
    }
    elev->naz = n;

    table->max_azimuths = MAX(table->max_azimuths, elev->naz);
    table->nazimuths = table->max_azimuths;

    /*
     * alloc for azimuths
     */

    RfAllocScanTableAzArrays(table, ielev);
    
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
	return (R_FAILURE);
      }
      
      if (sscanf(line, "%lg", &az->angle) != 1) {
	fprintf(stderr, "ERROR: %s\n", calling_sequence);
	fprintf(stderr, "Reading file %s, line %d\n", file_path, line_num);
	fprintf(stderr, "Reading azimuth line\n");
	fclose(table_file);
	return (R_FAILURE);
      }
      
      min_az = MIN(min_az, az->angle);
      
    } /* iaz */

  } /* ielev */

  /*
   * set number of indices in table
   */
  
  table->nbeams_vol = beam_num;

  fclose(table_file);

  return (R_SUCCESS);

}

#undef THIS_ROUTINE

/*
 * Compute limits of elevation influence.
 *
 * The lower limits are computed. One extra limit is included, the
 * lower limit of the angle above the max, which is effectively the
 * upper limit of the max angle.
 */

void RfComputeScanTableElevLimits(scan_table_t *table)
     
{

  si32 ielev;
  si32 nelevations = table->nelevations;
  double bottom_limit;
  double top_limit;

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

void RfComputeScanTableExtElev(scan_table_t *table)
     
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

void RfComputeScanTableAzLimits(scan_table_t *table)
     
{

  si32 ielev, iaz;
  si32 naz;
  double bottom_limit;
  double top_limit;
  scan_table_az_t *az;
  scan_table_elev_t *elev;

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

void RfLoadScanTableExtElevs(scan_table_t *table)
     
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

si32 RfScanTableAng2AzNum(scan_table_elev_t *elev,
			  double az_ang)
     
{
  
  return (get_ang_index(az_ang,
			elev->naz,
			elev->rel_az_limits,
			elev->az_reference));

}

/*
 * get elev index in table given elevation angle
 */

si32 RfScanTableAng2ElNum(scan_table_t *table,
			  double elev_ang)
     
{
  
  return (get_ang_index(elev_ang,
			table->nelevations,
			table->elev_limits, 0.0));

}

/*
 * get beam_num in table given elevation and azimuth angles
 */

si32 RfScanTableAngs2BeamNum(scan_table_t *table,
			     double elev_ang, double az_ang)
     
{
  
  si32 jelev, jaz;
  scan_table_elev_t *elev;
  
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

/*
 * get the angle index, given a set of angle limits and
 * the minimim angle
 */

static si32 get_ang_index(double search_ang,
			  si32 n_ang,
			  double *ang_limits,
			  double bottom_limit)
     
{
  
  si32 iang;
  double rel_ang;

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

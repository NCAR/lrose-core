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
#ifndef scan_table_h
#define scan_table_h

#ifdef __cplusplus
 extern "C" {
#endif


/*
 * scan_table.h
 *
 * Header file for scan table structs and routines
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000, USA
 *
 * Sept 1995
 */

#include <dataport/port_types.h>

typedef struct {

  double angle;
  si32 beam_num;

} scan_table_az_t;

typedef struct {

  si32 naz;
  si32 start_beam_num;
  si32 end_beam_num;
  double az_reference;
  double *rel_az_limits;
  scan_table_az_t *azs;

} scan_table_elev_t;

typedef struct {

  si32 use_azimuth_table;  /* use table to azimuths, as opposed to
			    * evenly distributed azimuths */

  si32 extend_below;       /* if TRUE, data from lowest elevation
                            * angle will be propagated into the grid below
                            * the scan */

  si32 nelevations;
  si32 nazimuths;
  si32 max_azimuths;
  si32 ngates;
  si32 nbeams_vol;
  si32 missing_data_index;

  double delta_azimuth;
  double start_azimuth;
  double beam_width;
  double gate_spacing;
  double start_range;

  double *elev_angles;       /* the elevation angles for the scan */

  double *ext_elev_angles;   /* extended array of elevation angles,
			      * including an extra one at the start and
			      * end for computational purposes */

  double *elev_limits;       /* the lower limit of each angle in the
			      * elevation array, plus one extra which
			      * is the upper limit of the stop angle */

  scan_table_elev_t *elevs;
  scan_table_elev_t *ext_elevs;

} scan_table_t;

extern void RfAllocScanTableElevs(scan_table_t *table);

extern void RfAllocScanTableAzArrays(scan_table_t *table, int ielev);

extern void RfFreeScanTableArrays(scan_table_t *table);

extern void RfComputeScanTableAzLimits(scan_table_t *table);

extern void RfComputeScanTableElevLimits(scan_table_t *table);

extern void RfComputeScanTableExtElev(scan_table_t *table);

extern void RfLoadScanTableExtElevs(scan_table_t *table);

extern void RfPrintScanTable(FILE *out, const char *spacer,
			     scan_table_t *table);

extern int RfReadScanTable(scan_table_t *table,
			   const char *file_path,
			   const char *calling_routine);

extern si32 RfScanTableAng2AzNum(scan_table_elev_t *elev,
				 double az_ang);
  
extern si32 RfScanTableAng2ElNum(scan_table_t *table,
				 double elev_ang);

extern si32 RfScanTableAngs2BeamNum(scan_table_t *table,
				    double elev_ang, double az_ang);

#ifdef __cplusplus
}
#endif

#endif /* scan_table_h */


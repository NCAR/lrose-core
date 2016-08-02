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
#ifndef radar_scan_table_h
#define radar_scan_table_h

#ifdef __cplusplus
 extern "C" {
#endif


/*
 * radar_scan_table.h
 *
 * Header file for scan table structs and routines
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000, USA
 *
 * Sept 1995
 */

#include <stdio.h>
#include <dataport/port_types.h>

typedef struct {

  fl32 angle;
  si32 beam_num;

} radar_scan_table_az_t;

typedef struct {

  si32 naz;
  si32 start_beam_num;
  si32 end_beam_num;
  fl32 az_reference;
  fl32 *rel_az_limits;
  radar_scan_table_az_t *azs;

} radar_scan_table_elev_t;

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

  fl32 delta_azimuth;
  fl32 start_azimuth;
  fl32 beam_width;
  fl32 gate_spacing;
  fl32 start_range;

  fl32 *elev_angles;       /* the elevation angles for the scan */

  fl32 *ext_elev_angles;   /* extended array of elevation angles,
			    * including an extra one at the start and
			    * end for computational purposes */

  fl32 *elev_limits;       /* the lower limit of each angle in the
			    * elevation array, plus one extra which
			    * is the upper limit of the stop angle */

  radar_scan_table_elev_t *elevs;
  radar_scan_table_elev_t *ext_elevs;

} radar_scan_table_t;

extern void RadarAllocScanTableElevs(radar_scan_table_t *table,
				     int elevations);

extern void RadarAllocScanTableAzArrays(radar_scan_table_t *table,
					int ielev, int azimuths);

extern void RadarFreeScanTableArrays(radar_scan_table_t *table);

extern void RadarComputeScanTableAzLimits(radar_scan_table_t *table);

extern void RadarComputeScanTableElevLimits(radar_scan_table_t *table);

extern void RadarComputeScanTableExtElev(radar_scan_table_t *table);

extern void RadarLoadScanTableExtElevs(radar_scan_table_t *table);

extern void RadarInitScanTable(radar_scan_table_t *table);

extern void RadarPrintRadarElevations(FILE *out,
				      const char *spacer,
				      const char *label,
				      si32 nelevations,
				      fl32 *radar_elevations);

extern void RadarPrintScanTable(FILE *out, const char *spacer,
				radar_scan_table_t *table);

extern int RadarReadScanTable(radar_scan_table_t *table,
			      const char *file_path,
			      const char *calling_routine);

extern si32 RadarScanTableAng2AzNum(radar_scan_table_elev_t *elev,
				    fl32 az_ang);

extern si32 RadarScanTableAng2ElNum(radar_scan_table_t *table,
				    fl32 elev_ang);

extern si32 RadarScanTableAngs2BeamNum(radar_scan_table_t *table,
				       fl32 elev_ang, fl32 az_ang);

extern void RadarPrintScanTable(FILE *out, const char *spacer,
				radar_scan_table_t *table);
     
#ifdef __cplusplus
}
#endif

#endif /* radar_scan_table_h */


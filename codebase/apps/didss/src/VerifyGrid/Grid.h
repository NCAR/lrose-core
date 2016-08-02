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
/////////////////////////////////////////////////////////////
// Grid.h
//
// Grid object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1998
//
///////////////////////////////////////////////////////////////

#ifndef Grid_H
#define Grid_H

#include "VerifyGrid.h"
#include <dataport/port_types.h>
#include <Mdv/mdv/mdv_handle.h>

class Grid {
  
public:

  // constructor

  Grid(const char *prog_name, Params *params,
       const char *label, const char *grid_file_path,
       int field_num, double plane_ht);

  // destructor
  
  virtual ~Grid();

  // grid and range data

  ui08 *byteArray;
  float *dataArray;
  float *rangeArray;

  float Scale,Bias,Missing,Bad; // For intermediate field writing.

  int OK;

  int npoints;
  int nx, ny;
  double minx, miny;
  double dx, dy;
  double originLat, originLon;

  time_t timeStart;
  time_t timeCent;
  time_t timeEnd;

  int checkGeom(Grid *other);

  void printGeom(FILE *out, const char *spacer);

protected:
  
private:

  char *_progName;
  char *_label;
  Params *_params;

  char *_gridFilePath;

  void _freeGrids();

  void _load(int field_num, int plane_num,
	     MDV_handle_t *mdv);

  void _loadNative(ui08 *plane,
		   MDV_field_header_t *fhdr,
		   double radarX, double radarY);
  
  void _loadNearest(ui08 *plane,
		    MDV_field_header_t *fhdr,
		    double radarX, double radarY);
  
  void _loadMeans(ui08 *plane,
		  MDV_field_header_t *fhdr,
		  double radarX, double radarY);
  

};

#endif













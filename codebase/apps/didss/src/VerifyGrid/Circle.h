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
// Circle.h
//
// Contingency analysis for circular region.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
///////////////////////////////////////////////////////////////

#ifndef Circle_H
#define Circle_H

#include "Cont.h"

class Circle {
  
public:

  friend class RegionCont;
  
  // constructor
  
  Circle(char *prog_name, Params *params,
	 char *region_name, double lat, double lon,
	 double radius, double percent_covered_target);

  // destructor
  
  virtual ~Circle();
  
  // update region analysis
  
  int update(Grid *forecast_grid, Grid *truth_grid,
	     contingency_t *global_cont);

  // print header and contingency table

  void printHeader(FILE *out);
  void printCont(FILE *out);

protected:
  
private:

  char *_progName;
  Params *_params;

  char *_regionName;
  double _lat;
  double _lon;
  double _radius;
  double _percentCoveredTarget;
  double _percentCoveredForecast;
  double _percentCoveredTruth;

  // circle analysis contingency table

  contingency_t _cont;

  int _isCovered(const char *label, Grid *grid,
		 double low_thresh, double high_thresh,
		 int is_forecast);
  
};

#endif


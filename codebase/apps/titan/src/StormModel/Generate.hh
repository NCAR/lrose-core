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
// Generate.h
//
// Storm generation object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1998
//
///////////////////////////////////////////////////////////////

#ifndef Generate_HH
#define Generate_HH

#include "Params.hh"
#include "MidPoint.hh"
#include "Duration.hh"
#include "Area.hh"
#include "StartTime.hh"
#include "Velocity.hh"

class Generate {
  
public:

  // constructor

  Generate(const char *prog_name, const Params &params);

  // destructor
  
  ~Generate();

  // Print the header

  void printHeader(FILE *out);
  
  // generate another storm, write to output file

  int Another(int num, FILE *out);

  int OK;

  double sumArea() { return (_sumArea); }

protected:
  
private:

  char *_progName;
  const Params &_params;

  StartTime *_startTime;
  MidPoint *_midPoint;
  Duration *_duration;
  Area *_area;
  Velocity *_velocity;

  double _sumArea;

  void _interp_dBZmax(double Dm, double *mean_p, double *sdev_p);

  static double _durDiff(double dur_guess);
 
  double rtbis(double (*func)(double),
	       double x1, double x2, double xacc);

};

#endif


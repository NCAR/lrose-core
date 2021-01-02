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
// DistFit.h
//
// DistFit object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 1998
//
///////////////////////////////////////////////////////////////

#ifndef DistFit_H
#define DistFit_H

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <tdrp/tdrp.h>

#include "Args.h"
#include "Params.h"

#define LABEL_MAX 64

class DistFit {
  
public:

  // constructor

  DistFit (int argc, char **argv);

  // destructor
  
  ~DistFit();

  // run 

  int Run();

  // data members

  int OK;
  int Done;
  char *_progName;
  Args *_args;
  Params *_params;

protected:
  
private:

  char _xLabel[LABEL_MAX];
  char _yLabel[LABEL_MAX];
  char _condLabel[LABEL_MAX];

  int _xPos;
  int _yPos;
  int _condPos;
  
  int _nData;

  double *_xData;
  double *_yData;

  double _xMin, _xMax;
  double _yMin, _yMax;
  
  int _loadLabels();

  int _loadXYData();

};

#endif

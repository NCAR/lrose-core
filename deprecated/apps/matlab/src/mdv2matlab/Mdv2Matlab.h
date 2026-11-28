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
// Mdv2Matlab.h
//
// Mdv2Matlab object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1998
//
///////////////////////////////////////////////////////////////

#ifndef Mdv2Matlab_H
#define Mdv2Matlab_H

#include <stdio.h>

#include <tdrp/tdrp.h>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>

#include <matrix/matrix.h>
#include "Mdv2MatlabArgs.h"
#include "Mdv2MatlabParams.h"


class Mdv2Matlab
{
  
public:

  // constructor

  Mdv2Matlab (int argc, char **argv);

  // destructor
  
  ~Mdv2Matlab();

  // run 

  int run();

  // data members

  int okay;
  int done;

protected:
  
private:

  char              *_programName;
  Mdv2MatlabArgs    *_args;
  Mdv2MatlabParams  *_params;

  void _processFile(char *file_name);
  
  static char *_getMatlabFilename(char    *output_dir,
				  char    *output_ext,
                                  bool    makeOutputSubdir,
				  time_t  data_time);

  static char *_className()
  {
    return("Mdv2Matlab");
  }
  
};

#endif

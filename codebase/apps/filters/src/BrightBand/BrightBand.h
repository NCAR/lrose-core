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
// BrightBand.h: Main program object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1997
//
/////////////////////////////////////////////////////////////

#ifndef BrightBand_H
#define BrightBand_H

#include <sys/stat.h>
#include <vector>

#include <dsdata/DsTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <tdrp/tdrp.h>
#include <toolsa/pmu.h>
#include <toolsa/utim.h>
#include <toolsa/umisc.h>

#include "Args.h"
#include "Params.hh"

#include "Refl_Interest.h"
#include "Incr_Refl_Interest.h"
#include "Template_Interest.h"
#include "Max_Interest.h"
#include "Texture_Interest.h"
#include "Clump.h"

using namespace std;

class BrightBand {
  
public:

  // constructor

  BrightBand (int argc, char **argv);

  // destructor
  
  virtual ~BrightBand();

  // init

  bool init();

  // run 

  bool Run();

  // data members

  int OK;
  int Done;
  char *name;
  Args *args;
  Params *params;
  DsTrigger *trigger;
  bool time_trigger;
  
protected:
  
private:

  // encoding and compression

  Mdvx::encoding_type_t _inputEncoding;
  Mdvx::encoding_type_t _outputEncoding;

  Mdvx::compression_type_t _inputCompression;
  Mdvx::compression_type_t _outputCompression;

  // Interest classes

  Refl_Interest *_reflInterest;
  Incr_Refl_Interest *_incrReflInterest;
  vector< Template_Interest* > _templateInterest;
  Max_Interest *_maxInterest;
  Texture_Interest *_textureInterest;
  Clump *_clumps;

  bool _checkDbzField(MdvxField &dbz_field);
  bool _checkInputFile(DsMdvx &input_file);
  bool _checkParams();
  string _createUrl(const string &input_source);
  bool _initInterestFields();
  bool _initTrigger();
  bool _processFile(DsMdvx &input_file);
  bool _readFile(DsMdvx &input_file, const DateTime &data_time);
  bool _readFile(DsMdvx &input_file, const string &input_path);
  
};

#endif

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// RefractCompute
//
// Nancy Rehak, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2026
//
/////////////////////////////////////////////////////////////
//
// RefractCompute computes the refractivity value at each radar
// gate, given the AIQ/NIQ data, plus the calibration results
// from previously running RefractCalib.
//
//////////////////////////////////////////////////////////////

#ifndef RefractCompute_HH
#define RefractCompute_HH

#include <string>
#include <didss/DsInputPath.hh>
#include <toolsa/DateTime.hh>

#include "Args.hh"
#include "Params.hh"
#include "Reader.hh"
#include "CalibDayNight.hh"
#include "Processor.hh"

class Input;
class DsMdvx;

/** 
 * @class RefractCompute
 */

class RefractCompute
{
 public:

  /**
   * @brief Flag indicating whether constructor completes OK
   */
  
  bool okay;
  
  // Constructor/Destructor

  RefractCompute(int argc, char **argv);
  virtual ~RefractCompute();
  
  /**
   * @brief Run the program.
   */

  int run();
  
 private:

  string _progName; /**< program name */
  Args _args;
  char *_paramsPath;
  Params _params; /**< Parameter file paramters. */
  time_t _startTime, _endTime;

  // data trigger

  DsInputPath *_input;
  
  // data reader

  Reader *_reader;
  
  // Refractivity data processor object

  Processor *_processor;

  /**
   * @brief Process data for the given trigger time.
   *
   * @param trigger_time The current trigger time.  Data for this time
   *                     should be processed.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(string inputPath,
                    const DateTime &trigger_time);

  bool _readInputFile(DsMdvx &mdvx, const DateTime &data_time);

};


#endif

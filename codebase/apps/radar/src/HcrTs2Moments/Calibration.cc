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
///////////////////////////////////////////////////////////
// Calibration.cc
// 
// read and store calibration data
//
// EOL, NCAR, Boulder CO
//
// August 2007
//
// Mike Dixon
/////////////////////////////////////////////////////////////

#include <cerrno>
#include <cmath>
#include <cassert>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/ReadDir.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/TaArray.hh>
#include <radar/IwrfTsInfo.hh>
#include <Spdb/DsSpdb.hh>
#include "Calibration.hh"

//////////////////
// Constructor

Calibration::Calibration(const Params &params,
                         double pulse_width_us) :
        _params(params),
        _pulseWidthUs(pulse_width_us)
{

  // initialize
  
  _calTime = 0;

}

//////////////////
// Destructor


Calibration::~Calibration()
{
}

//////////////////////////////////////////////////////////
// Read calibration for a given file path.
// Returns 0 on success, -1 on failure

int Calibration::readCal(const string &calPath)

{

  string errStr;
  if (_calib.readFromXmlFile(calPath, errStr)) {
    cerr << "ERROR - Calibration::_readCalFromFile" << endl;
    cerr << "  Cannot decode cal file: " << calPath << endl;
    cerr << errStr;
    return -1;
  }
  
  _calFilePath = calPath;
  
  // apply corrections as appropriate
  
  _applyCorrections();

  if (_params.debug) {
    cerr << "Done reading calibration file: " << calPath << endl;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// apply corrections

void Calibration::_applyCorrections()
  
{

  
  if (_params.override_cal_dbz_correction) {
    _calib.setDbzCorrection(_params.dbz_correction);
    if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
      cerr << "Calibration::_applyCorrections()" << endl;
      cerr << "  setting dbz_correction: " << _params.dbz_correction << endl;
    }
  }
  
  if (_params.override_cal_ldr_corrections) {
    _calib.setLdrCorrectionDbH(_params.ldr_correction_db_h);
    _calib.setLdrCorrectionDbV(_params.ldr_correction_db_v);
    if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
      cerr << "Calibration::_applyCorrections()" << endl;
      cerr << "  setting ldr_correction_db_h: " << _params.ldr_correction_db_h << endl;
      cerr << "  setting ldr_correction_db_v: " << _params.ldr_correction_db_v << endl;
    }
  }
  
}


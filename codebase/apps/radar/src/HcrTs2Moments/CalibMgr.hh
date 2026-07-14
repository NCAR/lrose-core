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
// CalibMgr.hh
// 
// Manages calibration data for different pulse widths
//
// EOL, NCAR, Boulder CO
//
// July 2026
//
// Mike Dixon
//
///////////////////////////////////////////////////////////

#ifndef CalibMgr_HH
#define CalibMgr_HH

#include "Params.hh"
#include "Calibration.hh"

#include <radar/IwrfCalib.hh>
#include <map>

typedef multimap<time_t, string, less<time_t> > FileMap;
typedef pair<const time_t, string > FilePair;

using namespace std;

/////////////////////////////////////////////////////
// clibration for specific pulse width

class CalibMgr
{

public:
  
  // Constructor

  CalibMgr(const Params &params);

  // Destructor
  
  ~CalibMgr();
  
  // Read calibrations for pulse widths specified in params file
  // Returns 0 on success, -1 on failure
  
  int readCals(time_t dataStartTime);
  
  // Load calibration appropriate to a given pulse width
  
  const IwrfCalib &getIwrfCalib(double pulseWidthUs) const;

private:

  // params
  
  const Params &_params;

  // list of available cals
  
  vector<Calibration *> _cals;
  
  // functions
  
  int _getBestFilePath(const string &calDir,
                       time_t dataTime,
                       string &calPath);
    
  int _compileFileList(const string &dirPath,
                       FileMap &fileMap);
  

};


#endif

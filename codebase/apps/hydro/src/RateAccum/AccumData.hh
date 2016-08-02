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
//////////////////////////////////////////////////////////
// AccumData.hh
//
// Accumulation data object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2015
//
//////////////////////////////////////////////////////////
//
// This module acummulates the precip data in fl32 arrays.
//
///////////////////////////////////////////////////////////

#ifndef AccumData_HH
#define AccumData_HH

#include "Params.hh"
#include <Mdv/DsMdvx.hh>
#include <string>
using namespace std;

class AccumData {
  
public:
  
  AccumData(const string &prog_name, const Params &params);
  ~AccumData();
  
  // init for computations
  
  void init();
  void setTargetPeriod(double period);

  // process data from a file

  int processFile(const string &file_path,
                  time_t file_time,
                  double file_duration);
  
  // compute and write
  // returns 0 on success, -1 on failure
  
  int write(const string &output_url);

  // free up memory

  void free();

protected:
  
private:
  
  const string &_progName;
  const Params &_params;
  vector<string> _rateFieldNames;
  vector<string> _accumFieldNames;
  vector<bool> _inputIsDepth;

  bool _dataFound;   // flag to indicate the some data has been found
  Mdvx::coord_t _grid; // coord grid from first file read

  int _nxy;
  vector<fl32 *> _accumFieldData;

  time_t _dataStartTime;
  time_t _dataEndTime;

  double _targetAccumPeriod;
  double _actualAccumPeriod;

  void _updateAccum(const string &fieldName,
                    bool inputIsDepth,
                    const MdvxField *fld,
                    fl32 *accumFieldData,
                    double fileDuration);

  void _setFieldName(const Mdvx::master_header_t &mhdr,
                     Mdvx::field_header_t &fhdr,
                     const string &rateName,
                     const string &accumName);

  void _adjustDepth(const fl32 *accum,
                    fl32 *adjusted);
  
};

#endif

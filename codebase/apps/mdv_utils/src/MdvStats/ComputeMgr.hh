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
// ComputeMgr.hh
//
// Manages the computations for statistics
//
// Yan Chen, RAL, NCAR
//
// Dec. 2007
//
/////////////////////////////////////////////////////////////

#ifndef COMPUTEMGR_HH
#define COMPUTEMGR_HH

#include <string>
#include <vector>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include "Params.hh"
using namespace std;

class ComputeMgr {
  
public:

  ComputeMgr(const Params &params);
  ~ComputeMgr();

  int computeStatistics();
  int computeStatistics(vector<time_t>& timelist);
  int processOneInput(DsMdvx& in);
  int writeOutput();

protected:

private:

  const Params &_params;

  time_t _startTime, _endTime;

  DsMdvx stats_StdDev;
  DsMdvx stats_Mean;
  DsMdvx stats_Min;
  DsMdvx stats_Max;
  DsMdvx stats_Sum;
  DsMdvx stats_Cov;

  bool _need_setup;

  set<string> _fieldNameSet;

  void _addChunk(
    const Mdvx::master_header_t&,
    const string& info,
    DsMdvx& out
  );

  int _setupOutput(const DsMdvx& in);
  void _updateOutput(const DsMdvx& in);

  MdvxField *_createField(
    const MdvxField&,
    const Mdvx::encoding_type_t,
    const Mdvx::transform_type_t
  ) const;

  string _getNumFieldName(const char* data_field_name, bool short_name);
  string _getCovFieldName(
    const char* data_field1_name,
    const char* data_field2_name,
    bool short_name
  );

  bool _fieldsMatch(
    MdvxField *field1,
    MdvxField *field2,
    string &err_str
  );

  int _write(DsMdvx&, const string& stats);

};

#endif


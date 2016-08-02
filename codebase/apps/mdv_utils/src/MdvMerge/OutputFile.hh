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
// OutputFile.hh
//
// OutputFile class - handles the output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1998
//
///////////////////////////////////////////////////////////////

#ifndef OutputFile_HH
#define OutputFile_HH

#include <ctime>
#include <string>
#include <vector>

#include <Mdv/Mdvx.hh>
#include <Fmq/NowcastQueue.hh> 

using namespace std;

class MdvxProj;
class DsMdvx;
class Params;

class OutputFile {
  
public:
  
  // constructor
  
  OutputFile(string prog_name,
	     Params *params,
	     MdvxProj *grid,
	     NowcastQueue &nowcastQueue);
  
  // destructor
  
  virtual ~OutputFile();
  
  // initialize the headers
  void initHeaders(DsMdvx *input_mdv);

  // add string to data set info
  void addToInfo(const char *info_str);

  // clear data from volume ready for new merge
  void clearVol();

  // load scale and bias for a given field
  void loadScaleAndBias(const int& i, const double& scale, const double& bias);

  // write out merged volume
  bool writeVol(const time_t& merge_time, const time_t& start_time, 
	       const time_t& end_time);

  // get handle
  DsMdvx *handle() { return (_handle); }

  // get at a field name
  string getFieldName(const int i) { return _fieldNames[i]; }

protected:
  
private:
  
  string _progName;
  Params *_params;

  DsMdvx *_handle;
  MdvxProj *_grid;

  time_t _mergeTime;
  time_t _startTime;
  time_t _endTime;

  Mdvx::master_header_t _outMasterHdr;

  vector<double> _scale;
  vector<double> _bias;
  vector<string> _fieldNames;

  //
  // A nowcast FMQ to send trigger messages to, if desired.
  //
  NowcastQueue _nowcastQueue;

};

#endif

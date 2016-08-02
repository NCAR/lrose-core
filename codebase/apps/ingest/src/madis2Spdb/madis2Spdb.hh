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
// madis2Spdb.hh
//
// madis2Spdb object
//
//
///////////////////////////////////////////////////////////////


#ifndef madis2Spdb_H
#define madis2Spdb_H

#include <netcdf.h>

#include <rapformats/station_reports.h>

#include "Params.hh"

using namespace std;

class madis2Spdb {
  
public:
  
  // constructor.
  madis2Spdb (Params *TDRP_params);

  // 
  bool processFile(char *fileName);
  
  // destructor.
  ~madis2Spdb();

  
protected:
  
private:

  /////////////////////
  // Private members //
  /////////////////////

  Params *_params;
  
  int *_QCR;
  
  int _arrayAlloc;
  
  /////////////////////
  // Private methods //
  /////////////////////

  void _allocateArrays(const int num_records);
  bool _checkStatus(const int status, const string &err, const bool printMsg = TRUE);
  int _Hash(char *label);

  // Read variables from the netCDF file

  float *_readFloatVar(const int nc_id,
		       const unsigned int num_records,
		       const string &nc_var_name,
		       const string &nc_fill_value_name,
		       const string &nc_missing_value_name);

  float *_readFloatVar(const int nc_id,
		       const unsigned int num_records,
		       const string &nc_var_name,
		       const string &nc_qcr_var_name,
		       const string &nc_fill_value_name,
		       const string &nc_missing_value_name);
  
  double *_readDoubleVar(const int nc_id,
			 const unsigned int num_records,
			 const string &nc_var_name,
			 const string &nc_fill_value_name,
			 const string &nc_missing_value_name);

  // Read a text string from the netCDF file

  bool _readString(const int nc_id,
		   const int string_index,
		   const unsigned int str_len,
		   const string &nc_var_name,
		   char *nc_string,
		   const bool printMsg = TRUE);
  
};

#endif

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
// OutputFile class - handles the output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2004
//
///////////////////////////////////////////////////////////////

#ifndef OutputFile_HH
#define OutputFile_HH

#include <ctime>
#include <string>
#include <vector>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include "Params.hh"

using namespace std;

class OutputFile {
  
public:
  
  // constructor
  
  OutputFile(const string &prog_name,
	     const Params &params,
	     const MdvxProj &proj,
	     const Mdvx::master_header_t &exampleMhdr,
	     const vector<Mdvx::field_header_t> &exampleFhdrs);
  
  // destructor
  
  virtual ~OutputFile();
  
  // write output file
  // returns 0 on success, -1 on failure

  int write(const time_t& mergeTime,
	    const time_t& startTime, 
	    const time_t& endTime,
	    const vector<void *> fieldData,
	    const string &dataSetInfo);

  // add string to data set info

  void addToInfo(const char *info_str);

  // missing values
  
  static const fl32 missingFl32;
  static const int missingInt = 0;

protected:
  
private:

  const string &_progName;
  const Params &_params;
  const MdvxProj &_proj;
  Mdvx::master_header_t _exampleMhdr;
  vector<Mdvx::field_header_t> _exampleFhdrs;

};

#endif

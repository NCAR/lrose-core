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
// MdvStats.hh
//
// MdvStats object
//
// Yan Chen, RAL, NCAR
//
// Dec. 2007
//
///////////////////////////////////////////////////////////////
//
// MdvStats computes the statistics values from a series of MDV files.
//
///////////////////////////////////////////////////////////////

#ifndef MdvStats_HH
#define MdvStats_HH

#include <string>
#include <vector>
#include <toolsa/DateTime.hh>
#include "Args.hh"
#include "Params.hh"

using namespace std;

////////////////////////
// This class

class MdvStats {
  
public:

  // constructor

  MdvStats (int argc, char **argv);

  // destructor
  
  ~MdvStats();

  // run 

  int Run();

  // data members

  bool isOK;

protected:

  date_time_t startTime;
  date_time_t endTime;
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  vector< vector<time_t> > vvTimeList;

  int _processArchive(vector<time_t>&);
  int _processFileList();
  int _generateTimeList();
  
};

#endif


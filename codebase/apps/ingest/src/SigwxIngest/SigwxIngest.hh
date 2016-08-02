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


#ifndef SIGWXINGEST_HH
#define SIGWXINGEST_HH

#include <tdrp/tdrp.h>
#include <string>
#include <map>
#include <vector>
#include <cstdlib>
#include <sys/time.h>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <didss/LdataInfo.hh>
#include "Args.hh"
#include "Params.hh"

using namespace std;

class Args;
class Params;

class SigwxIngest {
  
public:

  // constructor

  SigwxIngest (int argc, char **argv);

  // destructor
  
  ~SigwxIngest();

  // run 

  int Run();

  // data members

  int isOK;

 

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  vector < pair <long,long> > _sigwxReps;

  //
  // File handling
  //
  DsInputPath       *fileTrigger;

  int processFile(char *inputFile);

  int _findSigwxReports(FILE *fptr);

};


#endif



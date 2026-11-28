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
/************************************************************************
 * Pirep2Spdb.hh : header file for the Pirep2Spdb program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2005
 *
 * Mike Dixon
 *
 ************************************************************************/

#ifndef Pirep2Spdb_HH
#define Pirep2Spdb_HH

// C++ include files
#include <vector>

// System/RAP include files
#include <toolsa/udatetime.h>
#include <Spdb/DsSpdb.hh>

// Local include files
#include "Args.hh"
#include "Params.hh"

using namespace std;

// Forward class declarations
class DsInputPath;
class PirepDecoder;

class Pirep2Spdb
{

 public:

  // constructor
  
  Pirep2Spdb(int argc, char **argv);
  
  // Destructor

  ~Pirep2Spdb(void);
  
  // Run the program.

  int Run();
  
  // Flag indicating whether the program status is currently okay.

  bool isOK;
  
 private:

  static const int SPDB_BUFFER_INCR = 10;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // Input objects

  DsInputPath *_inputPath;


  // Decoder

  PirepDecoder *_decoder;

  bool _inAirep;
  bool _decodedAirep;
  date_time_t _airepTime;
  date_time_t _inputFileTime;

  // functions

  int _writeSpdb(DsSpdb &spdb, string url);
  
  int _read(FILE *input_file);
  char *_fgets(char *buffer, int len, FILE *input_file);
  void _fgets_save(char *buffer);

};



#endif



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
// VerifyTracks.h
//
// VerifyTracks object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2005
//
///////////////////////////////////////////////////////////////
//
// VerifyTracks computes verification stats on TITAN forecasts,
// updates TITAN files accordingly and writes results to stdout
//
////////////////////////////////////////////////////////////////

#ifndef VerifyTracks_H
#define VerifyTracks_H

#include <string>
#include <vector>
#include "Args.hh"
#include "Params.hh"
#include "vt_structs.hh"
#include <didss/DsInputPath.hh>
using namespace std;

#define BOOL_STR(a) (a == 0? "false" : "true")

class VerifyTracks {
  
public:

  // constructor

  VerifyTracks (int argc, char **argv);

  // destructor
  
  ~VerifyTracks();

  // run 

  int Run();

  // print routines
  
  static void print_contingency_table(FILE *fout,
                                      const char *label,
                                      const vt_count_t *count);
  
  static void print_stats(FILE *fout,
                          const char *heading,
                          const vt_stats_t *stats);
  
  static void print_stat(FILE *fout,
                         const char *label,
                         const fl32 *bias_p,
                         int print_norm,
                         const vt_stats_t *stats);
  
  static void print_grid(FILE *fout,
                         const char *label,
                         ui08 **grid,
                         int nx, int ny);
  
  static void print_grid(FILE *fout,
                         const char *label,
                         ui08 **forecast_grid,
                         ui08 **verify_grid,
                         int nx, int ny);

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsInputPath *_input;
  
};

#endif

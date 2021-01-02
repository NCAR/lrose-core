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

Header: Process.hh

Author: Dave Albo

Date:   Thu Jan 26 16:43:33 2006

Description: Processing of MDV. Built from Niles Oien Mdv filtering example.

*************************************************************************/

# ifndef    Process_HH
# define    Process_HH

/* System include files / Local include files */
#include <vector>
#include <string>
#include <map>
#include <toolsa/pmu.h>
#include <Mdv/DsMdvxTimes.hh>
#include "TempDataState.hh"
#include "Data.hh"
#include "Params.hh"
using namespace std;

/* Class definitions */
class Process {

public:

  //
  // Constructor
  //
  Process();


  //
  // Main method - run.
  //
  int Derive(Params *P, time_t t, TempDataState &T);

  //
  // Destructor
  //
  ~Process();

private :

  DsMdvx Out;
  char *OutputUrl;
  Data *D;

  Mdvx::field_header_t InFhdr;
  Mdvx::vlevel_header_t InVhdr;

  bool _filter(string &name, Params::filter_parm_t &f, Params *P,
	       TempDataState &T);
  void _filter_combine(Params::mdv_combine_parm_t &p, TempDataState &T);
  bool _initialize(DsMdvx &New, Params *P, time_t t, TempDataState &T);
  void _initialize_out(DsMdvx &New, Params *P);
  bool _extract_field(DsMdvx &New, Params *P);
  int _filter_init(string &name, Params::filter_parm_t &f, Params *P,
		   TempDataState &T);
  bool _filter_finish(Params::filter_parm_t &f, Params *P);
  void _evaluate(bool debug);
  bool _set_fld(Params::filter_parm_t &f, Params *P);
};


# endif     /* Process_HH */

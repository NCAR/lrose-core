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

#ifndef _STATISTICIAN_INC_
#define _STATISTICIAN_INC_

#include <cstdio>

#include "Params.hh"
using namespace std;


class Statistician
{

public:

  Statistician(const string &Name);

  void Init(time_t Start, Params *P);

  void Accumulate(unsigned long num_non, 
		  unsigned long num_fail, 
		  unsigned long num_false, 
		  unsigned long num_success,
		  time_t TruthTime,
		  Params *P);

  ~Statistician();

private:

  double total_num_non, total_num_fail, 
    total_num_false, total_num_success;

  double subtotal_num_non, subtotal_num_fail, 
    subtotal_num_false, subtotal_num_success;

  unsigned long Num,SubNum;

  FILE *sfp, *fsfp;

  time_t LastOutputTime,StartTime, _OutputInterval;
  string _Name;

  double NoNaN(double val);


};

#endif
































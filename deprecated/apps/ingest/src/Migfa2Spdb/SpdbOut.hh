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


#ifndef SPDB_OUT_H
#define SPDB_OUT_H


#include "Params.hh"

#include <rapformats/bdry.h>
using namespace std;

class SpdbOut {
 
public:
 
  // constructor. Does nothing.
 
  SpdbOut();
 
  // destructor. Does nothing.
 
  ~SpdbOut();    
    
  // public method.
 
  void WriteOut(Params *P,
		int num,
		double *lat,
		double *lon,
		time_t dataTime,
		int leadMinutes,
		int id,
		int seqNum,
		double U,
		double V);



protected:
 
private:
 
  // global variables:
 
  BDRY_spdb_product_t *__spdb_buffer; // the intermediate storage for data
  int __spdb_buffer_alloc;            // the ammount of intermediate storage.
  int __spdb_data_type;               // data type for intermediate store.
  int __spdb_id;                      // id value for intermediate store.
  time_t __spdb_valid_time;           // time valid for inter. store.
  time_t __spdb_expire_time;          // time expire  for inter. store.
  char __spdb_product_type[100];      // type of inter. storage.
  char __spdb_label[100];             // label for inter. storage.

  bool _set_label_values(int extrap_time);
  void _set_buffer_size(int npt);
 
};  

#endif
   

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
////////////////////////////////////////////////////////////////////////////////
//
//  Abstract base class for EDR input stream
//
//  Sue Dettling RAP, NCAR, Boulder, CO, 80307, USA
//  November 2004
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _EDR_INPUT_INC_
#define _EDR_INPUT_INC_

#include "Params.hh"
#include "EdrReport.hh"
#include <toolsa/DateTime.hh>
using namespace std;


class EdrInput
{
public:
   EdrInput(){}
   virtual ~EdrInput(){};

   //
   // Optional initialization for the sub-classes
   // Return 0 upon success, -1 upon failure
   //
   // virtual int init( Params& params ){ return 0; }
  
  virtual int init( Params& params , const vector<string> &fileList, string &prog ) { return 0; }
   
  //
  // Sub-classes are required to provide a read method
  // which sets a pointer to the buffer.  
  //
  virtual EdrReport::status_t readFile( ui08* &buffer, DateTime &msgTime) = 0;
   
  static const int BUF_LEN = 1024 ;
};

#endif

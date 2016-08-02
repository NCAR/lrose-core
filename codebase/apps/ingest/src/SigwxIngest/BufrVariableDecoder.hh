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

#ifndef BUFR_VARIABLE_DECODER_HH
#define BUFR_VARIABLE_DECODER_HH

#include <string>
#include <cstdlib>
#include <sys/time.h>
#include <toolsa/DateTime.hh>
#include <mel_bufr/mel_bufr.h>

using namespace std;

class BufrVariableDecoder {
  
public:

   /**
   * Return value for BufrVariableDecoder  methods
   *  indicates successful method execution
   *  indicates unsuccessful method execution
   */
  enum Status_t {BUFR_DECODER_SUCCESS,
		 BUFR_DECODER_FAILURE,
		 BUFR_DECODER_EOM,
		 BUFR_DECODER_EOF};
  
  

  // constructor

  BufrVariableDecoder();

  // destructor
  
  ~BufrVariableDecoder();
 
  Status_t checkVar(char *fxy, BUFR_Val_t &bv);
  
  Status_t getVar(char *stringVar, char* fxy,  BUFR_Val_t &bv);
  
  Status_t getVar(int &intVar, char* fxy,  BUFR_Val_t &bv);
  
  Status_t getVar(float &floatVar, char* fxy,  BUFR_Val_t &bv);

  Status_t getValue(int &intVar, char* fxy,  BUFR_Val_t &bv);
  
  Status_t getValue(float &floatVar, char* fxy,  BUFR_Val_t &bv);
  
  Status_t getFxyString(char *fxy, BUFR_Val_t &bv);

  char* getFxyString(BUFR_Val_t &bv);

protected:
  
private:

  bool _debug;
 
};


#endif



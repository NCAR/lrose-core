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
///////////////////////////////////////////////////////////////////
//
// Variable data for multiple linear regression
//
// $Id: MultRegVar.hh,v 1.6 2016/03/07 01:33:50 dixon Exp $
//
///////////////////////////////////////////////////////////////////
#ifndef _MULT_VAR_INC_
#define _MULT_VAR_INC_

#include <vector>
using namespace std;

class MultRegVar {

 public:

   MultRegVar();
   ~MultRegVar(){ clear(); }

   void      clear();

   void      addVal( double inputVal );

   double    getVal( int idex );
   double    getVecLen();
   double    getStdDev();
   int       getSize(){ return( (int) values.size() ); }
   
   
   //
   // Constants
   //
   static const double MISSING_VAL;

 private:

   double           vecLen;
   double           stdDev;
   vector< double > values;

   void             findVecLen();
   void             findStdDev();
   
};

#endif

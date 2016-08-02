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
////////////////////////////////////////////////////////////
//
// MM5 data 
//
// $Id: MM5Data.hh,v 1.3 2016/03/07 01:33:51 dixon Exp $
//
//////////////////////////////////////////////////////////////
#ifndef _MM5_DATA_
#define _MM5_DATA_

#include <string>
#include <map>
#include <ctime>

#include "DataServer.hh"
using namespace std;

//
// Forward class declarations
//
class MM5Point;

class MM5Data 
{
 public:
   MM5Data( DataServer& server );
   ~MM5Data();

   void clear();

   int createList( char* stationId, int leadTime );

   map< time_t, MM5Point*, less< time_t > >& getData() { return dataPoints; }

   //
   // Constants
   //
   static const string RH;
   static const string TEMP;
   static const string PRS;
   static const string U;
   static const string V;
   static const string W;
   
   
 private:
 
   //
   // Server for MM5 data
   //
   DataServer&            dataServer;
   
   //
   // Data 
   //
   map< time_t, MM5Point*, less<time_t> > dataPoints;

   //
   // Functions to get data
   //
   double  get2DVal( const string& fieldName,const GenPt& multiLevelPt,
                     time_t dataTime, int fcastTime, char* id);
   
   
};

#endif
   
   

   
   

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
// Brenda Javornik, UCAR, Boulder, CO, 80307, USA
// July 2019
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _HAWKEYELOGGER_HH
#define _HAWKEYELOGGER_HH


#include <fstream>
#include <unistd.h>
#include <sys/times.h>
#include <toolsa/MsgLog.hh>
//#include <toolsa/LogStream.hh>
using namespace std;

/**
 * Set output file  as stream to use
 */
#define LOG_STREAM_TO_FILE() (LogState::getPointer()->setOutFile())

class IpsEyeLogger : public MsgLog { // NOTE: incompatible because both MsgLog & LogStream have "#define DEBUG" // , public LogStream {

public:
   IpsEyeLogger();
   IpsEyeLogger( const string &appName,
              const char *instance = NULL );
  virtual ~IpsEyeLogger();

  //   void ( char* description=NULL );

  void        init();

private:
  
  // singleton instance

  //  void msg(
  // long        clktck;
  //struct tms  *startTime;
  // clock_t     start;

  // bool        marking;
  //void        stopMarking();
  // void        startMarking( char *description );
};

#endif

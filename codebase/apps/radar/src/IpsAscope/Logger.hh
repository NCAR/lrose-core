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
// Mike Dixon, EOL, NCAR, Boulder, CO, 80307, USA
// Oct 2020
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _LOGGER_HH
#define _LOGGER_HH


#include <fstream>
#include <unistd.h>
#include <sys/times.h>
#include <toolsa/MsgLog.hh>
using namespace std;

/**
 * Set output file  as stream to use
 */
#define LOG_STREAM_TO_FILE() (LogState::getPointer()->setOutFile())

class Logger : public MsgLog {

public:
  Logger();
  Logger(const string &appName,
         const char *instance = NULL);
  ~Logger();
  
  void init();
  
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

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
// $Id: TriggerInfo.hh,v 1.10 2017/06/09 16:27:58 prestop Exp $
//
//   Description: Contains basic trigger info
//
////////////////////////////////////////////////////////////////////////////////
#ifndef TRIGGER_INFO_HH
#define TRIGGER_INFO_HH

#include <string>
#include <cassert>
#include <iostream>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <dataport/bigend.h> 
using namespace std;

class TriggerInfo {

public:

   TriggerInfo();
   TriggerInfo( const TriggerInfo& source );

  ~TriggerInfo(){};

   //
   // The basics
   //
   void copy( const TriggerInfo& source );
   const TriggerInfo& operator=( const TriggerInfo& source );
   void clear();

   //
   // Setting the trigger info explicitly
   //
   void setInfo( const time_t& issueTime,
                 const time_t& forecastTime,
                 const string& filePath,
                 const string& saysWho );

   void setIssueTime( const time_t& iTime ){ issueTime = iTime; }
   void setForecastTime( const time_t& fTime ){ forecastTime = fTime; }
   void setFilePath( const string path ){ filePath = path; }
   void setSaysWho( const string& who ){ saysWho = who; }
  
   //
   // Setting the trigger info from a message buffer
   //
   void setInfoFromMsg( const void* message );
  
   //
   // Fetching the trigger info explicitly
   //
   time_t     getIssueTime() const { return issueTime; }
   time_t     getForecastTime() const { return forecastTime; }
   const string&    getSaysWho() const { return saysWho; }
   const string&    getFilePath() const { return filePath; }

   //
   // Fetching the trigger info as a message buffer
   //
   void* getMsgFromInfo();
   int   getMsgLen(){ return sizeof( trigger_msg_t ); }

   //
   // Input/output methods
   //

   void print(ostream &stream) const;
  
   //
   // Static constants
   //
   static const size_t nameLen = 128;
   static const size_t pathLen = 1024;

private:

  //
  // Trigger info
  //
  time_t  issueTime;
  time_t  forecastTime;
  string  saysWho;
  string  filePath;

  //
  // For message <--> info translation
  //
  typedef struct 
  {
    si32 issueTime;
    si32 forecastTime;
    char saysWho[nameLen];
    char filePath[pathLen];
  } trigger_msg_t; 

  trigger_msg_t  triggerMsg;

};

#endif

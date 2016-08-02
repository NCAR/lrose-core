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
// $Id: TriggerInfo.cc,v 1.11 2016/03/03 18:06:33 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <dsdata/TriggerInfo.hh>
using namespace std;

TriggerInfo::TriggerInfo()
{
   clear();
}

TriggerInfo::TriggerInfo( const TriggerInfo& source )
{
   copy( source );
}

void 
TriggerInfo::copy( const TriggerInfo& source )
{
   issueTime    = source.issueTime;
   forecastTime = source.forecastTime;
   saysWho      = source.saysWho;
   filePath     = source.filePath;
}

const TriggerInfo& 
TriggerInfo::operator=( const TriggerInfo& source )
{
  if (this != &source)
    copy( source );

  return( *this );
}

void
TriggerInfo::clear()
{
   issueTime    = DateTime::NEVER;
   forecastTime = DateTime::NEVER;
   saysWho      = "";
   filePath     = "";
}

void
TriggerInfo::setInfo( const time_t& issueTime,
		      const time_t& forecastTime,
		      const string& filePath,
		      const string& saysWho)
{
  clear();
  
  setIssueTime(issueTime);
  setForecastTime(forecastTime);
  setFilePath(filePath);
  setSaysWho(saysWho);
}

void
TriggerInfo::setInfoFromMsg( const void* message )
{
   //
   // parse trigger message. Byte swap where necessary.
   //
   triggerMsg = (*((trigger_msg_t*)(message)));

   issueTime = (time_t)BE_to_si32( triggerMsg.issueTime );
   forecastTime = (time_t)BE_to_si32( triggerMsg.forecastTime );
}

void*
TriggerInfo::getMsgFromInfo()
{
  //
  // Setup the trigger message structure
  // Do the necessary byte swapping and safe string copy into trigger_msg_t.
  //
  triggerMsg.issueTime = BE_from_si32( (si32)issueTime );
  triggerMsg.forecastTime = BE_from_si32( (si32)forecastTime );

  STRncopy( triggerMsg.saysWho, saysWho.c_str(), (int)nameLen );

  return( &triggerMsg );
}

void
TriggerInfo::print(ostream &stream) const
{
  if (issueTime == DateTime::NEVER)
    stream << "   issue time: NOT SET" << endl;
  else
    stream << "   issue time: " << DateTime::str(issueTime) << endl;
  if (forecastTime == DateTime::NEVER)
    stream << "   forecast time: NOT SET" << endl;
  else
    stream << "   forecast time: " << DateTime::str(forecastTime) << endl;
  stream << "   says who: " << saysWho << endl;
  stream << "   file path: " << filePath << endl;
}

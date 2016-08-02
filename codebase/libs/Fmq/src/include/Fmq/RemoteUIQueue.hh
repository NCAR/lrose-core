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
//  $Id: 
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _RemoteUIQueue_INC
#define _RemoteUIQueue_INC

#include <string>
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <Fmq/DsFmq.hh>
using namespace std;


class RemoteUIQueue : public DsFmq
{
public:
   RemoteUIQueue();

   enum msgType {  REMOTE_UI_COMMAND };
   //
   // FMQ method for receiver application - Returns a const reference to
   // The command string
   //
   string& readNextContents(int &status);

   //
   // FMQ methods for source applications - Returns 0 on success
   //
   int  sendFileContents( const char * filename);

   int  sendStringContents( const string&  str);

private:
   //
   // The Struct where the latest message data is kept
   //
   string Contents;  // Contents of the  file - Contains Commands
   string fname;     // Name of the file whose contents

   //
   // Process management
   //
   pid_t            processId;

};

#endif

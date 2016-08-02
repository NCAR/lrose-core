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

#include <toolsa/MsgLog.hh>
#include <Fmq/RemoteUIQueue.hh>
#include <sys/types.h>
#include <sys/stat.h> 
#include <cerrno>
using namespace std;



///////////////////////////////////////////////////////////////////
// Constructor
RemoteUIQueue::RemoteUIQueue()
             :DsFmq()
{
   processId        = getpid();
}

///////////////////////////////////////////////////////////////////
//
//
int
RemoteUIQueue::sendFileContents( const char * filename)
{
   int status;
   FILE *infile;
   struct stat sbuf;

   // Check the file
   if(stat(filename,&sbuf) < 0) {
     int errNum = errno;
     cerr << "ERROR - RemoteUIQueue" << endl;
     cerr << "  Cannot stat input file: " << filename << endl;
     cerr << "  " << strerror(errNum) << endl;
     return -1;
   } 

   if((infile = fopen(filename,"r")) == NULL) {
     int errNum = errno;
     cerr << "ERROR - RemoteUIQueue" << endl;
     cerr << "  Cannot open input file: " << filename << endl;
     cerr << "  " << strerror(errNum) << endl;
     return -1;
   }

   // Allocate space for a complete message
   char *buf = new char[sbuf.st_size];

   if((int) fread(buf,1,sbuf.st_size,infile) != sbuf.st_size) {
     int errNum = errno;
     cerr << "ERROR - RemoteUIQueue" << endl;
     cerr << "  Cannot read input file: " << filename << endl;
     cerr << "  " << strerror(errNum) << endl;
     return -1;
   }

   // put it in the FMQ
   if ( writeMsg( REMOTE_UI_COMMAND, 0, (void*)(buf),sbuf.st_size)) {
      status = -1;
   } else {
      status = 0;
   }

   // close the file 
   fclose(infile);

   // Free the buffer
   delete []buf;

   return status;
}


///////////////////////////////////////////////////////////////////
//
//
int
RemoteUIQueue::sendStringContents( const string& str)
{
   int status;

   // put the string in the FMQ
   if ( writeMsg( REMOTE_UI_COMMAND, 0, (void*)(str.c_str()),str.size() )) {
      status = -1;
   } else {
      status = 0;
   }

   return status;
}

//////////////////////////////////////////////////////
// Returns a const reference to the latest message.
// Status < 0 indicates no new message was retrieved.

string&
RemoteUIQueue::readNextContents( int &status )
{
   int        msg_status;                                            
   bool       gotMsg;
   
   msg_status = readMsg( &gotMsg, REMOTE_UI_COMMAND );
   if ( msg_status == -1  ||  !gotMsg  ||  getMsgLen() == 0 ) {
      status = -1;
   } else {
      
      Contents  = (char *)getMsg();
      status = 0;
   }

   return(Contents);  
}

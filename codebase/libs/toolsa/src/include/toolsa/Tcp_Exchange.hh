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
// Tcp_Exchange.h : CLass to support Milti client TCP/IP messaging
//                   Based on the SOK2 library in toolsa
// -F. Hage Nov 1994
//


#include <cstdio>
#include <cstdlib>
#include <cerrno>


#define MAX_TCP_CLIENTS 32

#include <toolsa/Except.hh>
#include <toolsa/sok2.h>
using namespace std;

class Service_port {
protected:
 int port;         // TCP/IP port number
 int timeout;
 int index;        // ID of this service port
 int status;
 unsigned long mesage_num;  // Count of messages sent through this port.

public:
 // Constructors
    Service_port( int port, int time_out_msec = 100);

 // Set functions
     void set_timeout(int msec);	// set communications timeout values
     
 // Access functions


 // Utility
    void send_message(void *message, int length, int message_id);
    void send_message(void *message, int length, int message_id, int client);

    // pass in  pointer messaage handler - deques all messages until times out
    void get_messages(void (*func)(int msg_id, void* msg, int len, int client));

 // ~destructors
    ~Service_port();
         
};

class Client_port {
protected:
 int port;         // TCP/IP port number
 int timeout;
 int retry_seconds;
 int index;        // ID of this service port
 int status;

public:
 // Constructors
    Client_port(char * hostname, int port, int time_out_msec = 100, int retry_secs = 30);

 // Set functions
     void set_timeout(int msec);	// set communications timeout values
     
 // Access functions

 // Utility
    void send_message(void *message, int length, int message_id);

    // pass in a pointer to a message handler - deques all messages 
    void get_messages(void (*func)(int msg_id, void* msg, int len));

 // Destructors
    ~Client_port();
         
};

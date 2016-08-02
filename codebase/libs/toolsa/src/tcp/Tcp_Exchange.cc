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
// TCP_EXCHANGE.C : CLass to support Milti client TCPIP messaging
// Class: Service_port:  For data services - Accepts concections
// Class Client_port:  To access data services: _ Initiates connections
//
// -F. Hage  Nov 1994


#include <toolsa/Tcp_Exchange.hh>
using namespace std;


////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
Service_port::Service_port(int prt,int t_out)
{
    static char init_str[13] = "TCP_EXCHANGE";
    SOK2init(init_str);

    if((index = SOK2openServer(prt)) < 0) THROW(PORT_UNAVAILIBLE,"SOK2 can't get port?");
    port = prt;
    timeout = t_out;
}

////////////////////////////////////////////////////////////////////////////////
// SET FUNCTIONS

void   Service_port::set_timeout(int msec) { timeout = msec; }

////////////////////////////////////////////////////////////////////////////////
// DESTRUCTORS
Service_port::~Service_port()
{
  if(SOK2close(index) == 0) THROW(PORT_UNAVAILIBLE,"SOK2 Can't close port");
}


////////////////////////////////////////////////////////////////////////////////
// UTILITY  Pass in a pointer to a function to handle messages

void Service_port::get_messages(void (*func)( int msg_id, void* msg, int len, int client))  
{
    int  mess_len;
    int  clt;
    int  idx;
    int stats;
    char    *msg;
    SOK2head shead;

     do {
        stats = SOK2getMessage(timeout,&idx,&clt,&shead,&msg,&mess_len);
        if(stats == 1)  {
            func((int) shead.id,(void *) msg,mess_len,clt);
        } else if(stats < 0) THROW(errno,"erroro from SOK2getMessage()");

     } while(stats > 0);

}

void Service_port::send_message(void* message, int length, int message_id)  
{
     status = SOK2sendMessageAll(timeout,index,message_id,(char *) message,length,1);
}

void Service_port::send_message(void* message, int length, int message_id, int client)  
{
     status = SOK2sendMessage(timeout,index,client, message_id,(char *) message,length);
}


////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
Client_port::Client_port(char *hname, int prt,int t_out, int retry_secs)
{
    static char init_str[13] = "TCP_EXCHANGE";
    SOK2init(init_str);

    if((index = SOK2openClient(hname,prt,retry_secs)) < 0) THROW(PORT_UNAVAILIBLE,"SOK2 can't get port?");
    port = prt;
    timeout = t_out;
    retry_seconds = retry_secs;
}

////////////////////////////////////////////////////////////////////////////////
// SET FUNCTIONS

inline void   Client_port::set_timeout(int msec) { timeout = msec; }

////////////////////////////////////////////////////////////////////////////////
// DESTRUCTORS
Client_port::~Client_port()
{
  if(SOK2close(index) == 0) THROW(PORT_UNAVAILIBLE,"SOK2 Can't close port");
}


////////////////////////////////////////////////////////////////////////////////
// UTILITY  Pass in a pointer to a function to handle messages

void Client_port::get_messages(void (*func)( int msg_id, void* msg, int len))  
{
    int  mess_len;
    int  clt;
    int  idx;
    int stats;
    SOK2head shead;
    char    *msg;

     do {

        stats = SOK2getMessage(timeout,&idx,&clt,&shead,&msg,&mess_len);
        if(stats == 1)  {
            func((int) shead.id,(void*) msg,mess_len);
        } else if(stats < 0) THROW(errno,"erroro from SOK2getMessage()");

     } while(stats > 0);

}

void Client_port::send_message(void* message, int length, int message_id)  
{
     status = SOK2sendMessage(timeout,index,0,message_id,(char*) message,length);
}

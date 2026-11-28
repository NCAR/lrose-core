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
/*
-----------------------------------------------------------------------------
Globals.hh

Header for global variable definitions for SstNc2Mdv
-----------------------------------------------------------------------------
*/

#ifndef SstNc2MdvGlobals_h
#define SstNc2MdvGlobals_h

#include <string>   /* for strings */
#include <fstream>  /* for ofstream */

#include <toolsa/MsgLog.hh>

using namespace std;

/*
--- Variables defined in SstNc2MdvMain and declared "extern" elsewhere
*/

#ifdef SstNc2MdvMain_c

   const char         *VERSION       = "0.1";
   const char         *PROGRAM_NAME  = "SstNc2Mdv";

#else

   extern const char  *VERSION;
   extern const char  *PROGRAM_NAME;

#endif /* SstNc2MdvMain_c */

#define SUCCESS 0
#define FAILURE -1

#define POSTMSG            GetMsgLog().postMsg
#define DEBUG_ENABLED      GetMsgLog().isEnabled( DEBUG )
#define STR_EQL(test,std)  (strcmp(test,std)==0)

#endif /* SstNc2MdvGlobals_h */

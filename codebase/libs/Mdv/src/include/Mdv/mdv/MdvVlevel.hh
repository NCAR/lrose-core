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

#ifndef _MDV_VLEVEL_INC_
#define _MDV_VLEVEL_INC_

#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_user.h>
#include <Mdv/mdv/mdv_macros.h>
using namespace std;

class MdvVlevel
{
public:
   MdvVlevel( ); // No body provided. Do not use.
   MdvVlevel( int type, float param = 0.0 );
   MdvVlevel( const MDV_vlevel_header_t & info );
   MdvVlevel( const MdvVlevel &source );
  ~MdvVlevel(){};

   //
   // Copying 
   //
   MdvVlevel& operator= ( const MdvVlevel &source );
   void copy( const MdvVlevel &source );

   //
   // Get the header info and muck with it yo'self
   //
   MDV_vlevel_header_t&    getInfo(){ return info; }

   //
   // Setting info
   //
   int     addInfo( int type, float param = 0.0 );

private:

   size_t                  count;
   MDV_vlevel_header_t     info;

};

#endif

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

#include <Mdv/mdv/MdvVlevel.hh>
using namespace std;

MdvVlevel::MdvVlevel( int type, float param )
{
   MDV_init_vlevel_header( &info );
   addInfo( type, param );
}

MdvVlevel::MdvVlevel( const MDV_vlevel_header_t & srcinfo )
{
   info = srcinfo;
   count = MDV_MAX_VLEVELS - 1;
}

MdvVlevel::MdvVlevel( const MdvVlevel &source )
{
   copy( source );
}

MdvVlevel& 
MdvVlevel::operator= ( const MdvVlevel &source )
{
   copy( source );
   return *this;
}

void 
MdvVlevel::copy( const MdvVlevel &source )
{
   size_t i;

   MDV_init_vlevel_header( &info );
   count = source.count;

   //
   // NOTE: unused_ fields are not copied!
   //
   for( i=0; i < count; i++ ) {
      info.vlevel_type[i]   = source.info.vlevel_type[i];
      info.vlevel_params[i] = source.info.vlevel_params[i];
   }
}

int 
MdvVlevel::addInfo( int type, float param )
{
   if ( count < MDV_MAX_VLEVELS ) {
      info.vlevel_type[count] = type;
      info.vlevel_params[count] = param;
      count++;
      return( MDV_SUCCESS );
   }
   else {
      return( MDV_FAILURE );
   }
}

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

#ifndef _MDV_MASTER_HDR_INC_
#define _MDV_MASTER_HDR_INC_

#include <ctime>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_user.h>
#include <Mdv/mdv/mdv_macros.h>
using namespace std;

class MdvMasterHdr
{
public:
   MdvMasterHdr();
   MdvMasterHdr( const MdvMasterHdr &source );
  ~MdvMasterHdr(){};

   //
   // Copying 
   //
   MdvMasterHdr& operator= ( const MdvMasterHdr &source );
   void copy( const MdvMasterHdr &source );

   //
   // Get the header info and muck with it yo'self
   //
   MDV_master_header_t &    getInfo()                           { return info; }

   // 
   // Set the header info. Do not do this without keeping some important items
   //   in synch. See MdvFile::setMasterHeader.
   // 
   void                     setInfo(const MDV_master_header_t & i) { info = i; }

   //
   // Setting info
   //
   void    setTime( time_t timeStamp );
   void    clearTime();
   void    setSensor( double lat, double lon, double alt );
   void    setDescription( const char *name, 
                           const char *source=NULL,
                           const char *desc=NULL );

   void    setGeometry( size_t nxMax, size_t nyMax, size_t nzMax,
                        size_t dimension, bool differ );
   void    clearGeometry();

   inline  void    setNumFields( size_t num ){ info.n_fields = (si32)num; }
   inline  void    setNumLevels( size_t num ){ info.vlevel_included = num ? 
                                               1 : 0; }
   inline  void    setNumChunks( size_t num ){ info.n_chunks = (si32)num; }

   //
   // Fetching info
   //
   inline  size_t  numFields(){ return info.n_fields; }
   inline  size_t  numLevels(){ return( info.vlevel_included ? 
                                info.n_fields : 0 ); }
   inline  size_t  numChunks(){ return info.n_chunks; }
   inline  char*   getSource(){ return info.data_set_source; }
           void    getGeometry( size_t *nxMax, size_t *nyMax, size_t *nzMax,
                                size_t *dimension, bool *differ=NULL );

   //
   // We'll do it for you
   //
   void    calcOffsets();

private:
   MDV_master_header_t     info;
};

#endif

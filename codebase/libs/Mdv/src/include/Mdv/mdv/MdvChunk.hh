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

#ifndef _MDV_CHUNK_INC_
#define _MDV_CHUNK_INC_

#include <toolsa/str.h>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_user.h>
#include <Mdv/mdv/mdv_macros.h>
using namespace std;

class MdvChunk
{
public:
   MdvChunk( void *chunkData, size_t size, int id = -1, 
                                           const char *descrip = NULL );
   MdvChunk( const MdvChunk &source );
   MdvChunk();
  ~MdvChunk(){};

   //
   // Copying 
   //
   MdvChunk& operator= ( const MdvChunk &source );
   void copy( const MdvChunk &source );

   //
   // Setting the members
   //
   void clear();
   void setData( void *chunkData, size_t size ){ data = chunkData; 
                                                 info.size = (si32)size; }
   void setId( int id ){ info.chunk_id = (si32)id; }
   void setDescription( const char* descrip ){ STRncopy( info.info, descrip, 
                                               MDV_CHUNK_INFO_LEN ); }

   //
   // Get the header info and muck with it yo'self
   //
   MDV_chunk_header_t&    getInfo(){ return info; }
   void*                  getData(){ return data; }

   //
   // Offsets
   //
   void  setOffset( int offset ){ info.chunk_data_offset = (si32)offset; }

private:

   void                  *data;
   MDV_chunk_header_t     info;

};

#endif

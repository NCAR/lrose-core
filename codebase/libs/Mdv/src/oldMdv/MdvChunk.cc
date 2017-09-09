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

#include <Mdv/mdv/MdvChunk.hh>
#include <toolsa/str.h>
using namespace std;

MdvChunk::MdvChunk( void *chunkData, size_t size, int id, const char *descrip )
{
   MDV_init_chunk_header( &info );
   setDescription( descrip );

   data          = chunkData;
   info.size     = (si32)size;
   info.chunk_id = (si32)id;
}

MdvChunk::MdvChunk()
{
   clear();
}

MdvChunk::MdvChunk( const MdvChunk &source )
{
   copy( source );
}

MdvChunk& 
MdvChunk::operator= ( const MdvChunk &source )
{
   copy( source );
   return *this;
}

void
MdvChunk::clear()
{
   MDV_init_chunk_header( &info );

   data          = NULL;
   info.size     = 0;
   info.chunk_id = -1;
}

void 
MdvChunk::copy( const MdvChunk &source )
{
   MDV_init_chunk_header( &info );

   //
   // NOTE: unused_ fields are not copied!
   //
   data          = source.data;
   info.chunk_id = source.info.chunk_id;
   info.size     = source.info.size;

   STRncopy( info.info, source.info.info, MDV_CHUNK_INFO_LEN );
}

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

#include <string>
#include <toolsa/DateTime.hh>
#include <Mdv/mdv/MdvChunk.hh>
#include <Mdv/mdv/TimestampChunk.hh>
using namespace std;

TimestampChunk::TimestampChunk()
{
   init();
}

TimestampChunk::TimestampChunk( string &newName, time_t newTime )
{
   init();
   setName( newName );
   setTime( newTime );
}

TimestampChunk::TimestampChunk( const char *newName, time_t newTime )
{
   init();
   setName( newName );
   setTime( newTime );
}

TimestampChunk::TimestampChunk( void *data, si32 )
{
   //
   // Create a timestamp from an MDV chunk
   // Do the necessary byte swapping
   //
   init();
   time_t newTime = (time_t)BE_to_si32(((si32 *)data)[0]);
   string newName = ((char *)data + sizeof(si32));
   setPostMark( newName, newTime );
}

TimestampChunk::~TimestampChunk()
{
   delete chunk;
}

void
TimestampChunk::init()
{
   when  = -1;
   chunk = new MdvChunk;
   chunk->setId( MDV_CHUNK_NOWCAST_DATA_TIMES );
   chunk->setData( &chunkData, sizeof(chunkData) );
   chunk->setDescription( "Timestamp" );
}

void 
TimestampChunk::setName( const string &newName )
{ 
   //
   // Set both the swapped and unswapped names
   //
   name = newName; 
   STRncopy( chunkData.who, name.c_str(), MDV_LONG_FIELD_LEN );
}

void 
TimestampChunk::setTime( time_t newTime )
{ 
   //
   // Set both the swapped and unswapped times
   //
   when = newTime; 
   chunkData.when = BE_from_si32((si32)newTime);
}

void 
TimestampChunk::setPostMark( const string &newName, time_t newTime )
{ 
   setName( newName ); 
   setTime( newTime ); 
}

void 
TimestampChunk::print( FILE *outfile )
{
   //
   // Print chunk info
   //
   fprintf( outfile, "Timestamp: %s   %s", 
                     when.dtime(), name.c_str() );  
}

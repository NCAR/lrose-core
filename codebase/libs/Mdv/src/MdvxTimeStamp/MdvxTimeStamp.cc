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
#include <dataport/bigend.h>
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/DateTime.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/MdvxTimeStamp.hh>
using namespace std;

MdvxTimeStamp::MdvxTimeStamp()
{
  init();
}

MdvxTimeStamp::MdvxTimeStamp( const string &newName, time_t newTime )
{
  init();
  setName( newName );
  setTime( newTime );
}

MdvxTimeStamp::MdvxTimeStamp( const char *newName, time_t newTime )
{
  init();
  setName( newName );
  setTime( newTime );
}

MdvxTimeStamp::MdvxTimeStamp( const MdvxChunk &chunk )
{
  //
  // Create a timestamp from an MDV chunk
  // Do the necessary byte swapping
  //
  init();
  loadFromChunk(chunk);
}

MdvxTimeStamp::~MdvxTimeStamp()
{
}

void
MdvxTimeStamp::init()
{
  when  = -1;
}

void 
MdvxTimeStamp::setName( const string &newName )
{ 
  name = newName; 
}

void 
MdvxTimeStamp::setTime( time_t newTime )
{ 
  when = newTime; 
}

void 
MdvxTimeStamp::setPostMark( const string &newName, time_t newTime )
{ 
  setName( newName ); 
  setTime( newTime ); 
}

//////////////////////////
// load object from  chunk
//
// returns 0 on siccess, -1 on failure

int MdvxTimeStamp::loadFromChunk(const MdvxChunk &chunk)

{

  if (chunk.getSize() < (int) sizeof(si32)) {
    return -1;
  }
  if (chunk.getId() != Mdvx::CHUNK_NOWCAST_DATA_TIMES) {
    return -1;
  }

  // load up time

  si32 ttime;
  memcpy(&ttime, chunk.getData(), sizeof(si32));
  BE_to_array_32(&ttime, sizeof(si32));
  setTime(ttime);

  // load up name
  
  if (chunk.getSize() == sizeof(si32)) {
    setName("");
  } else {
    MemBuf nameBuf;
    nameBuf.add((char *) chunk.getData() + sizeof(si32),
		chunk.getSize() - sizeof(si32));
    // make sure it is null-terminated
    int i = 0;
    nameBuf.add(&i, sizeof(int));
    setName((char *) nameBuf.getPtr());
  }

  return 0;

}

//////////////////////////////////////////////////
// create a chunk from the object
// returns pointer to chunk object.

MdvxChunk *MdvxTimeStamp::createChunk()
{

  MdvxChunk *chunk = new MdvxChunk;
  chunk->setId(Mdvx::CHUNK_NOWCAST_DATA_TIMES);
  chunk->setInfo("Timestamp");
  
  MemBuf workBuf;
  si32 ttime = BE_from_si32(when.utime());
  workBuf.add(&ttime, sizeof(si32));
  char nameStr[MDV_LONG_FIELD_LEN];
  MEM_zero(nameStr);
  STRncopy(nameStr, name.c_str(), MDV_LONG_FIELD_LEN);
  workBuf.add(nameStr, MDV_LONG_FIELD_LEN);

  chunk->setData(workBuf.getPtr(), workBuf.getLen());

  return chunk;

}

void 
MdvxTimeStamp::print( FILE *outfile )
{
  fprintf( outfile, "Timestamp: %s   %s", 
	   when.dtime(), name.c_str() );  
}

void 
MdvxTimeStamp::print( ostream &out )
{
  out << "Timestamp: " << when.dtime() << "  " << name.c_str() << endl;
}


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
///////////////////////////////////////////////////////////////
// TstormMgr class
//   Reads and interprets tstorm data from spdb
//
// $Id: TstormMgr.cc,v 1.8 2016/03/03 18:06:33 dixon Exp $
//////////////////////////////////////////////////////////////

#include <dataport/port_types.h>
#include <dsdata/TstormMgr.hh>
#include <dsdata/Tstorm.hh>

using namespace std;

TstormMgr::TstormMgr() 
{
   _url          = "";
   _timeMargin   = 0;
}

TstormMgr::TstormMgr(const string& spdbUrl, const time_t margin ) 
{
   _url          = spdbUrl;
   _timeMargin   = margin;
}

TstormMgr::~TstormMgr() 
{
   clearData();
}

void
TstormMgr::clearData()
{
   vector< TstormGroup* >::iterator groupIt;
   
   for( groupIt = _groups.begin(); groupIt != _groups.end(); groupIt++ ) {
      delete ( *groupIt );
   }
   _groups.erase( _groups.begin(), _groups.end() );
}

int
TstormMgr::readTstorms(const time_t when ) 
{
   int                 nInputChunks;
   Spdb::chunk_ref_t  *inputChunkHdrs;
   char               *inputChunkData;

   //
   // Query the server
   //
   if ( _spdbMgr.getFirstBefore( _url, when, _timeMargin ) != 0 )
      return( -1 );

   //
   // Get pointers to the headers and data 
   //
   nInputChunks   = _spdbMgr.getNChunks();
   inputChunkHdrs = _spdbMgr.getChunkRefs();
   inputChunkData = (char *) _spdbMgr.getChunkData();

   //
   // Return if we didn't get anything
   //
   if( nInputChunks == 0 ) {
      return( 0 );
   }

   //
   // Process each chunk
   //
   char* chunkPtr = inputChunkData;
   
   for( int iChunk = 0; iChunk < nInputChunks; iChunk++ ) {

      //
      // Get pointer to group of storms
      //
      chunkPtr = (char *) (inputChunkData + inputChunkHdrs[iChunk].offset);

      //
      // Byte swapping
      //
      tstorm_spdb_buffer_from_BE( (ui08*) chunkPtr );

      //
      // Set up the group of storms
      //
      TstormGroup *newGroup = new TstormGroup( chunkPtr );
      newGroup->setExpireTime(inputChunkHdrs[iChunk].expire_time);
      _groups.push_back( newGroup );

   }

   return( nInputChunks );
   
}

int
TstormMgr::readTstorms(const string& spdbUrl,
		       const time_t when,
		       const time_t margin ) 
{
   _url        = spdbUrl;
   _timeMargin = margin;

   return( readTstorms( when ) );
}

   
bool TstormMgr::writeTstorms(const string& spdb_url)
{
  // If the URL was specified, update the intermal member

  if (spdb_url != "")
    _url = spdb_url;

  // Each TstormGroup will be a separate chunk in the database

  _spdbMgr.setPutMode(Spdb::putModeAdd);
  _spdbMgr.clearPutChunks();
  
  vector< TstormGroup* >::iterator group_iter;
  
  for (group_iter = _groups.begin(); group_iter != _groups.end();
       ++group_iter)
  {
    TstormGroup *group = *group_iter;
    
    // Create the SPDB chunk for the group

    int spdb_chunk_size = group->getSpdbNumBytes();
    ui08 *spdb_chunk = new ui08[spdb_chunk_size];
    
    group->writeSpdb(spdb_chunk);
    
    // Add the chunk to the SPDB put buffer

    _spdbMgr.addPutChunk(SPDB_TSTORMS_PROD_TYPE,
			 group->getDataTime(),
			 group->getExpireTime(),
			 spdb_chunk_size,
			 (void *)spdb_chunk);
    
    delete [] spdb_chunk;
  } /* endfor - group_iter */
  
  // Now write all of the chunks to the database

  if (_spdbMgr.put(_url,
		   SPDB_TSTORMS_ID,
		   SPDB_TSTORMS_LABEL) != 0)
    return false;
  
  return true;
}

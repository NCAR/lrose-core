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
//////////////////////////////////////////////////////////
// spdb buffer 
//
//////////////////////////////////////////////////////////

#include <cstdio>
#include <toolsa/str.h>
#include <toolsa/MsgLog.hh>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <rapformats/station_reports.h>
#include <symprod/spdb_client.h>
#include <symprod/spdb_products.h>
#include <Spdb/DsSpdb.hh>

#include "SpdbBuffer.hh"
#include "Metar.hh"
#include "Metar2Spdb.hh"
using namespace std;

SpdbBuffer::SpdbBuffer(int spdb_id, const char *spdb_label,
		       int expSecs, bool use_urls)
{
   expireSecs    = expSecs;
   nChunks       = 0;
   spdbId = spdb_id;
   spdbLabel = spdb_label;
   useUrls = use_urls;
}

SpdbBuffer::~SpdbBuffer() 
{

}

void 
SpdbBuffer::reset()
{
  nChunks = 0;
  chunkHdrBuf.free();
  chunkBuf.free();
}

void SpdbBuffer::clearDests()
{
  destinations.erase(destinations.begin(), destinations.end());
}

void SpdbBuffer::addDest(const char *destination)
{
  string destStr(destination);
  destinations.push_back(destStr);
}

void 
SpdbBuffer::addDecodedMetar( Metar& metar )
{

  if( INFO_ENABLED ) {
     metar.printStationReport();
  }

  station_report_t stationReport = metar.getStationReport();
 
  //
  // Fill in chunk header
  //
  spdb_chunk_ref_t chunkHdr;
  MEM_zero(chunkHdr);
  chunkHdr.valid_time  = stationReport.time;
  chunkHdr.expire_time = chunkHdr.valid_time + expireSecs;
  chunkHdr.data_type   = metar.getHashId();
  chunkHdr.offset      = chunkBuf.getLen();
  chunkHdr.len         = sizeof(station_report_t);

  //
  // Convert the data to BIG ENDIAN 
  //
  station_report_to_be(&stationReport);

  //
  // Add to buffers
  //
  chunkHdrBuf.add(&chunkHdr, sizeof(chunkHdr));
  chunkBuf.add(&stationReport, sizeof(stationReport));

  //
  // Increment n chunks
  //
  nChunks++;

}

void 
SpdbBuffer::addRawMetar( Metar& metar )
{

  POSTMSG( INFO, "Raw metar: %s", metar.getRawText().c_str());
  
  const station_report_t &stationReport = metar.getStationReport();
  
  //
  // Fill in chunk header
  //
  spdb_chunk_ref_t chunkHdr;
  MEM_zero(chunkHdr);
  chunkHdr.valid_time  = stationReport.time;
  chunkHdr.expire_time = chunkHdr.valid_time + expireSecs;
  chunkHdr.data_type   = metar.getHashId();
  chunkHdr.offset      = chunkBuf.getLen();
  chunkHdr.len         = metar.getRawText().size() + 1;

  //
  // Add to buffers
  //
  chunkHdrBuf.add(&chunkHdr, sizeof(chunkHdr));
  chunkBuf.add(metar.getRawText().c_str(), metar.getRawText().size() + 1);

  //
  // Increment n chunks
  //
  nChunks++;

}

void 
SpdbBuffer::send()
{

  if (useUrls && destinations.size() > 0) {

    DsSpdb spdb;
    spdb.addPutChunks(nChunks, 
		      (Spdb::chunk_ref_t *) chunkHdrBuf.getPtr(),
		      chunkBuf.getPtr());

    if (spdb.put(destinations[0], spdbId, spdbLabel)) {
      POSTMSG( WARNING, "Couldn't write metars: %s",
	       spdb.getErrStr().c_str() );
      
    }

  } else {

    for( size_t i = 0; i < destinations.size(); i++) {
      if (SPDB_put_over((char *) destinations[i].c_str(),
			spdbId,
			(char *) spdbLabel.c_str(),
			nChunks,
			(spdb_chunk_ref_t *) chunkHdrBuf.getPtr(),
			chunkBuf.getPtr(),
			chunkBuf.getLen()) != 0) {
	POSTMSG( WARNING, "Couldn't write metars to %s",
		 destinations[i].c_str() );
      }
    }
    
  } // if (useUrls) 
    
  SPDB_reap_children();
  
}



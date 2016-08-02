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
/////////////////////////////////////////////////////////////////////
// DsRadarAz.cc
//
// Radar azimuths Class
//
// Mike Dixon, RAP, NCAR, Boulder, CO, USA
//
// Jan 2004
//
///////////////////////////////////////////////////////////////////////
//
// This class allows you to read a chunk and load the object, or
// save the object out to a chunk.
//
////////////////////////////////////////////////////////////////////////


#include <dataport/bigend.h>
#include <rapformats/DsRadarAz.hh>
#include <iostream>
#include <iomanip>
#include <string.h>
using namespace std;

////////////////////////
// constructor
//
// Assume just 1 region

DsRadarAz::DsRadarAz()

{

  _nAz = 0;

}

/////////////
// destructor

DsRadarAz::~DsRadarAz()

{

}

/////////////////////////////////////////////////////
// Alloc space for a given number of azimuth angles

void DsRadarAz::alloc(int n_az)

{

  _nAz = n_az;
  _azBuf.prepare(_nAz * sizeof(fl32));
  int chunkLen = _nAz * sizeof(fl32) + sizeof(si32);
  _chunkBuf.prepare(chunkLen);

}

//////////////////
// load from chunk

void DsRadarAz::loadFromChunk(void *chunk_data, int chunk_len)
  
{

  _chunkBuf.free();
  _chunkBuf.add(chunk_data, chunk_len);
  si32 n_az;
  memcpy(&n_az, chunk_data, sizeof(si32));
  _nAz = n_az;
  _azBuf.free();
  _azBuf.add((char *) chunk_data + sizeof(si32),
	       _nAz * sizeof(fl32));
  
}

////////////////
// save to chunk

void DsRadarAz::saveToChunk()
  
{

  si32 n_az = getNAz();
  _chunkBuf.free();
  _chunkBuf.add(&n_az, sizeof(si32));
  _chunkBuf.add(_azBuf.getPtr(), _azBuf.getLen());

}

////////
// print

void DsRadarAz::print(ostream &out) const
  
{

  out << "RADAR AZIMUTHS" << endl;
  out << "--------------" << endl;

  out << "  N azimuths: " << getNAz() << endl;
  const fl32 *azs = getAzArray();
  for (int i = 0; i < getNAz(); i++) {
    out << "  az[" << setw(2) << i << "]: "
	<< azs[i] << endl;
  }
  out << endl;

}

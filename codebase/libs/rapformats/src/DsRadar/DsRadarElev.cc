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
// DsRadarElev.cc
//
// Radar elevations Class
//
// Mike Dixon, RAP, NCAR, Boulder, CO, USA
//
// Sept 1999
//
///////////////////////////////////////////////////////////////////////
//
// This class allows you to read a chunk and load the object, or
// save the object out to a chunk.
//
////////////////////////////////////////////////////////////////////////


#include <dataport/bigend.h>
#include <rapformats/DsRadarElev.hh>
#include <iostream>
#include <iomanip>
#include <string.h>
using namespace std;

////////////////////////
// constructor
//
// Assume just 1 region

DsRadarElev::DsRadarElev()

{

  _nElev = 0;

}

/////////////
// destructor

DsRadarElev::~DsRadarElev()

{

}

/////////////////////////////////////////////////////
// Alloc space for a given number of elevation angles

void DsRadarElev::alloc(int n_elev)

{

  _nElev = n_elev;
  _elevBuf.prepare(_nElev * sizeof(fl32));
  int chunkLen = _nElev * sizeof(fl32) + sizeof(si32);
  _chunkBuf.prepare(chunkLen);

}

//////////////////
// load from chunk

void DsRadarElev::loadFromChunk(void *chunk_data, int chunk_len)
  
{

  _chunkBuf.free();
  _chunkBuf.add(chunk_data, chunk_len);
  si32 n_elev;
  memcpy(&n_elev, chunk_data, sizeof(si32));
  _nElev = n_elev;
  _elevBuf.free();
  _elevBuf.add((char *) chunk_data + sizeof(si32),
	       _nElev * sizeof(fl32));
  
}

////////////////
// save to chunk

void DsRadarElev::saveToChunk()
  
{

  si32 n_elev = getNElev();
  _chunkBuf.free();
  _chunkBuf.add(&n_elev, sizeof(si32));
  _chunkBuf.add(_elevBuf.getPtr(), _elevBuf.getLen());

}

////////
// print

void DsRadarElev::print(ostream &out) const
  
{

  out << "RADAR ELEVATIONS" << endl;
  out << "----------------" << endl;

  out << "  N elevations: " << getNElev() << endl;
  const fl32 *elevs = getElevArray();
  for (int i = 0; i < getNElev(); i++) {
    out << "  elev[" << setw(2) << i << "]: "
	<< elevs[i] << endl;
  }
  out << endl;

}

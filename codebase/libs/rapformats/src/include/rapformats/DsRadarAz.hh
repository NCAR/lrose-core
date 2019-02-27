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
// DsRadarAz.hh
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

#ifndef DsRadarAz_h
#define DsRadarAz_h


#include <toolsa/MemBuf.hh>
using namespace std;

class DsRadarAz

{
  
public:

  DsRadarAz();
  
  /////////////
  // destructor
  
  virtual ~DsRadarAz();

  /////////////////////////////
  // Alloc space for azimuths
  // Sets number of azimuths.

  void alloc(int n_az);
  
  //////////////////
  // load from chunk
  
  void loadFromChunk(void *chunk_data, int chunk_len);

  //////////////////
  // save to chunk
  
  void saveToChunk();

  ////////
  // print

  void print(ostream &out) const;

  // accessing the data

  int getNAz() const { return (_nAz); }
  int getChunkLen() const { return (_chunkBuf.getLen()); }

  const fl32 *getAzArray() const { return ((fl32 *) _azBuf.getPtr()); }
  const void *getChunkData() const { return (_chunkBuf.getPtr()); }

  // non-const methods should only be used by apps writing 
  // to this object

  fl32 *getAzArray() { return ((fl32 *) _azBuf.getPtr()); }
  void *getChunkData() { return (_chunkBuf.getPtr()); }
  

protected:

  int _nAz;
  MemBuf _azBuf;
  MemBuf _chunkBuf;

private:

};
  
#endif


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
// DsRadarElev.hh
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

#ifndef DsRadarElev_h
#define DsRadarElev_h


#include <toolsa/MemBuf.hh>
using namespace std;

class DsRadarElev

{
  
public:

  DsRadarElev();
  
  /////////////
  // destructor
  
  virtual ~DsRadarElev();

  /////////////////////////////
  // Alloc space for elevations
  // Sets number of elevations.

  void alloc(int n_elev);
  
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

  int getNElev() const { return (_nElev); }
  int getChunkLen() const { return (_chunkBuf.getLen()); }

  const fl32 *getElevArray() const { return ((fl32 *) _elevBuf.getPtr()); }
  const void *getChunkData() const { return (_chunkBuf.getPtr()); }

  // non-const methods should only be used by apps writing 
  // to this object

  fl32 *getElevArray() { return ((fl32 *) _elevBuf.getPtr()); }
  void *getChunkData() { return (_chunkBuf.getPtr()); }
  

protected:

  int _nElev;
  MemBuf _elevBuf;
  MemBuf _chunkBuf;

private:

};
  
#endif


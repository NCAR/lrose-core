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
// mdv/MdvReadRadar.hh
//
// Class for representing radar info in MDV file chunk data.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1999
//
////////////////////////////////////////////////////////////
//
// This class is provided because radar paramters in MDV chunks
// are stored in various formats, depending upon when the file
// was written. This class converts the radar parameters into
// a standard format.
//
// Use the function MdvRead::loadRadar() to load up this object
// from chunk data.
//
// radarParamsAvail() returns true if radar params are available.
// radarElevsAvail() returns true if radar elevs are available.
//
// getRadarParams() returns a reference to the radar params struct.
// getRadarElevs() returns a reference to the elevations struct.
//
////////////////////////////////////////////////////////////////

#ifndef MdvReadRadar_hh
#define MdvReadRadar_hh

#include <rapformats/ds_radar.h>
using namespace std;

class MdvRead;
class MdvReadChunk;

class MdvReadRadar {

  friend class MdvRead;

public:

  // constructor
  
  MdvReadRadar();

  // destructor

  virtual ~MdvReadRadar();

  // clear memory
  
  void clear();

  // access to data members

  bool radarParamsAvail() { return (_radarParamsAvail); }
  bool radarElevsAvail() { return (_radarElevsAvail); }

  DsRadarParams_t &getRadarParams() { return (_radarParams); }
  DsRadarElev_t &getRadarElevs() { return (_radarElevs); }

protected:

  bool _radarParamsAvail;
  bool _radarElevsAvail;
  DsRadarParams_t _radarParams;
  DsRadarElev_t _radarElevs;

  void _loadFromChunk(MdvReadChunk &chunk, int nfields);
  
private:

};

#endif



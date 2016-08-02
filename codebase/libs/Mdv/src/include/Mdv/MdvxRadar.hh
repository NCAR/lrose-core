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
// Mdv/MdvxRadar.hh
//
// Class for representing radar info in Mdvx class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1999
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
// radarCalibAvail() returns true if radar calib are available.
// radarElevAvail() returns true if radar elevs are available.
// radarAzAvail() returns true if radar azimuths are available.
//
// getRadarParams() returns a reference to the radar params object.
// getRadarCalib() returns a reference to the radar calib object.
// getRadarElev() returns a reference to the elevations object.
// getRadarAz() returns a reference to the azimuths object.
//
////////////////////////////////////////////////////////////////

#ifndef MdvxRadar_hh
#define MdvxRadar_hh

#include <rapformats/DsRadarParams.hh>
#include <rapformats/DsRadarCalib.hh>
#include <rapformats/DsRadarElev.hh>
#include <rapformats/DsRadarAz.hh>
using namespace std;

class Mdvx;
class MdvxChunk;

class MdvxRadar {

  friend class Mdvx;

public:

  // constructor
  
  MdvxRadar();

  // destructor

  virtual ~MdvxRadar();

  // clear memory
  
  void clear();

  // load radar object from MdvxObject
  //
  // returns 0 on success, -1 on failure
  
  int loadFromMdvx(const Mdvx &mdvx);

  // create chunk object for radar params
  //
  // Performs a 'new' to create the chunk.
  // Passes ownership of chunk object to calling routine.
  //
  // Returns pointer to chunk on success, NULL on failure
  
  MdvxChunk *createParamsChunk();

  // create chunk object for radar calib
  //
  // Performs a 'new' to create the chunk.
  // Passes ownership of chunk object to calling routine.
  //
  // Returns pointer to chunk on success, NULL on failure
  
  MdvxChunk *createCalibChunk();

  // create chunk object for radar elevations
  //
  // Performs a 'new' to create the chunk.
  // Passes ownership of chunk object to calling routine.
  //
  // Returns pointer to chunk on success, NULL on failure
  
  MdvxChunk *createElevChunk();
  
  // create chunk object for radar azimuths
  //
  // Performs a 'new' to create the chunk.
  // Passes ownership of chunk object to calling routine.
  //
  // Returns pointer to chunk on success, NULL on failure
  
  MdvxChunk *createAzChunk();
  
  // create chunk object for variable radar elevations
  //
  // Performs a 'new' to create the chunk.
  // Passes ownership of chunk object to calling routine.
  //
  // Returns pointer to chunk on success, NULL on failure
  
  MdvxChunk *createVarElevChunk();
  
  // print

  void print(ostream &out) const;
  void printVarElev(ostream &out) const;

  // access to data members
  
  bool radarParamsAvail() const { return (_paramsAvail); }
  bool radarCalibAvail() const { return (_calibAvail); }
  bool radarElevAvail() const { return (_elevAvail); }
  bool radarAzAvail() const { return (_elevAvail); }

  DsRadarParams &getRadarParams() { return (_params); }
  DsRadarCalib &getRadarCalib() { return (_calib); }
  DsRadarElev &getRadarElev() { return (_elev); }
  DsRadarAz &getRadarAz() { return (_az); }

  const DsRadarParams &getRadarParams() const { return (_params); }
  const DsRadarCalib &getRadarCalib() const { return (_calib); }
  const DsRadarElev &getRadarElev() const { return (_elev); }
  const DsRadarAz &getRadarAz() const { return (_az); }

  int getNVarElev() const { return (_nVarElev); }
  fl32 *getVarElevs() { return ((fl32 *) _varElevBuf.getPtr()); }
  const fl32 *getVarElevs() const { return ((fl32 *) _varElevBuf.getPtr()); }

protected:

  bool _paramsAvail;
  bool _calibAvail;
  bool _elevAvail;
  bool _azAvail;
  bool _varElevAvail;

  DsRadarParams _params;
  DsRadarCalib _calib;
  DsRadarElev _elev;
  DsRadarAz _az;

  int _nVarElev;
  MemBuf _varElevBuf;

  void _loadFromChunk(MdvxChunk &chunk, int nfields);
  
private:

};

#endif



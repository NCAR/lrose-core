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

/////////////////////////////////////////////////////////////
// fieldDataMgr.hh
//
// Defines fieldDataMgr class
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#ifndef FIELD_DATA_H
#define FIELD_DATA_H

#include <vector>

#include <Mdv/DsMdvx.hh>

#include "Params.hh"

using namespace std;

class fieldDataMgr {
  
public:

  // Constructor, from filename.
  fieldDataMgr ( char *fileName, bool isVis, Params *TDRP_params );

  // See if the constructor went OK.
  bool isOk();

  // See if we have a north hemisphere, south hemisphere or both.
  bool northCovered();
  bool southCovered();

  // Get the data time.
  time_t getTime();

  // Load data.
  void loadData(double minLat, double minLon, 
		double maxLat, double maxLon, 
		double delLat, double delLon);

  // Get data loaded with call to loadData()
  fl32 *getData();

  // Get some specifics about the data.
  int getNx();
  int getNy();



  // Destructor.
  ~fieldDataMgr ();

  // public data

  const static fl32 badVal = -999.0;

protected:
  
private:

  //
  // Variables starting with 'field' pertain to data the
  // caller sees.
  //
  int _fieldNx, _fieldNy;
  fl32  *_fieldData;
  time_t _fieldTime;

  ui08 *_byteData;
  int _nx, _ny;
  bool _isVis;

  ui16 *_pixelData;


  //
  // Stuff for the lookup table.
  //
  typedef struct {
    int countVal;
    double physicalVal;
  } _convertPoint_t;

  vector <_convertPoint_t> _convertPoints;

  double _physicalArray[1024];

  bool _ok;

  bool _haveNorth;
  bool _haveSouth;

  double _subLon;
  double _coff, _loff, _cfac, _lfac;

  Params *_params;

  bool _thisFieldRequested;

};

#endif






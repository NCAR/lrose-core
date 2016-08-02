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
// Terrain.hh
//
// Terrain class - handles the terrain.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan  1999
//
///////////////////////////////////////////////////////////////

#ifndef Terrain_HH
#define Terrain_HH

#include <euclid/Pjg.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxFieldCode.hh>     
#include <Mdv/MdvxPjg.hh>

using namespace std;

class Terrain {
  
public:
  
  /*********************************************************************
   * Constructors
   */

  Terrain(const string &mdv_path, const string &field_name,
	  const Pjg &proj);

  // destructor
  //
  // Frees the memory that was used to hold the Terrain.

  ~Terrain();


  // Public data.


  fl32 *getTerrain(fl32 &terrain_bad_value)
  {
    if (!_readTerrainData())
      return 0;
    
    terrain_bad_value = _terrainField->getFieldHeader().bad_data_value;
    
    return (fl32 *)_terrainField->getVol();
  }
  
protected:
  
  string _mdvPath;
  string _fieldName;
  
  MdvxPjg _proj;
  
  bool _terrainGridRead;
  
  MdvxField *_terrainField;
  

  /*********************************************************************
   * _readTerrainData() - Read the terrain data from the MDV file.
   *
   * Returns true on success, false on failure.
   */

  bool _readTerrainData();
  
};

#endif




















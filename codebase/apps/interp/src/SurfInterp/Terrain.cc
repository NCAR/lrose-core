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
// Terrain.cc
//
// Terrain class - handles the terrain.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan  1999
//
///////////////////////////////////////////////////////////////
//
//

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxFieldCode.hh>  

#include <toolsa/mem.h>

#include "Terrain.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

Terrain::Terrain(const string &mdv_path, const string &field_name,
		 const Pjg &proj) :
  _mdvPath(mdv_path),
  _fieldName(field_name),
  _proj(proj),
  _terrainGridRead(false),
  _terrainField(0)
{
}


/*********************************************************************
 * Destructor
 */

Terrain::~Terrain()
{
  delete _terrainField;
}


/*********************************************************************
 * _readTerrainData() - Read the terrain data from the MDV file.
 *
 * Returns true on success, false on failure.
 */

bool Terrain::_readTerrainData()
{
  // If we've already read the data, we don't need to read it again

  if (_terrainGridRead)
    return true;
  
  // Set up the read request

  DsMdvx mdvx;

  mdvx.setReadPath(_mdvPath); 

  mdvx.setReadRemap(_proj);
  
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_fieldName[0] == '#')
    mdvx.addReadField(atoi(_fieldName.substr(1).c_str()));
  else
    mdvx.addReadField(_fieldName);
  
  // Read the data

  if (mdvx.readVolume()){
    cerr << "Failed to read terrain file " << _mdvPath << endl;
    
    return false;
  }

  // The first field in the file is the terrain field we are using

  MdvxField *field = mdvx.getFieldByNum(0);
  _terrainField = new MdvxField(*field);
  
  // If the bad and missing data values differ, we need to make them
  // the same

  Mdvx::field_header_t field_hdr = _terrainField->getFieldHeader();
  
  if (field_hdr.bad_data_value != field_hdr.missing_data_value)
  {
    fl32 *data = (fl32 *)_terrainField->getVol();
    int vol_size = field_hdr.nx * field_hdr.ny * field_hdr.nz;
    
    for (int i = 0; i < vol_size; ++i)
      if (data[i] == field_hdr.missing_data_value)
	data[i] = field_hdr.bad_data_value;
    
    field_hdr.missing_data_value = field_hdr.bad_data_value;
    
    _terrainField->setFieldHeader(field_hdr);
  }
  
  _terrainGridRead = true;
  
  return true;

}

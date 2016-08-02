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
/**
 *
 * @file TerrainMask.cc
 *
 * @class TerrainMask
 *
 * TerrainMask creates a terrain mask for the current radar dataset.
 *  
 * @date 7/22/2002
 *
 */

#include <string>

#include <dataport/port_types.h>
#include <rapmath/math_macros.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>

#include "TerrainMask.hh"
#include "ApRemoval.hh"

//
// Constants
//
const float TerrainMask::FUZZY_VALUE = 0.5;
const float TerrainMask::LAND_VALUE  = 1.0;
const float TerrainMask::WATER_VALUE = 0.0;
const float TerrainMask::MISSING_VALUE = -1.0;

/**
 * Constructor
 */

TerrainMask::TerrainMask() :
  _terrainMask(0),
  _nGates(0),
  _nBeams(0),
  _gateSpacing(0.0),
  _deltaAzimuth(0.0),
  _radarLat(0.0),
  _radarLon(0.0)
{
}


/**
 * Destructor
 */

TerrainMask::~TerrainMask() 
{
  delete [] _terrainMask;
}


/**
 * init()
 */

int TerrainMask::init(const char* terrain_url, const char* terrain_field_name,
		      const int max_num_gates, const double gate_spacing,
		      const double delta_az, 
		      const double lat, const double lon)
{
  // Set geometry stuff

  _nGates = max_num_gates;
  _gateSpacing = gate_spacing;
  _deltaAzimuth = delta_az;
  _nBeams = (int)(360.0 / _deltaAzimuth + 0.5);
  _radarLat = lat;
  _radarLon = lon;
   
  // Set up data arrays

  _terrainMask = new fl32[_nBeams * _nGates];
  fl32 *nearest_nbr = new fl32[_nBeams * _nGates];

  // Read the mdv file

  _inputMdvx.clearRead();
  _inputMdvx.setReadPath(terrain_url);
  _inputMdvx.addReadField(terrain_field_name);
  _inputMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _inputMdvx.setReadScalingType(Mdvx::SCALING_NONE);
  _inputMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (_inputMdvx.readVolume() != 0)
  {
    POSTMSG(ERROR, "Could not read mdv file");
    cerr << _inputMdvx.getErrStr() << endl;
    delete [] nearest_nbr;
    return -1;
  }

  // Get the master header

  const Mdvx::master_header_t &masterHeader = _inputMdvx.getMasterHeader();

  // Get the field from the mdv file

  MdvxField *mdv_field = _inputMdvx.getField(0);

  const Mdvx::field_header_t &field_hdr = mdv_field->getFieldHeader();

  if (field_hdr.nz != 1)
  {
    POSTMSG(ERROR, "Wrong number of planes in terrain data");
    delete [] nearest_nbr;
    return -1;
  }

  // Get a projection object

  MdvxPjg proj(masterHeader, field_hdr);

  // Get the data

  fl32 *elev_data = (fl32 *)mdv_field->getVol();

  // Set all gates to their closest neighbor value

  for (int ibeam = 0; ibeam < _nBeams; ++ibeam)
  {
    // Get the angle for this beam

    double theta = ibeam * _deltaAzimuth;

    for (int igate = 0; igate < _nGates; ++igate)
    {
      // Calculate the lat/lon for the center of this gate

      double gate_lat, gate_lon;
	
      Pjg::latlonPlusRTheta(_radarLat, _radarLon,
			    igate * _gateSpacing, theta,
			    gate_lat, gate_lon);
	
      // Get the x and y indices for this lat/lon location

      int gate_x_index, gate_y_index;
      int terrain_index = ibeam * _nGates + igate;
       
      if (proj.latlon2xyIndex(gate_lat, gate_lon,
			      gate_x_index, gate_y_index) < 0)
      {
	nearest_nbr[terrain_index] = MISSING_VALUE;
      }
      else
      {
	float elev = elev_data[gate_y_index * field_hdr.nx + gate_x_index];
	
	nearest_nbr[terrain_index] = (elev == 0.0) ? WATER_VALUE : LAND_VALUE;
       }
       
    } /* endfor - igate */
  } /* endfor - ibeam */

  // Now set any gate with a different value on any side to a
  // fuzzy value

  for (int ibeam = 0; ibeam < _nBeams; ++ibeam)
  {
    for (int igate = 0; igate < _nGates; ++igate)
    {
      fl32 curr_val = nearest_nbr[ibeam * _nGates + igate];

      if (curr_val == MISSING_VALUE)
	continue;
	 
      // If we are not on the zeroth gate, check the previous
      // gate, same beam

      if (igate != 0)
      {
	if (curr_val != nearest_nbr[ibeam * _nGates + igate - 1] &&
	    nearest_nbr[ibeam * _nGates + igate - 1] != MISSING_VALUE)
	{
	  _terrainMask[ibeam * _nGates + igate] = FUZZY_VALUE;
	  continue;
	}
      }
         
      // If we are not on the nth gate, check the next gate, same
      // beam

      if (igate != _nGates - 1)
      {
	if (curr_val != nearest_nbr[ibeam * _nGates + igate + 1] &&
	    nearest_nbr[ibeam * _nGates + igate + 1] != MISSING_VALUE)
	{
	  _terrainMask[ibeam * _nGates + igate] = FUZZY_VALUE;
	  continue;
	}
      }

      // Check the same gate in the previous beam.  If we are on the
      // zeroth beam, check the nth beam, which will be just to the left
      // of the zeroth beam in polar space.

      if (ibeam == 0)
      {
	if (curr_val != nearest_nbr[(_nBeams-1) * _nGates + igate] &&
	    nearest_nbr[(_nBeams-1) * _nGates + igate] != MISSING_VALUE)
	{
	  _terrainMask[ibeam * _nGates + igate] = FUZZY_VALUE;
	  continue;
	}
      }
      else
      {
	if (curr_val != nearest_nbr[(ibeam-1) * _nGates + igate] &&
	    nearest_nbr[(ibeam-1) * _nGates + igate] != MISSING_VALUE)
	{
	  _terrainMask[ibeam * _nGates + igate] = FUZZY_VALUE;
	  continue;
	}
      }
         
      // Check the same gate in the next beam.  If we are on the nth beam,
      // check the zeroth beam which will be just to the right of the
      // current beam in polar space.  

      if (ibeam == _nBeams - 1)
      {
	if (curr_val != nearest_nbr[igate] &&
	    nearest_nbr[igate] != MISSING_VALUE)
	{
	  _terrainMask[ibeam * _nGates + igate] = FUZZY_VALUE;
	  continue;
	}
      }
      else
      {
	if (curr_val != nearest_nbr[(ibeam+1) * _nGates + igate] &&
	    nearest_nbr[(ibeam+1) * _nGates + igate] != MISSING_VALUE)
	{
	  _terrainMask[ibeam * _nGates + igate] = FUZZY_VALUE;
	  continue;
	}
      }

      // If none of the above gates were different from the
      // current one, set it to the same as the nearest nbr
      // array at this gate

      _terrainMask[ibeam * _nGates + igate] =
	nearest_nbr[ibeam * _nGates + igate];
    }
  }

  delete [] nearest_nbr;
  return 0;
}
      

/**
 * writeMask()
 */

void TerrainMask::writeMask(const char* file_path) const
{
  // Create new master header

  Mdvx::master_header_t master_hdr = _inputMdvx.getMasterHeader();

  time_t curr_time = time(0);
   
  master_hdr.time_gen = curr_time;
  master_hdr.time_begin = curr_time;
  master_hdr.time_end = curr_time;
  master_hdr.time_centroid = curr_time;
  master_hdr.time_expire = curr_time;
  master_hdr.data_collection_type = Mdvx::DATA_SYNTHESIS;
  master_hdr.n_fields = 1;
  master_hdr.max_nx = _nGates;
  master_hdr.max_ny = _nBeams;
  master_hdr.sensor_lat = _radarLat;
  master_hdr.sensor_lon = _radarLon;
  
  strcpy(master_hdr.data_set_info, "Terrain Mask");
  strcpy(master_hdr.data_set_name, "Terrain Mask");
  strcpy(master_hdr.data_set_source,"ApRemoval");

  // Set the master header in the mdvx object

  DsMdvx output_mdvx;
  output_mdvx.setMasterHeader(master_hdr);

  // Create a new mdvx field header

  MdvxField *mdv_field = _inputMdvx.getField(0);

  Mdvx::field_header_t field_hdr = mdv_field->getFieldHeader();
   
  field_hdr.nx = _nGates;
  field_hdr.ny = _nBeams;
  field_hdr.proj_type = Mdvx::PROJ_POLAR_RADAR;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.proj_origin_lat = _radarLat;
  field_hdr.proj_origin_lon = _radarLon;
  field_hdr.grid_dx = _gateSpacing;
  field_hdr.grid_dy = _deltaAzimuth;
  field_hdr.grid_minx = 0.0;
  field_hdr.grid_miny = 0.0;
  field_hdr.bad_data_value = -1.0;
  field_hdr.missing_data_value = -1.0;
   
  strcpy(field_hdr.field_name_long, "Mask");
  strcpy(field_hdr.field_name, "Mask");
  strcpy(field_hdr.units, "none");

  // Create a new mdvx field
  //   Note that we do not need to delete it because it will
  //   be owned by the mdvx object once we add it

  MdvxField *mask_field = new MdvxField(field_hdr, 
					mdv_field->getVlevelHeader(),
					(void *)_terrainMask,
					false,
					true);

  // Scale, bias and compress data

  mask_field->convertRounded(Mdvx::ENCODING_INT8,
			     Mdvx::COMPRESSION_RLE);
   
  // Add the field to the output mdvx object

  output_mdvx.addField(mask_field);
   
  // Write the file

  output_mdvx.clearWrite();
  if (output_mdvx.writeToPath(file_path) != 0)
    POSTMSG(ERROR, "Could not write out terrain data");
}


   
   
   

   
   
   

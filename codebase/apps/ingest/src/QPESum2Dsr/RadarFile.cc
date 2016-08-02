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
 * @file RadarFile.cc
 *
 * @class RadarFile
 *
 * Class controlling access to a DeTect radar file.
 *  
 * @date 7/27/2010
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <toolsa/file_io.h>

#include "RadarFile.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

RadarFile::RadarFile(const string &input_path,
		       const bool debug) :
  _debug(debug),
  _inputPath(input_path),
  _inputFile(0),
  _tiltData(0)
{
}


/*********************************************************************
 * Destructor
 */

RadarFile::~RadarFile() 
{
  if (_inputFile != 0)
    fclose(_inputFile);

  delete [] _tiltData;
}


/*********************************************************************
 * getRay()
 */

bool RadarFile::getRay(const size_t ray_index,
		       double &azimuth,
		       double &elev_angle,
		       DateTime &data_time,
		       vector< double > &ray_data)
{
  static const string method_name = "RadarFile::getRay()";
  
  if (ray_index < 0 || ray_index >= _numRays)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Invalid ray index: " << ray_index << endl;
    
    return false;
  }
  
  // azimuth

  azimuth = _azim1 + (ray_index * _azimSpacing);
  if (azimuth < 0.0)
    azimuth += 360.0;
  if (azimuth >= 360.0)
    azimuth -= 360.0;
  
  // elevation angle

  elev_angle = _elevAngle;
  
  // data time

  data_time = _dataTime;
  
  // ray data

  ray_data.clear();

  for (size_t i = 0; i < _numGates; ++i)
  {
    size_t data_index = (ray_index * _numGates) + i;
    
    ray_data.push_back(_tiltData[data_index]);
  } /* endfor - i */
  
  return true;
}


/*********************************************************************
 * print()
 */

void RadarFile::print(ostream &stream, const string &leader) const
{
  stream << leader << "Radar name = <" << _radarName << ">" << endl;
  stream << leader << "Height = " << _height << endl;
  stream << leader << "Latitude = " << _lat << endl;
  stream << leader << "Longitude = " << _lon << endl;
  stream << leader << "Data time = " << _dataTime << endl;
  stream << leader << "Volume number = " << _volNum << endl;
  stream << leader << "VCP mode = " << _vcpMode << endl;
  stream << leader << "Tilt number = " << _tiltNum << endl;
  stream << leader << "Elevation angle = " << _elevAngle << endl;
  stream << leader << "Number of rays = " << _numRays << endl;
  stream << leader << "Number of gates = " << _numGates << endl;
  stream << leader << "Start azimuth = " << _azim1 << endl;
  stream << leader << "Azimuth spacing = " << _azimSpacing << endl;
  stream << leader << "Start gate = " << _gate1 << endl;
  stream << leader << "Gate spacing = " << _gateSpacing << endl;
  stream << leader << "Missing data value = " << _missingDataValue << endl;
}


/*********************************************************************
 * readFile()
 */

bool RadarFile::readFile()
{
  static const string method_name = "RadarFile::readFile()";
  
  // Open the file

  if ((_inputFile = ta_fopen_uncompress(_inputPath.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening file" << endl;
    perror(_inputPath.c_str());
    
    return false;
  }
  
  // Read the radar header.

  radar_header_t radar_hdr;
  
  if (ta_fread(&radar_hdr, sizeof(radar_hdr), 1, _inputFile) != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading radar header from input file" << endl;
    perror(_inputPath.c_str());
    
    return false;
  }
  
  // Extract the information from the radar header.  The header starts
  // with 4 integers that represent the radar name.  Each integer must be
  // interpreted as a single character value.  After that we get a header
  // scale value that is used for all of the following header values.

  char radar_name_chars[5];
  
  radar_name_chars[0] = (char)radar_hdr.radar_name[0];
  radar_name_chars[1] = (char)radar_hdr.radar_name[1];
  radar_name_chars[2] = (char)radar_hdr.radar_name[2];
  radar_name_chars[3] = (char)radar_hdr.radar_name[3];
  radar_name_chars[4]='\0';

  _radarName = radar_name_chars;
  
  _height = (double)radar_hdr.height / (double)radar_hdr.header_scale;
  _lat = (double)radar_hdr.lat / (double)radar_hdr.header_scale;
  _lon = (double)radar_hdr.lon / (double)radar_hdr.header_scale;
  _dataTime.set(radar_hdr.year / radar_hdr.header_scale,
		radar_hdr.month / radar_hdr.header_scale,
		radar_hdr.day / radar_hdr.header_scale,
		radar_hdr.hour / radar_hdr.header_scale,
		radar_hdr.minute / radar_hdr.header_scale,
		radar_hdr.second / radar_hdr.header_scale);
  _volNum = radar_hdr.vol_num / radar_hdr.header_scale;
  _vcpMode = radar_hdr.vcp_mode / radar_hdr.header_scale;
  _tiltNum = radar_hdr.tilt_num / radar_hdr.header_scale;
  _elevAngle = (double)radar_hdr.elev_angle / (double)radar_hdr.header_scale;
  _numRays = radar_hdr.num_rays / radar_hdr.header_scale;
  _numGates = radar_hdr.num_gates / radar_hdr.header_scale;
  _azim1 = (double)radar_hdr.azim1 / (double)radar_hdr.header_scale;
  _azimSpacing =
    (double)radar_hdr.azim_spacing / (double)radar_hdr.header_scale;
  _gate1 = (double)radar_hdr.gate1 / (double)radar_hdr.header_scale;
  _gateSpacing =
    (double)radar_hdr.gate_spacing / (double)radar_hdr.header_scale;
  _missingDataValue = radar_hdr.missing_data_value / radar_hdr.header_scale;

  double data_scale =
    (double)radar_hdr.data_scale / (double)radar_hdr.header_scale;

  // Now read the data

  int num_data_values = _numGates * _numRays;

  delete [] _tiltData;
  _tiltData = new double[num_data_values];
  
  si32 *read_buffer = new si32[num_data_values];

  if (ta_fread(read_buffer, sizeof(si32), num_data_values, _inputFile)
      != num_data_values)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading data from input file" << endl;
    perror(_inputPath.c_str());
    
    delete [] read_buffer;
    
    return false;
  }
  
  size_t i = 0;
  
  for (size_t ray = 0; ray < _numRays; ++ray)
  {
    for (size_t gate = 0; gate < _numGates; ++gate, ++i)
    {
      _tiltData[i] = (double)read_buffer[i] / data_scale;
    } /* endfor - gate */
  } /* endfor - ray */

  delete [] read_buffer;

  return true;
}

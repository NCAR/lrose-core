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
////////////////////////////////////////////////////////////////////////
// Beam - Class representing a single beam to be added to the sweep file.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>

#include "Beam.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

Beam::Beam(const DsRadarMsg &radar_msg,
           bool debug_flag,
           bool verbose_flag) :
        _debug(debug_flag),
        _verbose(verbose_flag)
{
  DsBeamHdr_t *beam_hdr = radar_msg.getRadarBeam().getBeamHdr();
  
  _time = beam_hdr->time;
  _azimuth = beam_hdr->azimuth;
  _elevation = beam_hdr->elevation;
  _targetAzimuth = beam_hdr->target_az;
  _targetElevation = beam_hdr->target_elev;
  _volNum = beam_hdr->vol_num;
  _tiltNum = beam_hdr->tilt_num;
  _antennaTransition = beam_hdr->antenna_transition;
}


/*********************************************************************
 * Destructor
 */

Beam::~Beam()
{
  // Reclaim the field data

  vector< RayDoubles* >::iterator field_data;
  for (field_data = _fieldData.begin(); field_data != _fieldData.end();
       ++field_data)
    delete *field_data;
}


/*********************************************************************
 * addBeamToFile() - Add this beam to the given sweep file.
 */

void Beam::addBeamToFile(RayFile *ray_file)
{
  static const string method_name = "Beam::addBeamToFile()";
  
  PMU_auto_register("Adding beam to file");
  
  try
  {
    if (_verbose)
      cerr << "About to set general beam info in file" << endl;
    
    ray_file->set_time("ray_time", _time);
    ray_file->set_angle("ray_azimuth", _azimuth);
    ray_file->set_angle("ray_elevation", _elevation);
    ray_file->set_double("ray_true_scan_rate", -9999.0);
    if (_antennaTransition) {
      ray_file->set_integer("ray_status", 1);
    } else {
      ray_file->set_integer("ray_status", 0);
    }

    if (_verbose)
    {
      cerr << "Successfully set general beam info in file" << endl;
      cerr << "    azimuth = " << _azimuth << endl;
      cerr << "    elevation = " << _elevation << endl;
    }
    
    for (size_t i = 0; i < _fieldData.size(); ++i)
    {
      PMU_auto_register("Setting field beam data");
      
      RayDoubles *field_data = _fieldData[i];

      if (_verbose)
      {
	cerr << "About to set beam data for field " << i << endl;

	cerr << "field_data = " << field_data << endl;
	cerr << "field_data->size() = " << field_data->size() << endl;
      }
      
      ray_file->set_ray_data(i, *field_data);

      if (_verbose)
	cerr << "Successfully set beam data for field " << i << endl;
    } /* endfor - i */
  }
  catch (ForayUtility::Fault &fault)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << fault.msg() << endl;
  }

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

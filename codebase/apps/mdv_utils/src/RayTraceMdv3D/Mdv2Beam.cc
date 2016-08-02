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
// Reads MDV, makes beams.
//
// Niles Oien, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2005
//
/////////////////////////////////////////////////////////////

#include <cmath>
#include <ctime>
#include <physics/physics.h>
#include <toolsa/pjg_flat.h>

#include "Mdv2Beam.hh"

using namespace std;

//
// Constructor - reads the MDV.
//
Mdv2Beam::Mdv2Beam(time_t dataTime, Params *P){

  _OK = 1;
  //
  // Copy pointer to params.
  //
  _params = P;

  //
  // Read the MDV data.
  //
  for (int k=0; k < _params->radials_n; k++){
    _mdvMgr.addReadField( _params->_radials[k].uNameMdv );
    _mdvMgr.addReadField( _params->_radials[k].vNameMdv );

    if (_params->Debug){
      cerr << "Adding radial pair " << _params->_radials[k].uNameMdv;
      cerr << ", " << _params->_radials[k].vNameMdv << " to read list." << endl;
    }

  }

  for (int k=0; k < _params->fields_n; k++){
    _mdvMgr.addReadField( _params->_fields[k].nameMdv );

    if (_params->Debug){
      cerr << "Adding field " << _params->_fields[k].nameMdv;
      cerr  << " to read list." << endl;
    }

  }

  _mdvMgr.setReadTime(Mdvx::READ_FIRST_BEFORE, _params->TriggerUrl, 0, dataTime);
  _mdvMgr.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _mdvMgr.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (_mdvMgr.readVolume()){
    cerr << "Read failed at " << utimstr(dataTime) << " from ";
    cerr << _params->TriggerUrl  << endl;
    _OK = 0;
  }

  return;

}
  
//
// Get the number of gates.
//
int Mdv2Beam::getNumGates(int tiltNum){
  //
  // Calculate the number of gates.
  //
  double dist = _params->_tilts[tiltNum].tiltLastGateRange - _params->_tilts[tiltNum].tiltFirstGateRange;
  int nGates = (int) ceil(dist/_params->sensorGeometry.sensorRangeSpacing);

  return nGates;

}


//
// Get a beam at an azimuth, tilt number
//
void Mdv2Beam::getBeam(double az, int tiltNum, int numFields, int numGates, fl32 *beamData) {

  // Initialize to all bad/missing.

  for (int k=0; k < numFields * numGates; k++){
    beamData[k] = Mdv2Beam::badVal;
  }

  //
  // If the MDV read failed, return
  //
  if (!(_OK)) return;


  int fieldNum = 0; // Handy count of which field number we are processing


  //
  // Loop through the radials.
  //
  for (int iradial=0; iradial < _params->radials_n; iradial++){

      MdvxField *ufield = _mdvMgr.getFieldByName( _params->_radials[iradial].uNameMdv );
      MdvxField *vfield = _mdvMgr.getFieldByName( _params->_radials[iradial].vNameMdv );

      if ((ufield == NULL) || (vfield == NULL)) return;

      Mdvx::field_header_t ufhdr = ufield->getFieldHeader();
      Mdvx::field_header_t vfhdr = vfield->getFieldHeader();

      // Get the U vlevel header. Exit if it is not of correct type.
      // Assume U and V are the same. Set the minimum and
      // maximum heights.
      Mdvx::vlevel_header_t uvhdr = ufield->getVlevelHeader();
      if (uvhdr.type[0] != Mdvx::VERT_TYPE_Z){
	cerr << "Vert type is not Km above MSL, I cannot cope." << endl;
	exit(-1);
      }

      double maxHeight= -1.0;
      double minHeight=  1.0;

      if (ufhdr.nz > 1){
	maxHeight = uvhdr.level[ufhdr.nz-1] + _params->vertSlop;
	minHeight = uvhdr.level[0] - _params->vertSlop;
      }

      Mdvx::master_header_t mhdr = _mdvMgr.getMasterHeader();
      MdvxProj Proj(mhdr, ufhdr); // Assume u,v fields are on same projection.

      fl32 *udata = (fl32 *)ufield->getVol();
      fl32 *vdata = (fl32 *)vfield->getVol();

      //
      // Loop over the gates.
      //
      for (int igate=0; igate < numGates; igate++){
	//
	// Get the lat/lon of this point.
	//
	double plat, plon;
	double radialDistKm = (_params->_tilts[tiltNum].tiltFirstGateRange + igate * _params->sensorGeometry.sensorRangeSpacing)/1000.0;

	// Get height in Km relative to sensor, then add sensor height
	double heightKm = radialDistKm * sin(3.1415927*_params->_tilts[tiltNum].tiltElevation/180.0);
	heightKm += _params->sensorGeometry.sensorAlt/1000.0; // m to Km

	if (
	    (maxHeight > minHeight) &&
	    ((heightKm < minHeight) || (heightKm > maxHeight))
	    ){
	  continue; // This point is too high or too low.
	}

	// Correct horizontal distance for elevation angle.
	radialDistKm *= cos(3.1415927*_params->_tilts[tiltNum].tiltElevation/180.0);

	PJGLatLonPlusRTheta(_params->sensorGeometry.sensorLat,
			    _params->sensorGeometry.sensorLon,
			    radialDistKm,
			    az, &plat, &plon);
	//
	// Get the x,y indicies of this point.
	//
	int ix, iy;
	if (!(Proj.latlon2xyIndex(plat, plon, ix, iy))){

	  // Get iz

	  int iz = 0;
	  double minDiff = 0.0;

	  for (int izi = 0; izi < ufhdr.nz; izi++){
	    if (izi == 0){
	      minDiff = fabs(uvhdr.level[izi] - heightKm);
	    } else {
	      if (fabs(uvhdr.level[izi] - heightKm) < minDiff){
		minDiff = fabs(uvhdr.level[izi] - heightKm);
		iz = izi;
	      }
	    }
	  }

	  if (
	      (udata[iz*ufhdr.ny*ufhdr.nx + iy*ufhdr.nx + ix] == ufhdr.bad_data_value) ||
	      (udata[iz*ufhdr.ny*ufhdr.nx + iy*ufhdr.nx + ix] == ufhdr.missing_data_value) ||
	      (vdata[iz*vfhdr.ny*vfhdr.nx + iy*vfhdr.nx + ix] == vfhdr.bad_data_value) ||
	      (vdata[iz*vfhdr.ny*vfhdr.nx + iy*vfhdr.nx + ix] == vfhdr.missing_data_value)
	      ){
	    // hit missing data, may not want to continue filling beam.
	    if (_params->stopOnMissing){
	      break;
	    }
	  } else {
	
	    // OK, have U,V, geometry - figure out radial wind. Go from U,V to
	    // speed, direction.

	    double wSpd = PHYwind_speed( udata[iz*ufhdr.ny*ufhdr.nx + iy*ufhdr.nx + ix],
					 vdata[iz*ufhdr.ny*ufhdr.nx + iy*ufhdr.nx + ix]);
	    wSpd *= _params->_radials[iradial].scaleFactor;

	    double wDir = PHYwind_dir( udata[iz*ufhdr.ny*ufhdr.nx + iy*ufhdr.nx + ix],
				       vdata[iz*ufhdr.ny*ufhdr.nx + iy*ufhdr.nx + ix]);
	    if (wDir < 0.0) wDir += 360.0;

	    double angleDiffDeg = wDir - az;

	    double radialVal = wSpd * cos(3.1415927*angleDiffDeg/180.0);
	    radialVal *= cos(3.1415927*_params->_tilts[tiltNum].tiltElevation/180.0);

	    beamData[igate * numFields + fieldNum] = radialVal;
	  }
	}
      } // End of loop through gates
      fieldNum++;
  } // End of loop though radials



  // The loop through radial extractions is done.
  // Now loop through direct field extractions.

  for (int ifield=0; ifield < _params->fields_n; ifield++){

      MdvxField *field = _mdvMgr.getFieldByName( _params->_fields[ifield].nameMdv );

      if (field == NULL) return;

      Mdvx::field_header_t fhdr = field->getFieldHeader();

      // Get the U vlevel header. Exit if it is not of correct type.
      // Assume U and V are the same. Set the minimum and
      // maximum heights.
      Mdvx::vlevel_header_t vhdr = field->getVlevelHeader();
      if (vhdr.type[0] != Mdvx::VERT_TYPE_Z){
	cerr << "Vert type is not Km above MSL, I cannot cope." << endl;
	exit(-1);
      }

      double maxHeight= -1.0;
      double minHeight=  1.0;

      if (fhdr.nz > 1){
	maxHeight = vhdr.level[fhdr.nz-1] + _params->vertSlop;
	minHeight = vhdr.level[0] - _params->vertSlop;
      }

      Mdvx::master_header_t mhdr = _mdvMgr.getMasterHeader();
      MdvxProj Proj(mhdr, fhdr);

      fl32 *data = (fl32 *)field->getVol();

      //
      // Loop over the gates.
      //
      for (int igate=0; igate < numGates; igate++){
	//
	// Get the lat/lon of this point.
	//
	double plat, plon;
	double radialDistKm = (_params->_tilts[tiltNum].tiltFirstGateRange + igate * _params->sensorGeometry.sensorRangeSpacing)/1000.0;

	// Get height in Km relative to sensor, then add sensor height
	double heightKm = radialDistKm * sin(3.1415927*_params->_tilts[tiltNum].tiltElevation/180.0);
	heightKm += _params->sensorGeometry.sensorAlt/1000.0; // m to Km



	if (
	    (maxHeight > minHeight) &&
	    ((heightKm < minHeight) || (heightKm > maxHeight))
	    ){
	  continue; // This point is too high or too low.
	}

	// Correct horizontal distance for elevation angle.
	radialDistKm *= cos(3.1415927*_params->_tilts[tiltNum].tiltElevation/180.0);

	PJGLatLonPlusRTheta(_params->sensorGeometry.sensorLat,
			    _params->sensorGeometry.sensorLon,
			    radialDistKm,
			    az, &plat, &plon);
	//
	// Get the x,y indicies of this point.
	//
	int ix, iy;
	if (!(Proj.latlon2xyIndex(plat, plon, ix, iy))){

	  // Get iz

	  int iz = 0;
	  double minDiff = 0.0;

	  for (int izi = 0; izi < fhdr.nz; izi++){
	    if (izi == 0){
	      minDiff = fabs(vhdr.level[izi] - heightKm);
	    } else {
	      if (fabs(vhdr.level[izi] - heightKm) < minDiff){
		minDiff = fabs(vhdr.level[izi] - heightKm);
		iz = izi;
	      }
	    }
	  }

	  if (
	      (data[iz*fhdr.ny*fhdr.nx + iy*fhdr.nx + ix] == fhdr.bad_data_value) ||
	      (data[iz*fhdr.ny*fhdr.nx + iy*fhdr.nx + ix] == fhdr.missing_data_value)
	      ){
	    // hit missing data, may not want to continue filling beam.
	    if (_params->stopOnMissing){
	      break;
	    }
	  } else {
	    // All we have to do here is pick off the value and apply our scale factor
	    beamData[igate * numFields + fieldNum] = data[iz*fhdr.ny*fhdr.nx + iy*fhdr.nx + ix] * _params->_fields[ifield].scaleFactor;
	  }
	}
      } // End of loop through gates
      fieldNum++;
  } // End of loop though fields

  return;

}




//
// Desructor -does nothing.
//
Mdv2Beam::~Mdv2Beam(){
  return;
}


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

#include <toolsa/pjg_flat.h>

#include "Mdv2Beam.hh"

using namespace std;

//
// Constructor - reads the MDV, calculates _numR
//
Mdv2Beam::Mdv2Beam(time_t dataTime,
		   string url,
		   string fieldName,
		   double r0,
		   double dR,
		   double Rmax,
		   double lidarLat,
		   double lidarLon){

  _OK = 1;
  //
  // Just make copies of some things.
  //
  _lidarLon = lidarLon; _lidarLat = lidarLat;
  _r0 = r0; _dR = dR; _Rmax = Rmax;

  //
  // Set _subSampleDr to the bad value, thereby
  // indicating that we are not doing subsampling.
  // Invoking the setSubSample() method will do this.
  //
  _subSampleDr = badVal;
  //
  // Calculate the number of gates.
  //
  _numR = (int) ceil((Rmax - r0)/dR);

  //
  // Read the MDV data.
  //
  _mdvMgr.addReadField( fieldName );
  _mdvMgr.setReadTime(Mdvx::READ_FIRST_BEFORE, url, 0, dataTime);
  _mdvMgr.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _mdvMgr.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (_mdvMgr.readVolume()){
    cerr << "Read failed at " << utimstr(dataTime) << " from ";
    cerr << url  << endl;
    _OK = 0;
  }

  return;

}

//
// Invoke subsampling.
//
void Mdv2Beam::setSubSample(double subSampleDr){
  _subSampleDr = subSampleDr;
  return;
}
  
//
// Get the number of gates.
//
int Mdv2Beam::getNumGates(){
  return _numR;
}


//
// Get a beam at an azimuth
//
vector<double> Mdv2Beam::getBeam(double az) {

  // cerr << "Beam requested at azimuth " << az << endl;

  _beamData.clear();
  //
  // If the MDV read failed, return a vector of all bad/missing data.
  //
  if (!(_OK)){
    for (int i=0; i < _numR; i++){
      _beamData.push_back( Mdv2Beam::badVal );
    }
    return _beamData;
  }
  //
  // Otherwise loop through, getting the values.
  // Need to set up a proj object.
  //
  MdvxField *InField = _mdvMgr.getFieldByNum( 0 );

  if (InField == NULL){
    for (int i=0; i < _numR; i++){
      _beamData.push_back( Mdv2Beam::badVal );
    }
    return _beamData;
  }

  Mdvx::field_header_t InFhdr = InField->getFieldHeader();
  Mdvx::master_header_t InMhdr = _mdvMgr.getMasterHeader();

  MdvxProj Proj(InMhdr, InFhdr);

  fl32 *InData = (fl32 *) InField->getVol();

  if (_subSampleDr == badVal){
    //
    // No subsampling.
    //

    for (int i=0; i < _numR; i++){
      //
      // Get the lat/lon of this point.
      //
      double plat, plon;
      PJGLatLonPlusRTheta(_lidarLat, _lidarLon,
			  (_r0 + i * _dR)/1000.0, az,
			  &plat, &plon);
      //
      // Get the x,y indicies of this point.
      //
      int ix, iy;
      if (Proj.latlon2xyIndex(plat, plon, ix, iy)){
	//
	// Outside of grid, push back the bad value.
	//
	_beamData.push_back( Mdv2Beam::badVal );
      } else {
	if (
	    (InData[iy*InFhdr.nx + ix] == InFhdr.bad_data_value) ||
	    (InData[iy*InFhdr.nx + ix] == InFhdr.missing_data_value)
	    ){
	  //
	  // Inside the grid but the data are bad/missing.
	  // Push back the bad value.
	  //
	  _beamData.push_back( Mdv2Beam::badVal );
	} else {
	  //
	  // Push back the data at that grid point.
	  //
	  _beamData.push_back( InData[iy*InFhdr.nx + ix] );
	}
      }
    }

  } else {
    //
    // We are doing subsampling - similar, but different, there
    // is one more nested loop.
    //

    for (int i=0; i < _numR; i++){

      //
      //
      double total = 0.0;
      int numToAvg = 0;
      double startRange = ((_r0 + i * _dR) - _dR/2.0)/1000.0;
      double endRange =   ((_r0 + i * _dR) + _dR/2.0)/1000.0;

      int sscount = 0;
      double range = 0.0;

      do {

	range = startRange + double(sscount)*_subSampleDr/1000.0;
	sscount++;

	// cerr << " At range " << range << endl;

	//
	// Get the lat/lon of this point.
	//
	double plat, plon;
	PJGLatLonPlusRTheta(_lidarLat, _lidarLon,
			    range, az,
			    &plat, &plon);
	//
	// Get the x,y indicies of this point.
	//
	int ix, iy;
	if (Proj.latlon2xyIndex(plat, plon, ix, iy)){
	  //
	  // Outside of grid, continue.
	  //
	  continue;
	  //
	} else {
	  if (
	      (InData[iy*InFhdr.nx + ix] == InFhdr.bad_data_value) ||
	      (InData[iy*InFhdr.nx + ix] == InFhdr.missing_data_value)
	      ){
	    //
	    // Inside the grid but the data are bad/missing.
	    // Continue.
	    //
	    continue;
	  } else {
	    //
	    // Add to the total.
	    //
	    total += InData[iy*InFhdr.nx + ix];
	    numToAvg++;
	  }
	}
      } while (range <= endRange);

      if (numToAvg == 0){
	 _beamData.push_back( Mdv2Beam::badVal );
      } else {
	_beamData.push_back( total / double(numToAvg) );
      }
    } 
  }
  return _beamData;
}


//
// Desructor -does nothing.
//
Mdv2Beam::~Mdv2Beam(){
  return;
}


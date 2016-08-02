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
// doPrint.cc
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 2003
//

#include <cstdlib>
#include "doPrint.hh"
#include <Mdv/MdvxField.hh>
using namespace std;

// constructor
doPrint::doPrint (Params *TDRP_Params, int refNum){
  _params = TDRP_Params;
  _firstTime = true;
  _dataOK = false;
  _lastTime = 0L;
  _refNum = refNum;
  _Proj = NULL;
  _Fhdr = NULL;
  _Vhdr = NULL;
}


// destructor.

doPrint::~doPrint(){
  delete _Proj;
  delete _Fhdr;
  delete _Vhdr;
}

  // Do a printout of data at the time/location specified.

void doPrint::printData(time_t dataTime,
			double lat,
			double lon){

  //
  // If this is our first go around, then of course we need 
  // to update the data.
  //
  if (_firstTime){
    _lastTime = dataTime; 
    _dataUpdate(dataTime);
    _firstTime = false;
  }
  //
  // If the data time hase changed, then we need to
  // update the data.
  //
  if (dataTime != _lastTime){
    _lastTime = dataTime;
    _dataUpdate(dataTime);
  }

  if (!(_dataOK)){
    fprintf(stderr,"%s :\tNO DATA\n", 
	    _params->_sources[_refNum].fieldName);
    return;
  }

  //
  // Get the indicies.
  //
  int ixc,iyc;
  if (_Proj->latlon2xyIndex(lat, lon, ixc, iyc)){
    fprintf(stderr,"%s : point %g, %g outside grid.\n", 
	    _params->_sources[_refNum].fieldName, 
	    lat, lon);
  return;
  }

  int method =  _params->_sources[_refNum].method;
  int spreadPoints = _params->_sources[_refNum].spreadPoints;
  double minLevel = _params->_sources[_refNum].minLevel;
  double maxLevel = _params->_sources[_refNum].maxLevel;


  int numFound = 0;
  double min=0.0, max=0.0, total=0.0;

  for (int iz=0; iz < _Fhdr->nz; iz++){

    if (
	(_Fhdr->nz == 1) || // For 2D data don't worry re heights.
	((_Vhdr->level[iz] <= maxLevel) && (_Vhdr->level[iz] >= minLevel))
	){

      for (int ixx = ixc - spreadPoints; ixx <= ixc + spreadPoints; ixx++){
	for (int iyy = iyc - spreadPoints; iyy <= iyc + spreadPoints; iyy++){

	  if (
	      (ixx > -1) && (iyy > -1) &&
	      (ixx < _Fhdr->nx) &&
	      (iyy < _Fhdr->ny)
	      ){

	    int index = iz * _Fhdr->nx * _Fhdr->ny + iyy * _Fhdr->nx + ixx;

	    if (
		(_dataPointer[index] != _Fhdr->bad_data_value) &&
		(_dataPointer[index] != _Fhdr->missing_data_value)
		){
	      if (numFound == 0){
		max = _dataPointer[index];
		min=max;
	      } else {
		if (max < _dataPointer[index]) max = _dataPointer[index];
		if (min > _dataPointer[index]) min = _dataPointer[index];
	      }
	      numFound++;
	      total+=_dataPointer[index];
	    }
	  }
	}
      }

    }

  }
  //
  // If we are the first object to print, then print a few
  // blank lines followed by a header.
  //
  if (_refNum == 0){
    for (int i=0; i < _params->numHeaderBlanks; i++){
      fprintf(stderr,"\n");
    }
 
    fprintf(stderr,"Time : %s Lat : %g Lon : %g\n",
	    utimstr( dataTime ), lat, lon );
  }
  //
  // If we found no data, say so and return.
  //
  if (numFound == 0){
    if (_params->printFieldTimes){
      fprintf(stderr,"%s : ", utimstr(_actualDataTime));
    }

    fprintf(stderr,"%s :\tBAD\n", 
	    _params->_sources[_refNum].fieldName);
    return;
  }

  double mean = total / double(numFound);
  //
  // Print what they asked for.
  //
  double dataVal;
  switch (method) {

  case 0 :
    dataVal = min;
    break;

  case 1 :
    dataVal = max;
    break;

  case 2 :
    dataVal = mean;
    break;

  default :
    fprintf(stderr, "Method must be 0, 1 or 2 - exiting.\n");
    exit(-1);
    break;

  }
  //
  // Print the data.
  //
  if (_params->printFieldTimes){
    fprintf(stderr,"%s : ", utimstr(_actualDataTime));
  }

  fprintf(stderr,"%s :\t%g\n", 
	  _params->_sources[_refNum].fieldName,
	  dataVal);

  return;
}
 

void doPrint::_dataUpdate(time_t dataTime){

  if ((_refNum == 0) && (_params->debug)){
    time_t now = time(NULL);
    fprintf(stderr,"Data update requested at %s\n",
	    utimstr(now));
  }

  _dataOK = false;
  if (!(_firstTime)){
    if (_Proj != NULL) delete _Proj;
    if (_Fhdr != NULL) delete _Fhdr;
    if (_Vhdr != NULL) delete _Vhdr;
  }

  _mdvxObject.setReadTime(Mdvx::READ_FIRST_BEFORE, 
			  _params->_sources[_refNum].url, 
			  _params->_sources[_refNum].lookBack, 
			  dataTime);
  _mdvxObject.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _mdvxObject.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _mdvxObject.addReadField( _params->_sources[_refNum].fieldName );

  if (_mdvxObject.readVolume()){
    if (_params->debug){
      fprintf(stderr, "Read failed at %s from %s\n",
	      utimstr(dataTime),  _params->_sources[_refNum].url);
    }
    return;
  }

  MdvxField *InField;

  // Should be field 0 since that's all we requested with addField. 

  InField = _mdvxObject.getFieldByNum( 0 );

  if (InField == NULL){
    if (_params->debug){
      fprintf(stderr,"Unable to find field %s\n", 
	      _params->_sources[_refNum].fieldName );
    }
    return;
  }
  //
  // Get the master, field and vlevel headers.
  //
  Mdvx::master_header_t InMhdr = _mdvxObject.getMasterHeader();   
  Mdvx::field_header_t InFhdr = InField->getFieldHeader();
  Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

  _Fhdr = new  Mdvx::field_header_t( InFhdr );
  _Vhdr = new  Mdvx::vlevel_header_t( InVhdr );

  _actualDataTime = InMhdr.time_centroid;

  // Set up the Proj, and return a pointer to the data.
  
  _Proj = new MdvxProj(InMhdr, InFhdr);
 
  _dataPointer = (fl32 *) InField->getVol();

  _dataOK = true;

}

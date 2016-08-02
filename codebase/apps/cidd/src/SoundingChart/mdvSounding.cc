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
//
// Object that creates and stores an MDV sounding.
//

#include "mdvSounding.hh"
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <stdlib.h>

const double mdvSounding::badVal = -999.0;

//
// Constructor. Reads the data and fills up the vectors.
//
mdvSounding::mdvSounding(time_t dataTime,
			 string url,
			 double lat,
			 double lon,
			 vector <string> fieldNames){ // May want to pass this const - ask mike
  //
  //
  //
  _data = NULL;
  _ok = false;
  //
  // Copy the time.
  //
  _dataTime = dataTime;
  //
  // Read the MDV data for this time.
  //
  DsMdvx mdvObj;

  mdvObj.setReadTime(Mdvx::READ_FIRST_BEFORE, url, 0, dataTime);
  mdvObj.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvObj.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  mdvObj.addReadWayPt(lat, lon);

  //
  // Add the desired field names to the read list.
  //
  for (unsigned int i=0; i < fieldNames.size(); i++){
    mdvObj.addReadField(fieldNames[i]);
  }
  //
  // Read the data.
  //
  if (mdvObj.readVsection()){
    cerr << "Read failed at " << utimstr(dataTime) << " from ";
    cerr << url  << endl;
    cerr << mdvObj.getErrStr() << endl;
    return;
  }
  //
  // Get the master field header.
  //
  Mdvx::master_header_t Mhdr = mdvObj.getMasterHeader();
  _numLevels = Mhdr.max_nz;
  if((_data = (double *)malloc( _numLevels * (1 + fieldNames.size()) * sizeof(double))) == NULL) {
	cerr << "Malloc failed." << endl;
	exit(-1);
  }
  bool levelsOK = false;

  for (unsigned int ifield = 0; ifield < fieldNames.size(); ifield++){

    MdvxField *Field = mdvObj.getFieldByName( fieldNames[ifield] );

    if (Field == NULL){
      cerr << "Failed to find field " << fieldNames[ifield] << endl;
      return;
    }
    Mdvx::field_header_t Fhdr =  Field->getFieldHeader();
    Mdvx::vlevel_header_t Vhdr = Field->getVlevelHeader();

    if (Fhdr.nz == _numLevels && levelsOK == false){ // If this field hold all levels - record them.
      //
      // Record the vlevels.
      //
      for (int j=0; j <  _numLevels; j++){
	_data[j * (1 + fieldNames.size())] = Vhdr.level[j];
      }
      levelsOK = true;
    } 
    //
    // Extract the data into our array.
    //
    fl32 *fieldData = (fl32 *) Field->getVol();

    for (int iz =0; iz < _numLevels; iz++){
      if (iz >= Fhdr.nz || 
	  (fieldData[iz] == Fhdr.bad_data_value) ||
	  (fieldData[iz] == Fhdr.missing_data_value)
	  ){
	_data[iz*(1 + fieldNames.size()) + 1 + ifield] = badVal;
      } else {
	_data[iz*(1 + fieldNames.size()) + 1 + ifield] = fieldData[iz];
      }
    }
  }

  _ok = true;
  _numFields = fieldNames.size();
  return;

}

time_t mdvSounding::getTime(){
  return _dataTime;
}

int mdvSounding::getNlevels(){
  return _numLevels;
}

bool mdvSounding::isOK(){
  return _ok;
}

double *mdvSounding::getData(){
  if (!(_ok)) return NULL;
  return _data;
}


//
// Destructor
//
mdvSounding::~mdvSounding(){
    if (_ok) free(_data);
}


void mdvSounding::Print(){
  
  if (!(_ok)){
    cerr << "No data to print." << endl;
    return;
  }

  cerr << "MDV Sounding data at " << utimstr(_dataTime) << endl;

  for (int il = 0; il < _numLevels; il++){
    cerr << "Level : " <<  _data[il * (_numFields+1)] << " : ";
    for (int ifd = 0; ifd < _numFields; ifd++){
      cerr << _data[il*(_numFields+1) + 1 + ifd] << " ";
    }
    cerr << endl;
  }

}



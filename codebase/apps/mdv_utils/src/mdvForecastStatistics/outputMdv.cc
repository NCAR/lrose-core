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
//
// Niles Oien, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2006
//
/////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>

#include "outputMdv.hh"

/**
 *
 * @file outputMdv.cc
 *
 * Implementation of outputMdv class.
 *
 * @author Niles Oien
 *
 */

using namespace std;

outputMdv::outputMdv(string url,
		     string name,
		     string forecastFilename,
		     string truthFilename,
		     Mdvx::master_header_t mhdr,
		     Mdvx::field_header_t fhdr,
		     Mdvx::vlevel_header_t vhdr){

  _forecastMode = false; // Default.
  _url = url + "/" + name; // Append name as subdir to url.

  _mhdr = mhdr;
  sprintf(_mhdr.data_set_info, "Forecast file : %s\nTruth file : %s",
	  forecastFilename.c_str(), truthFilename.c_str());
  sprintf(_mhdr.data_set_name, "Intermediate validation grids");
  sprintf(_mhdr.data_set_source, "mdvForecastStatistics");
  _outMdvx.setMasterHeader( _mhdr );
  _outMdvx.clearFields();

  _fhdr = fhdr;
  _vhdr = vhdr;

  _vhdr2D.type[0] =  Mdvx::VERT_TYPE_SURFACE;
  _vhdr2D.level[0] = 0.0;

  return;

}


void outputMdv::addField(fl32 *data,
			 fl32 bad_data_value,
			 fl32 missing_data_value,
			 string name,
			 string longName,
			 string units,
			 bool is3D){

  if (data==NULL) return;
  //
  // Set up a field header. Start with a copy of what was passed
  // into the constructor.
  //
  Mdvx::field_header_t fhdr = _fhdr;
  fhdr.bad_data_value = bad_data_value;
  fhdr.missing_data_value = missing_data_value;
  if (!(is3D)) fhdr.nz = 1;
  fhdr.volume_size = sizeof(fl32)*fhdr.nx*fhdr.ny*fhdr.nz;

  //
  // Set up a field.
  //
  MdvxField *fld;
  if (is3D)
    fld = new MdvxField(fhdr, _vhdr, data);
  else
    fld = new MdvxField(fhdr, _vhdr2D, data);

  fld->setUnits( units.c_str() );
  fld->setFieldName( name.c_str() );
  fld->setFieldNameLong( longName.c_str() );

  if (fld->convertRounded(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }  

  _outMdvx.addField( fld );

  return;
}



void outputMdv::addField(ui08 *data,
			 fl32 bad_data_value,
			 fl32 missing_data_value,
			 string name,
			 string longName,
			 string units,
			 bool is3D){

  if (data==NULL) return;

  int size = _fhdr.nx*_fhdr.ny;
  if (is3D) size *= _fhdr.nz;

  fl32 *fdata = (fl32 *) malloc(size * sizeof(fl32));
  if (fdata == NULL){
    cerr << "Malloc failed in ui08 outputMdv::addField, size=" << size << endl;
    exit(-1);
  }

  for (int i=0; i < size; i++){
    fdata[i] = (fl32)data[i];
  }

  outputMdv::addField(fdata, bad_data_value, missing_data_value,
		      name, longName, units, is3D);

  free(fdata);

  return;
}


void outputMdv::setForecastMode( bool forecastMode ){
  _forecastMode = forecastMode;
  return;
}


outputMdv::~outputMdv(){

  //
  // Only do the write if fields were added. Otherwise we wind
  // up with MDV files with no fields in them which is just silly
  // in this case.
  //
  Mdvx::master_header_t Mhdr = _outMdvx.getMasterHeader();

  if (Mhdr.n_fields < 1){
    cerr << "Cowardly refusing to write an MDV file with ";
    cerr << Mhdr.n_fields << " fields in it." << endl;     
  } else {
    if (_forecastMode)
      _outMdvx.setWriteAsForecast();

    if (_outMdvx.writeToDir(_url)) {
      cerr << "Failed to wite to " << _url << endl;
      cerr << _outMdvx.getErrStr() << endl;
    }      
  }
  return;
}
  
